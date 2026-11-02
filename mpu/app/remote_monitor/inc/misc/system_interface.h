#ifndef _SYSTEM_INTERFACE_H_
#define _SYSTEM_INTERFACE_H_

#include <string>
#include <stdint.h>
#include "pm_server_api.h"
#define TBOX_SN_LENGTH              (27)
#define TBOX_HW_VERSION_LENGTH      (17) 
#define TBOX_SW_VERSION_LENGTH      (17)
#define TBOX_MAX_APN_NUM            (5)
#define TBOX_CURRENT_APN_NUM        (2)
#define TBOX_VIN_LENGTH             (17)
#define TBOX_PARTS_NUM_LENGTH       (13)
#define TBOX_HOSTIP_LENGTH          (16)
#define TBOX_HOSTPORT_LENGTH         (5)
#define KL15_IS_ON 		(2)
#define KL15_IS_OFF 	(1)
#define E_ParamId_CarNum (102)
#define E_ParamId_carDotaConfigMode (226)

typedef struct
{
    uint8_t kl15status;
    uint32_t kl30voltage;
    uint16_t fastCharge;
}HardLineSyncInfo_t;



int32_t GetHWVersion(uint8_t *hwversion);
int32_t GetSWVersion(uint8_t *swversion);
int32_t GetVIN(uint8_t *vin);
int32_t GetSN(uint8_t *sn);
void DeleteAppointmentFile(const char* filepatch);
int GetVehicleName(char * vehiclename);
//int32_t SrGetSystemRetValue(const char *cmd);
int GetCarotaConfigMode(char * carotaConfigMode);
bool SetCarotaConfigMode(char *carotaConfigMode, int32_t carotaConfigModeLen);

void AddCarotaConfigModeParam();
#endif
