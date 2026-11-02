#include <stdio.h>
#include <string.h>

#include "hsm_api.h"
#include "hsm_crt_util.h"
#include "sr_log.h"
#include "ctx.h"

// HED SDK 头文件
#include "se.h"
#include "fs.h"
#include "comm.h"
#include "key.h"
#include "port_config.h"
#include "pin.h"   
#include "crypto.h"
#include "info.h"

// OpenSSL 头文件
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>

#define HED_TRANSKEY_ID     0x02
#define HED_ADMIN_PIN_LEN   0x10

extern se_error_t api_asym_encrypt(pub_key_t *key, alg_asym_param_t *asym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);
extern se_error_t api_asym_decrypt(pri_key_t *key, alg_asym_param_t *asym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);
extern se_error_t api_asym_sign(pri_key_t *key, alg_asym_param_t *asym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *sign_buf, uint32_t *sign_buf_len);
extern se_error_t api_asym_verify(pub_key_t *key, alg_asym_param_t *asym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *sign_buf, uint32_t *sign_buf_len);
extern se_error_t api_get_random(uint32_t expected_len, uint8_t *random);

static const uint8_t g_hedAdminPin[HED_ADMIN_PIN_LEN] = {
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0
};

static int HsmParseKeyInfo(uint8_t algId, uint8_t rawBits, KeyInfo_t *keyInfo)
{
    if (keyInfo == NULL) {
        return -1;
    }

    keyInfo->keyBits = 0;

    switch (algId) {
        case 0x80: /* RSA public */
            keyInfo->keyAlg = E_HSM_KEY_ALG_RSA;
            keyInfo->keyType = E_HSM_KEY_TYPE_PUBLIC;
            keyInfo->keyBits = rawBits * 32;
            break;
        case 0x81: /* RSA_ND private */
        case 0x82: /* RSA_CRT private */
            keyInfo->keyAlg = E_HSM_KEY_ALG_RSA;
            keyInfo->keyType = E_HSM_KEY_TYPE_PRIVATE;
            keyInfo->keyBits = rawBits * 32;
            break;
        case 0x83: /* RSA_ND key pair */
        case 0x84: /* RSA_CRT key pair */
            keyInfo->keyAlg = E_HSM_KEY_ALG_RSA;
            keyInfo->keyType = E_HSM_KEY_TYPE_KEY_PAIR;
            keyInfo->keyBits = rawBits * 32;
            break;
        case 0x90: /* SM2 public */
            keyInfo->keyAlg = E_HSM_KEY_ALG_SM2;
            keyInfo->keyType = E_HSM_KEY_TYPE_PUBLIC;
            keyInfo->keyBits = rawBits;
            break;
        case 0x91: /* SM2 private */
            keyInfo->keyAlg = E_HSM_KEY_ALG_SM2;
            keyInfo->keyType = E_HSM_KEY_TYPE_PRIVATE;
            keyInfo->keyBits = rawBits;
            break;
        case 0x92: /* SM2 key pair */
            keyInfo->keyAlg = E_HSM_KEY_ALG_SM2;
            keyInfo->keyType = E_HSM_KEY_TYPE_KEY_PAIR;
            keyInfo->keyBits = rawBits;
            break;
        case 0xA0: /* ECC public */
            keyInfo->keyAlg = E_HSM_KEY_ALG_ECC;
            keyInfo->keyType = E_HSM_KEY_TYPE_PUBLIC;
            keyInfo->keyBits = rawBits;
            break;
        case 0xA1: /* ECC private */
            keyInfo->keyAlg = E_HSM_KEY_ALG_ECC;
            keyInfo->keyType = E_HSM_KEY_TYPE_PRIVATE;
            keyInfo->keyBits = rawBits;
            break;
        case 0xA2: /* ECC key pair */
            keyInfo->keyAlg = E_HSM_KEY_ALG_ECC;
            keyInfo->keyType = E_HSM_KEY_TYPE_KEY_PAIR;
            keyInfo->keyBits = rawBits;
            break;
        case 0x00: /* 3DES-128 */
        case 0x01: /* 3DES-192 */
            keyInfo->keyAlg = E_HSM_KEY_ALG_3DES;
            keyInfo->keyType = E_HSM_KEY_TYPE_SECRET;
            break;
        case 0x40: /* SM4 */
            keyInfo->keyAlg = E_HSM_KEY_ALG_SM4;
            keyInfo->keyType = E_HSM_KEY_TYPE_SECRET;
            break;
        case 0x60: /* AES-128 */
        case 0x61: /* AES-192 */
        case 0x62: /* AES-256 */
            keyInfo->keyAlg = E_HSM_KEY_ALG_AES;
            keyInfo->keyType = E_HSM_KEY_TYPE_SECRET;
            break;
        default:
            return -1;
    }

    return 0;
}

static void HsmWriteBnFixed(const BIGNUM *bn, uint8_t *buf, uint32_t len)
{
    int bnLen = 0;

    if (bn == NULL || buf == NULL || len == 0) {
        return;
    }

    bnLen = BN_num_bytes(bn);
    if (bnLen < 0) {
        bnLen = 0;
    }

    memset(buf, 0, len);
    if ((uint32_t)bnLen > 0) {
        BN_bn2bin(bn, buf + len - (uint32_t)bnLen);
    }
}

/*
 * E(4B) + N -> X.509 SubjectPublicKeyInfo DER
 */
static int HsmConvertEnToX509Der(const uint8_t *enBuf, uint32_t enLen,
                                 uint8_t *outDer, uint32_t *outDerLen)
{
    int ret = -1;
    RSA *rsa = NULL;
    BIGNUM *n = NULL;
    BIGNUM *e = NULL;
    EVP_PKEY *pkey = NULL;
    int derLen = 0;
    uint8_t *p = NULL;

    do {
        if (enBuf == NULL || outDer == NULL || outDerLen == NULL || enLen < 5) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        rsa = RSA_new();
        if (rsa == NULL) {
            LOG_Error("[%s] RSA_new failed", __func__);
            break;
        }

        n = BN_bin2bn(enBuf + 4, enLen - 4, NULL);
        e = BN_bin2bn(enBuf, 4, NULL);
        if (n == NULL || e == NULL) {
            LOG_Error("[%s] BN_bin2bn failed", __func__);
            break;
        }

        if (RSA_set0_key(rsa, n, e, NULL) != 1) {
            LOG_Error("[%s] RSA_set0_key failed", __func__);
            break;
        }
        n = NULL;
        e = NULL;

        pkey = EVP_PKEY_new();
        if (pkey == NULL || EVP_PKEY_assign_RSA(pkey, rsa) != 1) {
            LOG_Error("[%s] EVP_PKEY_assign_RSA failed", __func__);
            break;
        }
        rsa = NULL;

        derLen = i2d_PUBKEY(pkey, NULL);
        if (derLen <= 0) {
            LOG_Error("[%s] i2d_PUBKEY failed", __func__);
            break;
        }

        if ((uint32_t)derLen > *outDerLen) {
            LOG_Error("[%s] Buffer too small: %u < %d", __func__, *outDerLen, derLen);
            *outDerLen = (uint32_t)derLen;
            break;
        }

        p = outDer;
        derLen = i2d_PUBKEY(pkey, &p);
        if (derLen <= 0) {
            LOG_Error("[%s] i2d_PUBKEY write failed", __func__);
            break;
        }

        *outDerLen = (uint32_t)derLen;
        LOG_Info("[%s] Converted EN to X.509 DER, len=%u", __func__, *outDerLen);
        ret = 0;
    } while (0);

    if (rsa != NULL) {
        RSA_free(rsa);
    }
    if (n != NULL) {
        BN_clear_free(n);
    }
    if (e != NULL) {
        BN_clear_free(e);
    }
    if (pkey != NULL) {
        EVP_PKEY_free(pkey);
    }

    return ret;
}

/*
 * X.509 SubjectPublicKeyInfo DER -> E(4B) + N
 */
static int HsmConvertX509DerToEn(const uint8_t *der, uint32_t derLen,
                                 uint8_t *outEn, uint32_t *outEnLen)
{
    int ret = -1;
    RSA *rsa = NULL;
    const uint8_t *p = der;
    int nLen = 0;
    int eLen = 0;
    uint32_t totalLen = 0;

    do {
        if (der == NULL || outEn == NULL || outEnLen == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        rsa = d2i_RSA_PUBKEY(NULL, &p, derLen);
        if (rsa == NULL) {
            LOG_Error("[%s] d2i_RSA_PUBKEY failed", __func__);
            break;
        }

        nLen = BN_num_bytes(RSA_get0_n(rsa));
        eLen = BN_num_bytes(RSA_get0_e(rsa));
        if (nLen <= 0 || eLen <= 0 || eLen > 4) {
            LOG_Error("[%s] Invalid RSA key", __func__);
            break;
        }

        totalLen = 4 + (uint32_t)nLen;
        if (*outEnLen < totalLen) {
            LOG_Error("[%s] Buffer too small: %u < %u", __func__, *outEnLen, totalLen);
            *outEnLen = totalLen;
            break;
        }

        memset(outEn, 0, totalLen);
        BN_bn2bin(RSA_get0_e(rsa), outEn + 4 - eLen);
        BN_bn2bin(RSA_get0_n(rsa), outEn + 4);
        *outEnLen = totalLen;
        LOG_Info("[%s] Converted X.509 DER to EN, len=%u", __func__, *outEnLen);
        ret = 0;
    } while (0);

    if (rsa != NULL) {
        RSA_free(rsa);
    }

    return ret;
}

/*
 * HSM 裸 RSA 格式 -> PKCS#8 DER
 * 公钥: E(4B) | N(N B)
 * 私钥: E(4B) | P(N/2) | Q(N/2) | DP(N/2) | DQ(N/2) | Qinv(N/2)
 * 密钥对: E(4B) | N(N) | P(N/2) | Q(N/2) | DP(N/2) | DQ(N/2) | Qinv(N/2)
 */
static int HsmConvertHsmKeyToPkcs8(const uint8_t *inKeyVal, uint32_t inKeyValLen,
                                   uint8_t keyType, uint32_t bits,
                                   uint8_t *outPkcs8, uint32_t *outPkcs8Len)
{
    int ret = -1;
    RSA *rsa = NULL;
    BIGNUM *n = NULL;
    BIGNUM *e = NULL;
    BIGNUM *d = NULL;
    BIGNUM *p = NULL;
    BIGNUM *q = NULL;
    BIGNUM *dmp1 = NULL;
    BIGNUM *dmq1 = NULL;
    BIGNUM *iqmp = NULL;
    BIGNUM *phi = NULL;
    BIGNUM *p1 = NULL;
    BIGNUM *q1 = NULL;
    BN_CTX *bnCtx = NULL;
    EVP_PKEY *pkey = NULL;
    PKCS8_PRIV_KEY_INFO *pkcs8Info = NULL;
    int derLen = 0;
    uint32_t nLen = bits / 8;
    uint32_t halfLen = nLen / 2;
    uint8_t *pOut = NULL;

    do {
        if (inKeyVal == NULL || inKeyValLen == 0 || outPkcs8 == NULL || outPkcs8Len == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        rsa = RSA_new();
        if (rsa == NULL) {
            LOG_Error("[%s] RSA_new failed", __func__);
            break;
        }

        e = BN_bin2bn(inKeyVal, 4, NULL);
        if (e == NULL) {
            LOG_Error("[%s] BN_bin2bn e failed", __func__);
            break;
        }

        if (keyType == E_KEY_TYPE_PUB) {
            if (inKeyValLen != nLen + 4) {
                LOG_Error("[%s] public key length mismatch: %u != %u", __func__, inKeyValLen, nLen + 4);
                break;
            }
            n = BN_bin2bn(inKeyVal + 4, nLen, NULL);
            if (n == NULL || RSA_set0_key(rsa, n, e, NULL) != 1) {
                LOG_Error("[%s] RSA_set0_key failed", __func__);
                break;
            }
            n = NULL;
            e = NULL;
        } else if (keyType == E_KEY_TYPE_PRI) {
            if (inKeyValLen != halfLen * 5 + 4) {
                LOG_Error("[%s] private key length mismatch: %u != %u", __func__, inKeyValLen, halfLen * 5 + 4);
                break;
            }
            p = BN_bin2bn(inKeyVal + 4, halfLen, NULL);
            q = BN_bin2bn(inKeyVal + 4 + halfLen, halfLen, NULL);
            dmp1 = BN_bin2bn(inKeyVal + 4 + halfLen * 2, halfLen, NULL);
            dmq1 = BN_bin2bn(inKeyVal + 4 + halfLen * 3, halfLen, NULL);
            iqmp = BN_bin2bn(inKeyVal + 4 + halfLen * 4, halfLen, NULL);
            if (p == NULL || q == NULL || dmp1 == NULL || dmq1 == NULL || iqmp == NULL) {
                LOG_Error("[%s] BN_bin2bn failed", __func__);
                break;
            }

            bnCtx = BN_CTX_new();
            if (bnCtx == NULL) {
                LOG_Error("[%s] BN_CTX_new failed", __func__);
                break;
            }
            n = BN_new();
            phi = BN_new();
            p1 = BN_new();
            q1 = BN_new();
            d = BN_new();
            if (n == NULL || phi == NULL || p1 == NULL || q1 == NULL || d == NULL) {
                LOG_Error("[%s] BN_new failed", __func__);
                break;
            }
            if (BN_mul(n, p, q, bnCtx) != 1 ||
                BN_sub(p1, p, BN_value_one()) != 1 ||
                BN_sub(q1, q, BN_value_one()) != 1 ||
                BN_mul(phi, p1, q1, bnCtx) != 1 ||
                BN_mod_inverse(d, e, phi, bnCtx) == NULL) {
                LOG_Error("[%s] compute n/d failed", __func__);
                break;
            }

            if (RSA_set0_key(rsa, n, e, d) != 1 ||
                RSA_set0_factors(rsa, p, q) != 1 ||
                RSA_set0_crt_params(rsa, dmp1, dmq1, iqmp) != 1) {
                LOG_Error("[%s] RSA_set0 failed", __func__);
                break;
            }
            n = NULL;
            e = NULL;
            d = NULL;
            p = NULL;
            q = NULL;
            dmp1 = NULL;
            dmq1 = NULL;
            iqmp = NULL;

            if (RSA_check_key(rsa) != 1) {
                LOG_Error("[%s] RSA private key invalid", __func__);
                break;
            }
        } else if (keyType == E_KEY_TYPE_PRI_PUB_PAIR) {
            if (inKeyValLen != halfLen * 7 + 4) {
                LOG_Error("[%s] key pair length mismatch: %u != %u", __func__, inKeyValLen, halfLen * 7 + 4);
                break;
            }
            n = BN_bin2bn(inKeyVal + 4, nLen, NULL);
            p = BN_bin2bn(inKeyVal + 4 + nLen, halfLen, NULL);
            q = BN_bin2bn(inKeyVal + 4 + nLen + halfLen, halfLen, NULL);
            dmp1 = BN_bin2bn(inKeyVal + 4 + nLen + halfLen * 2, halfLen, NULL);
            dmq1 = BN_bin2bn(inKeyVal + 4 + nLen + halfLen * 3, halfLen, NULL);
            iqmp = BN_bin2bn(inKeyVal + 4 + nLen + halfLen * 4, halfLen, NULL);
            if (n == NULL || p == NULL || q == NULL || dmp1 == NULL || dmq1 == NULL || iqmp == NULL) {
                LOG_Error("[%s] BN_bin2bn failed", __func__);
                break;
            }

            bnCtx = BN_CTX_new();
            if (bnCtx == NULL) {
                LOG_Error("[%s] BN_CTX_new failed", __func__);
                break;
            }
            phi = BN_new();
            p1 = BN_new();
            q1 = BN_new();
            d = BN_new();
            if (phi == NULL || p1 == NULL || q1 == NULL || d == NULL) {
                LOG_Error("[%s] BN_new failed", __func__);
                break;
            }
            if (BN_sub(p1, p, BN_value_one()) != 1 ||
                BN_sub(q1, q, BN_value_one()) != 1 ||
                BN_mul(phi, p1, q1, bnCtx) != 1 ||
                BN_mod_inverse(d, e, phi, bnCtx) == NULL) {
                LOG_Error("[%s] compute d failed", __func__);
                break;
            }

            if (RSA_set0_key(rsa, n, e, d) != 1 ||
                RSA_set0_factors(rsa, p, q) != 1 ||
                RSA_set0_crt_params(rsa, dmp1, dmq1, iqmp) != 1) {
                LOG_Error("[%s] RSA_set0 failed", __func__);
                break;
            }
            n = NULL;
            e = NULL;
            d = NULL;
            p = NULL;
            q = NULL;
            dmp1 = NULL;
            dmq1 = NULL;
            iqmp = NULL;

            if (RSA_check_key(rsa) != 1) {
                LOG_Error("[%s] RSA key pair invalid", __func__);
                break;
            }
        } else {
            LOG_Error("[%s] unsupported keyType: %u", __func__, keyType);
            break;
        }

        pkey = EVP_PKEY_new();
        if (pkey == NULL || EVP_PKEY_assign_RSA(pkey, rsa) != 1) {
            LOG_Error("[%s] EVP_PKEY_assign_RSA failed", __func__);
            break;
        }
        rsa = NULL;

        pkcs8Info = EVP_PKEY2PKCS8(pkey);
        if (pkcs8Info == NULL) {
            LOG_Error("[%s] EVP_PKEY2PKCS8 failed", __func__);
            break;
        }

        derLen = i2d_PKCS8_PRIV_KEY_INFO(pkcs8Info, NULL);
        if (derLen <= 0) {
            LOG_Error("[%s] i2d_PKCS8_PRIV_KEY_INFO failed", __func__);
            break;
        }

        if ((uint32_t)derLen > *outPkcs8Len) {
            LOG_Error("[%s] Buffer too small: %u < %d", __func__, *outPkcs8Len, derLen);
            *outPkcs8Len = (uint32_t)derLen;
            break;
        }

        pOut = outPkcs8;
        derLen = i2d_PKCS8_PRIV_KEY_INFO(pkcs8Info, &pOut);
        if (derLen <= 0) {
            LOG_Error("[%s] i2d_PKCS8_PRIV_KEY_INFO write failed", __func__);
            break;
        }

        *outPkcs8Len = (uint32_t)derLen;
        LOG_Info("[%s] Converted HSM key to PKCS#8, type=%u, len=%u", __func__, keyType, *outPkcs8Len);
        ret = 0;
    } while (0);

    if (rsa != NULL) {
        RSA_free(rsa);
    }
    if (n != NULL) {
        BN_clear_free(n);
    }
    if (e != NULL) {
        BN_clear_free(e);
    }
    if (d != NULL) {
        BN_clear_free(d);
    }
    if (p != NULL) {
        BN_clear_free(p);
    }
    if (q != NULL) {
        BN_clear_free(q);
    }
    if (dmp1 != NULL) {
        BN_clear_free(dmp1);
    }
    if (dmq1 != NULL) {
        BN_clear_free(dmq1);
    }
    if (iqmp != NULL) {
        BN_clear_free(iqmp);
    }
    if (phi != NULL) {
        BN_free(phi);
    }
    if (p1 != NULL) {
        BN_free(p1);
    }
    if (q1 != NULL) {
        BN_free(q1);
    }
    if (bnCtx != NULL) {
        BN_CTX_free(bnCtx);
    }
    if (pkey != NULL) {
        EVP_PKEY_free(pkey);
    }
    if (pkcs8Info != NULL) {
        PKCS8_PRIV_KEY_INFO_free(pkcs8Info);
    }

    return ret;
}

/*
 * PKCS#8 DER -> HSM 裸 RSA 格式
 */
static int HsmConvertPkcs8ToHsmKey(const uint8_t *pkcs8, uint32_t pkcs8Len,
                                   uint8_t keyType, uint32_t bits,
                                   uint8_t *outKeyVal, uint32_t *outKeyValLen)
{
    int ret = -1;
    PKCS8_PRIV_KEY_INFO *pkcs8Info = NULL;
    const uint8_t *p = pkcs8;
    EVP_PKEY *pkey = NULL;
    RSA *rsa = NULL;
    const BIGNUM *bnN = NULL;
    const BIGNUM *bnE = NULL;
    const BIGNUM *bnD = NULL;
    const BIGNUM *bnP = NULL;
    const BIGNUM *bnQ = NULL;
    const BIGNUM *bnDmp1 = NULL;
    const BIGNUM *bnDmq1 = NULL;
    const BIGNUM *bnIqmp = NULL;
    uint32_t nLen = bits / 8;
    uint32_t halfLen = nLen / 2;
    uint32_t expectedLen = 0;

    do {
        if (pkcs8 == NULL || pkcs8Len == 0 || outKeyVal == NULL || outKeyValLen == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        pkcs8Info = d2i_PKCS8_PRIV_KEY_INFO(NULL, &p, pkcs8Len);
        if (pkcs8Info == NULL) {
            LOG_Error("[%s] d2i_PKCS8_PRIV_KEY_INFO failed", __func__);
            break;
        }

        pkey = EVP_PKCS82PKEY(pkcs8Info);
        if (pkey == NULL) {
            LOG_Error("[%s] EVP_PKCS82PKEY failed", __func__);
            break;
        }

        rsa = EVP_PKEY_get1_RSA(pkey);
        if (rsa == NULL) {
            LOG_Error("[%s] EVP_PKEY_get1_RSA failed", __func__);
            break;
        }

        RSA_get0_key(rsa, &bnN, &bnE, &bnD);
        RSA_get0_factors(rsa, &bnP, &bnQ);
        RSA_get0_crt_params(rsa, &bnDmp1, &bnDmq1, &bnIqmp);

        if (bnE == NULL || BN_num_bytes(bnE) == 0) {
            LOG_Error("[%s] invalid RSA public key components", __func__);
            break;
        }

        if (keyType == E_KEY_TYPE_PUB) {
            if (bnN == NULL) {
                LOG_Error("[%s] missing RSA modulus", __func__);
                break;
            }
            expectedLen = 4 + (uint32_t)BN_num_bytes(bnN);
        } else if (keyType == E_KEY_TYPE_PRI) {
            if (bnP == NULL || bnQ == NULL || bnDmp1 == NULL || bnDmq1 == NULL || bnIqmp == NULL) {
                LOG_Error("[%s] missing RSA private key components", __func__);
                break;
            }
            expectedLen = 4 + halfLen * 5;
        } else if (keyType == E_KEY_TYPE_PRI_PUB_PAIR) {
            if (bnN == NULL || bnP == NULL || bnQ == NULL || bnDmp1 == NULL || bnDmq1 == NULL || bnIqmp == NULL) {
                LOG_Error("[%s] missing RSA key pair components", __func__);
                break;
            }
            expectedLen = 4 + nLen + halfLen * 5;
        } else {
            LOG_Error("[%s] unsupported keyType: %u", __func__, keyType);
            break;
        }

        if (*outKeyValLen < expectedLen) {
            LOG_Error("[%s] Buffer too small: %u < %u", __func__, *outKeyValLen, expectedLen);
            *outKeyValLen = expectedLen;
            break;
        }

        memset(outKeyVal, 0, expectedLen);
        HsmWriteBnFixed(bnE, outKeyVal, 4);

        if (keyType == E_KEY_TYPE_PUB) {
            HsmWriteBnFixed(bnN, outKeyVal + 4, (uint32_t)BN_num_bytes(bnN));
        } else if (keyType == E_KEY_TYPE_PRI) {
            HsmWriteBnFixed(bnP, outKeyVal + 4, halfLen);
            HsmWriteBnFixed(bnQ, outKeyVal + 4 + halfLen, halfLen);
            HsmWriteBnFixed(bnDmp1, outKeyVal + 4 + halfLen * 2, halfLen);
            HsmWriteBnFixed(bnDmq1, outKeyVal + 4 + halfLen * 3, halfLen);
            HsmWriteBnFixed(bnIqmp, outKeyVal + 4 + halfLen * 4, halfLen);
        } else if (keyType == E_KEY_TYPE_PRI_PUB_PAIR) {
            HsmWriteBnFixed(bnN, outKeyVal + 4, nLen);
            HsmWriteBnFixed(bnP, outKeyVal + 4 + nLen, halfLen);
            HsmWriteBnFixed(bnQ, outKeyVal + 4 + nLen + halfLen, halfLen);
            HsmWriteBnFixed(bnDmp1, outKeyVal + 4 + nLen + halfLen * 2, halfLen);
            HsmWriteBnFixed(bnDmq1, outKeyVal + 4 + nLen + halfLen * 3, halfLen);
            HsmWriteBnFixed(bnIqmp, outKeyVal + 4 + nLen + halfLen * 4, halfLen);
        }

        *outKeyValLen = expectedLen;
        LOG_Info("[%s] Converted PKCS#8 to HSM key, type=%u, len=%u", __func__, keyType, *outKeyValLen);
        ret = 0;
    } while (0);

    if (rsa != NULL) {
        RSA_free(rsa);
    }
    if (pkey != NULL) {
        EVP_PKEY_free(pkey);
    }
    if (pkcs8Info != NULL) {
        PKCS8_PRIV_KEY_INFO_free(pkcs8Info);
    }

    return ret;
}

static int hsmApiInit(void* ctx)
{
    Ctx_t *hsmCtx = (Ctx_t *)ctx;
    uint8_t comOutbuf[300] = {0};
    uint32_t outlen = 0;
    int ret = -1;
    int connected = 0;
    se_id_t se_id= {0};
    se_info_t se_info = {0};
    se_error_t se_ret = 0;
    pin_t pin = {0};

    LOG_Warn("[%s] enter", __func__);

    do {
        LOG_Info("[%s] Initializing...", __func__);

        if (hsmCtx == NULL) {
            LOG_Error("[%s] Invalid parameters: hsmCtx is NULL", __func__);
            break;
        }

        port_mcu_init();

        LOG_Info("[%s] select interface: %s, supportIrq: %d, dev: %s, speed: %d, resetPin: %d, irqPin: %d", __func__, (hsmCtx->config.select == E_HSM_COMM_METHOD_I2C) ? "I2C" : "SPI", hsmCtx->config.hedSDKConfig.supportIrq, hsmCtx->config.hedSDKConfig.dev, hsmCtx->config.hedSDKConfig.speed, hsmCtx->config.hedSDKConfig.resetPin, hsmCtx->config.hedSDKConfig.irqPin);
        if (hsmCtx->config.select == E_HSM_COMM_METHOD_SPI)
        {
            LOG_Debug("[%s] mode: %d, bpw: %d", __func__, hsmCtx->config.hedSDKConfig.spi.mode, hsmCtx->config.hedSDKConfig.spi.bpw);
            se_ret = api_register(PERIPHERAL_SPI, SPI_PERIPHERAL_SE0);
            if (se_ret != SE_SUCCESS) {
                LOG_Error("[%s] api_register failed: 0x%08X", __func__, se_ret);
                ret = se_ret;
                break;
            }

            se_ret = api_select(PERIPHERAL_SPI, SPI_PERIPHERAL_SE0);
            if (se_ret != SE_SUCCESS) {
                LOG_Warn("[%s] api_select failed, trying api_reset...", __func__);
                se_ret = api_reset(comOutbuf, &outlen);
                if (se_ret != SE_SUCCESS) {
                    LOG_Error("[%s] api_reset failed: 0x%08X", __func__, se_ret);
                    ret = se_ret;
                    break;
                }
            }
        }
        else
        {
            LOG_Debug("[%s] slaveAddr: 0x%02x, timeout: %dms, retry: %d", __func__, hsmCtx->config.hedSDKConfig.i2c.slaveAddr, hsmCtx->config.hedSDKConfig.i2c.timeout, hsmCtx->config.hedSDKConfig.i2c.retry);
            se_ret = api_register(PERIPHERAL_I2C, I2C_PERIPHERAL_SE0);
            if (se_ret != SE_SUCCESS) {
                LOG_Error("[%s] api_register failed: 0x%08X", __func__, se_ret);
                ret = se_ret;
                break;
            }

            se_ret = api_select(PERIPHERAL_I2C, I2C_PERIPHERAL_SE0);
            if (se_ret != SE_SUCCESS) {
                LOG_Warn("[%s] api_select failed, trying api_reset...", __func__);
                se_ret = api_reset(comOutbuf, &outlen);
                if (se_ret != SE_SUCCESS) {
                    LOG_Error("[%s] api_reset failed: 0x%08X", __func__, se_ret);
                    ret = se_ret;
                    break;
                }
            }
        }

#ifndef CONNECT_NEED_AUTH
        se_ret = api_connect(comOutbuf, &outlen, &hsmCtx->config.hedSDKConfig);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_connect failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }
        connected = 1;
#else
        sym_key_t eKey = {0};
        const uint8_t eKeyBin[16] = {
            0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
            0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F
        };
        eKey.val_len = 0x10;
        eKey.alg = ALG_SM4;
        memcpy(eKey.val, eKeyBin, 0x10);

        se_ret = api_connect_auth(&eKey, comOutbuf, &outlen, &hsmCtx->config.hedSDKConfig);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_connect_auth failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }
        connected = 1;
#endif

        pin.owner = ADMIN_PIN;
        pin.pin_len = HED_ADMIN_PIN_LEN;
        memcpy(pin.pin_value, g_hedAdminPin, pin.pin_len);

        se_ret = api_verify_pin(&pin);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_verify_pin failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }
#if 0
        {
            sym_key_t tranKey = {0};
            se_error_t sk_ret = 0;
            const uint8_t transkeyVal[16] = {
                0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
                0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F
            };
            memcpy(tranKey.val, transkeyVal, 16);
            tranKey.val_len = 16;
            tranKey.id = HED_TRANSKEY_ID;

            sk_ret = api_set_transkey(&tranKey);
            if (sk_ret != SE_SUCCESS)
                LOG_Error("[%s] api_set_transkey failed: 0x%08X", __func__, sk_ret);
        }
#endif

#if 0
        /** USR ID **/
        se_ret = api_get_id (&se_id);
        if (se_ret != SE_SUCCESS){
            LOG_Error("[%s] api_get_id failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }
        LOG_DebugHex(se_id.val, sizeof(se_id.val_len), "USR_ID:");
#endif
        /** CHIP_ID **/
        se_ret = api_get_info (CHIP_ID, &se_info);
        if (se_ret != SE_SUCCESS){
            LOG_Error("[%s] get CHIP_ID failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }
        LOG_DebugHex(se_info.chip_id, sizeof(se_info.chip_id), "CHIP_ID:");

        /** PRODUCT_INFO **/
        se_ret = api_get_info (PRODUCT_INFO, &se_info);
        if (se_ret != SE_SUCCESS){
            LOG_Error("[%s] get PRODUCT_INFO failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }
        LOG_DebugHex(se_info.product_info, sizeof(se_info.product_info), "PRODUCT_INFO:");

        /** LOADER_VERSION **/
        se_ret = api_get_info (LOADER_VERSION, &se_info);
        if (se_ret != SE_SUCCESS){
            LOG_Error("[%s] get LOADER_VERSION failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }
        LOG_DebugHex(se_info.loader_version, sizeof(se_info.loader_version), "LOADER_VERSION:");

        /** LOADER_FEK_INFO **/
        se_ret = api_get_info (LOADER_FEK_INFO, &se_info);
        if (se_ret != SE_SUCCESS){
            LOG_Error("[%s] get LOADER_FEK_INFO failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }
        LOG_DebugHex(se_info.loader_FEK_info, sizeof(se_info.loader_FEK_info), "LOADER_FEK_INFO:");

        /** LOADER_FVK_INFO **/
        se_ret = api_get_info (LOADER_FVK_INFO, &se_info);
        if (se_ret != SE_SUCCESS){
            LOG_Error("[%s] get LOADER_FVK_INFO failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }
        LOG_DebugHex(se_info.loader_FVK_info, sizeof(se_info.loader_FVK_info), "LOADER_FVK_INFO:");

        ret = 0;
    } while(0);

    if (ret != 0 && connected) {
        se_ret = api_disconnect();
        if (se_ret != SE_SUCCESS) {
            LOG_Warn("[%s] api_disconnect failed: 0x%08X", __func__, se_ret);
        }
    }

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiRelease(void* ctx)
{
    int ret = -1;
    se_error_t se_ret = 0;
    (void)ctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        LOG_Info("[%s] Closing connection...", __func__);

        se_ret = api_disconnect();
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_disconnect failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiRandom(void* ctx, uint32_t expectedLen, uint8_t *random)
{
    int ret = -1;
    uint32_t remaining = 0;
    uint8_t *ptr = NULL;
    uint32_t chunk = 0;
    se_error_t se_ret = 0;
    (void)ctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (random == NULL) {
            LOG_Error("[%s] Invalid parameters: random is NULL", __func__);
            break;
        }

        LOG_Debug("[%s] Request: expectedLen=%u", __func__, expectedLen);

        remaining = expectedLen;
        ptr = random;

        while (remaining > 0) {
            chunk = (remaining > 16) ? 16 : remaining;
            se_ret = api_get_random(chunk, ptr);
            if (se_ret != SE_SUCCESS) {
                LOG_Error("[%s] api_get_random failed at offset %u, remaining %u, error 0x%08X",
                          __func__, (uint32_t)(ptr - random), remaining, se_ret);
                ret = se_ret;
                break;
            }
            ptr += chunk;
            remaining -= chunk;
        }
        if (remaining != 0) {
            break;
        }

        LOG_Info("[%s] Generated %u bytes", __func__, expectedLen);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiRsaDecrypt(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, RsaSignPaddingAlg_e padding, uint8_t *encryptedData, uint32_t encryptedDataLen, uint8_t *out, uint32_t* outBufSize)
{
    int ret = -1;
    pri_key_t priKey = {0};
    alg_asym_param_t asymParam = {0};
    se_error_t se_ret = 0;
    (void)ctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (encryptedData == NULL || out == NULL || outBufSize == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        LOG_Debug("[%s] Request: alg=%d, keyId=0x%04X, padding=%d, encryptedDataLen=%u",
                  __func__, alg, keyId, padding, encryptedDataLen);

        switch (alg) {
            case E_ALG_RSA1024:
                priKey.alg = ALG_RSA1024_CRT;
                break;
            case E_ALG_RSA2048:
                priKey.alg = ALG_RSA2048_CRT;
                break;
            default:
                LOG_Error("[%s] Unsupported algorithm: %d", __func__, alg);
                break;
        }
        if (priKey.alg == 0) {
            break;
        }

        switch (padding) {
            case E_RSA_SIGN_PADDING_NONE:
            case E_RSA_SIGN_PADDING_OAEP:
            case E_RSA_SIGN_PADDING_PSS:
                asymParam.padding_type = PADDING_NOPADDING;
                break;
            case E_RSA_SIGN_PADDING_PKCS1:
                asymParam.padding_type = PADDING_PKCS1;
                break;
            default:
                LOG_Error("[%s] Unsupported padding_type: %d", __func__, padding);
                break;
        }
        // if (asymParam.padding_type == 0 && padding != E_RSA_SIGN_PADDING_NONE) {
        //     break;
        // }

        priKey.id = keyId;

        LOG_Debug("[%s] Calling api_asym_decrypt with priKey: alg=0x%02X, id=0x%04X, padding_type=0x%02X",
                  __func__, priKey.alg, priKey.id, asymParam.padding_type);

        se_ret = api_asym_decrypt(&priKey, &asymParam, encryptedData, encryptedDataLen, out, outBufSize);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_asym_decrypt failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }

        LOG_Info("[%s] Decrypted success: %u bytes", __func__, *outBufSize);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiRsaEncrypt(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, RsaSignPaddingAlg_e padding, uint8_t *plain, uint32_t plainLen, uint8_t *out, uint32_t* outBufSize)
{
    int ret = -1;
    pub_key_t pubKey = {0};
    alg_asym_param_t asymParam = {0};
    se_error_t se_ret = 0;
    (void)ctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (plain == NULL || out == NULL || outBufSize == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        LOG_Debug("[%s] Request: alg=%d, keyId=0x%04X, padding=%d, plainLen=%u",
                  __func__, alg, keyId, padding, plainLen);

        switch (alg) {
            case E_ALG_RSA1024:
                pubKey.alg = ALG_RSA1024_CRT;
                break;
            case E_ALG_RSA2048:
                pubKey.alg = ALG_RSA2048_CRT;
                break;
            default:
                LOG_Error("[%s] Unsupported algorithm: %d", __func__, alg);
                break;
        }
        if (pubKey.alg == 0) {
            break;
        }

        switch (padding) {
            case E_RSA_SIGN_PADDING_NONE:
            case E_RSA_SIGN_PADDING_OAEP:
            case E_RSA_SIGN_PADDING_PSS:
                asymParam.padding_type = PADDING_NOPADDING;
                break;
            case E_RSA_SIGN_PADDING_PKCS1:
                asymParam.padding_type = PADDING_PKCS1;
                break;
            default:
                LOG_Error("[%s] Unsupported padding_type: %d", __func__, padding);
                break;
        }
        // if (asymParam.padding_type == 0 && padding != E_RSA_SIGN_PADDING_NONE) {
        //     break;
        // }

        pubKey.id = keyId;

        LOG_Debug("[%s] Calling api_asym_encrypt with pubKey: alg=0x%02X, id=0x%04X, padding_type=0x%02X",
                  __func__, pubKey.alg, pubKey.id, asymParam.padding_type);

        se_ret = api_asym_encrypt(&pubKey, &asymParam, plain, plainLen, out, outBufSize);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_asym_encrypt failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }

        LOG_Info("[%s] Encrypted success: %u bytes", __func__, *outBufSize);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiRsaKeyPairGenerate(void* ctx, RsaKeyAlg_e alg, uint16_t keyId, uint8_t* pubKeyDer, uint32_t *pubKeyDerBufSize)
{
    int ret = -1;
    pri_key_t priKey = {0};
    pub_key_t pubKey = {0};
    se_error_t se_ret = 0;
    (void)ctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (pubKeyDer == NULL || pubKeyDerBufSize == NULL) {
            LOG_Error("[%s] Invalid output parameters", __func__);
            break;
        }

        LOG_Info("[%s] Request: alg=%d, keyId=0x%04X", __func__, alg, keyId);

        switch (alg) {
            case E_ALG_RSA1024:
                pubKey.alg = ALG_RSA1024_STANDAND;
                break;
            case E_ALG_RSA2048:
                pubKey.alg = ALG_RSA2048_STANDAND;
                break;
            default:
                LOG_Error("[%s] Unsupported algorithm: %d", __func__, alg);
                break;
        }
        if (pubKey.alg == 0) {
            break;
        }

        pubKey.id = keyId;
        priKey.id = keyId;

        LOG_Debug("[%s] Key params - pubKey: alg=0x%02X, id=0x%04X, val=%p, val_len=%u",
                  __func__, pubKey.alg, pubKey.id, pubKey.val, pubKey.val_len);
        LOG_Debug("[%s] Key params - priKey:  id=0x%04X, val=%p, val_len=%u",
                  __func__, priKey.id, priKey.val, priKey.val_len);

        se_ret = api_generate_keypair(&pubKey, &priKey);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_generate_keypair failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }

        LOG_Debug("[%s] RSA Key Pair generated successfully", __func__);
        LOG_Debug("[%s]   pubKey: alg=0x%02X, id=0x%02X, val_len=%u",
                  __func__, pubKey.alg, pubKey.id, pubKey.val_len);
        LOG_Debug("[%s]   priKey: id=0x%02X, val_len=%u",
                  __func__, priKey.id, priKey.val_len);

        ret = HsmConvertEnToX509Der(pubKey.val, pubKey.val_len, pubKeyDer, pubKeyDerBufSize);
        if (ret != 0) {
            LOG_Error("[%s] HsmConvertEnToX509Der failed: %d", __func__, ret);
            break;
        }

        LOG_Info("[%s] Public key exported: len=%u", __func__, *pubKeyDerBufSize);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiRsaSign(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, DigistAlg_e digist, uint8_t *plain, uint32_t plainLen, uint8_t *sign, uint32_t* signBufSize)
{
    int ret = -1;
    pri_key_t priKey = {0};
    alg_asym_param_t asymParam = {0};
    uint32_t signLen = 0;
    se_error_t se_ret = 0;
    (void)ctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (plain == NULL || sign == NULL || signBufSize == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        LOG_Debug("[%s] Request: alg=%d, keyId=0x%04X, digist=%d, plainLen=%u",
                  __func__, alg, keyId, digist, plainLen);

        switch (alg) {
            case E_ALG_RSA1024:
                priKey.alg = ALG_RSA1024_CRT;
                break;
            case E_ALG_RSA2048:
                priKey.alg = ALG_RSA2048_CRT;
                break;
            default:
                LOG_Error("[%s] Unsupported algorithm: %d", __func__, alg);
                break;
        }
        if (priKey.alg == 0) {
            break;
        }

        switch (digist) {
            case E_DIGIST_SHA1:
                asymParam.hash_type = ALG_SHA1;
                break;
            case E_DIGIST_SHA224:
                asymParam.hash_type = ALG_SHA224;
                break;
            case E_DIGIST_SHA256:
                asymParam.hash_type = ALG_SHA256;
                break;
            case E_DIGIST_SHA384:
                asymParam.hash_type = ALG_SHA384;
                break;
            case E_DIGIST_SHA512:
                asymParam.hash_type = ALG_SHA512;
                break;
            case E_DIGIST_SM3:
                asymParam.hash_type = ALG_SM3;
                break;
            default:
                LOG_Error("[%s] Unsupported hash algorithm: %d", __func__, digist);
                break;
        }
        if (asymParam.hash_type == 0 && digist != E_DIGIST_NONE) {
            break;
        }

        priKey.id = keyId;

        LOG_Debug("[%s] Calling api_asym_sign with priKey: alg=0x%02X, id=0x%04X", __func__, priKey.alg, priKey.id);

        se_ret = api_asym_sign(&priKey, &asymParam, plain, plainLen, sign, &signLen);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_asym_sign failed: 0x%08X", __func__, se_ret);
            *signBufSize = 0;
            ret = se_ret;
            break;
        }

        *signBufSize = signLen;
        LOG_Info("[%s] Signature success: len=%u", __func__, signLen);
        LOG_Debug("[%s] Signature[0-15]: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                  __func__,
                  sign[0], sign[1], sign[2], sign[3], sign[4], sign[5], sign[6], sign[7],
                  sign[8], sign[9], sign[10], sign[11], sign[12], sign[13], sign[14], sign[15]);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiRsaVerify(void* ctx, RsaKeyAlg_e alg, const uint16_t keyId, DigistAlg_e digist, uint8_t *plain, uint32_t plainLen, uint8_t *sign, uint32_t signBufSize)
{
    int ret = -1;
    pub_key_t pubKey = {0};
    alg_asym_param_t asymParam = {0};
    se_error_t se_ret = 0;
    (void)ctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (plain == NULL || sign == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        LOG_Debug("[%s] Request: alg=%d, keyId=0x%04X, digist=%d, plainLen=%u, signLen=%u",
                  __func__, alg, keyId, digist, plainLen, signBufSize);

        switch (alg) {
            case E_ALG_RSA1024:
                pubKey.alg = ALG_RSA1024_CRT;
                break;
            case E_ALG_RSA2048:
                pubKey.alg = ALG_RSA2048_CRT;
                break;
            default:
                LOG_Error("[%s] Unsupported algorithm: %d", __func__, alg);
                break;
        }
        if (pubKey.alg == 0) {
            break;
        }

        switch (digist) {
            case E_DIGIST_SHA1:
                asymParam.hash_type = ALG_SHA1;
                break;
            case E_DIGIST_SHA224:
                asymParam.hash_type = ALG_SHA224;
                break;
            case E_DIGIST_SHA256:
                asymParam.hash_type = ALG_SHA256;
                break;
            case E_DIGIST_SHA384:
                asymParam.hash_type = ALG_SHA384;
                break;
            case E_DIGIST_SHA512:
                asymParam.hash_type = ALG_SHA512;
                break;
            case E_DIGIST_SM3:
                asymParam.hash_type = ALG_SM3;
                break;
            default:
                LOG_Error("[%s] Unsupported hash algorithm: %d", __func__, digist);
                break;
        }
        if (asymParam.hash_type == 0 && digist != E_DIGIST_NONE) {
            break;
        }

        pubKey.id = keyId;

        LOG_Debug("[%s] Calling api_asym_verify with pubKey: alg=0x%02X, id=0x%04X", __func__, pubKey.alg, pubKey.id);

        se_ret = api_asym_verify(&pubKey, &asymParam, plain, plainLen, sign, &signBufSize);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_asym_verify failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }

        LOG_Info("[%s] Signature verification success", __func__);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiQueryKey(void* ctx, QueryKeyResult_t *keyResult)
{
    int ret = -1;
    se_error_t se_ret = 0;
    uint8_t rawBuf[QUERY_HSM_MAX_LEN] = {0};
    uint32_t rawLen = sizeof(rawBuf);
    uint16_t keyFlag = 0;
    uint16_t i = 0;
    uint8_t validCount = 0;
    (void)ctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (keyResult == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        se_ret = api_get_key_info(true, rawBuf, &rawLen);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_get_key_info failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }

        memset(keyResult, 0, sizeof(QueryKeyResult_t));

        for (i = 0; i < KEY_SLOT_COUNT && validCount < KEY_SLOT_COUNT; i++) {
            keyFlag = (rawBuf[i * KEY_INFO_SIZE] << 8) | rawBuf[i * KEY_INFO_SIZE + 1];
            if (keyFlag != KEY_NOT_EXIST) {
                if (HsmParseKeyInfo(rawBuf[i * KEY_INFO_SIZE], rawBuf[i * KEY_INFO_SIZE + 1],
                                    &keyResult->keyList[validCount]) != 0) {
                    LOG_Warn("[%s] skip unknown algId=0x%02X at keyId=%u", __func__, rawBuf[i * KEY_INFO_SIZE], (uint8_t)i);
                    continue;
                }
                keyResult->keyList[validCount].keyId = (uint8_t)i;
                validCount++;
            }
        }

        keyResult->validKeyCount = validCount;
        LOG_Info("[%s] success, validKeyCount=%u", __func__, validCount);
        LOG_DebugHex(rawBuf, rawLen, "Key info (hex):");
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiQueryKeyExists(void* ctx, const uint16_t keyId)
{
    QueryKeyResult_t keyResult = {0};
    int ret = 0;
    uint32_t i = 0;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (keyId >= KEY_SLOT_COUNT) {
            ret = 0;
            break;
        }

        ret = hsmApiQueryKey(ctx, &keyResult);
        if (ret != 0) {
            ret = -2;
            break;
        }

        for (i = 0; i < keyResult.validKeyCount; i++) {
            if (keyResult.keyList[i].keyId == (uint8_t)keyId) {
                ret = 1;
                break;
            }
        }
        if (i == keyResult.validKeyCount) {
            ret = 0;
        }
    } while (0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiDeleteKey(void* ctx, const uint16_t keyId)
{
    int ret = -1;
    se_error_t se_ret = 0;
    (void)ctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        LOG_Debug("[%s] Request: keyId=0x%04X", __func__, keyId);

        se_ret = api_del_key(keyId);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_del_key failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }

        LOG_Info("[%s] Key deleted successfully", __func__);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiExportKey(void* ctx, RsaKeyAlg_e exKeyAlg, const uint16_t exKeyId, uint8_t exKeyType, uint8_t *out, uint32_t *outBufSize)
{
    int ret = -1;
    se_error_t se_ret = 0;
    (void)ctx;
    unikey_t ukey = {0};
    unikey_t exkey = {0};
    uint32_t bits = 0;

    LOG_Warn("[%s] enter", __func__);

    do {
        LOG_Debug("[%s] Request: exKeyAlg=%d, exKeyId=0x%04X, exKeyType=%d", __func__, exKeyAlg, exKeyId, exKeyType);

        switch (exKeyAlg) {
            case E_ALG_RSA1024:
                exkey.alg = ALG_RSA1024_CRT;
                bits = 1024;
                break;
            case E_ALG_RSA2048:
                exkey.alg = ALG_RSA2048_CRT;
                bits = 2048;
                break;
            default:
                LOG_Error("[%s] Unsupported algorithm: %d", __func__, exKeyAlg);
                break;
        }
        if (exkey.alg == 0) {
            break;
        }

        switch (exKeyType) {
            case E_KEY_TYPE_PRI:
                exkey.type = PRI;
                break;
            case E_KEY_TYPE_PUB:
                exkey.type = PUB;
                break;
            case E_KEY_TYPE_PRI_PUB_PAIR:
                exkey.type = PRI_PUB_PAIR;
                break;
            default:
                LOG_Error("[%s] Unsupported keyType: %d", __func__, exKeyType);
                break;
        }
        if (exkey.type == 0 && exKeyType != E_KEY_TYPE_PRI) {
            break;
        }

        exkey.id = exKeyId;

        se_ret = api_export_key(&ukey, &exkey, false, false);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_export_key failed: 0x%08X", __func__, se_ret);
            *outBufSize = 0;
            ret = se_ret;
            break;
        }

        if (exKeyType == E_KEY_TYPE_PUB) {
            ret = HsmConvertEnToX509Der(exkey.val, exkey.val_len, out, outBufSize);
        } else {
            ret = HsmConvertHsmKeyToPkcs8(exkey.val, exkey.val_len, exKeyType, bits, out, outBufSize);
        }
        if (ret != 0) {
            LOG_Error("[%s] key conversion failed: %d", __func__, ret);
            *outBufSize = 0;
            break;
        }

        LOG_Info("[%s] Key exported successfully: len=%u", __func__, *outBufSize);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiImportKey(void* ctx, RsaKeyAlg_e inKeyAlg, const uint16_t inKeyId, uint8_t inKeyType, const uint8_t *inKeyVal, uint32_t inKeyValLen)
{
    int ret = -1;
    se_error_t se_ret = 0;
    (void)ctx;
    unikey_t ukey = {0};
    unikey_t inkey = {0};
    uint32_t bits = 0;
    uint8_t convBuf[IMPORT_KEY_MAX_LEN] = {0};
    uint32_t convLen = sizeof(convBuf);

    LOG_Warn("[%s] enter", __func__);

    do {
        switch (inKeyAlg) {
            case E_ALG_RSA1024:
                inkey.alg = ALG_RSA1024_CRT;
                bits = 1024;
                break;
            case E_ALG_RSA2048:
                inkey.alg = ALG_RSA2048_CRT;
                bits = 2048;
                break;
            default:
                LOG_Error("[%s] Unsupported algorithm: %d", __func__, inKeyAlg);
                break;
        }
        if (inkey.alg == 0) {
            break;
        }

        switch (inKeyType) {
            case E_KEY_TYPE_PRI:
                inkey.type = PRI;
                break;
            case E_KEY_TYPE_PUB:
                inkey.type = PUB;
                break;
            case E_KEY_TYPE_PRI_PUB_PAIR:
                inkey.type = PRI_PUB_PAIR;
                break;
            default:
                LOG_Error("[%s] Unsupported keyType: %d", __func__, inKeyType);
                break;
        }
        if (inkey.type == 0 && inKeyType != E_KEY_TYPE_PRI) {
            break;
        }

        inkey.id = inKeyId;

        if (inKeyType == E_KEY_TYPE_PUB) {
            ret = HsmConvertX509DerToEn(inKeyVal, inKeyValLen, convBuf, &convLen);
        } else {
            ret = HsmConvertPkcs8ToHsmKey(inKeyVal, inKeyValLen, inKeyType, bits, convBuf, &convLen);
        }
        if (ret != 0) {
            LOG_Error("[%s] key conversion failed: %d", __func__, ret);
            break;
        }

        if (convLen > sizeof(inkey.val)) {
            LOG_Error("[%s] converted key too long: %u", __func__, convLen);
            break;
        }

        memcpy(inkey.val, convBuf, convLen);
        inkey.val_len = convLen;

        se_ret = api_import_key(&ukey, &inkey, false, false);
        if (se_ret != SE_SUCCESS) {
            LOG_Error("[%s] api_import_key failed: 0x%08X", __func__, se_ret);
            ret = se_ret;
            break;
        }

        LOG_Info("[%s] Key imported successfully", __func__);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiQueryCrt(void* ctx, QueryCrtResult_t *crtResult)
{
    int ret = -1;
    uint8_t outBuf[QUERY_HSM_MAX_LEN] = {0};
    uint32_t outLen = sizeof(outBuf);
    uint32_t count = 0;
    (void)ctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        if (crtResult == NULL) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        ret = hsm_crt_list_query(outBuf, &outLen);
        if (ret != 0) {
            LOG_Error("[%s] hsm_crt_list_query failed: %d", __func__, ret);
            break;
        }

        count = outLen / sizeof(uint16_t);
        if (count > CRT_SLOT_COUNT) {
            count = CRT_SLOT_COUNT;
        }

        memset(crtResult, 0, sizeof(QueryCrtResult_t));
        crtResult->validCrtCount = (uint8_t)count;
        memcpy(crtResult->crtList, outBuf, count * sizeof(uint16_t));

        LOG_Info("[%s] Certificate list queried successfully: validCrtCount=%u", __func__, crtResult->validCrtCount);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiQueryCrtExists(void* ctx, const uint16_t crtId)
{
    QueryCrtResult_t crtResult = {0};
    int ret = 0;
    uint32_t i = 0;

    LOG_Warn("[%s] enter", __func__);

    do {
        ret = hsmApiQueryCrt(ctx, &crtResult);
        if (ret != 0) {
            ret = -2;
            break;
        }

        for (i = 0; i < crtResult.validCrtCount; i++) {
            if (crtResult.crtList[i] == crtId) {
                ret = 1;
                break;
            }
        }
        if (i == crtResult.validCrtCount) {
            ret = 0;
        }
    } while (0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiDeleteCrt(void* ctx, const uint16_t crtId)
{
    int ret = -1;
    (void)ctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        LOG_Debug("[%s] Request: crtId=0x%04X", __func__, crtId);

        ret = hsm_crt_delete(crtId);
        if (ret != 0) {
            LOG_Error("[%s] hsm_crt_delete failed: %d", __func__, ret);
            break;
        }

        LOG_Info("[%s] Certificate deleted successfully: crtId=0x%04X", __func__, crtId);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiExportCrt(void* ctx, const uint16_t crtId, uint8_t *out, uint32_t *outBufSize)
{
    int ret = -1;
    (void)ctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        LOG_Debug("[%s] Request: crtId=0x%04X, bufSize=%u", __func__, crtId, *outBufSize);

        if (out == NULL || outBufSize == NULL || *outBufSize == 0) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        ret = hsm_crt_export(crtId, *outBufSize, out, outBufSize);
        if (ret != 0) {
            LOG_Error("[%s] hsm_crt_export failed: %d", __func__, ret);
            break;
        }

        LOG_Info("[%s] Certificate exported successfully: crtId=0x%04X, len=%u", __func__, crtId, *outBufSize);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

static int hsmApiImportCrt(void* ctx, const uint16_t crtId, const uint8_t *inCrtVal, uint32_t inCrtValLen)
{
    int ret = -1;
    (void)ctx;

    LOG_Warn("[%s] enter", __func__);

    do {
        LOG_Debug("[%s] Request: crtId=0x%04X, len=%u", __func__, crtId, inCrtValLen);

        if (inCrtVal == NULL || inCrtValLen == 0) {
            LOG_Error("[%s] Invalid parameters", __func__);
            break;
        }

        ret = hsm_crt_import(crtId, inCrtVal, inCrtValLen);
        if (ret != 0) {
            LOG_Error("[%s] hsm_crt_import failed: %d", __func__, ret);
            break;
        }

        LOG_Info("[%s] Certificate imported successfully: crtId=0x%04X", __func__, crtId);
        ret = 0;
    } while(0);

    LOG_Warn("[%s] exit(%d)", __func__, ret);
    return ret;
}

const HsmApi_t g_hsmCIU98BV2_Api = {
    .init = hsmApiInit,
    .release = hsmApiRelease,
    .random = hsmApiRandom,
    .rsaDecrypt = hsmApiRsaDecrypt,
    .rsaEncrypt = hsmApiRsaEncrypt,
    .rsaKeyPairGenerate = hsmApiRsaKeyPairGenerate,
    .rsaSign = hsmApiRsaSign,
    .rsaVerify = hsmApiRsaVerify,

    .queryKey = hsmApiQueryKey,
    .queryKeyExists = hsmApiQueryKeyExists,
    .deleteKey = hsmApiDeleteKey,
    .exportKey = hsmApiExportKey,
    .importKey = hsmApiImportKey,

    .queryCrt  = hsmApiQueryCrt,
    .queryCrtExists = hsmApiQueryCrtExists,
    .deleteCrt = hsmApiDeleteCrt,
    .exportCrt = hsmApiExportCrt,
    .importCrt = hsmApiImportCrt,
};
