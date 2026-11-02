#include "https.h"
#include <curl/curl.h>
#include "https_interface.h"
#include "cjson_interface.h"
#include "system_interface.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sr_log.h"

int HttpsInit()
{
	return curl_global_init(CURL_GLOBAL_ALL);
}

void HttpsDeinit()
{
	curl_global_cleanup();
}

static size_t HttpsPostClientWriteCallback(void *contents, size_t size, size_t nmemb, void *userp) 
{
    LOG_Debug("HttpsPostClientWriteCallback start...\n");
    size_t totalSize = size * nmemb;
    HTTPSClientParams *param = (HTTPSClientParams *)userp;
    
    if (param->httpscallback) {
        if(param->httpscallback(contents,size,nmemb,userp) < 0)
        {
            return -1;
        }

    }

    return totalSize;
}

static size_t HttpPostClientWriteCallback(void *contents, size_t size, size_t nmemb, void *userp) 
{
    LOG_Debug("HttpPostClientWriteCallback start...\n");
    size_t totalSize = size * nmemb;
    HTTPSClientParams *param = (HTTPSClientParams *)userp;
    
    if (param->httpscallback) {
        
        if(param->httpscallback(contents,size,nmemb,userp) < 0)
        {
            return -1;
        }

    }

    return totalSize;
}



int HttpPostClient(const HTTPSClientParams *params) 
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
    
    if(params->header2)
    {
        headers = curl_slist_append(headers, params->header2);
    }
    
    if(params->header3)
    {
        headers = curl_slist_append(headers, params->header3);
    }

    if(params->header1 || params->header2 || params->header3)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
    
#if 0
    // 设置为 POST 请求
    res = curl_easy_setopt(curl, CURLOPT_POST, 1L);
    if (res != CURLE_OK) {

        fprintf(stderr, "[ERROR] curl_easy_setopt CURLOPT_POST failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return -1;
    }
#endif
    
    if(params->post_data)
    {
        LOG_Debug("params->post_data = %s",params->post_data);
        res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params->post_data);
        if (res != CURLE_OK) {

            LOG_Error("[ERROR] curl_easy_setopt  CURLOPT_POSTFIELDS  failed: %s\n", curl_easy_strerror(res));
            if(headers)
            {
                curl_slist_free_all(headers);
            }
            curl_easy_cleanup(curl);
            return -1;
        }
    }

    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpPostClientWriteCallback);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_setopt CURLOPT_WRITEFUNCTION failed: %s\n", curl_easy_strerror(res));
        if(headers)
        {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
        return -1;
    }

    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)params);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform CURLOPT_WRITEDATA failed: %s\n", curl_easy_strerror(res));
        if(headers)
        {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
        return -1;
    }

    res = curl_easy_setopt (curl,CURLOPT_CONNECTTIMEOUT, 60L);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform CURLOPT_CONNECTTIMEOUT failed: %s\n", curl_easy_strerror(res));
        if(headers)
        {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
        return -1;
    }

    res = curl_easy_setopt (curl,CURLOPT_TIMEOUT, 600L);
    if (res != CURLE_OK) {

        LOG_Error( "[ERROR] curl_easy_perform CURLOPT_TIMEOUT failed: %s\n", curl_easy_strerror(res));
        if(headers)
        {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
        return -1;
    }
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
   
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        if(headers)
        {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
        return -1;
    }else{
        LOG_Debug("curl_easy_perform 成功 \n");
        printf("HttpPostClient 成功... \n");
    }

    if(headers)
    {
        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
    
    return 0;
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

    LOG_Debug("HttpPostClient url = %s\n",params->url);
    res = curl_easy_setopt(curl, CURLOPT_URL, params->url);
    if (res != CURLE_OK) {

        fprintf(stderr, "[ERROR] curl_easy_setopt  CURLOPT_URL  failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return -1;
    }
    
    
    if(params->header1)
    {
        headers = curl_slist_append(headers, params->header1);
    }
    
    if(params->header2)
    {
        headers = curl_slist_append(headers, params->header2);
    }
    
    if(params->header1 || params->header2)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    res = curl_easy_setopt(curl, CURLOPT_POST, 1L);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_setopt CURLOPT_POST failed: %s\n", curl_easy_strerror(res));
        if(headers)
        {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
        curl_easy_cleanup(curl);
        return -1;
    }
  
    if(params->post_data)
    {
        res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params->post_data);
        if (res != CURLE_OK) {

            LOG_Error("[ERROR] curl_easy_setopt  CURLOPT_POSTFIELDS  failed: %s\n", curl_easy_strerror(res));
            if(headers)
            {
                curl_slist_free_all(headers);
            }
            curl_easy_cleanup(curl);
            return -1;
        }
    }

    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpsPostClientWriteCallback);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_setopt CURLOPT_WRITEFUNCTION failed: %s\n", curl_easy_strerror(res));
        if(headers)
        {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
        return -1;
    }

    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)params);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform CURLOPT_WRITEDATA failed: %s\n", curl_easy_strerror(res));
        if(headers)
        {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
        return -1;
    }
   

   res = curl_easy_setopt (curl,CURLOPT_CONNECTTIMEOUT, 60L);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform CURLOPT_CONNECTTIMEOUT failed: %s\n", curl_easy_strerror(res));
        if(headers)
        {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
        return -1;
    }

    res = curl_easy_setopt (curl,CURLOPT_TIMEOUT, 600L);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform CURLOPT_TIMEOUT failed: %s\n", curl_easy_strerror(res));
        if(headers)
        {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        if(headers)
        {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
        return -1;
    }else{
        LOG_Debug("curl_easy_perform 成功 \n");
    }


    if(headers)
    {
        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
    
    return 0;
}

static size_t http_write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) 
{

    size_t totalSize = size * nmemb;
    HTTPSClientParams *param = (HTTPSClientParams *)userdata;
    LOG_Debug("http_write_callback start ...\n");

    if (param->httpscallback) {

        if(param->httpscallback(ptr,size,nmemb,userdata) < 0)
        {
            return -1;
        }

    }

    return totalSize;
}


int HttpsGetClient(HTTPSClientParams *params) 
{

    if (!params || !params->url || !params->file) 
    {
        LOG_Error("[ERROR] Invalid parameters!  \n");
        return -1;
    }

    CURL *curl;
    CURLcode res;
  
    FILE *file = fopen(params->file, "wb");
    if (file == NULL) {
        LOG_Error("Error opening or creating file \n");
        return -1;
    }

    params->hardler = file;
    curl = curl_easy_init();
    if (!curl) {
        LOG_Error("[ERROR] Failed to initialize CURL.\n");
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, params->url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, params);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    res = curl_easy_setopt (curl,CURLOPT_CONNECTTIMEOUT, 60L);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform CURLOPT_CONNECTTIMEOUT failed: %s\n", curl_easy_strerror(res));
        fclose(file);
        curl_easy_cleanup(curl);
        return -1;
    }

    res = curl_easy_setopt (curl,CURLOPT_TIMEOUT, 800L);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform CURLOPT_TIMEOUT failed: %s\n", curl_easy_strerror(res));
        fclose(file);
        curl_easy_cleanup(curl);
        return -1;
    }

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_Error("[ERROR] curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        fclose(file);
        curl_easy_cleanup(curl);
        return -1;
    }

    if (fclose(file) != 0) {
        LOG_Error("Error closing file \n");
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_cleanup(curl);
    return 0;
}


