#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <curl/curl.h>
#include <unistd.h>
#include "system_interface.h"
#include "pm_server_api.h"
#include "sr_param.h"
#include "err_code.h"
#include "sr_log.h" 
#include "cJSON.h"


int32_t GetHWVersion(uint8_t *hwversion)
{
    int32_t ret = RET_OK;
    ParamInfo_t paranInfo;

    if(hwversion == NULL)
    {
        LOG_Error("Get hwversion invalid param");
        return RET_ERR;
    }

    memset(&paranInfo, 0, sizeof(ParamInfo_t));
    paranInfo.id = E_ParamId_HW_Version;
    if(GetParam(&paranInfo) < 0)
    {
        LOG_Error("Get hwversion error");
        return RET_ERR;
    }
    memcpy(hwversion, paranInfo.value, TBOX_SW_VERSION_LENGTH);

    return ret;
}

int32_t GetSWVersion(uint8_t *swversion)
{
    int32_t ret = RET_OK;
    ParamInfo_t paranInfo;

    if(swversion == NULL)
    {
        LOG_Error("Get swversion invalid param");
        return RET_ERR;
    }

    memset(&paranInfo, 0, sizeof(ParamInfo_t));
    paranInfo.id = E_ParamId_SW_Version;
    if(GetParam(&paranInfo) < 0)
    {
        LOG_Error("Get swversion error");
        return RET_ERR;
    }
    memcpy(swversion, paranInfo.value, TBOX_SW_VERSION_LENGTH);

    return ret;
}



int32_t GetVIN(uint8_t *vin)
{
    int32_t ret = RET_OK;
    ParamInfo_t paranInfo;

    if(vin == NULL)
    {
        LOG_Error("Get VIN invalid param");
        return RET_ERR;
    }

    memset(&paranInfo, 0, sizeof(ParamInfo_t));
    paranInfo.id = E_ParamId_VIN;
    if(GetParam(&paranInfo) < 0)
    {
        LOG_Error("Get vin error");
        return RET_ERR;
    }

    //memcpy(vin, paranInfo.value, TBOX_VIN_LENGTH);
    sprintf((char *)vin, "%s", paranInfo.value); 
    return ret;
}


int32_t GetSN(uint8_t *sn)
{
    int32_t ret = RET_OK;
    ParamInfo_t paranInfo;

    if(sn == NULL)
    {
        LOG_Error("Get SN invalid param");
        return RET_ERR;
    }

    memset(&paranInfo, 0, sizeof(ParamInfo_t));
    paranInfo.id = E_ParamId_SN;
    if(GetParam(&paranInfo) < 0)
    {
        LOG_Error("Get SN error");
        return RET_ERR;
    }
    memcpy(sn, paranInfo.value, TBOX_SN_LENGTH);

    return ret;
}



void DeleteAppointmentFile(const char* filepatch)
{
    char cmd[64] = {0};
    sprintf(cmd,"rm -rf %s",filepatch);
    system(cmd);
}


int GetVehicleName(char * vehiclename)
{

    int32_t ret = RET_OK;
    ParamInfo_t paranInfo;

    if(vehiclename == NULL)
    {
        LOG_Error("Get VIN invalid param");
        return RET_ERR;
    }

    memset(&paranInfo, 0, sizeof(ParamInfo_t));
    paranInfo.id = E_ParamId_CarNum;
    if(GetParam(&paranInfo) < 0)
    {
        LOG_Error("Get vin error");
        return RET_ERR;
    }

    sprintf((char *)vehiclename, "%s", paranInfo.value); 
    return ret;
}


void AddCarotaConfigModeParam()
{
    int ret = 0;
    ParamInfo_t data = {0};

    data.id = E_ParamId_carFotaConfigMode;
    if(GetParam(&data) < 0)
    {
        LOG_Error("Get E_ParamId_carotaConfigMode error");
        memset(&data, 0, sizeof(ParamInfo_t));
        data.id = E_ParamId_carFotaConfigMode;
        strcpy((char *)data.name, "E_ParamId_carFotaConfigMode");
        data.type = 0;
        data.min = 0;
        data.max = 128;
        data.paramSources = 3;
        data.rpcWriteEnable = 1;
        strcpy((char *)data.value, "uat");
        ret = AddUserParam(data);
        if(ret < 0 )
        {
            LOG_Error("AddUserParam Fail");
        }

    }else{
        LOG_Debug("GetParam ok");
        return;
    }
   
    
}




int GetCarotaConfigMode(char * carotaConfigMode)
{

    int32_t ret = RET_OK;
    ParamInfo_t paranInfo = {0};

    if(carotaConfigMode == NULL)
    {
        LOG_Error("Get carotaConfigMode invalid param");
        return RET_ERR;
    }

    memset(&paranInfo, 0, sizeof(ParamInfo_t));
    paranInfo.id = E_ParamId_carFotaConfigMode;
    if(GetParam(&paranInfo) < 0)
    {
        LOG_Error("Get carotaConfigMode error");
        return RET_ERR;
    }

    sprintf((char *)carotaConfigMode, "%s", paranInfo.value); 
   
    printf("carotaConfigMode : %s \n",carotaConfigMode);
    return 0;
}

bool SetCarotaConfigMode(char *carotaConfigMode, int32_t carotaConfigModeLen)
{
    if ((NULL == carotaConfigMode) || carotaConfigModeLen <= 0)
    {
        LOG_Error("param is NULL or len is too short");
        return FALSE;  
    }

    if(UpdateParam(E_ParamId_carFotaConfigMode, (unsigned char *)carotaConfigMode, carotaConfigModeLen) < 0)
    {
        LOG_Error("Set E_ParamId_carotaConfigMode error");
        return FALSE;
    }

    return TRUE;
}


// ==================== 副模组启动时向主模组同步域参数(226/227) ====================
#define MASTER_DOMAIN_PARAM_URL "http://192.168.1.201:20003/getDomainParam"

typedef struct
{
    char *data;
    size_t len;
} SyncParamResp_t;

static size_t SyncParamWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    SyncParamResp_t *resp = (SyncParamResp_t *)userp;
    char *ptr = (char *)realloc(resp->data, resp->len + realsize + 1);
    if (NULL == ptr)
    {
        LOG_Error("[SyncDomainParamsFromMaster] realloc fail");
        return 0;
    }
    resp->data = ptr;
    memcpy(&(resp->data[resp->len]), contents, realsize);
    resp->len += realsize;
    resp->data[resp->len] = '\0';
    return realsize;
}

int SyncDomainParamsFromMaster(void)
{
    // 仅副模组需要同步；主模组直接返回
    if (0 == access("/dev/mmcblk0", F_OK))
    {
        return 0;
    }

    CURL *curl = curl_easy_init();
    if (NULL == curl)
    {
        LOG_Error("[SyncDomainParamsFromMaster] curl init fail");
        return -1;
    }

    SyncParamResp_t resp = {0};
    resp.data = (char *)malloc(1);
    if (NULL == resp.data)
    {
        curl_easy_cleanup(curl);
        return -1;
    }
    resp.data[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, MASTER_DOMAIN_PARAM_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, SyncParamWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&resp);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    LOG_Debug("[SyncDomainParamsFromMaster] request url: %s", MASTER_DOMAIN_PARAM_URL);
    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || http_code != 200)
    {
        LOG_Error("[SyncDomainParamsFromMaster] request fail res:%d http:%ld", res, http_code);
        free(resp.data);
        return -1;
    }
    LOG_Debug("[SyncDomainParamsFromMaster] response http:%ld body: %s", http_code, resp.data);

    cJSON *root = cJSON_Parse(resp.data);
    free(resp.data);
    if (NULL == root)
    {
        LOG_Error("[SyncDomainParamsFromMaster] json parse fail");
        return -1;
    }

    int ret = 0;
    cJSON *dota = cJSON_GetObjectItem(root, "carDotaConfigMode");
    if (dota && dota->valuestring && dota->valuestring[0] != '\0')
    {
        LOG_Debug("[SyncDomainParamsFromMaster] recv 226(carDotaConfigMode): %s", dota->valuestring);
        if (UpdateParam(E_ParamId_carDotaConfigMode, (uint8_t *)dota->valuestring, strlen(dota->valuestring)) < 0)
        {
            LOG_Error("[SyncDomainParamsFromMaster] UpdateParam 226 fail");
            ret = -1;
        }
    }

    cJSON *fota = cJSON_GetObjectItem(root, "carFotaConfigMode");
    if (fota && fota->valuestring && fota->valuestring[0] != '\0')
    {
        LOG_Debug("[SyncDomainParamsFromMaster] recv 227(carFotaConfigMode): %s", fota->valuestring);
        if (UpdateParam(E_ParamId_carFotaConfigMode, (uint8_t *)fota->valuestring, strlen(fota->valuestring)) < 0)
        {
            LOG_Error("[SyncDomainParamsFromMaster] UpdateParam 227 fail");
            ret = -1;
        }
    }

    cJSON_Delete(root);
    return ret;
}



/**
 * @name SrGetSystemRetValue
 * @brief  执行shell 命令，并获取返回值
 * @param 
 * @return -1 执行命令失败，其他值为命令的返回值
 */
int32_t SrGetSystemRetValue(const char *cmd)
{
	char cmdBuff[128] = {0};
	FILE *fileFp;				// 文件指针  
    int32_t cmdStatus;			// 用于存储命令的退出状态
	
	if(NULL == cmd)
	{
		LOG_Error("%s cmd is NULL", __FUNCTION__);
		return RET_ERR;
	}
    
    LOG_Debug("start exec cmd :%s", cmd);

	fileFp = popen(cmd, "r");
	if(NULL == fileFp)
	{
		LOG_Error("%s popen error :%s", __FUNCTION__, strerror(errno));
		return RET_ERR;
	}
	
	while(fgets(cmdBuff, 128, fileFp) != NULL)
	{
		LOG_Debug("get cmd ret: %s", cmdBuff);
	}
	
	cmdStatus = pclose(fileFp);
	if(-1 == cmdStatus)
	{
		LOG_Error("%s pclose error :%s", __FUNCTION__, strerror(errno));
		return RET_ERR;
	}
	
	if(WIFEXITED(cmdStatus))
	{  
        LOG_Debug("Command exited with status %d", WEXITSTATUS(cmdStatus));
		return WEXITSTATUS(cmdStatus);
    }
	else if(WIFSIGNALED(cmdStatus))
	{  
        LOG_Error("Command was terminated by signal %d", WTERMSIG(cmdStatus));
		return RET_ERR;
    }
	
	LOG_Error("Command was terminated not find signal" );
	return RET_ERR;
}
