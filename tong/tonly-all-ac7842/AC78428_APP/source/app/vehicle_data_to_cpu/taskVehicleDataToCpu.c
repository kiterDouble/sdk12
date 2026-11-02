/*************************************************
 Copyright (C) 2025 SiRun (AnHui). All rights reserved.
 File Name: taskVehicleDataToCpu.c
 Author:
 Created Time:
 Description: FreeRTOS task for 4-channel CAN data
              collection and SPI transparent forwarding to SOC.
              CAN IDs to forward are subscribed by SOC via the
              MPU dynamic configure protocol.
 Others:
 *************************************************/

/****************************** include ***************************************/
#include "taskVehicleDataToCpu.h"
#include "vehicleSignalApp.h"
#include "taskManage.h"
#include "canHal.h"
#include "mpuHal.h"
#include "logHal.h"
#include "powerManageSdk.h"
#include "canMsgDynamicConfigure.h"
#include "ecuNodeMissingCheck.h"
#include "canMsgToCpu.h"
#include "canMsgSpiTransmit.h"
#include "canToCpuTest.h"
#include "FreeRTOS.h"
#include "task.h"

/****************************** Macro Definitions ******************************/
#define VEHICLE_TO_CPU_TASK_CYCLE_TIME       (5)
#define CAN_RX_BUFFER_SIZE_PER_CHANNEL       (20)
#define MPU_DRIVER_RX_BUFFER_SIZE            (300)
#define MPU_RX_DATA_BUFFER_SIZE              (100)
#define CAN_TO_CPU_CHANNEL_NUM               (4)
/****************************** Type Definitions ******************************/

/****************************** Global Variables ******************************/
static CanHalMsg_t g_canDriverRxBuffer[CAN_TO_CPU_CHANNEL_NUM][CAN_RX_BUFFER_SIZE_PER_CHANNEL];
static uint8_t g_mpuDriverRxBuffer[MPU_DRIVER_RX_BUFFER_SIZE];
static int16_t g_mpuHandle = -1;
static int16_t g_canHandle[CAN_TO_CPU_CHANNEL_NUM];
static uint8_t g_mpuRxDataBuffer[MPU_RX_DATA_BUFFER_SIZE];

/* CAN channel mapping: CAN1~CAN4 */
static const uint8_t g_canChannelMap[CAN_TO_CPU_CHANNEL_NUM] = {
    TBOX_CAN_CHANNEL_1,  /* 0 -> CAN1 */
    TBOX_CAN_CHANNEL_2,  /* 1 -> CAN2 */
    TBOX_CAN_CHANNEL_3,  /* 2 -> CAN3 */
    TBOX_CAN_CHANNEL_4,  /* 3 -> CAN4 */
};

/****************************** Function Declarations *************************/
static void MultiChannelCanInitialize(void);
static void MultiChannelCanReceive(void);

/****************************** Private Function Implementations ***************/

/* Initialize all 4 CAN channels with all-pass filter and RX buffers */
static void MultiChannelCanInitialize(void)
{
    CanHalMsgFilter_t allPassFilter;
    uint8_t i;

    allPassFilter.useStdFilter = 0x00;
    allPassFilter.canIdStdMin = 0x00;
    allPassFilter.canIdStdMax = 0xFFF;
    allPassFilter.useExdtFilter = 0x01;
    allPassFilter.canIdExtdMin = 0x00;
    allPassFilter.canIdExtdMax = 0xFFFFFFFF;

    for(i = 0; i < CAN_TO_CPU_CHANNEL_NUM; i++)
    {
        g_canHandle[i] = CanHalOpen(g_canChannelMap[i]);
        if(g_canHandle[i] < 0)
        {
            TBOX_PRINT("vehicle data to cpu: CAN%d handle open error\r\n", i + 1);
            continue;
        }
        CanHalSetFilter(g_canHandle[i], &allPassFilter);
        CanHalSetRxBuffer(g_canHandle[i], g_canDriverRxBuffer[i], CAN_RX_BUFFER_SIZE_PER_CHANNEL);
    }
}

/* Poll all 4 CAN channels for received messages, filter by
   subscription table, and push matched frames into the dual ring buffer. */
static void MultiChannelCanReceive(void)
{
    CanHalMsg_t canMsg;
    uint8_t ch;
    int16_t ret;
    uint32_t totalUs;
    uint32_t tsMs;
    uint32_t tsUs;
    uint32_t timestampTickHz;

    for(ch = 0; ch < CAN_TO_CPU_CHANNEL_NUM; ch++)
    {
        if(g_canHandle[ch] < 0)
        {
            continue;
        }
        timestampTickHz = CanHalGetTimestampTickHz(g_canHandle[ch]);
        while(1)
        {
            ret = CanHalReceive(g_canHandle[ch], &canMsg, 0);
            if(ret != 0)
            {
                break;
            }

            /* 1. local signal dispatch (existing path) */
            Can0RxCanMsgDispatch(canMsg.canId, canMsg.dlc, canMsg.canData);

            /* 2. ECU node missing check */
            EcuNodeMissingCheckCanId(g_canChannelMap[ch], canMsg.canId);
            if(ch == 0)
            {
                ReceiveSeedMessage(canMsg);
                ReceivePeriodMessage(canMsg);
            }
            /* 3. timestamp from AC7842x TSC (hardware-captured at frame EOF) */
            totalUs = CanHalHwTsToUs(canMsg.hwTimeStamp, timestampTickHz);
            tsMs = totalUs / 1000UL;
            tsUs = totalUs % 1000UL;

            /* 4. push to dual ring buffer (subscription-filtered) */
            CanFramePush(ch, &canMsg, tsMs, tsUs);
        }
    }
}

/****************************** Public Function Implementations ***************/

void TaskVehicleDataToCpu(void *pvParameters)
{
    uint16_t timeCount = 0U;
    uint32_t rtosTickCount = 0U;
    int16_t sleepFlag = 0U;
    int16_t lastSleepState = 0x01U;
    int16_t sleepCommandHandle = -1;
    int16_t ret = 0U;
    MpuHalFilter_t mpuFilter = {0U};
    MpuHalDataPack_t rxPack = {0U};

    (void)pvParameters;

#if(TASK_TEST_ENABLE == 1)
    CanToCpuTestInit();
    rtosTickCount = xTaskGetTickCount();
    while(1)
    {
        CanToCpuTestCycle();
        vTaskDelayUntil(&rtosTickCount, VEHICLE_TO_CPU_TASK_CYCLE_TIME);
    }
#else

    /* 1. Open MPU handle for SPI communication with SOC */
    g_mpuHandle = MpuHalOpen();
    mpuFilter.aid = 0x02;
    mpuFilter.midMin = 0x01;
    mpuFilter.midMax = 0x02;
    MpuHalSetRxFilter(g_mpuHandle, &mpuFilter);
    MpuHalSetRxBuffer(g_mpuHandle, g_mpuDriverRxBuffer, MPU_DRIVER_RX_BUFFER_SIZE);

    /* 2. Initialize 4 CAN channels */
    MultiChannelCanInitialize();

    /* 3. Initialize sub-modules */
    CanMsgTansmitToCpuInitialize();
    CanMsgDynamicConfigureInitialize(g_mpuHandle);
    CanMsgSpiTransmitInitialize(g_mpuHandle);

    /* 4. Power management */
    rtosTickCount = xTaskGetTickCount();
    sleepCommandHandle = PowerManageSdkOpenHandle("ToMpu");
    rxPack.pDataBuffer = g_mpuRxDataBuffer;
    rxPack.dataBufferSize = MPU_RX_DATA_BUFFER_SIZE;

    while(1)
    {
        /* CAN receive from all 4 channels */
        MultiChannelCanReceive();

        /* SPI batch transmit (time-window / threshold triggered) */
        CanMsgSpiTransmitCycleProcess(VEHICLE_TO_CPU_TASK_CYCLE_TIME);

        /* sleep management */
        sleepFlag = PowerManageSdkGetSleepState(sleepCommandHandle);
        if(sleepFlag == 0)
        {
            if(lastSleepState != sleepFlag)
            {
                /* flush remaining data before sleep */
                CanMsgSpiTransmitFlushAll();
                PowerManageSdkSetSleepAck(sleepCommandHandle);
            }
        }
        lastSleepState = sleepFlag;

        /* subscription config sync (every other cycle) */
        timeCount++;
        if((timeCount & 0x01) == 0x00)
        {
            rxPack.dataLength = 0;
            ret = MpuHalReceive(g_mpuHandle, &rxPack, 0);
            if(MPU_HAL_STATUS_OK == ret)
            {
                CanMsgToCpuConfigureSyncCycleProcess(&rxPack, 10);
            }
            else
            {
                CanMsgToCpuConfigureSyncCycleProcess(NULL, 10);
            }
        }

        /* ECU node missing check timeout */
        NodeMissingCheckTimeOut(VEHICLE_TO_CPU_TASK_CYCLE_TIME);
        Can0RxCanMsgCycleCheck(VEHICLE_TO_CPU_TASK_CYCLE_TIME);
        vTaskDelayUntil(&rtosTickCount, VEHICLE_TO_CPU_TASK_CYCLE_TIME);
    }
#endif
}

