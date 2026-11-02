#include "rpc_hsm_api.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include <string.h>
#include <stdio.h>
#include <stddef.h>

int32_t RpcHsmCall(uint16_t subcmd, const uint8_t *reqData, uint16_t reqLen,
                                     uint8_t *respData, uint16_t *respLen)
{
    int32_t ret = -1;

    do {
        ret = RpcGetMsgSync(RPCSERVER_TYPE_HSM_OPENSSL_ENGINE_SERVER, subcmd, 15000,
                            reqData, reqLen, respData, respLen);
    } while(0);
    return ret;
}

int RpcHsmQueryCrtExists(uint16_t crtId)
{
    int32_t ret = -1;
    CommonResp_t resp;
    uint16_t respLen = sizeof(resp);

    LOG_Warn("[%s] enter", __func__);
    do {
        LOG_Info("[%s] crtId=0x%04X", __func__, crtId);

        ret = RpcHsmCall(E_HSM_MSGTYPE_QUERY_CRT_EXISTS, (uint8_t*)&crtId, sizeof(uint16_t), (uint8_t*)&resp, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        if (resp.result != 0 && resp.result != 1) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp.result);
            ret = resp.result;
            break;
        }

        LOG_Info("[%s] exit(%d)", __func__, resp.result);
        ret = resp.result;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmRandom(uint32_t expectedLen, uint8_t *random)
{
    int32_t ret = -1;
    uint8_t reqData[sizeof(uint32_t)] = {0};
    RandomResp_t resp = {0};
    uint16_t respLen = sizeof(resp);

    LOG_Warn("[%s] enter", __func__);
    do {
        if (random == NULL || expectedLen == 0) {
            LOG_Error("[%s] Invalid parameters: random=%p, expectedLen=%u", __func__, random, expectedLen);
            break;
        }

        LOG_Debug("[%s] expectedLen=%u", __func__, expectedLen);

        memcpy(reqData, &expectedLen, sizeof(uint32_t));

        ret = RpcHsmCall(E_HSM_MSGTYPE_RANDOM, reqData, sizeof(reqData), (uint8_t*)&resp, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        if (resp.result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp.result);
            ret = resp.result;
            break;
        }

        memcpy(random, resp.randomData, resp.randomLen);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmGenKey(RsaKeyAlg_e alg, uint16_t keyId, uint8_t* pubKeyDer, uint32_t *pubKeyDerBufSize)
{
    int32_t ret = -1;
    GenKeyReq_t req = {0};
    GenKeyResp_t resp = {0};
    uint16_t respLen = sizeof(resp);

    LOG_Warn("[%s] enter", __func__);
    do {
        if (pubKeyDer == NULL || pubKeyDerBufSize == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        LOG_Debug("[%s] alg=%d, keyId=0x%04X", __func__, alg, keyId);

        req.alg = alg;
        req.keyId = keyId;

        ret = RpcHsmCall(E_HSM_MSGTYPE_GEN_KEY, (uint8_t*)&req, sizeof(req), (uint8_t*)&resp, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        if (resp.result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp.result);
            ret = resp.result;
            break;
        }

        if (resp.pubKeyLen > 0 && resp.pubKeyLen <= *pubKeyDerBufSize) {
            memcpy(pubKeyDer, resp.pubKey, resp.pubKeyLen);
            *pubKeyDerBufSize = resp.pubKeyLen;
            LOG_Debug("[%s] success, pubKeyLen=%u", __func__, resp.pubKeyLen);
        } else if (resp.pubKeyLen > *pubKeyDerBufSize) {
            LOG_Warn("[%s] pubKeyLen=%u exceeds buffer size=%u", __func__, resp.pubKeyLen, *pubKeyDerBufSize);
            *pubKeyDerBufSize = resp.pubKeyLen;
            ret = -1;
            break;
        } else {
            *pubKeyDerBufSize = 0;
        }

        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmSign(RsaKeyAlg_e alg, uint16_t keyId, DigistAlg_e digist, const uint8_t *data, uint32_t dataLen,
    uint8_t *signature, uint32_t *signLen)
{
    int32_t ret = -1;
    SignReq_t req = {0};
    uint8_t respData[sizeof(SignResp_t)] = {0};
    uint16_t respLen = sizeof(respData);
    SignResp_t *resp;

    LOG_Warn("[%s] enter", __func__);
    do {
        if (data == NULL || signature == NULL || signLen == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        if (dataLen > DATA_MAX_LEN) {
            LOG_Error("[%s] dataLen=%u exceeds max %d", __func__, dataLen, DATA_MAX_LEN);
            break;
        }

        LOG_Info("[%s] dataLen=%u", __func__, dataLen);

        req.alg = alg;
        req.keyId = keyId;
        req.digist = digist;
        req.dataLen = dataLen;
        memcpy(req.data, data, dataLen);

        ret = RpcHsmCall(E_HSM_MSGTYPE_SIGN, (uint8_t*)&req, sizeof(SignReq_t), respData, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        resp = (SignResp_t *)respData;
        if (resp->result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp->result);
            ret = -1;
            break;
        }

        if (resp->signLen > SIGNATURE_MAX_LEN || resp->signLen > *signLen) {
            LOG_Error("[%s] Invalid signLen: %u", __func__, resp->signLen);
            ret = -1;
            break;
        }

        *signLen = resp->signLen;
        memcpy(signature, resp->signature, resp->signLen);

        LOG_Info("[%s] success, len=%u", __func__, *signLen);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmVerify(RsaKeyAlg_e alg, uint16_t keyId, DigistAlg_e digist, const uint8_t *data, uint32_t dataLen,
    uint8_t *signature, uint32_t signLen)
{
    int32_t ret = -1;
    VerifyReq_t req = {0};
    uint8_t respData[sizeof(CommonResp_t)] = {0};
    uint16_t respLen = sizeof(respData);
    CommonResp_t *resp;

    LOG_Warn("[%s] enter", __func__);
    do {
        if (data == NULL || signature == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        if (dataLen > DATA_MAX_LEN || signLen > SIGNATURE_MAX_LEN) {
            LOG_Error("[%s] dataLen=%u or signLen=%u exceeds limit", __func__, dataLen, signLen);
            break;
        }

        LOG_Info("[%s] alg=%d, keyId=0x%04X, digist=%d, dataLen=%u, signLen=%u",
                 __func__, alg, keyId, digist, dataLen, signLen);

        req.alg = alg;
        req.keyId = keyId;
        req.digist = digist;
        req.dataLen = dataLen;
        memcpy(req.data, data, dataLen);
        req.signLen = signLen;
        memcpy(req.signature, signature, signLen);

        ret = RpcHsmCall(E_HSM_MSGTYPE_VERIFY, (uint8_t*)&req, sizeof(VerifyReq_t), respData, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        resp = (CommonResp_t *)respData;
        ret = resp->result;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmEncrypt(RsaKeyAlg_e alg, uint16_t keyId, uint8_t padding, const uint8_t *plaintext, uint32_t plainLen,
    uint8_t *ciphertext, uint32_t *cipherLen)
{
    int32_t ret = -1;
    RsaEncryptReq_t req = {0};
    uint8_t respData[sizeof(RsaEncryptResp_t)] = {0};
    uint16_t respLen = sizeof(respData);
    RsaEncryptResp_t *resp;

    LOG_Warn("[%s] enter", __func__);
    do {
        if (plaintext == NULL || ciphertext == NULL || cipherLen == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        if (plainLen > DATA_MAX_LEN) {
            LOG_Error("[%s] plainLen=%u exceeds max %d", __func__, plainLen, DATA_MAX_LEN);
            break;
        }

        LOG_Info("[%s] alg=%d, keyId=0x%04X, padding=%d, len=%u", __func__, alg, keyId, padding, plainLen);

        req.keyId = keyId;
        req.alg = alg;
        req.padding = padding;
        req.dataLen = plainLen;
        memcpy(req.plaintext, plaintext, plainLen);

        ret = RpcHsmCall(E_HSM_MSGTYPE_RSA_ENCRYPT, (uint8_t*)&req, sizeof(RsaEncryptReq_t), respData, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        resp = (RsaEncryptResp_t *)respData;
        if (resp->result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp->result);
            ret = -1;
            break;
        }

        *cipherLen = resp->encLen;
        memcpy(ciphertext, resp->ciphertext, resp->encLen);

        LOG_Info("[%s] success, len=%u", __func__, *cipherLen);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmDecrypt(RsaKeyAlg_e alg, uint16_t keyId, uint8_t padding, const uint8_t *ciphertext, uint32_t cipherLen,
                                 uint8_t *plaintext, uint32_t *plainLen)
{
    int32_t ret = -1;
    RsaDecryptReq_t req = {0};
    uint8_t respData[sizeof(RsaDecryptResp_t)] = {0};
    uint16_t respLen = sizeof(respData);
    RsaDecryptResp_t *resp;

    LOG_Warn("[%s] enter", __func__);
    do {
        if (ciphertext == NULL || plaintext == NULL || plainLen == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        if (cipherLen > DATA_MAX_LEN) {
            LOG_Error("[%s] cipherLen=%u exceeds max %d", __func__, cipherLen, DATA_MAX_LEN);
            break;
        }

        LOG_Info("[%s] keyId=0x%04X, len=%u", __func__, keyId, cipherLen);

        req.alg = alg;
        req.padding = padding;
        req.keyId = keyId;
        req.dataLen = cipherLen;
        memcpy(req.ciphertext, ciphertext, cipherLen);

        ret = RpcHsmCall(E_HSM_MSGTYPE_RSA_DECRYPT, (uint8_t*)&req, sizeof(RsaDecryptReq_t), respData, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        resp = (RsaDecryptResp_t *)respData;
        if (resp->result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp->result);
            ret = -1;
            break;
        }

        *plainLen = resp->decLen;
        memcpy(plaintext, resp->plaintext, resp->decLen);

        LOG_Info("[%s] success, len=%u", __func__, *plainLen);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmQueryKey(QueryKeyResult_t *keyResult)
{
    int32_t ret = -1;
    uint8_t respData[sizeof(QueryKeyResp_t)] = {0};
    uint16_t respLen = sizeof(respData);
    uint8_t dummyReq = 0;
    QueryKeyResp_t *resp = NULL;

    // LOG_Warn("[%s] enter", __func__);
    do {
        if (keyResult == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        ret = RpcHsmCall(E_HSM_MSGTYPE_QUERY_KEY, &dummyReq, sizeof(uint8_t), respData, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        resp = (QueryKeyResp_t *)respData;
        if (resp->result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp->result);
            ret = -1;
            break;
        }

        memset(keyResult, 0, sizeof(QueryKeyResult_t));
        keyResult->validKeyCount = resp->validKeyCount;
        if (resp->validKeyCount >= KEY_SLOT_COUNT) {
            keyResult->validKeyCount = KEY_SLOT_COUNT - 1;
        }
        memcpy(keyResult->keyList, resp->keyList, keyResult->validKeyCount * sizeof(KeyInfo_t));

        LOG_Info("[%s] success, validKeyCount=%u", __func__, keyResult->validKeyCount);
        ret = 0;
    } while(0);

    // LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmDeleteKey(uint16_t keyId)
{
    int32_t ret = -1;
    CommonResp_t resp;
    uint16_t respLen = sizeof(resp);

    LOG_Warn("[%s] enter", __func__);
    do {
        LOG_Info("[%s] keyId=0x%04X", __func__, keyId);

        ret = RpcHsmCall(E_HSM_MSGTYPE_DELETE_KEY, (uint8_t*)&keyId, sizeof(uint16_t), (uint8_t*)&resp, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        if (resp.result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp.result);
            ret = resp.result;
            break;
        }

        LOG_Info("[%s] success", __func__);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmExportKey(RsaKeyAlg_e exKeyAlg, uint16_t exKeyId, KeyType_e exKeyType, uint8_t *outBuf, uint32_t *outBufLen)
{
    int32_t ret = -1;
    ExportKeyReq_t req = {0};
    uint8_t respData[sizeof(ExportKeyResp_t)] = {0};
    uint16_t respLen = sizeof(respData);
    ExportKeyResp_t *resp;

    LOG_Warn("[%s] enter", __func__);
    do {
        if (outBuf == NULL || outBufLen == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        LOG_Info("[%s] exKeyAlg=%d, exKeyId=0x%04X, exKeyType=%d", __func__, exKeyAlg, exKeyId, exKeyType);

        req.exKeyAlg = exKeyAlg;
        req.exKeyId = exKeyId;
        req.exKeyType = exKeyType;

        ret = RpcHsmCall(E_HSM_MSGTYPE_EXPORT_KEY, (uint8_t*)&req, sizeof(ExportKeyReq_t), respData, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        resp = (ExportKeyResp_t *)respData;
        if (resp->result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp->result);
            ret = -1;
            break;
        }

        *outBufLen = resp->outBufLen;
        memcpy(outBuf, resp->outBuf, resp->outBufLen);

        LOG_Info("[%s] success, len=%u", __func__, *outBufLen);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmImportKey(RsaKeyAlg_e inKeyAlg, uint16_t inKeyId, KeyType_e inKeyType, const uint8_t *inKeyVal, uint32_t inKeyValLen)
{
    int32_t ret = -1;
    ImportKeyReq_t req = {0};
    uint8_t respData[sizeof(CommonResp_t)] = {0};
    uint16_t respLen = sizeof(respData);
    uint16_t reqLen = 0;
    CommonResp_t *resp;

    LOG_Warn("[%s] enter", __func__);
    do {
        if (inKeyVal == NULL || inKeyValLen == 0 || inKeyValLen > IMPORT_KEY_MAX_LEN) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        LOG_Info("[%s] inKeyAlg=%d, inKeyId=0x%04X, inKeyType=%d, inKeyValLen=%u",
                 __func__, inKeyAlg, inKeyId, inKeyType, inKeyValLen);

        req.inKeyAlg = inKeyAlg;
        req.inKeyId = inKeyId;
        req.inKeyType = inKeyType;
        req.inKeyValLen = inKeyValLen;
        memcpy(req.inKeyVal, inKeyVal, inKeyValLen);
        reqLen = offsetof(ImportKeyReq_t, inKeyVal) + (uint16_t)inKeyValLen;

        ret = RpcHsmCall(E_HSM_MSGTYPE_IMPORT_KEY, (uint8_t*)&req, reqLen, respData, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        resp = (CommonResp_t *)respData;
        if (resp->result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp->result);
            ret = -1;
            break;
        }

        LOG_Info("[%s] success", __func__);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int RpcHsmQueryKeyExists(uint16_t keyId)
{
    int32_t ret = -1;
    CommonResp_t resp;
    uint16_t respLen = sizeof(resp);

    LOG_Warn("[%s] enter", __func__);
    do {
        LOG_Info("[%s] keyId=0x%04X", __func__, keyId);

        ret = RpcHsmCall(E_HSM_MSGTYPE_QUERY_KEY_EXISTS, (uint8_t*)&keyId, sizeof(uint16_t), (uint8_t*)&resp, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        if (resp.result != 0 && resp.result != 1) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp.result);
            ret = resp.result;
            break;
        }

        LOG_Info("[%s] exit(%d)", __func__, resp.result);
        ret = resp.result;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmQueryCrt(QueryCrtResult_t *crtResult)
{
    int32_t ret = -1;
    uint8_t respData[sizeof(QueryCrtResp_t)] = {0};
    uint16_t respLen = sizeof(respData);
    uint8_t dummyReq = 0;
    QueryCrtResp_t *resp;

    LOG_Warn("[%s] enter", __func__);
    do {
        if (crtResult == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        ret = RpcHsmCall(E_HSM_MSGTYPE_QUERY_CRT, &dummyReq, sizeof(uint8_t), respData, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        resp = (QueryCrtResp_t *)respData;
        if (resp->result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp->result);
            ret = -1;
            break;
        }

        memset(crtResult, 0, sizeof(QueryCrtResult_t));
        crtResult->validCrtCount = resp->validCrtCount;
        if (crtResult->validCrtCount > CRT_SLOT_COUNT) {
            crtResult->validCrtCount = CRT_SLOT_COUNT;
        }
        memcpy(crtResult->crtList, resp->crtList, crtResult->validCrtCount * sizeof(uint16_t));

        LOG_Info("[%s] success, validCrtCount=%u", __func__, crtResult->validCrtCount);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmDeleteCrt(uint16_t crtId)
{
    int32_t ret = -1;
    CommonResp_t resp;
    uint16_t respLen = sizeof(resp);

    LOG_Warn("[%s] enter", __func__);
    do {
        LOG_Info("[%s] crtId=0x%04X", __func__, crtId);

        ret = RpcHsmCall(E_HSM_MSGTYPE_DELETE_CRT, (uint8_t*)&crtId, sizeof(uint16_t), (uint8_t*)&resp, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        if (resp.result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp.result);
            ret = resp.result;
            break;
        }

        LOG_Info("[%s] success", __func__);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmExportCrt(uint16_t exCrtId, uint8_t *outBuf, uint32_t *outBufLen)
{
    int32_t ret = -1;
    ExportCrtReq_t req = {0};
    uint8_t respData[sizeof(ExportCrtResp_t)] = {0};
    uint16_t respLen = sizeof(respData);
    ExportCrtResp_t *resp;

    LOG_Warn("[%s] enter", __func__);
    do {
        if (outBuf == NULL || outBufLen == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        LOG_Info("[%s] crtId=0x%04X", __func__, exCrtId);

        req.crtId = exCrtId;

        ret = RpcHsmCall(E_HSM_MSGTYPE_EXPORT_CRT, (uint8_t*)&req, sizeof(ExportCrtReq_t), respData, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        resp = (ExportCrtResp_t *)respData;
        if (resp->result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp->result);
            ret = -1;
            break;
        }

        *outBufLen = resp->outBufLen;
        memcpy(outBuf, resp->outBuf, resp->outBufLen);

        LOG_Info("[%s] success, len=%u", __func__, *outBufLen);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmImportCrt(uint16_t inCrtId, const uint8_t *inCrtVal, uint32_t inCrtValLen)
{
    int32_t ret = -1;
    ImportCrtReq_t req = {0};
    uint8_t respData[sizeof(CommonResp_t)] = {0};
    uint16_t respLen = sizeof(respData);
    CommonResp_t *resp;

    LOG_Warn("[%s] enter", __func__);
    do {
        if (inCrtVal == NULL || inCrtValLen == 0 || inCrtValLen > IMPORT_CRT_MAX_LEN) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        LOG_Info("[%s] crtId=0x%04X, crtValLen=%u", __func__, inCrtId, inCrtValLen);

        req.crtId = inCrtId;
        req.crtValLen = inCrtValLen;
        memcpy(req.crtVal, inCrtVal, inCrtValLen);

        ret = RpcHsmCall(E_HSM_MSGTYPE_IMPORT_CRT, (uint8_t*)&req, sizeof(ImportCrtReq_t), respData, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        resp = (CommonResp_t *)respData;
        if (resp->result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp->result);
            ret = -1;
            break;
        }

        LOG_Info("[%s] success", __func__);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

int32_t RpcHsmGetWorkState(uint8_t *state)
{
    int32_t ret = -1;
    HsmWorkStateResp_t resp;
    uint16_t respLen = sizeof(resp);
    uint8_t dummyReq = 0;

    LOG_Warn("[%s] enter", __func__);
    do {
        if (state == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        ret = RpcHsmCall(E_HSM_MSGTYPE_GET_WORK_STATE, &dummyReq, sizeof(uint8_t), (uint8_t*)&resp, &respLen);
        if (ret != 0) {
            LOG_Error("[%s] RPC call failed: %d", __func__, ret);
            ret = -2;
            break;
        }

        if (resp.result != 0) {
            LOG_Error("[%s] Server returned error: %d", __func__, resp.result);
            ret = resp.result;
            break;
        }

        *state = resp.state;
        LOG_Info("[%s] success, state=0x%02X", __func__, *state);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}
