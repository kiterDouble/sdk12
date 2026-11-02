#include "hsm_openssl_provider.h"

typedef struct {
    HsmProviderKey_t *key;          /* 关联的密钥对象（由 KEYMGMT 提供） */
    const EVP_MD *md;               /* 摘要算法 */
    int padding;                    /* RSA padding mode (PKCS#1, PSS, OAEP...) */
    uint8_t *tbs_buf;               /* streaming 模式下累积的待签名/验签数据 */
    size_t tbs_len;                 /* 已累积数据长度 */
    size_t tbs_cap;                 /* 缓冲区容量 */
} HsmProviderSignatureCtx_t;

static void *HsmOpensslProviderSigNewCtx(void *provctx, const char *propq)
{
    HsmProviderSignatureCtx_t *ctx = NULL;

    LOG_Warn("[%s] enter", __func__);

    do {
        ctx = OPENSSL_zalloc(sizeof(HsmProviderSignatureCtx_t));
        if (!ctx)
            break;
        ctx->padding = RSA_PKCS1_PADDING;   /* 默认 PKCS#1 v1.5 */
    } while(0);

    return ctx;
}

static void *HsmOpensslProviderSigDupCtx(void *vctx)
{
    HsmProviderSignatureCtx_t *src = vctx;
    HsmProviderSignatureCtx_t *dst = NULL;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (!src)
            break;
        dst = OPENSSL_zalloc(sizeof(HsmProviderSignatureCtx_t));
        if (!dst)
            break;
        dst->key       = src->key;
        dst->md        = src->md;
        dst->padding   = src->padding;
        dst->tbs_len   = src->tbs_len;
        dst->tbs_cap   = src->tbs_cap;
        if (src->tbs_buf && src->tbs_cap > 0) {
            dst->tbs_buf = OPENSSL_malloc(src->tbs_cap);
            if (!dst->tbs_buf)
                break;
            memcpy(dst->tbs_buf, src->tbs_buf, src->tbs_len);
        }
        LOG_Warn("[%s] dup done, keyId=0x%x", __func__, dst->key ? dst->key->keyId : 0);
        return dst;
    } while(0);

    if (dst) {
        OPENSSL_free(dst->tbs_buf);
        OPENSSL_free(dst);
    }
    return NULL;
}

static void HsmOpensslProviderSigFreeCtx(void *vctx)
{
    HsmProviderSignatureCtx_t *ctx = vctx;
    LOG_Warn("[%s] enter", __func__);

    if (ctx) {
        OPENSSL_free(ctx->tbs_buf);
        OPENSSL_free(ctx);
    }
}

static int HsmOpensslProviderSigSetCtxParams(void *vctx, const OSSL_PARAM params[])
{
    const OSSL_PARAM *p;
    HsmProviderSignatureCtx_t *ctx = vctx;
    int pad = 0;
    int ret = 0;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (!ctx || !params) {
            break;
        }
        p = OSSL_PARAM_locate_const(params, OSSL_SIGNATURE_PARAM_PAD_MODE);
        if (p) {
            if (!OSSL_PARAM_get_int(p, &pad)) {
                break;
            }
            LOG_Warn("[%s] pad_mode=%d", __func__, pad);
            ctx->padding = pad;
        }
        ret = 1;
    } while(0);

    return ret;
}

static const OSSL_PARAM *HsmOpensslProviderSigSettableCtxParams(void *provctx, void *provkey)
{
    static const OSSL_PARAM settable[] = {
        OSSL_PARAM_int(OSSL_SIGNATURE_PARAM_PAD_MODE, NULL),
        OSSL_PARAM_END
    };

    LOG_Warn("[%s] enter", __func__);

    return settable;
}

static int HsmOpensslProviderDigestSignVerifyInit(void *vctx, const char *mdname, void *provkey, const OSSL_PARAM params[])
{
    int ret = 0;
    HsmProviderSignatureCtx_t *ctx = vctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (!ctx || !provkey) {
            break;
        }
        ctx->key = (HsmProviderKey_t *)provkey;
        ctx->md = mdname ? EVP_get_digestbyname(mdname) : NULL;
        /* 每次 init 时清空累积缓冲区，支持重用 context */
        ctx->tbs_len = 0;
        LOG_Warn("[%s] md:%s, keyId:0x%x, keyBits:%zu", __func__, EVP_MD_get0_name(ctx->md), ctx->key->keyId, ctx->key->keyBits);
        if (params && !HsmOpensslProviderSigSetCtxParams(ctx, params)) {
            /* 不支持当前输入签名算法 */
            break;
        }
        ret = 1;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);

    return ret;
}

static int HsmOpensslProviderDigestSignVerifyUpdate(void *vctx, const uint8_t *data, size_t datalen)
{
    HsmProviderSignatureCtx_t *ctx = vctx;
    size_t new_cap;
    uint8_t *new_buf;

    if (!ctx || !data)
        return 0;

    if (datalen == 0)
        return 1;

    if (ctx->tbs_len + datalen > ctx->tbs_cap) {
        new_cap = ctx->tbs_cap ? ctx->tbs_cap * 2 : 1024;
        while (new_cap < ctx->tbs_len + datalen)
            new_cap *= 2;
        new_buf = OPENSSL_realloc(ctx->tbs_buf, new_cap);
        if (!new_buf)
            return 0;
        ctx->tbs_buf = new_buf;
        ctx->tbs_cap = new_cap;
    }

    memcpy(ctx->tbs_buf + ctx->tbs_len, data, datalen);
    ctx->tbs_len += datalen;
    return 1;
}

static int HsmOpensslProviderDigestSign(void *vctx, uint8_t *sig, size_t *siglen,
                                        size_t sigsize, const uint8_t *tbs, size_t tbslen)
{
    int ret = 0;
    int rc = 0;
    HsmProviderSignatureCtx_t *ctx = vctx;
    HsmProviderKey_t *key;
    uint32_t hsmSigLen;
    size_t expectLen;
    RsaKeyAlg_e keyAlg = E_ALG_NONE;
    DigistAlg_e digistAlg = E_DIGIST_NONE;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (!ctx || !ctx->key || !siglen) {
            break;
        }
        key = ctx->key;
        keyAlg = BitsToKeyAlg(key->keyBits);
        digistAlg = DigestToEngine(ctx->md);
        expectLen = key->keyBits / 8;

        LOG_Warn("[%s] md=%s padding=%d tbslen=%zu sigsize=%zu", __func__, EVP_MD_get0_name(ctx->md), ctx->padding, tbslen, sigsize);
        // LOG_DebugHex((uint8_t *)tbs, (int)tbslen, "plain");

        if (sig == NULL) {
            *siglen = expectLen;
            LOG_Warn("[%s] query siglen=%zu", __func__, *siglen);
            ret = 1;
            break;
        }
        if (sigsize < expectLen) {
            *siglen = expectLen;
            LOG_Error("[%s] buffer too small", __func__);
            break;
        }
        hsmSigLen = (uint32_t)expectLen;
        rc = RpcHsmSign(keyAlg, key->keyId, digistAlg, (uint8_t *)tbs, tbslen, sig, &hsmSigLen);
        LOG_Error("[%s] RpcHsmSign rc=%d", __func__, rc);
        if (rc == 0) {
            *siglen = hsmSigLen;
            ret = 1;
        }
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);

    return ret;
}

static int HsmOpensslProviderVerifyInternal(void *vctx, const uint8_t *sig, size_t siglen, const uint8_t *tbs, size_t tbslen, int isDigest)
{
    int ret = 0;
    int rc = 0;
    HsmProviderSignatureCtx_t *ctx = vctx;
    HsmProviderKey_t *key;
    RsaKeyAlg_e keyAlg = E_ALG_NONE;
    DigistAlg_e digistAlg = E_DIGIST_NONE;

    do {
        LOG_Warn("[%s] enter (isDigest=%d)", __func__, isDigest);
        if (!ctx || !ctx->key) {
            break;
        }
        key = ctx->key;
        keyAlg = BitsToKeyAlg(key->keyBits);
        if (isDigest && ctx->md) {
            digistAlg = DigestToEngine(ctx->md);
            LOG_Warn("[%s] md=%s padding=%d tbslen=%zu", __func__, EVP_MD_get0_name(ctx->md), ctx->padding, tbslen);
        } else {
            /* Raw RSA verify: X509_verify passes pre-hashed tbs, use SHA256 as default digest for HSM */
            digistAlg = E_DIGIST_SHA256;
            LOG_Warn("[%s] raw verify, default digist=SHA256, tbslen=%zu", __func__, tbslen);
        }
        rc = RpcHsmVerify(keyAlg, key->keyId, digistAlg, (uint8_t *)tbs, tbslen, (uint8_t *)sig, siglen);
        LOG_Warn("[%s] RpcHsmVerify rc=%d", __func__, rc);
        ret = (rc == 0) ? 1 : 0;
    } while(0);

    return ret;
}

static int HsmOpensslProviderVerifyInit(void *vctx, void *provkey, const OSSL_PARAM params[])
{
    int ret = 0;
    HsmProviderSignatureCtx_t *ctx = vctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (!ctx || !provkey) {
            break;
        }
        ctx->key = (HsmProviderKey_t *)provkey;
        /* Raw verify does not use md, but clear tbs buffer for safety */
        ctx->tbs_len = 0;
        if (params && !HsmOpensslProviderSigSetCtxParams(ctx, params)) {
            break;
        }
        ret = 1;
    } while(0);

    return ret;
}

static int HsmOpensslProviderVerify(void *vctx, const uint8_t *sig, size_t siglen, const uint8_t *tbs, size_t tbslen)
{
    return HsmOpensslProviderVerifyInternal(vctx, sig, siglen, tbs, tbslen, 0);
}

static int HsmOpensslProviderDigestVerify(void *vctx, const uint8_t *sig, size_t siglen, const uint8_t *tbs, size_t tbslen)
{
    return HsmOpensslProviderVerifyInternal(vctx, sig, siglen, tbs, tbslen, 1);
}

static int HsmOpensslProviderDigestSignFinal(void *vctx, uint8_t *sig, size_t *siglen, size_t sigsize)
{
    HsmProviderSignatureCtx_t *ctx = vctx;

    if (!ctx)
        return 0;

    return HsmOpensslProviderDigestSign(vctx, sig, siglen, sigsize,
                                        ctx->tbs_buf, ctx->tbs_len);
}

static int HsmOpensslProviderDigestVerifyFinal(void *vctx, const uint8_t *sig, size_t siglen)
{
    HsmProviderSignatureCtx_t *ctx = vctx;

    if (!ctx)
        return 0;

    return HsmOpensslProviderDigestVerify(vctx, sig, siglen,
                                          ctx->tbs_buf, ctx->tbs_len);
}

const OSSL_DISPATCH g_hsmOpensslProviderSignatureDispatch[] = {
    { OSSL_FUNC_SIGNATURE_NEWCTX,                (void (*)(void))HsmOpensslProviderSigNewCtx },
    { OSSL_FUNC_SIGNATURE_FREECTX,               (void (*)(void))HsmOpensslProviderSigFreeCtx },
    { OSSL_FUNC_SIGNATURE_DUPCTX,                (void (*)(void))HsmOpensslProviderSigDupCtx },
    { OSSL_FUNC_SIGNATURE_SET_CTX_PARAMS,        (void (*)(void))HsmOpensslProviderSigSetCtxParams },
    { OSSL_FUNC_SIGNATURE_SETTABLE_CTX_PARAMS,   (void (*)(void))HsmOpensslProviderSigSettableCtxParams },
    { OSSL_FUNC_SIGNATURE_VERIFY_INIT,           (void (*)(void))HsmOpensslProviderVerifyInit },
    { OSSL_FUNC_SIGNATURE_VERIFY,                (void (*)(void))HsmOpensslProviderVerify },
    { OSSL_FUNC_SIGNATURE_DIGEST_SIGN_INIT,      (void (*)(void))HsmOpensslProviderDigestSignVerifyInit },
    { OSSL_FUNC_SIGNATURE_DIGEST_SIGN_UPDATE,    (void (*)(void))HsmOpensslProviderDigestSignVerifyUpdate },
    { OSSL_FUNC_SIGNATURE_DIGEST_SIGN_FINAL,     (void (*)(void))HsmOpensslProviderDigestSignFinal },
    { OSSL_FUNC_SIGNATURE_DIGEST_SIGN,           (void (*)(void))HsmOpensslProviderDigestSign },
    { OSSL_FUNC_SIGNATURE_DIGEST_VERIFY_INIT,    (void (*)(void))HsmOpensslProviderDigestSignVerifyInit },
    { OSSL_FUNC_SIGNATURE_DIGEST_VERIFY_UPDATE,  (void (*)(void))HsmOpensslProviderDigestSignVerifyUpdate },
    { OSSL_FUNC_SIGNATURE_DIGEST_VERIFY_FINAL,   (void (*)(void))HsmOpensslProviderDigestVerifyFinal },
    { OSSL_FUNC_SIGNATURE_DIGEST_VERIFY,         (void (*)(void))HsmOpensslProviderDigestVerify },
    { 0, NULL }
};