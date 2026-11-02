#include <curl/curl.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include "json/inc/json.h"
#include "tsp_gettoken.h"
#include "dns.h"

using namespace std;

#define TBOX_TOKEN_CONFIG_FILE_NAME     "/oemdata/data/token.txt"
#define TBOX_TOKEN_CONFIG_FILE_BACK     "/data/token.txt"
#define AUTHENTICATION_TEST_ADDRESS     "https://iov-staging.sgmwcloud.com.cn:444/sgmwcarshare/deviceMng/v1/device/login" //测试环境POST
#define CERTIFIED_PRODUCTION_ADDRESS    "https://iov-prod.sgmwcloud.com.cn:443/sgmwcarshare/deviceMng/v1/device/login" //生产环境POST

#define AUTHENTICATION_TEST_HOST_ADDRESS "iov-staging.sgmwcloud.com.cn"
#define AUTHENTICATION_HOST_ADDRESS      "iov-prod.sgmwcloud.com.cn"

#define AUTHENTICATION_TEST_PORT_PATH    "444/sgmwcarshare/deviceMng/v1/device/login"
#define AUTHENTICATION_PORT_PATH         "443/sgmwcarshare/deviceMng/v1/device/login"

pthread_mutex_t tokenLock = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
    char buffer[MAX_BUFFER_SIZE];
    size_t size;
}Token;

static Token tokenBuf;

void GetTokenInfo(gchar *buf)
{
    pthread_mutex_lock(&tokenLock);
    memcpy(buf,tokenBuf.buffer,tokenBuf.size);
    pthread_mutex_unlock(&tokenLock);
}

gint GetHostIpAddr(const char *domainName, gint domainNameLen,
                   char       *ipAddrBuff, gint ipAddrBuffLen)
{
    return GetHostIpAddrFromDNSServer(domainName, domainNameLen,
                                      "ccinet1", 12,
                                      ipAddrBuff, ipAddrBuffLen);
    // gint retValue = -1;
    // const char *dnsAddr = "218.4.4.4";
	// const char *interface = "rmnet_data0";
	// uint8_t ipV4Addr[4] = {0};

	// retValue = gb_dns_domain_2_ipv4(dnsAddr, 53, interface,
    //                                 domainName, ipV4Addr, 200);

    // if(retValue < 0)
    // {
    //     return RET_ERR;
    // }

    // snprintf(ipAddrBuff, ipAddrBuffLen,"%d.%d.%d.%d\n", 
    //         ipV4Addr[0], ipV4Addr[1], ipV4Addr[2], ipV4Addr[3]);
	
	// return RET_OK;
    // gint ipCount = 0;
    // gint hostCount = 0;
    // char **stIpAddr = NULL;
    // struct hostent *host = NULL;
    // if(NULL == domainName || NULL == ipAddrBuff)
    // {
    //     return RET_ERR;
    // }

    // if(ipAddrBuffLen < 24)
    // {
    //     return RET_ERR;
    // }

    // host = gethostbyname(domainName);
    // if(NULL == host)
    // {
    //     return RET_ERR;
    // }

    // for(stIpAddr = host->h_addr_list; NULL != *stIpAddr ; stIpAddr++)
    // {
    //     hostCount++;
    // }

    // srand((int)time(NULL));
    // ipCount = rand() % hostCount;
    // inet_ntop(host->h_addrtype, host->h_addr_list[ipCount],
    //                             ipAddrBuff, ipAddrBuffLen);
    // ipCount++;

    // return RET_OK;
}


gint GetFileFromTokenCheck()
{
    gint fileFd = -1;
    struct stat fileStat;
    gint readSize = -1;
    char tokenBuffer[MAX_BUFFER_SIZE] = {0};
    Json::Reader reader;
    Json::Value root;
    //判断文件是否存在，不存在则从备份文件中拷贝
    if(0 != access(TBOX_TOKEN_CONFIG_FILE_NAME, F_OK))
    {
        LOG_Error("%s does not exist", TBOX_TOKEN_CONFIG_FILE_NAME);
        if(0 != access(TBOX_TOKEN_CONFIG_FILE_BACK, F_OK))
        {
            LOG_Error("%s does not exist", TBOX_TOKEN_CONFIG_FILE_BACK);
            return RET_ERR;
        }
        system("cp /data/token.txt /oemdata/data/token.txt && sync ");
    }
    //判断文件是否存在，存在则拷贝至备份文件中
    else
    {
        if(0 != access(TBOX_TOKEN_CONFIG_FILE_BACK, F_OK))
        {
            system("cp /oemdata/data/token.txt  /data/token.txt && sync");
        }
    }

    fileFd = open(TBOX_TOKEN_CONFIG_FILE_NAME,  O_RDONLY);
    if(-1 == fileFd)
    {
        LOG_Debug("open %s failed",TBOX_TOKEN_CONFIG_FILE_NAME);
        return -1;
    }

    if(0 != fstat(fileFd, &fileStat))
    {
        LOG_Error("get file stat Error");
        close(fileFd);
        return RET_ERR;
    }

    if(!S_ISREG(fileStat.st_mode))
    {
        LOG_Error("get file not a file");
        close(fileFd);
        return RET_ERR;
    }

    if(fileStat.st_size > MAX_BUFFER_SIZE)
    {
        LOG_Error("get file size > MAX_BUFFER_SIZE");
        close(fileFd);
        return RET_ERR;
    }

    readSize = read(fileFd, tokenBuffer, sizeof(tokenBuffer));
    if(readSize < 0)
    {
        LOG_Error("read file error");
        close(fileFd);
        return RET_ERR;
    }

    string tmpdata(tokenBuffer, 0, readSize);
    LOG_Debug("tmpdata :%s", tmpdata.c_str());
    LOG_Debug("tokenBuffer :%s", tokenBuffer);
    if(!reader.parse(tmpdata, root))
    {
        LOG_Error("parse string error");
        close(fileFd);
        return RET_ERR;
    }
    close(fileFd);
    pthread_mutex_lock(&tokenLock);
    memcpy(tokenBuf.buffer, tokenBuffer, readSize);
    tokenBuf.size = readSize;
    pthread_mutex_unlock(&tokenLock);
    return RET_OK;
}


static gint SaveRequestInfoToFile(char *buff, gint buffSize)
{
    gint retValue = 0;
    gint fileFd = -1;
    gint writeLen = 0;

    if(NULL == buff || buffSize < 0)
    {
        LOG_Error("buff is NULL or buffSize < 0");
        return RET_ERR;
    }

    retValue = access(TBOX_TOKEN_CONFIG_FILE_NAME, F_OK);
    if(0 == retValue)
    {
        fileFd = open(TBOX_TOKEN_CONFIG_FILE_NAME, O_RDWR);
    }
    else
    {
        fileFd = open(TBOX_TOKEN_CONFIG_FILE_NAME, O_RDWR | O_CREAT);
    }

    if(fileFd < 0)
    {
        LOG_Error("open file error");
        return RET_ERR;
    }

    if(0 != ftruncate(fileFd, 0))
    {
        LOG_Error("Empty file error");
        return RET_ERR;
    }

    lseek(fileFd, 0, SEEK_SET);

    while(writeLen < buffSize)
    {
        retValue = write(fileFd, &buff[writeLen], buffSize - writeLen);
        if(retValue <= 0)
        {
            break;
        }
        writeLen += retValue;
    }
    close(fileFd);

    system("cp /oemdata/data/token.txt  /data/token.txt && sync");

    return RET_OK;
}

static gint GetDomainNameFromUrl(const char *url, gint urlLen, char *domainName, gint domainNameLen)
{
    gint count = 0;
    gint offset = 0;

    if(NULL == url || NULL == domainName)
    {
        return RET_ERR;
    }

    for(count = 0; count < urlLen; count++)
    {
        if(url[count] == '/' && url[count + 1] == '/')
        {
            break;
        }
    }

    for(count += 2; count < urlLen; count++)
    {
        if(offset >= domainNameLen)
        {
            break;
        }

        if(url[count] == ':')
        {
            break;
        }
        domainName[offset++] = url[count];
    }

    if(count >= urlLen || offset >= domainNameLen)
    {
        return RET_ERR;
    }

    return RET_OK;
}

static gint  SetRequestTokenURL(OUT gchar url[], IN guint len)
{
    // sprintf(url, "%s", AUTHENTICATION_TEST_ADDRESS);
    //char ipAddrBuff[32] = {0};
    uint8_t urlBuff[128] = {0}; 
    char domainNameBuff[128] = {0}; 
    gint retValue = 0;
    //从参数中获取url，获取上传次数的类型，判断是int还是string，再获取其值
    if (!GetTspTokenUrlUrl(urlBuff, sizeof(urlBuff) - 1))
    {
        return RET_ERR;
    }

    // retValue = GetDomainNameFromUrl(urlBuff, 128, domainNameBuff, 128);
    // if(retValue < RET_OK)
    // {
    //     return RET_ERR;
    // }

    // retValue = GetHostIpAddr(domainNameBuff, 
    //                          strlen(domainNameBuff),
    //                          ipAddrBuff,
    //                          32);
    // if(retValue != RET_OK)
    // {
    //     return RET_ERR;
    // }

    retValue = GetDomainNameFromUrl((char *)urlBuff, 128, domainNameBuff, 128);
    if(retValue < RET_OK)
    {
        return RET_ERR;
    }
    struct hostent *host = NULL;
    host = gethostbyname(domainNameBuff);
	if(NULL != host)
	{
        char tmpIpAddr[32] = {0};
        char cmdBuff[1024] = {0};
        inet_ntop(AF_INET, host->h_addr_list[0], tmpIpAddr, sizeof(tmpIpAddr));
        memset(cmdBuff, 0, 1024);
        snprintf(cmdBuff, 1024,"route add %s %s", tmpIpAddr, APN1_INTERFACE);
        int retValue = GetSystemRetValue(cmdBuff);
        LOG_Debug("cmdBuff exe ret [%d]:%s", retValue, cmdBuff);        
	}

    retValue = snprintf(url, len ,"%s", urlBuff);
    return RET_OK;
}

gint ParserInfo(gchar *tspInfo)
{
    if(NULL == tspInfo)
    {
        LOG_Error("tspInfo is NULL");
        return RET_ERR;
    }
    Json::Reader reader;
    Json::Value root;

    if(reader.parse(tspInfo,root))
    {
        LOG_Debug("root[\"msgCode\"]:%s",root["msgCode"].asString().c_str());
        LOG_Debug("root[\"msg\"]:%s",root["msg"].asString().c_str());
        LOG_Debug("root[\"content\"][\"deviceId\"]:%s",root["content"]["deviceId"].asString().c_str());
        LOG_Debug("root[\"content\"][\"deviceSecret\"]:%s",root["content"]["deviceSecret"].asString().c_str());
        LOG_Debug("root[\"content\"][\"clientId\"]:%s",root["content"]["clientId"].asString().c_str());
        LOG_Debug("root[\"content\"][\"host\"]:%s",root["content"]["host"].asString().c_str());
        LOG_Debug("root[\"content\"][\"port\"]:%s",root["content"]["port"].asString().c_str());
        LOG_Debug("root[\"status\"]:%d",root["status"].asInt());
        LOG_Debug("root[\"total\"]:%d",root["total"].asInt());

        if(strcmp(root["msgCode"].asString().c_str(), "10000") && strcmp(root["msg"].asString().c_str(), "OK"))
        {
            LOG_Debug("root[\"msgCode\"] or root[\"msg\"] is error");
            return RET_ERR;
        }

    }
    else
    {
        return RET_ERR;
    }

    return RET_OK;
}


static size_t StoreRequestInfo(char *ptr ,size_t size , size_t nmemb, void * userData)
{
	char cmd[512] = {0};
    if(nmemb > MAX_BUFFER_SIZE)
    {
        LOG_Error("StoreRequestInfo size too long [%d]",nmemb);
        return nmemb;
    }
    
    char buffer[MAX_BUFFER_SIZE] = {0};
    memcpy(buffer , ptr, nmemb);
    if(RET_OK != ParserInfo(buffer))
    {
        LOG_Debug("buffer:%s",buffer);
        LOG_Error("ParserInfo  Error");
        return 0;
    }
    else
    {
        LOG_Debug("ParserInfo  succes");
    }

    pthread_mutex_lock(&tokenLock);
    memset(tokenBuf.buffer , 0, MAX_BUFFER_SIZE);
    memcpy(tokenBuf.buffer , ptr, nmemb);
    tokenBuf.size = nmemb;
    pthread_mutex_unlock(&tokenLock);
    LOG_Debug("buffer:%s",tokenBuf.buffer);
    LOG_Debug("SaveRequestInfoToFile: %d", 
               SaveRequestInfoToFile(tokenBuf.buffer, tokenBuf.size));
    return nmemb;
}

gint PostFotaMsg(char type, guchar *msgBuff, int msgLen)
{
    CURL *curl;
    CURLcode res;

    struct curl_slist *headers = NULL;
    int retValue = 0;
    char jsonBuff[2048] = {0};
    char msgBuff1[2048] = {0};


    if(NULL == msgBuff)
    {
        return RET_ERR;
    }

    HexToStr((guchar *)msgBuff1,(guchar *)msgBuff,msgLen);

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl == NULL)
    {
        LOG_Error("curl init error");
        curl_global_cleanup();
        return RET_ERR;
    }
   
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:20003/tspmessage?dev=tbox");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);


    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    snprintf(jsonBuff, 2048, "{\"subfun\":\"%d\",\"message\":\"%s\"}", type, msgBuff1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBuff);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0 );
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0 );
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

    /*perform crul command*/
    res = curl_easy_perform( curl );
    if (res != CURLE_OK)
    {
        LOG_Error("perform curl error = %s",curl_easy_strerror(res));
    }
    else
    {
        LOG_Debug("perform curl success");
    }
    

    /*clean up curl*/
    curl_slist_free_all(headers);
    headers = NULL;
    curl_easy_cleanup( curl );
    curl = NULL;
    curl_global_cleanup();
    return ((res != CURLE_OK) ? RET_ERR: RET_OK );
}


static gint RequestToken(IN gchar url[])
{
    CURL *curl;
    CURLcode res;
    
    guchar tboxICCID[24] = {0};
    guchar token[45] = {0};
    guchar mdResult[SHA256_DIGEST_LENGTH] = {0x00};
    guint fileLen = 32;
    guchar shabuf[65] = {0x00};
    guchar headers[128] = {0x00};
    struct curl_slist *head = NULL;
    int retValue = 0;
    
    LOG_Debug("url: %s", url);
    //从参数中获取iccid
    GetIccid(tboxICCID, sizeof(tboxICCID) - 1);
    //memcpy(tboxICCID,"898602C9981740037923",20);
    sprintf((char *)token,"%s%s",(char *)tboxICCID,(char *)tboxICCID);
    LOG_Debug("token: %s", token);
    //用哈希算法生成消息摘要
    SHA256(token, fileLen, mdResult);
    //16进制数组转字符串
    retValue = HexToStr2(shabuf, 64, mdResult,SHA256_DIGEST_LENGTH);
    if(retValue < 0)
    {
        LOG_Error("HexToStr2 error");
        return RET_ERR;
    }
    LOG_Debug("shabuf: %s", shabuf);
    memset(headers,0x00,128);
    //设置请求头
    snprintf((char *)headers, 128,"token:%s",(char *)shabuf);
    LOG_Debug("headers: %s", headers);

    //curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl == NULL)
    {
        LOG_Error("curl init error");
        return RET_ERR;
    }
    //https协议中的post请求
    /*set post request*/
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    /*set url of curl*/
    curl_easy_setopt(curl, CURLOPT_URL, url );
    /*set callback for process return of https*/
    head = curl_slist_append(head, (char *)headers);
    //存储请求信息
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &StoreRequestInfo );
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head);
    /*set no ssl verify*/
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0 );
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0 );
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

    /*perform crul command*/
    res = curl_easy_perform( curl );
    if (res != CURLE_OK)
    { 
        LOG_Error("perform curl error = %s",curl_easy_strerror(res));
    }
    else
    {
        LOG_Debug("perform curl success");
    }
    
    
    /*clean up curl*/
    curl_slist_free_all(head);
    head = NULL;
    curl_easy_cleanup( curl );
    curl = NULL;
    curl_global_cleanup();
    return ((res != CURLE_OK) ? RET_ERR: RET_OK );
}



gint TaskRequestToken(void)
{
    gchar url[1024] = {
        0x0,
    };
   //设置需要请求的url
    if ( SetRequestTokenURL( url, sizeof(url) ) != RET_OK )
    {
        LOG_Error("Get Request URL Error,URL:[%s]", url );
        goto err_RequestToken;
    }
    //请求token，并将返回的信息存储到文件中
    if ( RequestToken( url ) != RET_OK )
    {
        LOG_Error("Request Http post Error");
        goto err_RequestToken;
    }
    else
    {
        LOG_Debug("Http Response post sucessful");
        return RET_OK;
    }
err_RequestToken:
    return RET_ERR;
}
