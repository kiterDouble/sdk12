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
#include "mqtt_gettoken.h"

using namespace std;

#define TBOX_MQTT_TOKEN_CONFIG_FILE_NAME     "/oemdata/data/mqttToken.txt"
#define TBOX_MQTT_CONFIG_FILE_BACK     "/data/mqttToken.txt"
#define AUTHENTICATION_TEST_ADDRESS     "https://iov-staging.sgmwcloud.com.cn:444/sgmwcarshare/deviceMng/v1/device/login" //测试环境POST
#define CERTIFIED_PRODUCTION_ADDRESS    "https://iov-prod.sgmwcloud.com.cn:443/sgmwcarshare/deviceMng/v1/device/login" //生产环境POST

#define AUTHENTICATION_TEST_HOST_ADDRESS "iov-staging.sgmwcloud.com.cn"
#define AUTHENTICATION_HOST_ADDRESS      "iov-prod.sgmwcloud.com.cn"

#define AUTHENTICATION_TEST_PORT_PATH    "444/sgmwcarshare/deviceMng/v1/device/login"
#define AUTHENTICATION_PORT_PATH         "443/sgmwcarshare/deviceMng/v1/device/login"

#define MQTT_GET_TOKEN_IP_ADDR          "mqtttoken.sgmwcloud.com.cn"
#define MQTT_GET_TOKEN_PORT             (19080)

pthread_mutex_t tokenLock = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
    char buffer[1024];
    size_t size;
}Token;

static Token g_tokenBuf;

static char g_mqttIpAddr[32] = {0};
#define TBOXMQTTHOST			"/oemdata/tmp/mqtt_ip.txt"

gint GetTokenInfo(gchar *buf, gint buffSize)
{
    if(NULL == buf || buffSize < TOKEN_BUFF_MAX_SIZE)
    {
        return RET_ERR;
    }
    pthread_mutex_lock(&tokenLock);
    memcpy(buf, g_tokenBuf.buffer, g_tokenBuf.size);
    pthread_mutex_unlock(&tokenLock);

    return RET_OK;
}

static int getlocalhost(char *localHost)
{
    if (localHost == NULL)
    {
        LOG_Debug("localHost is NULL");
        return RET_ERR;
    }

    char buf[256] = {0};
    FILE *fp = fopen(TBOXMQTTHOST, "r");
    if (fp == NULL)
    {
        LOG_Debug("Cannot open file: %s", TBOXMQTTHOST);
        return RET_ERR;
    }

    if (fgets(buf, sizeof(buf), fp) != NULL)
    {
        LOG_Debug("Read mqtt host from File[%s] - [%s]", TBOXMQTTHOST, buf);
        strncpy(localHost, buf, strlen(buf));
    }
    else
    {
        LOG_Debug("Read mqtt host error: %s", strerror(errno));
        fclose(fp);
        return RET_ERR;
    }

    fclose(fp);
    return RET_OK;
}

static int setlocalhost(const char *localHost)
{
    if (localHost == NULL)
    {
        LOG_Debug("localHost is NULL");
        return RET_ERR;
    }

    FILE *fp = fopen(TBOXMQTTHOST, "w");
    if (fp == NULL)
    {
        LOG_Debug("Cannot open file: %s", TBOXMQTTHOST);
        return RET_ERR;
    }

    if (fputs(localHost, fp) != EOF)
    {
        LOG_Debug("Write mqtt host to File[%s] - [%s]", TBOXMQTTHOST, localHost);
    }
    else
    {
        LOG_Debug("Write mqtt host error: %s", strerror(errno));
        fclose(fp);
        return RET_ERR;
    }

    fclose(fp);
    return RET_OK;
}

gint GetFileFromMqttTokenCheck()
{
    gint fileFd = -1;
    struct stat fileStat;
    gint readSize = -1;
    char tokenBuffer[TOKEN_BUFF_MAX_SIZE] = {0};
    //判断文件是否存在，不存在则从备份文件中拷贝
    if(0 != access(TBOX_MQTT_TOKEN_CONFIG_FILE_NAME, F_OK))
    {
        LOG_Error("%s does not exist", TBOX_MQTT_TOKEN_CONFIG_FILE_NAME);
        if(0 != access(TBOX_MQTT_CONFIG_FILE_BACK, F_OK))
        {
            LOG_Error("%s does not exist", TBOX_MQTT_CONFIG_FILE_BACK);
            return RET_ERR;
        }
        system("cp /data/mqttToken.txt /oemdata/data/mqttToken.txt && sync ");
    }
    //判断文件是否存在，存在则拷贝至备份文件中
    else
    {
        if(0 != access(TBOX_MQTT_CONFIG_FILE_BACK, F_OK))
        {
            system("cp /oemdata/data/mqttToken.txt  /data/mqttToken.txt && sync");
        }
    }

    fileFd = open(TBOX_MQTT_TOKEN_CONFIG_FILE_NAME,  O_RDONLY);
    if(-1 == fileFd)
    {
        LOG_Debug("open %s failed",TBOX_MQTT_TOKEN_CONFIG_FILE_NAME);
        return RET_ERR;
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

    if(fileStat.st_size > TOKEN_BUFF_MAX_SIZE)
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

    // string tmpdata(tokenBuffer, 0, readSize);
    // LOG_Debug("tmpdata :%s", tmpdata.c_str());
    LOG_Debug("tokenBuffer :%s", tokenBuffer);
    // if(!reader.parse(tmpdata, root))
    // {
    //     LOG_Error("parse string error");
    //     close(fileFd);
    //     return RET_ERR;
    // }
    close(fileFd);
    pthread_mutex_lock(&tokenLock);
    memcpy(g_tokenBuf.buffer, tokenBuffer, readSize);
    g_tokenBuf.size = readSize;
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

    retValue = access(TBOX_MQTT_TOKEN_CONFIG_FILE_NAME, F_OK);
    if(0 == retValue)
    {
        fileFd = open(TBOX_MQTT_TOKEN_CONFIG_FILE_NAME, O_RDWR);
    }
    else
    {
        fileFd = open(TBOX_MQTT_TOKEN_CONFIG_FILE_NAME, O_RDWR | O_CREAT);
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
    system("cp /oemdata/data/mqttToken.txt  /data/mqttToken.txt && sync");
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

static gint ConnectTokenServer(gchar *ipAddr, int port)
{
    gint socketFd = -1;
    struct hostent *host = NULL;
	struct sockaddr_in SerAddr;
	struct timeval timeout={3,0};
    guint gethost_ret = 0;

    if(NULL == ipAddr)
    {
        return RET_ERR;
    }
	
    host = gethostbyname(ipAddr);
	if(NULL != host)
	{
        char tmpIpAddr[32] = {0};
        inet_ntop(AF_INET, host->h_addr_list[0], tmpIpAddr, sizeof(tmpIpAddr));

        if (0 != strcmp(g_mqttIpAddr, tmpIpAddr))
        {
            strncpy(g_mqttIpAddr, tmpIpAddr, sizeof(g_mqttIpAddr) - 1);
            LOG_Debug("setlocalhost :%d", setlocalhost(g_mqttIpAddr));
        }

        gethost_ret = 1;
	}
    else if (NULL == host)
    {
        LOG_Debug("Gethostbyname Error:%s", strerror(errno));
        // 获取本地缓存的MQTT地址
        if (getlocalhost(g_mqttIpAddr) != RET_OK)
        {
            LOG_Debug("Cannot get localhost");
            return RET_ERR;  // 获取本地缓存失败，退出程序
        }

		host = static_cast<struct hostent*>(malloc(sizeof(struct hostent)));
    	host->h_addr_list = static_cast<char**>(malloc(sizeof(char*) * 2));
    	host->h_addr_list[0] = static_cast<char*>(malloc(INET_ADDRSTRLEN)); // 分配足够的空间来存储二进制 IP 地址
		if (host->h_addr_list[0] == NULL) {
			printf("Failed to allocate memory for IP address");
			free(host->h_addr_list);
			free(host);
			return RET_ERR;
		}   
        char ipBinaryAddr[32] = {0}; // 用于存储IP地址的二进制形式

        // 将tmpIpAddr赋值给host
        inet_pton(AF_INET, g_mqttIpAddr, ipBinaryAddr);
        memcpy(host->h_addr_list[0], ipBinaryAddr, INET_ADDRSTRLEN); // 复制二进制 IP 地址到 host->h_addr_list[0]

        gethost_ret = 0;
    }

    LOG_Debug("g_mqttIpAddr :%s", g_mqttIpAddr);

	if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		LOG_Debug("Socket Error:%s", strerror(errno));

        if (0 == gethost_ret)
        {
            free(host->h_addr_list[0]); // 释放为二进制 IP 地址分配的内存
            free(host->h_addr_list); // 释放为地址列表分配的内存
            free(host); // 释放为 host 分配的内存
        }
		return RET_ERR;
	}
/*
    //检查IP是否仅被路由到APN1上
    if (checkIPonInterface(APN2_INTERFACE, *((struct in_addr *)host->h_addr)))
    {
		char cmdBuff[1024] = {0};
		snprintf(cmdBuff, 1024,"route del %s %s", inet_ntoa(*((struct in_addr *)host->h_addr)), APN2_INTERFACE);
        int ret = GetSystemRetValue(cmdBuff);
		LOG_Debug("index [%d]cmdBuff exe ret [%d]:%s", index, ret, cmdBuff);
        return E_RES_ROUTE_ERROR;
    }
    else if (checkIPonInterface(APN3_INTERFACE, *((struct in_addr *)host->h_addr)))
    {
		char cmdBuff[1024] = {0};
		snprintf(cmdBuff, 1024,"route del %s %s", inet_ntoa(*((struct in_addr *)host->h_addr)), APN3_INTERFACE);
        int ret = GetSystemRetValue(cmdBuff);
		LOG_Debug("index [%d]cmdBuff exe ret [%d]:%s", index, ret, cmdBuff);
        return E_RES_ROUTE_ERROR;
    }
    else if (!checkIPonInterface(APN1_INTERFACE, *((struct in_addr *)host->h_addr)))
    {
		char cmdBuff[1024] = {0};
		snprintf(cmdBuff, 1024,"route add %s %s", inet_ntoa(*((struct in_addr *)host->h_addr)), APN1_INTERFACE);
        int ret = GetSystemRetValue(cmdBuff);
		LOG_Debug("index [%d]cmdBuff exe ret [%d]:%s", index, ret, cmdBuff);
        return E_RES_ROUTE_ERROR;
    }
*/

	char cmdBuff[1024] = {0};
	snprintf(cmdBuff, 1024,"route add %s %s", inet_ntoa(*((struct in_addr *)host->h_addr)), APN1_INTERFACE);
    int ret = GetSystemRetValue(cmdBuff);
	LOG_Debug("index [0] cmdBuff exe ret [%d]:%s", ret, cmdBuff);

	SerAddr.sin_family = AF_INET;
	SerAddr.sin_port = htons(port);
	SerAddr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(SerAddr.sin_zero), 8);

	setsockopt(socketFd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
    setsockopt(socketFd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));

	if (connect(socketFd, (struct sockaddr *)&SerAddr, sizeof(struct sockaddr)) == -1)
	{
		close(socketFd);
		LOG_Debug("Connect Error:%s close fd ", strerror(errno));
        if (0 == gethost_ret)
        {
            free(host->h_addr_list[0]); // 释放为二进制 IP 地址分配的内存
            free(host->h_addr_list); // 释放为地址列表分配的内存
            free(host); // 释放为 host 分配的内存
        }
		return RET_ERR;
	}

    if (0 == gethost_ret)
    {
        free(host->h_addr_list[0]); // 释放为二进制 IP 地址分配的内存
        free(host->h_addr_list); // 释放为地址列表分配的内存
        free(host); // 释放为 host 分配的内存
    }
    return socketFd;    
}

static gint PackageReqTokenData(gchar *reqData, gint dataLen)
{
    gint retValue = -1;
    gint offset = 0;
    gchar tmpData[128] = {0}; 
    uint8_t vinData[32] = {0};
    gint timeDataLen = 0;
    static gint seqNum = 0;
    char xorData = 0;
    gint count = 0;

    if(NULL == reqData || dataLen < 512)
    {
        return RET_ERR;
    }

    reqData[offset++] = 0x24;
    reqData[offset++] = 0x24;
    reqData[offset++] = 0x01;
    reqData[offset++] = 0x00;

    reqData[offset++] = 0x00;
    reqData[offset++] = 42;

    timeDataLen = GetBJTime(tmpData, 128);
    if(timeDataLen < 0)
    {
        return RET_ERR;
    }

    retValue = NumToBCDCode(tmpData, timeDataLen, &reqData[offset], dataLen - offset);
    if(retValue < 0)
    {
        return RET_ERR;
    }

    offset += timeDataLen;

    seqNum++;
    reqData[offset++] = seqNum / 256;
    reqData[offset++] = seqNum % 256;
    if(seqNum >= 65535)
    {
        seqNum = 0;
    }

    reqData[offset++] = 17;

    if(!GetVin(vinData, sizeof(vinData) - 1))
    {
        return RET_ERR;
    }

    memcpy(&reqData[offset], vinData, 17);
    offset += 17;

    retValue = GetMd5sumValue(&reqData[offset], dataLen - offset, &reqData[6], offset - 6);
    if(retValue < 0)
    {
        return RET_ERR;
    }

    offset += 16;

    xorData = reqData[0];
    for(count = 1; count < offset; count++)
    {
        xorData = xorData ^ reqData[count];
    }

    reqData[offset++] = xorData;

    return offset;
}



static gint SendReqkenDataToServer(gint socketFd)
{
    gchar sendData[1024] = {0};
    gint dataLen = 0;

    if(socketFd < 2)
    {
        LOG_Error("socketFd error : %d", socketFd);
        return RET_ERR;
    }

    dataLen = PackageReqTokenData(sendData, 1024);
    if(dataLen < 0)
    {
        return RET_ERR;
    }
    LOG_DebugHex(sendData, dataLen, "ReqTokenData:");
    return send(socketFd, sendData, dataLen, MSG_NOSIGNAL);
}

static gint ParseTokenData(gchar *tokenData, gint dataLen, gchar *recvData, gint recvLen)
{
    gint tokenLen = 0;

    if(NULL == recvData)
    {
        LOG_Error("recvData is NULL or recvLen <= 0: %d", recvLen);
        return RET_ERR;
    }

    if(NULL == tokenData)
    {
        LOG_Error("recvData is NULL or recvLen <= 0: %d", recvLen);
        return RET_ERR;
    }

    LOG_Debug("recvData[6] : %d", recvData[6]);
    switch (recvData[6])
    {
    case 0:
        break;
    case 1:
        return E_RES_SIGNATURE_ERROR;
        break;
    case 2:
        return E_RES_VIN_ERROR;
        break;    
    default:
        return RET_ERR;
        break;
    }

    tokenLen = recvData[9] * 256 + recvData[10];
    if(dataLen < tokenLen)
    {
        LOG_Error("tokenLen is error : %d", tokenLen);
        return RET_ERR;
    }

    memcpy(tokenData, &recvData[11], tokenLen);
    return tokenLen;
}
static gint RecvTokenDataFromServer(gint socketFd, gchar *tokenData, gint dataLen)
{
    gint retValue = -1;
    gchar recvDataBuff[1024] = {0};
    gint recvLen = 0;

    if(NULL == tokenData || dataLen <= 0)
    {
        LOG_Error("sendData is NULL or dataLen <= 0: %d", dataLen);
        return RET_ERR;
    }

    if(socketFd < 2)
    {
        LOG_Error("socketFd error : %d", socketFd);
        return RET_ERR;
    }

    recvLen =  recv(socketFd, recvDataBuff, 1024, MSG_NOSIGNAL);
    if(recvLen <= 0)
    {
        LOG_Error("recv is error : %d", recvLen);
        return RET_ERR;
    }

    LOG_DebugHex(recvDataBuff, recvLen, "RecvTokenData:");
    return ParseTokenData(tokenData, dataLen, recvDataBuff, recvLen);
}


static gint  RequestToken(gchar *ipAddr, int port)
{
    gint socketFd = -1;
    gint retValue = 0;


    if(NULL == ipAddr)
    {
        LOG_Debug("RequestToken upAddr is NULL");
        return RET_ERR;
    }

    socketFd = ConnectTokenServer(ipAddr, port);
    if(socketFd < 0)
    {
        LOG_Error("ConnectTokenServer  error ");
        return RET_ERR;
    }
    LOG_Debug("ConnectTokenServer success");

    retValue = SendReqkenDataToServer(socketFd);
    if(retValue < 0)
    {
        close(socketFd);
        LOG_Error("SendReqkenDataToServer  error ");
        return RET_ERR;
    }
    LOG_Debug("SendReqkenDataToServer success");

    retValue = RecvTokenDataFromServer(socketFd, g_tokenBuf.buffer, 1024);
    if(retValue < 0)
    {
        close(socketFd);
        LOG_Error("RecvTokenDataFromServer  error : %d",retValue);
        return retValue;
    }

    g_tokenBuf.size = retValue;

    // 解析后的数据保存到文件
    SaveRequestInfoToFile(g_tokenBuf.buffer,g_tokenBuf.size);
    close(socketFd);
    return RET_OK;
}



gint TaskRequestToken(void)
{
    return RequestToken((gchar *)MQTT_GET_TOKEN_IP_ADDR, MQTT_GET_TOKEN_PORT);   
}
