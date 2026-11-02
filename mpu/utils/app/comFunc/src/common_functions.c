#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "err_code.h"
#include "sr_log.h"
#include <pthread.h>
#include "common_functions.h"

/**
 * @name GetSystemRetValue
 * @brief 获取系统命令执行的返回值
 *        本函数通过调用system()函数执行指定的命令，并返回该命令的执行状态。
 * @param cmd 指向要执行的命令字符串的指针。
 * @return 如果执行成功，返回true；如果执行失败，返回false。
 */
gboolean GetSystemRetValue(const char *cmd)
{
	pid_t status;
	
	if(NULL == cmd)
	{
		return false;
	}
	//LOG_Debug("%s exe start", cmd);
	status = system((char *)cmd);
	//LOG_Debug("%s exe end:%d", cmd, status);
	if(-1 == status)
	{
        
		return false;
	}
	
	if(!WIFEXITED(status))
	{
		return false;
	}
	
	return true;
}

/**
 * @brief   获取VIN 
 * 
 * @param   len为tboxVin指向空间大小，必须大于等于18；
 * 
 * @retval	ture -- VIN获取成功
 *          false -- VIN获取失败
 */
gboolean GetVin(uint8_t *tboxVin, size_t len)
{
    if ((NULL == tboxVin) || (18 > len))
    {
        LOG_Error("param is NULL or len is too short");
        return FALSE;  
    }

    ParamInfo_t paramInfo;

    paramInfo.id = E_ParamId_VIN;
    if(GetParam(&paramInfo) < 0)
    {
        LOG_Error("Get Vin error");
        return FALSE;
    }

    if(strlen((char *)paramInfo.value) != 17)
    {
        LOG_Error("Get Vin Len error [%d] : %s", strlen((char *)paramInfo.value), paramInfo.value);
        return FALSE;
    }

    if (CheckVINIsEmpty(paramInfo.value) == FALSE)
    {
        LOG_Error("Vin is invalid");
        return FALSE;
    }

    size_t copyLen = MIN(strlen((char *)paramInfo.value) , (len - 1));
    memcpy(tboxVin, paramInfo.value, copyLen);
    tboxVin[copyLen] = '\0';
    
    LOG_Debug("Get Vin %s", tboxVin);

    return TRUE;
}

/**
 * @brief   判断VIN是否为空
 * 
 * @retval	ture -- VIN不为空
 *          false -- VIN为空
 */
gboolean CheckVINIsEmpty(const guint8 *vin)
{
    if(vin != NULL && strlen((const char *)vin) != 0)
    {
        return (g_strcmp0((char *)vin, "00000000000000000") == 0 ? FALSE : TRUE);
    }
    else
    {
        return FALSE;
    }
}

/**
 * @brief   判断VIN是否为空
 * 
 * @retval	ture -- VIN为空
 *          false -- VIN不为空
 */
gboolean CheckTboxVINIsEmpty()
{

    uint8_t tboxVin[32] = {0};
    if(GetVin(tboxVin, sizeof(tboxVin) - 1))
    {
        LOG_Debug("tboxVin = %s", (char *)tboxVin);
        return FALSE;
    }
    else
    {
        //LOG_Debug("tboxVin = %s", (char *)tboxVin);
        return TRUE;
    }
}

/**
 * @brief   获取ICCID
 * 
 * @param   len为tboxIccid指向空间大小，必须大于等于21；
 * 
 * @retval	ture -- ICCID获取成功
 *          false -- ICCID获取失败
 */
gboolean GetIccid(uint8_t *tboxIccid, size_t len)
{
    if ((NULL == tboxIccid) || (21 > len))
    {
        LOG_Error("param is NULL or len is too short");
        return FALSE;  
    }

    ParamInfo_t paramInfo;

    paramInfo.id = E_ParamId_ICCID;
    if(GetParam(&paramInfo) < 0)
    {
        LOG_Error("Get ICCID error");
        return FALSE;
    }

    if(strlen((char *)paramInfo.value) != 20)
    {
        LOG_Error("Get ICCID Len error [%d] : %s", strlen((char *)paramInfo.value), paramInfo.value);
        return FALSE;
    }

    if (CheckIccidIsEmpty(paramInfo.value) == FALSE)
    {
        LOG_Error("ICCID is invalid");
        return FALSE;
    }

    size_t copyLen = MIN(strlen((char *)paramInfo.value), len - 1);
    memcpy(tboxIccid, paramInfo.value, copyLen);
    tboxIccid[copyLen] = '\0';
    
    LOG_Debug("Get ICCID %s", tboxIccid);

    return TRUE;
}


gboolean GetCarNum(uint8_t *carNum, size_t len)
{
    if ((NULL == carNum) || (len < 8))
    {
        LOG_Error("param is NULL or len is too short");
        return FALSE;  
    }

    ParamInfo_t paramInfo;

    paramInfo.id = CAR_NUM_SQLITE_INDEX;
    if(GetParam(&paramInfo) < 0)
    {
        LOG_Error("Get ICCID error");
        return FALSE;
    }

    if(strlen((char *)paramInfo.value) != 7)
    {
        LOG_Error("Get car Num Len error [%d] : %s", strlen((char *)paramInfo.value), paramInfo.value);
        return FALSE;
    }

    if(0 == strncmp(paramInfo.value, "0000000", 7))
    {
        LOG_Error("Get car Num Len error [%d] : %s", strlen((char *)paramInfo.value), paramInfo.value);
        return FALSE;
    }
    memcpy(carNum, paramInfo.value, 7);
    LOG_Debug("Get carNum %s", carNum);

    return TRUE;
}


/**
 * @brief   判断ICCID是否为空
 * 
 * @retval	ture -- ICCID不为空
 *          false -- ICCID为空
 */
gboolean CheckIccidIsEmpty(const guint8 *iccid)
{
    if(iccid != NULL && strlen((const char *)iccid) != 0)
    {
        return (g_strcmp0((char *)iccid, "00000000000000000000") == 0 ? FALSE : TRUE);
    }
    else
    {
        return FALSE;
    }
}

/**
 * @brief   判断ICCID是否为空
 * 
 * @retval	ture -- ICCID不为空
 *          false -- ICCID为空
 */
gboolean CheckTboxIccidIsEmpty()
{
    uint8_t tboxIccid[32] = {0};
    if(GetIccid(tboxIccid, sizeof(tboxIccid) - 1))
    {
        LOG_Debug("tboxIccid = %s", (char *)tboxIccid);
        return TRUE;
    }
    else
    {
        LOG_Debug("tboxIccid = %s", (char *)tboxIccid);
        return FALSE;
    }
}

/**
 * @brief   获取软件版本号
 * 
 * @param   len为softVersion指向空间大小，必须大于等于12；
 * 
 * @retval	ture -- 版本获取成功
 *          false -- 版本获取失败
 */
gboolean GetSoftVersion(uint8_t *softVersion, size_t len)
{
    if ((NULL == softVersion) || (len < 12)) {
        LOG_Error("param is NULL or len is too short");
        return FALSE;
    }

    FILE *file = fopen("/oemdata/tmp/version.txt", "r");

    if (file == NULL) {
        LOG_Warn("Failed to open file /oemdata/tmp/version.txt");
        return FALSE;
    }

    char line[16] = {0};
    if (fgets(line, sizeof(line), file) == NULL) {
        LOG_Warn("File is empty or error occurred while reading.");
        fclose(file);
        return FALSE;
    }

    char *newline = strchr(line, '\n');
    if (newline != NULL) {
        *newline = '\0';
    }

    LOG_Debug("line = %s", line);

    fclose(file);
    size_t copyLen = MIN(strlen(line) , (len - 1));
    memcpy(softVersion, line, copyLen);
    softVersion[copyLen] = '\0';

    return TRUE;
}

/**
 * @brief   获取定位（从本地文件/oemdata/tmp/location中获取）
 * 
 * @param   len为location指向空间大小，必须大于等于22；
 * 
 * @retval	ture -- 定位获取成功
 *          false -- 定位获取失败
 */
gboolean GetLocation(uint8_t *location, size_t len)
{
    if ((NULL == location) || (len < 22)) {
        LOG_Error("param is NULL or len is too short");
        return FALSE;
    }

    FILE *file = fopen("/oemdata/tmp/location", "r");

    if (file == NULL) {
        LOG_Warn("Failed to open file /oemdata/tmp/location");
        return FALSE;
    }

    char line[32] = {0};
    if (fgets(line, sizeof(line), file) == NULL) {
        LOG_Warn("File is empty or error occurred while reading.");
        fclose(file);
        return FALSE;
    }

    char *newline = strchr(line, '\n');
    if (newline != NULL) {
        *newline = '\0';
    }

    LOG_Debug("line = %s", line);
    fclose(file);
    size_t copyLen = MIN(strlen(line) , (len - 1));
    memcpy(location, line, copyLen);
    location[copyLen] = '\0';

    return TRUE;
}

/**
 * @brief   获取TspTokenUrl
 * 
 * @param   len为TspTokenUrl指向空间大小，必须大于等于77；
 * 
 * @retval	ture -- TspTokenUrl获取成功
 *          false -- TspTokenUrl获取失败
 */
gboolean GetTspTokenUrlUrl(uint8_t *TspTokenUrl, size_t len)
{
    if ((NULL == TspTokenUrl) || (77 > len))
    {
        LOG_Error("param is NULL or len is too short");
        return FALSE;  
    }

    ParamInfo_t paramInfo;

    paramInfo.id = MQTT_ADDR_SQLITE_INDEX;
    if(GetParam(&paramInfo) < 0)
    {
        LOG_Error("Get TspTokenUrl error");
        return FALSE;
    }

    if (strlen((char *)paramInfo.value) > (len - 1))
    {
        LOG_Error("Get TspTokenUrl error");
        return FALSE;
    }

    size_t copyLen = MIN(strlen((char *)paramInfo.value) , (len - 1));
    memcpy(TspTokenUrl, paramInfo.value, copyLen);
    TspTokenUrl[copyLen] = '\0';
    
    LOG_Debug("Get TspTokenUrl %s", TspTokenUrl);

    return TRUE;
}
/**
 * @brief   获取编译时间
 * 
 * @param   
 * 
 * @retval	
 *          
 */

time_t GetCompareTime() {
    struct tm compile_tm;
    time_t compile_time;
    time_t current_time;
    memset(&compile_tm, 0, sizeof(struct tm));  // 清零初始化
    // 解析编译日期和时间
    strptime(__DATE__ " " __TIME__, "%b %d %Y %H:%M:%S", &compile_tm);
    compile_tm.tm_isdst = -1; 
    // 将编译日期和时间转换为时间戳
    compile_time = mktime(&compile_tm);
    return compile_time;
}

/**
 * @brief   获取在线配置参数
 * 
 * @param   len为ConfigurationParam指向空间大小，必须大于等于77；
 * 
 * @retval	ture -- ConfigurationParam获取成功
 *          false -- ConfigurationParam获取失败
 */
gboolean GetConfiguration(uint8_t *ConfigurationParam, size_t len)
{
    if ((NULL == ConfigurationParam) || (64 > len))
    {
        LOG_Error("param is NULL or len is too short");
        return FALSE;  
    }
    ParamInfo_t paramInfo = {0};

    paramInfo.id = E_ParamId_Configuration;
    if(GetParam(&paramInfo) < 0)
    {
        LOG_Error("Get TspTokenUrl error");
        return FALSE;
    }
    if (strlen((char *)paramInfo.value) > (len - 1))
    {
        LOG_Error("Get TspTokenUrl error");
        return FALSE;
    }
    size_t copyLen = MIN(strlen((char *)paramInfo.value) , (len - 1));
    memcpy(ConfigurationParam, paramInfo.value, copyLen);
    ConfigurationParam[copyLen] = '\0';
    
    LOG_Debug("Get ConfigurationParam %s", ConfigurationParam);

    return TRUE;
}

/**
 * @brief   获取SN序列号
 * 
 * @param   len为指向空间大小，必须大于等于28；
 * 
 * @retval	ture -- tboxSN获取成功
 *          false -- tboxSN获取失败
 */
gboolean GetSN(uint8_t *tboxSN, size_t len)
{
    if ((NULL == tboxSN) || (28 > len))
    {
        LOG_Error("param is NULL or len is too short");
        return FALSE;  
    }

    ParamInfo_t paramInfo;

    paramInfo.id = E_ParamId_SN;
    if(GetParam(&paramInfo) < 0)
    {
        LOG_Error("Get tboxSN error");
        return FALSE;
    }

    // if(strlen((char *)paramInfo.value) != 27)
    // {
    //     LOG_Error("Get tboxSN Len error [%d] : %s", strlen((char *)paramInfo.value), paramInfo.value);
    //     return FALSE;
    // }

    if (CheckSNIsEmpty(paramInfo.value) == FALSE)
    {
        LOG_Error("tboxSN is invalid");
        return FALSE;
    }

    size_t copyLen = MIN(strlen((char *)paramInfo.value) , (len - 1));
    memcpy(tboxSN, paramInfo.value, copyLen);
    tboxSN[copyLen] = '\0';
    
    LOG_Debug("Get tboxSN %s", tboxSN);

    return TRUE;
}

/**
 * @brief   判断TboxSN是否为空
 * 
 * @retval	ture -- TboxSN不为空
 *          false -- TboxSN为空
 */
gboolean CheckSNIsEmpty(const guint8 *tboxSN)
{
    if(tboxSN != NULL && strlen((const char *)tboxSN) != 0)
    {
        return (g_strcmp0((char *)tboxSN, "000000000000000000000000000") == 0 ? FALSE : TRUE);
    }
    else
    {
        return FALSE;
    }
}

/**
 * @brief   判断TboxSN是否为空
 * 
 * @retval	ture -- TboxSN不为空
 *          false -- TboxSN为空
 */
gboolean CheckTboxSNIsEmpty()
{

    uint8_t tboxSN[32] = {0};
    if(GetSN(tboxSN, sizeof(tboxSN) - 1))
    {
        LOG_Debug("tboxSN = %s", (char *)tboxSN);
        return TRUE;
    }
    else
    {
        LOG_Debug("tboxSN = %s", (char *)tboxSN);
        return FALSE;
    }
}

/**
 * @brief   获取IMEI
 * 
 * @param   len为指向空间大小，必须大于等于16；
 * 
 * @retval	ture -- IMEI获取成功
 *          false -- IMEI获取失败
 */
gboolean GetIMEI(uint8_t *IMEI, size_t len)
{
    if ((NULL == IMEI) || (15 > len))
    {
        LOG_Error("param is NULL or len is too short");
        return FALSE;  
    }

    ParamInfo_t paramInfo;

    paramInfo.id = E_ParamId_IMEI;
    if(GetParam(&paramInfo) < 0)
    {
        LOG_Error("Get IMEI error");
        return FALSE;
    }

    if(strlen((char *)paramInfo.value) != 15)
    {
        LOG_Error("Get IMEI Len error [%d] : %s", strlen((char *)paramInfo.value), paramInfo.value);
        return FALSE;
    }

    if (CheckIMEIIsEmpty(paramInfo.value) == FALSE)
    {
        LOG_Error("IMEI is invalid");
        return FALSE;
    }

    size_t copyLen = MIN(strlen((char *)paramInfo.value) , (len - 1));
    memcpy(IMEI, paramInfo.value, copyLen);
    IMEI[copyLen] = '\0';
    
    LOG_Debug("Get IMEI %s", IMEI);

    return TRUE;
}

/**
 * @brief   判断IMEI是否为空
 * 
 * @retval	ture -- IMEI不为空
 *          false -- IMEI为空
 */
gboolean CheckIMEIIsEmpty(const guint8 *IMEI)
{
    if(IMEI != NULL && strlen((const char *)IMEI) != 0)
    {
        return (g_strcmp0((char *)IMEI, "000000000000000") == 0 ? FALSE : TRUE);
    }
    else
    {
        return FALSE;
    }
}

/**
 * @brief   判断IMEI是否为空
 * 
 * @retval	ture -- IMEI不为空
 *          false -- TIMEI为空
 */
gboolean CheckTboxIMEIIsEmpty()
{

    uint8_t IMEI[32] = {0};
    if(GetIMEI(IMEI, sizeof(IMEI) - 1))
    {
        LOG_Debug("IMEI = %s", (char *)IMEI);
        return TRUE;
    }
    else
    {
        LOG_Debug("IMEI = %s", (char *)IMEI);
        return FALSE;
    }
}

/**
 * @brief   获取GB addr
 * 
 * @param   len为GBAddr指向空间大小，必须大于等于65；
 * 
 * @retval	ture -- GB addr获取成功
 *          false -- GB addr获取失败
 */
gboolean GetGBAddr(uint8_t *GBAddr, size_t len)
{
    if ((NULL == GBAddr) || (64 > len))
    {
        LOG_Error("param is NULL or len is too short");
        return FALSE;  
    }

    ParamInfo_t paramInfo;

    paramInfo.id = E_ParamId_GB32960Addr;
    if(GetParam(&paramInfo) < 0)
    {
        LOG_Error("Get GBAddr error");
        return FALSE;
    }

    if (strlen((char *)paramInfo.value) > (len - 1))
    {
        LOG_Error("Get GBAddr error");
        return FALSE;
    }

    size_t copyLen = MIN(strlen((char *)paramInfo.value) , (len - 1));
    memcpy(GBAddr, paramInfo.value, copyLen);
    GBAddr[copyLen] = '\0';
    
    LOG_Debug("Get GBAddr %s", GBAddr);

    return TRUE;
}

gboolean GetTboxSoftVersion(uint8_t *softVersion, int32_t softVersionLen)
{
    if ((NULL == softVersion) || softVersionLen <= 0)
    {
        LOG_Error("param is NULL or len is too short");
        return FALSE;  
    }

    ParamInfo_t paramInfo;

    paramInfo.id = E_ParamId_SW_Version;
    if(GetParam(&paramInfo) < 0)
    {
        LOG_Error("Get E_ParamId_SW_Version error");
        return FALSE;
    }
    
    if (strlen((char *)paramInfo.value) >= softVersionLen)
    {
        LOG_Error("Get E_ParamId_SW_Version len error");
        return FALSE;
    }
    
    memcpy(softVersion, paramInfo.value, strlen((char *)paramInfo.value));

    return TRUE;
}

gboolean SetTboxSoftVersion(uint8_t *softVersion, int32_t softVersionLen)
{
    if ((NULL == softVersion) || softVersionLen <= 0)
    {
        LOG_Error("param is NULL or len is too short");
        return FALSE;  
    }

    if(UpdateParam(E_ParamId_SW_Version, softVersion, softVersionLen) < 0)
    {
        LOG_Error("Set E_ParamId_SW_Version error");
        return FALSE;
    }

    return TRUE;
}


/**
 * @brief   获取GB Frequency 不可用
 * 
 * @param   整形；
 * 
 * @retval	ture -- GBFrequency获取成功
 *          false -- GBFrequency获取失败
 */
/*gboolean GetGBFrequency(uint8_t *GBFrequency)
{
    if ((NULL == GBFrequency))
    {
        LOG_Error("param is NULL");
        return FALSE;  
    }

    ParamInfo_t paramInfo;

    paramInfo.id = E_ParamId_GBFrequency;
    if(GetParam(&paramInfo) < 0)
    {
        LOG_Error("Get GBFrequency error");
        return FALSE;
    }

    if((paramInfo.value[0] < 0) || (paramInfo.value[0] > 30))
    {
        LOG_Error("GBFrequency is over");
        return FALSE;
    }

    *GBFrequency = paramInfo.value[0];
    
    LOG_Debug("Get GBFrequency %d", *GBFrequency);

    return TRUE;
}*/

/**
 * @brief   设置GB Frequency 不可用
 * 
 * @param   GBFrequency大于0 小于等于30；
 * 
 * @retval	ture -- GBFrequency设置成功
 *          false -- GBFrequency设置失败
 */
/*gboolean SetGBFrequency(uint8_t GBFrequency)
{
    if((GBFrequency < 0) || (GBFrequency > 30))
    {
        LOG_Error("GBFrequency is over");
        return FALSE;   
    }

    if(UpdateParam(E_ParamId_GBFrequency, &GBFrequency, 1) < 0)
    {
        LOG_Error("Set GBFrequency error");
        return FALSE;
    }
    
    LOG_Debug("Set GBFrequency %d", GBFrequency);

    return TRUE;
}*/

/**
 * @brief   检查IP地址是否被路由到某个网卡上
 * 
 * @param   ifname 网卡名称；
 *          ipaddr IP地址；
 * 
 * @retval	TRUE -- IP被路由到网卡中
 *          FALSE -- IP没有被路由到网卡中
 */
gboolean checkIPonInterface(const char* ifname, struct in_addr ipaddr) 
{
    if (!ifname)
    {
        LOG_Error("ifname is null");
        return FALSE;
    }

    FILE *fp;
    char line[256];

    fp = fopen("/proc/net/route", "r");
    if (fp == NULL) {
        LOG_Debug("open /proc/net/route failed");
        return FALSE;
    }

    // 将IPv4地址从网络字节序转换为主机字节序
    uint32_t host_order_addr = ntohl(ipaddr.s_addr);

    // 将主机字节序IPv4地址转换为小端十六进制字符串
    char hex_str[sizeof("0xFFFFFFFF")] = "0"; // 存储十六进制字符串的缓冲区
    snprintf(hex_str, sizeof(hex_str), "%02X%02X%02X%02X", 
             (host_order_addr & 0xFF),
             ((host_order_addr >> 8) & 0xFF),
             ((host_order_addr >> 16) & 0xFF),
             ((host_order_addr >> 24) & 0xFF));
	//LOG_Debug("hex_str = %s", hex_str);

    while (fgets(line, sizeof(line), fp)) {
		//LOG_Debug("line = %s", line);
        char iface[16], dest[16];
        sscanf(line, "%s %s", iface, dest);
		//LOG_Debug("iface = %s, dest = %s", iface, dest);

        if (strcmp(dest, hex_str) == 0 && strcmp(iface, ifname) == 0) 
		{
			//LOG_Debug("IP %s has bound on %s", hex_str, ifname);
            fclose(fp);
            return TRUE;
        }
    }

    fclose(fp);
    return FALSE;
}

