/*
 * hsm_openssl_provider_random.c
 * OpenSSL 3.0 RAND provider for HSM TRNG (CamelCase naming, LOG_Warn logging)
 */

#include "hsm_openssl_provider.h"
#include <string.h>
#include <openssl/rand.h>

typedef struct {
    int dummy;
} HsmProviderRandCtx_t;

static void *HsmOpensslProviderRandNewCtx(void *provctx, void *parent,
                                          const OSSL_DISPATCH *parent_calls)
{
    HsmProviderRandCtx_t *ctx = OPENSSL_zalloc(sizeof(HsmProviderRandCtx_t));
    LOG_Warn("[%s] enter", __func__);
    return ctx;
}

static void HsmOpensslProviderRandFreeCtx(void *vctx)
{
    LOG_Warn("[%s] enter", __func__);
    OPENSSL_free(vctx);
}

static int HsmOpensslProviderRandInstantiate(void *vctx, unsigned int strength,
                                             int prediction_resistance,
                                             const uint8_t *pstr, size_t pstr_len,
                                             const OSSL_PARAM params[])
{
    LOG_Warn("[%s] enter", __func__);
    return 1;
}

static int HsmOpensslProviderRandUninstantiate(void *vctx)
{
    LOG_Warn("[%s] enter", __func__);
    return 1;
}

static int HsmOpensslProviderRandGenerate(void *vctx, uint8_t *out, size_t outlen,
                                          unsigned int strength, int prediction_resistance,
                                          const uint8_t *adin, size_t adin_len)
{
    LOG_Warn("[%s] outlen=%zu", __func__, outlen);
    if (!out || outlen == 0)
        return 1;
    if (outlen > UINT32_MAX)
        return 0;
    return RpcHsmRandom((uint32_t)outlen, out) == 0;
}

static int HsmOpensslProviderRandReseed(void *vctx, int prediction_resistance,
                                        const uint8_t *ent, size_t ent_len,
                                        const uint8_t *adin, size_t adin_len)
{
    LOG_Warn("[%s] enter", __func__);
    return 1;
}

static size_t HsmOpensslProviderRandNonce(void *vctx, uint8_t *out, unsigned int strength,
                                          size_t min_len, size_t max_len)
{
    size_t len = 0;

    LOG_Warn("[%s] min=%zu max=%zu", __func__, min_len, max_len);

    if (out == NULL)
        return max_len;

    len = (max_len > 0) ? max_len : min_len;
    if (len > 0) {
        if (RpcHsmRandom((uint32_t)len, out) != 0) {
            len = 0;
        }
    }
    return len;
}

static int HsmOpensslProviderRandEnableLocking(void *vctx)
{
    LOG_Warn("[%s] enter", __func__);
    return 1;
}

static int HsmOpensslProviderRandLock(void *vctx)
{
    return 1;
}

static void HsmOpensslProviderRandUnlock(void *vctx)
{
}

static const OSSL_PARAM *HsmOpensslProviderRandGettableParams(void *provctx)
{
    static const OSSL_PARAM params[] = { OSSL_PARAM_END };
    LOG_Warn("[%s] enter", __func__);
    return params;
}

static int HsmOpensslProviderRandGetParams(OSSL_PARAM params[])
{
    LOG_Warn("[%s] enter", __func__);
    return 1;
}

static const OSSL_PARAM *HsmOpensslProviderRandGettableCtxParams(void *ctx, void *provctx)
{
    static const OSSL_PARAM params[] = {
        OSSL_PARAM_size_t(OSSL_RAND_PARAM_MAX_REQUEST, NULL),
        OSSL_PARAM_END
    };
    LOG_Warn("[%s] enter", __func__);
    return params;
}

static int HsmOpensslProviderRandGetCtxParams(void *ctx, OSSL_PARAM params[])
{
    OSSL_PARAM *p = NULL;

    LOG_Warn("[%s] enter", __func__);

    p = OSSL_PARAM_locate(params, OSSL_RAND_PARAM_MAX_REQUEST);
    if (p) {
        size_t max_request = 1024 * 1024;
        if (!OSSL_PARAM_set_size_t(p, max_request))
            return 0;
    }
    return 1;
}

static const OSSL_PARAM *HsmOpensslProviderRandSettableCtxParams(void *ctx, void *provctx)
{
    static const OSSL_PARAM params[] = { OSSL_PARAM_END };
    LOG_Warn("[%s] enter", __func__);
    return params;
}

static int HsmOpensslProviderRandSetCtxParams(void *ctx, const OSSL_PARAM params[])
{
    LOG_Warn("[%s] enter", __func__);
    return 1;
}

static int HsmOpensslProviderRandVerifyZeroization(void *vctx)
{
    LOG_Warn("[%s] enter", __func__);
    return 1;
}

static size_t HsmOpensslProviderRandGetSeed(void *vctx, uint8_t **buffer,
                                            int entropy, size_t min_len, size_t max_len,
                                            int prediction_resistance,
                                            const uint8_t *adin, size_t adin_len)
{
    LOG_Warn("[%s] enter", __func__);
    return 0;
}

static void HsmOpensslProviderRandClearSeed(void *vctx, uint8_t *buffer, size_t b_len)
{
    LOG_Warn("[%s] enter", __func__);
}

const OSSL_DISPATCH g_hsmOpensslProviderRandDispatch[] = {
    { OSSL_FUNC_RAND_NEWCTX,                (void (*)(void))HsmOpensslProviderRandNewCtx },
    { OSSL_FUNC_RAND_FREECTX,               (void (*)(void))HsmOpensslProviderRandFreeCtx },
    { OSSL_FUNC_RAND_INSTANTIATE,           (void (*)(void))HsmOpensslProviderRandInstantiate },
    { OSSL_FUNC_RAND_UNINSTANTIATE,         (void (*)(void))HsmOpensslProviderRandUninstantiate },
    { OSSL_FUNC_RAND_GENERATE,              (void (*)(void))HsmOpensslProviderRandGenerate },
    { OSSL_FUNC_RAND_RESEED,                (void (*)(void))HsmOpensslProviderRandReseed },
    { OSSL_FUNC_RAND_NONCE,                 (void (*)(void))HsmOpensslProviderRandNonce },
    { OSSL_FUNC_RAND_ENABLE_LOCKING,        (void (*)(void))HsmOpensslProviderRandEnableLocking },
    { OSSL_FUNC_RAND_LOCK,                  (void (*)(void))HsmOpensslProviderRandLock },
    { OSSL_FUNC_RAND_UNLOCK,                (void (*)(void))HsmOpensslProviderRandUnlock },
    { OSSL_FUNC_RAND_GETTABLE_PARAMS,       (void (*)(void))HsmOpensslProviderRandGettableParams },
    { OSSL_FUNC_RAND_GET_PARAMS,            (void (*)(void))HsmOpensslProviderRandGetParams },
    { OSSL_FUNC_RAND_GETTABLE_CTX_PARAMS,   (void (*)(void))HsmOpensslProviderRandGettableCtxParams },
    { OSSL_FUNC_RAND_GET_CTX_PARAMS,        (void (*)(void))HsmOpensslProviderRandGetCtxParams },
    { OSSL_FUNC_RAND_SETTABLE_CTX_PARAMS,   (void (*)(void))HsmOpensslProviderRandSettableCtxParams },
    { OSSL_FUNC_RAND_SET_CTX_PARAMS,        (void (*)(void))HsmOpensslProviderRandSetCtxParams },
    { OSSL_FUNC_RAND_VERIFY_ZEROIZATION,    (void (*)(void))HsmOpensslProviderRandVerifyZeroization },
    { OSSL_FUNC_RAND_GET_SEED,              (void (*)(void))HsmOpensslProviderRandGetSeed },
    { OSSL_FUNC_RAND_CLEAR_SEED,            (void (*)(void))HsmOpensslProviderRandClearSeed },
    { 0, NULL }
};

