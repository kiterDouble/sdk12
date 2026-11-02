#include "mqtt_common.h"
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <curl/curl.h>
#include <json/json.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// curl 写入回调函数
static size_t CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    char *response_buffer = (char *)userp;
    
    // 检查缓冲区是否为 NULL（表示未初始化）
    if (response_buffer == NULL) {
        return 0;
    }
    
    // 复制数据到缓冲区（假设缓冲区足够大）
    memcpy(response_buffer, contents, realsize);
    response_buffer[realsize] = '\0';
    
    return realsize;
}

/**pm state*/
static volatile guint8 g_pmState = SGMW_PM_STATE_INIT;
static uint8_t g_WakeupSrc = PM_WAKEUP_SOURCE_INVALID;
static uint8_t g_RtcFlag = 0; //1:Rtc 唤醒标志 0:非 Rtc 唤醒标志
static uint8_t g_OneMqttFlag = 0; //1:1 次 MQTT 发送标志 0:默认
static time_t g_lastHeartbeatTime = 0; // 上次心跳唤醒时间
static uint8_t g_RtcSleepEventSet = 0; // 1:RTC 休眠事件已设置，0:未设置

static pthread_rwlock_t g_MQTTPmStateMutex = PTHREAD_RWLOCK_INITIALIZER;
static pthread_mutex_t g_PmEventMutex = PTHREAD_MUTEX_INITIALIZER; // 保护 PM 事件操作的互斥锁

// 全局升级状态缓存
static int g_slaveModemUpgradeStatus = MODEM_UPGRADE_STATUS_INVALID;
static pthread_mutex_t g_slaveModemUpgradeStatusMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief 获取缓存的副模组升级状态
 * @retval 升级状态值
 */
int GetCachedSlaveModemUpgradeStatus(void)
{
    int status = MODEM_UPGRADE_STATUS_INVALID;
    
    pthread_mutex_lock(&g_slaveModemUpgradeStatusMutex);
    status = g_slaveModemUpgradeStatus;
    pthread_mutex_unlock(&g_slaveModemUpgradeStatusMutex);
    
    return status;
}

/**
 * @brief 更新缓存的副模组升级状态
 * @param[in] status 新的升级状态
 */
void UpdateCachedSlaveModemUpgradeStatus(int status)
{
    pthread_mutex_lock(&g_slaveModemUpgradeStatusMutex);
    
    if (status != g_slaveModemUpgradeStatus) {
        g_slaveModemUpgradeStatus = status;
        LOG_Debug("Slave modem upgrade status updated to: %d", status);
    }
    
    pthread_mutex_unlock(&g_slaveModemUpgradeStatusMutex);
}

/**
 * @brief 周期性获取副模组升级状态线程
 * @param[in] arg 线程参数（未使用）
 */
void* curlGetUpgradeStatusThread(void *arg)
{
    int current_status = 0;
    
    LOG_Debug("Slave modem upgrade status monitoring thread started");
    
    while (1) {
        // 调用 curl 接口获取升级状态
        current_status = ReadSlaveModemUpgradeStatusFromUrl();
        
        if (current_status != 0) {
            // 更新全局缓存
            UpdateCachedSlaveModemUpgradeStatus(current_status);
            LOG_Debug("Slave modem upgrade status: %d", current_status);
        } else {
            //UpdateCachedSlaveModemUpgradeStatus(current_status);
            LOG_Error("Failed to get slave modem upgrade status");
        }
        
        // 周期 2 秒
        sleep(1);
    }
    
    return NULL;
}

unsigned char *StrToHex(unsigned char *pbDest, const unsigned char *pbSrc,int nLen) 
{
    char h1, h2;
    char s1, s2;
    int i;

    for (i = 0; i < nLen; i++)
    {
        h1 = pbSrc[2 * i];
        h2 = pbSrc[2 * i + 1];

        s1 = toupper(h1) - 0x30;
        if (s1 > 9)
        {
            s1 -= 7;
        }

        s2 = toupper(h2) - 0x30;
        if (s2 > 9)
        {
            s2 -= 7;
        }
        pbDest[i] = s1 * 16 + s2;
    }
    return pbDest;
}


unsigned char *HexToStr(unsigned char *pbDest, const unsigned char *pbSrc,int nLen) 
{
    char ddl, ddh;
    int i;

    for (i = 0; i < nLen; i++)
    {
        ddh = 48 + pbSrc[i] / 16;
        ddl = 48 + pbSrc[i] % 16;
        if (ddh > 57)
        {
            ddh = ddh + 39;
        }
        if (ddl > 57)
        {
            ddl = ddl + 39;
        }
        pbDest[i * 2] = ddh;
        pbDest[i * 2 + 1] = ddl;
    }
    pbDest[nLen * 2] = '\0';
    return pbDest;
}



int HexToStr2(unsigned char *pbDest, int destLen, const unsigned char *pbSrc,int nLen) 
{
    char ddl, ddh;
    int i;

    if((nLen * 2 ) < destLen)
    {
        return RET_ERR;
    }

    for (i = 0; i < nLen; i++)
    {
        ddh = 48 + pbSrc[i] / 16;
        ddl = 48 + pbSrc[i] % 16;
        if (ddh > 57)
        {
            ddh = ddh + 7;
        }
        if (ddl > 57)
        {
            ddl = ddl + 7;
        }
        
        pbDest[i * 2] = ddh;
        pbDest[i * 2 + 1] = ddl;
    }
    
    return RET_OK;
}

int GetFotaTmpFileContentAndEmptyFile(const char *filePath, char *content, int contentLen)
{
	int fileFd = -1;
	int readSize = -1;
	struct stat fileStat;
	
	if(NULL == content)
	{
		return RET_ERR;
	}
	
	if(0 != access(filePath, F_OK))
    {
        LOG_Error("%s does not exist", filePath);
        return RET_ERR;
    }

    fileFd = open(filePath,  O_RDONLY);
    if(-1 == fileFd)
    {
        LOG_Debug("open %s failed\n",filePath);
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

    if(fileStat.st_size > contentLen)
    {
        LOG_Error("get file size > %d", contentLen);
		close(fileFd);
        return RET_ERR;
    }

    readSize = read(fileFd, content, contentLen);
    if(readSize < 0)
    {
        LOG_Error("read file error");
		close(fileFd);
        return RET_ERR;
    }
	LOG_Error("read file success");
	close(fileFd);

	if(truncate(filePath, 0) < 0)
    {
        LOG_Error("Empty file error");
		close(fileFd);
        return RET_ERR;
    }

	return readSize;
}


int GetFileContent(const char *filePathName,char *content, int contentLen)
{
	int fileFd = -1;
	int readSize = -1;
	struct stat fileStat;
	
	if(NULL == content || NULL == filePathName)
	{
		return RET_ERR;
	}
	
	if(0 != access(filePathName, F_OK))
    {
        LOG_Error("%s does not exist", filePathName);
        return RET_ERR;
    }

    fileFd = open(filePathName,  O_RDONLY);
    if(fileFd < 0)
    {
        LOG_Debug("open %s failed",filePathName);
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

    if(fileStat.st_size > contentLen)
    {
        LOG_Error("get file size > %d", contentLen);
		close(fileFd);
        return RET_ERR;
    }

    readSize = read(fileFd, content, contentLen);
    if(readSize < 0)
    {
        LOG_Error("read file error");
		close(fileFd);
        return RET_ERR;
    }
	LOG_Debug("file content: %s", content);
	close(fileFd);
	
	return readSize;
}


int GetSystemRetValue(const char *cmd)
{
	pid_t status;
	
	if(NULL == cmd)
	{
		return RET_ERR;
	}
	LOG_Debug("%s exe start", cmd);
	status = system(cmd);
	LOG_Debug("%s exe end:%d", cmd, status);
	if(-1 == status)
	{
		return RET_ERR;
	}
	
	if(!WIFEXITED(status))
	{
		return RET_ERR;
	}
	
	return WEXITSTATUS(status);
}

gint NumToBCDCode(char *numData, int numDataLen, char *bcdData, int bcdDataLen)
{
    gint retValue = -1;
    gint count = 0;

    if(NULL == numData || NULL == bcdData)
    {
        return RET_ERR;
    }

    if(numDataLen <= 0)
    {
        return RET_ERR;
    }

    if(numDataLen > bcdDataLen)
    {
        return RET_ERR;
    }

    for(count = 0; count < numDataLen; count++)
    {
        bcdData[count] = (numData[count] / 10) << 4;
        bcdData[count] |= (numData[count] % 10);
    }

    return RET_OK;
}

int GetMd5sumValue(char *md5SumValue, int md5SumLen, char *md5Data, int md5DataLen)
{
    MD5_CTX ctx;
    
    if(NULL == md5SumValue || NULL == md5Data)
    {
        return RET_ERR;
    }

    if(md5SumLen < 33)
    {
        return RET_ERR;
    }

    MD5_Init(&ctx);
    MD5_Update(&ctx, md5Data, md5DataLen);
    MD5_Final((unsigned char *)md5SumValue, &ctx);

    return RET_OK;
}

int GetBJTime(char *data, int dataLen)
{
    time_t stTmpTime;
	struct tm *pstRetTime;
    int offset = 0;

	if(NULL == data || dataLen < 6)
	{
		return RET_ERR;
	}

	time(&stTmpTime);
	stTmpTime += 28800;
	pstRetTime = gmtime(&stTmpTime);
    if(NULL == pstRetTime)
    {
        return RET_ERR;
    }

	data[offset++] = ((pstRetTime->tm_year + 1900) % 100);
	data[offset++] = pstRetTime->tm_mon + 1;
	data[offset++] = pstRetTime->tm_mday;
	data[offset++] = pstRetTime->tm_hour;
	data[offset++] = pstRetTime->tm_min;
	data[offset++] = pstRetTime->tm_sec;

	return offset;
}

/**
 * @brief Get the Pm State object
 * 
 * @retval	RET_ERR     Error
 *          SGMW_PM_STATE_INIT     INIT
 *          SGMW_PM_STATE_SLEEP    Sleep
 *          SGMW_PM_STATE_NORMAL   normal
 */
uint8_t GetPmState()
{
    LOG_Debug("%s, Get g_pmState:%d", __FUNCTION__, g_pmState);
    pthread_rwlock_rdlock(&g_MQTTPmStateMutex);
    uint8_t pmState = g_pmState;
	pthread_rwlock_unlock(&g_MQTTPmStateMutex);
    return pmState;
}

/**
 * @brief Set the Power State object
 * 
 * @param[in]	pmState	
 */
void SetPmState(uint8_t pmState)
{
    pthread_rwlock_wrlock(&g_MQTTPmStateMutex);
    g_pmState = pmState;
    LOG_Debug("%s, Set g_pmState:%d", __FUNCTION__, g_pmState);
	pthread_rwlock_unlock(&g_MQTTPmStateMutex);
}

//主模组获取升级状态
int ReadMainModemUpgradeStatus(void)
{
    const char *file_path = "/oemdata/parameters/MainModemUpgradeStatus";
    char cmd[512] = {0};
    char buffer[32] = {0};
    FILE *fp = NULL;
    
    // 使用 cat 命令读取文件
    snprintf(cmd, sizeof(cmd), "cat %s 2>/dev/null", file_path);
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        LOG_Debug("ReadMainModemUpgradeStatus: Failed to execute command, returning default status 0");
        return 0;
    }
    
    // 读取输出内容
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // 去除换行符和空格
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r' || buffer[len - 1] == ' ')) {
            buffer[--len] = '\0';
        }
        LOG_Debug("ReadMainModemUpgradeStatus: Read status %s ", buffer);
        // 使用 atoi 转换
        int status = atoi(buffer);
        pclose(fp);
        if(status == -1) status = 3;
        // 验证读取的值是否在有效范围内 (0-4)
        if (status >= 0 && status <= 4) {
            LOG_Debug("ReadMainModemUpgradeStatus: Successfully read valid status %d from %s", status, file_path);
        } else {
            LOG_Debug("ReadMainModemUpgradeStatus: Invalid status %d (not in 0-4 range), returning default status 0", status);
        }
        if(status == 1 ) //升级中
        {
            return MODEM_UPGRADE_STATUS_UPGRADING;
        }
        else if(status == 4 ) //升级请求
        {
            return MODEM_UPGRADE_STATUS_REQUESTED;
        }else if(status == -1)
        {
            return MODEM_UPGRADE_STATUS_INVALID;  // 升级失败
        }else //升级完成
		{		
            return MODEM_UPGRADE_STATUS_COMPLETED;
        }
    }
    
    pclose(fp);
    LOG_Debug("ReadMainModemUpgradeStatus: File not found or empty, returning default status 0");
    return 0;
}

//副模组通过 CURL 接口获取当前升级状态
int ReadSlaveModemUpgradeStatusFromUrl(void)
{
    CURL *curl = NULL;
    CURLcode res;
    char response_data[4096] = {0};  // 使用固定大小数组，最大 4KB 响应
    long response_code = 0;
    int ret_status = 0;
    Json::Value json_rootvalue;
    Json::Reader json_reader;
    int status_value = -1;
    struct curl_slist *headers = NULL;
    
    // 初始化 curl
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl) {
        LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: Failed to initialize curl");
        return 0;
    }
    
    // 设置请求 URL
    res = curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.201:20003/srUpgradeStatus");
    if (res != CURLE_OK) {
        LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: curl_easy_setopt CURLOPT_URL failed: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 0;
    }
    
    // 设置为 GET 请求
    res = curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    if (res != CURLE_OK) {
        LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: curl_easy_setopt CURLOPT_HTTPGET failed: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 0;
    }
    
    // 设置超时时间
    res = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    if (res != CURLE_OK) {
        LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: curl_easy_setopt CURLOPT_TIMEOUT failed: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 0;
    }
    
    res = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3L);
    if (res != CURLE_OK) {
        LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: curl_easy_setopt CURLOPT_CONNECTTIMEOUT failed: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 0;
    }
    
    // 设置 NOSIGNAL 防止信号干扰
    res = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    if (res != CURLE_OK) {
        LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: curl_easy_setopt CURLOPT_NOSIGNAL failed: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 0;
    }
    
    // 设置响应数据接收回调，传入数组指针
    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    if (res != CURLE_OK) {
        LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: curl_easy_setopt CURLOPT_WRITEFUNCTION failed: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 0;
    }
    
    // 设置写入数据指针为数组地址
    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response_data);
    if (res != CURLE_OK) {
        LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: curl_easy_setopt CURLOPT_WRITEDATA failed: %s", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 0;
    }
    
    // 设置请求头
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (headers) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
    
    // 执行请求
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: curl_easy_perform failed: %s", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 0;
    }
    
    // 获取响应码
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (res != CURLE_OK) {
        LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: curl_easy_getinfo failed: %s", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 0;
    }
    
    if (response_code != 200) {
        LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: HTTP response code: %ld", response_code);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 0;
    }
    
    LOG_Debug("ReadSlaveModemUpgradeStatusFromUrl: HTTP request succeeded, response code: %ld", response_code);
    
    // 检查响应数据
    if (response_data[0] == '\0') {
        LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: Response data is empty");
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 0;
    }
    
    LOG_Debug("ReadSlaveModemUpgradeStatusFromUrl: Response data: %s", response_data);
    
    // 使用 JsonCpp 解析 JSON 响应 {"UpgradeStatus":xxxx}
    std::string responseStr(response_data);
    if (json_reader.parse(responseStr, json_rootvalue)) {
        if (json_rootvalue["UpgradeStatus"].isNull()) {
            LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: UpgradeStatus field is NULL or not found");
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return 0;
        }
        
        // 获取 UpgradeStatus 字段的整数值
        status_value = json_rootvalue["UpgradeStatus"].asInt();
        LOG_Debug("ReadSlaveModemUpgradeStatusFromUrl: UpgradeStatus = %d", status_value);
        
        // 验证状态值范围 (0-4)
        if (status_value < 0 || status_value > 4) {
            LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: Invalid status value %d (not in 0-4 range)", status_value);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return 0;
        }
        
        // 根据状态值返回对应的宏定义
        if (status_value == 1) {
            ret_status = MODEM_UPGRADE_STATUS_UPGRADING;  // 升级中
            LOG_Debug("ReadSlaveModemUpgradeStatusFromUrl: Status is UPGRADING");
        } else if (status_value == 4) {
            ret_status = MODEM_UPGRADE_STATUS_REQUESTED;  // 升级请求
            LOG_Debug("ReadSlaveModemUpgradeStatusFromUrl: Status is REQUESTED");
        }else if(status_value == 3){
            ret_status = MODEM_UPGRADE_STATUS_FAILED;  // 升级失败
            LOG_Debug("ReadSlaveModemUpgradeStatusFromUrl: Status is FAILED");
        } else {
            ret_status = MODEM_UPGRADE_STATUS_COMPLETED;  // 升级完成
            LOG_Debug("ReadSlaveModemUpgradeStatusFromUrl: Status is COMPLETED");
        }
    } else {
        LOG_Error("ReadSlaveModemUpgradeStatusFromUrl: Failed to parse JSON response");
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 0;
    }
    
    // 清理资源
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    LOG_Debug("ReadSlaveModemUpgradeStatusFromUrl: Successfully retrieved and parsed status: %d", ret_status);
    
    return ret_status;
}

//副模组接口返回当前升级状态
int ReadSlaveModemUpgradeStatus(void)
{
    const char *file_path = "/oemdata/parameters/SlaveModemUpgradeStatus";
    char cmd[512] = {0};
    char buffer[32] = {0};
    FILE *fp = NULL;
    
    // 使用 cat 命令读取文件
    snprintf(cmd, sizeof(cmd), "cat %s 2>/dev/null", file_path);
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        LOG_Debug("ReadSlaveModemUpgradeStatus: Failed to execute command, returning default status 0");
        return 0;
    }
    
    // 读取输出内容
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // 去除换行符和空格
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r' || buffer[len - 1] == ' ')) {
            buffer[--len] = '\0';
        }
        LOG_Debug("ReadSlaveModemUpgradeStatus: Read status %s ", buffer);
        // 使用 atoi 转换
        int status = atoi(buffer);
        pclose(fp);
        
        // 验证读取的值是否在有效范围内 (0-4)
        if (status >= 0 && status <= 4) {
            LOG_Debug("ReadSlaveModemUpgradeStatus: Successfully read valid status %d from %s", status, file_path);
            
        } else {
            LOG_Debug("ReadSlaveModemUpgradeStatus: Invalid status %d (not in 0-4 range), returning default status 0", status);
            return 0;
        }
        if(status == 1 ) //升级中
        {
            return MODEM_UPGRADE_STATUS_UPGRADING;
        }
        else if(status == 4 ) //升级请求
        {
            return MODEM_UPGRADE_STATUS_REQUESTED;
        }else if(status == 3){
            return MODEM_UPGRADE_STATUS_INVALID;  // 升级失败
        }else //升级完成
		{		
            return MODEM_UPGRADE_STATUS_COMPLETED;
        }
    }
    
    pclose(fp);
    LOG_Debug("ReadSlaveModemUpgradeStatus: File not found or empty, returning default status 0");
    return 0;
}
//主模组升级进度
//读取 /oemdata/parameters/MainModemUpProgress
uint8_t ReadMainUpProgress(void){
    FILE *fp = fopen("/oemdata/parameters/MainModemUpProgress", "r");
    if (fp == NULL) {
        LOG_Debug("ReadMainUpProgress: File not found or cannot open, returning default progress 0");
       return 0;
    }
    
    char buffer[16] = {0};
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        LOG_Debug("ReadMainUpProgress: Failed to read file content, returning default progress 0");
        fclose(fp);
       return 0;
    }
    
    fclose(fp);
    
    // 将字符串转换为整数
    int progress = atoi(buffer);
    
    // 确保值在 0-100 范围内
    if (progress < 0) {
        progress = 0;
    } else if (progress > 100) {
        progress = 100;
    }
    
    LOG_Debug("ReadMainUpProgress: Main modem upgrade progress = %d%%", progress);
   return (uint8_t)progress;
}

//副模组升级进度
//读取  /oemdata/parameters/SlaveModemUpProgress
uint8_t ReadSlaveUpProgress(void){
    FILE *fp = fopen("/oemdata/parameters/SlaveModemUpProgress", "r");
    if (fp == NULL) {
        LOG_Debug("ReadSlaveUpProgress: File not found or cannot open, returning default progress 0");
       return 0;
    }
    
    char buffer[16] = {0};
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        LOG_Debug("ReadSlaveUpProgress: Failed to read file content, returning default progress 0");
        fclose(fp);
       return 0;
    }
    
    fclose(fp);
    
    // 将字符串转换为整数
    int progress = atoi(buffer);
    
    // 确保值在 0-100 范围内
    if (progress < 0) {
        progress = 0;
    } else if (progress > 100) {
        progress = 100;
    }
    
    LOG_Debug("ReadSlaveUpProgress: Slave modem upgrade progress = %d%%", progress);
   return (uint8_t)progress;
}

//设置或清除RTC上报休眠事件
void SetOrClearRtcSleepEvent(uint8_t flag)
{
    int8_t ret = -1;
    
    pthread_mutex_lock(&g_PmEventMutex);
    
    if (flag == 1) {
        // 设置事件
        ret = PmSetSleepEvent("mqtt_rtc_send");
        LOG_Debug("%s, SetSleepEvent: mqtt_rtc_send, ret: %d", __FUNCTION__, ret);
        if (ret == 0) {
            g_RtcSleepEventSet = 1; // 标记事件已设置
        }
    } else {
        // 清除事件（只有在设置过之后才清除）
        if (g_RtcSleepEventSet == 1) {
            ret = PmCleanSleepEvent("mqtt_rtc_send");
            LOG_Debug("%s, CleanSleepEvent: mqtt_rtc_send, ret: %d", __FUNCTION__, ret);
            if (ret == 0) {
                g_RtcSleepEventSet = 0; // 重置标志
            }
        } else {
            LOG_Debug("%s, Skip clean operation, event not set yet", __FUNCTION__);
            ret = 0; // 未设置过也返回成功
        }
    }
    
    pthread_mutex_unlock(&g_PmEventMutex);
}

//设置当前唤醒源
void SetWakeupSrc(uint8_t WakeupSrc)
{
    // 如果唤醒源为心跳定时器且距离上次设置不足 15 秒，则跳过设置
    if (WakeupSrc == PM_WAKEUP_SOURCE_TIMER_HEARTBEAT) {
        time_t currentTime = time(NULL);
        pthread_rwlock_wrlock(&g_MQTTPmStateMutex);
        if (g_lastHeartbeatTime != 0 && (currentTime - g_lastHeartbeatTime) < 15) {
            pthread_rwlock_unlock(&g_MQTTPmStateMutex);
            LOG_Debug("%s, Skip heartbeat wakeup source set, interval: %ld seconds", 
                     __FUNCTION__, (long)(currentTime - g_lastHeartbeatTime));
            return;
        }
        g_lastHeartbeatTime = currentTime;
        pthread_rwlock_unlock(&g_MQTTPmStateMutex);
    }
    
    pthread_rwlock_wrlock(&g_MQTTPmStateMutex);
    g_WakeupSrc = WakeupSrc;
    LOG_Debug("%s, Set WakeupSrc:%u", __FUNCTION__, WakeupSrc);
	pthread_rwlock_unlock(&g_MQTTPmStateMutex);
    if(g_WakeupSrc == PM_WAKEUP_SOURCE_TIMER_HEARTBEAT)
    {
        SetOnceRtcflag();
        LOG_Debug("%s,Is RTC WakeupSrc:%u", __FUNCTION__, WakeupSrc);
    }
}

//获取当前唤醒源
uint8_t GetWakeupSrc(void)
{
    pthread_rwlock_rdlock(&g_MQTTPmStateMutex);
    uint8_t pmState = g_WakeupSrc;
	pthread_rwlock_unlock(&g_MQTTPmStateMutex);
    LOG_Debug("%s, Get g_WakeupSrc:%u", __FUNCTION__, pmState);
    return pmState;
}

//设置一次性RTC 唤醒标志
uint8_t SetOnceRtcflag(void)
{
    pthread_rwlock_wrlock(&g_MQTTPmStateMutex);
    g_RtcFlag = 1;
    g_OneMqttFlag = 0;  // 直接在锁内修改，避免调用 SetOneMqttFlag 导致死锁
    pthread_rwlock_unlock(&g_MQTTPmStateMutex);
    SetOrClearRtcSleepEvent(1);
    
    LOG_Debug("%s, Set g_RtcFlag:%u, g_OneMqttFlag:0", __FUNCTION__, g_RtcFlag);
    return g_RtcFlag;
}
//获取一次性 RTC 唤醒标志
uint8_t GetOnceRtcflag(void)
{
    pthread_rwlock_rdlock(&g_MQTTPmStateMutex);
    uint8_t rtcFlag = g_RtcFlag;
	pthread_rwlock_unlock(&g_MQTTPmStateMutex);
    LOG_Debug("%s, Get g_RtcFlag:%u", __FUNCTION__, rtcFlag);
    return rtcFlag;
}
//清除一次性sleep/RTC 唤醒标志
uint8_t ClearOnceRtcflag(void)
{
    pthread_rwlock_wrlock(&g_MQTTPmStateMutex);
    g_RtcFlag = 0;
	pthread_rwlock_unlock(&g_MQTTPmStateMutex);
    LOG_Debug("%s, Clear g_RtcFlag:%u", __FUNCTION__, g_RtcFlag);
    return g_RtcFlag;
}
//返回是否发送过 1 次 MQTT
uint8_t GetOneMqttFlag(void){
    pthread_rwlock_rdlock(&g_MQTTPmStateMutex);
    uint8_t mqttFlag = g_OneMqttFlag;
	pthread_rwlock_unlock(&g_MQTTPmStateMutex);
    LOG_Debug("%s, Get g_OneMqttFlag:%d", __FUNCTION__, mqttFlag);
    return mqttFlag;
}
//设置/清除    已发 1 次 MQTT 标志
uint8_t SetOneMqttFlag(uint8_t mqttFlag){
    pthread_rwlock_wrlock(&g_MQTTPmStateMutex);
    g_OneMqttFlag = mqttFlag;
	pthread_rwlock_unlock(&g_MQTTPmStateMutex);
    LOG_Debug("%s, Set g_OneMqttFlag:%d", __FUNCTION__, g_OneMqttFlag);
    return g_OneMqttFlag;
}
// 通用保存函数
int32_t SaveInfoToFile(const void* data, size_t dataSize, const char* fileName) {
    // 创建临时文件路径，在目标文件名后拼接"_tmp"
    size_t fileNameLen = strlen(fileName);
    char *tempFilePath = (char *)malloc(fileNameLen + 5);
    if(tempFilePath == NULL) {
        LOG_Error("SaveInfoToFile: unable to allocate memory for temp file path");
        return RET_ERR;
    }
    
    strcpy(tempFilePath, fileName);
    strcat(tempFilePath, "_tmp");
    
    FILE *tempFile = fopen(tempFilePath, "wb");
    if(tempFile == NULL) {
        LOG_Error("SaveInfoToFile: unable to create temp file %s", tempFilePath);
        free(tempFilePath);
        return RET_ERR;
    }
    
    size_t writeResult = fwrite(data, 1, dataSize, tempFile);
    if(writeResult != dataSize) {
        LOG_Error("SaveInfoToFile: write failed, wrote %zu bytes out of %zu", writeResult, dataSize);
        fclose(tempFile);
        unlink(tempFilePath);
        free(tempFilePath);
        return RET_ERR;
    }
    
    fflush(tempFile);
    fsync(fileno(tempFile));
    fclose(tempFile);
    
    if(rename(tempFilePath, fileName) != 0) {
        LOG_Error("SaveInfoToFile: unable to rename from %s to %s", tempFilePath, fileName);
        unlink(tempFilePath);
        free(tempFilePath);
        return RET_ERR;
    }
    
    // 确保目录更改也被写入磁盘
    DIR* dir_ptr = opendir("/oemdata/");
    if(dir_ptr != NULL) {
        int dirfd_val = dirfd(dir_ptr);
        fsync(dirfd_val);
        closedir(dir_ptr);
    }
    
    LOG_Debug("SaveInfoToFile: successfully saved data to %s", fileName);
    free(tempFilePath);
    return RET_OK;
}

// 通用加载函数
int32_t LoadInfoFromFile(void* data, size_t dataSize, const char* fileName) {
    FILE *file = fopen(fileName, "rb");
    if(file == NULL) {
        LOG_Warn("LoadInfoFromFile: backup file %s does not exist", fileName);
        memset(data, 0, dataSize); // 初始化为默认值
        return RET_ERR;
    }
    
    size_t readResult = fread(data, 1, dataSize, file);
    if(readResult != dataSize) {
        LOG_Warn("LoadInfoFromFile: read error, read %zu bytes out of %zu", readResult, dataSize);
        memset(data, 0, dataSize); // 初始化为默认值
        fclose(file);
        return RET_ERR;
    }
    
    fclose(file);
    LOG_Debug("LoadInfoFromFile: successfully loaded data from %s", fileName);
    return RET_OK;
}

// 检查数据是否为预设值（全0）
bool IsPresetValue(const void* data, size_t dataSize) {
    const uint8_t *pData = (const uint8_t*)data;
    for(size_t i = 0; i < dataSize; i++) {
        if(pData[i] != 0) {
            return false;
        }
    }
    return true;
}

#ifdef __cplusplus
}
#endif