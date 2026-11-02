#include <cstring>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "system_interface.h"
#include "pm_server_api.h"
#include "sr_param.h"
#include "err_code.h"
#include "sr_log.h" 
#include <unistd.h>
#include "ota_api.h"
#include "sr_ipc.h"
#include "file_interface.h"
#include "http_thread.h"


//static HardLineSyncInfo_t hardLine;  
static uint8_t Kl15Status = KL15_IS_OFF;
//static int Kl15Status = KL15_IS_ON;
static MPUPmStateChange_t PmState;
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

int32_t GetSWVersion(char *swversion)
{
    int32_t ret = RET_OK;
    char data[64] = {0};
    if(swversion == NULL)
    {
        LOG_Error("Get swversion invalid param");
        return RET_ERR;
    }

    ret =  OpenReadFile(TBOXVERSION , data, sizeof(data));
    if(ret < 0)
    {
        LOG_Error("GetSWVersion OpenReadFile fail\n");
    }

    strcpy(swversion,data);

    return ret;
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

//	memcpy(vehiclename,"ZL00030",7);
//    return 0;
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


void HardLineSyncCallBack(const uint8_t* data, const uint16_t len, void* userdata)
{

    Kl15Status = data[0];
#if 0
    static int kl15 = KL15_IS_OFF;
    hardLine.kl15status = data[0];
    
    if(hardLine.kl15status != kl15)
    {
        Kl15Status = hardLine.kl15status;
        kl15 = hardLine.kl15status;
    }
#endif
    //LOG_Debug("%s called, kl15 valtage:%u mv.", __FUNCTION__, hardLine.kl15status);
    LOG_Debug("%s called, kl15 :%d .", __FUNCTION__, Kl15Status);
}

void HandlePmSleepCmd(const uint8_t *data, const uint16_t len, void *userdata)
{

    LOG_DebugHex(data,len,"HandlePmSleepCmd");
    
    memset(&PmState, 0x00, sizeof(MPUPmStateChange_t));
    if (sizeof(MPUPmStateChange_t) == len)
    {
        memmove(&PmState, data, len);
    }
   
}


int GetKl15Status()
{

    LOG_Debug("%s Kl15Status :%d .", __FUNCTION__, Kl15Status);
    return Kl15Status;
}


MPUPmStateChange_t GetPmState()
{
    return PmState;
}

int32_t SetSWVersion(uint8_t *swversion,int swversionlen)
{
    int32_t ret = RET_OK;
   
    if(swversion == NULL)
    {
        LOG_Error("Get swversion invalid param");
        return RET_ERR;
    }

    ret = UpdateParam(E_ParamId_SW_Version, swversion, swversionlen);
    if(ret < 0)
    {
        LOG_Error("UpdateParam swversion error");
        return RET_ERR;
    }

    return ret;
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


int BlockTheProcess()
{
    sigset_t mask;
    if (sigfillset(&mask) == -1) {
        LOG_Error("sigfillset error");
        return -1;
    }

    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
        LOG_Error("sigprocmask error");
        return -1;
    }

    LOG_Debug("PID: %d\n", getpid());
    LOG_Debug("Blocking all signals (except SIGKILL/SIGSTOP)...\n");

    pause();

    // 以下代码永远不会执行
    LOG_Error("This line will not be printed!\n");
    return 0;
}

void CheckFileExists()
{
    while (access(ETHERNETFILEPATH,F_OK) == 0)
    {
        LOG_Debug("文件存在：%s\n", ETHERNETFILEPATH);
        sleep(1);
    }
    LOG_Debug("文件已被移除或不再存在：%s\n", ETHERNETFILEPATH);
}


void RpcOtaAppCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    //回调处理函数中分别处理不同类型的请求
    if(RPCAPPOTA_MSGTYPE_SUBCMD_ID1 == subcmd)
    {
       *respData = GetAppOtaStatus();
       *respLen = sizeof(respData);
        LOG_Debug("RpcOtaAppCallBack respData = %d.....\n",*respData);
    }

}