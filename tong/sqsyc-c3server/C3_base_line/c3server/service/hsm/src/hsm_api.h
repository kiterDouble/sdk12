
#ifndef _HSM_API_H
#define _HSM_API_H

#include <stdint.h>
#include <stdbool.h>

#include "rpc_hsm_api.h"

typedef struct
{
    int (*init)(void* ctx);

    int (*release)(void* ctx);

    int (*random)(void* ctx, uint32_t expectedLen, uint8_t *random);

    int (*rsaKeyPairGenerate)(void* ctx, RsaKeyAlg_e alg, uint16_t keyId, uint8_t* pubKeyDer,uint32_t *pubKeyDerBufSize);

    int (*rsaSign)(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, DigistAlg_e digist, uint8_t *plain, uint32_t plainLen, uint8_t *sign, uint32_t* signBufSize);

    int (*rsaVerify)(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, DigistAlg_e digist, uint8_t *plain, uint32_t plainLen, uint8_t *sign, uint32_t signBufSize);

    int (*rsaEncrypt)(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, RsaSignPaddingAlg_e padding, uint8_t *plain, uint32_t plainLen, uint8_t *out, uint32_t* outBufSize);

    int (*rsaDecrypt)(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, RsaSignPaddingAlg_e padding, uint8_t *encryptedData, uint32_t encryptedDataLen, uint8_t *out, uint32_t* outBufSize);

 
    int (*queryKey)(void* ctx, QueryKeyResult_t *keyResult);

    int (*queryKeyExists)(void* ctx, const uint16_t keyId);

    int (*deleteKey)(void* ctx, const uint16_t keyId);

    int (*exportKey)(void* ctx, RsaKeyAlg_e exKeyAlg, const uint16_t exKeyId, uint8_t exKeyType, uint8_t *out, uint32_t *outBufSize);

    int (*importKey)(void* ctx, RsaKeyAlg_e inKeyAlg, const uint16_t inKeyId, uint8_t inKeyType, const uint8_t *inKeyVal, uint32_t inKeyValLen);


    int (*queryCrt)(void* ctx, QueryCrtResult_t *crtResult);

    int (*queryCrtExists)(void* ctx, const uint16_t crtId);

    int (*deleteCrt)(void* ctx, const uint16_t crtId);
    
    int (*exportCrt)(void* ctx, const uint16_t crtId, uint8_t *out, uint32_t *outBufSize);

    int (*importCrt)(void* ctx, const uint16_t crtId, const uint8_t *inCrtVal, uint32_t inCrtValLen);

}HsmApi_t;

extern const HsmApi_t g_hsmCIU98BV2_Api;
extern const HsmApi_t g_hsmSimulate_Api;
#ifdef HSM_TEE_SUPPORTED
extern const HsmApi_t g_hsmTee_Api;
#endif

#endif // _HSM_API_H

