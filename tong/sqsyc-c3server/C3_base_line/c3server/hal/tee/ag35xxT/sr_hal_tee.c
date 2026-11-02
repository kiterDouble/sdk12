#include "sr_hal_tee.h"
#include "sr_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/random.h>

#include <openssl/evp.h>
#include <openssl/rsa.h>

#include "ql_tee_service.h"
#include "ql_tee_asymm_utils.h"
static int qlTeeKmInited = 0;
static int qlTeeSsInited = 0;

/*
 * HSM 层传入的 padding/digest 值与 QL_KM 枚举的映射。
 * 数值与 rpc_hsm_api.h 中 RsaSignPaddingAlg_e / DigistAlg_e 保持一致，
 * 避免 HAL 层反向依赖 srsdk 头文件。
 */
#define TEE_PAD_NONE        0x0
#define TEE_PAD_OAEP        0x1
#define TEE_PAD_PSS         0x2
#define TEE_PAD_PKCS1       0x3

static int QlTeeMapPadding(uint8_t padding, ql_km_purpose_t purpose)
{
    switch (padding) {
        case TEE_PAD_NONE:  return QL_KM_PAD_RSA_NONE;
        case TEE_PAD_OAEP:  return QL_KM_PAD_RSA_OAEP;
        case TEE_PAD_PSS:   return QL_KM_PAD_RSA_PSS;
        case TEE_PAD_PKCS1:
            return (purpose == QL_KM_PURPOSE_ENCRYPT || purpose == QL_KM_PURPOSE_DECRYPT)
                   ? QL_KM_PAD_RSA_PKCS1_1_5_ENCRYPT
                   : QL_KM_PAD_RSA_PKCS1_1_5_SIGN;
        default:            return -1;
    }
}

static int QlTeeMapDigest(uint8_t digist)
{
    switch (digist) {
        case 0x0:  return QL_KM_DIGEST_NONE;
        case 0x1:  return QL_KM_DIGEST_SHA1;
        case 0x2:  return QL_KM_DIGEST_SHA_2_224;
        case 0x3:  return QL_KM_DIGEST_SHA_2_256;
        case 0x4:  return QL_KM_DIGEST_SHA_2_384;
        case 0x5:  return QL_KM_DIGEST_SHA_2_512;
        default:   return -1;
    }
}

int SrHalTeeInit(void)
{
    int ret = -1;
    ql_tee_error_t err;

    do {
        if (qlTeeKmInited) {
            ret = 0;
            break;
        }

        err = ql_km_initialize();
        if (err != QL_TEE_OK) {
            LOG_Error("[%s] ql_km_initialize failed: %d", __func__, err);
            break;
        }

        qlTeeKmInited = 1;
        LOG_Info("[%s] TEE Keymaster initialized", __func__);
        ret = 0;
    } while (0);

    return ret;
}

void SrHalTeeRelease(void)
{
    if (qlTeeKmInited) {
        ql_km_deinitialize();
        qlTeeKmInited = 0;
        LOG_Info("[%s] TEE Keymaster released", __func__);
    }
}

int SrHalTeeRandom(uint32_t expectedLen, uint8_t *random)
{
    int ret = -1;
    ssize_t rc = 0;
    size_t total = 0;
    int fd = -1;
    ssize_t n = 0;

    do {
        if (random == NULL || expectedLen == 0) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        rc = getrandom(random, expectedLen, 0);
        if (rc == (ssize_t)expectedLen) {
            ret = 0;
            break;
        }

        /* fallback to /dev/urandom */
        fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0) {
            LOG_Error("[%s] open /dev/urandom failed: %s", __func__, strerror(errno));
            break;
        }

        total = 0;
        while (total < expectedLen) {
            n = read(fd, random + total, expectedLen - total);
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }
                LOG_Error("[%s] read /dev/urandom failed: %s", __func__, strerror(errno));
                break;
            }
            if (n == 0) {
                LOG_Error("[%s] /dev/urandom EOF", __func__);
                break;
            }
            total += n;
        }

        if (total == expectedLen) {
            ret = 0;
        }
    } while (0);

    if (fd >= 0) {
        close(fd);
    }

    return ret;
}

int SrHalTeeRsaGenKey(uint32_t bits, uint8_t **pubDer, uint32_t *pubDerLen,
                      uint8_t **keyBlob, uint32_t *keyBlobLen)
{
    int ret = -1;
    ql_tee_error_t err;
    ql_km_key_args_t keyArgs = {0};
    ql_km_key_t key = {0};
    ql_km_blob_t pubKey = {0};

    do {
        if (pubDer == NULL || pubDerLen == NULL || keyBlob == NULL || keyBlobLen == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        err = ql_rsa_genkey_args(65537, bits, &keyArgs);
        if (err != QL_TEE_OK) {
            LOG_Error("[%s] ql_rsa_genkey_args failed: %d", __func__, err);
            break;
        }

        err = ql_km_gen_key(&keyArgs, &key);
        if (err != QL_TEE_OK) {
            LOG_Error("[%s] ql_km_gen_key failed: %d", __func__, err);
            break;
        }

        err = ql_km_export_key(&key, &pubKey);
        if (err != QL_TEE_OK) {
            LOG_Error("[%s] ql_km_export_key failed: %d", __func__, err);
            break;
        }

        *pubDer = (uint8_t *)malloc(pubKey.data_size);
        *keyBlob = (uint8_t *)malloc(key.key_blob_size);
        if (*pubDer == NULL || *keyBlob == NULL) {
            LOG_Error("[%s] malloc failed", __func__);
            break;
        }
        memcpy(*pubDer, pubKey.data, pubKey.data_size);
        *pubDerLen = pubKey.data_size;
        memcpy(*keyBlob, key.key_blob, key.key_blob_size);
        *keyBlobLen = key.key_blob_size;

        LOG_Info("[%s] RSA key generated, bits=%u, pubDerLen=%u, blobLen=%u", __func__,
                 bits, *pubDerLen, *keyBlobLen);
        ret = 0;
    } while (0);

    if (pubKey.data) {
        ql_km_destroy_blob(&pubKey);
    }
    if (key.key_blob) {
        ql_km_destroy_key(&key);
    }

    return ret;
}

int SrHalTeeImportKey(const uint8_t *pkcs8Der, uint32_t derLen, uint8_t **keyBlob, uint32_t *keyBlobLen)
{
    int ret = -1;
    ql_tee_error_t err;
    ql_km_key_args_t keyArgs = {0};
    ql_km_blob_t rawKey = {0};
    ql_km_key_t key = {0};
    EVP_PKEY *pkey = NULL;
    const uint8_t *p = NULL;
    int keyBits = 2048;

    do {
        if (pkcs8Der == NULL || derLen == 0 || keyBlob == NULL || keyBlobLen == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        /* 从 PKCS#8 DER 中自适应识别 RSA 密钥长度 */
        p = pkcs8Der;
        pkey = d2i_AutoPrivateKey(NULL, &p, (long)derLen);
        if (pkey == NULL) {
            LOG_Error("[%s] d2i_AutoPrivateKey failed", __func__);
            break;
        }
        keyBits = EVP_PKEY_bits(pkey);
        if (keyBits <= 0) {
            LOG_Error("[%s] EVP_PKEY_bits failed: %d", __func__, keyBits);
            break;
        }
        EVP_PKEY_free(pkey);
        pkey = NULL;

        /* ql_rsa_genkey_args 并非真正生成密钥，而是填充 ql_km_key_args_t 密钥参数
         *（算法类型、公钥指数 e、密钥长度），供 ql_km_import_key 识别目标密钥规格 */
        err = ql_rsa_genkey_args(65537, (uint32_t)keyBits, &keyArgs);
        if (err != QL_TEE_OK) {
            LOG_Error("[%s] ql_rsa_genkey_args failed: %d", __func__, err);
            break;
        }

        rawKey.data = (uint8_t *)pkcs8Der;
        rawKey.data_size = derLen;
  
        LOG_Info("[%s] ql_km_import_key enter, derLen=%u, head=%02X%02X%02X%02X", __func__,
                 derLen, pkcs8Der[0], pkcs8Der[1], pkcs8Der[2], pkcs8Der[3]);
        err = ql_km_import_key(&keyArgs, &rawKey, &key);
        LOG_Info("[%s] ql_km_import_key exit, err=%d", __func__, err);
        if (err != QL_TEE_OK) {
            LOG_Error("[%s] ql_km_import_key failed: %d", __func__, err);
            break;
        }

        *keyBlob = (uint8_t *)malloc(key.key_blob_size);
        if (*keyBlob == NULL) {
            LOG_Error("[%s] malloc failed", __func__);
            break;
        }
        memcpy(*keyBlob, key.key_blob, key.key_blob_size);
        *keyBlobLen = key.key_blob_size;

        LOG_Info("[%s] Key imported, bits=%d, blobLen=%u", __func__, keyBits, *keyBlobLen);
        ret = 0;
    } while (0);

    if (pkey != NULL) {
        EVP_PKEY_free(pkey);
    }
    if (key.key_blob) {
        ql_km_destroy_key(&key);
    }

    return ret;
}

int SrHalTeeDestroyKey(uint8_t *keyBlob, uint32_t keyBlobLen)
{
    ql_km_key_t key = { .key_blob = keyBlob, .key_blob_size = keyBlobLen };
    ql_tee_error_t err;

    err = ql_km_destroy_key(&key);
    if (err != QL_TEE_OK) {
        LOG_Error("[%s] ql_km_destroy_key failed: %d", __func__, err);
        return -1;
    }
    return 0;
}

int SrHalTeeExportPubKey(const uint8_t *keyBlob, uint32_t keyBlobLen, uint8_t **pubDer, uint32_t *pubDerLen)
{
    int ret = -1;
    ql_tee_error_t err;
    ql_km_key_t key = { .key_blob = (uint8_t *)keyBlob, .key_blob_size = keyBlobLen };
    ql_km_blob_t pubKey = {0};

    do {
        if (keyBlob == NULL || keyBlobLen == 0 || pubDer == NULL || pubDerLen == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        err = ql_km_export_key(&key, &pubKey);
        if (err != QL_TEE_OK) {
            LOG_Error("[%s] ql_km_export_key failed: %d", __func__, err);
            break;
        }

        *pubDer = (uint8_t *)malloc(pubKey.data_size);
        if (*pubDer == NULL) {
            LOG_Error("[%s] malloc failed", __func__);
            break;
        }
        memcpy(*pubDer, pubKey.data, pubKey.data_size);
        *pubDerLen = pubKey.data_size;

        LOG_Info("[%s] Public key exported, len=%u", __func__, *pubDerLen);
        ret = 0;
    } while (0);

    if (pubKey.data) {
        ql_km_destroy_blob(&pubKey);
    }

    return ret;
}

/* ql_km_operation_finish 为变参函数：
 * - sign/crypto: 第 4 个参数为 ql_km_blob_t *output
 * - verify:      第 4 个参数为 NULL，第 5/6 个参数为 signature/sig_size */
static int SrHalTeeRsaOperation(const uint8_t *keyBlob, uint32_t keyBlobLen,
                                ql_km_purpose_t purpose, uint8_t padding, uint8_t digest,
                                const uint8_t *in, uint32_t inLen,
                                uint8_t *out, uint32_t *outLen,
                                const uint8_t *sig, uint32_t sigLen)
{
    int ret = -1;
    ql_tee_error_t err;
    ql_km_key_t key = { .key_blob = (uint8_t *)keyBlob, .key_blob_size = keyBlobLen };
    ql_km_operation_args_t opArgs = {0};
    ql_km_operation_handle_t opHandle = {0};
    ql_km_blob_t output = {0};
    ql_km_blob_t signature = {0};
    int qPadding = 0;
    int qDigest = 0;

    do {
        if (keyBlob == NULL || keyBlobLen == 0 || in == NULL || out == NULL || outLen == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        qPadding = QlTeeMapPadding(padding, purpose);
        qDigest = QlTeeMapDigest(digest);
        if (qPadding < 0 || qDigest < 0) {
            LOG_Error("[%s] Invalid padding=%u or digest=%u", __func__, padding, digest);
            break;
        }

        err = ql_rsa_operation_args(purpose, qPadding, qDigest, &opArgs);
        if (err != QL_TEE_OK) {
            LOG_Error("[%s] ql_rsa_operation_args failed: %d", __func__, err);
            break;
        }

        err = ql_km_operation_begin(&key, &opArgs, &opHandle);
        if (err != QL_TEE_OK) {
            LOG_Error("[%s] ql_km_operation_begin failed: %d", __func__, err);
            break;
        }

        if (purpose == QL_KM_PURPOSE_SIGN) {
            /* 让 TEE 自行分配输出缓冲区，避免预分配时 size 被清零 */
            signature.data = NULL;
            signature.data_size = 0;
            err = ql_km_operation_finish(&opHandle, (uint8_t *)in, inLen, &signature);
            if (err != QL_TEE_OK) {
                LOG_Error("[%s] ql_km_operation_finish sign failed: %d", __func__, err);
                break;
            }
            if (signature.data == NULL || signature.data_size == 0) {
                LOG_Error("[%s] sign output is empty", __func__);
                break;
            }
            if (signature.data_size > *outLen) {
                LOG_Error("[%s] sign output too large: %u > %u", __func__, signature.data_size, *outLen);
                break;
            }
            memcpy(out, signature.data, signature.data_size);
            *outLen = signature.data_size;
            LOG_Info("[%s] sign result len=%u, head=%02X%02X%02X%02X", __func__,
                     *outLen, out[0], out[1], out[2], out[3]);
        } else if (purpose == QL_KM_PURPOSE_VERIFY) {
            if (sig == NULL || sigLen == 0) {
                LOG_Error("[%s] Invalid signature for verify", __func__);
                break;
            }
            err = ql_km_operation_finish(&opHandle, (uint8_t *)in, inLen, NULL, sig, sigLen);
            if (err != QL_TEE_OK) {
                LOG_Error("[%s] ql_km_operation_finish verify failed: %d", __func__, err);
                if (err == QL_KM_ERROR_VERIFICATION_FAILED) {
                    ret = 1; /* signature mismatch */
                }
                break;
            }
        } else {
            /* encrypt / decrypt */
            output.data = NULL;
            output.data_size = 0;
            err = ql_km_operation_finish(&opHandle, (uint8_t *)in, inLen, &output);
            if (err != QL_TEE_OK) {
                LOG_Error("[%s] ql_km_operation_finish crypto failed: %d", __func__, err);
                break;
            }
            if (output.data == NULL || output.data_size == 0) {
                LOG_Error("[%s] crypto output is empty", __func__);
                break;
            }
            if (output.data_size > *outLen) {
                LOG_Error("[%s] crypto output too large: %u > %u", __func__, output.data_size, *outLen);
                break;
            }
            memcpy(out, output.data, output.data_size);
            *outLen = output.data_size;
            LOG_Info("[%s] crypto result len=%u, head=%02X%02X%02X%02X", __func__,
                     *outLen, out[0], out[1], out[2], out[3]);
        }

        ret = 0;
    } while (0);

    /* 统一释放 TEE 分配的 blob（含成功和异常路径），避免泄漏 */
    if (signature.data != NULL) {
        ql_km_destroy_blob(&signature);
    }
    if (output.data != NULL) {
        ql_km_destroy_blob(&output);
    }

    return ret;
}

int SrHalTeeRsaSign(const uint8_t *keyBlob, uint32_t keyBlobLen,
                    uint8_t padding, uint8_t digest,
                    const uint8_t *data, uint32_t dataLen,
                    uint8_t *sig, uint32_t *sigLen)
{
    return SrHalTeeRsaOperation(keyBlob, keyBlobLen, QL_KM_PURPOSE_SIGN, padding, digest,
                                data, dataLen, sig, sigLen, NULL, 0);
}

int SrHalTeeRsaVerify(const uint8_t *keyBlob, uint32_t keyBlobLen,
                      uint8_t padding, uint8_t digest,
                      const uint8_t *data, uint32_t dataLen,
                      const uint8_t *sig, uint32_t sigLen)
{
    uint32_t dummy = 0;
    return SrHalTeeRsaOperation(keyBlob, keyBlobLen, QL_KM_PURPOSE_VERIFY, padding, digest,
                                data, dataLen, (uint8_t *)&dummy, &dummy, sig, sigLen);
}

int SrHalTeeRsaEncrypt(const uint8_t *keyBlob, uint32_t keyBlobLen,
                       uint8_t padding, uint8_t digest,
                       const uint8_t *in, uint32_t inLen,
                       uint8_t *out, uint32_t *outLen)
{
    return SrHalTeeRsaOperation(keyBlob, keyBlobLen, QL_KM_PURPOSE_ENCRYPT, padding, digest,
                                in, inLen, out, outLen, NULL, 0);
}

int SrHalTeeRsaDecrypt(const uint8_t *keyBlob, uint32_t keyBlobLen,
                       uint8_t padding, uint8_t digest,
                       const uint8_t *in, uint32_t inLen,
                       uint8_t *out, uint32_t *outLen)
{
    return SrHalTeeRsaOperation(keyBlob, keyBlobLen, QL_KM_PURPOSE_DECRYPT, padding, digest,
                                in, inLen, out, outLen, NULL, 0);
}

int SrHalTeeSsInit(void)
{
    int ret = -1;
    ql_tee_error_t err;

    do {
        if (qlTeeSsInited) {
            ret = 0;
            break;
        }

        err = ql_ss_initialize();
        if (err != QL_TEE_OK) {
            LOG_Error("[%s] ql_ss_initialize failed: %d", __func__, err);
            break;
        }

        qlTeeSsInited = 1;
        LOG_Info("[%s] TEE Secure Storage initialized", __func__);
        ret = 0;
    } while (0);

    return ret;
}

void SrHalTeeSsDeinit(void)
{
    if (qlTeeSsInited) {
        ql_ss_deinitialize();
        qlTeeSsInited = 0;
        LOG_Info("[%s] TEE Secure Storage released", __func__);
    }
}

int SrHalTeeSsWrite(const char *id, const uint8_t *data, uint32_t len)
{
    int ret = -1;
    ql_tee_error_t err;
    uint32_t obj = 0;
    int objOpened = 0;

    do {
        if (id == NULL || data == NULL || len == 0) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        /* 先尝试打开已存在对象，避免 ql_ss_create overwrite 可能触发的 TEE 状态异常 */
        err = ql_ss_open(id, strlen(id), &obj);
        if (err == QL_TEE_OK) {
            objOpened = 1;
            err = ql_ss_trunc(obj, len);
            if (err != QL_TEE_OK) {
                LOG_Error("[%s] ql_ss_trunc failed: %d", __func__, err);
                break;
            }
            err = ql_ss_write(obj, (void *)data, len);
            if (err != QL_TEE_OK) {
                LOG_Error("[%s] ql_ss_write failed: %d", __func__, err);
                break;
            }
        } else if (err == QL_SS_ERROR_ITEM_NOT_FOUND) {
            err = ql_ss_create(id, strlen(id), (void *)data, len, &obj, false);
            if (err != QL_TEE_OK) {
                LOG_Error("[%s] ql_ss_create failed: %d", __func__, err);
                break;
            }
            ql_ss_close(obj);
        } else {
            LOG_Error("[%s] ql_ss_open failed: %d", __func__, err);
            break;
        }

        ret = 0;
    } while (0);

    if (objOpened) {
        (void)ql_ss_close(obj);
    }

    return ret;
}

int SrHalTeeSsRead(const char *id, uint8_t *data, uint32_t maxLen, uint32_t *outLen)
{
    int ret = -1;
    ql_tee_error_t err;
    uint32_t obj = 0;

    do {
        if (id == NULL || data == NULL || outLen == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        err = ql_ss_open(id, strlen(id), &obj);
        if (err != QL_TEE_OK) {
            if (err == QL_SS_ERROR_ITEM_NOT_FOUND) {
                LOG_Debug("[%s] ql_ss_open item not found: %s", __func__, id);
            } else {
                LOG_Error("[%s] ql_ss_open failed: %d", __func__, err);
            }
            break;
        }

        err = ql_ss_read(obj, data, maxLen, outLen);
        if (err != QL_TEE_OK) {
            LOG_Error("[%s] ql_ss_read failed: %d", __func__, err);
            ql_ss_close(obj);
            break;
        }

        ql_ss_close(obj);
        ret = 0;
    } while (0);

    return ret;
}

int SrHalTeeSsDelete(const char *id)
{
    int ret = -1;
    ql_tee_error_t err;
    uint32_t obj = 0;
    int objOpened = 0;

    do {
        if (id == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        err = ql_ss_open(id, strlen(id), &obj);
        if (err != QL_TEE_OK) {
            if (err == QL_SS_ERROR_ITEM_NOT_FOUND) {
                /* idempotent delete */
                ret = 0;
            } else {
                LOG_Error("[%s] ql_ss_open failed: %d", __func__, err);
            }
            break;
        }
        objOpened = 1;

        /* 当前模组 TEE 的 ql_ss_unlink 会触发 TEE SS target 异常（-2000），
         * 导致后续所有 SS 操作失败。临时跳过物理删除，依赖索引管理。
         * 跳过删除会造成 Secure Storage 中的对象残留（空间泄漏），
         * 需待移远修复 ql_ss_unlink 后恢复真实删除。
         * TODO: 恢复 ql_ss_unlink 物理删除。 */
        (void)ql_ss_close(obj);
        LOG_Warn("[%s] SS object deletion skipped to avoid TEE target death: %s", __func__, id);
        ret = 0;
    } while (0);

    return ret;
}
