#include "sr_engine.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sr_log.h"
#include "sr_hal_hsm.h"

static const char *eng_id = "engine-sirun";
static const char *eng_name = "engine for OpenSSL(sirun)";

/*
将hash编码为ASN.1 DER数据
hash[in] : 源数据
hashlen[in] : 源数据长度
der[out] : 编码后的数据，传入NULL时，可以获得编码后的数据长度
derlen[out] : 编码后的数据长度
type[in] : 声明哈希的算法ID，NID_sha256 NID_sha256WithRSAEncryption type不一样会导致生成的der头不一样，比如:
NID_sha256：der: 30 31 30 0d 06 09 60 86 48 01 65 03 04 02 01 05 00 04 20 + 32byte sha256
NID_sha256WithRSAEncryption：der: 30 31 30 0d 06 09 2a 86 48 86 f7 0d 01 01 0b 05 00 04 20 + 32byte sha256
在生成csr文件的过程中，需要使用NID_sha256WithRSAEncryption算法，因为openssl req -in test.csr -text解析出来的Signature Algorithm: sha256WithRSAEncryption
*/
int RSA_sig_to_der(const unsigned char *hash, unsigned int hashlen, unsigned char *der, unsigned int *derlen, int type)
{
    int ret = -1;
    X509_SIG *xsig;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
#else
	X509_ALGOR *algor;
	ASN1_OCTET_STRING *pdigest;
#endif
    do
    {
        // 不同于SM2算法的签名转换，此定义已在OpenSSL中声明
        // SM2的签名数据需要自己做声明
        xsig = X509_SIG_new();
        if (!xsig)
        {
            break;
        }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
        if (!X509_ALGOR_set0(xsig->algor, OBJ_nid2obj(type), V_ASN1_NULL, NULL))
        {
            break;
        }
        if (!ASN1_OCTET_STRING_set(xsig->digest, hash, hashlen))
        {
            break;
        }
#else
        // 获得algor和pdigest在xsig中的地址，从而设置xsig的值
        X509_SIG_getm(xsig, &algor, &pdigest);
        if (!X509_ALGOR_set0(algor, OBJ_nid2obj(type), V_ASN1_NULL, NULL))
        {
            break;
        }
        if (!ASN1_OCTET_STRING_set(pdigest, hash, hashlen))
        {
            break;
        }
#endif
        if (der)
        {
            *derlen = i2d_X509_SIG(xsig, &der);
        }
        else
        {
            *derlen = i2d_X509_SIG(xsig, NULL);
        }
        ret = 0;
    } while (0);

    if (xsig)
    {
        X509_SIG_free(xsig);
    }
    return ret;
}

/*
将ASN1编码数据转换为hash
pbDer[in] : ASN1编码数据
uiDerLen[in] : ASN1编码数据长度
hash[out] : 哈希数据
hashlen[out] : 哈希数据长度
*/
int RSA_sig_to_hash(const unsigned char *pbDer, unsigned int uiDerLen, unsigned char *hash, unsigned int *hashlen)
{
    int ret = -1;
    X509_SIG *xsig = NULL;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
#else
    const ASN1_OCTET_STRING *doct = NULL;
#endif
    const unsigned char *data = NULL;
    int datalen = 0;

    do
    {
        xsig = d2i_X509_SIG(NULL, &pbDer, uiDerLen);
        if (!xsig)
        {
            break;
        }
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        datalen = ASN1_STRING_length(xsig->digest);
        if (datalen <= 0)
        {
            break;
        }
        data = ASN1_STRING_data(xsig->digest);
        if (!data)
        {
            break;
        }
#else
        // 这里没有对哈希算法进行解析，如需解析，把NULL变为变量再解析
        X509_SIG_get0(xsig, NULL, &doct);

        datalen = ASN1_STRING_length(doct);
        if (datalen <= 0)
        {
            break;
        }
        data = ASN1_STRING_get0_data(doct);
        if (!data)
        {
            break;
        }
#endif

        memcpy(hash, data, datalen);
        *hashlen = datalen;
        ret = 0;
    } while (0);

    if (xsig)
    {
        X509_SIG_free(xsig);
    }
    return ret;
}

int RSA_signature(const unsigned char *m, unsigned int m_length,
                  unsigned char *sigret, unsigned int *siglen, RSA *rsa)
{
    LOG_Debug("RSA_signature m_length: %d", m_length);
    unsigned char hash[SHA256_DIGEST_LENGTH] = {0};
    // 先用openssl接口算sha256
    SHA256(m, m_length, hash);
    // 签名，会将原始数据传给 my_rsa_sign 进行签名
    return RSA_sign(0, hash, SHA256_DIGEST_LENGTH, sigret, siglen, rsa); // my_rsa_sign
}

int RSA_verifySignature(const unsigned char *m, unsigned int m_length,
                        const unsigned char *sigbuf, unsigned int siglen, RSA *rsa)
{
    unsigned char hash[SHA256_DIGEST_LENGTH] = {0};
    // 先用openssl接口算原始消息sha256
    SHA256(m, m_length, hash);
    // 验签 会调用 my_rsa_verify 进行验签
    return RSA_verify(0, hash, SHA256_DIGEST_LENGTH, sigbuf, siglen, rsa); // my_rsa_verify
}

static int myengine_init(ENGINE *e)
{
    LOG_Debug("myengine_init");
    return HalHsmInit();
}
static int myengine_finish(ENGINE *e)
{
    LOG_Debug("myengine_finish");
    HalHsmDeinit();
    return 1;
}
static int myengine_destroy(ENGINE *e)
{
    LOG_Debug("myengine_destroy");
    return 1;
}

static int myrand_status(void)
{
    LOG_Debug("myrand_status");
    return 1;
}

static int myrand_bytes(unsigned char *buf, int num)
{
    LOG_Debug("myrand_bytes number:%d", num);
    return HalHsmRand(buf, num);
}

static const RAND_METHOD rand_method =
    {
        NULL,
        myrand_bytes,
        NULL,
        NULL,
        NULL,
        myrand_status};

#if OPENSSL_VERSION_NUMBER < 0x10100000L
static int my_engine_sha256_update(EVP_MD_CTX *ctx, const void *data, size_t count);
static int my_engine_sha256_init(EVP_MD_CTX *ctx)
{
    ctx->update = &my_engine_sha256_update;

    LOG_Debug("initialized! SHA256");
    return 1;
}

static int my_engine_sha256_update(EVP_MD_CTX *ctx, const void *data, size_t count)
{
    LOG_Debug("SHA256 update ! size:%d", count);
    return HalHsmSHA256Update(ctx, data, count);
}

static int my_engine_sha256_final(EVP_MD_CTX *ctx, unsigned char *md)
{
    LOG_Debug("SHA256 final size of EVP_MD: %d", sizeof(EVP_MD));
    memcpy(md, ctx->md_data, ctx->digest->md_size);
    return 1;
}

int my_engine_sha256_copy(EVP_MD_CTX *to, const EVP_MD_CTX *from)
{
    LOG_Debug("Copy SHA256");
    if (to->md_data && from->md_data)
    {
        memcpy(to->md_data, from->md_data, sizeof(*from->md_data));
    }
    return 1;
}

static int my_engine_sha256_clean(EVP_MD_CTX *ctx)
{
    LOG_Debug("SHA256 cleanup");
    if (ctx->md_data)
        memset(ctx->md_data, 0, ctx->digest->md_size);
    return 1;
}

// 摘要算法 digest
// typedef struct env_md_st EVP_MD
static EVP_MD my_sha256_engine = {
    NID_sha256,
    0,
    32,
    EVP_MD_FLAG_DIGALGID_ABSENT,
    my_engine_sha256_init,
    my_engine_sha256_update,
    my_engine_sha256_final,
    my_engine_sha256_copy,
    my_engine_sha256_clean,
    NULL,
    NULL,
    {0},
    64,
    32,
    NULL};
#else
#endif

static int my_rsa_pub_enc(int flen, const unsigned char *from,
                          unsigned char *to, RSA *rsa, int padding)
{
    LOG_Debug("my_rsa_pub_enc start-------------------------------------------------------------------------------------------------------------------");
    int rsabits = 2048;
    // 需要传入的rsa里面有公钥n和e，用来判断bit长度，判断用RSA1024、1280、2048哪种方式进行加密
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (rsa != NULL && rsa->n != NULL)
    {
        rsabits = RSA_size(rsa) * 8;
    }
#else
    if (rsa != NULL)
    {
        const BIGNUM *n = NULL;
        RSA_get0_key(rsa, &n, NULL, NULL);
        if (n != NULL)
        {
            rsabits = RSA_size(rsa) * 8;
        }
    }
#endif
    // pkcs随机数 先生成256个非0随机数备用
    unsigned char rand_buf[256] = {0};
    int err = RAND_bytes(rand_buf, 256);
    LOG_Debug("RAND_bytes ret : %x", err);
    for (int i = 0; i < 256; i++)
    {
        // 保持数据没有0
        if (rand_buf[i] == 0x00)
            rand_buf[i] = 0xff;
    }

    int ret = HalHsmRSAPubEnc(flen, rsabits, rand_buf, from, to);
    LOG_Debug("my_rsa_pub_enc end-------------------------------------------------------------------------------------------------------------------");
    return ret;
}
static int my_rsa_pub_dec(int flen, const unsigned char *from,
                          unsigned char *to, RSA *rsa, int padding)
{
    LOG_Debug("my_rsa_pub_dec start");
    // for (int i = 0; i < flen; i++)
    // {
    //     to[i]=from[i]-10;
    // }
    return flen;
}
static int my_rsa_priv_enc(int flen, const unsigned char *from,
                           unsigned char *to, RSA *rsa, int padding)
{
    LOG_Debug("my_rsa_priv_enc");
    return 1;
}
static int my_rsa_priv_dec(int flen, const unsigned char *from,
                           unsigned char *to, RSA *rsa, int padding)
{
    LOG_Debug("my_rsa_priv_dec start! -------------------------------------------------------------------------------------------------------------------");
    int rsabits = 2048;
    // 需要传入的rsa里面有公钥n和e，用来判断bit长度，判断用RSA1024、1280、2048哪种方式进行加密
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (rsa != NULL && rsa->n != NULL)
    {
        rsabits = RSA_size(rsa) * 8;
    }
#else
    if (rsa != NULL)
    {
        const BIGNUM *n = NULL;
        RSA_get0_key(rsa, &n, NULL, NULL);
        if (n != NULL)
        {
            rsabits = RSA_size(rsa) * 8;
        }
    }
#endif
    int ret = HalHsmRSAPrivDec(flen, rsabits, from, to);
    LOG_Debug("my_rsa_priv_dec end! -------------------------------------------------------------------------------------------------------------------");
    return ret;
}

static int my_rsa_sign(int type, const unsigned char *m, unsigned int m_length,
                       unsigned char *sigret, unsigned int *siglen, const RSA *rsa)
{
    // 传入的已经是一个sha256了
    LOG_Debug("my_rsa_sign start,m_length:%d,type:%d-------------------------------------------------------------------------------------------------", m_length, type);

    // 对sha256进行签名进行ASN.1编码,加上杂凑算法的国际OID,得到一串稍微长一点的DER数据
    unsigned char der[128] = {0};
    unsigned int derlen = 0;
    int ret = RSA_sig_to_der(m, m_length, der, &derlen, NID_sha256WithRSAEncryption); // 在32byte sha256前加入编码
    if (ret != 0)
    {
        return -1;
    }
    // 使用RSA私钥计算签名
    int rsabits = 2048;
    // 需要传入的rsa里面有公钥n和e，用来判断bit长度，判断用RSA1024、1280、2048哪种方式进行加密
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (rsa != NULL && rsa->n != NULL)
    {
        rsabits = RSA_size(rsa) * 8;
    }
#else
    if (rsa != NULL)
    {
        const BIGNUM *n = NULL;
        RSA_get0_key(rsa, &n, NULL, NULL);
        if (n != NULL)
        {
            rsabits = RSA_size(rsa) * 8;
        }
    }
#endif
    ret = HalHsmRSASign(rsabits, der, derlen, sigret, siglen);
    LOG_Debug("my_rsa_sign end---------------------------------------------------------------------------------------------------------");
    return ret;
}

static int my_rsa_verify(int dtype, const unsigned char *m,
                         unsigned int m_length, const unsigned char *sigbuf,
                         unsigned int siglen, const RSA *rsa)
{
    LOG_Debug("my_rsa_verify start---------------------------------------------------------------------------------------------------------");
    // 使用RSA公钥进行签名验证
    int rsabits = 2048;
    // 需要传入的rsa里面有公钥n和e，用来判断bit长度，判断用RSA1024、1280、2048哪种方式进行加密
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (rsa != NULL && rsa->n != NULL)
    {
        rsabits = RSA_size(rsa) * 8;
    }
#else
    if (rsa != NULL)
    {
        const BIGNUM *n = NULL;
        RSA_get0_key(rsa, &n, NULL, NULL);
        if (n != NULL)
        {
            rsabits = RSA_size(rsa) * 8;
        }
    }
#endif
    uint16_t rlen;
    uint8_t rbuf[300] = {0};
    int ret = HalHsmRSAVerify(rsabits,&rlen,rbuf,sigbuf,siglen);
    if(ret == -1)
    {
        return -1;
    }
    LOG_Debug("my_rsa_verify end---------------------------------------------------------------------------------------------------------");
    // 验签后的数据也是pkcs1填充的，所以需要找到第二个0x00出现的位置，从第二个0x00之后才是der数据
    int count = 0;
    unsigned char hash[64] = {0};
    unsigned int hashlen = 0;
    for (size_t i = 0; i < rlen; i++)
    {
        if (rbuf[i] == 0x00)
        {
            count++;
            if (count == 2)
            {
                // return ((memcmp(m, rbuf+i+1, m_length) == 0) ? 1 : 0);
                int ret = RSA_sig_to_hash(rbuf + i + 1, rlen - i - 1, hash, &hashlen);
                if (ret != 0)
                {
                    return 0;
                }
                else
                {
                    return ((memcmp(m, hash, m_length) == 0) ? 1 : 0);
                }
            }
        }
    }
    return 0;
}

static int my_rsa_init(RSA *rsa)
{
    LOG_Debug("my_rsa_init");
    return 1;
}

static int my_rsa_finish(RSA *rsa)
{
    LOG_Debug("my_rsa_finish");
    return 1;
}

static int my_rsa_keygen(RSA *rsa, int bits, BIGNUM *e, BN_GENCB *cb)
{

    BIGNUM *n = NULL;
    BIGNUM *d = NULL;
    int ret = HalHsmRSAKeygen(bits,e,&n,&d);
    if(ret == -1)
    {
        return -1;
    }
    // LOG_Debug("n:");
    // BN_print_fp(stdout, n);
    // LOG_Debug("");
    // LOG_Debug("e:");
    // BN_print_fp(stdout, e);
    // LOG_Debug("");
    if (!n || !d)
    {
        BN_free(n);
        BN_free(d);
        return 0; // 转换失败
    }
    // 公钥指数e，加密芯片支持e=65537或3 默认限制为65537

    // 将生成的密钥设置到 RSA 结构体中
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    rsa->n = BN_dup(n);
    rsa->e = BN_dup(e);
    rsa->d = BN_dup(d);
    BN_free(n);
    BN_free(d);
#else
    if (RSA_set0_key(rsa, n, e, NULL) != 1)
    {
        LOG_Debug("Failed to set RSA key");
    }
#endif
    return 1;
}


#if OPENSSL_VERSION_NUMBER < 0x10100000L
const RSA_METHOD rsa_method = {
    "rsa_engine_test",
    my_rsa_pub_enc,
    my_rsa_pub_dec,
    my_rsa_priv_enc,
    my_rsa_priv_dec,
    NULL,
    NULL,
    my_rsa_init,
    my_rsa_finish,
    ENGINE_METHOD_RSA | RSA_FLAG_SIGN_VER,
    NULL,
    my_rsa_sign,
    my_rsa_verify,
    my_rsa_keygen};
#else
RSA_METHOD *rsa_method = NULL;
int RSA_meth_init(void)
{
    // rsa_method = RSA_meth_new("rsa_engine_sirun", ENGINE_METHOD_RSA | RSA_FLAG_SIGN_VER);
    rsa_method = RSA_meth_new("rsa_engine_sirun", 0);
    if (rsa_method == NULL)
    {
        return -1;
    }
    RSA_meth_set_pub_enc(rsa_method, my_rsa_pub_enc);
    RSA_meth_set_pub_dec(rsa_method, my_rsa_pub_dec);
    RSA_meth_set_priv_enc(rsa_method, my_rsa_priv_enc);
    RSA_meth_set_priv_dec(rsa_method, my_rsa_priv_dec);
    RSA_meth_set_init(rsa_method, my_rsa_init);
    RSA_meth_set_finish(rsa_method, my_rsa_finish);
    RSA_meth_set_sign(rsa_method, my_rsa_sign);
    RSA_meth_set_verify(rsa_method, my_rsa_verify);
    RSA_meth_set_keygen(rsa_method, my_rsa_keygen);
    return 0;
}
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
static int my_digest_ids[] = {NID_sha256};
static int my_engine_digest_selector(ENGINE *e, const EVP_MD **digest,
                                     const int **nids, int nid)
{
    int ok = 1;
    if (!digest)
    {
        *nids = my_digest_ids;
        LOG_Debug(" Digest is empty! Nid:%d", nid);
        return 2;
    }
    LOG_Debug("Digest no %d requested", nid);
    if (nid == NID_sha256)
    {
        *digest = &my_sha256_engine;
    }
    else
    {
        ok = 0;
        *digest = NULL;
    }
    return ok;
}
#else
#endif

//aes的暂时不支持，此为测试代码
#if 0 
#define AESNI_MIN_ALIGN 16
#define AESNI_ALIGN(x) \
    ((void *)(((unsigned long)(x) + AESNI_MIN_ALIGN - 1) & ~(AESNI_MIN_ALIGN - 1)))
static int aesni_cipher_nids[] = {
    NID_aes_128_ecb,
    NID_aes_128_cbc,

    NID_aes_192_ecb,
    NID_aes_192_cbc,

    NID_aes_256_ecb,
    NID_aes_256_cbc,
};
static int aesni_cipher_nids_num =
    (sizeof(aesni_cipher_nids) / sizeof(aesni_cipher_nids[0]));

typedef struct
{
    AES_KEY ks;
    unsigned int _pad1[3];
} AESNI_KEY;

int aesni_set_encrypt_key(const unsigned char *userKey, int bits,
                          AES_KEY *key)
{
    LOG_Debug("aesni_set_encrypt_key");
    memcpy((void *)key, userKey, bits / 8);
    return 1;
}

int aesni_set_decrypt_key(const unsigned char *userKey, int bits,
                          AES_KEY *key)
{
    LOG_Debug("aesni_set_encrypt_key");
    memcpy((void *)key, userKey, bits / 8);
    return 1;
}

void aesni_ecb_encrypt(const unsigned char *in,
                       unsigned char *out,
                       size_t length,
                       const AES_KEY *key,
                       int enc)
{
    LOG_Debug("aesni_ecb_encrypt");
}

void aesni_cbc_encrypt(const unsigned char *in,
                       unsigned char *out,
                       size_t length,
                       const AES_KEY *key,
                       unsigned char *ivec, int enc)
{
    LOG_Debug("aesni_cbc_encrypt");
}

static int aesni_cipher_cbc(EVP_CIPHER_CTX *ctx, unsigned char *out,
                            const unsigned char *in, size_t inl)
{
    // 参考ecb实现
    return 1;
}

static int aesni_cipher_ecb(EVP_CIPHER_CTX *ctx, unsigned char *out,
                            const unsigned char *in, size_t inl)
{
    uint8_t sbuf[256] = {0};
    uint16_t slen = 0, sw = 0;
    // AES_KEY *key = (void*)(ctx->cipher_data);
    LOG_Debug("aesni_cipher_ecb:size:%d,key:", inl);
    for (int i = 0; i < ctx->key_len; i++)
    {
        // LOG_Debug("%x ", ((unsigned char *)key)[i]);
    }
    // LOG_Debug("");
    memcpy(sbuf + 4, in, inl);
    slen = inl + 4;
    uint16_t outlen = 0;
    if (ctx->encrypt)
    {
        // 进行加密操作
        // 实现自己的 AES-256-CBC 加密逻辑
        // 将结果写入 out
        // memcpy(out, in, inl);
        sbuf[3] = *((unsigned char *)ctx->app_data);
        sw = DataEnDecrypt(0x0040, slen, sbuf, out, &outlen);
        if (sw != 0x9000)
        {
            LOG_Debug("DataEnDecrypt sw : %x", sw);
            return 3;
        }
        LOG_Debug("aesni_cbc_encrypt size:%d", inl);
    }
    else
    {
        // 进行解密操作
        // 实现自己的 AES-256-CBC 解密逻辑
        // 将结果写入 out
        // memcpy(out, in, inl);
        sbuf[3] = *((unsigned char *)(ctx->app_data) + 1);
        sw = DataEnDecrypt(0x00c0, slen, sbuf, out, &outlen);
        if (sw != 0x9000)
        {
            LOG_Debug("DataEnDecrypt sw : %x", sw);
            return 3;
        }
        LOG_Debug("aesni_cbc_decrypt size:%d", inl);
    }
    return 1;
}

static int
aesni_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *user_key,
               const unsigned char *iv, int enc)
{
    LOG_Debug("aesni_init_key");
    uint8_t sbuf[256] = {0}, rbuf[8] = {0};
    uint16_t slen = 0, rlen = 0, sw = 0;
    AES_KEY *key = (void *)(ctx->cipher_data);
    memcpy((void *)key, user_key, ctx->key_len);
    sbuf[0] = 0xc1;
    sbuf[1] = ctx->key_len;
    memcpy(sbuf + 2, user_key, ctx->key_len);
    slen = ctx->key_len + 2;
    // AES 大端数据格式
    uint16_t p1p2 = 0x0710;
    if (ctx->key_len == 192 / 8)
    {
        p1p2 = 0x4710;
    }
    else if (ctx->key_len == 256 / 8)
    {
        p1p2 = 0x8710;
    }
    else
    {
    }
    sw = ImportSessionKey(p1p2, slen, sbuf, rbuf, &rlen);
    if (sw != 0x9000)
    {
        LOG_Debug("ImportSessionKey sw : %x", sw);
        return 2;
    }
    if (ctx->app_data == NULL)
    {
        ctx->app_data = malloc(sizeof(unsigned char) * 2);
    }

    if (enc == 0)
    {
        // ret = aesni_set_decrypt_key(user_key, ctx->key_len * 8, key);

        // 保存解密的对称会话密钥的索引号
        *((unsigned char *)(ctx->app_data) + 1) = rbuf[0];
    }
    else
    {
        // 加密
        //  ret = aesni_set_encrypt_key(user_key, ctx->key_len * 8, key);
        // 保存加密的对称会话密钥的索引号
        *((unsigned char *)ctx->app_data) = rbuf[0];
    }
    LOG_Debug("aesni_init_key:key:");
    for (int i = 0; i < ctx->key_len; i++)
    {
        // LOG_Debug("%x ", ((unsigned char *)key)[i]);
    }
    // LOG_Debug("");

    // if (ret < 0)
    // {
    //     EVPerr(EVP_F_AESNI_INIT_KEY, EVP_R_AES_KEY_SETUP_FAILED);
    //     return 0;
    // }

    // if (ctx->cipher->flags & EVP_CIPH_CUSTOM_IV)
    // {
    //     if (iv != NULL)
    //         memcpy(ctx->iv, iv, ctx->cipher->iv_len);
    //     else
    //     {
    //         EVPerr(EVP_F_AESNI_INIT_KEY, EVP_R_AES_IV_SETUP_FAILED);
    //         return 0;
    //     }
    // }

    return 1;
}

#define AES_BLOCK_SIZE 16

#define EVP_CIPHER_block_size_ECB AES_BLOCK_SIZE
#define EVP_CIPHER_block_size_CBC AES_BLOCK_SIZE
/* Declaring so many ciphers by hand would be a pain.
   Instead introduce a bit of preprocessor magic :-) */
#define DECLARE_AES_EVP(ksize, lmode, umode)            \
    static const EVP_CIPHER aesni_##ksize##_##lmode = { \
        NID_aes_##ksize##_##lmode,                      \
        EVP_CIPHER_block_size_##umode,                  \
        ksize / 8,                                      \
        AES_BLOCK_SIZE,                                 \
        0 | EVP_CIPH_##umode##_MODE,                    \
        aesni_init_key,                                 \
        aesni_cipher_##lmode,                           \
        NULL,                                           \
        sizeof(AESNI_KEY),                              \
        EVP_CIPHER_set_asn1_iv,                         \
        EVP_CIPHER_get_asn1_iv,                         \
        NULL,                                           \
        NULL}

DECLARE_AES_EVP(128, ecb, ECB);
DECLARE_AES_EVP(128, cbc, CBC);

DECLARE_AES_EVP(192, ecb, ECB);
DECLARE_AES_EVP(192, cbc, CBC);

DECLARE_AES_EVP(256, ecb, ECB);
DECLARE_AES_EVP(256, cbc, CBC);

static int
aesni_ciphers(ENGINE *e, const EVP_CIPHER **cipher,
              const int **nids, int nid)
{
    /* No specific cipher => return a list of supported nids ... */
    if (!cipher)
    {
        *nids = aesni_cipher_nids;
        return aesni_cipher_nids_num;
    }

    /* ... or the requested "cipher" otherwise */
    switch (nid)
    {
    case NID_aes_128_ecb:
        *cipher = &aesni_128_ecb;
        break;
    case NID_aes_128_cbc:
        *cipher = &aesni_128_cbc;
        break;
    case NID_aes_192_ecb:
        *cipher = &aesni_192_ecb;
        break;
    case NID_aes_192_cbc:
        *cipher = &aesni_192_cbc;
        break;
    case NID_aes_256_ecb:
        *cipher = &aesni_256_ecb;
        break;
    case NID_aes_256_cbc:
        *cipher = &aesni_256_cbc;
        break;
    default:
        /* Sorry, we don't support this NID */
        *cipher = NULL;
        return 0;
    }
    return 1;
}

/*绑定函数*/
static int bind_pp(ENGINE *e)
{
    if (!ENGINE_set_id(e, eng_id) ||
        !ENGINE_set_name(e, eng_name) ||
        !ENGINE_set_RAND(e, &rand_method) ||
        !ENGINE_set_destroy_function(e, myengine_destroy) ||
        !ENGINE_set_init_function(e, myengine_init) ||
        !ENGINE_set_finish_function(e, myengine_finish) ||
        !ENGINE_set_RSA(e, &rsa_method) ||
        !ENGINE_set_digests(e, &my_engine_digest_selector) || // 设置摘要选择器函数
        !ENGINE_set_ciphers(e, aesni_ciphers))
        return 0;
    // RAND_set_rand_method(&rand_method);
    return 1;
}
#endif
/*绑定函数*/
static int bind_pp(ENGINE *e)
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
#else
    RSA_meth_init();
#endif
    if (!ENGINE_set_id(e, eng_id) ||
        !ENGINE_set_name(e, eng_name) ||
        !ENGINE_set_RAND(e, &rand_method) ||
        !ENGINE_set_destroy_function(e, myengine_destroy) ||
        !ENGINE_set_init_function(e, myengine_init) ||
        !ENGINE_set_finish_function(e, myengine_finish) ||
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        !ENGINE_set_RSA(e, &rsa_method) ||
        !ENGINE_set_digests(e, &my_engine_digest_selector))
#else
        !ENGINE_set_RSA(e, rsa_method))
#endif
        return 0;
    // RAND_set_rand_method(&rand_method);
    return 1;
}

static ENGINE *engine_sirun_new(void)
{
    ENGINE *e = ENGINE_new();
    if (!e)
        return NULL;
    if (!bind_pp(e))
    {
        ENGINE_free(e);
        return NULL;
    }
    return e;
}
void ENGINE_Init_sirun()
{
    // 初始化 OpenSSL 引擎系统
    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();
    ENGINE *e = engine_sirun_new();
    if (!e)
        return;
    ENGINE_add(e);

    // ERR_add_error(e);
    display_engine_list();
    // ENGINE *e = NULL;
    // e = ENGINE_by_id(eng_id);
    // if(!e)
    // {
    //     LOG_Debug("err load engine failed");
    //     return 0;
    // }

    ENGINE_init(e);         // 会调用 myengine_init 函数
    ENGINE_register_RSA(e); // 会调用my_rsa_init函数
    // ENGINE_set_default_ciphers(e);//使能后调用的就是用户实现的ciphers (aes)算法，不加这一句的话调用的就是openssl的默认实现
    // fmse rand长度仅支持4 8 16 所以不用它的
    // ENGINE_set_default_RAND(e);
    // 使用自己的sha256 engine会使产生的csr文件解析出错。。
    // ENGINE_set_default_digests(e);
}
void ENGINE_DeInit_sirun()
{
    ENGINE *e = NULL;
    e = ENGINE_by_id(eng_id);
    if (!e)
    {
        LOG_Debug("err load engine failed");
        return;
    }
    // 清理引擎资源
    ENGINE_finish(e); // myengine_finish
    ENGINE_free(e);
}

int GenX509ReqFile(int version, const char *subjectCN, const char *subjectOU, const char *subjectO, const char *subjectL, const char *subjectST, const char *subjectC,
                   const char *subhectEMail, RSA *rsa, const char *fileName)
{
    X509_REQ *x509_req = NULL;
    X509_NAME *x509_name = NULL;
    int nVersion = version;
    x509_req = X509_REQ_new();
    x509_name = X509_REQ_get_subject_name(x509_req);
    X509_REQ_set_version(x509_req, nVersion);
    X509_NAME_add_entry_by_txt(x509_name, "CN", MBSTRING_ASC, (const unsigned char *)subjectCN, -1, -1, 0); // 证书所请求的域名 主题通用名称
    X509_NAME_add_entry_by_txt(x509_name, "OU", MBSTRING_ASC, (const unsigned char *)subjectOU, -1, -1, 0); // 组织单位
    X509_NAME_add_entry_by_txt(x509_name, "O", MBSTRING_ASC, (const unsigned char *)subjectO, -1, -1, 0);   // 组织
    X509_NAME_add_entry_by_txt(x509_name, "L", MBSTRING_ASC, (const unsigned char *)subjectL, -1, -1, 0);   // 城市
    X509_NAME_add_entry_by_txt(x509_name, "ST", MBSTRING_ASC, (const unsigned char *)subjectST, -1, -1, 0); // 省
    X509_NAME_add_entry_by_txt(x509_name, "C", MBSTRING_ASC, (const unsigned char *)subjectC, -1, -1, 0);   // 国家
    X509_NAME_add_entry_by_txt(x509_name, "emailAddress", MBSTRING_ASC, (const unsigned char *)subhectEMail, -1, -1, 0);
    EVP_PKEY *pKey = NULL;
    pKey = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(pKey, rsa);
    int ret = X509_REQ_set_pubkey(x509_req, pKey);
    if (ret != 1)
    {
        LOG_Debug("MakeLocalCsrSSLApi X509_REQ_set_pubkey err");
    }

    // my_rsa_sign 签名，会将原始数据做sha256运算后，将32byte sha256 传给my_rsa_sign进行签名
    if (0 >= X509_REQ_sign(x509_req, pKey, EVP_sha256()))
    {
        LOG_Debug("failed to sign the certificate");
        X509_REQ_free(x509_req);
        x509_req = NULL;
    }

    FILE *pFile = NULL;
    pFile = fopen(fileName, "w");
    if (pFile == NULL)
    {
        LOG_Debug("fopen failed");
    }
    PEM_write_X509_REQ(pFile, x509_req);
    fclose(pFile);
    return 1;
}

int SaveRSAPubKeyFile(RSA *rsa, char *fileName)
{
    FILE *file = fopen(fileName, "w");
    if (!file)
    {
        LOG_Debug("Error opening file!");
        return -1;
    }
    if (!PEM_write_RSA_PUBKEY(file, rsa))
    {
        LOG_Debug("Error writing RSA public key!");
        return -1;
    }
    fclose(file);
    return 1;
}

int LoadRSAPubKey(RSA **rsa, char *fileName)
{
    FILE *fp = fopen(fileName, "rb");
    if (!fp)
    {
        LOG_Debug("Error: Failed to open public key file.");
        return -1;
    }

    *rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
    fclose(fp);

    if (!*rsa)
    {
        LOG_Debug("Error: Failed to read public key from PEM file.");
        return -1;
    }
    return 1;
}

void display_engine_list()
{
    ENGINE *h;
    int loop;

    h = ENGINE_get_first();
    loop = 0;
    LOG_Debug("start:");
    while (h)
    {
        LOG_Debug("engine %i, id = \"%s\", name = \"%s\"",
                  loop++, ENGINE_get_id(h), ENGINE_get_name(h));
        h = ENGINE_get_next(h);
    }
    LOG_Debug("end of list");
    ENGINE_free(h);
}

void opensslHandleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}
