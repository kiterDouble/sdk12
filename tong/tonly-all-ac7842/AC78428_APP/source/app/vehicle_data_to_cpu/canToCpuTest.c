/*************************************************
 Copyright (C) 2025 SiRun (AnHui). All rights reserved.
 File Name: canToCpuTest.c
 Author:
 Created Time:
 Description: Test helpers for 4-channel CAN data collection.
              Called from TaskVehicleDataToCpu when TASK_TEST_ENABLE=1.
              Simulates 4-channel CAN frames and forwards them to SOC
              via SPI transparent transmission.
 Others:
 *************************************************/

/****************************** include ***************************************/
#include "canToCpuTest.h"
#include "canMsgToCpu.h"
#include "canMsgSpiTransmit.h"
#include "mpuHal.h"
#include "logHal.h"
#include "peripheralHal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

/****************************** Macro Definitions ******************************/
#define TEST_CHANNEL_NUM                (4)
#define TEST_FRAMES_PER_CHANNEL         (5)
#define TEST_CYCLE_TIME_MS              (5)

/****************************** Global Variables ******************************/
static uint32_t g_testTotalFrames = 0;
static uint32_t g_testTotalCycles = 0;
static uint32_t g_testVerifyErrors = 0;
static uint32_t g_testLastPrintTick = 0;
static int16_t  g_testMpuHandle = -1;
static uint8_t  g_testMpuRxBuffer[100];

/****************************** Function Declarations *************************/
static void SimulateMultiChannelCanReceive(void);

/****************************** Private Function Implementations ***************/

static void SimulateMultiChannelCanReceive(void)
{
    CanHalMsg_t simMsg;
    uint8_t ch;
    uint8_t i;
    uint32_t tickMs;

    for(ch = 0; ch < TEST_CHANNEL_NUM; ch++)
    {
        for(i = 0; i < TEST_FRAMES_PER_CHANNEL; i++)
        {
            memset(&simMsg, 0, sizeof(simMsg));

            if((i & 0x01) == 0)
            {
                simMsg.canId = 0x100 + (ch * 0x10) + i;
            }
            else
            {
                simMsg.canId = 0x80000000u | (0x18F10000u + (ch << 8) + i);
            }

            simMsg.dlc = ((i & 0x01) == 0) ? 8 : 64;

            {
                uint8_t d;
                for(d = 0; d < simMsg.dlc; d++)
                {
                    simMsg.canData[d] = (uint8_t)(ch * 0x40 + i * 0x10 + d);
                }
            }

            tickMs = (uint32_t)xTaskGetTickCount();

            if(CanFramePush(ch, &simMsg, tickMs, 0u) == 0)
            {
                g_testTotalFrames++;
            }
        }
    }
}

/****************************** Public Function Implementations ***************/

void CanToCpuTestCycle(void)
{
    CanFrameDualBuf_t *pDual;
    uint32_t nowTick;

    g_testTotalCycles++;

    SimulateMultiChannelCanReceive();

    CanMsgSpiTransmitCycleProcess(TEST_CYCLE_TIME_MS);

    pDual = CanFrameDualBufGet();

    nowTick = xTaskGetTickCount();
    if((nowTick - g_testLastPrintTick) >= 2000)
    {
        g_testLastPrintTick = nowTick;
        TBOX_PRINT("CanToCpuTest cycle %d: frames=%d spiSent=%d errors=%d drops=%d\r\n",
                   g_testTotalCycles,
                   g_testTotalFrames,
                   CanMsgSpiTransmitGetSentBytes(),
                   g_testVerifyErrors,
                   pDual->frameDropCount);
    }
}

void CanToCpuTestInit(void)
{
    MpuHalFilter_t mpuFilter;

    CanMsgTansmitToCpuInitialize();
    g_testMpuHandle = MpuHalOpen();
    if(g_testMpuHandle >= 0)
    {
        mpuFilter.aid = 0x02;
        mpuFilter.midMin = 0x01;
        mpuFilter.midMax = 0x02;
        MpuHalSetRxFilter(g_testMpuHandle, &mpuFilter);
        MpuHalSetRxBuffer(g_testMpuHandle, g_testMpuRxBuffer, sizeof(g_testMpuRxBuffer));
        CanMsgSpiTransmitInitialize(g_testMpuHandle);
    }

    g_testTotalFrames = 0;
    g_testTotalCycles = 0;
    g_testVerifyErrors = 0;
    g_testLastPrintTick = 0;
    TBOX_PRINT("=== CAN-to-CPU SPI Forward Test Start ===\r\n");
    TBOX_PRINT("Simulating %d channels x %d frames/cycle, forwarding to SOC via SPI\r\n",
               TEST_CHANNEL_NUM, TEST_FRAMES_PER_CHANNEL);
}
