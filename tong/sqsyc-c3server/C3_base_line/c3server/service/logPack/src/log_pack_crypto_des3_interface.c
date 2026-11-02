#include <string.h>

#include "sr_log.h"
#include "log_pack_crypto_interface.h"

#define ITER_NUMBER 10000
#define KEY_SIZE 24
#define IV_SIZE 8
#define SALT_SIZE 8
#define PREFIX_SIZE 8
#define PREFIX "Salted__"


/* 使用 HMAC - SHA256 实现类似 PBKDF2 的密钥派生 */
static void Pbkdf2HmacSha256(const unsigned char *password, int password_len,
    const unsigned char *salt, int salt_len,
    int iterations, unsigned char *dk, int dk_len) {
    int hlen = EVP_MD_size(EVP_sha256());
    int l = (dk_len + hlen - 1) / hlen;
    int r = dk_len - (l - 1) * hlen;
    unsigned char t[EVP_MAX_MD_SIZE];
    unsigned char u[EVP_MAX_MD_SIZE];
    unsigned char block[SALT_SIZE + 4];
    int i, j, k;

    for (i = 1; i <= l; i++) {
        memcpy(block, salt, salt_len);
        block[salt_len] = (i >> 24) & 0xff;
        block[salt_len + 1] = (i >> 16) & 0xff;
        block[salt_len + 2] = (i >> 8) & 0xff;
        block[salt_len + 3] = i & 0xff;

        // 第一次 HMAC - SHA256 计算
        HMAC(EVP_sha256(), password, password_len, block, salt_len + 4, u, NULL);
        memcpy(t, u, hlen);

        for (j = 1; j < iterations; j++) {
            HMAC(EVP_sha256(), password, password_len, u, hlen, u, NULL);
            for (k = 0; k < hlen; k++) {
                t[k] ^= u[k];
            }
        }

        int copy_len = (i == l) ? r : hlen;
        memcpy(dk + (i - 1) * hlen, t, copy_len);
    }
}


int des3_init_interface(){
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    return 0;
}

int des3_start_interface(Crypto_Dev_t* dev, const char* password, uint8_t* prefixData, int32_t* prefixDataLen)
{
    /* 盐值 */
    unsigned char salt[SALT_SIZE] = {0};

    /* 使用 PBKDF2 派生密钥和 IV */ 
    unsigned char derived[KEY_SIZE + IV_SIZE];
    unsigned char key[KEY_SIZE] = {0};
    unsigned char iv[SALT_SIZE] = {0};

    if(dev == NULL || prefixData == NULL || prefixDataLen == NULL)
        return -1;

    dev->algo = CRYPTO_ALGO_DES3;
    dev->u.des3_ctx = NULL;
    
    /* 生成salt随机数 */
    if (RAND_bytes(salt, SALT_SIZE) != 1) {
        ERR_print_errors_fp(stderr);
        LOG_Error("generate random error!!! \r\n");
        return -2;
    }
    
    /* "Salted__ + 随机数 " */
    memcpy(prefixData, PREFIX, PREFIX_SIZE);
    memcpy(prefixData + PREFIX_SIZE, salt, SALT_SIZE);
    *prefixDataLen = PREFIX_SIZE + SALT_SIZE;
    
#if 0
    /** 该方法生成 key 和 iv 与终端指令不一致，导致指令执行失败 **/
    if (EVP_BytesToKey(EVP_des_ede3_cbc(), EVP_sha256(), salt, (unsigned char *)password, strlen(password), ITER_NUMBER, key, iv) != KEY_SIZE) {
        ERR_print_errors_fp(stderr);
        LOG_Error("EVP_CIPHER_CTX_new error!!! \r\n");
        return -13;
    }
#else
    Pbkdf2HmacSha256((uint8_t*)password, strlen(password), salt, SALT_SIZE, ITER_NUMBER, derived, KEY_SIZE + IV_SIZE);
    memcpy(key, derived, KEY_SIZE);
    memcpy(iv, derived + KEY_SIZE, IV_SIZE);
#endif

    /* 创建和初始化 加密上下文 */
    dev->u.des3_ctx = EVP_CIPHER_CTX_new();
    if (!dev->u.des3_ctx) {
        ERR_print_errors_fp(stderr);
        LOG_Error("EVP_CIPHER_CTX_new error!!! \r\n");
        return -3;
    }

    if (EVP_EncryptInit_ex(dev->u.des3_ctx, EVP_des_ede3_cbc(), NULL, key, iv) != 1) {
        ERR_print_errors_fp(stderr);
        LOG_Error("EVP_EncryptInit_ex error!!! \r\n");
        return -4;
    }

    /* 设置填充方式为 PKCS#7, 默认就是PKCS#7, 可以不调用 */
    EVP_CIPHER_CTX_set_padding(dev->u.des3_ctx, 1);

    return 0;
}

int des3_encrypt_interface(Crypto_Dev_t* dev, uint8_t* plainData, int32_t plainDataLen, uint8_t* cryptoData, int32_t* cryptoDataLen)
{
    if(dev == NULL || dev->algo != CRYPTO_ALGO_DES3 || dev->u.des3_ctx == NULL || plainData == NULL || plainDataLen <= 0 || cryptoData == NULL || cryptoDataLen == NULL)
        return -1;

    if (EVP_EncryptUpdate(dev->u.des3_ctx, cryptoData, cryptoDataLen, plainData, plainDataLen) != 1) {
        ERR_print_errors_fp(stderr);
        LOG_Error("EVP_EncryptUpdate error!!! \r\n");
        return -2;
    }

    return 0;
}

int des3_finish_interface(Crypto_Dev_t* dev, uint8_t* tailData, int32_t* tailDataLen)
{
    if(dev == NULL || dev->algo != CRYPTO_ALGO_DES3 || dev->u.des3_ctx == NULL || tailData == NULL || tailDataLen == NULL)
        return -1;

    if (EVP_EncryptFinal_ex(dev->u.des3_ctx, tailData, tailDataLen) != 1) {
        ERR_print_errors_fp(stderr);
        LOG_Error("EVP_EncryptFinal_ex error!!! \r\n");
        return -2;
    }

    return 0;
}

int des3_free_interface(Crypto_Dev_t* dev)
{
    if(dev == NULL)
        return -1;

    if(dev->algo == CRYPTO_ALGO_DES3 && dev->u.des3_ctx != NULL){
        EVP_CIPHER_CTX_free(dev->u.des3_ctx);
        /** 清空，只释放一次 **/
        dev->u.des3_ctx = NULL;
    }

    return 0;
}

int des3_deinit_interface(){
    EVP_cleanup();
    ERR_free_strings();
    return 0;
}

const Crypto_interface_t g_des3_crypto_interface = {
    .name = "des3",
    .crypto_init = des3_init_interface,
    .crypto_start = des3_start_interface,
    .crypto_encrypt = des3_encrypt_interface,
    .crypto_finish = des3_finish_interface,
    .crypto_free = des3_free_interface,
    .crypto_deinit = des3_deinit_interface,
};
