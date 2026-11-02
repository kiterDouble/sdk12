
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/err.h>
#include <stdio.h>
#include "sr_engine.h"
#include "sr_log.h"
#include "file_interface.h"
#include "system_interface.h"

static const char *privatekey_data = 
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpQIBAAKCAQEAvVraof9u8gM5mwmgOb6DNFAKTJc/4pHe4sG+74BPeol+Bsxb\n"
"zLw/8FGnMqRSZGCH8I127Xp59djpxfxmdixnrBgq+ISH8stHbG7NFCn96ytYqtWw\n"
"dU5tBmTt2V8qfxJPBdWGctun9zMa94+Tn/rsXeGUM+uS2Itw2B+H+/Qj+pl+28bQ\n"
"ztW3BRR6fBIrXEoofbajoG4nt/Hmyi234x3DpxnIwteRXWmWBPsNuRpe1CkVDq/S\n"
"IL/whVQ1G9rS9xhJcX6r5k44nu8AoUVijbjI8s1wuTBdVOikZDxGuWwFcUIOGWjA\n"
"KZrTKthYaoI89lVnEHasnyaxmWIUF2Uf7BBHyQIDAQABAoIBADVnzooqNOxlJHYp\n"
"vNsMdIzolj8L2aTZZ3elFWkPU1tINBdOxAV2gsIRBYsu9V4CK9UY0RYhmQxR5X1F\n"
"ttBVrdfH6Ta2inFcDAT1KzoorckKLLPiGSa8u9s11C+eu6mjTE5k7squUS1TaKOy\n"
"Bvz0FH5WBSqvoncYvPb3za6uKQegjo1/QCxOvmmkJj+4iwKnWB6Za6gllQD/8oix\n"
"dN2S5DnEaz6wa+P25QvBO0Yhh7WOdWalBrCKfrKcM/Ej76pyXQYlxfC66ELDCWCY\n"
"P8xKnuU++JPHA1rujA5++UOChgy98c4WWkwpO3UiNowCb6sqh/A+wsi6EW7QZtF5\n"
"63vXJkECgYEA8HVHTd0ZlE/6fVhoydXxeQFVPYGWV8S0b3bvXnRb0ouS0to+7q09\n"
"nzyCkOMu9r6Dlc35qtPFrCNm+XU8N4QhPLX/LVh/ycn6TDE8VKm7quEzuek2BgPE\n"
"1AxpJ1Iuvb2EN2AWzzwO7Jkc0SknG/VPkNgj0CyXFhZi92pq6ySmM7UCgYEAyZgA\n"
"A8DxcohNrYhIj4RuJFn0yPyIoX1kh2/+83S+e24siPNZKujcrHS7F0wvk9zVC0lu\n"
"Vmx7vukPi5LJzrc+hal0W54jLho29d6IzK2bys41hujNbJ5lQR8b+d0TmSYyTuJs\n"
"lTUSzZtZT1OfxefJIbDVyHSZU+xzieWj/dqZ+EUCgYEApmFP9EWPpHrupPqej0H0\n"
"8riEZamcAKEABLx0BpRW0Ml4z2Lp9rZylBMHeP9piJvsaqR6I8Si22gnoWaIQxWu\n"
"9dYsehDpYxPZmAzdZOuJCkUHSJVgpyrGb78ip22LIAjZSoO3ZK922CldsjBaelL0\n"
"OhIEPdAMvuDAm9fLoWbbhZ0CgYEAxuF1Cj78NKDx+UQdSbE9tBEnJG23IZKCbfd4\n"
"j5haH41G1Gk3S9TleCHFPavEuFxBtdvgxdRKlMJ6upi7DXvMa7MMfyJY1K+6DwBW\n"
"945Cx2Z+ejUMDJ+Lhx54zB1lq056jUZmFyGABmW/SzpVwrnzvJAnqJYXeisopR9v\n"
"lQ5nnf0CgYEAgUSHdROxFQfSRf0av4P1D85fvSzs4BOPIvSMGdZX160nCkJSG8TY\n"
"UqcOlnQKtjgQj+nitk6mdapL/h13/A2XceYcmBWbilQ4gczqC/GiKtkITRjTQny4\n"
"oYPqrM95s8cvt8HFKDwEayWnVxLw/7ZtS1t7H/B6+op5Ipsim3Hav3w=\n"
"-----END RSA PRIVATE KEY-----\n";

void print_bn(const char *label, const BIGNUM *bn) {
    char *hex = BN_bn2hex(bn);
    LOG_Debug("%s:\n%s\n", label, hex);

    OPENSSL_free(hex);
}

// 打印证书信息
void print_cert_info(X509 *cert) {
    char buf[256];
    X509_NAME_oneline(X509_get_subject_name(cert), buf, sizeof(buf));
    LOG_Debug("Subject: %s\n", buf);
    X509_NAME_oneline(X509_get_issuer_name(cert), buf, sizeof(buf));
    LOG_Debug("Issuer: %s\n", buf);
}

RSA* LoadRsaFromString(const char *pem_data)
{
    BIO *bio = BIO_new_mem_buf(pem_data, -1);
    RSA *rsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
    BIO_free(bio);
    return rsa;
}

int ExtractRsaParams(const char *data,char *buf,int datalen,int flag) {
   
    RSA *rsa = NULL;
    if(!data)
    {
        LOG_Error("data null");
        return -1;
    }

    if(flag)
    {
        FILE *fp = fopen(data, "r");
        if (!fp) {
            LOG_Error("Failed to open PEM file");
            return -1;
        }

        // 1. 从PEM文件加载RSA私钥
        rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
        fclose(fp);
    }else{

        rsa = LoadRsaFromString(data);
    }
   
    if (!rsa) {
        LOG_Error("Failed to read RSA private key");
        ERR_print_errors_fp(stderr);
        return -1;
    }

    // 2. 提取参数
    const BIGNUM *n, *e, *d, *p, *q, *dp, *dq, *iqmp;
    RSA_get0_key(rsa, &n, &e, &d);
    RSA_get0_factors(rsa, &p, &q);
    RSA_get0_crt_params(rsa, &dp, &dq, &iqmp);
#if 0
    // 3. 打印参数信息
    printf("----- RSA Private Key Parameters -----\n");
    print_bn("Modulus (N)", n);
    print_bn("Public Exponent (E)", e);
    print_bn("Prime1 (P)", p);
    print_bn("Prime2 (Q)", q);
    print_bn("Exponent1 (Dp = d mod p-1)", dp);
    print_bn("Exponent2 (Dq = d mod q-1)", dq);
    print_bn("Coefficient (Qinv = q^-1 mod p)", iqmp);
#endif
    // 4. 验证参数
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *tmp = BN_new();

    // 验证 P * Q == N
    BN_mul(tmp, p, q, ctx);
    LOG_Debug("\nVerification:\n");
    LOG_Debug("P * Q == N? %s\n", BN_cmp(tmp, n) == 0 ? "YES" : "NO");

    // 验证 Qinv * Q ≡ 1 mod P
    BN_mod_mul(tmp, iqmp, q, p, ctx);
    LOG_Debug("Qinv * Q mod P == 1? %s\n", BN_is_one(tmp) ? "YES" : "NO");
	char *hex_n = BN_bn2hex(n);
    char *hex_e = BN_bn2hex(e);
	char *hex_p = BN_bn2hex(p);
	char *hex_q = BN_bn2hex(q);
	char *hex_dp = BN_bn2hex(dp);
	char *hex_dq = BN_bn2hex(dq);
	char *hex_iqmp = BN_bn2hex(iqmp);
    
	snprintf(buf,datalen,"%s00010001%s%s%s%s%s",hex_n,hex_p,hex_q,hex_dp,hex_dq,hex_iqmp);
	
	OPENSSL_free(hex_n);
	OPENSSL_free(hex_e);
	OPENSSL_free(hex_p);
	OPENSSL_free(hex_q);
	OPENSSL_free(hex_dp);
	OPENSSL_free(hex_dq);
	OPENSSL_free(hex_iqmp);



    BN_free(tmp);
    BN_CTX_free(ctx);
    RSA_free(rsa);
    return 0;
}

#if 0
int HSMPrivateKeyImport(const char *data,uint16_t pub_Fid,uint16_t pri_Fid)
{
    int ret = 0;
    
    ret = ImportRSAPrivate(data, pub_Fid, pri_Fid);
    if(ret != 0)
    {
        LOG_Error("ImportRSAPrivate fail ret = %d",ret);
        return -1;
    }
   
}

int PrivateKeyImport(const char *data,uint16_t pub_Fid,uint16_t pri_Fid)
{
    int ret = 0;
    char buf[2000] = {0};
    ret  = ExtractRsaParams(data,buf,sizeof(buf),0);
    if(ret != 0)
    {
        LOG_Error("ExtractRsaParams fail");
        return -1;
    }
    ret = HSMPrivateKeyImport(buf ,pub_Fid, pri_Fid);
    if(ret != 0)
    {
        LOG_Error("HSMPrivateKeyImport fail");
        return -1;
    }
    return 0;
}

int CAPrivateKeyImport()
{
    int ret = 0;
    char buf[2000] = {0};


    ret  = ExtractRsaParams(privatekey_data,buf,sizeof(buf),0);
    if(ret != 0)
    {
        LOG_Error("ExtractRsaParams fail");
        return -1;
    }
    
    ret =  HsmInit();
    if(ret != 0)
    {
        LOG_Error("HsmInit fail");
        return -1;
    }
    ret = HSMPrivateKeyImport(buf ,PUB_FID_02, PRI_FID_02);
    if(ret != 0)
    {
        LOG_Error("HSMPrivateKeyImport fail");
        HsmDeInit();
        return -1;
    }

   
    ret = GetSystemRetValue("touch /oemdata/cert/private.txt");
    if(ret == -1)
    {
        LOG_Error("touch /oemdata/cert/private.txt fail");
    }

    HsmDeInit();
    return 0;
}
#endif

int TeePrivateKeyImport(const char *data,uint32_t pri_len,uint16_t pri_Fid)
{
    int ret = 0;
    LOG_Debug("TeePrivateKeyImport pri_len:%d pri_Fid:%d",pri_len,pri_Fid);
    ret = DeleteRSAPrivate(pri_Fid);
    if(ret < 0 )
    {
        LOG_Error("DeleteRsaPrivate fail");
    }
    ret = ImportRSAPrivate(data ,pri_len, pri_Fid);
    if(ret != 0)
    {
        LOG_Error("PrivateKeyImport fail");
        return -1;
    }
    return 0;
}

int CAPrivateKeyImport()
{
    int ret = 0;

    ret = TeePrivateKeyImport(privatekey_data ,strlen(privatekey_data), 1);
    if(ret != 0)
    {
        LOG_Error("PrivateKeyImport fail");
        return -1;
    }

    ret = GetSystemRetValue("touch /oemdata/cert/private.txt");
    if(ret == -1)
    {
        LOG_Error("touch /oemdata/cert/private.txt fail");
    }

    return 0;
}

// 验证证书链
int VerifyCertificateChain(const char *device_cert_path, const char *ca_chain_path) 
{
    int ret = 0;
    X509_STORE *store = NULL;
    X509_STORE_CTX *ctx = NULL;
    X509 *cert = NULL;
    FILE *fp = NULL;

    // 1. 创建证书存储
    store = X509_STORE_new();
    if (!store) {
        LOG_Error("Error creating X509 store");
        goto end;
    }

    // 2. 加载CA证书链（PEM格式）
    if (X509_STORE_load_locations(store, ca_chain_path, NULL) != 1) {
        LOG_Error("Error loading CA chain");
        ERR_print_errors_fp(stderr);
        goto end;
    }

    // 3. 加载设备证书
    fp = fopen(device_cert_path, "r");
    if (!fp) {
        LOG_Error("Error opening device cert");
        goto end;
    }
    cert = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose(fp);
    if (!cert) {
        LOG_Error("Error reading device cert");
        ERR_print_errors_fp(stderr);
        goto end;
    }

    LOG_Debug("--- Device Certificate ---\n");
    //print_cert_info(cert);

    // 4. 创建验证上下文
    ctx = X509_STORE_CTX_new();
    if (!ctx || !X509_STORE_CTX_init(ctx, store, cert, NULL)) {
        LOG_Error("Error initializing verify context");
        goto end;
    }

    // 5. 执行验证（检查证书链）
    ret = X509_verify_cert(ctx);
    if (ret == 1) {
        LOG_Debug("Certificate verification SUCCESS");
    } else {
        LOG_Error("Certificate verification FAILED");
        int err = X509_STORE_CTX_get_error(ctx);
        LOG_Error("Error code: %d (%s)\n", err, X509_verify_cert_error_string(err));
        ERR_print_errors_fp(stderr);
    }

end:
    // 6. 清理资源
    X509_STORE_CTX_free(ctx);
    X509_free(cert);
    X509_STORE_free(store);
    return ret == 1 ? 0 : -1;
}



void SiRunPKIEngineInit()
{
    int ret = 0;
    ENGINE *engine = NULL;
    engine = ENGINE_by_id("engine-sirun");
    if (!engine)
    {
        LOG_Debug("ENGINE_by_id null");
        ret = ENGINE_Init_sirun();
        if(ret != 0)
        {
            LOG_Error("ENGINE Init sirun fail");
            return ;
        }
    }

    return ;
}

void SiRunPKIEngineDeInit()
{
    ENGINE *engine = NULL;
    engine = ENGINE_by_id("engine-sirun");
    if (engine)
    {
        LOG_Debug("engine true");
        ENGINE_DeInit_sirun();
    }
    
}



static int GetCNFromCert(const char *cert_path, char* cert_cn) 
{
    FILE *fp = NULL;
    X509 *cert = NULL;
    X509_NAME *subject_name = NULL;
    char common_name[256] = {0};
    
    // 打开证书文件
    fp = fopen(cert_path, "r");
    if (!fp) {
        LOG_Error("Unable to open certificate file");
        return -1;
    }

    // 读取证书 (PEM 格式)
    cert = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose(fp);

    if (!cert) {
        LOG_Error("Error reading certificate");
        ERR_print_errors_fp(stderr);
        return -1;
    }

    // 获取证书的 subject (主题)
    subject_name = X509_get_subject_name(cert);
    if (!subject_name) {
        LOG_Error("Failed to get subject from certificate");
        X509_free(cert);
        return -1;
    }

    // 从 subject 中提取 CN
    if (X509_NAME_get_text_by_NID(subject_name, NID_commonName, cert_cn, sizeof(common_name)) > 0) {
        LOG_Debug("Common Name (CN): %s", cert_cn);
    } else {
        LOG_Error("No CN found in certificate");
        return -1;
    }

    // 释放证书对象
    X509_free(cert);
    return 0;
}


int CertificateOfVehicleNumberChangeApplication()
{
    char carid[30] = {0};
    char cn[50] = {0};
    int ret = 0;

    if(!GetCarNum((uint8_t*)carid, 10))
    {
        LOG_Error("GetCarNum fail");
        return -1;
    }

    ret = GetCNFromCert(DEVICECERTFILE ,cn);
    if(ret < 0 )
    {
        LOG_Error("GetCNFromCert fail");
        return -1;
    }

    char *result = strstr(cn,carid);
    if (result != NULL) {
        LOG_Debug("Vehicle number no change cn: %s  car: %s", cn, carid);
        return 0;
    } else {
        LOG_Debug("Vehicle number change cn: %s  car: %s", cn, carid);
        return 1;
    }


    return 0;
}