#include "hsm_openssl_provider.h"

static const OSSL_ALGORITHM g_hsmOpensslProviderKeyMgmtAlgs[] = {
    { "RSA", SIRUN_SSL_PROVIDER_DEFINATION, g_hsmOpensslProviderKeyMgmtDispatch,   "HSM RSA Key Management" },
    { NULL, NULL, NULL, NULL }
};

static const OSSL_ALGORITHM g_hsmOpensslProviderAsymCipherAlgs[] = {
    { "RSA", SIRUN_SSL_PROVIDER_DEFINATION, g_hsmOpensslProviderAsymCipherDispatch, "HSM RSA Asymmetric Cipher" },
    { NULL, NULL, NULL, NULL }
};

static const OSSL_ALGORITHM g_hsmOpensslProviderRandAlgs[] = {
    { "CTR-DRBG", SIRUN_SSL_PROVIDER_DEFINATION, g_hsmOpensslProviderRandDispatch, "HSM TRNG" },
    { NULL, NULL, NULL, NULL }
};

static const OSSL_ALGORITHM g_hsmOpensslProviderSignatureAlgs[] = {
    { "RSA", SIRUN_SSL_PROVIDER_DEFINATION, g_hsmOpensslProviderSignatureDispatch,  "HSM RSA Signature" },
    { NULL, NULL, NULL, NULL }
};

static const OSSL_ALGORITHM *HsmQueryOperation(void *provctx, int operationId, int *no_cache)
{
    // LOG_Warn("[%s] enter(operationId:%d)", __func__, operationId);

    (void)provctx;
    *no_cache = 0;

    switch (operationId) {
        case OSSL_OP_KEYMGMT:
            return g_hsmOpensslProviderKeyMgmtAlgs;
        case OSSL_OP_SIGNATURE:
            return g_hsmOpensslProviderSignatureAlgs;
        case OSSL_OP_ASYM_CIPHER:
            return g_hsmOpensslProviderAsymCipherAlgs;
        case OSSL_OP_RAND:
            return g_hsmOpensslProviderRandAlgs;
        default:
            return NULL;
    }
}

static void HsmProviderTeardown(void *provctx)
{
    LOG_Warn("[%s] enter \n", __func__);

    if (provctx)
        OPENSSL_free(provctx);
}

static const OSSL_DISPATCH g_hsmProviderDispatch[] = {
    {OSSL_FUNC_PROVIDER_QUERY_OPERATION,  (void (*)(void))HsmQueryOperation },
    {OSSL_FUNC_PROVIDER_TEARDOWN,         (void (*)(void))HsmProviderTeardown },
    { 0, NULL }
};

int OSSL_provider_init(const OSSL_CORE_HANDLE *handle, const OSSL_DISPATCH *in, const OSSL_DISPATCH **out, void **provctx)
{
    (void)handle;
    (void)in;

    LOG_INIT("demo");

    LOG_Warn("[%s] enter \n", __func__);

    *provctx = OPENSSL_zalloc(1);
    if (!*provctx)
    {
        return 0;
    }
    else
    {
        *out = g_hsmProviderDispatch;
        return 1;
    }
}

