
#include "stdint.h"
#include "stddef.h"
#include "remoteDiagnosticSdk.h"
#include "remoteDiagnosticTask.h"
#include "virtualTpSdk.h"
#include "logHal.h"
#include "taskDiagnostic.h"
static int16_t RemoteUdsPreCondition(void);
static int16_t RemoteUdsCyleTestPresent(uint8_t stopFlag, uint8_t *pTxData, uint8_t txLength, uint32_t cycleTime_ms);

static UdsTpParameter_t g_canTpParameter =
    {
        .blockSize = 100, // application mode
        .STmin = 0,       // application mode
        .fillByte = 0x00,
        .N_As_enforced = 70,  // 70ms
        .N_Ar_enforced = 70,  // 70ms
        .N_Bs_enforced = 150, // 70ms
        .N_Br_enforced = 70,  // 70ms
        .N_Cs_enforced = 70,  // 70ms
        .N_Cr_enforced = 150, // 70ms
        .N_As_obd = 25,       // 25ms
        .N_Ar_obd = 25,       // 25ms
        .N_Bs_obd = 75,       // 70m
        .N_Br_obd = 0,        //(N_Br+N_Ar)ü25ms
        .N_Cs_obd = 0,        //(N_Cs+N_As) ü50ms
        .N_Cr_obd = 150,      //(N_Br+N_Ar)ü25ms
};

static EcuId_t g_ecuList[] = {
    /* ECU配置表：物理寻址requestId, 功能寻址functionalId, 响应responseId, CAN通道 */
    
    // VCU - 整车控制器 (CAN3动力)
    {0x7DC, 0x7DF, 0x7D4, TBOX_CAN_CHANNEL_3},
    
    // TMS - 机组A (CAN1整车)
    {0x67C, 0x0, 0x674, TBOX_CAN_CHANNEL_1},
    
    // TMS - 机组B (CAN1整车)
    {0x68C, 0x0, 0x684, TBOX_CAN_CHANNEL_1},
    
    // TCU - 变速箱控制器 (CAN3动力) - 扩展帧格式
    {0x18DA03F1, 0x18DBFFF1, 0x18DAF103, TBOX_CAN_CHANNEL_3},
    
    // MCU1系列 - TM在CAN3动力, 其他在CAN1整车
    // MCU1_TM1 (站号2)
    {0x18DAD2FA, 0x18DBFFFA, 0x18DAFAD2, TBOX_CAN_CHANNEL_3},
    // MCU1_TM2 (站号9)
    {0x18DAD9FA, 0x18DBFFFA, 0x18DAFAD9, TBOX_CAN_CHANNEL_3},
    // MCU1_PDU (站号6)
    {0x18DAD6FA, 0x18DBFFFA, 0x18DAFAD6, TBOX_CAN_CHANNEL_1},
    // MCU1_EPS (站号4)
    {0x18DAD4FA, 0x18DBFFFA, 0x18DAFAD4, TBOX_CAN_CHANNEL_1},
    // MCU1_ACM (站号5)
    {0x18DAD5FA, 0x18DBFFFA, 0x18DAFAD5, TBOX_CAN_CHANNEL_1},
    // MCU1_DCDC (站号7)
    {0x18DAD7FA, 0x18DBFFFA, 0x18DAFAD7, TBOX_CAN_CHANNEL_1},
    
    // MCU2系列 - TM在CAN3动力, 其他在CAN1整车
    // MCU2_TM1 (站号2)
    {0x18DAD2FB, 0x18DBFFFB, 0x18DAFBD2, TBOX_CAN_CHANNEL_3},
    // MCU2_TM2 (站号9)
    {0x18DAD9FB, 0x18DBFFFB, 0x18DAFBD9, TBOX_CAN_CHANNEL_3},
    // MCU2_PDU (站号6)
    {0x18DAD6FB, 0x18DBFFFB, 0x18DAFBD6, TBOX_CAN_CHANNEL_1},
    // MCU2_EPS (站号4)
    {0x18DAD4FB, 0x18DBFFFB, 0x18DAFBD4, TBOX_CAN_CHANNEL_1},
    // MCU2_ACM (站号5)
    {0x18DAD5FB, 0x18DBFFFB, 0x18DAFBD5, TBOX_CAN_CHANNEL_1},
    // MCU2_DCDC (站号7)
    {0x18DAD7FB, 0x18DBFFFB, 0x18DAFBD7, TBOX_CAN_CHANNEL_1},
    
    // MCU3系列 - 都在CAN3动力
    // MCU3_TM1 (站号2)
    {0x18DAD2FC, 0x18DBFFFC, 0x18DAFCD2, TBOX_CAN_CHANNEL_3},
    // MCU3_TM2 (站号9)
    {0x18DAD9FC, 0x18DBFFFC, 0x18DAFCD9, TBOX_CAN_CHANNEL_3},
    
};

static CanIdConfig_t g_ecuConfigure = {
    .functionalId = 0x7DF,
    .selfDiagnosticReqId = 0x72D,
    .selfDiagnosticRespId = 0x7AD,
    .ecuListSize = sizeof(g_ecuList) / sizeof(g_ecuList[0]),
    .pEcuList = g_ecuList,
    .pCheckPreconditionFun = RemoteUdsPreCondition,
    .pSetTestPresentFun = RemoteUdsCyleTestPresent,
};

static MpuBuffer_t g_mpuBuffer;

static uint8_t g_canChanl[] =
    {
        TBOX_CAN_CHANNEL_1,     
		TBOX_CAN_CHANNEL_3,
    };
static CanChanel_t g_canChanlList = {
    .canChanelList = g_canChanl,
    .canChanelListSize = sizeof(g_canChanl) / sizeof(g_canChanl[0]),
};

static int16_t RemoteUdsPreCondition(void)
{
    return 0;
}

static TestPreset_t g_canTestPresent;

static int16_t RemoteUdsCyleTestPresent(uint8_t stopFlag, uint8_t *pTxData, uint8_t txLength, uint32_t cycleTime_ms)
{
    uint8_t i;
    TestPreset_t *pTestPresentVarialble = NULL;
    pTestPresentVarialble = &g_canTestPresent;

    if (stopFlag)
    {
        pTestPresentVarialble->startFlag = 0;
    }
    else
    {
        if (txLength > 7)
        {
            // pTestPresentVarialble->txLength = 7;
            pTestPresentVarialble->txData[0] = 7;
        }
        else
        {
            pTestPresentVarialble->txData[0] = txLength;
            // pTestPresentVarialble->txLength = txLength;
        }
        pTestPresentVarialble->txLength = 0x08;
        for (i = 0; i < pTestPresentVarialble->txData[0]; i++)
        {
            pTestPresentVarialble->txData[i + 1] = pTxData[i];
        }
        for (; i < 7; i++)
        {
            pTestPresentVarialble->txData[i + 1] = g_canTpParameter.fillByte;
        }
        pTestPresentVarialble->cycleTime = cycleTime_ms;

        pTestPresentVarialble->startFlag = 1;
    }
    return 0;
}

/*************************************************
  Function:       remoteDiagnosticTaskMain
  Description:    远程诊断主任务接口
  Input:          无
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:
*************************************************/
void RemoteDiagnosticTaskMain(void *pvParameters)
{
    RemoteDiagnosticSdkProcess(&g_ecuConfigure, &g_mpuBuffer, &g_canTpParameter, &g_canChanlList);
}
