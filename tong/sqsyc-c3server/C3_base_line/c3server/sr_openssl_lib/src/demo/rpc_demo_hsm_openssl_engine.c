#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

// 必须在所有 OpenSSL 头文件之前定义，以抑制 3.0 弃用警告
#define OPENSSL_API_COMPAT 10101

#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/bn.h>
#include <openssl/sha.h>

#include "sr_log.h"
#include "rpc_hsm_api.h"
#include "hsm_openssl_engine.h"

#define DEMO_PRINT
#ifdef DEMO_PRINT
#ifdef LOG_Warn
#undef LOG_Warn
#endif
#define LOG_Warn(format, ...) printf(format "\n", ##__VA_ARGS__)
#endif

/* ++++++++++++++++++++++++++++++ log ++++++++++++++++++++++++++++++ */
static void DumpHex(uint8_t* hexData, int hexDataLength)
{
    static char hexBuf[256] = {0};
    int hexBufPos= 0;

    for(int i = 0; i < hexDataLength; i++){
        hexBufPos +=snprintf(hexBuf+hexBufPos, sizeof(hexBuf)-hexBufPos,"%02x ", hexData[i]);
        if ((i+1)%8 == 0){
            hexBuf[hexBufPos++] = ' ';
        }
        if ((i+1)%16 == 0){
            hexBuf[hexBufPos++] = ' ';
        }
        if ((i+1)%32 == 0){
            LOG_Warn("%s", hexBuf);     
            hexBufPos = 0; 
        }
    }
}
                                                           
static void DumpRasBignum(const BIGNUM *bn, const char *label)
{
    if (!bn) {
        LOG_Warn("%s: (null)\n", label);
        return;
    }else{
        char *hex = BN_bn2hex(bn);
        LOG_Warn("%s: %s", label, hex);
        OPENSSL_free(hex);
    }
}

static void DumpRasPublicKey(EVP_PKEY *pkey)
{
    BIGNUM *n = NULL, *e = NULL;

    if (!EVP_PKEY_get_bn_param(pkey, "n", &n) || !EVP_PKEY_get_bn_param(pkey, "e", &e)) {
        LOG_Warn("Failed to extract RSA components.");
    }else{
        DumpRasBignum(n, "modulus");
        DumpRasBignum(e, "exponent");
    }

    if (n != NULL){
        BN_free(n);
    }
    if (e != NULL){
        BN_free(e);
    }
}

/* ===================== 本地辅助函数（替代 openssl_util） ===================== */

static int DemoGenerateRsaPrivateKey(EVP_PKEY **pkey, int bits)
{
    EVP_PKEY_CTX *ctx = NULL;
    int ret = -1;

    if (pkey == NULL || bits < 1024) {
        return -1;
    }
    if (*pkey != NULL) {
        EVP_PKEY_free(*pkey);
        *pkey = NULL;
    }

    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (ctx == NULL) {
        return -1;
    }

    do {
        if (EVP_PKEY_keygen_init(ctx) <= 0) break;
        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) break;
        if (EVP_PKEY_keygen(ctx, pkey) <= 0) break;
        ret = 0;
    } while (0);

    EVP_PKEY_CTX_free(ctx);
    return ret;
}

static int DemoRsaEncrypt(EVP_PKEY *pkey, int padding, const uint8_t *plaintext, int plaintextLen,
                          uint8_t *ciphertext, int *ciphertextLen)
{
    EVP_PKEY_CTX *ctx = NULL;
    int ret = -1;

    if (pkey == NULL || plaintext == NULL || ciphertext == NULL || ciphertextLen == NULL || *ciphertextLen == 0) {
        return -1;
    }

    ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (ctx == NULL) {
        return -1;
    }

    do {
        if (EVP_PKEY_encrypt_init(ctx) <= 0) break;
        if (EVP_PKEY_CTX_set_rsa_padding(ctx, padding) <= 0) break;
        if (padding == RSA_PKCS1_OAEP_PADDING) {
            if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0) break;
            if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) <= 0) break;
        }
        if (EVP_PKEY_encrypt(ctx, ciphertext, (size_t *)ciphertextLen,
                             plaintext, (size_t)plaintextLen) <= 0) break;
        ret = 0;
    } while (0);

    EVP_PKEY_CTX_free(ctx);
    return ret;
}

static int DemoRsaDecrypt(EVP_PKEY *pkey, int padding, const uint8_t *ciphertext, int ciphertextLen,
                          uint8_t *plaintext, int *plaintextLen)
{
    EVP_PKEY_CTX *ctx = NULL;
    int ret = -1;

    if (pkey == NULL || ciphertext == NULL || plaintext == NULL || plaintextLen == NULL || *plaintextLen == 0) {
        return -1;
    }

    ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (ctx == NULL) {
        return -1;
    }

    do {
        if (EVP_PKEY_decrypt_init(ctx) <= 0) break;
        if (EVP_PKEY_CTX_set_rsa_padding(ctx, padding) <= 0) break;
        if (padding == RSA_PKCS1_OAEP_PADDING) {
            if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0) break;
            if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) <= 0) break;
        }
        if (EVP_PKEY_decrypt(ctx, plaintext, (size_t *)plaintextLen,
                             ciphertext, (size_t)ciphertextLen) <= 0) break;
        ret = 0;
    } while (0);

    EVP_PKEY_CTX_free(ctx);
    return ret;
}

static int DemoRsaSign(EVP_PKEY *pkey, const uint8_t *plain, uint32_t plainLen,
                       uint8_t *signBuf, uint32_t *signBufLen)
{
    uint8_t hash[SHA256_DIGEST_LENGTH] = {0};
    const RSA *rsa = EVP_PKEY_get0_RSA(pkey);

    if (rsa == NULL) {
        return -1;
    }

    SHA256(plain, plainLen, hash);
    return RSA_sign(NID_sha256, hash, SHA256_DIGEST_LENGTH, signBuf, signBufLen, (RSA *)rsa) == 1 ? 0 : -1;
}

static int DemoRsaVerify(EVP_PKEY *pkey, const uint8_t *plain, uint32_t plainLen,
                         const uint8_t *signBuf, uint32_t signBufLen)
{
    uint8_t hash[SHA256_DIGEST_LENGTH] = {0};
    const RSA *rsa = EVP_PKEY_get0_RSA(pkey);

    if (rsa == NULL) {
        return -1;
    }

    SHA256(plain, plainLen, hash);
    return RSA_verify(NID_sha256, hash, SHA256_DIGEST_LENGTH, signBuf, signBufLen, (RSA *)rsa) == 1 ? 0 : -1;
}

static int DemoBuildCsrSubject(X509_NAME **name,
    const char *country, const char *state, const char *city,
    const char *org, const char *unit, const char *vin, const char *tboxSn)
{
    char cn[64] = {0};

    if (name == NULL) {
        return -1;
    }

    *name = X509_NAME_new();
    if (*name == NULL) {
        return -1;
    }

    snprintf(cn, sizeof(cn), "%s_%s", vin, tboxSn);

    do {
        if (X509_NAME_add_entry_by_txt(*name, "C",  MBSTRING_ASC, (const uint8_t *)country, -1, -1, 0) != 1) break;
        if (X509_NAME_add_entry_by_txt(*name, "ST", MBSTRING_ASC, (const uint8_t *)state,   -1, -1, 0) != 1) break;
        if (X509_NAME_add_entry_by_txt(*name, "L",  MBSTRING_ASC, (const uint8_t *)city,    -1, -1, 0) != 1) break;
        if (X509_NAME_add_entry_by_txt(*name, "O",  MBSTRING_ASC, (const uint8_t *)org,     -1, -1, 0) != 1) break;
        if (X509_NAME_add_entry_by_txt(*name, "OU", MBSTRING_ASC, (const uint8_t *)unit,    -1, -1, 0) != 1) break;
        if (X509_NAME_add_entry_by_txt(*name, "CN", MBSTRING_ASC, (const uint8_t *)cn,      -1, -1, 0) != 1) break;
        return 0;
    } while (0);

    X509_NAME_free(*name);
    *name = NULL;
    return -1;
}

static int DemoGenerateCsr(X509_REQ **csr, EVP_PKEY *pkey,
    const char *country, const char *state, const char *city,
    const char *org, const char *unit, const char *vin, const char *tboxSn)
{
    int ret = -1;
    X509_NAME *subject = NULL;
    X509_REQ *csrPos = NULL;

    if (csr == NULL || pkey == NULL) {
        return -1;
    }
    *csr = NULL;

    do {
        if (DemoBuildCsrSubject(&subject, country, state, city, org, unit, vin, tboxSn) != 0) {
            break;
        }

        csrPos = X509_REQ_new();
        if (csrPos == NULL) {
            break;
        }

        if (X509_REQ_set_version(csrPos, 0L) != 1) break;
        if (X509_REQ_set_subject_name(csrPos, subject) != 1) break;
        if (X509_REQ_set_pubkey(csrPos, pkey) != 1) break;
        if (X509_REQ_sign(csrPos, pkey, EVP_sha256()) <= 0) break;

        *csr = csrPos;
        csrPos = NULL;
        ret = 0;
    } while (0);

    if (subject != NULL) {
        X509_NAME_free(subject);
    }
    if (csrPos != NULL) {
        X509_REQ_free(csrPos);
    }

    return ret;
}

/**
 ****************** Demo Main ******************
 **/
int OpensslGenX509ReqFile(int version, const char *subjectCN, const char *subjectOU, const char *subjectO, const char *subjectL, const char *subjectST, const char *subjectC,
                   const char *subhectEMail, EVP_PKEY *pKey, const char *fileName)
{
    FILE *pFile = NULL;
    X509_REQ *x509Req = NULL;
    X509_NAME *x509Name = NULL;
    int nVersion = version;
    int ret = 0;

    do{
        x509Req = X509_REQ_new();
        x509Name = X509_REQ_get_subject_name(x509Req);
        X509_REQ_set_version(x509Req, nVersion);
        X509_NAME_add_entry_by_txt(x509Name, "CN", MBSTRING_ASC, (const uint8_t *)subjectCN, -1, -1, 0);
        X509_NAME_add_entry_by_txt(x509Name, "OU", MBSTRING_ASC, (const uint8_t *)subjectOU, -1, -1, 0);
        X509_NAME_add_entry_by_txt(x509Name, "O", MBSTRING_ASC, (const uint8_t *)subjectO, -1, -1, 0);
        X509_NAME_add_entry_by_txt(x509Name, "L", MBSTRING_ASC, (const uint8_t *)subjectL, -1, -1, 0);
        X509_NAME_add_entry_by_txt(x509Name, "ST", MBSTRING_ASC, (const uint8_t *)subjectST, -1, -1, 0);
        X509_NAME_add_entry_by_txt(x509Name, "C", MBSTRING_ASC, (const uint8_t *)subjectC, -1, -1, 0);
        X509_NAME_add_entry_by_txt(x509Name, "emailAddress", MBSTRING_ASC, (const uint8_t *)subhectEMail, -1, -1, 0);

        ret = X509_REQ_set_pubkey(x509Req, pKey);
        LOG_Warn("X509_REQ_set_pubkey ret = %d", ret);
        if (ret != 1)
        {
            LOG_Warn("MakeLocalCsrSSLApi X509_REQ_set_pubkey err");
            ret = -1;
            break;
        }

        ret = X509_REQ_sign(x509Req, pKey, EVP_sha256());
        LOG_Warn("X509_REQ_sign ret = %d", ret);
        if (ret <= 0) {
            LOG_Warn("failed to signaturen the certificate");
            ret = -1;
            break;
        } 
           
        pFile = fopen(fileName, "w");
        if (pFile != NULL) {
            PEM_write_X509_REQ(pFile, x509Req);
            fclose(pFile);
        }

        ret = 0;
    }while(0);
    
    if (x509Req){
        X509_REQ_free(x509Req);
    }

    return ret;
}

int main()
{
    int ret = 0;
    EVP_PKEY* pkiComKey = NULL;
    EVP_PKEY* tmpKey = NULL;
    X509_REQ* tmpCsr = NULL;
    ENGINE* myOpensslEngine = NULL;
    uint8_t  randomBuf[384];

    LOG_INIT("demo");

    LOG_Warn("****************** demo start ******************");


    ret = OpensslEngineInit(&myOpensslEngine);
    LOG_Warn("OpensslEngineInit ret = %d", ret);
    if(ret !=0){
        LOG_Warn("OpensslEngineInit failed");
        return -1;
    }

    ret = OpensslRsaGenerateTmpKey(&tmpKey, 2048);
    LOG_Warn("OpensslRsaGenerateTmpKey ret = %d tmpKey:%p", ret, (void*)tmpKey);
    if(ret !=0){
        LOG_Warn("OpensslEngineInit failed");
        return -1;
    }

    /** 生成随机数 **/
#if 1
    {
        ret = RAND_bytes(randomBuf, sizeof(randomBuf));
        LOG_Warn("RAND_bytes ret:%d", ret);
        DumpHex(randomBuf, sizeof(randomBuf));
    }
#endif

    /** 生成秘钥对 **/
#if 1
    LOG_Warn("++++++++++++++++++++++++++++++++ Rsa KeyGenerate ++++++++++++++++++++++++++++++++");
    {
        ret = DemoGenerateRsaPrivateKey(&pkiComKey, 2048);
        LOG_Warn("DemoGenerateRsaPrivateKey ret = %d pkiComKey:%p", ret, (void*)pkiComKey);
        if (ret == 0){
            DumpRasPublicKey(pkiComKey);
        }
    }
#endif

    /** 加解密 **/
#if 1
    LOG_Warn("++++++++++++++++++++++++++++++++ Rsa Encrypt-Decrypt ++++++++++++++++++++++++++++++++");
    {

        uint8_t plain[] = "Hello, World Hello, World 123456789 abcdefg en-de";
        uint8_t encryptedData[256]={0};
        uint8_t decryptedData[256]={0};
        int encryptedDataSz = sizeof(encryptedData);
        int decryptedDataSz = sizeof(decryptedData);
        
        do{
            ret = DemoRsaEncrypt(tmpKey, RSA_PKCS1_OAEP_PADDING, plain, sizeof(plain)-1, encryptedData, &encryptedDataSz);
            LOG_Warn("DemoRsaEncrypt ret = %d, Sz = %d", ret, encryptedDataSz);
            if (ret != 0 || encryptedDataSz <= 0){
                break;
            }

            LOG_Warn("encryptedData[%d]", decryptedDataSz);
            DumpHex(encryptedData, encryptedDataSz);

            ret = DemoRsaDecrypt(tmpKey, RSA_PKCS1_OAEP_PADDING, encryptedData, encryptedDataSz, decryptedData, &decryptedDataSz);
            LOG_Warn("DemoRsaDecrypt ret = %d, Sz = %d", ret, decryptedDataSz);
            if (ret != 0 || decryptedData <= 0){
                break;
            }

            LOG_Warn("decryptedData[%d]: %s", decryptedDataSz, decryptedData);
        
        }while(0);
    }
#endif


#if 1
    LOG_Warn("++++++++++++++++++++++++++++++++ Rsa Sign-Verify ++++++++++++++++++++++++++++++++");
    {
        uint8_t plain[] = "Hello, World Hello, World 123456789 abcdefg signaturen-verify";
        uint8_t signature[256]={0};
        uint32_t signatureSz = sizeof(signature);

        do{

            ret = DemoRsaSign(tmpKey, plain, sizeof(plain)-1, signature, &signatureSz);
            LOG_Warn("DemoRsaSign ret = %d, Sz = %d", ret, signatureSz);
            if (ret != 0 || signatureSz <= 0){
                break;
            }
            
            DumpHex(signature, signatureSz);

            ret = DemoRsaVerify(tmpKey,  plain, sizeof(plain)-1, signature, signatureSz);
            LOG_Warn("DemoRsaVerify ret = %d", ret);

            ret = OpensslGenX509ReqFile(202403, "sirun.test1.test2.test3.test4",
                    "SIRUNTIANLANG INC. CERT",
                    "SIRUNTIANLANG",
                    "BeiJing",
                    "BeiJing",
                    "CN",
                    "jiangze.jia@sirun.net",
                    tmpKey,
                    "/tmp/test.csr");
            LOG_Warn("OpensslGenX509ReqFile ret = %d", ret);

            ret = DemoGenerateCsr(&tmpCsr, tmpKey, "CN", "BeiJing", "BeiJing", "SIRUNTIANLANG", "SIRUNTIANLANG INC. CERT", "VIN12334455","SN7389383");
            LOG_Warn("DemoGenerateCsr ret = %d", ret);

        }while(0);
    }
#endif

    if (pkiComKey != NULL){
        EVP_PKEY_free(pkiComKey);
        pkiComKey = NULL;
    }

    if (tmpKey != NULL){
        EVP_PKEY_free(tmpKey);
        tmpKey = NULL;
    }

    if (tmpCsr != NULL) {
        X509_REQ_free(tmpCsr);
        tmpCsr = NULL;
    }

    OpensslEngineDeinit(&myOpensslEngine);

    LOG_Warn("****************** demo exit ******************");

    return 0;
}
