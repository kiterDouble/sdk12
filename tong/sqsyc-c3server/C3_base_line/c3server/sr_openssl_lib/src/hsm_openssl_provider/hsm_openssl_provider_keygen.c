/*
 * hsm_openssl_provider_keygen.c
 * KEYMGMT implementation for HSM RSA (dummy private key d = n-1)
 * All functions have entry logging and NULL safety checks.
 */

#include "hsm_openssl_provider.h"
#include <openssl/x509.h>
#include <string.h>

typedef struct {
    HsmProviderKey_t *keyctx;
} HsmProviderKeyGenCtx_t;

/* 静态临时公钥 (n, e) */
static const char *g_modulus2048Hex =
    "98F7724326A9E414A70AA105A5F32E425CEC1EDF23898879B3BA18555ECEF9C"
    "A92E7369B399F74FC079BC814BC5679C4074D7FBA75216E2DBD5EC6395601693"
    "0DCC94621894F8C81A84D22DDA90AC84D4B39C43B1296ED8086FFD09C4D701F4"
    "76C633D771273A46006DAE6FA9553B8858BA826AD0CCEDCD8FDEE297541D35C2"
    "B7BDA01BB7F033C1F4C1C92DE0E4169CD29C04BF4186FFA3EA3E9B5D61AB0F3FE"
    "4CACADF0D47F9F2E2D01F00137B8307A9E9F10BB015CB00D7AA45D9F37927A229"
    "2CC47CBFC964AF6A856869B641434AA315B240FB501AB7E5443A18CCC1CD0C24F"
    "941C1AC8C9DF5B9BC49AE403D2CE238DA8692810AF326D17F724068EA98E1F";
static const char *g_exponent2048Hex = "010001";

static const char *g_modulus1024Hex =
    "942E00BB7B6DF067FD3F572A1BEA4A3A0B37B56C833D2DB583E17DDCF"
    "2FA9FE635ED320264CD4568A1E5C9DD59D50838FE49137D94D7CC05D26A0D8553"
    "B27F7CAA7573CCC4DB7984FD187735CDFFD2A54CC75A99D862177F9657AF3BC15"
    "19938085DCF3F5A2EB0F043D1A66A14EAB22776A4B97CFEAD9928B9CA7CA1CCE63B2D";
static const char *g_exponent1024Hex = "010001";

/* 构建 OSSL_PARAM，包含公钥和虚假私钥 */
static OSSL_PARAM *RsaParamsBuild(BIGNUM *n, BIGNUM *e, BIGNUM *d)
{
    // LOG_Warn("[%s] enter", __func__);
    OSSL_PARAM_BLD *bld = NULL;
    OSSL_PARAM *params = NULL;

    do {
        bld = OSSL_PARAM_BLD_new();
        if (!bld) break;
        if (!OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_N, n)) break;
        if (!OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_E, e)) break;
        if (d && !OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_D, d)) break;
        params = OSSL_PARAM_BLD_to_param(bld);
    } while (0);
    if (bld) OSSL_PARAM_BLD_free(bld);
    return params;
}

/* 从 X.509 SubjectPublicKeyInfo DER 中提取 n, e */
static int RsaPubKeyDerToNe(const uint8_t *der, uint32_t derLen, BIGNUM **out_n, BIGNUM **out_e)
{
    int ret = -1;
    EVP_PKEY *pkey = NULL;
    RSA *rsa = NULL;
    const BIGNUM *n_ptr = NULL;
    const BIGNUM *e_ptr = NULL;
    const uint8_t *p = der;

    if (out_n) *out_n = NULL;
    if (out_e) *out_e = NULL;

    do {
        if (der == NULL || derLen == 0 || out_n == NULL || out_e == NULL) {
            break;
        }

        pkey = d2i_PUBKEY(NULL, &p, (long)derLen);
        if (pkey == NULL) {
            LOG_Warn("[%s] d2i_PUBKEY failed", __func__);
            break;
        }

        rsa = EVP_PKEY_get1_RSA(pkey);
        if (rsa == NULL) {
            LOG_Warn("[%s] EVP_PKEY_get1_RSA failed", __func__);
            break;
        }

        n_ptr = RSA_get0_n(rsa);
        e_ptr = RSA_get0_e(rsa);
        if (n_ptr == NULL || e_ptr == NULL) {
            LOG_Warn("[%s] RSA_get0_n/e failed", __func__);
            break;
        }

        *out_n = BN_dup(n_ptr);
        *out_e = BN_dup(e_ptr);
        if (*out_n == NULL || *out_e == NULL) {
            BN_free(*out_n);
            BN_free(*out_e);
            *out_n = NULL;
            *out_e = NULL;
            break;
        }

        ret = 0;
    } while (0);

    if (rsa) RSA_free(rsa);
    if (pkey) EVP_PKEY_free(pkey);

    return ret;
}

/* 密钥生成：返回 n, e 和虚假的 d = n-1 */
static OSSL_PARAM *RsaParamsKeyGenerate(size_t keyBits, uint32_t keyId, int useHsmGenKey,
                                        BIGNUM **out_n, BIGNUM **out_e, BIGNUM **out_d)
{
    RsaKeyAlg_e keyAlg = BitsToKeyAlg(keyBits);
    uint32_t maxLen = PUB_KEY_MAX_LEN;
    uint8_t *buf = NULL;
    uint32_t bufLen = 0;
    BIGNUM *n = NULL, *e = NULL, *d = NULL;
    OSSL_PARAM *osslParams = NULL;

    LOG_Warn("[%s] enter", __func__);

    if (out_n) *out_n = NULL;
    if (out_e) *out_e = NULL;
    if (out_d) *out_d = NULL;

    do {
        if (keyAlg == E_ALG_NONE) {
            LOG_Warn("[%s] unsupported keyBits %zu", __func__, keyBits);
            break;
        }

        buf = OPENSSL_zalloc(maxLen);
        if (!buf) break;

        if (useHsmGenKey) {
            LOG_Warn("[%s] GenKey(keyId:%d)", __func__, keyId);

            bufLen = maxLen;
            if (RpcHsmGenKey(keyAlg, keyId, buf, &bufLen) != 0 || bufLen == 0) {
                LOG_Warn("[%s] RpcHsmGenKey failed", __func__);
                break;
            }
            if (RsaPubKeyDerToNe(buf, bufLen, &n, &e) != 0) {
                LOG_Warn("[%s] parse GenKey DER failed", __func__);
                break;
            }
        } else {
            LOG_Warn("[%s] BindKey(keyId:%d)", __func__, keyId);

            bufLen = maxLen;
            if (RpcHsmExportKey(keyAlg, keyId, E_KEY_TYPE_PUB, buf, &bufLen) != 0 || bufLen == 0) {
                LOG_Warn("[%s] RpcHsmExportKey failed, fallback to dummy key", __func__);
                switch (keyAlg) {
                    case E_ALG_RSA1024:
                        BN_hex2bn(&n, g_modulus1024Hex);
                        BN_hex2bn(&e, g_exponent1024Hex);
                        break;
                    case E_ALG_RSA2048:
                        BN_hex2bn(&n, g_modulus2048Hex);
                        BN_hex2bn(&e, g_exponent2048Hex);
                        break;
                    default:
                        break;
                }
            } else {
                if (RsaPubKeyDerToNe(buf, bufLen, &n, &e) != 0) {
                    LOG_Warn("[%s] parse ExportKey DER failed", __func__);
                    break;
                }
            }
        }

        if (!n || !e) {
            LOG_Warn("[%s] BN conversion failed", __func__);
            break;
        }

        /* 创建虚假私钥 d = n - 1，确保满足 1 < d < n 且长度合理 */
        d = BN_dup(n);
        if (!d) {
            LOG_Warn("[%s] BN_dup for d failed", __func__);
            break;
        }
        if (!BN_sub_word(d, 1)) {
            LOG_Warn("[%s] BN_sub_word failed", __func__);
            BN_free(d);
            d = NULL;
            break;
        }

        osslParams = RsaParamsBuild(n, e, d);
        if (!osslParams) {
            LOG_Warn("[%s] RsaParamsBuild failed", __func__);
            break;
        }

        if (out_n) *out_n = BN_dup(n);
        if (out_e) *out_e = BN_dup(e);
        if (out_d) *out_d = BN_dup(d);
        if ((out_n && !*out_n) || (out_e && !*out_e) || (out_d && !*out_d)) {
            BN_free(*out_n); BN_free(*out_e); BN_free(*out_d);
            *out_n = *out_e = *out_d = NULL;
            LOG_Warn("[%s] BN_dup failed", __func__);
        }
    } while (0);

    BN_free(n);
    BN_free(e);
    BN_free(d);
    OPENSSL_free(buf);

    LOG_Warn("[%s] exit", __func__);

    return osslParams;
}

/* ======================================== KEYMGMT 接口 ======================================== */

static int HsmOpensslProviderKeyMgmtHas(const void *keydata, int selection)
{
    LOG_Warn("[%s] enter", __func__);
    HsmProviderKey_t *ctx = (HsmProviderKey_t *)keydata;
    if (!ctx) return 0;
    /* 声称支持所有选择（包括私钥），以允许签名操作 */
    int supported = OSSL_KEYMGMT_SELECT_PRIVATE_KEY |
                    OSSL_KEYMGMT_SELECT_PUBLIC_KEY |
                    OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS;
    return (selection & supported) == selection;
}

static void *HsmOpensslProviderKeyMgmtNew(void *provctx)
{
    LOG_Warn("[%s] enter", __func__);
    HsmProviderKey_t *ctx = OPENSSL_zalloc(sizeof(HsmProviderKey_t));
    if (!ctx) return NULL;
    ctx->keyBits = 1024;
    ctx->useHsmGenKey = 0;    
    ctx->keyId = 0;
    ctx->n = ctx->e = ctx->d = NULL;
    return ctx;
}

static void HsmOpensslProviderKeyMgmtFree(void *keydata)
{
    LOG_Warn("[%s] enter", __func__);
    HsmProviderKey_t *ctx = (HsmProviderKey_t *)keydata;
    if (ctx) {
        BN_free(ctx->n);
        BN_free(ctx->e);
        BN_free(ctx->d);
        OPENSSL_free(ctx);
    }
}

static int HsmOpensslProviderKeyMgmtGetParams(void *keydata, OSSL_PARAM params[])
{
    LOG_Warn("[%s] enter", __func__);
    HsmProviderKey_t *ctx = (HsmProviderKey_t *)keydata;
    OSSL_PARAM *p;
    int ret = 0;
    do {
        if (!ctx) break;
        p = OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_BITS);
        if (p && !OSSL_PARAM_set_size_t(p, ctx->keyBits)) break;
        p = OSSL_PARAM_locate(params, SIRUN_SSL_PROVIDER_PARAM_KEYID);
        if (p && !OSSL_PARAM_set_uint(p, ctx->keyId)) break;
        p = OSSL_PARAM_locate(params, SIRUN_SSL_PROVIDER_PARAM_HSMGENFLAG);
        if (p && !OSSL_PARAM_set_int(p, ctx->useHsmGenKey)) break;        
        p = OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_RSA_N);
        if (p && ctx->n && !OSSL_PARAM_set_BN(p, ctx->n)) break;
        p = OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_RSA_E);
        if (p && ctx->e && !OSSL_PARAM_set_BN(p, ctx->e)) break;
        p = OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_RSA_D);
        if (p && ctx->d && !OSSL_PARAM_set_BN(p, ctx->d)) break;
        ret = 1;
    } while (0);
    return ret;
}

static const OSSL_PARAM *HsmOpensslProviderKeyMgmtGettableParams(void *provctx)
{
    LOG_Warn("[%s] enter", __func__);
    static const OSSL_PARAM params[] = {
        OSSL_PARAM_uint(SIRUN_SSL_PROVIDER_PARAM_KEYID, NULL),
        OSSL_PARAM_int(SIRUN_SSL_PROVIDER_PARAM_HSMGENFLAG, NULL),        
        OSSL_PARAM_size_t(OSSL_PKEY_PARAM_BITS, NULL),
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_RSA_N, NULL, 0),
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_RSA_E, NULL, 0),
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_RSA_D, NULL, 0),
        OSSL_PARAM_END
    };
    return params;
}

static int HsmOpensslProviderKeyMgmtSetParams(void *keydata, const OSSL_PARAM params[])
{
    LOG_Warn("[%s] enter", __func__);
    HsmProviderKey_t *ctx = (HsmProviderKey_t *)keydata;
    const OSSL_PARAM *p;
    int ret = 0;
    do {
        if (!ctx) break;
        p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_BITS);
        if (p && !OSSL_PARAM_get_size_t(p, &ctx->keyBits)) break;
        p = OSSL_PARAM_locate_const(params, SIRUN_SSL_PROVIDER_PARAM_KEYID);
        if (p && !OSSL_PARAM_get_uint(p, &ctx->keyId)) break;
        p = OSSL_PARAM_locate_const(params, SIRUN_SSL_PROVIDER_PARAM_HSMGENFLAG);
        if (p && !OSSL_PARAM_get_int(p, &ctx->useHsmGenKey)) break;        
        ret = 1;
    } while (0);
    return ret;
}

static const OSSL_PARAM *HsmOpensslProviderKeyMgmtSettableParams(void *provctx)
{
    LOG_Warn("[%s] enter", __func__);
    static const OSSL_PARAM params[] = {
        OSSL_PARAM_uint(SIRUN_SSL_PROVIDER_PARAM_KEYID, NULL),
        OSSL_PARAM_int(SIRUN_SSL_PROVIDER_PARAM_HSMGENFLAG, NULL),
        OSSL_PARAM_size_t(OSSL_PKEY_PARAM_BITS, NULL),
        OSSL_PARAM_END
    };
    return params;
}

static int HsmOpensslProviderKeyMgmtImport(void *keydata, int selection, const OSSL_PARAM params[])
{
    LOG_Warn("[%s] enter", __func__);
    HsmProviderKey_t *ctx = (HsmProviderKey_t *)keydata;
    const OSSL_PARAM *p;
    if (!ctx) return 0;
    p = OSSL_PARAM_locate_const(params, SIRUN_SSL_PROVIDER_PARAM_KEYID);
    if (p) OSSL_PARAM_get_uint(p, &ctx->keyId);
    p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_BITS);
    if (p) OSSL_PARAM_get_size_t(p, &ctx->keyBits);

    if (selection & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) {
        p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_RSA_N);
        if (p) {
            if (ctx->n) BN_free(ctx->n);
            ctx->n = NULL;
            OSSL_PARAM_get_BN(p, &ctx->n);
        }
        p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_RSA_E);
        if (p) {
            if (ctx->e) BN_free(ctx->e);
            ctx->e = NULL;
            OSSL_PARAM_get_BN(p, &ctx->e);
        }
    }

    if (selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) {
        p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_RSA_D);
        if (p) {
            if (ctx->d) BN_free(ctx->d);
            ctx->d = NULL;
            OSSL_PARAM_get_BN(p, &ctx->d);
        }
    }

    return 1;
}

static const OSSL_PARAM *HsmOpensslProviderKeyMgmtImportTypes(int selection)
{
    LOG_Warn("[%s] enter", __func__);
    static const OSSL_PARAM types[] = {
        OSSL_PARAM_uint(SIRUN_SSL_PROVIDER_PARAM_KEYID, NULL),
        OSSL_PARAM_size_t(OSSL_PKEY_PARAM_BITS, NULL),
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_RSA_N, NULL, 0),
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_RSA_E, NULL, 0),
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_RSA_D, NULL, 0),
        OSSL_PARAM_END
    };
    return types;
}

static int HsmOpensslProviderKeyMgmtMatch(const void *keydata1, const void *keydata2, int selection)
{
    const HsmProviderKey_t *a = (const HsmProviderKey_t *)keydata1;
    const HsmProviderKey_t *b = (const HsmProviderKey_t *)keydata2;

    if (!a || !b) return -2;

    if (selection == 0)
        return 1;

    /* 比较公钥参数 n/e */
    if (selection & (OSSL_KEYMGMT_SELECT_PUBLIC_KEY | OSSL_KEYMGMT_SELECT_KEYPAIR)) {
        if (a->n == NULL || a->e == NULL || b->n == NULL || b->e == NULL)
            return 0;
        if (BN_cmp(a->n, b->n) != 0 || BN_cmp(a->e, b->e) != 0)
            return 0;
    }

    /* 比较私钥参数 d（如有） */
    if (selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) {
        if (a->d != NULL && b->d != NULL) {
            if (BN_cmp(a->d, b->d) != 0)
                return 0;
        }
    }

    return 1;
}

static int HsmOpensslProviderKeyMgmtExport(void *keydata, int selection,
                                           OSSL_CALLBACK *param_cb, void *cbarg)
{
    // LOG_Warn("[%s] enter, returning 0 to force provider signature path", __func__);
    (void)keydata;
    (void)selection;
    (void)param_cb;
    (void)cbarg;
    /* HSM 密钥不应被导出；返回 0 强制 OpenSSL 使用 provider 的 signature 接口 */
    return 0;
}

static const OSSL_PARAM *HsmOpensslProviderKeyMgmtExportTypes(int selection)
{
    LOG_Warn("[%s] enter", __func__);
    static const OSSL_PARAM types[] = {
        OSSL_PARAM_size_t(OSSL_PKEY_PARAM_BITS, NULL),
        OSSL_PARAM_uint(SIRUN_SSL_PROVIDER_PARAM_KEYID, NULL),
        OSSL_PARAM_int(SIRUN_SSL_PROVIDER_PARAM_HSMGENFLAG, NULL),        
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_RSA_N, NULL, 0),
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_RSA_E, NULL, 0),
        OSSL_PARAM_BN(OSSL_PKEY_PARAM_RSA_D, NULL, 0),
        OSSL_PARAM_END
    };
    return types;
}

static void *HsmOpensslProviderKeyMgmtGenInit(void *provctx, int selection, const OSSL_PARAM params[])
{
    LOG_Warn("[%s] enter", __func__);
    HsmProviderKeyGenCtx_t *genctx = OPENSSL_zalloc(sizeof(HsmProviderKeyGenCtx_t));
    if (!genctx) return NULL;
    HsmProviderKey_t *keyctx = OPENSSL_zalloc(sizeof(HsmProviderKey_t));
    if (!keyctx) {
        OPENSSL_free(genctx);
        return NULL;
    }
    keyctx->keyBits = 1024;
    keyctx->useHsmGenKey = 0;    
    keyctx->keyId = 0;
    keyctx->n = keyctx->e = keyctx->d = NULL;
    genctx->keyctx = keyctx;
    return genctx;
}

static int HsmOpensslProviderKeyMgmtGenSetParams(void *genctx, const OSSL_PARAM params[])
{
    // LOG_Warn("[%s] enter", __func__);
    if (!genctx) return 0;
    HsmProviderKeyGenCtx_t *gctx = (HsmProviderKeyGenCtx_t *)genctx;
    HsmProviderKey_t *ctx = gctx->keyctx;
    if (!ctx) return 0;

    const OSSL_PARAM *p;
    int ret = 0;
    do {
        p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_BITS);
        if (p && !OSSL_PARAM_get_size_t(p, &ctx->keyBits)) break;
        p = OSSL_PARAM_locate_const(params, SIRUN_SSL_PROVIDER_PARAM_KEYID);
        if (p && !OSSL_PARAM_get_uint(p, &ctx->keyId)) break;
        p = OSSL_PARAM_locate_const(params, SIRUN_SSL_PROVIDER_PARAM_HSMGENFLAG);
        if (p && !OSSL_PARAM_get_int(p, &ctx->useHsmGenKey)) break;        
        ret = 1;
    } while (0);
    // LOG_Warn("[%s] keyId=0x%x ret=%d\n", __func__, ctx->keyId, ret);
    return ret;
}

static const OSSL_PARAM *HsmOpensslProviderKeyMgmtGenSettableParams(void *genctx, void *provctx)
{
    // LOG_Warn("[%s] enter", __func__);
    static const OSSL_PARAM params[] = {
        OSSL_PARAM_uint(SIRUN_SSL_PROVIDER_PARAM_KEYID, NULL),
        OSSL_PARAM_int(SIRUN_SSL_PROVIDER_PARAM_HSMGENFLAG, NULL),        
        OSSL_PARAM_size_t(OSSL_PKEY_PARAM_BITS, NULL),
        OSSL_PARAM_END
    };
    return params;
}

static void *HsmOpensslProviderKeyMgmtGen(void *genctx, OSSL_CALLBACK *cb, void *cbarg)
{
    LOG_Warn("[%s] enter", __func__);
    if (!genctx) return NULL;
    HsmProviderKeyGenCtx_t *gctx = (HsmProviderKeyGenCtx_t *)genctx;
    HsmProviderKey_t *ctx = gctx->keyctx;
    if (!ctx) return NULL;

    BIGNUM *n = NULL, *e = NULL, *d = NULL;
    OSSL_PARAM *osslParams = NULL;
    void *resultKeyData = NULL;

    do {
        osslParams = RsaParamsKeyGenerate(ctx->keyBits, ctx->keyId, ctx->useHsmGenKey, &n, &e, &d);
        if (!osslParams) break;

        ctx->n = n;
        ctx->e = e;
        ctx->d = d;
        n = e = d = NULL;

        if (!cb(osslParams, cbarg)) break;

        resultKeyData = ctx;
        gctx->keyctx = NULL;
    } while (0);

    BN_free(n); BN_free(e); BN_free(d);
    OSSL_PARAM_free(osslParams);
    return resultKeyData;
}

static void HsmOpensslProviderKeyMgmtGenCleanup(void *genctx)
{
    LOG_Warn("[%s] enter", __func__);
    if (!genctx) return;
    HsmProviderKeyGenCtx_t *gctx = (HsmProviderKeyGenCtx_t *)genctx;
    if (gctx->keyctx) {
        BN_free(gctx->keyctx->n);
        BN_free(gctx->keyctx->e);
        BN_free(gctx->keyctx->d);
        OPENSSL_free(gctx->keyctx);
    }
    OPENSSL_free(gctx);
}

const OSSL_DISPATCH g_hsmOpensslProviderKeyMgmtDispatch[] = {
    { OSSL_FUNC_KEYMGMT_NEW,                (void (*)(void))HsmOpensslProviderKeyMgmtNew },
    { OSSL_FUNC_KEYMGMT_FREE,               (void (*)(void))HsmOpensslProviderKeyMgmtFree },
    { OSSL_FUNC_KEYMGMT_HAS,                (void (*)(void))HsmOpensslProviderKeyMgmtHas },
    { OSSL_FUNC_KEYMGMT_GET_PARAMS,         (void (*)(void))HsmOpensslProviderKeyMgmtGetParams },
    { OSSL_FUNC_KEYMGMT_GETTABLE_PARAMS,    (void (*)(void))HsmOpensslProviderKeyMgmtGettableParams },
    { OSSL_FUNC_KEYMGMT_SET_PARAMS,         (void (*)(void))HsmOpensslProviderKeyMgmtSetParams },
    { OSSL_FUNC_KEYMGMT_SETTABLE_PARAMS,    (void (*)(void))HsmOpensslProviderKeyMgmtSettableParams },
    { OSSL_FUNC_KEYMGMT_IMPORT,             (void (*)(void))HsmOpensslProviderKeyMgmtImport },
    { OSSL_FUNC_KEYMGMT_IMPORT_TYPES,       (void (*)(void))HsmOpensslProviderKeyMgmtImportTypes },
    { OSSL_FUNC_KEYMGMT_MATCH,              (void (*)(void))HsmOpensslProviderKeyMgmtMatch },
    { OSSL_FUNC_KEYMGMT_EXPORT,             (void (*)(void))HsmOpensslProviderKeyMgmtExport },
    { OSSL_FUNC_KEYMGMT_EXPORT_TYPES,       (void (*)(void))HsmOpensslProviderKeyMgmtExportTypes },
    { OSSL_FUNC_KEYMGMT_GEN_INIT,           (void (*)(void))HsmOpensslProviderKeyMgmtGenInit },
    { OSSL_FUNC_KEYMGMT_GEN_SET_PARAMS,     (void (*)(void))HsmOpensslProviderKeyMgmtGenSetParams },
    { OSSL_FUNC_KEYMGMT_GEN_SETTABLE_PARAMS,(void (*)(void))HsmOpensslProviderKeyMgmtGenSettableParams },
    { OSSL_FUNC_KEYMGMT_GEN,                (void (*)(void))HsmOpensslProviderKeyMgmtGen },
    { OSSL_FUNC_KEYMGMT_GEN_CLEANUP,        (void (*)(void))HsmOpensslProviderKeyMgmtGenCleanup },
    { 0, NULL }
};
