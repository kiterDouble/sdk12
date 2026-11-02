/*************************************************
Copyright ? 2026 SiRun (AnHui) . All rights reserved.
  File Name:       firmwareUpdateSdk.h
  Author:          huShangrui
  Created Time:    2026年5月11日
  Description:     upgradation module
   Others:
*************************************************/
#ifndef _BOOT_FIRMWARE_UPDATE_SDK_H_
#define _BOOT_FIRMWARE_UPDATE_SDK_H_
/****************************** include ***************************************/
#include "stdint.h"

/****************************** Macro Definitions *****************************/
#define PROTOCOL_AID_FWUPD   (0x03U)

/****************************** Type Definitions ******************************/
typedef enum
{
    E_FlashState_Idle,
    E_FlashState_FlashIn,
    E_FlashState_FlashUnlock,
    E_FlashState_FlashErase,
    E_FlashState_FlashData,
    E_FlashState_FlashFinish,
    E_FlashState_Reset,
    E_FlashState_Error,
}FlashState_e;

typedef enum
{
    E_FirmwareUpdateSdkCmd_Default                = 0u,  
    E_FirmwareUpdateSdkCmd_GetMcuVersion          = 1u,
    E_FirmwareUpdateSdkCmd_GetMcuSeed             = 2u,
    E_FirmwareUpdateSdkCmd_UnlockMcuFlash         = 3u,
    E_FirmwareUpdateSdkCmd_EnterDownloadMode      = 4u,
    E_FirmwareUpdateSdkCmd_EraseMcuMemory         = 5u,
    E_FirmwareUpdateSdkCmd_DownloadMcuMemory      = 6u,
    E_FirmwareUpdateSdkCmd_VirifyMcuMemory        = 7u,
    E_FirmwareUpdateSdkCmd_SoftwareResetMcu       = 8u,
    E_FirmwareUpdateSdkCmd_GetMcuBankId           = 9u,
}FirmwareUpdateSdkCmd_e;

/****************************** Function Declarations *************************/
/*************************************************
 Function: FirmwareUpdateSdkInit
 Description: 初始化固件升级SDK
 Input: void
 Output: None
 Return: int16_t 0 成功 -1 失败
 Others:
*************************************************/
int16_t FirmwareUpdateSdkInit(void);

/*************************************************
  Function:       FirmwareUpdateSdkCycleProcess
  Description:    升级模块周期调用接口
  Input:          无
  Output:         无
  Return:         无
  Others:         
*************************************************/
void FirmwareUpdateSdkCycleProcess(void);

#endif
