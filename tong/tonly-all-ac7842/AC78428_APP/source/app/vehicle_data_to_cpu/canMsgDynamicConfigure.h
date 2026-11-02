#ifndef _CAN_MSG_TO_CPU_CONFIGURE_SYNC_H
#define _CAN_MSG_TO_CPU_CONFIGURE_SYNC_H

#include <stdint.h>
#include "mpuHal.h"

int16_t CanMsgDynamicConfigureInitialize(int16_t mpuHandle);
void CanMsgToCpuConfigureSyncCycleProcess(MpuHalDataPack_t *pRxData,uint32_t cycleTime_ms);

#endif    //_CAN_MSG_TO_CPU_CONFIGURE_SYNC_H
