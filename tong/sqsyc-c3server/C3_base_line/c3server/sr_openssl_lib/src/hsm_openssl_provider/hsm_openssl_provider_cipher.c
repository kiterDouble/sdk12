#include "hsm_openssl_provider.h"

typedef struct {
    HsmProviderKey_t *key;
    int padding;
    const EVP_MD *oaep_md;
    const EVP_MD *mgf1_md;
} HsmProviderCipherCtx_t;

static void *HsmProviderCipherNewCtx(void *provctx, const char *propq)
{
    HsmProviderCipherCtx_t *ctx = NULL;

    LOG_Warn("[%s] enter", __func__);

    do {
        ctx = OPENSSL_zalloc(sizeof(HsmProviderCipherCtx_t));
        if (!ctx) {
            break;
        }
        ctx->padding = RSA_PKCS1_OAEP_PADDING;
        ctx->oaep_md = EVP_sha256();
        ctx->mgf1_md = EVP_sha256();
    } while(0);

    return ctx;
}

static void HsmProviderCipherFreeCtx(void *vctx)
{
    LOG_Warn("[%s] enter", __func__);

    OPENSSL_free(vctx);
}

static int HsmProviderCipherEncryptInit(void *vctx, void *provkey, const OSSL_PARAM params[])
{
    int ret = 0;
    HsmProviderCipherCtx_t *ctx = (HsmProviderCipherCtx_t *)vctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (!ctx || !provkey) {
            break;
        }
        ctx->key = (HsmProviderKey_t *)provkey;
        ret = 1;
    } while(0);

    return ret;
}

static int HsmProviderCipherEncrypt(void *vctx, uint8_t *out, size_t *outlen, size_t outsize, const uint8_t *in, size_t inlen)
{
    int ret = 0;
    HsmProviderCipherCtx_t *ctx = (HsmProviderCipherCtx_t *)vctx;
    HsmProviderKey_t *key = NULL;
    RsaKeyAlg_e alg = E_ALG_NONE;
    uint32_t len;
    int rc;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (!ctx || !ctx->key) {
            break;
        }
        key = ctx->key;
        alg = BitsToKeyAlg(key->keyBits);
        if (alg == E_ALG_NONE) {
            break;
        }

        /* 第一阶段：查询输出大小 */
        if (out == NULL) {
            *outlen = key->keyBits / 8;
            ret = 1;
            break;
        }
        
        len = (uint32_t)outsize;

        // LOG_DebugHex((uint8_t *)in, (int)inlen, "in");

        rc = RpcHsmEncrypt(alg, key->keyId, PaddingToEngine(ctx->padding), (uint8_t *)in, inlen, out, &len);
        *outlen = len;
        ret = (rc == 0) ? 1 : 0;
    } while(0);

    return ret;
}

static int HsmProviderCipherDecryptInit(void *vctx, void *provkey, const OSSL_PARAM params[])
{
    int ret = 0;
    HsmProviderCipherCtx_t *ctx = (HsmProviderCipherCtx_t *)vctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (!ctx || !provkey) {
            break;
        }
        ctx->key = (HsmProviderKey_t *)provkey;
        ret = 1;
    } while(0);

    return ret;
}

static int HsmProviderCipherDecrypt(void *vctx, uint8_t *out, size_t *outlen, size_t outsize, const uint8_t *in, size_t inlen)
{
    int ret = 0;
    HsmProviderCipherCtx_t *ctx = (HsmProviderCipherCtx_t *)vctx;
    HsmProviderKey_t *key = NULL;
    RsaKeyAlg_e alg = E_ALG_NONE;
    uint32_t len;
    int rc;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (!ctx || !ctx->key) {
            break;
        }

        LOG_Warn("[%s] padding: %d", __func__, ctx->padding);

        key = ctx->key;
        alg = BitsToKeyAlg(key->keyBits);
        if (alg == E_ALG_NONE) {
            break;
        }

        /* 第一阶段：查询输出大小 */
        if (out == NULL) {
            *outlen = key->keyBits / 8;
            ret = 1;
            break;
        }
        
        len = (uint32_t)outsize;

        LOG_Warn("[%s] inlen:%zu", __func__, inlen);

        rc = RpcHsmDecrypt(alg, key->keyId, PaddingToEngine(ctx->padding), in, inlen, out, &len);

        LOG_Warn("[%s] RpcHsmDecrypt rc = %d len:%u", __func__, rc, len);

        /* OAEP 回退：实际 HSM 可能返回 raw RSA 结果（256 字节，未做 OAEP 去填充），
         * 当返回长度等于模数长度时，provider 尝试用软件做 OAEP 去填充。
         * RSA_padding_check_PKCS1_OAEP_mgf1 在 OpenSSL 3.0 中 deprecated，
         * 但对于已有 raw RSA 结果需做去填充的场景，无直接 EVP 替代。 */
        if (rc == 0 && ctx->padding == RSA_PKCS1_OAEP_PADDING && len == (uint32_t)(key->keyBits / 8)) {
            int plainLen;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            plainLen = RSA_padding_check_PKCS1_OAEP_mgf1(out, (int)outsize, out, (int)len, (int)(key->keyBits / 8), NULL, 0, ctx->oaep_md, ctx->mgf1_md);
#pragma GCC diagnostic pop
            if (plainLen < 0) {
                LOG_Error("[%s] OAEP unpadding failed", __func__);
                ret = 0;
                break;
            }
            len = (uint32_t)plainLen;
            LOG_Warn("[%s] OAEP fallback applied, plainLen=%u", __func__, len);
        }

        *outlen = len;
        ret = (rc == 0) ? 1 : 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);

    return ret;
}

static int HsmProviderCipherSetCtxParams(void *vctx, const OSSL_PARAM params[])
{
    int ret = 0;
    int pad = 0;
    const OSSL_PARAM *p;
    HsmProviderCipherCtx_t *ctx = (HsmProviderCipherCtx_t *)vctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (!ctx) {
            break;
        }
        p = OSSL_PARAM_locate_const(params, OSSL_ASYM_CIPHER_PARAM_PAD_MODE);
        if (p) {
            if (!OSSL_PARAM_get_int(p, &pad)) {
                break;
            }
            ctx->padding = pad;
        }
        /* 接收并保存 OAEP 参数，用于 HSM 返回 raw RSA 结果时做软件去填充 */
        p = OSSL_PARAM_locate_const(params, OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST);
        if (p) {
            char mdname[32] = {0}, *ptr = mdname;
            if (OSSL_PARAM_get_utf8_string(p, &ptr, sizeof(mdname)) > 0) {
                const EVP_MD *md = EVP_get_digestbyname(mdname);
                if (md) ctx->oaep_md = md;
                LOG_Warn("[%s] oaep-digest=%s", __func__, mdname);
            }
        }
        p = OSSL_PARAM_locate_const(params, OSSL_ASYM_CIPHER_PARAM_MGF1_DIGEST);
        if (p) {
            char mdname[32] = {0}, *ptr = mdname;
            if (OSSL_PARAM_get_utf8_string(p, &ptr, sizeof(mdname)) > 0) {
                const EVP_MD *md = EVP_get_digestbyname(mdname);
                if (md) ctx->mgf1_md = md;
                LOG_Warn("[%s] mgf1-digest=%s", __func__, mdname);
            }
        }
        LOG_Warn("[%s] padding:%d", __func__, ctx->padding);
        ret = 1;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);

    return ret;
}

static const OSSL_PARAM *HsmProviderCipherSettableCtxParams(void *provctx, void *provkey)
{
    static const OSSL_PARAM settable[] = {
        OSSL_PARAM_int(OSSL_ASYM_CIPHER_PARAM_PAD_MODE, NULL),
        OSSL_PARAM_utf8_string(OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST, NULL, 0),
        OSSL_PARAM_utf8_string(OSSL_ASYM_CIPHER_PARAM_MGF1_DIGEST, NULL, 0),
        OSSL_PARAM_END
    };

    LOG_Warn("[%s] enter", __func__);

    return settable;
}

const OSSL_DISPATCH g_hsmOpensslProviderAsymCipherDispatch[] = {
    { OSSL_FUNC_ASYM_CIPHER_NEWCTX,              (void (*)(void))HsmProviderCipherNewCtx },
    { OSSL_FUNC_ASYM_CIPHER_FREECTX,             (void (*)(void))HsmProviderCipherFreeCtx },
    { OSSL_FUNC_ASYM_CIPHER_ENCRYPT_INIT,        (void (*)(void))HsmProviderCipherEncryptInit },
    { OSSL_FUNC_ASYM_CIPHER_ENCRYPT,             (void (*)(void))HsmProviderCipherEncrypt },
    { OSSL_FUNC_ASYM_CIPHER_DECRYPT_INIT,        (void (*)(void))HsmProviderCipherDecryptInit },
    { OSSL_FUNC_ASYM_CIPHER_DECRYPT,             (void (*)(void))HsmProviderCipherDecrypt },
    { OSSL_FUNC_ASYM_CIPHER_SET_CTX_PARAMS,      (void (*)(void))HsmProviderCipherSetCtxParams },
    { OSSL_FUNC_ASYM_CIPHER_SETTABLE_CTX_PARAMS, (void (*)(void))HsmProviderCipherSettableCtxParams },
    { 0, NULL }
};

