#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/random.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>

#define OPENSSL_API_COMPAT 10101

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#include "hsm_api.h"
#include "sr_log.h"

#define SIMULATE_HSM_KEY_DIR    "/oemdata/simulate_hsm"
#define SIMULATE_KEY_SLOT_COUNT 256
#define SIMULATE_KEY_INFO_SIZE  2

static int simulateEnsureKeyDirExists(void)
{
    int ret = 0;
    struct stat st;

    LOG_Warn("[%s]enter", __func__);

    do {
        if (stat(SIMULATE_HSM_KEY_DIR, &st) != 0) {
            if (mkdir(SIMULATE_HSM_KEY_DIR, 0755) != 0) {
                LOG_Error("[%s] Failed to create key directory: %s", __func__, SIMULATE_HSM_KEY_DIR);
                ret = -1;
                break;
            }
        } else if (!S_ISDIR(st.st_mode)) {
            LOG_Error("[%s] Path exists but is not a directory: %s", __func__, SIMULATE_HSM_KEY_DIR);
            ret = -1;
            break;
        }
    } while (0);

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static void simulateGetKeyPath(uint16_t keyId, char *path, size_t pathLen, int isPublic)
{
    // LOG_Warn("[%s]enter", __func__);
    snprintf(path, pathLen, "%s/key_%04x_%s.pem",
             SIMULATE_HSM_KEY_DIR, keyId, isPublic ? "pub" : "priv");
    // LOG_Warn("[%s]exit", __func__);
}

static int simulateLoadKey(uint16_t keyId, EVP_PKEY **pkey, int isPublic)
{
    int ret = 0;
    char path[256] = {0};
    FILE *fp = NULL;

    // LOG_Warn("[%s]enter", __func__);

    do {
        simulateGetKeyPath(keyId, path, sizeof(path), isPublic);
        fp = fopen(path, "r");
        if (!fp) {
            ret = -1;
            break;
        }

        if (isPublic) {
            *pkey = PEM_read_PUBKEY(fp, NULL, NULL, NULL);
        } else {
            *pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
        }
        fclose(fp);

        if (*pkey == NULL) {
            ret = -1;
            break;
        }
    } while (0);

    // LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int simulateSaveKey(uint16_t keyId, EVP_PKEY *pkey, int isPublic)
{
    int ret = -1;
    char path[256] = {0};
    FILE *fp = NULL;

    // LOG_Warn("[%s]enter", __func__);

    do {
        simulateGetKeyPath(keyId, path, sizeof(path), isPublic);
        fp = fopen(path, "w");
        if (!fp) {
            LOG_Error("[%s] Failed to open file for writing: %s", __func__, path);
            break;
        }

        if (isPublic) {
            if (PEM_write_PUBKEY(fp, pkey)) {
                ret = 0;
            }
        } else {
            if (PEM_write_PrivateKey(fp, pkey, NULL, NULL, 0, NULL, NULL)) {
                ret = 0;
            }
        }
        fclose(fp);
        if (ret != 0) {
            LOG_Error("[%s] Failed to write key to file: %s", __func__, path);
        }
    } while (0);

    // LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateInit(void* ctx)
{
    int ret = 0;

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
        ret = simulateEnsureKeyDirExists();
        if (ret != 0) {
            break;
        }
        LOG_Info("[%s] Simulate HSM initialized", __func__);
    } while (0);

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateRelease(void* ctx)
{
    int ret = 0;

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
    } while (0);

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateRandom(void* ctx, uint32_t expectedLen, uint8_t *random)
{
    int ret = 0;
    ssize_t rc = 0;
    int fd = -1;
    size_t total = 0;
    ssize_t n = 0;

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
        LOG_Debug("[%s] Request: expectedLen=%u", __func__, expectedLen);

        if (random == NULL) {
            LOG_Error("[%s] Invalid parameters: random is NULL", __func__);
            ret = -1;
            break;
        }

        rc = getrandom(random, expectedLen, 0);
        if (rc < 0 || (size_t)rc != expectedLen) {
            /* fallback to /dev/urandom */
            fd = open("/dev/urandom", O_RDONLY);
            if (fd < 0) {
                LOG_Error("[%s] open /dev/urandom failed", __func__);
                ret = -1;
                break;
            }
            total = 0;
            while (total < expectedLen) {
                n = read(fd, random + total, expectedLen - total);
                if (n < 0) {
                    if (errno == EINTR) {
                        continue;
                    }
                    LOG_Error("[%s] read /dev/urandom failed", __func__);
                    ret = -1;
                    break;
                }
                total += n;
            }
            close(fd);
            if (ret != 0) {
                break;
            }
        }

        LOG_Info("[%s] Generated %u bytes", __func__, expectedLen);
    } while (0);

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateRsaKeyPairGenerate(void* ctx, RsaKeyAlg_e alg, uint16_t keyId, uint8_t* pubKeyDer, uint32_t *pubKeyDerBufSize)
{
    int ret = -1;
    int bits = 0;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx_gen = NULL;
    int derLen = 0;
    uint8_t *p = NULL;

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;

        ret = 0;
        switch (alg) {
            case E_ALG_RSA1024: bits = 1024; break;
            case E_ALG_RSA2048: bits = 2048; break;
            default:
                LOG_Error("[%s] Unsupported algorithm: %d", __func__, alg);
                ret = -1;
                break;
        }
        if (ret != 0) {
            break;
        }

        LOG_Info("[%s] Request: alg=%d, keyId=0x%04X, bits=%d", __func__, alg, keyId, bits);

        if (pubKeyDer == NULL || pubKeyDerBufSize == NULL) {
            LOG_Error("[%s] Invalid output parameters", __func__);
            ret = -1;
            break;
        }

        ctx_gen = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
        if (!ctx_gen) {
            LOG_Error("[%s] EVP_PKEY_CTX_new_id failed: %s", __func__,
                      ERR_error_string(ERR_get_error(), NULL));
            ret = -1;
            break;
        }

        if (EVP_PKEY_keygen_init(ctx_gen) <= 0) {
            LOG_Error("[%s] EVP_PKEY_keygen_init failed: %s", __func__,
                      ERR_error_string(ERR_get_error(), NULL));
            ret = -1;
            break;
        }

        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx_gen, bits) <= 0) {
            LOG_Error("[%s] EVP_PKEY_CTX_set_rsa_keygen_bits failed: %s", __func__,
                      ERR_error_string(ERR_get_error(), NULL));
            ret = -1;
            break;
        }

        if (EVP_PKEY_keygen(ctx_gen, &pkey) <= 0) {
            LOG_Error("[%s] EVP_PKEY_keygen failed: %s", __func__,
                      ERR_error_string(ERR_get_error(), NULL));
            ret = -1;
            break;
        }

        if (simulateSaveKey(keyId, pkey, 0) != 0) {
            LOG_Error("[%s] Failed to save private key", __func__);
            ret = -1;
            break;
        }

        if (simulateSaveKey(keyId, pkey, 1) != 0) {
            LOG_Error("[%s] Failed to save public key", __func__);
            ret = -1;
            break;
        }

        derLen = i2d_PUBKEY(pkey, NULL);
        if (derLen <= 0) {
            LOG_Error("[%s] i2d_PUBKEY failed", __func__);
            ret = -1;
            break;
        }

        if ((uint32_t)derLen > *pubKeyDerBufSize) {
            LOG_Error("[%s] Buffer too small: %u < %d", __func__, *pubKeyDerBufSize, derLen);
            *pubKeyDerBufSize = (uint32_t)derLen;
            ret = -1;
            break;
        }

        p = pubKeyDer;
        derLen = i2d_PUBKEY(pkey, &p);
        if (derLen <= 0) {
            LOG_Error("[%s] i2d_PUBKEY write failed", __func__);
            ret = -1;
            break;
        }

        *pubKeyDerBufSize = (uint32_t)derLen;
        LOG_Info("[%s] Key pair generated successfully, pubKeyLen=%u", __func__, *pubKeyDerBufSize);
        ret = 0;
    } while (0);

    if (ctx_gen) {
        EVP_PKEY_CTX_free(ctx_gen);
    }
    if (pkey) {
        EVP_PKEY_free(pkey);
    }

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static const EVP_MD* simulateGetEvpMd(DigistAlg_e digist)
{
    const EVP_MD *md = NULL;

    LOG_Warn("[%s]enter", __func__);

    do {
        switch (digist) {
            case E_DIGIST_SHA1:   md = EVP_sha1(); break;
            case E_DIGIST_SHA224: md = EVP_sha224(); break;
            case E_DIGIST_SHA256: md = EVP_sha256(); break;
            case E_DIGIST_SHA384: md = EVP_sha384(); break;
            case E_DIGIST_SHA512: md = EVP_sha512(); break;
            default:
                md = NULL;
                break;
        }
    } while (0);

    LOG_Warn("[%s]exit", __func__);
    return md;
}

static int hsmSimulateRsaSign(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, DigistAlg_e digist, uint8_t *plain, uint32_t plainLen, uint8_t *sign, uint32_t* signBufSize)
{
    int ret = -1;
    const EVP_MD *md = NULL;
    EVP_PKEY *pkey = NULL;
    EVP_MD_CTX *mdctx = NULL;
    size_t siglen = 0;

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
        (void)alg;
        LOG_Debug("[%s] Request: keyId=0x%04X, digist=%d, plainLen=%u", __func__, keyId, digist, plainLen);

        if (plain == NULL || sign == NULL || signBufSize == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            ret = -1;
            break;
        }

        md = simulateGetEvpMd(digist);
        if (md == NULL) {
            LOG_Error("[%s] Unsupported hash algorithm: %d", __func__, digist);
            ret = -1;
            break;
        }

        if (simulateLoadKey(keyId, &pkey, 0) != 0) {
            LOG_Error("[%s] Failed to load private key: keyId=0x%04X", __func__, keyId);
            ret = -1;
            break;
        }

        mdctx = EVP_MD_CTX_new();
        if (!mdctx) {
            LOG_Error("[%s] EVP_MD_CTX_new failed", __func__);
            ret = -1;
            break;
        }

        siglen = *signBufSize;

        if (EVP_DigestSignInit(mdctx, NULL, md, NULL, pkey) != 1) {
            LOG_Error("[%s] EVP_DigestSignInit failed", __func__);
            ret = -1;
            break;
        }

        if (EVP_DigestSignUpdate(mdctx, plain, plainLen) != 1) {
            LOG_Error("[%s] EVP_DigestSignUpdate failed", __func__);
            ret = -1;
            break;
        }

        if (EVP_DigestSignFinal(mdctx, NULL, &siglen) != 1) {
            LOG_Error("[%s] EVP_DigestSignFinal (get length) failed", __func__);
            ret = -1;
            break;
        }

        if (siglen > *signBufSize) {
            LOG_Error("[%s] Buffer too small: %zu > %u", __func__, siglen, *signBufSize);
            *signBufSize = siglen;
            ret = -1;
            break;
        }

        if (EVP_DigestSignFinal(mdctx, sign, &siglen) != 1) {
            LOG_Error("[%s] EVP_DigestSignFinal failed", __func__);
            ret = -1;
            break;
        }

        *signBufSize = siglen;
        LOG_Info("[%s] Signature success: len=%zu", __func__, siglen);
        ret = 0;
    } while (0);

    if (mdctx) {
        EVP_MD_CTX_free(mdctx);
    }
    if (pkey) {
        EVP_PKEY_free(pkey);
    }

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateRsaVerify(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, DigistAlg_e digist, uint8_t *plain, uint32_t plainLen, uint8_t *sign, uint32_t signBufSize)
{
    int ret = -1;
    const EVP_MD *md = NULL;
    EVP_PKEY *pkey = NULL;
    EVP_MD_CTX *mdctx = NULL;
    int vret = 0;

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
        (void)alg;
        LOG_Debug("[%s] Request: keyId=0x%04X, digist=%d, plainLen=%u, signLen=%u", __func__, keyId, digist, plainLen, signBufSize);

        if (plain == NULL || sign == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            ret = -1;
            break;
        }

        md = simulateGetEvpMd(digist);
        if (md == NULL) {
            LOG_Error("[%s] Unsupported hash algorithm: %d", __func__, digist);
            ret = -1;
            break;
        }

        if (simulateLoadKey(keyId, &pkey, 1) != 0) {
            /* 尝试从私钥加载并提取公钥 */
            if (simulateLoadKey(keyId, &pkey, 0) != 0) {
                LOG_Error("[%s] Failed to load key: keyId=0x%04X", __func__, keyId);
                ret = -1;
                break;
            }
        }

        mdctx = EVP_MD_CTX_new();
        if (!mdctx) {
            LOG_Error("[%s] EVP_MD_CTX_new failed", __func__);
            ret = -1;
            break;
        }

        if (EVP_DigestVerifyInit(mdctx, NULL, md, NULL, pkey) != 1) {
            LOG_Error("[%s] EVP_DigestVerifyInit failed", __func__);
            ret = -1;
            break;
        }

        if (EVP_DigestVerifyUpdate(mdctx, plain, plainLen) != 1) {
            LOG_Error("[%s] EVP_DigestVerifyUpdate failed", __func__);
            ret = -1;
            break;
        }

        vret = EVP_DigestVerifyFinal(mdctx, sign, signBufSize);
        if (vret == 1) {
            LOG_Info("[%s] Signature verification success", __func__);
            ret = 0;
        } else {
            LOG_Warn("[%s] Signature verification failed", __func__);
            ret = 1; /* 与原接口一致：验签失败返回1 */
        }
    } while (0);

    if (mdctx) {
        EVP_MD_CTX_free(mdctx);
    }
    if (pkey) {
        EVP_PKEY_free(pkey);
    }

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateRsaEncrypt(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, RsaSignPaddingAlg_e padding, uint8_t *plain, uint32_t plainLen, uint8_t *out, uint32_t* outBufSize)
{
    int ret = -1;
    int pad = RSA_PKCS1_PADDING;
    EVP_PKEY *pkey = NULL;
    size_t outlen = 0;
    EVP_PKEY_CTX *ectx = NULL;

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
        (void)alg;
        LOG_Debug("[%s] Request: keyId=0x%04X, padding=%d, plainLen=%u", __func__, keyId, padding, plainLen);

        if (plain == NULL || out == NULL || outBufSize == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            ret = -1;
            break;
        }

        ret = 0;
        switch (padding) {
            case E_RSA_SIGN_PADDING_NONE:
                pad = RSA_NO_PADDING;
                break;
            case E_RSA_SIGN_PADDING_OAEP:
                pad = RSA_PKCS1_OAEP_PADDING;
                break;
            case E_RSA_SIGN_PADDING_PKCS1:
                pad = RSA_PKCS1_PADDING;
                break;
            case E_RSA_SIGN_PADDING_PSS:
                LOG_Error("[%s] PSS padding not supported for encryption", __func__);
                ret = -1;
                break;
            default:
                LOG_Error("[%s] Unsupported padding: %d", __func__, padding);
                ret = -1;
                break;
        }
        if (ret != 0) {
            break;
        }

        if (simulateLoadKey(keyId, &pkey, 1) != 0) {
            /* 尝试从私钥加载 */
            if (simulateLoadKey(keyId, &pkey, 0) != 0) {
                LOG_Error("[%s] Failed to load key: keyId=0x%04X", __func__, keyId);
                ret = -1;
                break;
            }
        }

        ectx = EVP_PKEY_CTX_new(pkey, NULL);
        if (!ectx) {
            LOG_Error("[%s] EVP_PKEY_CTX_new failed", __func__);
            ret = -1;
            break;
        }

        if (EVP_PKEY_encrypt_init(ectx) <= 0) {
            LOG_Error("[%s] EVP_PKEY_encrypt_init failed", __func__);
            ret = -1;
            break;
        }

        if (EVP_PKEY_CTX_set_rsa_padding(ectx, pad) <= 0) {
            LOG_Error("[%s] EVP_PKEY_CTX_set_rsa_padding failed", __func__);
            ret = -1;
            break;
        }

        if (pad == RSA_PKCS1_OAEP_PADDING &&
            (EVP_PKEY_CTX_set_rsa_oaep_md(ectx, EVP_sha256()) <= 0 ||
             EVP_PKEY_CTX_set_rsa_mgf1_md(ectx, EVP_sha256()) <= 0)) {
            LOG_Error("[%s] EVP_PKEY_CTX_set_rsa_oaep_md/mgf1_md failed", __func__);
            ret = -1;
            break;
        }

        outlen = *outBufSize;
        if (EVP_PKEY_encrypt(ectx, out, &outlen, plain, plainLen) <= 0) {
            LOG_Error("[%s] EVP_PKEY_encrypt failed", __func__);
            ret = -1;
            break;
        }

        *outBufSize = outlen;
        LOG_Info("[%s] Encrypted success: len=%zu", __func__, outlen);
        ret = 0;
    } while (0);

    if (ectx) {
        EVP_PKEY_CTX_free(ectx);
    }
    if (pkey) {
        EVP_PKEY_free(pkey);
    }

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateRsaDecrypt(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, RsaSignPaddingAlg_e padding, uint8_t *encryptedData, uint32_t encryptedDataLen, uint8_t *out, uint32_t* outBufSize)
{
    int ret = -1;
    int pad = RSA_PKCS1_PADDING;
    EVP_PKEY *pkey = NULL;
    size_t outlen = 0;
    EVP_PKEY_CTX *dctx = NULL;

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
        (void)alg;
        LOG_Debug("[%s] Request: keyId=0x%04X, padding=%d, encryptedDataLen=%u", __func__, keyId, padding, encryptedDataLen);

        if (encryptedData == NULL || out == NULL || outBufSize == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            ret = -1;
            break;
        }

        ret = 0;
        switch (padding) {
            case E_RSA_SIGN_PADDING_NONE:
                pad = RSA_NO_PADDING;
                break;
            case E_RSA_SIGN_PADDING_OAEP:
                pad = RSA_PKCS1_OAEP_PADDING;
                break;
            case E_RSA_SIGN_PADDING_PKCS1:
                pad = RSA_PKCS1_PADDING;
                break;
            case E_RSA_SIGN_PADDING_PSS:
                LOG_Error("[%s] PSS padding not supported for decryption", __func__);
                ret = -1;
                break;
            default:
                LOG_Error("[%s] Unsupported padding: %d", __func__, padding);
                ret = -1;
                break;
        }
        if (ret != 0) {
            break;
        }

        if (simulateLoadKey(keyId, &pkey, 0) != 0) {
            LOG_Error("[%s] Failed to load private key: keyId=0x%04X", __func__, keyId);
            ret = -1;
            break;
        }

        dctx = EVP_PKEY_CTX_new(pkey, NULL);
        if (!dctx) {
            LOG_Error("[%s] EVP_PKEY_CTX_new failed", __func__);
            ret = -1;
            break;
        }

        if (EVP_PKEY_decrypt_init(dctx) <= 0) {
            LOG_Error("[%s] EVP_PKEY_decrypt_init failed", __func__);
            ret = -1;
            break;
        }

        if (EVP_PKEY_CTX_set_rsa_padding(dctx, pad) <= 0) {
            LOG_Error("[%s] EVP_PKEY_CTX_set_rsa_padding failed", __func__);
            ret = -1;
            break;
        }

        if (pad == RSA_PKCS1_OAEP_PADDING &&
            (EVP_PKEY_CTX_set_rsa_oaep_md(dctx, EVP_sha256()) <= 0 ||
             EVP_PKEY_CTX_set_rsa_mgf1_md(dctx, EVP_sha256()) <= 0)) {
            LOG_Error("[%s] EVP_PKEY_CTX_set_rsa_oaep_md/mgf1_md failed", __func__);
            ret = -1;
            break;
        }

        outlen = *outBufSize;
        if (EVP_PKEY_decrypt(dctx, out, &outlen, encryptedData, encryptedDataLen) <= 0) {
            LOG_Error("[%s] EVP_PKEY_decrypt failed", __func__);
            ret = -1;
            break;
        }

        *outBufSize = outlen;
        LOG_Info("[%s] Decrypted success: len=%zu", __func__, outlen);
        ret = 0;
    } while (0);

    if (dctx) {
        EVP_PKEY_CTX_free(dctx);
    }
    if (pkey) {
        EVP_PKEY_free(pkey);
    }

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateQueryKey(void* ctx, QueryKeyResult_t *keyResult)
{
    int ret = 0;
    int i = 0;
    EVP_PKEY *pkey = NULL;
    int hasPriv = 0;
    RSA *rsa = NULL;
    int bits = 0;
    uint8_t validCount = 0;

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
        LOG_Debug("[%s] Request", __func__);

        if (keyResult == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            ret = -1;
            break;
        }

        memset(keyResult, 0, sizeof(QueryKeyResult_t));

        for (i = 0; i < SIMULATE_KEY_SLOT_COUNT && validCount < KEY_SLOT_COUNT; i++) {
            pkey = NULL;
            hasPriv = 0;

            if (simulateLoadKey(i, &pkey, 0) == 0) {
                hasPriv = 1;
            } else if (simulateLoadKey(i, &pkey, 1) == 0) {
                hasPriv = 0;
            } else {
                continue;
            }

            rsa = EVP_PKEY_get1_RSA(pkey);
            if (rsa) {
                bits = RSA_bits(rsa);

                keyResult->keyList[validCount].keyId = (uint8_t)i;
                keyResult->keyList[validCount].keyAlg = E_HSM_KEY_ALG_RSA;
                keyResult->keyList[validCount].keyType = hasPriv ? E_HSM_KEY_TYPE_KEY_PAIR : E_HSM_KEY_TYPE_PUBLIC;
                keyResult->keyList[validCount].keyBits = (uint32_t)bits;
                validCount++;

                RSA_free(rsa);
                rsa = NULL;
            }
            EVP_PKEY_free(pkey);
            pkey = NULL;
        }

        keyResult->validKeyCount = validCount;
        LOG_Info("[%s] Success, validKeyCount=%u", __func__, validCount);
        ret = 0;
    } while (0);

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateQueryKeyExists(void* ctx, const uint16_t keyId)
{
    int ret = 0;
    char path[256] = {0};

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;

        if (keyId >= SIMULATE_KEY_SLOT_COUNT) {
            ret = 0;
            break;
        }

        simulateGetKeyPath(keyId, path, sizeof(path), 0);
        if (access(path, F_OK) == 0) {
            ret = 1;
            break;
        }

        simulateGetKeyPath(keyId, path, sizeof(path), 1);
        if (access(path, F_OK) == 0) {
            ret = 1;
            break;
        }

        ret = 0;
    } while (0);

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateDeleteKey(void* ctx, const uint16_t keyId)
{
    int ret = 0;
    char path[256] = {0};

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
        LOG_Debug("[%s] Request: keyId=0x%04X", __func__, keyId);

        simulateGetKeyPath(keyId, path, sizeof(path), 0);
        if (unlink(path) != 0 && errno != ENOENT) {
            LOG_Error("[%s] Failed to delete private key: %s", __func__, path);
            ret = -1;
        }

        simulateGetKeyPath(keyId, path, sizeof(path), 1);
        if (unlink(path) != 0 && errno != ENOENT) {
            LOG_Error("[%s] Failed to delete public key: %s", __func__, path);
            ret = -1;
        }

        if (ret == 0) {
            LOG_Info("[%s] Key deleted successfully", __func__);
        }
    } while (0);

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateExportKey(void* ctx, RsaKeyAlg_e exKeyAlg, const uint16_t exKeyId, uint8_t exKeyType, uint8_t *out, uint32_t *outBufSize)
{
    int ret = -1;
    EVP_PKEY *pkey = NULL;
    PKCS8_PRIV_KEY_INFO *pkcs8Info = NULL;
    int derLen = 0;
    uint8_t *p = NULL;

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
        (void)exKeyAlg;
        LOG_Debug("[%s] Request: exKeyId=0x%04X, exKeyType=%d", __func__, exKeyId, exKeyType);

        if (out == NULL || outBufSize == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            ret = -1;
            break;
        }

        if (exKeyType != E_KEY_TYPE_PUB && exKeyType != E_KEY_TYPE_PRI && exKeyType != E_KEY_TYPE_PRI_PUB_PAIR) {
            LOG_Error("[%s] Unsupported keyType: %d", __func__, exKeyType);
            ret = -1;
            break;
        }

        if (exKeyType == E_KEY_TYPE_PUB) {
            if (simulateLoadKey(exKeyId, &pkey, 1) != 0) {
                if (simulateLoadKey(exKeyId, &pkey, 0) != 0) {
                    LOG_Error("[%s] Failed to load key: exKeyId=0x%04X", __func__, exKeyId);
                    ret = -1;
                    break;
                }
            }

            derLen = i2d_PUBKEY(pkey, NULL);
            if (derLen <= 0) {
                LOG_Error("[%s] i2d_PUBKEY failed", __func__);
                ret = -1;
                break;
            }

            if ((uint32_t)derLen > *outBufSize) {
                LOG_Error("[%s] Buffer too small: %u < %d", __func__, *outBufSize, derLen);
                *outBufSize = (uint32_t)derLen;
                ret = -1;
                break;
            }

            p = out;
            derLen = i2d_PUBKEY(pkey, &p);
            if (derLen <= 0) {
                LOG_Error("[%s] i2d_PUBKEY write failed", __func__);
                ret = -1;
                break;
            }

            *outBufSize = (uint32_t)derLen;
        } else {
            if (simulateLoadKey(exKeyId, &pkey, 0) != 0) {
                LOG_Error("[%s] Failed to load private key: exKeyId=0x%04X", __func__, exKeyId);
                ret = -1;
                break;
            }

            pkcs8Info = EVP_PKEY2PKCS8(pkey);
            if (pkcs8Info == NULL) {
                LOG_Error("[%s] EVP_PKEY2PKCS8 failed", __func__);
                ret = -1;
                break;
            }

            derLen = i2d_PKCS8_PRIV_KEY_INFO(pkcs8Info, NULL);
            if (derLen <= 0) {
                LOG_Error("[%s] i2d_PKCS8_PRIV_KEY_INFO failed", __func__);
                ret = -1;
                break;
            }

            if ((uint32_t)derLen > *outBufSize) {
                LOG_Error("[%s] Buffer too small: %u < %d", __func__, *outBufSize, derLen);
                *outBufSize = (uint32_t)derLen;
                ret = -1;
                break;
            }

            p = out;
            derLen = i2d_PKCS8_PRIV_KEY_INFO(pkcs8Info, &p);
            if (derLen <= 0) {
                LOG_Error("[%s] i2d_PKCS8_PRIV_KEY_INFO write failed", __func__);
                ret = -1;
                break;
            }

            *outBufSize = (uint32_t)derLen;
        }

        LOG_Info("[%s] Key exported successfully: len=%u", __func__, *outBufSize);
        ret = 0;
    } while (0);

    if (pkcs8Info) {
        PKCS8_PRIV_KEY_INFO_free(pkcs8Info);
    }
    if (pkey) {
        EVP_PKEY_free(pkey);
    }

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateImportKey(void* ctx, RsaKeyAlg_e inKeyAlg, const uint16_t inKeyId, uint8_t inKeyType, const uint8_t *inKeyVal, uint32_t inKeyValLen)
{
    int ret = -1;
    EVP_PKEY *pkey = NULL;
    PKCS8_PRIV_KEY_INFO *pkcs8Info = NULL;
    const uint8_t *p = NULL;

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
        (void)inKeyAlg;
        LOG_Debug("[%s] Request: inKeyId=0x%04X, inKeyType=%d, inKeyValLen=%u",
                  __func__, inKeyId, inKeyType, inKeyValLen);

        if (inKeyVal == NULL || inKeyValLen == 0) {
            LOG_Error("[%s] Invalid parameters", __func__);
            ret = -1;
            break;
        }

        if (inKeyType != E_KEY_TYPE_PUB && inKeyType != E_KEY_TYPE_PRI && inKeyType != E_KEY_TYPE_PRI_PUB_PAIR) {
            LOG_Error("[%s] Unsupported keyType: %d", __func__, inKeyType);
            ret = -1;
            break;
        }

        if (inKeyType == E_KEY_TYPE_PUB) {
            p = inKeyVal;
            pkey = d2i_PUBKEY(NULL, &p, inKeyValLen);
            if (pkey == NULL) {
                LOG_Error("[%s] d2i_PUBKEY failed", __func__);
                ret = -1;
                break;
            }

            if (simulateSaveKey(inKeyId, pkey, 1) != 0) {
                LOG_Error("[%s] Failed to save public key", __func__);
                ret = -1;
                break;
            }
        } else {
            p = inKeyVal;
            pkcs8Info = d2i_PKCS8_PRIV_KEY_INFO(NULL, &p, inKeyValLen);
            if (pkcs8Info == NULL) {
                LOG_Error("[%s] d2i_PKCS8_PRIV_KEY_INFO failed", __func__);
                ret = -1;
                break;
            }

            pkey = EVP_PKCS82PKEY(pkcs8Info);
            if (pkey == NULL) {
                LOG_Error("[%s] EVP_PKCS82PKEY failed", __func__);
                ret = -1;
                break;
            }

            if (simulateSaveKey(inKeyId, pkey, 0) != 0) {
                LOG_Error("[%s] Failed to save private key", __func__);
                ret = -1;
                break;
            }

            if (inKeyType == E_KEY_TYPE_PRI_PUB_PAIR) {
                if (simulateSaveKey(inKeyId, pkey, 1) != 0) {
                    LOG_Error("[%s] Failed to save public key", __func__);
                    ret = -1;
                    break;
                }
            }
        }

        LOG_Info("[%s] Key imported successfully", __func__);
        ret = 0;
    } while (0);

    if (pkcs8Info) {
        PKCS8_PRIV_KEY_INFO_free(pkcs8Info);
    }
    if (pkey) {
        EVP_PKEY_free(pkey);
    }

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

/* ===================== 证书模拟存储 ===================== */

static void simulateGetCrtPath(uint16_t crtId, char *path, size_t pathLen)
{
    LOG_Warn("[%s]enter", __func__);
    snprintf(path, pathLen, "%s/cert_%04x.pem", SIMULATE_HSM_KEY_DIR, crtId);
    LOG_Warn("[%s]exit", __func__);
}

static int hsmSimulateQueryCrt(void* ctx, QueryCrtResult_t *crtResult)
{
    int ret = 0;
    int count = 0;
    uint16_t id = 0;
    DIR *dir = NULL;
    struct dirent *entry = NULL;

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
        LOG_Debug("[%s] Request", __func__);

        if (crtResult == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            ret = -1;
            break;
        }

        memset(crtResult, 0, sizeof(QueryCrtResult_t));

        dir = opendir(SIMULATE_HSM_KEY_DIR);
        if (!dir) {
            LOG_Error("[%s] Failed to open directory: %s", __func__, SIMULATE_HSM_KEY_DIR);
            ret = 0; /* 空目录视为成功 */
            break;
        }

        count = 0;
        while ((entry = readdir(dir)) != NULL && count < CRT_SLOT_COUNT) {
            if (sscanf(entry->d_name, "cert_%hx.pem", &id) == 1) {
                crtResult->crtList[count] = id;
                count++;
            }
        }
        closedir(dir);

        crtResult->validCrtCount = (uint8_t)count;
        LOG_Info("[%s] Success, validCrtCount=%d", __func__, count);
        ret = 0;
    } while (0);

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateQueryCrtExists(void* ctx, const uint16_t crtId)
{
    int ret = 0;
    char path[256] = {0};

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;

        simulateGetCrtPath(crtId, path, sizeof(path));
        if (access(path, F_OK) == 0) {
            ret = 1;
        } else {
            ret = 0;
        }
    } while (0);

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateDeleteCrt(void* ctx, const uint16_t crtId)
{
    int ret = 0;
    char path[256] = {0};

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
        LOG_Debug("[%s] Request: crtId=0x%04X", __func__, crtId);

        simulateGetCrtPath(crtId, path, sizeof(path));

        if (unlink(path) != 0 && errno != ENOENT) {
            LOG_Error("[%s] Failed to delete certificate: %s", __func__, path);
            ret = -1;
            break;
        }

        LOG_Info("[%s] Certificate deleted successfully", __func__);
    } while (0);

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateExportCrt(void* ctx, const uint16_t crtId, uint8_t *out, uint32_t *outBufSize)
{
    int ret = 0;
    char path[256] = {0};
    FILE *fp = NULL;
    long fileSize = 0;
    size_t readLen = 0;

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
        LOG_Debug("[%s] Request: crtId=0x%04X", __func__, crtId);

        if (out == NULL || outBufSize == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            ret = -1;
            break;
        }

        simulateGetCrtPath(crtId, path, sizeof(path));

        fp = fopen(path, "rb");
        if (!fp) {
            LOG_Error("[%s] Failed to open certificate file: %s", __func__, path);
            ret = -1;
            break;
        }

        fseek(fp, 0, SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        if (fileSize < 0 || (uint32_t)fileSize > *outBufSize) {
            LOG_Error("[%s] Buffer too small or file error: fileSize=%ld, bufSize=%u", __func__, fileSize, *outBufSize);
            fclose(fp);
            *outBufSize = (fileSize > 0) ? (uint32_t)fileSize : 0;
            ret = -1;
            break;
        }

        readLen = fread(out, 1, (size_t)fileSize, fp);
        fclose(fp);

        if (readLen != (size_t)fileSize) {
            LOG_Error("[%s] Failed to read certificate file", __func__);
            ret = -1;
            break;
        }

        *outBufSize = (uint32_t)readLen;
        LOG_Info("[%s] Certificate exported successfully: len=%zu", __func__, readLen);
        ret = 0;
    } while (0);

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

static int hsmSimulateImportCrt(void* ctx, const uint16_t crtId, const uint8_t *inCrtVal, uint32_t inCrtValLen)
{
    int ret = 0;
    char path[256] = {0};
    FILE *fp = NULL;
    size_t written = 0;

    LOG_Warn("[%s]enter", __func__);

    do {
        (void)ctx;
        LOG_Debug("[%s] Request: crtId=0x%04X, crtValLen=%u", __func__, crtId, inCrtValLen);

        if (inCrtVal == NULL || inCrtValLen == 0) {
            LOG_Error("[%s] Invalid parameters", __func__);
            ret = -1;
            break;
        }

        simulateGetCrtPath(crtId, path, sizeof(path));

        fp = fopen(path, "wb");
        if (!fp) {
            LOG_Error("[%s] Failed to open file for writing: %s", __func__, path);
            ret = -1;
            break;
        }

        written = fwrite(inCrtVal, 1, inCrtValLen, fp);
        fclose(fp);

        if (written != inCrtValLen) {
            LOG_Error("[%s] Failed to write certificate file", __func__);
            ret = -1;
            break;
        }

        LOG_Info("[%s] Certificate imported successfully", __func__);
        ret = 0;
    } while (0);

    LOG_Warn("[%s]exit(%d)", __func__, ret);
    return ret;
}

const HsmApi_t g_hsmSimulate_Api = {
    .init = hsmSimulateInit,
    .release = hsmSimulateRelease,
    .random = hsmSimulateRandom,
    .rsaDecrypt = hsmSimulateRsaDecrypt,
    .rsaEncrypt = hsmSimulateRsaEncrypt,
    .rsaKeyPairGenerate = hsmSimulateRsaKeyPairGenerate,
    .rsaSign = hsmSimulateRsaSign,
    .rsaVerify = hsmSimulateRsaVerify,
    .queryKey = hsmSimulateQueryKey,
    .queryKeyExists = hsmSimulateQueryKeyExists,
    .deleteKey = hsmSimulateDeleteKey,
    .exportKey = hsmSimulateExportKey,
    .importKey = hsmSimulateImportKey,
    .queryCrt = hsmSimulateQueryCrt,
    .queryCrtExists = hsmSimulateQueryCrtExists,
    .deleteCrt = hsmSimulateDeleteCrt,
    .exportCrt = hsmSimulateExportCrt,
    .importCrt = hsmSimulateImportCrt,
};
