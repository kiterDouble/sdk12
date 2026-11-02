#include "param_encrypt.h"
#include "err_code.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <string.h>
#include <stdio.h>

static const char *key_str = "ParamAES256Key";
static const char *iv_str = "ParamAES256Iv";


int32_t ParamAES256CBCEncrypt(unsigned char *plaintext, int32_t plaintext_len, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int32_t len;

    int32_t ciphertext_len;

    unsigned char key[32] = {0}; // 256位密钥
    unsigned char iv[16] = {0};  // 128位IV
    strncpy((char *)key, key_str, sizeof(key) - 1);
    strncpy((char *)iv, iv_str, sizeof(iv) - 1);


    /* 创建并初始化上下文 */
    if (!(ctx = EVP_CIPHER_CTX_new()))
        return RET_ERR;

    /* 初始化加密操作 */
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        return RET_ERR;

    /* 提供要加密的数据，并获得加密后的输出 */
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        return RET_ERR;
    ciphertext_len = len;

    /* 处理可能剩余的最后部分数据 */
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        return RET_ERR;
    ciphertext_len += len;

    /* 清理 */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int32_t ParamAES256CBCDecrypt(unsigned char *ciphertext, int32_t ciphertext_len, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;

    int32_t len;

    int32_t plaintext_len;

    unsigned char key[32] = {0}; // 256位密钥
    unsigned char iv[16] = {0};  // 128位IV
    strncpy((char *)key, key_str, sizeof(key) - 1);
    strncpy((char *)iv, iv_str, sizeof(iv) - 1);

    /* 创建并初始化上下文 */
    if (!(ctx = EVP_CIPHER_CTX_new()))
        return RET_ERR;

    /* 初始化解密操作 */
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        return RET_ERR;

    /* 提供要解密的数据，并获得解密后的输出 */
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        return RET_ERR;
    plaintext_len = len;

    /* 处理可能剩余的最后部分数据 */
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        return RET_ERR;
    plaintext_len += len;

    /* 清理 */
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

