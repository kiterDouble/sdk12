#ifndef __TSP_ODER_CHARGING__
#define __TSP_ODER_CHARGING__
#include "tsp_common.h"
#ifdef __cplusplus
extern "C"
{
#endif
#define CPU_STATUS_LEN (4)
#define TSP_STATUS_LEN (2)
gpointer OderChargingHandleTaskEntry(gpointer usrData);
gpointer CPUStatusThread(gpointer pArg);
#define  TSP_CONNECT_STATUS_FILE   "/oemdata/tmp/.tsp_connectStatus"
#define  GB_CONNECT_STATUS_FILE   "/oemdata/tmp/.connectStatus"
#define  CPU_TEMPRATURE_FILE   "/sys/class/thermal/thermal_zone0/temp"
#ifdef __cplusplus
}
#endif
#endif

