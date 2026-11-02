#include "logHal.h"
#include "stdlib.h"
#include "stateSyncSdk.h"
#include "taskDtcProcess.h"
#include <string.h>
#include "peripheralHal.h"
#include "canPeriodTask.h"
#define CPU_INFO_FAILURE_TIME 30                            // cpu信息失效时间 单位：秒
static CpuModuleInfo4GSync_t g_cpu4gModuleInfo;             // 0xB24C_cxl 0xB24D_cxl
static CpuEthernetIdSync_t g_cpuEthernetId;                 // 0xB256_cxl
static CpuEmmcDeleteStatusSync_t g_cpuEmmcDeleteStatus;     // 0xB259_cxl
static CpuAppVersion4GSync_t g_cpu4gAppVersion;             // 0xB25B_cxl
static CpuPkiCertStatusSync_t g_cpuPkiCertStatus;           // 0xB262_cxl
static CpuTspConnectionStatusSync_t g_cpuTspConnStatusSync; // 0xB28E_cxl
static CpuFtyDataSync_t g_cpuFtyDataSync;                   // 0xDTC_table_cxl

static uint16_t g_processCycleTime = 10; // 保存StateSyncSdkCycleProcess接口调用周期，单位 ms
// static int16_t	g_mpuHandle = 0;                  //保存同步任务的CPU通信句柄
// static int16_t	g_powerManageHandle = -1;     //电源管理状态句柄
static CpuLocationInfo_t g_cpuLocationInfo; // MPU 定位信息存储
static CpuNetInfoSync_t g_cpuNetInfo;       // MPU 网络信息存储
static CpuHalStateSync_t g_cpuHalState;     // MPU硬件信息存储
static CpuTspStateSync_t g_cpuTspState;     // MPU平台连接信息
// static GsensorStateSync_t g_gsensorState;          //G-Sensor状态

/*************************************************
  Function:       StateSyncSdkInit
  Description:    状态同步模块初始化接口
  Input:          mpuHandle：mpu通信句柄
                  cycleTime：周期调用接口调用周期
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:
*************************************************/
int16_t StateSyncSdkInit(int16_t mpuHandle, uint16_t cycleTime)
{
    if ((mpuHandle < 0) && (cycleTime <= 0))
    {
        return -1;
    }
    g_processCycleTime = cycleTime;
    // g_mpuHandle = mpuHandle;
    g_cpuFtyDataSync.failureTime = CPU_INFO_FAILURE_TIME * 1000;
    g_cpuFtyDataSync.timeCount = 0;
    g_cpuFtyDataSync.validity = 0;
    memset(&g_cpuFtyDataSync.payload, 0, sizeof(ftyCircleDataToMcu_t));

    g_cpuLocationInfo.failureTime = CPU_INFO_FAILURE_TIME * 1000;
    g_cpuLocationInfo.timeCount = 0;
    g_cpuLocationInfo.validity = 0;

    g_cpuNetInfo.failureTime = CPU_INFO_FAILURE_TIME * 1000;
    g_cpuNetInfo.timeCount = 0;
    g_cpuNetInfo.validity = 0;

    g_cpuHalState.failureTime = CPU_INFO_FAILURE_TIME * 1000;
    g_cpuHalState.timeCount = 0;
    g_cpuHalState.validity = 0;

    g_cpuTspState.failureTime = CPU_INFO_FAILURE_TIME * 1000;
    g_cpuTspState.timeCount = 0;
    g_cpuTspState.validity = 0;
    // 0xB24C_cxl 0xB24D_cxl
    g_cpu4gModuleInfo.failureTime = CPU_INFO_FAILURE_TIME * 1000;
    g_cpu4gModuleInfo.timeCount = 0;
    g_cpu4gModuleInfo.validity = 0;
    // 0xB256_cxl
    g_cpuEthernetId.failureTime = CPU_INFO_FAILURE_TIME * 1000;
    g_cpuEthernetId.timeCount = 0;
    g_cpuEthernetId.validity = 0;
    // 0xB259_cxl
    g_cpuEmmcDeleteStatus.failureTime = CPU_INFO_FAILURE_TIME * 1000;
    g_cpuEmmcDeleteStatus.timeCount = 0;
    g_cpuEmmcDeleteStatus.validity = 0;
    // 0xB25B_cxl
    g_cpu4gAppVersion.failureTime = CPU_INFO_FAILURE_TIME * 1000;
    g_cpu4gAppVersion.timeCount = 0;
    g_cpu4gAppVersion.validity = 0;
    // 0xB262_cxl
    g_cpuPkiCertStatus.failureTime = CPU_INFO_FAILURE_TIME * 1000;
    g_cpuPkiCertStatus.timeCount = 0;
    g_cpuPkiCertStatus.validity = 0;
    // 0xB28E_cxl
    g_cpuTspConnStatusSync.failureTime = CPU_INFO_FAILURE_TIME * 1000;
    g_cpuTspConnStatusSync.timeCount = 0;
    g_cpuTspConnStatusSync.validity = 0;

    // g_gsensorState.state = 1;
    // g_gsensorState.failureTime = CPU_INFO_FAILURE_TIME*1000;
    // g_gsensorState.timeCount = 0;
    // g_gsensorState.validity = 0;

    return 0;
}

/*************************************************
  Function:       StateSyncSdkCycleProcess
  Description:    状态同步模块周期调用接口
  Input:          pRxData：传入接收到的CPU数据
                  rxLength：数据长度
  Output:         无
  Return:         无
  Others:
*************************************************/
void StateSyncSdkCycleProcess(MpuHalDataPack_t *msgData)
{    
    if (msgData != NULL && msgData->aid == 0x01)
    {
        // TBOX_PRINT("State sync : aid %d ,mid %d, subcommond %d\r\n",msgData->aid,msgData->mid,(msgData->subcommand & 0x7F));
        if (msgData->mid == E_STATE_SYNC_LOCALTION_INFO_MID)
        {
            if (msgData->dataLength == 41)
            {
                g_cpuLocationInfo.locationInfo.moduleState = msgData->pDataBuffer[0];
                g_cpuLocationInfo.locationInfo.wireState = msgData->pDataBuffer[1];
                g_cpuLocationInfo.locationInfo.locationState = msgData->pDataBuffer[2];
                g_cpuLocationInfo.locationInfo.longitude = msgData->pDataBuffer[3] << 24 | msgData->pDataBuffer[4] << 16 | msgData->pDataBuffer[5] << 8 | msgData->pDataBuffer[6];
                g_cpuLocationInfo.locationInfo.latitude = msgData->pDataBuffer[7] << 24 | msgData->pDataBuffer[8] << 16 | msgData->pDataBuffer[9] << 8 | msgData->pDataBuffer[10];
                for (uint8_t i = 0; i < 8; i++)
                {
                    g_cpuLocationInfo.locationInfo.flongitude[i] = msgData->pDataBuffer[11 + i];
                    g_cpuLocationInfo.locationInfo.flatitude[i] = msgData->pDataBuffer[19 + i];
                }
                g_cpuLocationInfo.locationInfo.altitude = msgData->pDataBuffer[27] << 8 | msgData->pDataBuffer[28];
                g_cpuLocationInfo.locationInfo.speed = msgData->pDataBuffer[29] << 8 | msgData->pDataBuffer[30];
                g_cpuLocationInfo.locationInfo.heading = msgData->pDataBuffer[31] << 8 | msgData->pDataBuffer[32];
                g_cpuLocationInfo.locationInfo.accuracy = msgData->pDataBuffer[33] << 8 | msgData->pDataBuffer[34];
                g_cpuLocationInfo.locationInfo.svsNum = msgData->pDataBuffer[35];
                g_cpuLocationInfo.locationInfo.useSvsnum = msgData->pDataBuffer[36];
                g_cpuLocationInfo.locationInfo.timeStamp = msgData->pDataBuffer[37] << 24 | msgData->pDataBuffer[38] << 16 | msgData->pDataBuffer[39] << 8 | msgData->pDataBuffer[40];

                g_cpuLocationInfo.timeCount = 0;
                g_cpuLocationInfo.validity = 1;
            }
        }
        else if (msgData->mid == E_STATE_SYNC_NET_INFO_MID)
        {
            if (msgData->dataLength == 5)
            {
                g_cpuNetInfo.netInfo.creg = msgData->pDataBuffer[0];
                g_cpuNetInfo.netInfo.csq = msgData->pDataBuffer[1];
                g_cpuNetInfo.netInfo.netType = msgData->pDataBuffer[2];
                g_cpuNetInfo.netInfo.apnCounter = msgData->pDataBuffer[3];
                g_cpuNetInfo.netInfo.apnStatus = msgData->pDataBuffer[4];
                g_cpuNetInfo.netInfo.mainAntennaStatus = msgData->pDataBuffer[5] & 0x0F;
                g_cpuNetInfo.netInfo.slaveAntennaStatus = (msgData->pDataBuffer[5] & 0xF0) >> 4;

                g_cpuNetInfo.timeCount = 0;
                g_cpuNetInfo.validity = 1;
            }
        }
        else if (msgData->mid == E_STATE_SYNC_HAL_STATE_MID)
        {
            if (msgData->dataLength == 4)
            {
                g_cpuHalState.halState.MemUsageStatus = msgData->pDataBuffer[0];
                g_cpuHalState.halState.cpuUsage = msgData->pDataBuffer[1];
                g_cpuHalState.halState.tmpStatus = msgData->pDataBuffer[2] << 8 + msgData->pDataBuffer[3];

                g_cpuHalState.timeCount = 0;
                g_cpuHalState.validity = 1;
            }
        }
        else if (msgData->mid == E_STATE_SYNC_TSP_CONNECT_STATE_MID)
        {

            g_cpuTspState.tspState.tspStatus = msgData->pDataBuffer[0];

            g_cpuTspState.timeCount = 0;
            g_cpuTspState.validity = 1;
        }
        else if ((msgData->mid == E_STATE_SYNC_ALL_FTY_DATA_MID))
        {

            if ((msgData->subcommand & 0x7F) == 0x02)
            {                
                if (msgData->dataLength == sizeof(ftyCircleDataToMcu_t))
                {
                    // TimeService_t Beijing_Time;
                    // if (0 == TimeSyncSdkGetRealTime(0, &Beijing_Time))
                    // {
                    //     TBOX_PRINT("State sync : Beijing_Time %04d-%02d-%02d %02d:%02d:%02d\r\n",
                    //                Beijing_Time.year,
                    //                Beijing_Time.month,
                    //                Beijing_Time.day,
                    //                Beijing_Time.hour,
                    //                Beijing_Time.minute,
                    //                Beijing_Time.second);
                    // }
                    // TimerHal_GetRtcTime(&g_cpuFtyDataSync.payload.timeData);
                    TimerHal_RtcRemoteTimeSync(&g_cpuFtyDataSync.payload.timeData);
                    
                    memcpy(&g_cpuFtyDataSync.payload, msgData->pDataBuffer, sizeof(ftyCircleDataToMcu_t));
                    g_cpuFtyDataSync.validity = 1;
                    g_cpuFtyDataSync.timeCount = 0;

                    const DtcTable_t *dtcTable = &g_cpuFtyDataSync.payload.dtc;

                    /* ================== 1. 内部存储器错误 ================== */
                    if (dtcTable->storeState == 1 || dtcTable->storeState == 2)
                    {
                        SetDtcFaultState(E_DTC_ITEM_MEMORY_FAILURE); // 内部数据存储器故障
                    }
                    else
                    {
                        ClearDtcFaultState(E_DTC_ITEM_MEMORY_FAILURE);
                        // TBOX_PRINT("Memory Failure DTC clear\r\n");
                    }

                    /* ================== 2. 主天线故障 ================== */
                    if (dtcTable->antMain == 1)
                        SetDtcFaultState(E_DTC_ITEM_MPU_4G_MAIN_ANT_OPEN); // 主天线开路
                    else
                        ClearDtcFaultState(E_DTC_ITEM_MPU_4G_MAIN_ANT_OPEN);

                    if (dtcTable->antMain == 2)
                        SetDtcFaultState(E_DTC_ITEM_MPU_4G_MAIN_ANT_SHORT_BAT); // 主天线短路到电源
                    else
                        ClearDtcFaultState(E_DTC_ITEM_MPU_4G_MAIN_ANT_SHORT_BAT);

                    if (dtcTable->antMain == 3)
                        SetDtcFaultState(E_DTC_ITEM_MPU_4G_MAIN_ANT_SHORT_GND); // 主天线短路到地
                    else
                        ClearDtcFaultState(E_DTC_ITEM_MPU_4G_MAIN_ANT_SHORT_GND);

                    /* ================== 3. 副天线故障 ================== */
                    if (dtcTable->antSec == 1)
                        SetDtcFaultState(E_DTC_ITEM_MPU_4G_AUX_ANT_OPEN); // 副天线开路
                    else
                        ClearDtcFaultState(E_DTC_ITEM_MPU_4G_AUX_ANT_OPEN);

                    if (dtcTable->antSec == 2)
                        SetDtcFaultState(E_DTC_ITEM_MPU_4G_AUX_ANT_SHORT_BAT); // 副天线短路到电源
                    else
                        ClearDtcFaultState(E_DTC_ITEM_MPU_4G_AUX_ANT_SHORT_BAT);

                    if (dtcTable->antSec == 3)
                        SetDtcFaultState(E_DTC_ITEM_MPU_4G_AUX_ANT_SHORT_GND); // 副天线短路到地
                    else
                        ClearDtcFaultState(E_DTC_ITEM_MPU_4G_AUX_ANT_SHORT_GND);

                    /* ================== 4. SIM卡故障 ================== */
                    if (dtcTable->simFault)
                        SetDtcFaultState(E_DTC_ITEM_MPU_SIM_FAULT);
                    else
                        ClearDtcFaultState(E_DTC_ITEM_MPU_SIM_FAULT);

                    /* ================== 5. 4G模块内部通讯故障 ================== */
                    if (dtcTable->lteFault)
                        SetDtcFaultState(E_DTC_ITEM_MPU_4G_INTERNAL_COMM_FAULT);
                    else
                        ClearDtcFaultState(E_DTC_ITEM_MPU_4G_INTERNAL_COMM_FAULT);

                    /* ================== 6. 以太网相关故障 ================== */
                    if (dtcTable->ethState == 1 || dtcTable->ethState == 2 || dtcTable->ethState == 3 || dtcTable->ethState == 4)
                        SetDtcFaultState(E_DTC_ITEM_MPU_ETH_LINE_FAULT); // 初始化失败/链路失败/SMI控制失败/4.以太网通讯错误
                    else
                        ClearDtcFaultState(E_DTC_ITEM_MPU_ETH_LINE_FAULT);

                    if (dtcTable->ethState == 4)
                        SetDtcFaultState(E_DTC_ITEM_MPU_ETH_COMM_FAULT); // 通信错误
                    else
                        ClearDtcFaultState(E_DTC_ITEM_MPU_ETH_COMM_FAULT);

                    // /* ================== 7. DDR 故障 ================== */
                    // if (dtcTable->ddrState == 1)
                    //     SetDtcFaultState(E_DTC_ITEM_MPU_DDR_FAULT);
                    // else
                    //     ClearDtcFaultState(E_DTC_ITEM_MPU_DDR_FAULT);

                    // /* ================== 8. MPU过温 ================== */
                    // if (dtcTable->overTemperature == 1)
                    //     SetDtcFaultState(E_DTC_ITEM_MPU_OPENCPU_OVER_TEMP);
                    // else
                    //     ClearDtcFaultState(E_DTC_ITEM_MPU_OPENCPU_OVER_TEMP);
                }
            }
        }
    }
    // TimeSevice_t Beijing_Time;
    // if (0 == TimeSyncSdkGetRealTime(0, &Beijing_Time))
    // {
    //     TBOX_PRINT("State sync : Beijing_Time %04d-%02d-%02d %02d:%02d:%02d\r\n",
    //                Beijing_Time.year,
    //                Beijing_Time.month,
    //                Beijing_Time.day,
    //                Beijing_Time.hour,
    //                Beijing_Time.minute,
    //                Beijing_Time.second);
    // }
    if (g_processCycleTime == 0)
    {
        return;
    }
    if ((g_cpuLocationInfo.validity == 1) && (g_cpuLocationInfo.timeCount++ > (g_cpuLocationInfo.failureTime) / g_processCycleTime))
    {
        g_cpuLocationInfo.validity = 0;
    }

    if ((g_cpuNetInfo.validity == 1) && (g_cpuNetInfo.timeCount++ > (g_cpuNetInfo.failureTime) / g_processCycleTime))
    {
        g_cpuNetInfo.validity = 0;
    }

    if ((g_cpuHalState.validity == 1) && (g_cpuHalState.timeCount++ > (g_cpuHalState.failureTime) / g_processCycleTime))
    {
        g_cpuHalState.validity = 0;
    }

    if ((g_cpuTspState.validity == 1) && (g_cpuTspState.timeCount++ > (g_cpuTspState.failureTime) / g_processCycleTime))
    {
        g_cpuTspState.validity = 0;
    }
    // 0xB24C_cxl 0xB24D_cxl
    if ((g_cpu4gModuleInfo.validity == 1) && (g_cpu4gModuleInfo.timeCount++ > (g_cpu4gModuleInfo.failureTime) / g_processCycleTime))
    {
        g_cpu4gModuleInfo.validity = 0;
    }
    // 0xB256_cxl
    if ((g_cpuEthernetId.validity == 1) && (g_cpuEthernetId.timeCount++ > (g_cpuEthernetId.failureTime) / g_processCycleTime))
    {
        g_cpuEthernetId.validity = 0;
    }
    // 0xB259_cxl
    if ((g_cpuEmmcDeleteStatus.validity == 1) && (g_cpuEmmcDeleteStatus.timeCount++ > (g_cpuEmmcDeleteStatus.failureTime) / g_processCycleTime))
    {
        g_cpuEmmcDeleteStatus.validity = 0;
    }
    // 0xB25B_cxl
    if ((g_cpu4gAppVersion.validity == 1) && (g_cpu4gAppVersion.timeCount++ > (g_cpu4gAppVersion.failureTime) / g_processCycleTime))
    {
        g_cpu4gAppVersion.validity = 0;
    }
    // 0xB262_cxl
    if ((g_cpuPkiCertStatus.validity == 1) && (g_cpuPkiCertStatus.timeCount++ > (g_cpuPkiCertStatus.failureTime) / g_processCycleTime))
    {
        g_cpuPkiCertStatus.validity = 0;
    }
    // 0xB28E_cxl
    if ((g_cpuTspConnStatusSync.validity == 1) && (g_cpuTspConnStatusSync.timeCount++ > (g_cpuTspConnStatusSync.failureTime) / g_processCycleTime))
    {
        g_cpuTspConnStatusSync.validity = 0;
    }
    // dtc_cxl
    if ((g_cpuFtyDataSync.validity == 1) && (g_cpuFtyDataSync.timeCount++ > (g_cpuFtyDataSync.failureTime / g_processCycleTime)))
    {
        g_cpuFtyDataSync.validity = 0;
    }
    return;
}

/*************************************************
  Function:       StateSyncGetLocationInfo
  Description:    获取CPU定位信息接口
  Input:          无
  Output:         localtioninfo：定位信息
  Return:         无
  Others:
*************************************************/
int16_t StateSyncGetLocationInfo(LocationInfoSync_t *localtionInfo)
{
    uint8_t *pSrc = NULL;
    uint8_t *pDes = NULL;
    uint8_t i = 0;

    if (localtionInfo == NULL)
    {
        return -1;
    }

    if (g_cpuLocationInfo.validity == 0)
    {
        return -1;
    }

    pSrc = (uint8_t *)&(g_cpuLocationInfo.locationInfo);
    pDes = (uint8_t *)localtionInfo;

    for (i = 0; i < sizeof(LocationInfoSync_t); i++)
    {
        pDes[i] = pSrc[i];
    }

    return 0;
}

/*************************************************
  Function:       StateSyncGetNetInfo
  Description:    获取cpu网络信息接口
  Input:          无
  Output:         netInfo：cpu网络信息
  Return:         无
  Others:
*************************************************/
int16_t StateSyncGetNetInfo(NetInfoSync_t *netInfo)
{
    uint8_t *pSrc = NULL;
    uint8_t *pDes = NULL;
    uint8_t i = 0;

    if (netInfo == NULL)
    {
        return -1;
    }

    if (g_cpuNetInfo.validity == 0)
    {
        return -1;
    }

    pSrc = (uint8_t *)&g_cpuNetInfo.netInfo;
    pDes = (uint8_t *)netInfo;

    for (i = 0; i < sizeof(NetInfoSync_t); i++)
    {
        pDes[i] = pSrc[i];
    }

    return 0;
}

/*************************************************
  Function:       StateSyncGetHalstate
  Description:    获取cpu硬件状态
  Input:          无
  Output:         halState：cpu硬件状态
  Return:         无
  Others:
*************************************************/
int16_t StateSyncGetHalstate(HalStateSync_t *halState)
{
    uint8_t *pSrc = NULL;
    uint8_t *pDes = NULL;
    uint8_t i = 0;

    if (halState == NULL)
    {
        return -1;
    }

    if (g_cpuHalState.validity == 0)
    {
        return -1;
    }

    pSrc = (uint8_t *)&g_cpuHalState.halState;
    pDes = (uint8_t *)halState;

    for (i = 0; i < sizeof(HalStateSync_t); i++)
    {
        pDes[i] = pSrc[i];
    }

    return 0;
}

/*************************************************
  Function:       StateSyncGetTspState
  Description:    获取远程平台连接状态
  Input:          无
  Output:         tspState：平台连接状态
  Return:         无
  Others:
*************************************************/
int16_t StateSyncGetTspState(TspStateSync_t *tspState)
{
    if (tspState == NULL)
    {
        return -1;
    }

    if (g_cpuTspState.validity == 0)
    {
        return -1;
    }

    tspState->tspStatus = g_cpuTspState.tspState.tspStatus;

    return 0;
}

int16_t StateSyncGet4gModuleInfo(ModuleInfo4GPayload_t *pModuleInfo) // 0xB24C_cxl 0xB24D_cxl
{
    if ((pModuleInfo == NULL) || (g_cpu4gModuleInfo.validity == 0))
    {
        return -1;
    }

    memcpy(pModuleInfo, &g_cpu4gModuleInfo.moduleInfo, sizeof(ModuleInfo4GPayload_t));
    return 0;
}

int16_t StateSyncGetEthernetId(EthernetIdPayload_t *pEthId) // 0xB256_cxl
{
    if (pEthId == NULL)
    {
        return -1;
    }
    if (g_cpuEthernetId.validity == 0)
    {
        return -1;
    }
    memcpy(pEthId, &g_cpuEthernetId.ethId, sizeof(EthernetIdPayload_t));
    return 0;
}

/*************************************************
  Function:       StateSyncSendEmmcDeleteCommand
  Description:    发送EMMC删除命令到MPU (0xB259_cxl)
*************************************************/
// int16_t StateSyncSendEmmcDeleteCommand(uint8_t command)
// {
//     MpuHalDataPack_t txPack;
//     uint8_t dataBuffer[1];

//     if (g_mpuHandle_stateSync < 0)
//     {
//         return -1;
//     }

//     dataBuffer[0] = command;

//     txPack.aid = 0x01;                                // 与MPU约定的AID
//     txPack.mid = E_STATE_SYNC_EMMC_DELETE_STATUS_MID; // 复用MID作为命令通道
//     txPack.subcommand = 0x01;                         // 子命令
//     txPack.pDataBuffer = dataBuffer;
//     txPack.dataLength = 1;

//     return MpuHalTransmit(g_mpuHandle_stateSync, &txPack, MPU_HAL_UART_MODE);
// }

/*************************************************
  Function:       StateSyncGetEmmcDeleteStatus
  Description:    获取EMMC删除状态 (0xB259_cxl)
*************************************************/
int16_t StateSyncGetEmmcDeleteStatus(EmmcDeleteStatusPayload_t *pDeleteStatus)
{
    if (pDeleteStatus == NULL)
    {
        return -1;
    }
    if (g_cpuEmmcDeleteStatus.validity == 0)
    {
        return -1;
    }
    memcpy(pDeleteStatus, &g_cpuEmmcDeleteStatus.deleteStatus, sizeof(EmmcDeleteStatusPayload_t));
    return 0;
}
int16_t StateSyncGet4gAppVersion(AppVersion4GPayload_t *pAppVersion) // 0xB25B_cxl
{
    if (pAppVersion == NULL)
    {
        return -1;
    }
    if (g_cpu4gAppVersion.validity == 0)
    {
        return -1;
    }
    memcpy(pAppVersion, &g_cpu4gAppVersion.appVersion, sizeof(AppVersion4GPayload_t));
    return 0;
}
// 0xB262_cxl
int16_t StateSyncGetPkiCertStatus(PkiCertStatusPayload_t *pPkiStatus)
{
    if (pPkiStatus == NULL)
    {
        return -1;
    }
    if (g_cpuPkiCertStatus.validity == 0)
    {
        return -1;
    }
    memcpy(pPkiStatus, &g_cpuPkiCertStatus.pkiStatus, sizeof(PkiCertStatusPayload_t));
    return 0;
}
// 0xB28E_cxl
int16_t StateSyncGetTspConnectionStatus(TspConnectionStatusPayload_t *pStatus)
{
    if (pStatus == NULL)
    {
        return -1;
    }

    if (g_cpuTspConnStatusSync.validity == 1)
    {
        *pStatus = g_cpuTspConnStatusSync.pkiStatus;
        return 0;
    }
    return -1;
}
const ftyCircleDataToMcu_t *StateSyncGetFtyData(void)
{
    if (g_cpuFtyDataSync.validity == 1)
    {
        return &g_cpuFtyDataSync.payload;
    }
    else
    {
        return NULL;
    }
}
#if 0
/*************************************************
  Function:       StateSyncGetGsersorState
  Description:    获取G-Sensor状态
  Input:          无
  Output:         state：状态 0正常，1错误
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t StateSyncGetGsensorState(uint8_t *state)
{
    if(state == NULL)
    {
        return -1;
    }
    if(g_gsensorState.validity == 0)
    {
        *state = 1;
        return -1;
    }
    *state = g_gsensorState.state;
    return 0;
    
}
#endif
