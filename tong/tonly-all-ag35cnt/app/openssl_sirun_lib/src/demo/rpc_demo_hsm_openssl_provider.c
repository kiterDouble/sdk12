#include "hsm_openssl_provider.h"

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
            printf("%s\n", hexBuf);     
            hexBufPos = 0; 
        }
    }
}
   

/* =========================================================
 * LOAD PROVIDER
 * ========================================================= */

static OSSL_PROVIDER *LoadProvider(void)
{
    OSSL_PROVIDER *prov;

    prov = OSSL_PROVIDER_load(NULL, SIRUN_SSL_PROVIDER_NAME);

    if (!prov) {
        printf("[%s] failed to load provider\n", __func__);
        return NULL;
    }

    printf("[%s] provider loaded\n", __func__);

    return prov;
}

static EVP_PKEY *GenerateKey(uint16_t keyId, int bits, int useHsmGenKey)
{
    EVP_PKEY_CTX *kctx = EVP_PKEY_CTX_new_from_name(NULL, "RSA", SIRUN_SSL_PROVIDER_DEFINATION);
    if (!kctx) {
        printf( "EVP_PKEY_CTX_new_from_name failed\n");
        return NULL;
    }

    EVP_PKEY *pkey = NULL;
    OSSL_PARAM params[] = {
        OSSL_PARAM_construct_uint(SIRUN_SSL_PROVIDER_PARAM_KEYID, (uint32_t *)&keyId),
        OSSL_PARAM_construct_int(SIRUN_SSL_PROVIDER_PARAM_HSMGENFLAG, &useHsmGenKey),        
        OSSL_PARAM_END
    };

    if (EVP_PKEY_keygen_init(kctx) <= 0) {
        printf( "EVP_PKEY_keygen_init failed\n");
        EVP_PKEY_CTX_free(kctx);
        return NULL;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(kctx, bits) <= 0) {
        printf( "set bits failed\n");
        EVP_PKEY_CTX_free(kctx);
        return NULL;
    }

    if (EVP_PKEY_CTX_set_params(kctx, params) <= 0) {
        printf( "set_params failed\n");
        EVP_PKEY_CTX_free(kctx);
        return NULL;
    }

    if (EVP_PKEY_keygen(kctx, &pkey) <= 0) {
        printf( "EVP_PKEY_keygen failed\n");
        EVP_PKEY_CTX_free(kctx);
        return NULL;
    }

    EVP_PKEY_CTX_free(kctx);
    return pkey;
}

static int test_provider_random(void)
{
    uint8_t buf[384];

    EVP_RAND *rand_alg = EVP_RAND_fetch(NULL, "CTR-DRBG", SIRUN_SSL_PROVIDER_DEFINATION);
    if (!rand_alg) {
        printf( "EVP_RAND_fetch failed\n");
        return 0;
    }

    EVP_RAND_CTX *rand_ctx = EVP_RAND_CTX_new(rand_alg, NULL);
    if (!rand_ctx) {
        printf( "EVP_RAND_CTX_new failed\n");
        EVP_RAND_free(rand_alg);
        return 0;
    }

    if (EVP_RAND_instantiate(rand_ctx, 256, 0, NULL, 0, NULL) != 1) {
        printf( "EVP_RAND_instantiate failed\n");
        EVP_RAND_CTX_free(rand_ctx);
        EVP_RAND_free(rand_alg);
        return 0;
    }

    if (EVP_RAND_generate(rand_ctx, buf, sizeof(buf), 256, 0, NULL, 0) != 1) {
        printf( "EVP_RAND_generate failed\n");
        EVP_RAND_CTX_free(rand_ctx);
        EVP_RAND_free(rand_alg);
        return 0;
    }

    printf("Random bytes: ");
    DumpHex(buf, sizeof(buf));

    EVP_RAND_CTX_free(rand_ctx);
    EVP_RAND_free(rand_alg);
    return 1;
}

/* =========================================================
 * Asymmetric SignData/VerifyData
 * ========================================================= */
static int SignData(EVP_PKEY *pkey, const uint8_t *msg, size_t msglen, uint8_t *sig, size_t *siglen)
{
    int ret = 0;

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx)
        return 0;

    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkey, SIRUN_SSL_PROVIDER_DEFINATION);

    if (!pctx) {
        printf("EVP_PKEY_CTX_new_from_pkey failed\n");
        goto end;
    }

    EVP_MD_CTX_set_pkey_ctx(mdctx, pctx);

    if (EVP_DigestSignInit(mdctx,NULL,EVP_sha256(),NULL,pkey) <= 0)
    {
        printf("EVP_DigestSignInit failed\n");
        goto end;
    }

    if (EVP_PKEY_CTX_set_rsa_padding( pctx, RSA_NO_PADDING) <= 0)
    {
        printf("set padding failed\n");
        goto end;
    }

    if (EVP_DigestSign(mdctx,sig,siglen,msg,msglen) <= 0)
    {
        printf("EVP_DigestSign(sign) failed\n");
        goto end;
    }
    printf("sign success siglen=%zu\n", *siglen);
    ret = 1;
end:
    EVP_MD_CTX_free(mdctx);

    return ret;
}

static int VerifyData(EVP_PKEY *pkey, const uint8_t *msg, size_t msg_len, const uint8_t *sig, size_t sig_len)
{
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx)
        return 0;

    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkey, SIRUN_SSL_PROVIDER_DEFINATION);

    if (!pctx) {
        printf( "EVP_PKEY_CTX_new_from_pkey failed\n");
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    EVP_MD_CTX_set_pkey_ctx(mdctx, pctx);

    int ret = 0;
    if (EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, pkey) != 1)
    {
        printf( "EVP_DigestVerifyInit failed\n");
        goto end;
    }

    if (EVP_PKEY_CTX_set_rsa_padding( pctx, RSA_NO_PADDING) <= 0)
    {
        printf("set padding failed\n");
        goto end;
    }    

    ret = EVP_DigestVerify(mdctx, sig, sig_len, msg,msg_len);
    if (ret != 1) {
        printf("EVP_DigestVerify failed\n");
        
    }

end:
    EVP_MD_CTX_free(mdctx);

    return (ret == 1);
}

/* =========================================================
 * Asymmetric Encryption/Decryption (RSA OAEP)
 * ========================================================= */
static int EncryptData(EVP_PKEY *pkey,const uint8_t *in, size_t inlen,uint8_t *out, size_t *outlen)
{
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx)
        return 0;

    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkey, SIRUN_SSL_PROVIDER_DEFINATION);

    if (!pctx) {
        printf( "EVP_PKEY_CTX_new_from_pkey failed\n");
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    EVP_MD_CTX_set_pkey_ctx(mdctx, pctx);

    if (EVP_PKEY_encrypt_init(pctx) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        return 0;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_PADDING) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        return 0;
    }

    int ret = EVP_PKEY_encrypt(pctx, out, outlen, in, inlen);
    EVP_PKEY_CTX_free(pctx);
    return ret == 1;
}

static int DecryptData(EVP_PKEY *pkey, const uint8_t *in, size_t inlen, uint8_t *out, size_t *outlen)
{
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx)
        return 0;

    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkey, SIRUN_SSL_PROVIDER_DEFINATION);

    if (!pctx) {
        printf( "EVP_PKEY_CTX_new_from_pkey failed\n");
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    EVP_MD_CTX_set_pkey_ctx(mdctx, pctx);

    if (EVP_PKEY_decrypt_init(pctx) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        return 0;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_PADDING) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        return 0;
    }

    int ret = EVP_PKEY_decrypt(pctx, out, outlen, in, inlen);
    EVP_PKEY_CTX_free(pctx);
    return ret == 1;
}

/* =========================================================
 * MAIN DEMO
 * ========================================================= */
 
int main()
{
    printf("==== HSM EVP DEMO START ====\n");

    EVP_PKEY *pkey  = NULL;
    OSSL_PROVIDER *prov = LoadProvider();

    if (!prov)
        return -1;

    printf("\n\n ==== random ==== \n");

    test_provider_random();

    printf("\n\n ==== GenerateKey ==== \n");
    do {
        pkey = GenerateKey(0xE1, 2048, 1);
        if (!pkey) {
            printf("keygen failed\n");
            break;
        }

        const OSSL_PROVIDER *prov = EVP_PKEY_get0_provider(pkey);
        const char *provname = OSSL_PROVIDER_get0_name(prov);
        printf("PKEY provider: %s\n", provname);

        printf("keygen success\n");
    }while(0);

#if 1
    printf("\n\n ==== SignData - VerifyData ==== \n");
    do {
        const char *test_msg = "Hello, HSM Provider! This message will be signed.";
        size_t msg_len = strlen(test_msg);
        uint8_t signature[1024] = {0};
        size_t sig_len = sizeof(signature);

        if (!pkey){
            break;
        }

        if (!SignData(pkey, (const uint8_t*)test_msg, msg_len, signature, &sig_len)) {
            printf( "Signing failed\n");
            break;
        }
        printf("Signature created, length = %zu bytes\n", sig_len);
        DumpHex(signature, sig_len);      

        if (VerifyData(pkey, (const uint8_t*)test_msg, msg_len, signature, sig_len)) {
            printf("Verification succeeded: signature is valid.\n");
        } else {
            printf( "Verification failed: signature is invalid.\n");
            break;
        }
    }while(0);
#endif

#if 1
    printf("\n\n ==== Encrypt - Decrypt ==== \n");
    do {
        uint8_t enc[256];
        size_t enclen = sizeof(enc);
        uint8_t plaintext[] = "A secret message for RSA encryption.";
        size_t plainlen = sizeof(plaintext) - 1;

        if (!EncryptData(pkey, plaintext, plainlen, enc, &enclen)) {
            printf("Encryption failed\n");
            
        } else {
            printf("Encryption succeeded, ciphertext length = %zu\n", enclen);

            uint8_t dec[256];
            size_t declen = sizeof(dec);
            if (!DecryptData(pkey, enc, enclen, dec, &declen)) {
                printf("Decryption failed\n");
                
            } else {
                printf("Decryption succeeded: %.*s\n", (int)declen, dec);
            }
        }
    }while(0);
#endif

    /* cleanup */
    EVP_PKEY_free(pkey);
    OSSL_PROVIDER_unload(prov);

    printf("==== DEMO END ====\n");

    return 0;
}


