#ifndef _TASK_APPVEHICLE_DATA_TO_CPU_H
#define _TASK_APPVEHICLE_DATA_TO_CPU_H

#include <stdint.h>

//int16_t TaskAppVehicleDataToCpuInit(void);
//void DataUploadProcessSleep(void);
//void DataUploadProcessAwake(void);


//void VehicleToCpuGetRxCanCnt(uint16_t *pCanRxCnt);
//void VehicleToCpuUartTxDisable(void);
//void VehicleToCpuUartTxEnable(void);

void TaskVehicleDataToCpu( void *pvParameters );
#endif    //_TASK_APPVEHICLE_DATA_TO_CPU_H
