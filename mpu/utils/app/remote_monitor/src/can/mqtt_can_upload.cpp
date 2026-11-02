#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <stdint.h>
#include <openssl/engine.h>
#include "err_code.h"
#include "sr_log.h"
#include "system_interface.h"
#include "cycle_get_modem_info.h"
#include "cjson_interface.h"
#include "mqtt.h"
#include "pki_sdk.h"


#define  RET_ERR  (-1)
#define  RET_OK  (0)
// #define  CAN_ASC_POST_ADDR              "http://gateway-uat.zelostech.com.cn/data-server/tbox/upload_log_file"
#define  CAN_ASC_POST_ADDR              "https://dota-api-uat.zelosapi.com/api/file/upload"
#define  CAN_UPLOAD_RATE_LIMIT          (100*1024 * 1024)
#define  ASC_FILE_HANDLE_NAME           "/oemapp/scripts/asc_file_handle.sh"
#define  CAN_UPLOAD_FILE_NAME           "/mnt/canlog/tmp.asc"
#define  CAN_ASC_FILE_PATH              "/mnt/canlog/"
#define  CAN_UPLOAD_FILE_TMP_DIR        "/mnt/canlog/log"

// 自定义数据结构，用于控制读取速率
typedef struct readCanData {
    FILE *fileFp;
    size_t rateLimit; // 字节/秒
    size_t totalRead;
    time_t startTime;
}UploadCan_t;

typedef struct responseString {
    char *ptr;
    char *batchNumber;
    size_t len;
}UploadCanRes_t;

static pthread_mutex_t g_uploadCanLock = PTHREAD_MUTEX_INITIALIZER;
static uint64_t g_uploadCanStartTime = 0;
static uint64_t g_uploadCanEndTime = 0;
static char g_batchNumber[64] = {0};



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


CURLcode SslCtxCallback(CURL *curl, void *ssl_ctx, void *userdata) 
{
    SSL_CTX *ctx = (SSL_CTX*)ssl_ctx;
    EVP_PKEY *ssl_pubKey = NULL;
    LOG_Debug("SslCtxCallback start");
    OpenSSL_add_all_algorithms();
    

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
    SSL_CTX_set_verify_depth(ctx, 4); 
    
    if (SSL_CTX_use_certificate_file(ctx, DEVICE_CERT, SSL_FILETYPE_PEM) <= 0) {
        LOG_Error("SSL_CTX_use_certificate_file fail \n");
        return CURLE_SSL_CERTPROBLEM;
    }

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
            EVP_PKEY_free(ssl_pubKey);
            LOG_Error("Failed to use private key: %s",
            ERR_error_string(ERR_get_error(), NULL));
            ssl_pubKey = NULL;
           return CURLE_OK;
        }

        if(!SSL_CTX_set1_client_sigalgs_list(ctx,"RSA+SHA256"))
        {
            LOG_Error("SSL_CTX_set1_client_sigalgs_list fail");
            EVP_PKEY_free(ssl_pubKey);
            ssl_pubKey = NULL;
            return CURLE_OK;
        }

        EVP_PKEY_free(ssl_pubKey);

    }
    
    if (SSL_CTX_check_private_key(ctx) != 1) {
        LOG_Error("SSL_CTX_check_private_key fail \n");
        return CURLE_SSL_CERTPROBLEM;
    }

    if (SSL_CTX_load_verify_locations(ctx, "/oemapp/cert/certificate_chain_client.pem", NULL) <= 0) {
        LOG_Error("SSL_CTX_load_verify_locations fail \n");
        return CURLE_SSL_CACERT;
    }
    
    LOG_Debug("SslCtxCallback end....\n");
    return CURLE_OK;
}


int32_t SetUpLoadCanFileInfo(uint64_t startTime, uint64_t endTime, const char* batchNumber)
{
    if(NULL == batchNumber)
    {
        LOG_Error("batchNumber is NULL");
        return RET_ERR;
    }

    if(strlen(batchNumber) > 64)
    {
        LOG_Error("batchNumber len >= 64");
        return RET_ERR;
    }

    if(0 == startTime)
    {
        LOG_Error("startTime is 0");
        return RET_ERR;
    }

    if(0 == endTime)
    {
        LOG_Error("startTime is 0");
        return RET_ERR;
    }

    if((endTime < startTime) || ((endTime - startTime) > 1800))
    {
        LOG_Error("endTime[%lld] < startTime[%lld]", endTime, startTime);
        return RET_ERR;
    }

    if((endTime - startTime) > 1800)
    {
        LOG_Error("(endTime[%d] - startTime[%d]) > 1800", endTime, startTime);
        return RET_ERR;
    }
  
    pthread_mutex_lock(&g_uploadCanLock);
    g_uploadCanStartTime = startTime;
    g_uploadCanEndTime = endTime;
    memset(g_batchNumber, 0, 64);
    memcpy(g_batchNumber, batchNumber, strlen(batchNumber));
    pthread_mutex_unlock(&g_uploadCanLock);

    LOG_Debug("can upload info is set");

    return RET_OK;
}


// 自定义读取回调函数，用于控制读取速率
static size_t readCanDataCallback(void *ptr, size_t size, size_t nmemb, void *userp)
{
    UploadCan_t *uploadCan = (UploadCan_t *)userp;
    size_t retcode;
    time_t currentTime = 0;
    double elapsedTime = 0.0;
    double waitTime = 0.0;
    static int firstCall = 1;
    size_t maxAllowed = 0;
    
    size_t bytesToRead = size * nmemb;
    
    // 如果是第一次调用，记录开始时间
    if(firstCall) 
    {
        uploadCan->startTime = time(NULL);
        firstCall = 0;
    }

    // 计算已经过去的时间
    currentTime = time(NULL);
    elapsedTime = difftime(currentTime, uploadCan->startTime);
    
    // 计算允许读取的最大字节数（基于速率限制）
    maxAllowed = (size_t)(uploadCan->rateLimit * elapsedTime) - uploadCan->totalRead;
    
    // 如果已经读取的字节数超过了允许的最大值，等待一段时间
    if (maxAllowed < bytesToRead) 
    {
        if (maxAllowed <= 0) 
        {
            // 需要等待
            waitTime = (double)bytesToRead / uploadCan->rateLimit - elapsedTime;
            if (waitTime > 0 && waitTime < 5) 
            {
                usleep((useconds_t)(waitTime * 1000000)); // 微秒
            }
        } 
        else 
        {
            bytesToRead = maxAllowed;
        }
    }
    
    // 从文件中读取数据
    retcode = fread(ptr, 1, bytesToRead, uploadCan->fileFp);
    
    // 更新已读取的总字节数
    uploadCan->totalRead += retcode;
    
    if(0 == retcode)
    {
        firstCall = 1;
    }
    LOG_Debug("post len %d", retcode);

    return retcode;
}


size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) 
{
    char resBuff[1024] = {0};
    int ret = 0;
    char *jsonstring = NULL;
    size_t realsize = size * nmemb;
    UploadCanRes_t *s = (UploadCanRes_t *)userp;
    HttpCanJsonResponse response = {0};
    ReplyLogDataParameter parameter = {0};
    mqtt_data data = {0};
    if(NULL != contents && realsize < 1024)
    {
        LOG_Debug("post res [%d]:%s", realsize, contents);
        
        ret = ParseHttpCanJsonResponse((char*)contents, &response);
        if(ret < 0 )
        {
            LOG_Error("Parse Http Can Json Response Fail");

        }else{

            LOG_Debug("Parse Http Can Json Response ok code = %ld data = %s msg = %s",response.code,response.data,response.msg);
        }

        if(s->batchNumber)
        {
            parameter.batchNumber = s->batchNumber;
        }
        
        if(response.data)
        {
            parameter.fileKey = response.data;
        }

        if(response.msg)
        {
            if(strcmp(response.msg,"成功") == 0)
            {
                parameter.status = 1;
            }else{
                parameter.status = 0;
            }
        }
        
        ret = ReplyLogDataJsonInterface(parameter,&jsonstring);
        if(ret < 0 )
        {
            LOG_Error("Reply Log Data Json Interface Error");
        }
        
        FreeHttpCanJson(&response);
        memset(&data,0,sizeof(mqtt_data));
        data.payload  = jsonstring;
        if(!data.payload)
        {
            LOG_Error("CAN Json string NULL");
           
        }
        data.topic = MqttHandle::getInstance().topic_handle.tbox_can_reply_topic;
        data.payloadlen = strlen(data.payload);
        mqtt_publish(data,1);
        if(data.payload)
        {
            free(data.payload);
        }

    }else{
        LOG_Error("call back error");
    }

    
    return realsize;
}

int PostCanFileToZelos(const char* curlAddr, const char* fileName, char *batchNumber, uint32_t startTime) 
{
    CURL *curl;
    CURLcode res;
    UploadCan_t readCanData = {0};
    UploadCanRes_t resStr = {0};
    long fileSize = 0;
    char carNum[30] = {0};
    char tmpBuff[128] = {0};
    time_t stTmpTime;
    struct tm *pstRetTime;
    int ret = 0;

    if(NULL == curlAddr)
    {
        LOG_Error("curlAddr is NULL");
        return RET_ERR;
    }

    if(NULL == fileName)
    {
        LOG_Error("fileName is NULL");
        return RET_ERR;
    }

    if(NULL == batchNumber)
    {
        LOG_Error("batchNumber is NULL");
        return RET_ERR;
    }

    ret = CertificateInspection();
    if(ret < 0 )
    {
        LOG_Error("CertificateInspection error ");
        return RET_ERR;
    }
    
    // 打开要上传的文件
    FILE *fileFp = fopen(fileName, "rb");
    if(NULL == fileFp) 
    {
        LOG_Error("无法打开文件: %s\n", fileName);
        return RET_ERR;
    }
    
    if(GetVehicleName(carNum) == -1)
    {
        LOG_Error("%s ,GetCarNum error",__FUNCTION__);
        return RET_ERR;
    }

    
    stTmpTime = startTime;
    stTmpTime += 28800;
    pstRetTime = gmtime(&stTmpTime);

    snprintf(tmpBuff, 128, "%s-%d%d%d%d%d%d.asc", carNum, (pstRetTime->tm_year + 1900),\
                            (pstRetTime->tm_mon + 1), pstRetTime->tm_mday, pstRetTime->tm_hour,\
                            pstRetTime->tm_min, pstRetTime->tm_sec);

    // 获取文件大小
    fseek(fileFp, 0, SEEK_END);
    fileSize = ftell(fileFp);
    fseek(fileFp, 0, SEEK_SET);
    
    // // 初始化libcurl
    // curl_global_init(CURL_GLOBAL_DEFAULT);
    
    LOG_Debug("start post");
    LOG_Debug("file tmpBuff:%s",tmpBuff);
    // 创建curl句柄
    curl = curl_easy_init();
    if(curl) 
    {
        // 创建multipart表单
        struct curl_httppost *formpost = NULL;
        struct curl_httppost *lastptr = NULL;
        
        // 添加字符串参数N到表单
        curl_formadd(&formpost, &lastptr,
                     CURLFORM_COPYNAME, "batchNumber",
                     CURLFORM_COPYCONTENTS, batchNumber,
                     CURLFORM_END);
        
        // 准备自定义读取数据结构
        
        readCanData.fileFp = fileFp;
        readCanData.rateLimit = CAN_UPLOAD_RATE_LIMIT; 
        readCanData.totalRead = 0;
        
        curl_formadd(&formpost, &lastptr,
                     CURLFORM_COPYNAME, "file",
                     CURLFORM_FILENAME, tmpBuff,
                     CURLFORM_STREAM, &readCanData,
                     CURLFORM_CONTENTTYPE, "application/octet-stream",
                     CURLFORM_END);
        
        // 设置目标URL（替换为你的实际URL）
        curl_easy_setopt(curl, CURLOPT_URL, curlAddr);
        
        //tls1.2
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1 );
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0 );
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, SslCtxCallback);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, TlsTrace);
        // 设置读取回调函数和数据
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCanDataCallback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &readCanData);
        resStr.batchNumber = batchNumber;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resStr);

        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fileSize); // 必须！

        curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);

        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 0L); // 关闭 TCP keepalive
        curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L); // 强制新建连接   

        // 设置连接超时（建议 10-30 秒）
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);

        // 设置整个请求超时（包括上传）（建议 60-300 秒）
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L); // 5分钟超时

        // 可选：设置低速传输超时（防止卡住）
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1024L);     // 每秒至少 1KB
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);        // 否则超时     
        // 设置表单数据
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        
        // 如果需要，可以设置其他选项
        // 例如，设置详细模式以查看请求详情
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        LOG_Debug("开始上传文件，大小: %ld 字节，速率限制: %ld 字节/秒\n", fileSize, readCanData.rateLimit);
        
        // 执行请求
        res = curl_easy_perform(curl);
        
        // 检查错误
        if(res != CURLE_OK) 
        {
            LOG_Error("curl_easy_perform() 失败: %s\n", curl_easy_strerror(res));
        } 
        else 
        {
            LOG_Debug("上传完成，总耗时: %.2f 秒\n", difftime(time(NULL), readCanData.startTime));
        }
        
        // 清理表单
        curl_formfree(formpost);
        
        // 清理curl句柄
        curl_easy_cleanup(curl);
    }
    else
    {
        LOG_Error("curl_easy_init() errror");
    }
    
    // 关闭文件
    fclose(fileFp);
    // 全局清理
    //curl_global_cleanup();
    
    return 0;
}


void *UploadCanFileThread(void *pArg)
{

    char ascFileCmdBuff[256] = {0};


    while(1)
    {
        if(g_uploadCanStartTime == 0)
        {
            sleep(1);
            continue;
        }

        if(0 != access("/dev/mmcblk0", F_OK))
        {
            sleep(30);
            LOG_Debug("can upload not exec");
            continue;
        }

        memset(ascFileCmdBuff, 0, 256);
        snprintf(ascFileCmdBuff, 256,"%s %ld %ld %s %s", ASC_FILE_HANDLE_NAME,\
                g_uploadCanEndTime, g_uploadCanStartTime, CAN_ASC_FILE_PATH, CAN_UPLOAD_FILE_TMP_DIR);

        SrGetSystemRetValue(ascFileCmdBuff);
        LOG_Debug("can upload start");
       
        PostCanFileToZelos(CAN_ASC_POST_ADDR, CAN_UPLOAD_FILE_NAME, g_batchNumber, g_uploadCanStartTime);
        pthread_mutex_lock(&g_uploadCanLock);
        g_uploadCanStartTime = 0;
        g_uploadCanEndTime = 0;
        memset(g_batchNumber, 0, 64);
        pthread_mutex_unlock(&g_uploadCanLock);

        memset(ascFileCmdBuff, 0, 256);
        snprintf(ascFileCmdBuff, 256,"rm -rf %s  %s ", CAN_UPLOAD_FILE_NAME, CAN_UPLOAD_FILE_TMP_DIR);
        SrGetSystemRetValue(ascFileCmdBuff);
        LOG_Debug("can upload end");
    }
}
