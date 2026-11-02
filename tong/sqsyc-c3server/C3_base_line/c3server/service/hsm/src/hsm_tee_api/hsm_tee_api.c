#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bn.h>
#include <openssl/x509.h>
#include <openssl/asn1.h>
#include <openssl/sha.h>
#include <openssl/obj_mac.h>

#include "hsm_api.h"
#include "sr_hal_tee.h"
#include "sr_log.h"
#include "ctx.h"
#define TEE_KEY_INDEX_ID    "sr_hsm_key_index"
#define TEE_CRT_INDEX_ID    "sr_hsm_crt_index"
#define TEE_KEY_OBJ_PREFIX  "sr_hsm_key_"
#define TEE_CRT_OBJ_PREFIX  "sr_hsm_crt_"
#define TEE_OBJ_ID_LEN      32

#define TEE_KEY_MAX_COUNT   256
#define TEE_CRT_MAX_COUNT   256

#define TEE_SS_SINGLE_OBJ_MAX_SIZE (8 * 1024)

typedef struct {
    uint16_t keyIds[TEE_KEY_MAX_COUNT];
    uint8_t keyAlg[TEE_KEY_MAX_COUNT];      /* HsmKeyAlg_e */
    uint8_t keyType[TEE_KEY_MAX_COUNT];     /* HsmKeyType_e */
    uint8_t keyRawBits[TEE_KEY_MAX_COUNT];  /* bits/32 */
    uint32_t keyCount;
    uint16_t crtIds[TEE_CRT_MAX_COUNT];
    uint32_t crtCount;
} TeeContext_t;

/* 前向声明 */
static int TeeKeyToKeyInfo(uint8_t keyType, uint8_t algo, KeyInfo_t *keyInfo);

/* ============================================================
 * 工具函数
 * ============================================================ */

static void TeeContextClear(TeeContext_t* teeCtx)
{
    if (teeCtx != NULL) {
        memset(teeCtx, 0, sizeof(TeeContext_t));
    }
}

/* ID index management */
static int TeeFindKeyId(TeeContext_t* teeCtx, uint16_t keyId)
{
    uint32_t i = 0;
    for (i = 0; i < teeCtx->keyCount; i++) {
        if (teeCtx->keyIds[i] == keyId) {
            return (int)i;
        }
    }
    return -1;
}

static int TeeAddKeyId(TeeContext_t* teeCtx, uint16_t keyId, uint8_t keyType, uint8_t algo)
{
    KeyInfo_t keyInfo = {0};

    if (teeCtx->keyCount >= TEE_KEY_MAX_COUNT) {
        LOG_Error("[%s] key index full", __func__);
        return -1;
    }

    if (TeeFindKeyId(teeCtx, keyId) >= 0) {
        /* 已存在则无需重复添加 */
        return 0;
    }

    if (TeeKeyToKeyInfo(keyType, algo, &keyInfo) != 0) {
        LOG_Error("[%s] invalid key type or algo, keyId=0x%04X", __func__, keyId);
        return -1;
    }

    teeCtx->keyIds[teeCtx->keyCount] = keyId;
    teeCtx->keyAlg[teeCtx->keyCount] = (uint8_t)keyInfo.keyAlg;
    teeCtx->keyType[teeCtx->keyCount] = (uint8_t)keyInfo.keyType;
    teeCtx->keyRawBits[teeCtx->keyCount] = (uint8_t)(keyInfo.keyBits / 32);
    teeCtx->keyCount++;
    return 0;
}

static int TeeRemoveKeyId(TeeContext_t* teeCtx, uint16_t keyId)
{
    int idx = TeeFindKeyId(teeCtx, keyId);
    uint32_t remain = 0;

    if (idx < 0) {
        return -1;
    }

    remain = teeCtx->keyCount - 1 - (uint32_t)idx;
    if (remain > 0) {
        memmove(&teeCtx->keyIds[idx], &teeCtx->keyIds[idx + 1], sizeof(uint16_t) * remain);
        memmove(&teeCtx->keyAlg[idx], &teeCtx->keyAlg[idx + 1], sizeof(uint8_t) * remain);
        memmove(&teeCtx->keyType[idx], &teeCtx->keyType[idx + 1], sizeof(uint8_t) * remain);
        memmove(&teeCtx->keyRawBits[idx], &teeCtx->keyRawBits[idx + 1], sizeof(uint8_t) * remain);
    }
    teeCtx->keyCount--;
    return 0;
}

static int TeeFindCrtId(TeeContext_t* teeCtx, uint16_t crtId)
{
    uint32_t i = 0;
    for (i = 0; i < teeCtx->crtCount; i++) {
        if (teeCtx->crtIds[i] == crtId) {
            return (int)i;
        }
    }
    return -1;
}

static int TeeAddCrtId(TeeContext_t* teeCtx, uint16_t crtId)
{
    if (teeCtx->crtCount >= TEE_CRT_MAX_COUNT) {
        LOG_Error("[%s] crt index full", __func__);
        return -1;
    }

    if (TeeFindCrtId(teeCtx, crtId) >= 0) {
        return 0;
    }

    teeCtx->crtIds[teeCtx->crtCount++] = crtId;
    return 0;
}

static int TeeRemoveCrtId(TeeContext_t* teeCtx, uint16_t crtId)
{
    int idx = TeeFindCrtId(teeCtx, crtId);
    if (idx < 0) {
        return -1;
    }

    if ((uint32_t)idx < teeCtx->crtCount - 1) {
        memmove(&teeCtx->crtIds[idx], &teeCtx->crtIds[idx + 1],
                sizeof(uint16_t) * (teeCtx->crtCount - 1 - (uint32_t)idx));
    }
    teeCtx->crtCount--;
    return 0;
}

/* ============================================================
 * 独立 Secure Storage 对象读写
 * ============================================================ */

static void TeeFormatKeyObjId(uint16_t keyId, char *buf, uint32_t bufSize)
{
    snprintf(buf, bufSize, "%s%04X", TEE_KEY_OBJ_PREFIX, keyId);
}

static void TeeFormatCrtObjId(uint16_t crtId, char *buf, uint32_t bufSize)
{
    snprintf(buf, bufSize, "%s%04X", TEE_CRT_OBJ_PREFIX, crtId);
}

static int TeeReadKeyObject(uint16_t keyId, uint8_t* keyType, uint8_t* algo, uint8_t** blob, uint32_t* blobLen)
{
    int ret = -1;
    uint8_t buf[TEE_SS_SINGLE_OBJ_MAX_SIZE] = {0};
    uint32_t len = 0;
    uint32_t offset = 0;
    uint16_t readKeyId = 0;
    uint8_t localKeyType = 0;
    uint8_t localAlgo = 0;
    uint32_t localBlobLen = 0;
    char objId[TEE_OBJ_ID_LEN] = {0};

    do {
        if (keyType == NULL || algo == NULL || blob == NULL || blobLen == NULL) {
            LOG_Error("[%s] invalid parameters", __func__);
            break;
        }
        *keyType = 0;
        *algo = 0;
        *blob = NULL;
        *blobLen = 0;

        TeeFormatKeyObjId(keyId, objId, sizeof(objId));
        if (SrHalTeeSsRead(objId, buf, TEE_SS_SINGLE_OBJ_MAX_SIZE, &len) != 0) {
            LOG_Error("[%s] read key obj failed: %s", __func__, objId);
            break;
        }

        if (len < sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t)) {
            LOG_Error("[%s] key obj format error: %s", __func__, objId);
            break;
        }

        offset = 0;
        readKeyId = (buf[offset] << 8) | buf[offset + 1];
        offset += 2;
        localKeyType = buf[offset++];
        localAlgo = buf[offset++];
        localBlobLen = (buf[offset] << 24) | (buf[offset + 1] << 16) | (buf[offset + 2] << 8) | buf[offset + 3];
        offset += 4;

        if (readKeyId != keyId) {
            LOG_Error("[%s] keyId mismatch, expect=0x%04X, got=0x%04X", __func__, keyId, readKeyId);
            break;
        }
        if (localKeyType != E_KEY_TYPE_PUB && localKeyType != E_KEY_TYPE_PRI && localKeyType != E_KEY_TYPE_PRI_PUB_PAIR) {
            LOG_Error("[%s] invalid keyType: %u", __func__, localKeyType);
            break;
        }
        if (offset + localBlobLen > len) {
            LOG_Error("[%s] key blob truncated", __func__);
            break;
        }

        *blob = (uint8_t *)malloc(localBlobLen);
        if (*blob == NULL) {
            LOG_Error("[%s] malloc blob failed", __func__);
            break;
        }
        memcpy(*blob, &buf[offset], localBlobLen);
        *blobLen = localBlobLen;
        *keyType = localKeyType;
        *algo = localAlgo;

        ret = 0;
    } while (0);

    if (ret != 0 && blob != NULL && *blob != NULL) {
        free(*blob);
        *blob = NULL;
        *blobLen = 0;
    }
    return ret;
}

static int TeeReadKeyHeader(uint16_t keyId, uint8_t* keyType, uint8_t* algo, uint32_t* blobLen)
{
    int ret = -1;
    uint8_t buf[TEE_SS_SINGLE_OBJ_MAX_SIZE] = {0};
    uint32_t len = 0;
    uint32_t offset = 0;
    uint16_t readKeyId = 0;
    uint8_t localKeyType = 0;
    uint8_t localAlgo = 0;
    uint32_t localBlobLen = 0;
    char objId[TEE_OBJ_ID_LEN] = {0};

    do {
        if (keyType == NULL || algo == NULL || blobLen == NULL) {
            LOG_Error("[%s] invalid parameters", __func__);
            break;
        }
        *keyType = 0;
        *algo = 0;
        *blobLen = 0;

        TeeFormatKeyObjId(keyId, objId, sizeof(objId));
        if (SrHalTeeSsRead(objId, buf, TEE_SS_SINGLE_OBJ_MAX_SIZE, &len) != 0) {
            LOG_Error("[%s] read key obj failed: %s", __func__, objId);
            break;
        }

        if (len < sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t)) {
            LOG_Error("[%s] key obj format error: %s", __func__, objId);
            break;
        }

        offset = 0;
        readKeyId = (buf[offset] << 8) | buf[offset + 1];
        offset += 2;
        localKeyType = buf[offset++];
        localAlgo = buf[offset++];
        localBlobLen = (buf[offset] << 24) | (buf[offset + 1] << 16) | (buf[offset + 2] << 8) | buf[offset + 3];
        offset += 4;

        if (readKeyId != keyId) {
            LOG_Error("[%s] keyId mismatch, expect=0x%04X, got=0x%04X", __func__, keyId, readKeyId);
            break;
        }
        if (localKeyType != E_KEY_TYPE_PUB && localKeyType != E_KEY_TYPE_PRI && localKeyType != E_KEY_TYPE_PRI_PUB_PAIR) {
            LOG_Error("[%s] invalid keyType: %u", __func__, localKeyType);
            break;
        }
        if (offset + localBlobLen > len) {
            LOG_Error("[%s] key blob truncated", __func__);
            break;
        }

        *keyType = localKeyType;
        *algo = localAlgo;
        *blobLen = localBlobLen;

        ret = 0;
    } while (0);

    return ret;
}

static int TeeWriteKeyObject(uint16_t keyId, uint8_t keyType, uint8_t algo, const uint8_t* blob, uint32_t blobLen)
{
    int ret = -1;
    uint8_t buf[TEE_SS_SINGLE_OBJ_MAX_SIZE] = {0};
    uint32_t size = 0;
    uint32_t offset = 0;
    char objId[TEE_OBJ_ID_LEN] = {0};

    do {
        if (blob == NULL || blobLen == 0) {
            LOG_Error("[%s] invalid parameters", __func__);
            break;
        }

        size = sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t) + blobLen;
        if (size > TEE_SS_SINGLE_OBJ_MAX_SIZE) {
            LOG_Error("[%s] key object too large: %u", __func__, size);
            break;
        }

        offset = 0;
        buf[offset++] = (keyId >> 8) & 0xFF;
        buf[offset++] = keyId & 0xFF;
        buf[offset++] = keyType;
        buf[offset++] = algo;
        buf[offset++] = (blobLen >> 24) & 0xFF;
        buf[offset++] = (blobLen >> 16) & 0xFF;
        buf[offset++] = (blobLen >> 8) & 0xFF;
        buf[offset++] = blobLen & 0xFF;
        memcpy(&buf[offset], blob, blobLen);

        TeeFormatKeyObjId(keyId, objId, sizeof(objId));
        if (SrHalTeeSsWrite(objId, buf, size) != 0) {
            LOG_Error("[%s] write key obj failed: %s", __func__, objId);
            break;
        }

        ret = 0;
    } while (0);

    return ret;
}

static int TeeDeleteKeyObject(uint16_t keyId)
{
    char objId[TEE_OBJ_ID_LEN] = {0};
    TeeFormatKeyObjId(keyId, objId, sizeof(objId));
    return SrHalTeeSsDelete(objId);
}

static int TeeReadCrtObject(uint16_t crtId, uint8_t** cert, uint32_t* certLen)
{
    int ret = -1;
    uint8_t buf[TEE_SS_SINGLE_OBJ_MAX_SIZE] = {0};
    uint32_t len = 0;
    uint32_t offset = 0;
    uint16_t readCrtId = 0;
    uint32_t localCertLen = 0;
    char objId[TEE_OBJ_ID_LEN] = {0};

    do {
        if (cert == NULL || certLen == NULL) {
            LOG_Error("[%s] invalid parameters", __func__);
            break;
        }
        *cert = NULL;
        *certLen = 0;

        TeeFormatCrtObjId(crtId, objId, sizeof(objId));
        if (SrHalTeeSsRead(objId, buf, TEE_SS_SINGLE_OBJ_MAX_SIZE, &len) != 0) {
            LOG_Error("[%s] read crt obj failed: %s", __func__, objId);
            break;
        }

        if (len < sizeof(uint16_t) + sizeof(uint32_t)) {
            LOG_Error("[%s] crt obj format error: %s", __func__, objId);
            break;
        }

        offset = 0;
        readCrtId = (buf[offset] << 8) | buf[offset + 1];
        offset += 2;
        localCertLen = (buf[offset] << 24) | (buf[offset + 1] << 16) | (buf[offset + 2] << 8) | buf[offset + 3];
        offset += 4;

        if (readCrtId != crtId) {
            LOG_Error("[%s] crtId mismatch, expect=0x%04X, got=0x%04X", __func__, crtId, readCrtId);
            break;
        }
        if (offset + localCertLen > len) {
            LOG_Error("[%s] cert truncated", __func__);
            break;
        }

        *cert = (uint8_t *)malloc(localCertLen);
        if (*cert == NULL) {
            LOG_Error("[%s] malloc cert failed", __func__);
            break;
        }
        memcpy(*cert, &buf[offset], localCertLen);
        *certLen = localCertLen;

        ret = 0;
    } while (0);

    if (ret != 0 && cert != NULL && *cert != NULL) {
        free(*cert);
        *cert = NULL;
        *certLen = 0;
    }
    return ret;
}

static int TeeWriteCrtObject(uint16_t crtId, const uint8_t* cert, uint32_t certLen)
{
    int ret = -1;
    uint8_t buf[TEE_SS_SINGLE_OBJ_MAX_SIZE] = {0};
    uint32_t size = 0;
    uint32_t offset = 0;
    char objId[TEE_OBJ_ID_LEN] = {0};

    do {
        if (cert == NULL || certLen == 0) {
            LOG_Error("[%s] invalid parameters", __func__);
            break;
        }

        size = sizeof(uint16_t) + sizeof(uint32_t) + certLen;
        if (size > TEE_SS_SINGLE_OBJ_MAX_SIZE) {
            LOG_Error("[%s] crt object too large: %u", __func__, size);
            break;
        }

        offset = 0;
        buf[offset++] = (crtId >> 8) & 0xFF;
        buf[offset++] = crtId & 0xFF;
        buf[offset++] = (certLen >> 24) & 0xFF;
        buf[offset++] = (certLen >> 16) & 0xFF;
        buf[offset++] = (certLen >> 8) & 0xFF;
        buf[offset++] = certLen & 0xFF;
        memcpy(&buf[offset], cert, certLen);

        TeeFormatCrtObjId(crtId, objId, sizeof(objId));
        if (SrHalTeeSsWrite(objId, buf, size) != 0) {
            LOG_Error("[%s] write crt obj failed: %s", __func__, objId);
            break;
        }

        ret = 0;
    } while (0);

    return ret;
}

static int TeeDeleteCrtObject(uint16_t crtId)
{
    char objId[TEE_OBJ_ID_LEN] = {0};
    TeeFormatCrtObjId(crtId, objId, sizeof(objId));
    return SrHalTeeSsDelete(objId);
}

/* ============================================================
 * 索引持久化（每个 key/crt 独立 Secure Storage 对象）
 * ============================================================ */

static int TeeReadIdIndex(const char *indexId, uint16_t *ids, uint32_t maxCount, uint32_t *outCount)
{
    int ret = -1;
    uint8_t buf[TEE_SS_SINGLE_OBJ_MAX_SIZE] = {0};
    uint32_t len = 0;
    uint32_t offset = 0;
    uint32_t count = 0;
    uint32_t i = 0;

    do {
        if (indexId == NULL || ids == NULL || outCount == NULL) {
            LOG_Error("[%s] invalid parameters", __func__);
            break;
        }
        *outCount = 0;

        if (SrHalTeeSsRead(indexId, buf, TEE_SS_SINGLE_OBJ_MAX_SIZE, &len) != 0) {
            /* 首次运行可能不存在，视为空 */
            ret = 0;
            break;
        }

        if (len < sizeof(uint32_t)) {
            LOG_Error("[%s] index format error", __func__);
            break;
        }

        count = (buf[offset] << 24) | (buf[offset + 1] << 16) | (buf[offset + 2] << 8) | buf[offset + 3];
        offset += 4;

        if (count > maxCount) {
            LOG_Error("[%s] index count error: %u", __func__, count);
            break;
        }

        if (len < sizeof(uint32_t) + count * sizeof(uint16_t)) {
            LOG_Error("[%s] index truncated", __func__);
            break;
        }

        for (i = 0; i < count; i++) {
            ids[i] = (buf[offset] << 8) | buf[offset + 1];
            offset += 2;
        }
        *outCount = count;
        ret = 0;
    } while (0);

    return ret;
}

static int TeeWriteIdIndex(const char *indexId, const uint16_t *ids, uint32_t count)
{
    int ret = -1;
    uint8_t buf[TEE_SS_SINGLE_OBJ_MAX_SIZE] = {0};
    uint32_t size = 0;
    uint32_t offset = 0;
    uint32_t i = 0;

    do {
        if (indexId == NULL || (ids == NULL && count > 0)) {
            LOG_Error("[%s] invalid parameters", __func__);
            break;
        }

        size = sizeof(uint32_t) + count * sizeof(uint16_t);
        if (size > TEE_SS_SINGLE_OBJ_MAX_SIZE) {
            LOG_Error("[%s] index object too large: %u", __func__, size);
            break;
        }

        offset = 0;
        buf[offset++] = (count >> 24) & 0xFF;
        buf[offset++] = (count >> 16) & 0xFF;
        buf[offset++] = (count >> 8) & 0xFF;
        buf[offset++] = count & 0xFF;

        for (i = 0; i < count; i++) {
            buf[offset++] = (ids[i] >> 8) & 0xFF;
            buf[offset++] = ids[i] & 0xFF;
        }

        if (SrHalTeeSsWrite(indexId, buf, size) != 0) {
            LOG_Error("[%s] save index failed: %s", __func__, indexId);
            break;
        }

        ret = 0;
    } while (0);

    return ret;
}

static int TeeLoadKeyIndex(TeeContext_t* teeCtx)
{
    return TeeReadIdIndex(TEE_KEY_INDEX_ID, teeCtx->keyIds, TEE_KEY_MAX_COUNT, &teeCtx->keyCount);
}

static int TeeSaveKeyIndex(TeeContext_t* teeCtx)
{
    int ret = -1;
    uint16_t oldIds[TEE_KEY_MAX_COUNT] = {0};
    uint32_t oldCount = 0;
    uint32_t i = 0;
    uint32_t j = 0;
    int found = 0;

    do {
        /* 读取旧索引，用于后续清理已删除对象的独立 SS 对象 */
        (void)TeeReadIdIndex(TEE_KEY_INDEX_ID, oldIds, TEE_KEY_MAX_COUNT, &oldCount);

        if (TeeWriteIdIndex(TEE_KEY_INDEX_ID, teeCtx->keyIds, teeCtx->keyCount) != 0) {
            LOG_Error("[%s] write key index failed", __func__);
            break;
        }

        /* 清理旧索引中有、新索引中无的 key 对象 */
        for (i = 0; i < oldCount; i++) {
            found = 0;
            for (j = 0; j < teeCtx->keyCount; j++) {
                if (oldIds[i] == teeCtx->keyIds[j]) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                (void)TeeDeleteKeyObject(oldIds[i]);
            }
        }

        LOG_Info("[%s] key index saved, count=%u", __func__, teeCtx->keyCount);
        ret = 0;
    } while (0);

    return ret;
}

static int TeeLoadCrtIndex(TeeContext_t* teeCtx)
{
    return TeeReadIdIndex(TEE_CRT_INDEX_ID, teeCtx->crtIds, TEE_CRT_MAX_COUNT, &teeCtx->crtCount);
}

static int TeeSaveCrtIndex(TeeContext_t* teeCtx)
{
    int ret = -1;
    uint16_t oldIds[TEE_CRT_MAX_COUNT] = {0};
    uint32_t oldCount = 0;
    uint32_t i = 0;
    uint32_t j = 0;
    int found = 0;

    do {
        (void)TeeReadIdIndex(TEE_CRT_INDEX_ID, oldIds, TEE_CRT_MAX_COUNT, &oldCount);

        if (TeeWriteIdIndex(TEE_CRT_INDEX_ID, teeCtx->crtIds, teeCtx->crtCount) != 0) {
            LOG_Error("[%s] write crt index failed", __func__);
            break;
        }

        for (i = 0; i < oldCount; i++) {
            found = 0;
            for (j = 0; j < teeCtx->crtCount; j++) {
                if (oldIds[i] == teeCtx->crtIds[j]) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                (void)TeeDeleteCrtObject(oldIds[i]);
            }
        }

        LOG_Info("[%s] crt index saved, count=%u", __func__, teeCtx->crtCount);
        ret = 0;
    } while (0);

    return ret;
}

/* 初始化时遍历 key 索引，把每把 key 的 query 元数据缓存到内存，避免 query 时再读 SS */
static int TeeLoadKeyMeta(TeeContext_t* teeCtx)
{
    uint32_t i = 0;
    uint16_t keyId = 0;
    uint8_t keyType = 0;
    uint8_t algo = 0;
    uint32_t blobLen = 0;
    KeyInfo_t keyInfo = {0};
    int ret = 0;

    for (i = 0; i < teeCtx->keyCount; i++) {
        keyId = teeCtx->keyIds[i];
        if (TeeReadKeyHeader(keyId, &keyType, &algo, &blobLen) != 0) {
            LOG_Warn("[%s] load key meta failed: 0x%04X, skip", __func__, keyId);
            teeCtx->keyAlg[i] = 0xFF;
            teeCtx->keyType[i] = 0xFF;
            teeCtx->keyRawBits[i] = 0xFF;
            ret = -1;
            continue;
        }

        if (TeeKeyToKeyInfo(keyType, algo, &keyInfo) != 0) {
            LOG_Warn("[%s] invalid key meta: 0x%04X, type=%u, algo=%u", __func__, keyId, keyType, algo);
            teeCtx->keyAlg[i] = 0xFF;
            teeCtx->keyType[i] = 0xFF;
            teeCtx->keyRawBits[i] = 0xFF;
            ret = -1;
            continue;
        }

        teeCtx->keyAlg[i] = (uint8_t)keyInfo.keyAlg;
        teeCtx->keyType[i] = (uint8_t)keyInfo.keyType;
        teeCtx->keyRawBits[i] = (uint8_t)(keyInfo.keyBits / 32);
    }

    return ret;
}

static int TeeDigestToNid(DigistAlg_e digist)
{
    switch (digist) {
        case E_DIGIST_SHA1:   return NID_sha1;
        case E_DIGIST_SHA224: return NID_sha224;
        case E_DIGIST_SHA256: return NID_sha256;
        case E_DIGIST_SHA384: return NID_sha384;
        case E_DIGIST_SHA512: return NID_sha512;
        default:
            return -1;
    }
}

static const EVP_MD* TeeDigestToEvpMd(DigistAlg_e digist)
{
    int nid = TeeDigestToNid(digist);
    if (nid < 0) {
        return NULL;
    }
    return EVP_get_digestbynid(nid);
}


static void* HsmTeeContextCreate(void)
{
    TeeContext_t* teeCtx = (TeeContext_t*)calloc(1, sizeof(TeeContext_t));
    return (void*)teeCtx;
}

static void HsmTeeContextDestroy(void* ctx)
{
    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx != NULL) {
        TeeContextClear(teeCtx);
        free(teeCtx);
    }
}

static int TeeKeyToKeyInfo(uint8_t keyType, uint8_t algo, KeyInfo_t *keyInfo)
{
    if (keyInfo == NULL) {
        return -1;
    }

    keyInfo->keyAlg = E_HSM_KEY_ALG_RSA;

    /* 内部 algo 仅用于标识 RSA 位数 */
    switch (algo) {
        case 0x80: /* RSA1024 */
            keyInfo->keyBits = 1024;
            break;
        case 0x81: /* RSA2048 */
            keyInfo->keyBits = 2048;
            break;
        default:
            return -1;
    }

    /* 根据 keyType 返回密钥类型 */
    if (keyType == E_KEY_TYPE_PUB) {
        keyInfo->keyType = E_HSM_KEY_TYPE_PUBLIC;
    } else if (keyType == E_KEY_TYPE_PRI) {
        keyInfo->keyType = E_HSM_KEY_TYPE_PRIVATE;
    } else if (keyType == E_KEY_TYPE_PRI_PUB_PAIR) {
        keyInfo->keyType = E_HSM_KEY_TYPE_KEY_PAIR;
    } else {
        return -1;
    }

    return 0;
}

/*****************************************************/

static int hsmTeeInit(void* ctx)
{
    int ret = -1;
    Ctx_t* appCtx = (Ctx_t*)ctx;
    TeeContext_t* teeCtx = NULL;

    if (appCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    LOG_Warn("[%s] enter", __func__);

    do {
        teeCtx = (TeeContext_t*)HsmTeeContextCreate();
        if (teeCtx == NULL) {
            LOG_Error("[%s] allocate tee context failed", __func__);
            break;
        }

        if (SrHalTeeInit() != 0) {
            break;
        }
        if (SrHalTeeSsInit() != 0) {
            break;
        }

        if (TeeLoadKeyIndex(teeCtx) != 0) {
            break;
        }
        if (TeeLoadCrtIndex(teeCtx) != 0) {
            break;
        }
        (void)TeeLoadKeyMeta(teeCtx);

        appCtx->hsmUserData = teeCtx;
        LOG_Info("[%s] TEE HSM initialized, keys=%u, crts=%u", __func__, teeCtx->keyCount, teeCtx->crtCount);
        ret = 0;
    } while (0);

    if (ret != 0 && teeCtx != NULL) {
        HsmTeeContextDestroy(teeCtx);
        appCtx->hsmUserData = NULL;
    }

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmTeeRelease(void* ctx)
{
    Ctx_t* appCtx = (Ctx_t*)ctx;
    TeeContext_t* teeCtx = NULL;

    if (appCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    LOG_Warn("[%s] enter", __func__);

    teeCtx = (TeeContext_t*)appCtx->hsmUserData;
    if (teeCtx != NULL) {
        TeeContextClear(teeCtx);
        SrHalTeeSsDeinit();
        SrHalTeeRelease();
        HsmTeeContextDestroy(teeCtx);
        appCtx->hsmUserData = NULL;
    }

    LOG_Warn("[%s] exit(0)", __func__);
    return 0;
}

static int hsmTeeRandom(void* ctx, uint32_t expectedLen, uint8_t *random)
{
    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    return SrHalTeeRandom(expectedLen, random);
}

static int hsmTeeRsaKeyPairGenerate(void* ctx, RsaKeyAlg_e alg, uint16_t keyId, uint8_t* pubKeyDer, uint32_t *pubKeyDerBufSize)
{
    int ret = -1;
    uint32_t bits = 0;
    uint8_t *teePubDer = NULL;
    uint32_t teePubDerLen = 0;
    uint8_t *keyBlob = NULL;
    uint32_t keyBlobLen = 0;
    uint8_t algo = 0;
    int idAdded = 0;

    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    LOG_Warn("[%s] enter", __func__);

    do {
        switch (alg) {
            case E_ALG_RSA1024: bits = 1024; algo = 0x80; break;
            case E_ALG_RSA2048: bits = 2048; algo = 0x81; break;
            default:
                LOG_Error("[%s] Unsupported algorithm: %d", __func__, alg);
                break;
        }
        if (bits == 0) {
            break;
        }

        if (SrHalTeeRsaGenKey(bits, &teePubDer, &teePubDerLen, &keyBlob, &keyBlobLen) != 0) {
            break;
        }

        if (teePubDerLen > *pubKeyDerBufSize) {
            LOG_Error("[%s] buffer too small: %u < %u", __func__, *pubKeyDerBufSize, teePubDerLen);
            *pubKeyDerBufSize = teePubDerLen;
            ret = -1;
            break;
        }
        memcpy(pubKeyDer, teePubDer, teePubDerLen);
        *pubKeyDerBufSize = teePubDerLen;

        if (TeeWriteKeyObject(keyId, E_KEY_TYPE_PRI_PUB_PAIR, algo, keyBlob, keyBlobLen) != 0) {
            LOG_Error("[%s] write key object failed", __func__);
            break;
        }

        if (TeeAddKeyId(teeCtx, keyId, E_KEY_TYPE_PRI_PUB_PAIR, algo) != 0) {
            LOG_Error("[%s] add key id failed", __func__);
            (void)TeeDeleteKeyObject(keyId);
            break;
        }
        idAdded = 1;

        if (TeeSaveKeyIndex(teeCtx) != 0) {
            LOG_Error("[%s] save key index failed", __func__);
            if (idAdded) {
                (void)TeeRemoveKeyId(teeCtx, keyId);
                (void)TeeDeleteKeyObject(keyId);
            }
            break;
        }

        LOG_Info("[%s] RSA key pair generated, keyId=0x%04X, bits=%u, pubKeyLen=%u", __func__, keyId, bits, *pubKeyDerBufSize);
        ret = 0;
    } while (0);

    if (teePubDer != NULL) {
        free(teePubDer);
    }
    if (keyBlob != NULL) {
        if (keyBlobLen > 0) {
            memset(keyBlob, 0, keyBlobLen);
        }
        free(keyBlob);
    }

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmTeeRsaSign(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, DigistAlg_e digist,
                         uint8_t *plain, uint32_t plainLen, uint8_t *sign, uint32_t* signBufSize)
{
    int ret = -1;
    uint8_t keyType = 0;
    uint8_t algo = 0;
    uint8_t *blob = NULL;
    uint32_t blobLen = 0;

    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    (void)alg;
    LOG_Warn("[%s] enter", __func__);

    do {
        if (TeeReadKeyObject(keyId, &keyType, &algo, &blob, &blobLen) != 0) {
            LOG_Error("[%s] read key object failed: 0x%04X", __func__, keyId);
            break;
        }

        /* TEE Keymaster 会根据 digist 对 plain 进行散列后签名，直接传入原始数据 */
        if (SrHalTeeRsaSign(blob, blobLen, E_RSA_SIGN_PADDING_PKCS1, digist,
                            plain, plainLen, sign, signBufSize) != 0) {
            break;
        }

        LOG_Info("[%s] sign ok, keyId=0x%04X, len=%u", __func__, keyId, *signBufSize);
        ret = 0;
    } while (0);

    if (blob != NULL) {
        free(blob);
    }

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmTeeRsaVerify(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, DigistAlg_e digist,
                           uint8_t *plain, uint32_t plainLen, uint8_t *sign, uint32_t signBufSize)
{
    int ret = -1;
    uint8_t keyType = 0;
    uint8_t algo = 0;
    uint8_t *blob = NULL;
    uint32_t blobLen = 0;
    EVP_PKEY *pkey = NULL;
    const uint8_t *p = NULL;
    EVP_MD_CTX *mdCtx = NULL;
    const EVP_MD *md = NULL;

    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    (void)alg;
    LOG_Warn("[%s] enter", __func__);

    do {
        if (TeeReadKeyObject(keyId, &keyType, &algo, &blob, &blobLen) != 0) {
            LOG_Error("[%s] read key object failed: 0x%04X", __func__, keyId);
            break;
        }

        if (keyType == E_KEY_TYPE_PUB) {
            /* 公钥类型仅存储 X.509 SubjectPublicKeyInfo DER，未生成 TEE key_blob，
             * 因此本地使用 EVP 验签。公钥本身可公开，不降低安全性。 */
            p = blob;
            pkey = d2i_PUBKEY(NULL, &p, blobLen);
            if (pkey == NULL) {
                LOG_Error("[%s] parse public key failed", __func__);
                break;
            }

            md = TeeDigestToEvpMd(digist);
            if (md == NULL) {
                LOG_Error("[%s] unsupported digest: %u", __func__, digist);
                break;
            }

            mdCtx = EVP_MD_CTX_new();
            if (mdCtx == NULL) {
                LOG_Error("[%s] EVP_MD_CTX_new failed", __func__);
                break;
            }

            if (EVP_DigestVerifyInit(mdCtx, NULL, md, NULL, pkey) != 1) {
                LOG_Error("[%s] EVP_DigestVerifyInit failed", __func__);
                break;
            }

            if (EVP_DigestVerifyUpdate(mdCtx, plain, plainLen) != 1) {
                LOG_Error("[%s] EVP_DigestVerifyUpdate failed", __func__);
                break;
            }

            if (EVP_DigestVerifyFinal(mdCtx, sign, signBufSize) != 1) {
                LOG_Warn("[%s] signature verify failed", __func__);
                ret = 1; /* signature mismatch */
                break;
            }
        } else {
            /* TEE Keymaster 会根据 digist 对 plain 进行散列后验签，直接传入原始数据 */
            int verifyRet = SrHalTeeRsaVerify(blob, blobLen, E_RSA_SIGN_PADDING_PKCS1, digist,
                                              plain, plainLen, sign, signBufSize);
            if (verifyRet != 0) {
                ret = (verifyRet == 1) ? 1 : -1;
                break;
            }
        }

        LOG_Info("[%s] verify ok, keyId=0x%04X", __func__, keyId);
        ret = 0;
    } while (0);

    if (blob != NULL) {
        free(blob);
    }
    if (pkey != NULL) {
        EVP_PKEY_free(pkey);
    }
    if (mdCtx != NULL) {
        EVP_MD_CTX_free(mdCtx);
    }

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmTeeRsaEncrypt(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, RsaSignPaddingAlg_e padding,
                            uint8_t *plain, uint32_t plainLen, uint8_t *out, uint32_t* outBufSize)
{
    int ret = -1;
    uint8_t keyType = 0;
    uint8_t algo = 0;
    uint8_t *blob = NULL;
    uint32_t blobLen = 0;
    EVP_PKEY *pkey = NULL;
    const uint8_t *p = NULL;
    size_t outLen = 0;
    EVP_PKEY_CTX *pkeyCtx = NULL;

    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    (void)alg;
    LOG_Warn("[%s] enter", __func__);

    do {
        if (TeeReadKeyObject(keyId, &keyType, &algo, &blob, &blobLen) != 0) {
            LOG_Error("[%s] read key object failed: 0x%04X", __func__, keyId);
            break;
        }

        if (keyType == E_KEY_TYPE_PUB) {
            /* 公钥类型仅存储 X.509 SubjectPublicKeyInfo DER，未生成 TEE key_blob，
             * 因此本地使用 EVP 加密。公钥本身可公开，不降低安全性。 */
            int rsaPadding = 0;
            const EVP_MD *oaepMd = NULL;

            if (padding == E_RSA_SIGN_PADDING_PKCS1) {
                rsaPadding = RSA_PKCS1_PADDING;
            } else if (padding == E_RSA_SIGN_PADDING_OAEP) {
                rsaPadding = RSA_PKCS1_OAEP_PADDING;
                oaepMd = EVP_sha256();
            } else {
                LOG_Error("[%s] unsupported padding for public key encrypt: %u", __func__, padding);
                break;
            }

            p = blob;
            pkey = d2i_PUBKEY(NULL, &p, blobLen);
            if (pkey == NULL) {
                LOG_Error("[%s] parse public key failed", __func__);
                break;
            }

            pkeyCtx = EVP_PKEY_CTX_new(pkey, NULL);
            if (pkeyCtx == NULL) {
                LOG_Error("[%s] EVP_PKEY_CTX_new failed", __func__);
                break;
            }

            if (EVP_PKEY_encrypt_init(pkeyCtx) != 1) {
                LOG_Error("[%s] EVP_PKEY_encrypt_init failed", __func__);
                break;
            }

            if (EVP_PKEY_CTX_set_rsa_padding(pkeyCtx, rsaPadding) != 1) {
                LOG_Error("[%s] EVP_PKEY_CTX_set_rsa_padding failed", __func__);
                break;
            }

            if (padding == E_RSA_SIGN_PADDING_OAEP) {
                if (EVP_PKEY_CTX_set_rsa_oaep_md(pkeyCtx, oaepMd) != 1 ||
                    EVP_PKEY_CTX_set_rsa_mgf1_md(pkeyCtx, oaepMd) != 1) {
                    LOG_Error("[%s] EVP_PKEY_CTX_set_rsa_oaep_md/mgf1_md failed", __func__);
                    break;
                }
            }

            if (EVP_PKEY_encrypt(pkeyCtx, NULL, &outLen, plain, plainLen) != 1) {
                LOG_Error("[%s] EVP_PKEY_encrypt get length failed", __func__);
                break;
            }

            if (*outBufSize < outLen) {
                LOG_Error("[%s] output buffer too small: %u < %zu", __func__, *outBufSize, outLen);
                *outBufSize = (uint32_t)outLen;
                break;
            }

            if (EVP_PKEY_encrypt(pkeyCtx, out, &outLen, plain, plainLen) != 1) {
                LOG_Error("[%s] EVP_PKEY_encrypt failed", __func__);
                break;
            }
            *outBufSize = (uint32_t)outLen;
        } else {
            if (SrHalTeeRsaEncrypt(blob, blobLen, padding, E_DIGIST_SHA256,
                                   plain, plainLen, out, outBufSize) != 0) {
                break;
            }
        }

        LOG_Info("[%s] encrypt ok, keyId=0x%04X, len=%u", __func__, keyId, *outBufSize);
        ret = 0;
    } while (0);

    if (blob != NULL) {
        free(blob);
    }
    if (pkey != NULL) {
        EVP_PKEY_free(pkey);
    }
    if (pkeyCtx != NULL) {
        EVP_PKEY_CTX_free(pkeyCtx);
    }

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmTeeRsaDecrypt(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, RsaSignPaddingAlg_e padding,
                            uint8_t *encryptedData, uint32_t encryptedDataLen, uint8_t *out, uint32_t* outBufSize)
{
    int ret = -1;
    uint8_t keyType = 0;
    uint8_t algo = 0;
    uint8_t *blob = NULL;
    uint32_t blobLen = 0;

    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    (void)alg;
    LOG_Warn("[%s] enter", __func__);

    do {
        if (TeeReadKeyObject(keyId, &keyType, &algo, &blob, &blobLen) != 0) {
            LOG_Error("[%s] read key object failed: 0x%04X", __func__, keyId);
            break;
        }

        if (SrHalTeeRsaDecrypt(blob, blobLen, padding, E_DIGIST_SHA256,
                               encryptedData, encryptedDataLen, out, outBufSize) != 0) {
            break;
        }

        LOG_Info("[%s] decrypt ok, keyId=0x%04X, len=%u", __func__, keyId, *outBufSize);
        ret = 0;
    } while (0);

    if (blob != NULL) {
        free(blob);
    }

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmTeeQueryKey(void* ctx, QueryKeyResult_t *keyResult)
{
    int ret = -1;
    uint32_t i = 0;
    uint16_t keyId = 0;
    uint8_t validCount = 0;

    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    LOG_Warn("[%s] enter", __func__);

    do {
        if (keyResult == NULL) {
            LOG_Error("[%s] invalid parameters", __func__);
            break;
        }

        memset(keyResult, 0, sizeof(QueryKeyResult_t));

        for (i = 0; i < teeCtx->keyCount && validCount < KEY_SLOT_COUNT; i++) {
            keyId = teeCtx->keyIds[i];
            if (keyId >= KEY_SLOT_COUNT) {
                continue;
            }
            if (teeCtx->keyAlg[i] == 0xFF || teeCtx->keyType[i] == 0xFF || teeCtx->keyRawBits[i] == 0xFF) {
                continue;
            }

            keyResult->keyList[validCount].keyId = (uint8_t)keyId;
            keyResult->keyList[validCount].keyAlg = (HsmKeyAlg_e)teeCtx->keyAlg[i];
            keyResult->keyList[validCount].keyType = (HsmKeyType_e)teeCtx->keyType[i];
            keyResult->keyList[validCount].keyBits = teeCtx->keyRawBits[i] * 32;
            validCount++;
        }

        keyResult->validKeyCount = validCount;
        LOG_Info("[%s] query key ok, validKeyCount=%u", __func__, validCount);
        ret = 0;
    } while (0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmTeeQueryCrt(void* ctx, QueryCrtResult_t *crtResult)
{
    int ret = -1;
    uint32_t i = 0;

    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    LOG_Warn("[%s] enter", __func__);

    do {
        if (crtResult == NULL) {
            LOG_Error("[%s] invalid parameters", __func__);
            break;
        }

        memset(crtResult, 0, sizeof(QueryCrtResult_t));

        for (i = 0; i < teeCtx->crtCount && i < CRT_SLOT_COUNT; i++) {
            crtResult->crtList[i] = teeCtx->crtIds[i];
        }

        crtResult->validCrtCount = (uint8_t)i;
        LOG_Info("[%s] query crt ok, validCrtCount=%u", __func__, crtResult->validCrtCount);
        ret = 0;
    } while (0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmTeeQueryKeyExists(void* ctx, const uint16_t keyId)
{
    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    return (TeeFindKeyId(teeCtx, keyId) >= 0) ? 1 : 0;
}

static int hsmTeeDeleteKey(void* ctx, const uint16_t keyId)
{
    int ret = -1;
    uint8_t keyType = 0;
    uint8_t algo = 0;
    uint8_t *blob = NULL;
    uint32_t blobLen = 0;

    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    LOG_Warn("[%s] enter", __func__);

    do {
        if (TeeFindKeyId(teeCtx, keyId) < 0) {
            LOG_Error("[%s] key not found: 0x%04X", __func__, keyId);
            break;
        }

        /* 1. 先把 key 从索引中移除并持久化，即使后续对象读取/销毁失败或掉电，
         *    索引也不会再指向这个 key，避免产生死索引。 */
        if (TeeRemoveKeyId(teeCtx, keyId) != 0) {
            break;
        }

        if (TeeSaveKeyIndex(teeCtx) != 0) {
            break;
        }

        /* 2. 再尝试读取对象、销毁 TEE key、删除 SS 对象。
         *    这些失败只可能导致孤儿对象，不影响索引一致性。 */
        if (TeeReadKeyObject(keyId, &keyType, &algo, &blob, &blobLen) == 0) {
            /* 非 PUB 类型密钥由 ql_km_destroy_key 内部释放 blob，PUB 类型需手动释放 */
            if (keyType != E_KEY_TYPE_PUB) {
                if (SrHalTeeDestroyKey(blob, blobLen) != 0) {
                    LOG_Warn("[%s] SrHalTeeDestroyKey failed: 0x%04X, index already removed", __func__, keyId);
                }
                blob = NULL;
            }
        } else {
            LOG_Warn("[%s] read key object failed, still try to delete ss object: 0x%04X", __func__, keyId);
        }

        if (TeeDeleteKeyObject(keyId) != 0) {
            LOG_Warn("[%s] delete key object failed: 0x%04X, index already removed", __func__, keyId);
        }

        LOG_Info("[%s] key deleted: 0x%04X", __func__, keyId);
        ret = 0;
    } while (0);

    if (blob != NULL) {
        free(blob);
    }

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmTeeExportKey(void* ctx, RsaKeyAlg_e exKeyAlg, const uint16_t exKeyId, uint8_t exKeyType,
                           uint8_t *out, uint32_t *outBufSize)
{
    int ret = -1;
    uint8_t keyType = 0;
    uint8_t algo = 0;
    uint8_t *blob = NULL;
    uint32_t blobLen = 0;
    uint8_t *pubDer = NULL;
    uint32_t pubDerLen = 0;

    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    (void)exKeyAlg;
    LOG_Warn("[%s] enter", __func__);

    do {
        if (exKeyType != E_KEY_TYPE_PUB) {
            LOG_Error("[%s] TEE only supports public key export", __func__);
            break;
        }

        if (TeeReadKeyObject(exKeyId, &keyType, &algo, &blob, &blobLen) != 0) {
            LOG_Error("[%s] read key object failed: 0x%04X", __func__, exKeyId);
            break;
        }

        if (keyType == E_KEY_TYPE_PUB) {
            /* 公钥直接以 X.509 SubjectPublicKeyInfo DER 存储 */
            if (blobLen > *outBufSize) {
                LOG_Error("[%s] buffer too small: %u < %u", __func__, *outBufSize, blobLen);
                *outBufSize = blobLen;
                ret = -1;
                break;
            }
            memcpy(out, blob, blobLen);
            *outBufSize = blobLen;
        } else {
            if (SrHalTeeExportPubKey(blob, blobLen, &pubDer, &pubDerLen) != 0) {
                break;
            }

            if (pubDerLen > *outBufSize) {
                LOG_Error("[%s] buffer too small: %u < %u", __func__, *outBufSize, pubDerLen);
                *outBufSize = pubDerLen;
                ret = -1;
                break;
            }
            memcpy(out, pubDer, pubDerLen);
            *outBufSize = pubDerLen;
        }

        LOG_Info("[%s] public key exported, keyId=0x%04X, len=%u", __func__, exKeyId, *outBufSize);
        ret = 0;
    } while (0);

    if (blob != NULL) {
        free(blob);
    }
    if (pubDer != NULL) {
        free(pubDer);
    }

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmTeeImportKey(void* ctx, RsaKeyAlg_e inKeyAlg, const uint16_t inKeyId, uint8_t inKeyType,
                           const uint8_t *inKeyVal, uint32_t inKeyValLen)
{
    int ret = -1;
    uint8_t algo = 0;
    uint8_t *keyBlob = NULL;
    uint32_t keyBlobLen = 0;
    int idAdded = 0;

    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    LOG_Warn("[%s] enter", __func__);

    do {
        switch (inKeyAlg) {
            case E_ALG_RSA1024: algo = 0x80; break;
            case E_ALG_RSA2048: algo = 0x81; break;
            default:
                LOG_Error("[%s] Unsupported algorithm: %d", __func__, inKeyAlg);
                break;
        }
        if (algo == 0) {
            break;
        }

        if (inKeyType != E_KEY_TYPE_PUB && inKeyType != E_KEY_TYPE_PRI && inKeyType != E_KEY_TYPE_PRI_PUB_PAIR) {
            LOG_Error("[%s] Unsupported keyType: %u", __func__, inKeyType);
            break;
        }

        if (inKeyType == E_KEY_TYPE_PUB) {
            /* 公钥直接以 X.509 SubjectPublicKeyInfo DER 形式存储，不导入 TEE */
            keyBlob = (uint8_t *)malloc(inKeyValLen);
            if (keyBlob == NULL) {
                LOG_Error("[%s] malloc key blob failed", __func__);
                break;
            }
            memcpy(keyBlob, inKeyVal, inKeyValLen);
            keyBlobLen = inKeyValLen;
        } else {
            /* 私钥/密钥对以 PKCS#8 PrivateKeyInfo DER 形式传入 TEE */
            if (SrHalTeeImportKey(inKeyVal, inKeyValLen, &keyBlob, &keyBlobLen) != 0) {
                break;
            }
        }

        if (TeeWriteKeyObject(inKeyId, inKeyType, algo, keyBlob, keyBlobLen) != 0) {
            LOG_Error("[%s] write key object failed", __func__);
            break;
        }

        if (TeeAddKeyId(teeCtx, inKeyId, inKeyType, algo) != 0) {
            LOG_Error("[%s] add key id failed", __func__);
            (void)TeeDeleteKeyObject(inKeyId);
            break;
        }
        idAdded = 1;

        if (TeeSaveKeyIndex(teeCtx) != 0) {
            LOG_Error("[%s] save key index failed", __func__);
            if (idAdded) {
                (void)TeeRemoveKeyId(teeCtx, inKeyId);
                (void)TeeDeleteKeyObject(inKeyId);
            }
            break;
        }

        LOG_Info("[%s] key imported, keyId=0x%04X, type=%u", __func__, inKeyId, inKeyType);
        ret = 0;
    } while (0);

    if (keyBlob != NULL) {
        if (keyBlobLen > 0) {
            memset(keyBlob, 0, keyBlobLen);
        }
        free(keyBlob);
    }

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmTeeQueryCrtExists(void* ctx, const uint16_t crtId)
{
    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    return (TeeFindCrtId(teeCtx, crtId) >= 0) ? 1 : 0;
}

static int hsmTeeDeleteCrt(void* ctx, const uint16_t crtId)
{
    int ret = -1;

    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    LOG_Warn("[%s] enter", __func__);

    do {
        if (TeeFindCrtId(teeCtx, crtId) < 0) {
            LOG_Error("[%s] crt not found: 0x%04X", __func__, crtId);
            break;
        }

        if (TeeRemoveCrtId(teeCtx, crtId) != 0) {
            break;
        }

        /* 先持久化索引（把 crt 从索引中移除），再删除对象，
         * 避免掉电或对象删除失败时索引指向不存在的证书。 */
        if (TeeSaveCrtIndex(teeCtx) != 0) {
            break;
        }

        if (TeeDeleteCrtObject(crtId) != 0) {
            LOG_Warn("[%s] delete crt object failed: 0x%04X, index already removed", __func__, crtId);
        }

        LOG_Info("[%s] crt deleted: 0x%04X", __func__, crtId);
        ret = 0;
    } while (0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmTeeExportCrt(void* ctx, const uint16_t crtId, uint8_t *out, uint32_t *outBufSize)
{
    int ret = -1;
    uint8_t *cert = NULL;
    uint32_t certLen = 0;

    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    LOG_Warn("[%s] enter", __func__);

    do {
        if (TeeReadCrtObject(crtId, &cert, &certLen) != 0) {
            LOG_Error("[%s] read crt object failed: 0x%04X", __func__, crtId);
            break;
        }

        if (*outBufSize < certLen) {
            LOG_Error("[%s] buffer too small: %u < %u", __func__, *outBufSize, certLen);
            *outBufSize = certLen;
            break;
        }

        memcpy(out, cert, certLen);
        *outBufSize = certLen;

        LOG_Info("[%s] crt exported, crtId=0x%04X, len=%u", __func__, crtId, *outBufSize);
        ret = 0;
    } while (0);

    if (cert != NULL) {
        free(cert);
    }

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmTeeImportCrt(void* ctx, const uint16_t crtId, const uint8_t *inCrtVal, uint32_t inCrtValLen)
{
    int ret = -1;
    int idAdded = 0;

    TeeContext_t* teeCtx = (TeeContext_t*)ctx;
    if (teeCtx == NULL) {
        LOG_Error("[%s] ctx is NULL", __func__);
        return -1;
    }
    LOG_Warn("[%s] enter", __func__);

    do {
        if (TeeWriteCrtObject(crtId, inCrtVal, inCrtValLen) != 0) {
            LOG_Error("[%s] write crt object failed", __func__);
            break;
        }

        if (TeeAddCrtId(teeCtx, crtId) != 0) {
            LOG_Error("[%s] add crt id failed", __func__);
            (void)TeeDeleteCrtObject(crtId);
            break;
        }
        idAdded = 1;

        if (TeeSaveCrtIndex(teeCtx) != 0) {
            LOG_Error("[%s] save crt index failed", __func__);
            if (idAdded) {
                (void)TeeRemoveCrtId(teeCtx, crtId);
                (void)TeeDeleteCrtObject(crtId);
            }
            break;
        }

        LOG_Info("[%s] crt imported, crtId=0x%04X, len=%u", __func__, crtId, inCrtValLen);
        ret = 0;
    } while (0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

const HsmApi_t g_hsmTee_Api = {
    .init = hsmTeeInit,
    .release = hsmTeeRelease,
    .random = hsmTeeRandom,
    .rsaDecrypt = hsmTeeRsaDecrypt,
    .rsaEncrypt = hsmTeeRsaEncrypt,
    .rsaKeyPairGenerate = hsmTeeRsaKeyPairGenerate,
    .rsaSign = hsmTeeRsaSign,
    .rsaVerify = hsmTeeRsaVerify,
    .queryKey = hsmTeeQueryKey,
    .queryKeyExists = hsmTeeQueryKeyExists,
    .deleteKey = hsmTeeDeleteKey,
    .exportKey = hsmTeeExportKey,
    .importKey = hsmTeeImportKey,
    .queryCrt = hsmTeeQueryCrt,
    .queryCrtExists = hsmTeeQueryCrtExists,
    .deleteCrt = hsmTeeDeleteCrt,
    .exportCrt = hsmTeeExportCrt,
    .importCrt = hsmTeeImportCrt,
};
