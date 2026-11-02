#ifndef _HSM_OPENSSL_PROVIDER_H_
#define _HSM_OPENSSL_PROVIDER_H_

#include <stdint.h>

#include <openssl/core.h>
#include <openssl/core_dispatch.h>
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <openssl/param_build.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/provider.h>
#include <openssl/params.h>
#include <openssl/rsa.h>
#include <openssl/param_build.h>

#include "sr_log.h"
#include "rpc_hsm_api.h"

#define SIRUN_SSL_PROVIDER_PARAM_KEYID      "key-id"
#define SIRUN_SSL_PROVIDER_PARAM_HSMGENFLAG "hsm-genkey"

#define SIRUN_SSL_PROVIDER_NAME        "sirun_ssl_hsm_provider"
#define SIRUN_SSL_PROVIDER_DEFINATION  "provider="SIRUN_SSL_PROVIDER_NAME


#define BitsToKeyAlg(bits) \
    ((bits) == 1024 ? E_ALG_RSA1024 : \
     (bits) == 2048 ? E_ALG_RSA2048 : \
     E_ALG_NONE)

#define PaddingToEngine(padding) \
    ((padding) == RSA_PKCS1_PADDING      ? E_RSA_SIGN_PADDING_PKCS1 : \
     (padding) == RSA_PKCS1_OAEP_PADDING ? E_RSA_SIGN_PADDING_OAEP : \
     (padding) == RSA_PKCS1_PSS_PADDING  ? E_RSA_SIGN_PADDING_PSS : \
     (padding) == RSA_NO_PADDING         ? E_RSA_SIGN_PADDING_NONE : \
     E_RSA_SIGN_PADDING_PKCS1)

#define DigestToEngine(md)                                         \
    ((md) == NULL ? E_DIGIST_NONE :                                \
     EVP_MD_get_type(md) == NID_sha256   ? E_DIGIST_SHA256 :       \
     EVP_MD_get_type(md) == NID_sha384   ? E_DIGIST_SHA384 :       \
     EVP_MD_get_type(md) == NID_sha512   ? E_DIGIST_SHA512 :       \
     EVP_MD_get_type(md) == NID_sha1     ? E_DIGIST_SHA1   :       \
     EVP_MD_get_type(md) == NID_sha224   ? E_DIGIST_SHA224 :       \
     EVP_MD_get_type(md) == NID_sm3      ? E_DIGIST_SM3    :       \
     E_DIGIST_NONE)

typedef struct {
    uint32_t keyId;
    size_t keyBits;
    int useHsmGenKey;
    BIGNUM *n;   /* 保存生成的模数 */
    BIGNUM *e;   /* 保存生成的公钥指数 */
    BIGNUM *d;   /* 保存生成的公钥指数 */
} HsmProviderKey_t;

extern const OSSL_DISPATCH g_hsmOpensslProviderKeyMgmtDispatch[];
extern const OSSL_DISPATCH g_hsmOpensslProviderSignatureDispatch[];
extern const OSSL_DISPATCH g_hsmOpensslProviderAsymCipherDispatch[];
extern const OSSL_DISPATCH g_hsmOpensslProviderRandDispatch[];

#endif
