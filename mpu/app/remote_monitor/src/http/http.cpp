#include "http.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sr_log.h"


void HttpInit()
{
    CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
    if (ret != CURLE_OK)
    {
        LOG_Error("curl_global_init() failed: %s\n", curl_easy_strerror(ret));
    }
}

void HttpDeinit()
{
    curl_global_cleanup();
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
    

    if(params->header1)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
    
    if(params->post_data)
    {
        LOG_Debug("params->post_data = %s",params->post_data);
        res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params->post_data);
        if (res != CURLE_OK) {

            LOG_Error("[ERROR] curl_easy_setopt  CURLOPT_POSTFIELDS  failed: %s\n", curl_easy_strerror(res));

        }
    }

    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpPostClientWriteCallback);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_setopt CURLOPT_WRITEFUNCTION failed: %s\n", curl_easy_strerror(res));
    }

    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)params);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform CURLOPT_WRITEDATA failed: %s\n", curl_easy_strerror(res));
    }

    res = curl_easy_setopt (curl,CURLOPT_CONNECTTIMEOUT, 10L);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform CURLOPT_CONNECTTIMEOUT failed: %s\n", curl_easy_strerror(res));

    }

    res = curl_easy_setopt (curl,CURLOPT_TIMEOUT, 10L);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform CURLOPT_TIMEOUT failed: %s\n", curl_easy_strerror(res));
        
    }
   

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {

        LOG_Error("[ERROR] curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        
    }else{
        LOG_Debug("curl_easy_perform 成功 \n");
        printf("HttpPostClient 成功... \n");
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return res;
}
