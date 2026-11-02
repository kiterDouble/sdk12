
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "string.h"
#include "logHal.h"
#include "peripheralHal.h"
#include "batteryHal.h"
#include "gSensorHal.h"
#include "batterySdk.h"
#include "mpuPowerSyncSdk.h"
#include "taskDiagnostic.h"
#define SYNC_GSENSOR_ENABLE 0

static uint16_t g_processCycleTime = 0; // 保存StateSyncSdkCycleProcess接口调用周期，单位 ms
static int16_t g_mpuHandle = 0;         // 保存同步任务的CPU通信句柄
// static int16_t	g_powerManageHandle = -1;         //电源管理状态句柄
// static uint8_t g_sleepState = 0;                 //休眠状态，0：休眠1：唤醒

static uint8_t g_rkMode = 0;    // 功能模式  0：Init  1:正常模式  2：运输模式  3：生产模式
static uint8_t g_sleepMode = 0; // 休眠状态：0：正常模式 1：正常休眠模式 2：深度休眠模式

static uint8_t g_packData[64] = {0};
static MpuHalDataPack_t g_powerSyncPack; // mpu通信数据包

static uint8_t g_reqIndex = 0;        // 请求cpu休眠计数
static uint8_t g_mpuSleepAckFlag = 0; // MPU休眠响应标记  0：未响应 1：响应

static SemaphoreHandle_t g_mutexHandle = NULL; // 互斥锁句柄
static uint8_t g_sleepDisableEvent = 0;        // 0 :无阻止休眠事件 其它：休眠事件

static uint8_t g_mpuKeepAliveLoseFlag = 1; // mpu 心跳丢失标记 0：未丢失，1：丢失

static MpuErrorCbFun_t g_mpuErrorCallBackFunc = NULL;

typedef struct
{
    uint8_t setFlag;     // 0:没有设置  1：需要设置
    uint8_t setData;     // 设置数据
    uint8_t successFlag; // 设置成功标记
} WakeUpSourceSet_t;

static WakeUpSourceSet_t g_wakeUpSource; // 设置唤醒源

static int16_t MpuPowerSyncSdkRequstSleep(uint8_t sleepFlag)
{
    if (g_mpuHandle < 0)
    {
        return -1;
    }

    g_powerSyncPack.aid = 0x01;
    g_powerSyncPack.mid = 0x04;
    g_powerSyncPack.subcommand = 0x01;

    memset(g_packData, 0, sizeof(g_packData));

    g_powerSyncPack.dataBufferSize = sizeof(g_packData);
    if (g_reqIndex == 0xFF)
    {
        g_reqIndex = 0;
    }
    else
    {
        g_reqIndex++;
    }
    g_packData[0] = g_reqIndex;
    g_packData[1] = sleepFlag;

    g_powerSyncPack.dataLength = 2;
    g_powerSyncPack.pDataBuffer = g_packData;

    MpuHalTransmit(g_mpuHandle, &g_powerSyncPack, MPU_HAL_UART_MODE);
    return 0;
}

static int16_t MpuPowerSyncSdkSendWakeSource(uint8_t wakeupSource)
{
    if (g_mpuHandle < 0)
    {
        return -1;
    }
    g_powerSyncPack.aid = 0x01;
    g_powerSyncPack.mid = 0x04;
    g_powerSyncPack.subcommand = 0x04;

    memset(g_packData, 0, sizeof(g_packData));

    g_powerSyncPack.dataBufferSize = sizeof(g_packData);

    g_packData[0] = wakeupSource;

    g_powerSyncPack.pDataBuffer = g_packData;
    g_powerSyncPack.dataLength = 1;

    MpuHalTransmit(g_mpuHandle, &g_powerSyncPack, MPU_HAL_UART_MODE);

    return 0;
}

static int16_t MpuPowerSyncSdkSendPowerHalState(void)
{
    uint32_t voltage = 0;
    if (g_mpuHandle < 0)
    {
        return -1;
    }
    g_powerSyncPack.aid = 0x01;
    g_powerSyncPack.mid = 0x01;
    g_powerSyncPack.subcommand = 0x7F;

    memset(g_packData, 0, sizeof(g_packData));

    g_powerSyncPack.dataBufferSize = sizeof(g_packData);

    g_packData[0] = PeripheralHalGetKl15Status() + 1; // KL15状态
    // g_packData[1] = g_sleepMode;
    PeripheralHalAdGet(AD_CHANNEL_KL30, (uint32_t *)&voltage); // KL30电压
    g_packData[1] = (voltage >> 24) & 0xFF;
    g_packData[2] = (voltage >> 16) & 0xFF;
    g_packData[3] = (voltage >> 8) & 0xFF;
    g_packData[4] = (voltage) & 0xFF;
    g_packData[5] = 0;
    g_packData[6] = 0;
    g_packData[7] = GetTesterPresenceStatus();

    g_powerSyncPack.pDataBuffer = g_packData;
    g_powerSyncPack.dataLength = 8;

    MpuHalTransmit(g_mpuHandle, &g_powerSyncPack, MPU_HAL_UART_MODE);
    // TBOX_PRINT("MpuPowerSyncSdkSendPowerHalState: KL15=%d, KL30=%lu mV, TesterPresent=%d\r\n",
    //            g_packData[0], voltage, g_packData[7]);
    return 0;
}

static int16_t MpuPowerSyncSdkSendBatteryState(void)
{
    uint32_t voltage = 0;
    int32_t temp = 0;
    uint32_t chargingDuration = 0;
    if (g_mpuHandle < 0)
    {
        return -1;
    }
    g_powerSyncPack.aid = 0x01;
    g_powerSyncPack.mid = 0x15;
    g_powerSyncPack.subcommand = 0x7F;

    memset(g_packData, 0, sizeof(g_packData));

    g_powerSyncPack.dataBufferSize = sizeof(g_packData);

    g_packData[0] = BatteryHalGetState() & 0xFF;

    BatterySdkGetVoltage(&voltage);
    BatterySdkGetTemperature(&temp);
    temp = (temp / 100) + 400;
    g_packData[1] = (voltage >> 8) & 0xFF;
    g_packData[2] = voltage & 0xFF;
    if (temp < 0)
    {
        g_packData[3] = 0;
        g_packData[4] = 0;
    }
    else
    {
        g_packData[3] = (temp >> 8) & 0xFF;
        g_packData[4] = temp & 0xFF;
    }
    g_packData[5] = BatterySdkGetBatteySoc() & 0xFF;
    g_packData[6] = BatterySdkGetBatteyVoltageAlarm();
    g_packData[7] = BatterySdkGetBatteyTempAlarm();
    g_packData[8] = BatterySdkGetBatteyStatus();
    chargingDuration = BatterySdkGetBatteyChargingDuration();
    g_packData[9] = (chargingDuration >> 24) & 0xFF;
    g_packData[10] = (chargingDuration >> 16) & 0xFF;
    g_packData[11] = (chargingDuration >> 8) & 0xFF;
    g_packData[12] = chargingDuration & 0xFF;

    g_powerSyncPack.pDataBuffer = g_packData;
    g_powerSyncPack.dataLength = 13;

    MpuHalTransmit(g_mpuHandle, &g_powerSyncPack, MPU_HAL_UART_MODE);

    return 0;
}

static int16_t MpuPowerSyncSdkSendPowerState(void)
{
    if (g_mpuHandle < 0)
    {
        return -1;
    }
    g_powerSyncPack.aid = 0x01;
    g_powerSyncPack.mid = 0x04;
    g_powerSyncPack.subcommand = 0x05;

    memset(g_packData, 0, sizeof(g_packData));

    g_powerSyncPack.dataBufferSize = sizeof(g_packData);

    xSemaphoreTake(g_mutexHandle, portMAX_DELAY);
    g_packData[0] = g_rkMode;
    g_packData[1] = g_sleepMode;
    xSemaphoreGive(g_mutexHandle);

    g_powerSyncPack.pDataBuffer = g_packData;
    g_powerSyncPack.dataLength = 2;

    MpuHalTransmit(g_mpuHandle, &g_powerSyncPack, MPU_HAL_UART_MODE);

    return 0;
}
#if (SYNC_GSENSOR_ENABLE == 1)
static int16_t MpuPowerSyncSdkSendGsensorState(void)
{
    GSensorHalData_t gSensorData;
    if (g_mpuHandle < 0)
    {
        return -1;
    }
    g_powerSyncPack.aid = 0x01;
    g_powerSyncPack.mid = 0x14;
    g_powerSyncPack.subcommand = 0x7F;

    memset(g_packData, 0, sizeof(g_packData));

    g_powerSyncPack.dataBufferSize = sizeof(g_packData);

    if (GSensorHalGetValue(&gSensorData) == 0)
    {
        g_packData[0] = 0;
        g_packData[1] = (gSensorData.accel_x + 30 * 1000) >= 0 ? (gSensorData.accel_x + 30 * 1000) : 0;
        g_packData[2] = (gSensorData.accel_y + 30 * 1000) >= 0 ? (gSensorData.accel_y + 30 * 1000) : 0;
        g_packData[3] = (gSensorData.accel_z + 30 * 1000) >= 0 ? (gSensorData.accel_z + 30 * 1000) : 0;
    }
    else
    {
        g_packData[0] = 1; // 错误
        g_packData[1] = 0;
        g_packData[2] = 0;
        g_packData[3] = 0;
    }

    g_powerSyncPack.pDataBuffer = g_packData;
    g_powerSyncPack.dataLength = 4;

    MpuHalTransmit(g_mpuHandle, &g_powerSyncPack, MPU_HAL_UART_MODE);

    return 0;
}
#endif
/*************************************************
  Function:       MpuPowerSyncSdkInit
  Description:    与MPU同步电源状态模块的初始化接口
  Input:          mpuHandle：mpu通信句柄
                  cycleTime：模块周期调用接口调用周期，>0
  Output:         无
  Return:         0:执行成功
                  -1：执行失败
  Others:
*************************************************/
int16_t MpuPowerSyncSdkInit(int16_t mpuHandle, uint16_t cycleTime)
{
    //    const uint8_t modulName[] = {'m','p','u','P','o','w','e','r','S','d','k'};
    if (mpuHandle < 0 || cycleTime == 0)
    {
        return -1;
    }
    g_mpuHandle = mpuHandle;
    g_processCycleTime = cycleTime;

    //    g_powerManageHandle = PowerManageSdkOpenHandle(modulName);

    g_wakeUpSource.setFlag = 0;

    g_mutexHandle = xSemaphoreCreateMutex();

    return 0;
}

/*************************************************
  Function:       MpuPowerSyncSdkCycleProcess
  Description:    mpu同步电源状态模块周期调用接口
  Input:          msgData：mpu的消息，没有接收到mpu消息可传NULL
  Output:         无
  Return:         无
  Others:
*************************************************/
void MpuPowerSyncSdkCycleProcess(MpuHalDataPack_t *msgData)
{
    static uint16_t cycleTimeCount = 1000;
    static uint16_t wakeSourceSyncTimeCount = 0;
    static uint32_t mpuKeepAliveTimeCount = 0;
    static uint8_t mpuErrorResetCount = 0;
    // g_sleepState=PowerManageSdkGetSleepState(g_sleepStateHandle);
    ////是否休眠状态
    // if(g_sleepState == 0)
    //{
    //     //获取休眠状态成功响应
    //     PowerManageSdkSetSleepAck();
    // }
    mpuKeepAliveTimeCount += 1;
    if (msgData != NULL)
    {
        // TBOX_PRINT("power sync : aid %d ,mid %d, subcommond %d\r\n", msgData->aid, msgData->mid, (msgData->subcommand & 0x7F));
        if (msgData->aid == 0x01 && msgData->mid == 0x04)
        {
            if ((msgData->subcommand & 0x7F) == 0x01) // 请求休眠指令响应
            {
                if (g_reqIndex == msgData->pDataBuffer[0])
                {
                    xSemaphoreTake(g_mutexHandle, portMAX_DELAY);
                    g_mpuSleepAckFlag = 1;
                    xSemaphoreGive(g_mutexHandle);
                    TBOX_PRINT("mpu sleep ack success\r\n");
                }
            }
            else if ((msgData->subcommand & 0x7F) == 0x04) // MPU响应mcu的唤醒源
            {
                xSemaphoreTake(g_mutexHandle, portMAX_DELAY);
                if (g_wakeUpSource.setFlag == 1)
                {
                    // TBOX_PRINT("recv wake source response\r\n");
                    if (g_wakeUpSource.setData == msgData->pDataBuffer[0])
                    {
                        g_wakeUpSource.setFlag = 0;
                        g_wakeUpSource.successFlag = 1;
                        wakeSourceSyncTimeCount = 0;
                    }
                }
                xSemaphoreGive(g_mutexHandle);
            }
            else if ((msgData->subcommand & 0x7F) == 0x06) // mpu同步阻止休眠事件到MCU
            {
                xSemaphoreTake(g_mutexHandle, portMAX_DELAY);
                g_sleepDisableEvent = msgData->pDataBuffer[0];
                xSemaphoreGive(g_mutexHandle);
                mpuKeepAliveTimeCount = 0;
                mpuErrorResetCount = 0;
                if (g_mpuKeepAliveLoseFlag)
                {
                    g_mpuKeepAliveLoseFlag = 0;
                }
            }
            else
            {
            }
        }
    }

    if (cycleTimeCount++ >= 1000 / g_processCycleTime) // 1秒
    {
        cycleTimeCount = 0;
        // 同步车辆硬线状态
        MpuPowerSyncSdkSendPowerHalState();
        // 同步备用电池状态
        MpuPowerSyncSdkSendBatteryState();
        // 设置电源模式
        MpuPowerSyncSdkSendPowerState();
#if (SYNC_GSENSOR_ENABLE == 1)
        // G-sensor状态同步
        MpuPowerSyncSdkSendGsensorState();
#endif
    }

    if (g_wakeUpSource.setFlag == 1)
    {
        if (wakeSourceSyncTimeCount++ >= (200 / g_processCycleTime))
        {
            TBOX_PRINT("pm mcu send wake source %d,g_wakeUpSource.setData\r\n");
            MpuPowerSyncSdkSendWakeSource(g_wakeUpSource.setData);
            wakeSourceSyncTimeCount = 0;
        }
    }

    // CPU心跳是否丢失
    if (mpuKeepAliveTimeCount >= ((5 * 60 * 1000) / g_processCycleTime))
    {
        g_sleepDisableEvent = 0; // 清除阻止休眠标记
        g_mpuKeepAliveLoseFlag = 1;
        if (mpuErrorResetCount < 3)
        {
            TBOX_PRINT("pm mcu reset mpu\r\n");
            MpuHalReset();
            mpuErrorResetCount += 1;
        }
        if (g_mpuErrorCallBackFunc != NULL)
        {
            g_mpuErrorCallBackFunc();
        }
        mpuKeepAliveTimeCount = 0;
    }
}

/*************************************************
  Function:       MpuPowerSyncSdkSetSleep
  Description:    设置mpu进入休眠状态
  Input:          deepSleepFlag：休眠状态标记 0:正常休眠 1:深度休眠
  Output:         无
  Return:         无
  Others:
*************************************************/
int16_t MpuPowerSyncSdkSetSleep(uint8_t deepSleepFlag)
{
    g_mpuSleepAckFlag = 0;
    return MpuPowerSyncSdkRequstSleep(deepSleepFlag);
}

/*************************************************
  Function:       MpuPowerSyncSdkGetSleepStatus
  Description:    获取MPU休眠完成状态
  Input:          无
  Output:         无
  Return:         休眠状态：0：已休眠 1：未休眠
  Others:
*************************************************/
int16_t MpuPowerSyncSdkGetSleepStatus(void)
{
    if (g_mpuSleepAckFlag)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*************************************************
  Function:       MpuPowerSyncSdkSetWake
  Description:    设置MPU唤醒状态
  Input:          wakeupSource:唤醒源
  Output:         无
  Return:         无
  Others:
*************************************************/
int16_t MpuPowerSyncSdkSetWake(uint8_t wakeupSource)
{
    xSemaphoreTake(g_mutexHandle, portMAX_DELAY);

    g_wakeUpSource.setFlag = 1;
    g_wakeUpSource.setData = wakeupSource;
    g_wakeUpSource.successFlag = 0;

    xSemaphoreGive(g_mutexHandle);
    return 0;
}

/*************************************************
  Function:       MpuPowerSyncSdkSetDeepSleepFlag
  Description:    设置电源状态为深度休眠
  Input:          deepSleepFlag：0:正常休眠 1:深度休眠
  Output:         无
  Return:         无
  Others:
*************************************************/
void MpuPowerSyncSdkSetDeepSleepFlag(uint8_t deepSleepFlag)
{
    if (deepSleepFlag == 0)
    {
        g_sleepMode = 1;
    }
    else if (deepSleepFlag == 1)
    {
        g_sleepMode = 2;
    }
}

/*************************************************
  Function:       MpuPowerSyncSdkGetWakeStatus
  Description:    获取唤醒源更新状态
  Input:          无
  Output:         pWakeSource：0：mpu更新完成  1：MPU更新未完成
  Return:         0:成功
                  -1：失败
  Others:
*************************************************/
int16_t MpuPowerSyncSdkGetWakeStatus(uint8_t *pWakeSource)
{
    if (g_wakeUpSource.successFlag == 0)
    {
        *pWakeSource = 1;
    }
    else
    {
        *pWakeSource = 0;
    }

    return 0;
}

/*************************************************
  Function:       MpuPowerSyncSdkGetSleepDisableState
  Description:    获取MPU禁止休眠状态
  Input:          无
  Output:         无
  Return:         0：无阻值休眠时间
                  非零值：有阻止休眠事件
  Others:
*************************************************/
int16_t MpuPowerSyncSdkGetSleepDisableState(void)
{
    uint8_t sleepDisableEvent = 0;

    xSemaphoreTake(g_mutexHandle, portMAX_DELAY);
    sleepDisableEvent = g_sleepDisableEvent;
    xSemaphoreGive(g_mutexHandle);

    return sleepDisableEvent;
}

/*************************************************
  Function:       MpuPowerSyncSdkSetRkMode
  Description:    设置上报给MPU的rkmode
  Input:          mode ：0:INIT  1：正常模式（全功能）2：运输模式 3：生产模式
  Output:         无
  Return:         0：设置成功
                  -1：设置失败
  Others:
*************************************************/
int16_t MpuPowerSyncSdkSetRkMode(uint8_t mode)
{
    xSemaphoreTake(g_mutexHandle, portMAX_DELAY);
    g_rkMode = mode;
    xSemaphoreGive(g_mutexHandle);
    return 0;
}

/*************************************************
  Function:       MpuPowerSyncSdkRegisteCb
  Description:    注册MPU通信异常回调接口
  Input:          mpuErrorFunc:MPU心跳丢失5分钟回调接口，会被周期调用
  Output:         无
  Return:         0：设置成功
                  -1：设置失败
  Others:
*************************************************/
int16_t MpuPowerSyncSdkRegisteMpuErrorCb(MpuErrorCbFun_t mpuErrorFunc)
{
    if (mpuErrorFunc == NULL)
    {
        return -1;
    }

    g_mpuErrorCallBackFunc = mpuErrorFunc;
    return 0;
}

/*************************************************
  Function:       MpuPowerSyncSdkGetNadModuleStatus
  Description:    获取nad 模块的运行状态
  Input:          无
  Output:         无
  Return:         0：正常
                  1：异常
  Others:
*************************************************/
uint8_t MpuPowerSyncSdkGetNadModuleStatus(void)
{
    return g_mpuKeepAliveLoseFlag;
}
