#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

// 必须在所有 OpenSSL 头文件之前定义，以抑制 3.0 弃用警告
#define OPENSSL_API_COMPAT 10101

#include <openssl/core_names.h>
#include <openssl/engine.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/ecdh.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/x509v3.h>
#include <openssl/x509.h>
#include <openssl/bn.h>

#include "sr_log.h"
#include "rpc_hsm_api.h"

#define BITS_TO_KEYALG(bits)                   \
    ((bits) == 1024 ? E_ALG_RSA1024 :          \
     (bits) == 2048 ? E_ALG_RSA2048 :          \
     E_ALG_NONE)

#define PADDING_TO_ENGINE(padding) \
    ((padding) == RSA_PKCS1_PADDING ? E_RSA_SIGN_PADDING_PKCS1 : \
     (padding) == RSA_PKCS1_OAEP_PADDING ? E_RSA_SIGN_PADDING_OAEP : \
     (padding) == RSA_PKCS1_PSS_PADDING ? E_RSA_SIGN_PADDING_PSS : \
     (padding) == RSA_NO_PADDING ? E_RSA_SIGN_PADDING_NONE : \
     E_RSA_SIGN_PADDING_NONE)

#define DIGIST_TO_ENGINE(digest) \
    ((digest) == NID_sha256 ? E_DIGIST_SHA256 : \
     (digest) == NID_sha384 ? E_DIGIST_SHA384 : \
     (digest) == NID_sha512 ? E_DIGIST_SHA512 : \
     (digest) == NID_sha1 ?   E_DIGIST_SHA1 : \
     (digest) == NID_sha224 ? E_DIGIST_SHA224 : \
     (digest) == NID_sm3 ?    E_DIGIST_SM3 : \
     E_DIGIST_NONE)

#define SSL_SIRUN_ENG_ID      "engine-sirun"
#define SSL_SIRUN_ENG_NAME    "engine for OpenSSL(sirun)"
#define SSL_SIRUN_ENG_METHOD  "engine-sirun-method"

static int my_rand_status(void);
static int my_rand_bytes(uint8_t *buf, int num);

static const RAND_METHOD g_rand_method =
{
    NULL,
    my_rand_bytes,
    NULL,
    NULL,
    NULL,
    my_rand_status
};

static uint16_t g_hsmKeyId = KEYID_DIGITALKEY_PKI_TEMP;

/* ++++++++++++++++++++++++++++++ RSA ++++++++++++++++++++++++++++++ */

static int my_rsa_init(RSA *rsa)
{
    LOG_Warn("[%s]", __func__);
    return 1;
}

static int my_rsa_finish(RSA *rsa)
{
    LOG_Warn("[%s]", __func__);
    return 1;
}

static int my_rsa_pub_enc(int flen, const uint8_t *from, uint8_t *to, RSA *rsa, int padding)
{
    int ret = -1;
    RsaKeyAlg_e keyAlg = E_ALG_NONE;
    RsaSignPaddingAlg_e paddingAlg = E_RSA_SIGN_PADDING_NONE;
    uint32_t outLen = 0;

    LOG_Warn("[%s] enter keyId:%0x", __func__, g_hsmKeyId);

    do{
        if (flen <= 0 || !from || !to || !rsa) {
            break;
        }

        keyAlg = BITS_TO_KEYALG(RSA_size(rsa) * 8);
        if (keyAlg == E_ALG_NONE) {
            LOG_Warn("[%s] unsupport Alg", __func__);
            ret = -1;
            break;
        }    

        paddingAlg = PADDING_TO_ENGINE(padding);

        ret = RpcHsmEncrypt(keyAlg, g_hsmKeyId, paddingAlg, from, flen, to, &outLen);
        if (ret != 0) {
            LOG_Warn("[%s] RpcHsmEncrypt failed", __func__);
            ret = -1;
            break;
        }

        ret = outLen;
    }while(0);

    LOG_Warn("[%s] exit", __func__);

    return ret;
}

static int my_rsa_priv_dec(int flen, const uint8_t *from, uint8_t *to, RSA *rsa, int padding)
{
    int ret = -1;
    uint32_t decLen = 0;
    RsaKeyAlg_e keyAlg = E_ALG_NONE;
    RsaSignPaddingAlg_e paddingAlg = E_RSA_SIGN_PADDING_NONE;

    LOG_Warn("[%s] enter keyId:%0x", __func__, g_hsmKeyId);

    do{
        if (flen <= 0 || !from || !to || !rsa) {
            break;
        }        

        keyAlg = BITS_TO_KEYALG(RSA_size(rsa) * 8);
        if (keyAlg == E_ALG_NONE) {
            LOG_Warn("[%s] unsupport Alg", __func__);
            ret = -1;
            break;
        }    

        paddingAlg = PADDING_TO_ENGINE(padding);

        ret = RpcHsmDecrypt(keyAlg, g_hsmKeyId, paddingAlg, from, flen, to, &decLen);
        if (ret != 0) {
            LOG_Warn("[%s] RpcHsmDecrypt failed", __func__);
            ret = -1;
            break;
        }

        ret = decLen;

    }while(0);

    LOG_Warn("[%s] exit", __func__);

    return ret;
}

static int my_rsa_sign(int dtype, const uint8_t *plain, uint32_t plainLen, uint8_t *signature, uint32_t *signatureLen, const RSA *rsa)
{
    int ret = -1;
    DigistAlg_e digist = E_DIGIST_NONE;
    RsaKeyAlg_e keyAlg = E_ALG_NONE;

    LOG_Warn("[%s] enter keyId:%0x", __func__, g_hsmKeyId);

    do {
        if (plain == NULL || signature == NULL || signatureLen == NULL || rsa == NULL) {
            LOG_Warn("[%s]  invalid parameters", __func__);
            ret = -1;
            break;
        }
        
        LOG_Warn("[%s] plainLen:%d, signatureLen:%d, dtype:%0x", __func__, plainLen, *signatureLen, dtype);

        keyAlg = BITS_TO_KEYALG(RSA_size(rsa) * 8);
        if (keyAlg == E_ALG_NONE) {
            LOG_Warn("[%s] unsupport Alg", __func__);
            ret = -1;
            break;
        }      
        
        digist = DIGIST_TO_ENGINE(dtype);
        if (digist == E_DIGIST_NONE) {
            LOG_Warn("[%s] unsupported hash type", __func__);
            ret = -1;
            break;
        }

        ret = RpcHsmSign(keyAlg, g_hsmKeyId, digist, plain, plainLen, signature, signatureLen);
        if (ret == 0) {
            LOG_Warn("[%s] RpcHsmSign success, signatureLen=%u", __func__, *signatureLen);
        } else {
            LOG_Warn("[%s] RpcHsmSign failed, ret=%d", __func__, ret);
        }

    }while(0);

    LOG_Warn("[%s] exit", __func__);

    /** 成功返回 1，失败返回 0 **/
    return (ret==0)? 1: 0;
}

static int my_rsa_verify(int dtype, const uint8_t *plain, unsigned int plainLen, const uint8_t* signature, unsigned int signatureLen,const RSA *rsa)
{
    int ret = -1;
    RsaKeyAlg_e keyAlg = E_ALG_NONE;
    DigistAlg_e digist = E_DIGIST_NONE;

    LOG_Warn("[%s] enter keyId:%0x", __func__, g_hsmKeyId);

    do{
        if (!plain || !signature || !rsa || plainLen == 0 || signatureLen == 0) {
            LOG_Warn("[%s] invalid parameters", __func__);
            ret = -1;
            break;
        }
    
        LOG_Warn("[%s] plainLen=%u, signatureLen=%u, dtype=%0x", __func__, plainLen, signatureLen, dtype);

        keyAlg = BITS_TO_KEYALG(RSA_size(rsa) * 8);
        if (keyAlg == E_ALG_NONE) {
            LOG_Warn("[%s] unsupport Alg", __func__);
            ret = -1;
            break;
        }      

        digist = DIGIST_TO_ENGINE(dtype);
        if (digist == E_DIGIST_NONE) {
            LOG_Warn("[%s] unsupported hash type", __func__);
            ret = -1;
            break;
        }

        ret = RpcHsmVerify(keyAlg, g_hsmKeyId, digist, (const uint8_t*)plain, plainLen, (uint8_t*)signature, signatureLen);
        if (ret == 0) {
            LOG_Warn("[%s] RpcHsmVerify success", __func__);
        } else {
            LOG_Warn("[%s] RpcHsmVerify failed, ret=%d", __func__, ret);
        }

    }while(0);

    /** 成功返回 1，失败返回 0 **/
    return (ret==0)? 1: 0;
}

static int my_rsa_keygen(RSA *rsa, int bits, BIGNUM *e, BN_GENCB *cb)
{
    int ret = 0;

    BIGNUM *n_bn =NULL;
    BIGNUM *e_hw = NULL;

    uint8_t* pubKeyBigNum = NULL;
    uint8_t* pubKeyDer = NULL;
    uint32_t pubKeyDerBufSize = 0;

    RsaKeyAlg_e keyAlg = E_ALG_NONE;

    LOG_Warn("[%s] enter", __func__);

    do{
        if (!rsa || !e || !cb) {
            LOG_Warn("[%s] invalid parameters", __func__);
            ret = -1;
            break;
        }

        keyAlg = BITS_TO_KEYALG(bits);
        if (keyAlg == E_ALG_NONE) {
            LOG_Warn("[%s] unsupport Alg", __func__);
            ret = -1;
            break;
        }  
        
        pubKeyDerBufSize = bits/8 + 4;
        pubKeyDer = malloc(pubKeyDerBufSize);
        pubKeyBigNum = malloc(bits/8 + 4);
        if (!pubKeyDer || !pubKeyBigNum) {
            LOG_Warn("[%s] pubKeyDer or pubKeyBigNum malloc failed", __func__);
            ret = -3;
            break;
        }
    
        ret = RpcHsmGenKey(keyAlg, g_hsmKeyId, pubKeyDer, &pubKeyDerBufSize);
        LOG_Warn("[%s] RpcHsmGenKey ret = %d, pubKeyDerBufSize = %d", __func__, ret, pubKeyDerBufSize);
        if (ret != 0 || pubKeyDerBufSize == 0) {
            LOG_Warn("[%s] RpcHsmGenKey failed!!!", __func__);
            ret = -4;
            break;
        }

        LOG_DebugHex(pubKeyDer, pubKeyDerBufSize, "pubKeyDer");
    
        pubKeyBigNum[0] = 0x00;
        memcpy(pubKeyBigNum + 1, &pubKeyDer[4], bits/8);
        n_bn = BN_bin2bn(pubKeyBigNum, bits/8 +1, NULL);
        e_hw = BN_bin2bn(pubKeyDer, 4, NULL);
        if (n_bn == NULL || e_hw == NULL) {
            LOG_Warn("[%s] BN_bin2bn failed", __func__);
            ret = -5;
            break;
        }

        if (RSA_set0_key(rsa, n_bn, e_hw, NULL) != 1) {
            /* 填充RSA失败， 释放 n_bn,e_hw */
            LOG_Warn("[%s] RSA_set0_key failed", __func__);
            BN_free(n_bn);
            BN_free(e_hw);
            ret = -6; 
            break;
        }

    }while(0);

    if (pubKeyDer != NULL)      { free(pubKeyDer);      pubKeyDer = NULL;}
    if (pubKeyBigNum != NULL)   { free(pubKeyBigNum);   pubKeyBigNum = NULL;}

    /** 成功返回 1，失败返回 0 **/
    return (ret==0)? 1: 0;
}

/* ------------------------------ RSA ------------------------------ */

/* ++++++++++++++++++++++++++++++ random ++++++++++++++++++++++++++++++ */

static int my_rand_status(void)
{
    LOG_Warn("[%s]", __func__);
  
    return 1;
}

static int my_rand_bytes(uint8_t *buf, int num)
{
    int ret = 0;

    LOG_Warn("[%s] enter(%d)", __func__, num);

    ret = RpcHsmRandom((uint32_t)num, buf);

    LOG_Warn("[%s] RpcHsmRandom ret =%d", __func__, ret);

    return  ret == 0 ? 1 : 0;
}

/* ------------------------------ random ------------------------------ */


static int my_engine_init(ENGINE *e)
{
    LOG_Warn("[%s] enter", __func__);
    return 1;
}

static int my_engine_finish(ENGINE *e)
{
    LOG_Warn("[%s] enter", __func__);
    return 1;
}

static int my_engine_destroy(ENGINE *e)
{
    RSA_METHOD *method = NULL;
    LOG_Warn("[%s] enter", __func__);

    method = (RSA_METHOD*) ENGINE_get_RSA(e);
    if (method) {
        RSA_meth_free(method);
        ENGINE_set_RSA(e, NULL);
        method = NULL;
    }

    return 1;
}

/****************************************************************************************************/

#define RSA_TMP_2048_PEM \
    "-----BEGIN RSA PRIVATE KEY-----\n" \
    "MIIEpAIBAAKCAQEA055EcJKVpwVfIGkOOHAwsXDZK6TtKdkb3l/tRBwOn9CAvlny\n" \
    "mdDlJdaXv0M0axIveuwT7YnaK96RjUUr/uYv+SdLEJTlHpy1A/QR1fsN6PPLXAbm\n" \
    "Kmf19LQpmwZ5lHfevJ8Nes/z+4SN5/idWAFtAJbjw857TcSPgOl5jslssIaW0PWF\n" \
    "Qk4m7HKUsdYiybSJtA6xHPNouakk8Ib0OtDQn5cOnCgPMCUgyJV1xOFgWjgneTAC\n" \
    "yoo3ZG9muvEowN4A2JK1Xkmc8aWHZ0RBwIDv0lWTKsQ76G5skxbwHk+x46UgnZVi\n" \
    "fpKz4TFxyMvH3wChn07AGL3st6aKGqZ8GWh0JwIDAQABAoIBADfkz7LkJL7uvkdz\n" \
    "OBxUFahoZBQRBctN2vyLXcHlnz869SZR37Ac6slpur774zZ8oSnYfRL6nDOGH++u\n" \
    "W4aCCh3G8R66FSJWxcD7kFJycyQacJYoBFcjCmXNyEd4GFjMqHotkIPltU2znz+r\n" \
    "/PMnvUCwmgw3132c80JbaI38Scb96l1cIodPNMgaPdHaSIluRFGk4HvyAtddQmAV\n" \
    "gB/NZh6vPrTnid3KQkNdWgj8rZ1Lz0TC4LQAtOjHm1iFUatc168Gw/X3w70I/nYl\n" \
    "h97yWzhkyyQKDerC87gse/7zmUKCI7/dJichxHfJMIy/HlbrU99G6esTD5m0yX4d\n" \
    "1BKHKLECgYEA91gxEjS5B/VWOQ6Ye8d5SAGvkEp2BmEuGiQbh8GuenyifIf/7VWm\n" \
    "2EQ3pfnzalVx200ohJU3hh77celYJcO0GSP/DnnI+uIxRzF17XauQRhiVJ+iFvPA\n" \
    "WFmLn9Gzd3futfcnUp1EY9IVfrKfoD4L45fovsUVvFtP41NQL2RbtZsCgYEA2wYG\n" \
    "niCTX97IRJ/WekLRjC0akFmJcMJttMtqcc69n6MoMZeZWLOYk9vGpV9XwO0Ps9Xd\n" \
    "lqgQ3gfklqtQRp/WLt9mQ6WIzQ4r4VX1Kk9KpkkUSkYUetbLB24pMzPb911+ihd+\n" \
    "kQPECkpcCmzAxQOiU1TDfASFRsycpUG99/1hSmUCgYEA8UxhOvCiXigywKEDkyXL\n" \
    "pLq4SpzJvXmK30fAb9U8YRQ79B89Ro0boroSS+KnXGIVHCuOzqE/tKF/Debl7pmY\n" \
    "ljoutjGLJ24JQvn2oP36C1k9fG7rU1bim45oDzclzPP8thpyiz8PnA40mBCsd0hw\n" \
    "ksz86zcb+FBghFYf0MwoR+cCgYEAgaVCe41azw/nSb+3TC2LLkdwhuuJEIb2O2OB\n" \
    "QZaX5gvE+f37lmbQtQ0Nu8wA/MEWX2QDw27+dP8FDy6e2P854Ar61KYCoSZ3UO35\n" \
    "ri67SEzPxttOuhu0mFRb7OQQm47rHUCk+Z/aBiFX9A/F15i8GEl/YUSnw23Qd32/\n" \
    "vjBsy4UCgYBd0Ob9i4taVcbI0pdHK4cpbHRHBo6ZZifP1hNpBsON789JqPKtp+Ch\n" \
    "7gjg9jadx8myjbmOVZdj0tDGUYseUJ7oqZePdVWm5EfxVDC5DMnMlGcgYmyr2e6u\n" \
    "vNrYBvdH/mhENjr5d6V0Re0HUpLWyXbWfhk+2nNfTOfD755Cxd15Qw==\n" \
    "-----END RSA PRIVATE KEY-----\n" 


#define RSA_TMP_1024_PEM \
    "-----BEGIN RSA PRIVATE KEY-----\n" \
    "MIICXQIBAAKBgQDoMWTrMal5KXvD8skFbLeObi8to6kOg0m/etAjQ7MdizF9veZs\n" \
    "PYH+w59sYkQ3QSVcwjwISxHoPQiYMmEm5AzEZhpM/QqMDhwPYZhm2e0Q0JoxRfEM\n" \
    "zmtpf5DSE82xBi3yPzAaczHkbBpp4lFDH+FU9yzGp3JBuq4tp9MbhaT4iwIDAQAB\n" \
    "AoGBAIpIuJjWN9+AFETL+Gqxp9Q8zP46ZTnOeQJ/6yvFawxiVLRVf8DhFOWcrfrH\n" \
    "v2bE9rJ6v3QU+XI9vzudbXzS/sJqnLC9Vf+uZrvfMu1QauZpzOfx468kePnqxtCS\n" \
    "jxg7piwCc2kTZUxdtZ8BGmWHRO1pKCwDQIQi0oL26/Rsi2qJAkEA+4qMBIlXsIpS\n" \
    "W6tljX/jpuD5DEvP1hxGcxmbeavQ9MuIiVK/14jGAI4olPeQ0dQ7EDzGtmedcntK\n" \
    "JfJsGInDRQJBAOxPDEyk02VxnHO/yOPMM+T1QTtcGYtb0AwtyMO0alMMo1UtZSme\n" \
    "ZzXqebQvP/I//mMS4qjvrPNeUQZayjJBIY8CQHYQxMHIWVFyy09dtq/g7FDC3gLI\n" \
    "fsgnw2V5CoHkhJKyCecy4PrgptdR4nhxbxcd/48Sh87L+ZnnhpkjHVuzJeUCQQC3\n" \
    "pLrnOVS7ym4OmiXRulk9/sdCDVIgtipZvJ8AUE4s2m54fH3YuSx2fxEz0WB9r/PP\n" \
    "JNcW+KRMwahwKtBl+63LAkB22FphXe8/siskSDVhe06k0uw4VUlMXa+Um2Gnkmsh\n" \
    "W1A9FS8RV/Gwf45XP2M8GHc1ZRNKvR5m3Sjer2ZUq63/\n" \
    "-----END RSA PRIVATE KEY-----\n"

int OpensslRsaGenerateTmpKey(EVP_PKEY **pkey,int bits) {

    const char* pem_data = NULL;
    BIO *bio = NULL;

    if (bits != 1024 && bits != 2048){
        return -1;
    }

    pem_data = (bits == 1024) ? RSA_TMP_1024_PEM : RSA_TMP_2048_PEM;

    bio = BIO_new_mem_buf(pem_data, -1);
    if (!bio)
        return -1;

    *pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);

    BIO_free(bio);

    return 0;
}

int OpensslEngineInit(ENGINE** engine)
{
    int ret = -1;
    RSA_METHOD *method = NULL;

    LOG_Warn("[%s] %p", __func__, engine);

    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();

    do{
        *engine = ENGINE_new();
        if (!*engine){
            break;
        }

        method = RSA_meth_new(SSL_SIRUN_ENG_METHOD, 0);
        if (!method){
            break;
        }

        RSA_meth_set_init(method, my_rsa_init);
        RSA_meth_set_finish(method, my_rsa_finish);
        RSA_meth_set_keygen(method, my_rsa_keygen);
        RSA_meth_set_sign(method, my_rsa_sign);
        RSA_meth_set_verify(method, my_rsa_verify);
        RSA_meth_set_pub_enc(method, my_rsa_pub_enc);
        RSA_meth_set_priv_dec(method, my_rsa_priv_dec);   
    
        if (!ENGINE_set_id(*engine, SSL_SIRUN_ENG_ID) ||
            !ENGINE_set_name(*engine, SSL_SIRUN_ENG_NAME) ||
            !ENGINE_set_RAND(*engine, &g_rand_method) ||
            !ENGINE_set_destroy_function(*engine, my_engine_destroy) ||
            !ENGINE_set_init_function(*engine, my_engine_init) ||
            !ENGINE_set_finish_function(*engine, my_engine_finish) ||
            !ENGINE_set_RSA(*engine, method))
        {
            break;
        }
        
        ENGINE_init(*engine);
        ENGINE_register_RSA(*engine); 
        ENGINE_register_RAND(*engine);

        ret = 0;
    }while(0);

    if (ret != 0){
        if (method!= NULL)    { RSA_meth_free(method);  method = NULL; }
        if (*engine != NULL)  { ENGINE_free(*engine);   *engine = NULL;    }
    }
    
    return ret;
}

void OpensslEngineDeinit(ENGINE** engine)
{
    LOG_Warn("[%s] %p", __func__, engine);
    if (!engine || !*engine)
    {
        LOG_Warn("err load engine failed");
        return;
    }

    ENGINE_unregister_RSA(*engine);
    ENGINE_unregister_RAND(*engine);

    ENGINE_finish(*engine);
    ENGINE_free(*engine);
    
    *engine = NULL;
}

void OpensslEngineSwitchKeyId(uint16_t keyId)
{
    LOG_Warn("OpensslEnginSwitchKeyId:%x", keyId);
    g_hsmKeyId = keyId;
}

