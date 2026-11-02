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

#include "openssl_util.h"
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
    LOG_Warn("OpensslRsaGenerateTmpKey ret = %d tmpKey:%p", ret, tmpKey);
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
        ret = OpensslGenerateRsaPrivateKey(&pkiComKey, 2048);
        LOG_Warn("OpensslGenerateRsaPrivateKey ret = %d pkiComKey:%p", ret, pkiComKey);
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
            ret = OpensslRsaEncrypt(tmpKey, RSA_PKCS1_OAEP_PADDING, plain, sizeof(plain)-1, encryptedData, &encryptedDataSz);
            LOG_Warn("OpensslRsaEncrypt ret = %d, Sz = %d", ret, encryptedDataSz);
            if (ret != 0 || encryptedDataSz <= 0){
                break;
            }

            LOG_Warn("encryptedData[%d]", decryptedDataSz);
            DumpHex(encryptedData, encryptedDataSz);

            ret = OpensslRsaDecrypt(tmpKey, RSA_PKCS1_OAEP_PADDING, encryptedData, encryptedDataSz, decryptedData, &decryptedDataSz);
            LOG_Warn("OpensslRsaDecrypt ret = %d, Sz = %d", ret, decryptedDataSz);
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

            ret = OpensslRsaSign(tmpKey, plain, sizeof(plain)-1, signature, &signatureSz);
            LOG_Warn("OpensslRsaSign ret = %d, Sz = %d", ret, signatureSz);
            if (ret != 0 || signatureSz <= 0){
                break;
            }
            
            DumpHex(signature, signatureSz);

            ret = OpensslRsaVerify(tmpKey,  plain, sizeof(plain)-1, signature, signatureSz);
            LOG_Warn("OpensslRsaVerify ret = %d", ret);

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

            ret = OpensslGenerateCsr(&tmpCsr, tmpKey, "CN", "BeiJing", "BeiJing", "SIRUNTIANLANG", "SIRUNTIANLANG INC. CERT", "VIN12334455","SN7389383");
            LOG_Warn("OpensslGenerateCsr ret = %d", ret);

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
