#include "https.h"
#include <curl/curl.h>
#include "https_interface.h"
#include "cjson_interface.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sr_log.h"
#include <openssl/ssl.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include "sr_engine.h"
#include "file_interface.h"

int HttpsInit()
{
	return curl_global_init(CURL_GLOBAL_ALL);
}

void HttpsDeinit()
{
	curl_global_cleanup();
}

void TlsTrace(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr) 
{
    const char *text;
    switch (type) {
    case CURLINFO_TEXT:
        LOG_Debug( "== Info: %s", data);
    default:
        return;
    }
}


CURLcode CertSslCtxCallback(CURL *curl, void *ssl_ctx, void *userdata) 
{
    SSL_CTX *ctx = (SSL_CTX*)ssl_ctx;
    EVP_PKEY *s_pubKey = NULL;
    LOG_Debug("CertSslCtxCallback start");
    OpenSSL_add_all_algorithms();

    
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
    SSL_CTX_set_verify_depth(ctx, 4); 
    // 设置客户端证书
    if (SSL_CTX_use_certificate_file(ctx, CERTFILE, SSL_FILETYPE_PEM) <= 0) {
        LOG_Error("SSL_CTX_use_certificate_file fail \n");
        return CURLE_SSL_CERTPROBLEM;
    }
    
    // 设置客户端私钥
#if 0
    if (SSL_CTX_use_PrivateKey_file(ctx, "/oemdata/cert/private_key.key", SSL_FILETYPE_PEM) <= 0) {
         printf("SSL_CTX_use_PrivateKey_file fail \n");
        return CURLE_SSL_CERTPROBLEM;
    }
#endif

    // 2. 从加密芯片中提取RSA公钥
    if(NULL == s_pubKey)
    {
        ENGINE *engine = NULL;
        display_engine_list();
        engine = ENGINE_by_id("engine-sirun-ca");
        if (!engine)
        {
            LOG_Error("CertSslCtxCallback ENGINE_by_id failed");
            ENGINE_Init_ca_sirun();
            engine = ENGINE_by_id("engine-sirun-ca");
            if (!engine)
            {
                display_engine_list();
                LOG_Error("CertSslCtxCallback ENGINE_by_id failed");
                return CURLE_OK;
            }
        }

        s_pubKey = ENGINE_load_private_key(engine, "cert request", NULL, NULL);
    
        if (!s_pubKey)
        {
            LOG_Error("Failed to load private key");
            s_pubKey = NULL;
            return CURLE_OK;
        }
    

        if (!SSL_CTX_use_PrivateKey(ctx, s_pubKey))
        {
            EVP_PKEY_free(s_pubKey);   // 使用失败，释放
            LOG_Error("Failed to use private key: %s",
            ERR_error_string(ERR_get_error(), NULL));
            s_pubKey = NULL;
           return CURLE_OK;
        }

        if(!SSL_CTX_set1_client_sigalgs_list(ctx,"RSA+SHA256"))
        {
            LOG_Error("SSL_CTX_set1_client_sigalgs_list fail");
            EVP_PKEY_free(s_pubKey);   // 使用失败，释放
            s_pubKey = NULL;
            return CURLE_OK;
        }

        EVP_PKEY_free(s_pubKey);
    }

    // 验证私钥是否与证书匹配
    if (SSL_CTX_check_private_key(ctx) != 1) {
        LOG_Error("SSL_CTX_check_private_key fail \n");
        return CURLE_SSL_CERTPROBLEM;
    }

    // 可选：设置CA证书以验证服务器证书
    if (SSL_CTX_load_verify_locations(ctx, ROOTCACERTCHAINFILE, NULL) <= 0) {
        LOG_Error("SSL_CTX_load_verify_locations fail \n");
        return CURLE_SSL_CACERT;
    }
    LOG_Debug("CertSslCtxCallback end....\n");
    return CURLE_OK; // 必须返回 CURLE_OK
}

static size_t HttpsPostClientWriteCallback(void *contents, size_t size, size_t nmemb, void *userp) 
{
    
    size_t totalSize = size * nmemb;
    LOG_Debug("HttpsPostClientWriteCallback start totalSize=%d",totalSize);
    HTTPSClientParams *param = (HTTPSClientParams *)userp;
    
    if (param->httpscallback) {
        
        if(param->httpscallback(contents,size,nmemb,userp) < 0)
        {
            return -1;
        }

    }

    return totalSize;
}



int HttpsPostClient(const HTTPSClientParams *params) 
{
    if (!params || !params->url ) 
    {
        LOG_Error("[ERROR] Invalid parameters!\n");
        return -1;

    }

    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;

    curl = curl_easy_init();
    if (!curl) {

        LOG_Error("[ERROR] Failed to initialize CURL.\n");
        return -1;
    }
    
    LOG_Debug("HttpPostClient url = %s",params->url);
    res = curl_easy_setopt(curl, CURLOPT_URL, params->url);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_setopt  CURLOPT_URL  failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return -1;
    }
    
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    
    if(params->header1)
    {
        headers = curl_slist_append(headers, params->header1);
    }
    
    if(params->header1)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
    }

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1 );
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0 );
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, CertSslCtxCallback);

    // 设置为 POST 请求
    res = curl_easy_setopt(curl, CURLOPT_POST, 1L);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_setopt CURLOPT_POST failed: %s\n", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return -1;
    }

    LOG_Debug("params->post_data = %s \n",params->post_data);
    if(params->post_data)
    {
        res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params->post_data);
        if (res != CURLE_OK) {

            LOG_Error("[ERROR] curl_easy_setopt  CURLOPT_POSTFIELDS  failed: %s\n", curl_easy_strerror(res));
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return -1;
        }
    }

    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpsPostClientWriteCallback);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_setopt CURLOPT_WRITEFUNCTION failed: %s\n", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return -1;
    }

    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)params);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform CURLOPT_WRITEDATA failed: %s\n", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return -1;
    }

    res = curl_easy_setopt (curl,CURLOPT_CONNECTTIMEOUT, 60L);
    if (res != CURLE_OK) {

        LOG_Error( "[ERROR] curl_easy_perform CURLOPT_CONNECTTIMEOUT failed: %s\n", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return -1;
    }

    res = curl_easy_setopt (curl,CURLOPT_TIMEOUT, 100L);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform CURLOPT_TIMEOUT failed: %s\n", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, TlsTrace);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        long verify_result;
        curl_easy_getinfo(curl, CURLINFO_SSL_VERIFYRESULT, &verify_result);

        if (verify_result == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT || 
            verify_result == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN) {
            LOG_Error("双向认证失败:客户端证书未通过验证 (Code: %ld)\n", verify_result);
        } else if (res == CURLE_SSL_CERTPROBLEM) {
            LOG_Error("客户端证书问题:%s\n", curl_easy_strerror(res));
        } else if (res == CURLE_SSL_CACERT) {
            LOG_Error("CA证书不信任:%s\n", curl_easy_strerror(res));
        }
        curl_slist_free_all(headers);   
        curl_easy_cleanup(curl);
        return -1;
    }else{
        LOG_Debug("curl_easy_perform 成功 res = %d",res);
    }

   
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return res;
}


CURLcode SslCtxCallback(CURL *curl, void *ssl_ctx, void *userdata) 
{
    SSL_CTX *ctx = (SSL_CTX*)ssl_ctx;
    EVP_PKEY *ssl_pubKey = NULL;
    LOG_Debug("SslCtxCallback start");
    OpenSSL_add_all_algorithms();
    

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
    SSL_CTX_set_verify_depth(ctx, 4); 
    
    if (SSL_CTX_use_certificate_file(ctx, DEVICECERTFILE, SSL_FILETYPE_PEM) <= 0) {
        LOG_Error("SSL_CTX_use_certificate_file fail \n");
        return CURLE_SSL_CERTPROBLEM;
    }

    // 2. 从加密芯片中提取RSA公钥
    if(NULL == ssl_pubKey)
    {
        ENGINE *engine = NULL;
        engine = ENGINE_by_id("engine-sirun");
        if (!engine)
        {
            LOG_Error("ENGINE_by_id failed");
            return CURLE_OK;
        }
        
        ssl_pubKey = ENGINE_load_private_key(engine, "cert verify", NULL, NULL);
    
        if (!ssl_pubKey)
        {
            LOG_Error("Failed to load private key");
            ssl_pubKey = NULL;
            return CURLE_OK;
        }

        if (!SSL_CTX_use_PrivateKey(ctx, ssl_pubKey))
        {
            EVP_PKEY_free(ssl_pubKey);   // 使用失败，释放
            LOG_Error("Failed to use private key: %s",
            ERR_error_string(ERR_get_error(), NULL));
            ssl_pubKey = NULL;
           return CURLE_OK;
        }

        if(!SSL_CTX_set1_client_sigalgs_list(ctx,"RSA+SHA256"))
        {
            LOG_Error("SSL_CTX_set1_client_sigalgs_list fail");
            EVP_PKEY_free(ssl_pubKey);   // 使用失败，释放
            ssl_pubKey = NULL;
            return CURLE_OK;
        }

        EVP_PKEY_free(ssl_pubKey);

    }
    
    // 验证私钥是否与证书匹配
    if (SSL_CTX_check_private_key(ctx) != 1) {
        LOG_Error("SSL_CTX_check_private_key fail \n");
        return CURLE_SSL_CERTPROBLEM;
    }

    // 可选：设置CA证书以验证服务器证书
    if (SSL_CTX_load_verify_locations(ctx, "/oemapp/cert/certificate_chain_client.pem", NULL) <= 0) {
        LOG_Error("SSL_CTX_load_verify_locations fail \n");
        return CURLE_SSL_CACERT;
    }
    
    LOG_Debug("SslCtxCallback end....\n");
    return CURLE_OK; // 必须返回 CURLE_OK
}


static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) 
{
    LOG_Debug(" write_callback\n");
    return size * nmemb;
}


int HttpsPostClientCertVer(const HTTPSClientParams *params) 
{
    if (!params || !params->url ) 
    {
        LOG_Error("[ERROR] Invalid parameters!\n");
        return -1;

    }

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {

        LOG_Error("[ERROR] Failed to initialize CURL.\n");
        return -1;
    }
    
    LOG_Debug("HttpPostClient url = %s",params->url);
    res = curl_easy_setopt(curl, CURLOPT_URL, params->url);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_setopt  CURLOPT_URL  failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    
   
    res = curl_easy_setopt(curl, CURLOPT_POST, 1L);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_setopt CURLOPT_POST failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return -1;
    }


    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "certificate verification");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1 );
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0 );
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, SslCtxCallback);
   
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 50L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 50L);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, TlsTrace);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_Error("[ERROR] curl_easy_perform ret:%d failed: %s\n", res,curl_easy_strerror(res));
        // 检查错误码和SSL验证状态
        long verify_result;
        curl_easy_getinfo(curl, CURLINFO_SSL_VERIFYRESULT, &verify_result);

        if (verify_result == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT || 
            verify_result == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN) {
            LOG_Error("⚠️ 双向认证失败: 客户端证书未通过验证 (Code: %ld)\n", verify_result);
        } else if (res == CURLE_SSL_CERTPROBLEM) {
            LOG_Error("⚠️ 客户端证书问题: %s\n", curl_easy_strerror(res));
        } else if (res == CURLE_SSL_CACERT) {
            LOG_Error("⚠️ CA证书不信任: %s\n", curl_easy_strerror(res));
        }
        
        curl_easy_cleanup(curl);
        return res;
    }else{
        LOG_Debug("curl_easy_perform 成功 \n");
        
    }

    if (curl) 
    {
        curl_easy_cleanup(curl);
        curl = NULL; // 避免悬空指针
    }
    return 0;
}

