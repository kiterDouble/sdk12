#include "hsm_rpc_handler.h"
#include "sr_log.h"

#include <stdio.h>
#include <string.h>
#include <stddef.h>

// HED SDK 头文件
#include "se.h"
#include "crypto.h"
#include "key.h"
#include "ctx.h"

static void rpcRandom(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    int32_t ret = 0;
    uint32_t expectedLen = 0;
    uint8_t randomBuf[RANDOM_DATA_MAX_LEN] = {0};
    RandomResp_t msgResp = {0};
    LOG_Debug("[%s] receive request", __func__);
    if (hsmApi == NULL || hsmApi->random == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or random is NULL", __func__);
        msgResp.result = -1;
        msgResp.randomLen = 0;
        *respLen = sizeof(RandomResp_t);
        memcpy(respData, &msgResp, sizeof(RandomResp_t));
        return;
    }

    if (reqLen < sizeof(uint32_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(uint32_t)", __func__, reqLen);
        msgResp.result = -1;
        msgResp.randomLen = 0;
        *respLen = sizeof(RandomResp_t);
        memcpy(respData, &msgResp, sizeof(RandomResp_t));
        return;
    }

    memcpy(&expectedLen, reqData, sizeof(uint32_t));
    LOG_Debug("[%s] expectedLen=%u", __func__, expectedLen);

    // 验证 expectedLen
    if (expectedLen < RANDOM_DATA_MIN_LEN || expectedLen > RANDOM_DATA_MAX_LEN) {
        LOG_Error("[%s] Invalid expectedLen: %u (max 16)", __func__, expectedLen);
        msgResp.result = -1;
        msgResp.randomLen = 0;
        *respLen = sizeof(RandomResp_t);
        memcpy(respData, &msgResp, sizeof(RandomResp_t));
        return;
    }

    ret = hsmApi->random(hsmCtx, expectedLen, randomBuf);
    if (ret == 0) {
        msgResp.randomLen = expectedLen;
        memcpy(msgResp.randomData, randomBuf, expectedLen);
        LOG_Info("[%s] Generated %u random bytes", __func__, expectedLen);
    } else {
        msgResp.randomLen = 0;
        LOG_Error("[%s] random generation failed: %d", __func__, ret);
    }

    msgResp.result = ret;
    *respLen = sizeof(RandomResp_t);
    memcpy(respData, &msgResp, sizeof(RandomResp_t));
}

static void rpcGenKey(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    GenKeyReq_t *req = (GenKeyReq_t *)reqData;
    GenKeyResp_t msgResp = {0};
    uint8_t pubKeyDer[PUB_KEY_MAX_LEN] = {0};
    uint32_t pubKeyDerBufSize = sizeof(pubKeyDer);
    uint32_t ret = 0;
    LOG_Debug("[%s] receive request", __func__);
    if (hsmApi == NULL || hsmApi->rsaKeyPairGenerate == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or rsaKeyPairGenerate is NULL", __func__);
        msgResp.result = -1;
        msgResp.pubKeyLen = 0;
        *respLen = sizeof(GenKeyResp_t);
        memcpy(respData, &msgResp, sizeof(GenKeyResp_t));
        return;
    }

    if (reqLen < sizeof(GenKeyReq_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(GenKeyReq_t)=%lu", __func__, reqLen, (unsigned long)sizeof(GenKeyReq_t));
        msgResp.result = -1;
        *respLen = sizeof(GenKeyResp_t);
        memcpy(respData, &msgResp, sizeof(GenKeyResp_t));
        return;
    }

    LOG_Info("[%s] Request: alg=%d, keyId=0x%04X", __func__, req->alg, req->keyId);

    ret = hsmApi->rsaKeyPairGenerate(hsmCtx, req->alg, req->keyId, pubKeyDer, &pubKeyDerBufSize);
    if (ret == 0) {
        msgResp.pubKeyLen = pubKeyDerBufSize;
        memcpy(msgResp.pubKey, pubKeyDer, pubKeyDerBufSize);
        LOG_Info("[%s] Key generated successfully, pubKeyLen=%u", __func__, pubKeyDerBufSize);
    } else {
        msgResp.pubKeyLen = 0;
        LOG_Error("[%s] rsaKeyPairGenerate failed, ret=%d", __func__, ret);
    }

    msgResp.result = ret;
    *respLen = sizeof(GenKeyResp_t);
    memcpy(respData, &msgResp, sizeof(GenKeyResp_t));
}

static void rpcRsaEncrypt(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    RsaEncryptReq_t *req = (RsaEncryptReq_t *)reqData;
    uint8_t outBuf[RSA_OUTPUT_MAX_LEN] = {0};
    uint32_t outBufLen = sizeof(outBuf);
    RsaEncryptResp_t msgResp = {0};
    uint32_t ret = 0;
    LOG_Debug("[%s] receive request", __func__);
    if (hsmApi == NULL || hsmApi->rsaEncrypt == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or rsaEncrypt is NULL", __func__);
        msgResp.result = -1;
        msgResp.encLen = 0;
        *respLen = sizeof(RsaEncryptResp_t);
        memcpy(respData, &msgResp, sizeof(RsaEncryptResp_t));
        return;
    }

    if (reqLen < sizeof(RsaEncryptReq_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(RsaEncryptReq_t)=%lu", __func__, reqLen, (unsigned long)sizeof(RsaEncryptReq_t));
        msgResp.encLen = -1;
        *respLen = sizeof(RsaEncryptResp_t);
        memcpy(respData, &msgResp, sizeof(RsaEncryptResp_t));
        return;
    }

    LOG_Debug("[%s] alg=%d, keyId=0x%04X, padding=%d, plaintext len=%u", __func__, req->alg, req->keyId, req->padding, req->dataLen);

    ret = hsmApi->rsaEncrypt(hsmCtx, req->alg, req->keyId, req->padding, req->plaintext, req->dataLen, outBuf, &outBufLen);
    if (ret == 0)
    {
        msgResp.encLen = outBufLen;
        memcpy(msgResp.ciphertext, outBuf, outBufLen);
        LOG_Debug("[%s] encrypted %u bytes", __func__, outBufLen);
    }
    else
    {
        LOG_Error("[%s] rsaEncrypt failed: 0x%08X", __func__, ret);
        msgResp.encLen = -1;
    }

    msgResp.result = ret;
    *respLen = sizeof(RsaEncryptResp_t);
    memcpy(respData, &msgResp, sizeof(RsaEncryptResp_t));
}

static void rpcRsaDecrypt(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    RsaDecryptReq_t *req = (RsaDecryptReq_t *)reqData;
    RsaDecryptResp_t msgResp = {0};
    uint8_t outBuf[RSA_LARGE_OUTPUT_MAX_LEN] = {0};
    uint32_t outBufLen = sizeof(outBuf);
    uint32_t ret = 0;
    LOG_Debug("[%s] receive request", __func__);
    if (hsmApi == NULL || hsmApi->rsaDecrypt == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or rsaDecrypt is NULL", __func__);
        msgResp.result = -1;
        msgResp.decLen = 0;
        *respLen = sizeof(RsaDecryptResp_t);
        memcpy(respData, &msgResp, sizeof(RsaDecryptResp_t));
        return;
    }

    if (reqLen < sizeof(RsaDecryptReq_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(RsaDecryptReq_t)=%lu", __func__, reqLen, (unsigned long)sizeof(RsaDecryptReq_t));
        msgResp.decLen = -1;
        *respLen = sizeof(RsaDecryptResp_t);
        memcpy(respData, &msgResp, sizeof(RsaDecryptResp_t));
        return;
    }

    LOG_Debug("[%s] alg=%d, keyId=0x%04X, padding=%d, ciphertext len=%u", __func__, req->alg, req->keyId, req->padding, req->dataLen);

    ret = hsmApi->rsaDecrypt(hsmCtx, req->alg, req->keyId, req->padding, req->ciphertext, req->dataLen, outBuf, &outBufLen);
    if (ret == 0)
    {
        msgResp.decLen = outBufLen;
        memcpy(msgResp.plaintext, outBuf, outBufLen);
        LOG_Debug("[%s] decrypt %u bytes", __func__, outBufLen);
    }
    else
    {
        LOG_Error("[%s] decrypt failed: 0x%08X", __func__, ret);
        msgResp.decLen = -1;
    }

    msgResp.result = ret;
    *respLen = sizeof(RsaDecryptResp_t);
    memcpy(respData, &msgResp, sizeof(RsaDecryptResp_t));
}

static void rpcSign(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    SignReq_t *req = (SignReq_t *)reqData;
    SignResp_t msgResp = {0};
    uint8_t signature[SIGNATURE_MAX_LEN] = {0};
    uint32_t signBufSize = sizeof(signature);
    uint32_t ret = 0;
    LOG_Debug("[%s] receive request", __func__);
    if (hsmApi == NULL || hsmApi->rsaSign == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or rsaSign is NULL", __func__);
        msgResp.result = -1;
        *respLen = sizeof(SignResp_t);
        memcpy(respData, &msgResp, sizeof(SignResp_t));
        return;
    }

    if (reqLen < sizeof(SignReq_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(SignReq_t)=%lu", __func__, reqLen, (unsigned long)sizeof(SignReq_t));
        msgResp.result = -1;
        *respLen = sizeof(SignResp_t);
        memcpy(respData, &msgResp, sizeof(SignResp_t));
        return;
    }

    LOG_Debug("[%s] Request: alg=%d, keyId=0x%04X, digist=%d, dataLen=%u", __func__, req->alg, req->keyId, req->digist, req->dataLen);

    ret = hsmApi->rsaSign(hsmCtx, req->alg, req->keyId, req->digist, req->data, req->dataLen, signature, &signBufSize);
    if (ret == SE_SUCCESS)
    {
        msgResp.signLen = signBufSize;
        memcpy(msgResp.signature, signature, signBufSize);
        LOG_Debug("[%s] signed %u bytes success", __func__, signBufSize);
    }
    else
    {
        LOG_Error("[%s] rsaSign failed: 0x%08X", __func__, ret);
        msgResp.signLen = 0;
    }

    msgResp.result = ret;
    *respLen = sizeof(SignResp_t);
    memcpy(respData, &msgResp, sizeof(SignResp_t));
}

static void rpcVerify(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    VerifyReq_t *req = (VerifyReq_t *)reqData;
    CommonResp_t msgResp = {0};
    uint32_t ret = 0;
    LOG_Debug("[%s] receive request", __func__);
    if (hsmApi == NULL || hsmApi->rsaVerify == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or rsaVerify is NULL", __func__);
        msgResp.result = -1;
        *respLen = sizeof(CommonResp_t);
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        return;
    }

    if (reqLen < sizeof(VerifyReq_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(VerifyReq_t)=%lu", __func__, reqLen, (unsigned long)sizeof(VerifyReq_t));
        msgResp.result = -1;
        *respLen = sizeof(CommonResp_t);
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        return;
    }

    LOG_Debug("[%s] alg=%d, keyId=0x%04X, digist=%d, dataLen=%u, signLen=%u", __func__, req->alg, req->keyId, req->digist, req->dataLen, req->signLen);

    ret = hsmApi->rsaVerify(hsmCtx, req->alg, req->keyId, req->digist, req->data, req->dataLen, req->signature, req->signLen);
    if (ret != 0)
        LOG_Error("[%s] HSM verify failed: %d", __func__, ret);

    msgResp.result = ret;
    LOG_Debug("[%s] verify result: %d ", __func__, msgResp.result);

    *respLen = sizeof(CommonResp_t);
    memcpy(respData, &msgResp, sizeof(CommonResp_t));
}

static void rpcQueryKey(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    QueryKeyResp_t msgResp = {0};
    QueryKeyResult_t keyResult = {0};
    uint32_t ret = 0;
    LOG_Debug("[%s] receive request", __func__);

    (void)reqData;
    (void)reqLen;
    if (hsmApi == NULL || hsmApi->queryKey == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or queryKey is NULL", __func__);
        msgResp.result = -1;
        *respLen = sizeof(QueryKeyResp_t);
        memcpy(respData, &msgResp, sizeof(QueryKeyResp_t));
        return;
    }

    ret = hsmApi->queryKey(hsmCtx, &keyResult);
    msgResp.result = ret;
    msgResp.validKeyCount = keyResult.validKeyCount;
    if (keyResult.validKeyCount > 0) {
        memcpy(msgResp.keyList, keyResult.keyList, keyResult.validKeyCount * sizeof(KeyInfo_t));
    }

    *respLen = sizeof(QueryKeyResp_t);
    memcpy(respData, &msgResp, sizeof(QueryKeyResp_t));
}

static void rpcQueryKeyExists(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    CommonResp_t msgResp = {0};
    uint16_t keyId = 0;
    int32_t ret = 0;
    LOG_Debug("[%s] receive request", __func__);

    (void)reqLen;
    if (hsmApi == NULL || hsmApi->queryKeyExists == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or queryKeyExists is NULL", __func__);
        msgResp.result = -1;
        *respLen = sizeof(CommonResp_t);
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        return;
    }

    if (reqLen < sizeof(uint16_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(uint16_t)", __func__, reqLen);
        msgResp.result = -1;
        *respLen = sizeof(CommonResp_t);
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        return;
    }

    keyId = reqData[0] | (reqData[1] << 8);
    LOG_Debug("[%s] keyId=0x%04X", __func__, keyId);

    ret = hsmApi->queryKeyExists(hsmCtx, keyId);
    msgResp.result = ret;

    *respLen = sizeof(CommonResp_t);
    memcpy(respData, &msgResp, sizeof(CommonResp_t));
}

static void rpcDeleteKey(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    CommonResp_t msgResp = {0};
    uint16_t keyId = 0;
    uint32_t ret = 0;

    if (reqLen < sizeof(uint16_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(uint16_t)", __func__, reqLen);
        msgResp.result = -1;
        *respLen = sizeof(CommonResp_t);
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        return;
    }

    keyId = reqData[0] | (reqData[1] << 8);
    LOG_Debug("[%s] receive request: keyId=0x%04X", __func__, keyId);

    if (hsmApi == NULL || hsmApi->deleteKey == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or deleteKey is NULL", __func__);
        msgResp.result = -1;
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        *respLen = sizeof(CommonResp_t);
        return;
    }

    ret = hsmApi->deleteKey(hsmCtx, keyId);

    msgResp.result = ret;
    memcpy(respData, &msgResp, sizeof(CommonResp_t));
    *respLen = sizeof(CommonResp_t);

    if (ret != 0)
        LOG_Error("[%s] deleteKey failed: 0x%08X", __func__, ret);
}

static void rpcExportKey(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    ExportKeyReq_t *req = (ExportKeyReq_t *)reqData;
    ExportKeyResp_t msgResp = {0};
    uint32_t ret = 0;
    LOG_Debug("[%s] receive request", __func__);

    msgResp.outBufLen = sizeof(msgResp.outBuf);
    if (hsmApi == NULL || hsmApi->exportKey == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or exportKey is NULL", __func__);
        msgResp.result = -1;
        *respLen = sizeof(ExportKeyResp_t);
        memcpy(respData, &msgResp, sizeof(ExportKeyResp_t));
        return;
    }

    if (reqLen < sizeof(ExportKeyReq_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(ExportKeyReq_t)=%lu", __func__, reqLen, (unsigned long)sizeof(ExportKeyReq_t));
        msgResp.result = -1;
        *respLen = sizeof(ExportKeyResp_t);
        memcpy(respData, &msgResp, sizeof(ExportKeyResp_t));
        return;
    }

    LOG_Debug("[%s] Request: exKeyAlg=%d, exKeyId=0x%04X, exKeyType=%d", __func__, req->exKeyAlg, req->exKeyId, req->exKeyType);

    ret = hsmApi->exportKey(hsmCtx, req->exKeyAlg, req->exKeyId, req->exKeyType, msgResp.outBuf, &msgResp.outBufLen);
    if (ret != 0)
        LOG_Error("[%s] HSM exportKey failed: %d", __func__, ret);

    msgResp.result = ret;
    *respLen = sizeof(ExportKeyResp_t);
    memcpy(respData, &msgResp, sizeof(ExportKeyResp_t));
}

static void rpcImportKey(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    ImportKeyReq_t *req = (ImportKeyReq_t *)reqData;
    CommonResp_t msgResp = {0};
    uint32_t ret = 0;
    uint16_t headerLen = offsetof(ImportKeyReq_t, inKeyVal);
    LOG_Debug("[%s] receive request", __func__);

    if (hsmApi == NULL || hsmApi->importKey == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or importKey is NULL", __func__);
        msgResp.result = -1;
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        *respLen = sizeof(CommonResp_t);
        return;
    }

    if (reqLen < headerLen) {
        LOG_Error("[%s] reqLen=%u < headerLen=%u", __func__, reqLen, headerLen);
        msgResp.result = -1;
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        *respLen = sizeof(CommonResp_t);
        return;
    }

    if (req->inKeyValLen > IMPORT_KEY_MAX_LEN || reqLen < headerLen + req->inKeyValLen) {
        LOG_Error("[%s] invalid inKeyValLen=%u (max=%u) or reqLen=%u too short",
                  __func__, req->inKeyValLen, IMPORT_KEY_MAX_LEN, reqLen);
        msgResp.result = -1;
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        *respLen = sizeof(CommonResp_t);
        return;
    }

    LOG_Debug("[%s] Request: inKeyAlg=%d, inKeyId=0x%04X, inKeyType=%d, inKeyValLen=%u",
              __func__, req->inKeyAlg, req->inKeyId, req->inKeyType, req->inKeyValLen);

    ret = hsmApi->importKey(hsmCtx, req->inKeyAlg, req->inKeyId, req->inKeyType, req->inKeyVal, req->inKeyValLen);
    if (ret != 0)
        LOG_Error("[%s] HSM importKey failed: %d", __func__, ret);

    msgResp.result = ret;
    *respLen = sizeof(CommonResp_t);
    memcpy(respData, &msgResp, sizeof(CommonResp_t));
}

static void rpcQueryCrt(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    QueryCrtResp_t msgResp = {0};
    QueryCrtResult_t crtResult = {0};
    uint32_t ret = 0;
    LOG_Debug("[%s] receive request", __func__);

    (void)reqData;
    (void)reqLen;
    if (hsmApi == NULL || hsmApi->queryCrt == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or queryCrt is NULL", __func__);
        msgResp.result = -1;
        *respLen = sizeof(QueryCrtResp_t);
        memcpy(respData, &msgResp, sizeof(QueryCrtResp_t));
        return;
    }

    ret = hsmApi->queryCrt(hsmCtx, &crtResult);
    if (ret != 0) {
        LOG_Error("[%s] HSM queryCrt failed: %d", __func__, ret);
    }

    msgResp.result = ret;
    msgResp.validCrtCount = crtResult.validCrtCount;
    if (crtResult.validCrtCount > 0) {
        memcpy(msgResp.crtList, crtResult.crtList, crtResult.validCrtCount * sizeof(uint16_t));
    }
    *respLen = sizeof(QueryCrtResp_t);
    memcpy(respData, &msgResp, sizeof(QueryCrtResp_t));
}

static void rpcQueryCrtExists(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    CommonResp_t msgResp = {0};
    uint16_t crtId = 0;
    int32_t ret = 0;
    LOG_Debug("[%s] receive request", __func__);

    (void)reqLen;
    if (hsmApi == NULL || hsmApi->queryCrtExists == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or queryCrtExists is NULL", __func__);
        msgResp.result = -1;
        *respLen = sizeof(CommonResp_t);
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        return;
    }

    if (reqLen < sizeof(uint16_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(uint16_t)", __func__, reqLen);
        msgResp.result = -1;
        *respLen = sizeof(CommonResp_t);
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        return;
    }

    crtId = reqData[0] | (reqData[1] << 8);
    LOG_Debug("[%s] crtId=0x%04X", __func__, crtId);

    ret = hsmApi->queryCrtExists(hsmCtx, crtId);
    msgResp.result = ret;

    *respLen = sizeof(CommonResp_t);
    memcpy(respData, &msgResp, sizeof(CommonResp_t));
}

static void rpcDeleteCrt(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    CommonResp_t msgResp = {0};
    uint16_t crtId = 0;
    uint32_t ret = 0;
    LOG_Debug("[%s] receive request", __func__);

    if (hsmApi == NULL || hsmApi->deleteCrt == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or deleteCrt is NULL", __func__);
        msgResp.result = -1;
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        *respLen = sizeof(CommonResp_t);
        return;
    }

    if (reqLen < sizeof(uint16_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(uint16_t)", __func__, reqLen);
        msgResp.result = -1;
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        *respLen = sizeof(CommonResp_t);
        return;
    }

    crtId = reqData[0] | (reqData[1] << 8);
    LOG_Debug("[%s] Request: crtId=0x%04X", __func__, crtId);

    ret = hsmApi->deleteCrt(hsmCtx, crtId);
    if (ret != 0)
        LOG_Error("[%s] HSM deleteCrt failed: %d", __func__, ret);

    msgResp.result = ret;
    memcpy(respData, &msgResp, sizeof(CommonResp_t));
    *respLen = sizeof(CommonResp_t);
}

static void rpcExportCrt(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    ExportCrtReq_t *req = (ExportCrtReq_t *)reqData;
    ExportCrtResp_t msgResp = {0};
    uint32_t ret = 0;
    LOG_Debug("[%s] receive request", __func__);
    if (hsmApi == NULL || hsmApi->exportCrt == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or exportCrt is NULL", __func__);
        msgResp.result = -1;
        *respLen = sizeof(ExportCrtResp_t);
        memcpy(respData, &msgResp, sizeof(ExportCrtResp_t));
        return;
    }

    if (reqLen < sizeof(ExportCrtReq_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(ExportCrtReq_t)=%lu", __func__, reqLen, (unsigned long)sizeof(ExportCrtReq_t));
        msgResp.result = -1;
        *respLen = sizeof(ExportCrtResp_t);
        memcpy(respData, &msgResp, sizeof(ExportCrtResp_t));
        return;
    }

    LOG_Debug("[%s] Request: crtId=0x%04X", __func__, req->crtId);

    msgResp.outBufLen = EXPORT_CRT_MAX_LEN;
    ret = hsmApi->exportCrt(hsmCtx, req->crtId, msgResp.outBuf, &msgResp.outBufLen);
    if (ret != 0)
        LOG_Error("[%s] HSM exportCrt failed: %d", __func__, ret);

    msgResp.result = ret;
    *respLen = sizeof(ExportCrtResp_t);
    memcpy(respData, &msgResp, sizeof(ExportCrtResp_t));
}

static void rpcImportCrt(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t* ctx = (Ctx_t*)userdata;
    void* hsmCtx = (ctx != NULL) ? ctx->hsmUserData : NULL;
    HsmApi_t *hsmApi = ((Ctx_t*) userdata)->hsmApi;
    ImportCrtReq_t *req = (ImportCrtReq_t *)reqData;
    CommonResp_t msgResp = {0};
    uint32_t ret = 0;
    LOG_Debug("[%s] receive request", __func__);

    if (hsmApi == NULL || hsmApi->importCrt == NULL) {
        LOG_Error("[%s] ERROR: hsmApi or importCrt is NULL", __func__);
        msgResp.result = -1;
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        *respLen = sizeof(CommonResp_t);
        return;
    }

    if (reqLen < sizeof(ImportCrtReq_t)) {
        LOG_Error("[%s] reqLen=%u < sizeof(ImportCrtReq_t)=%lu", __func__, reqLen, (unsigned long)sizeof(ImportCrtReq_t));
        msgResp.result = -1;
        memcpy(respData, &msgResp, sizeof(CommonResp_t));
        *respLen = sizeof(CommonResp_t);
        return;
    }

    LOG_Debug("[%s] Request: crtId=0x%04X, crtValLen=%u", __func__, req->crtId, req->crtValLen);

    ret = hsmApi->importCrt(hsmCtx, req->crtId, req->crtVal, req->crtValLen);
    if (ret != 0)
        LOG_Error("[%s] HSM importCrt failed: %d", __func__, ret);

    msgResp.result = ret;
    memcpy(respData, &msgResp, sizeof(CommonResp_t));
    *respLen = sizeof(CommonResp_t);
}

static void rpcGetWorkState(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    HsmWorkStateResp_t msgResp = {0};
    Ctx_t *ctx = (Ctx_t*)userdata;
    LOG_Debug("[%s] receive request", __func__);

    (void)reqData;
    (void)reqLen;

    if (ctx == NULL)
    {
        LOG_Error("[%s] ctx is NULL", __func__);
        msgResp.result = -1;
    }
    else
    {
        msgResp.result = 0;
        msgResp.state = (uint8_t)ctx->hsmWorkState;
    }

    memcpy(respData, &msgResp, sizeof(msgResp));
    *respLen = sizeof(msgResp);
}

static const HsmSubProc_t g_hsmSubProcs[] = {
    { E_HSM_MSGTYPE_RANDOM,       rpcRandom },      // 随机数生成
    { E_HSM_MSGTYPE_GEN_KEY,      rpcGenKey },      // 生成密钥对
    { E_HSM_MSGTYPE_RSA_ENCRYPT,  rpcRsaEncrypt },  // RSA加密
    { E_HSM_MSGTYPE_RSA_DECRYPT,  rpcRsaDecrypt },  // RSA解密
    { E_HSM_MSGTYPE_SIGN,         rpcSign },        // RSA签名
    { E_HSM_MSGTYPE_VERIFY,       rpcVerify },      // RSA验签
    { E_HSM_MSGTYPE_QUERY_KEY,    rpcQueryKey },    // 查询密钥
    { E_HSM_MSGTYPE_DELETE_KEY,   rpcDeleteKey },   // 删除密钥
    { E_HSM_MSGTYPE_EXPORT_KEY,   rpcExportKey },   // 导出密钥
    { E_HSM_MSGTYPE_IMPORT_KEY,   rpcImportKey },   // 导入密钥
    { E_HSM_MSGTYPE_QUERY_CRT,    rpcQueryCrt },    // 查询证书
    { E_HSM_MSGTYPE_DELETE_CRT,   rpcDeleteCrt },   // 删除证书
    { E_HSM_MSGTYPE_EXPORT_CRT,   rpcExportCrt },   // 导出证书
    { E_HSM_MSGTYPE_IMPORT_CRT,   rpcImportCrt },   // 导入证书
    { E_HSM_MSGTYPE_QUERY_KEY_EXISTS, rpcQueryKeyExists }, // 查询密钥是否存在
    { E_HSM_MSGTYPE_QUERY_CRT_EXISTS, rpcQueryCrtExists }, // 查询证书是否存在
    { E_HSM_MSGTYPE_GET_WORK_STATE, rpcGetWorkState }, // 获取HSM工作状态
};

void hsmRpcCallback(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata)
{
    Ctx_t *ctx = (Ctx_t*)userdata;
    CommonResp_t msgResp = {0};
    uint32_t i = 0;
    LOG_Debug("[%s] subcmd[%u] reqLen[%u]", __func__, (unsigned)subcmd, (unsigned)reqLen);

    if (userdata == NULL || respData == NULL || respLen == NULL) {
        LOG_Error("[%s] invalid parameters: userdata=%p, respData=%p, respLen=%p",
                  __func__, userdata, respData, respLen);
        if (respData != NULL && respLen != NULL) {
            msgResp.result = -1;
            *respLen = sizeof(CommonResp_t);
            memcpy(respData, &msgResp, sizeof(CommonResp_t));
        }
        return;
    }

    g_mutex_lock(&ctx->hsmApiMutex);

    for (i = 0; i < sizeof(g_hsmSubProcs) / sizeof(g_hsmSubProcs[0]); ++i)
    {
        if (g_hsmSubProcs[i].sub == subcmd)
        {
            if (g_hsmSubProcs[i].procCb == NULL)
            {
                LOG_Error("[%s] handler for subcmd=%u is NULL", __func__, (unsigned)subcmd);
                msgResp.result = -1;
                *respLen = sizeof(CommonResp_t);
                memcpy(respData, &msgResp, sizeof(CommonResp_t));
                g_mutex_unlock(&ctx->hsmApiMutex);
                return;
            }

            g_hsmSubProcs[i].procCb(reqData, reqLen, respData, respLen, userdata);
            g_mutex_unlock(&ctx->hsmApiMutex);
            return;
        }
    }

    LOG_Error("[%s] subcmd invalid: %u", __func__, (unsigned)subcmd);
    msgResp.result = -1;
    *respLen = sizeof(CommonResp_t);
    memcpy(respData, &msgResp, sizeof(CommonResp_t));
    g_mutex_unlock(&ctx->hsmApiMutex);
}
