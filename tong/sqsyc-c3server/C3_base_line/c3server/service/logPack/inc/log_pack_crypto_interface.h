#ifndef __LOG_PACK_CRYPTO_INTERFACE__
#define __LOG_PACK_CRYPTO_INTERFACE__

#include <stdint.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/hmac.h>

typedef enum {
    CRYPTO_ALGO_INVALID = 0,
    CRYPTO_ALGO_DES3,
    CRYPTO_ALGO_AES128,
} CryptoAlgo_e;

typedef struct {
    CryptoAlgo_e algo;
    union {
        EVP_CIPHER_CTX *des3_ctx;
        EVP_CIPHER_CTX *aes_ctx;
        /* 未来可扩展：EVP_CIPHER_CTX *aes256_ctx; */
    } u;
} Crypto_Dev_t;

typedef struct {
    const char* name;
    int(*crypto_init)(void);
    int(*crypto_start)(Crypto_Dev_t* dev, const char* password, uint8_t* prefixData, int32_t* prefixDataLen);
    int(*crypto_encrypt)(Crypto_Dev_t* dev, uint8_t* plainData, int32_t plainDataLen, uint8_t* cryptoData, int32_t* cryptoDataLen);
    int(*crypto_finish)(Crypto_Dev_t* dev, uint8_t* tailData, int32_t* tailDataLen);
    int(*crypto_free)(Crypto_Dev_t* dev);
    int(*crypto_deinit)(void);
} Crypto_interface_t;

extern const Crypto_interface_t g_des3_crypto_interface;
extern const Crypto_interface_t g_aes128_crypto_interface;

/**
 * 可用加密接口注册表，NULL 结尾。
 * 新增算法时直接在此数组中添加对应接口指针即可。
 */
extern Crypto_interface_t* g_cryptoInterfaces[];

#endif
