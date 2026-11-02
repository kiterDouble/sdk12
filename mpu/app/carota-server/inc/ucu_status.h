// #ifndef __UCU_STATUS_H__
// #define __UCU_STATUS_H__
#ifndef UCU_STATUS_H  // 宏名建议用“头文件名大写+下划线”，避免冲突
#define UCU_STATUS_H

#ifdef __cplusplus
extern "C"
{
#endif
#pragma once 
typedef enum{
    UCU_STATUS_NORMAL = 0x00,
    UCU_STATUS_FOTA = 0x01,
    UCU_STAUTS_OBD = 0x02,
    UCU_STATUS_DOTA = 0x03,
}UcuStatus_e;// rdm status value

typedef enum{
    GW_STATUS_NORMAL = 0x00,
    GW_STATUS_FOTA = 0x01,
    GW_STAUTS_OBD = 0x02,
    GW_STATUS_DOTA = 0x03,
}GwStatus_e;// cpu-mcu value enum

typedef enum{
    CAN_LOCKED_NONE = 0x00,
    CAN_LOCKED_FOTA = 0x01,
    CAN_LOCKED_DOTA = 0x02,
}CanLockedType_e;//can locked typed;

typedef int (*UcuStatusChangedCb)(int old, int new);

int UcuStatusInit(UcuStatusChangedCb Cb);
int UcuStatusSetStatus(int status);
int UcuStatusClearStatus(int status);
int UcuStatusGetStatus(int *status);
int GWStatusGetStatus(int *status);
/**
 * Dbus数据填充获取接口
*/
int RpcGetVehicleStatus(int *status);
int UcuStatudCantpLock(int type);
int UcuStatudCantpUnlock(int type);
int UcuStatudGetCantpLockType(int *lockType);
void UcuStatusChangedCheck(int old, int new);
void CanLockCheckTimeout(int *lockType);
#ifdef __cplusplus
}
#endif
#endif