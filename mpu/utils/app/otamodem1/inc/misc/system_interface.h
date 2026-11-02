#ifndef _SYSTEM_INTERFACE_H_
#define _SYSTEM_INTERFACE_H_

#include <string>
#include <stdint.h>
#include "pm_server_api.h"
#define TBOX_SN_LENGTH              (20)
#define TBOX_HW_VERSION_LENGTH      (17) 
#define TBOX_SW_VERSION_LENGTH      (17)
#define TBOX_MAX_APN_NUM            (5)
#define TBOX_CURRENT_APN_NUM        (2)
#define TBOX_VIN_LENGTH             (17)
#define TBOX_PARTS_NUM_LENGTH       (13)
#define TBOX_HOSTIP_LENGTH          (16)
#define TBOX_HOSTPORT_LENGTH         (5)
#define KL15_IS_ON 		(1)
#define KL15_IS_OFF 	(0)

#define ETHERNETFILEPATH    "/oemdata/upgrade/ethernet/ethernet.txt"
#define TBOXVERSION         "/oemapp/upgrade/version.txt"
#define EMMCDEVICENODE                          "/dev/mmcblk0"

#define RPCSERVER_TYPE_OTAAPP  (0x70001002)
#define RPCAPPOTA_MSGTYPE_SUBCMD_ID1 (0)
#define E_ParamId_CarNum (102)

typedef struct
{
    uint8_t kl15status;
    uint32_t kl30voltage;
    uint16_t fastCharge;
}HardLineSyncInfo_t;



int32_t GetHWVersion(uint8_t *hwversion);
int32_t GetSWVersion(char *swversion);
int32_t GetVIN(uint8_t *vin);
int32_t GetSN(uint8_t *sn);
void HardLineSyncCallBack(const uint8_t* data, const uint16_t len, void* userdata);
void HandlePmSleepCmd(const uint8_t *data, const uint16_t len, void *userdata);
int GetKl15Status();
MPUPmStateChange_t GetPmState();
int GetVehicleName(char * vehiclename);
int GetSystemRetValue(const char *cmd);
void RpcOtaAppCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);
int BlockTheProcess();
void CheckFileExists();
#endif