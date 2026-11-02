
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "logHal.h"
#include "parameterSyncSdk.h"
#include "projectConfigure.h"
#include "parameterStoreManageApp.h"
#include "mcuMpuSyncTask.h"
#include <stdbool.h>


#define PARAM_SYNC_SUNBCOMMAND_GET_REQ 0x01     // 参数请求sunbcommand
#define PARAM_SYNC_SUNBCOMAND_GET_RES 0x02      // 参数请求响应sunbcommand
#define PARAM_SYNC_SUNBCOMMAND_SET_REQ 0x03     // 参数设置请求sunbcommand
#define PARAM_SYNC_SUNBCOMMAND_SET_RES 0x04     // 参数设置响应sunbcommand
#define SYNC_PARAM_TO_MPU_REQUST_COUNT_MAX 0x03 // Mcu请求同步参数到MPU重复请求最大次数

#define PARAM_SYNC_INDEX_SW_VERSION      9       // 内部版本号在同步列表中的索引

static int16_t g_mpuHandle = -1;                               // MPU通信句柄
static pMcuParametReadFun_t g_mcuParameterReadCbFunc = NULL;   // 提供写MCU参数函数
static pMcuParametWriteFun_t g_mcuParameterWriteCbFunc = NULL; // 读取MCU参数函数
static uint16_t g_cycleTime = 0;                               // 周期调用时间
// static uint8_t g_maxParametNumbr=0 ;                      //paramID最大值
static uint8_t g_syncMpuParamIdList[26] = {0};  // mcu请求获取CPU参数ID列表
static uint8_t g_syncMpuParamSize = 0;          // mcu请求获取CPU参数的参数个数
static uint8_t g_syncMpuParamRequstFlag = 0;    // mcu请求获取CPU参数请求发送标记0:未发送，1：发送
static uint8_t g_syncMpuParamResultFlag = 0;    // mcu请求获取CPU参数结果标记0:未成功,1:成功
static uint16_t g_syncMpuParamTimeCount = 0;    // mcu请求获取CPU参数计时
static uint8_t g_syncParamToMpuRequstFlag = 0;  // Mcu请求同步参数到MPU请求标记0：无请求，1：有请求
static uint16_t g_syncParamToMpuTimeCount = 0;  // Mcu请求同步参数至Mpu请求时间计数
static uint8_t g_syncParamToMpuRequstCount = 0; // Mcu请求同步参数至Mpu重复请求计数
static uint8_t g_syncParamToMpuData[64] = {0};  // Mcu请求同步参数至Mpu请求数据缓存
static MpuHalDataPack_t g_syncParamToMpuPack;   // Mcu请求同步参数至Mpu请求数据缓存结构体
static uint8_t g_mpuDataBuffer[200] = {0};      // 用来存储MCU请求和响应数据的缓存buffer
static MpuHalDataPack_t g_mpuDataPack;          // 用来存储要发送的请求和响应数据的缓存结构体

static const ParamLengthEntry_t g_paramLengthTable[] = {
    {E_ParamId_SN, 45},
    {E_ParamId_VIN, 17},
    // {E_ParamId_ParatNumber, 14},
};

/*************************************************
  Function:       ParameterSyncRequstGetAllParamPackage
  Description:    请求同步所有参数接口
  Input:          无
  Output:         无
  Return:         无
  Others:
*************************************************/
static void ParameterSyncRequstGetAllParamPackage(void)
{
    uint8_t i = 0;
    uint16_t lenth = 0;

    g_mpuDataPack.aid = 0x01;
    g_mpuDataPack.mid = 0x03;
    g_mpuDataPack.subcommand = 0x01;

    memset(g_mpuDataBuffer, 0, sizeof(g_mpuDataBuffer));
    g_mpuDataPack.dataBufferSize = sizeof(g_mpuDataBuffer);
    g_mpuDataBuffer[0] = g_syncMpuParamSize;
    lenth = 1;

    for (i = 0; i < g_syncMpuParamSize; i++)
    {
        g_mpuDataBuffer[lenth] = g_syncMpuParamIdList[i];
        lenth++;
    }

    g_mpuDataPack.pDataBuffer = g_mpuDataBuffer;
    g_mpuDataPack.dataLength = lenth;

    return;
}

/*************************************************
  Function:       ParameterSyncRequstGetParamPackage
  Description:    请求同步参数接口
  Input:          paramId：请求同步的参数ID
  Output:         无
  Return:         无
  Others:
*************************************************/
/*
static void ParameterSyncRequstGetParamPackage(uint8_t paramId)
{
    g_mpuDataPack.aid = 0x01;
    g_mpuDataPack.mid = 0x03;
    g_mpuDataPack.subcommand = 0x01;

    memset(g_mpuDataBuffer,0,sizeof(g_mpuDataBuffer));
    g_mpuDataPack.dataBufferSize = sizeof(g_mpuDataBuffer);
    g_mpuDataBuffer[0] = 1;
    g_mpuDataBuffer[1] =paramId;
    g_mpuDataPack.pDataBuffer = g_mpuDataBuffer;
    g_mpuDataPack.dataLength = 2;

    return ;
}
*/

// /*************************************************
//   Function:       ParameterSyncResponseGetParamPackage
//   Description:    生成MPU向MCU请求获取参数的响应包
//   Input:          无
//   Output:         无
//   Return:         无
//   Others:
// *************************************************/
// static int16_t ParameterSyncResponseGetParamPackage(MpuHalDataPack_t *recvDataPack)
// {
//     uint8_t i = 0;
//     uint16_t lenth = 0;
//     uint16_t paramLenth = 0;

//     if (recvDataPack == NULL)
//     {
//         return -1;
//     }

//     g_mpuDataPack.aid = 0x01;
//     g_mpuDataPack.mid = 0x03;
//     g_mpuDataPack.subcommand = 0x02;
//     memset(g_mpuDataBuffer, 0, sizeof(g_mpuDataBuffer));
//     g_mpuDataPack.dataBufferSize = sizeof(recvDataPack);
//     g_mpuDataBuffer[0] = recvDataPack->pDataBuffer[0];
//     lenth = 1;

//     for (i = 0; i < recvDataPack->pDataBuffer[0]; i++)
//     {
//         g_mpuDataBuffer[lenth] = recvDataPack->pDataBuffer[1 + i];
//         //        if(onlyRead)
//         //        {
//         //           memcpy();
//         //       }
//         //      else
//         //     {
//         //        g_mcuParameterReadCbFunc(recvDataPack->pDataBuffer[1 + i], g_mpuDataBuffer + 2 + lenth, &paramLenth);
//         //}
//         g_mcuParameterReadCbFunc(recvDataPack->pDataBuffer[1 + i], g_mpuDataBuffer + 2 + lenth, &paramLenth);

//         g_mpuDataBuffer[1 + lenth] = paramLenth;
//         lenth = lenth + 2 + g_mpuDataBuffer[1 + lenth];
//     }

//     g_mpuDataPack.pDataBuffer = g_mpuDataBuffer;
//     g_mpuDataPack.dataLength = lenth;

//     return 0;
// }

/*******************************************************************************
 * Function:       ParameterSyncResponseGetParamPackage
 * Description:    处理MPU的参数读取请求(ReqParam)，并构建响应报文(RespParam)。
 * Return:         0: 成功, -1: 失败
 * Others:         无
 *******************************************************************************/
// static int16_t ParameterSyncResponseGetParamPackage(MpuHalDataPack_t *recvDataPack)
// {
//     uint8_t i = 0;
//     uint16_t total_length = 0;
//     uint8_t num_of_params = 0;
//     uint16_t paramLenth = 0;

//     if (recvDataPack == NULL)
//     {
//         return -1;
//     }

//     g_mpuDataPack.aid = 0x01;
//     g_mpuDataPack.mid = 0x03;
//     g_mpuDataPack.subcommand = 0x02;

//     memset(g_mpuDataBuffer, 0, sizeof(g_mpuDataBuffer));
//     g_mpuDataPack.dataBufferSize = sizeof(g_mpuDataBuffer);

//     num_of_params = recvDataPack->pDataBuffer[0];
//     g_mpuDataBuffer[0] = num_of_params;
//     total_length = 1;

//     for (i = 0; i < num_of_params; i++)
//     {
//         uint8_t current_param_id = recvDataPack->pDataBuffer[1 + i];
//         g_mpuDataBuffer[total_length] = current_param_id;
//         paramLenth = 0;

//         switch (current_param_id)
//         {
//         case E_ParamId_HW_Version: // F17F_cxl
//         {
//             ProjectConfigGetGacSparePartNumber_F17F(g_mpuDataBuffer + total_length + 2, &paramLenth);
//             break;
//         }
//         case E_ParamId_CustomSW_Version: // F189_cxl
//         {
//             ProjectConfigGetEcuSwVersion_F189(g_mpuDataBuffer + total_length + 2, &paramLenth);
//             break;
//         }
//         case E_ParamId_SW_Version: // C100
//         {
//             ProjectConfigSiRunSwVersion_C100(g_mpuDataBuffer + total_length + 2, &paramLenth);
//             break;
//         }
//             // case E_ParamId_SN: // F18C_cxl
//             // {
//             //     ProjectConfigGetGacEcuSerialNumber_F18C(g_mpuDataBuffer + total_length + 2, &paramLenth);
//             //     break;
//             // }

//         default:
//         {

//             if (g_mcuParameterReadCbFunc != NULL)
//             {
//                 // g_mcuParameterReadCbFunc(current_param_id, para, &paramLenth);
//                 // for ()
//                 g_mcuParameterReadCbFunc(current_param_id, g_mpuDataBuffer + total_length + 2, &paramLenth);
//             }
//             break;
//         }
//         }
//         g_mpuDataBuffer[total_length + 1] = paramLenth;
//         total_length = total_length + 1 + 1 + paramLenth;
//     }
//     g_mpuDataPack.pDataBuffer = g_mpuDataBuffer;
//     g_mpuDataPack.dataLength = total_length;

//     return 0;
// }
static int16_t ParameterSyncResponseGetParamPackage(MpuHalDataPack_t *recvDataPack)
{
    uint8_t i = 0;
    uint16_t total_length = 0;
    uint8_t num_of_params = 0;
    uint16_t paramLenth = 0;

    if (recvDataPack == NULL)
    {
        return -1;
    }

    g_mpuDataPack.aid = 0x01;
    g_mpuDataPack.mid = 0x03;
    g_mpuDataPack.subcommand = 0x02;

    memset(g_mpuDataBuffer, 0, sizeof(g_mpuDataBuffer));
    g_mpuDataPack.dataBufferSize = sizeof(g_mpuDataBuffer);

    num_of_params = recvDataPack->pDataBuffer[0];
    g_mpuDataBuffer[0] = 0;
    total_length = 1;

    uint8_t valid_param_count = 0;

    for (i = 0; i < num_of_params; i++)
    {
        uint8_t current_param_id = recvDataPack->pDataBuffer[1 + i];
        uint8_t *pWrite = g_mpuDataBuffer + total_length;
        paramLenth = 0;

        switch (current_param_id)
        {
            case E_ParamId_SW_Version:
                ProjectConfigGetMcuMpuTotalVersion(pWrite + 2, &paramLenth);
                TBOX_PRINT("----SW Version resp: len=%d, data=", paramLenth);
                for (uint16_t dbg_i = 0; dbg_i < paramLenth; dbg_i++) {
                    TBOX_PRINT("%02X ", *(pWrite + 2 + dbg_i));
                }
                TBOX_PRINT("\r");
             break;
        case E_ParamId_HW_Version: // F193 控制器硬件版本
            ProjectConfigGetSupplierHdVersion_F193(pWrite + 2, &paramLenth);
            break;
		
        // case E_ParamId_CustomSW_Version: // F189_cxl
        //     ProjectConfigGetEcuSwVersion_F189(pWrite + 2, &paramLenth);
        //     break;
        // case E_ParamId_ParatNumber: // F187_cxl
        //     ProjectConfigGetGacEcuPartNumber_F187(pWrite + 2, &paramLenth);
        //     break;
        default:
            if (g_mcuParameterReadCbFunc != NULL)
            {
                g_mcuParameterReadCbFunc(current_param_id, pWrite + 2, &paramLenth);
            }
            break;
        }

        bool all_ff = true;
        for (uint16_t j = 0; j < paramLenth; j++)
        {
            if (*(pWrite + 2 + j) != 0xFF)
            {
                all_ff = false;
                break;
            }
        }

        if (all_ff)
        {
            continue;
        }

        pWrite[0] = current_param_id;
        pWrite[1] = (uint8_t)paramLenth;
        total_length += 2 + paramLenth;
        valid_param_count++;
    }

    g_mpuDataBuffer[0] = valid_param_count;

    g_mpuDataPack.pDataBuffer = g_mpuDataBuffer;
    g_mpuDataPack.dataLength = total_length;

    return 0;
}

/*************************************************
  Function:       ParameterSyncRequstSyncParamPackage
  Description:    生成MCU请求同步某一参数到MPU的请求包
  Input:          无
  Output:         无
  Return:         无
  Others:
*************************************************/
static void ParameterSyncRequstSyncParamPackage(uint8_t paramId)
{
    uint16_t lenth = 0;
    g_syncParamToMpuPack.aid = 0x01;
    g_syncParamToMpuPack.mid = 0x03;
    g_syncParamToMpuPack.subcommand = 0x03;

    memset(g_syncParamToMpuData, 0, sizeof(g_syncParamToMpuData));

    g_syncParamToMpuPack.dataBufferSize = sizeof(g_syncParamToMpuData);
    g_syncParamToMpuData[0] = paramId;

    g_mcuParameterReadCbFunc(paramId, g_syncParamToMpuData + 2, &lenth);
    g_syncParamToMpuData[1] = (lenth & 0xFF);
    lenth = 2 + g_syncParamToMpuData[1];
    g_syncParamToMpuPack.pDataBuffer = g_syncParamToMpuData;
    g_syncParamToMpuPack.dataLength = lenth;

    return;
}

/*************************************************
  Function:       ParameterSyncResponseSyncParamPackage
  Description:    生成CPU同步参数到MCU的响应数据包
  Input:          无
  Output:         无
  Return:         无
  Others:
*************************************************/
static void ParameterSyncResponseSyncParamPackage(uint8_t result, uint8_t paramId)
{
    uint16_t lenth = 0;

    g_mpuDataPack.aid = 0x01;
    g_mpuDataPack.mid = 0x03;
    g_mpuDataPack.subcommand = 0x04;

    memset(g_mpuDataBuffer, 0, sizeof(g_mpuDataBuffer));

    g_mpuDataPack.dataBufferSize = sizeof(g_mpuDataBuffer);
    g_mpuDataBuffer[0] = (result == 1) ? 0 : 1;
    g_mpuDataBuffer[1] = paramId;
    g_mcuParameterReadCbFunc(paramId, g_mpuDataBuffer + 3, &lenth);
    g_mpuDataBuffer[2] = (lenth & 0xFF);
    lenth = lenth + 3;
    g_mpuDataPack.pDataBuffer = g_mpuDataBuffer;
    g_mpuDataPack.dataLength = lenth;

    return;
}
// static void ParameterSyncResponseSyncParamPackage(uint8_t result, uint8_t paramId, uint8_t *pData, uint16_t len)

// {

//     g_mpuDataPack.aid = 0x01;
//     g_mpuDataPack.mid = 0x03;
//     g_mpuDataPack.subcommand = 0x04;                        // RespSyncParam
//     g_mpuDataPack.dataBufferSize = sizeof(g_mpuDataBuffer); // 假设缓冲区大小
//     g_mpuDataBuffer[0] = (result == 0) ? 0 : 1;             // status
//     g_mpuDataBuffer[1] = paramId;                           // ParaIndex
//     g_mpuDataBuffer[2] = (uint8_t)len;                      // ParaLength
//     if (pData != NULL && len > 0)
//     {
//         memcpy(&g_mpuDataBuffer[3], pData, len);
//     }
//     g_mpuDataPack.pDataBuffer = g_mpuDataBuffer;
//     g_mpuDataPack.dataLength = 3 + len;
// }
/*************************************************
  Function:       ParameterSyncSdkInit
  Description:    参数同步模块初始化接口
  Input:          mpuHandle ：mpu通信句柄
                  mcuParamReadCb：读取参数接口回调函数
                  mcuParamWriteCb：写入参数接口回调函数
                  cycleTime：周期调用接口调用周期，单位ms，>0
                  syncParamList:参数ID列表
                  syncParamSize：参数列表大小
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:
*************************************************/
int16_t ParameterSyncSdkInit(int16_t mpuHandle, pMcuParametReadFun_t mcuParamReadCb, pMcuParametWriteFun_t mcuParamWriteCb, uint16_t cycleTime, uint8_t *syncParamList, uint8_t syncParamSize)
{
    if (cycleTime == 0 || syncParamList == NULL || syncParamSize == 0)
    {
        return -1;
    }
    g_cycleTime = cycleTime;
    g_mpuHandle = mpuHandle;
    g_mcuParameterReadCbFunc = mcuParamReadCb;
    g_mcuParameterWriteCbFunc = mcuParamWriteCb;
    memcpy(g_syncMpuParamIdList, syncParamList, syncParamSize);
    g_syncMpuParamSize = syncParamSize;

    return 0;
}

static uint8_t IsValidParamData(uint8_t paramId, uint8_t *data, uint16_t length)
{
    uint8_t i;
    uint8_t found = 0;
    uint16_t expectedLen = 0;

    for (i = 0; i < sizeof(g_paramLengthTable) / sizeof(g_paramLengthTable[0]); i++)
    {
        if (g_paramLengthTable[i].paramId == paramId)
        {
            expectedLen = g_paramLengthTable[i].length;
            found = 1;
            break;
        }
    }

    if (found)
    {
        if (length != expectedLen)
        {
            return 0;
        }

        //if (paramId == E_ParamId_SN || paramId == E_ParamId_VIN || paramId == E_ParamId_ParatNumber)
        if (paramId == E_ParamId_SN || paramId == E_ParamId_VIN)
        {
            uint8_t isAll0x00 = 1;
            uint8_t isAll0x01 = 1;
            uint8_t isAll0xFF = 1;

            for (uint16_t j = 0; j < length; j++)
            {
                if (data[j] != 0x30) // ASCII '0'
                    isAll0x00 = 0;
                if (data[j] != 0x31) // ASCII '1'
                    isAll0x01 = 0;
                if (data[j] != 0xFF)
                    isAll0xFF = 0;
                //
                if (isAll0x01 == 0 && isAll0xFF == 0 && isAll0x00 == 0)
                {
                    return 1;
                }
            }

            if (isAll0x00 || isAll0x01 || isAll0xFF)
            {
                return 0;
            }
        }
    }

    return 1;
}

/*************************************************
  Function:       ParameterSyncSdkCycleProcess
  Description:    参数同步模块周期调用接口
  Input:          recvDataPack：接收到的mpu参数同步请求响应数据包，未收到传入NULL
  Output:         无
  Return:         无
  Others:
*************************************************/
void ParameterSyncSdkCycleProcess(MpuHalDataPack_t *recvDataPack)
{
    uint8_t i = 0;
    uint16_t offsetLen = 0;
    uint8_t paramData[256] = {0};
    uint16_t length = 0;

    if (recvDataPack != NULL)
    {
        // TBOX_PRINT("Parameter sync : aid %d ,mid %d, subcommond %d\r\n",recvDataPack->aid,recvDataPack->mid,(recvDataPack->subcommand & 0x7F));
        if ((recvDataPack->subcommand & 0x7F) == 2)
        {
            i = 0;
            offsetLen = 1;
            for (i = 0; i < recvDataPack->pDataBuffer[0]; i++)
            {
                 // TBOX_PRINT("ParameterSyncSdkCycleProcess: recvDataPack->pDataBuffer[offsetLen] %d\n",recvDataPack->pDataBuffer[offsetLen]);
                if (recvDataPack->pDataBuffer[offsetLen] == g_syncMpuParamIdList[PARAM_SYNC_INDEX_SW_VERSION])
                {
                    ProjectConfigGetMcuMpuTotalVersion(paramData, &length);
                    if (memcmp(paramData, &(recvDataPack->pDataBuffer[offsetLen + 2]), recvDataPack->pDataBuffer[offsetLen + 1]) != 0)
                    {
                        if (ProjectConfigSetMpuVersion(recvDataPack->pDataBuffer + offsetLen + 2, recvDataPack->pDataBuffer[offsetLen + 1]) == 0)
                        {
                            ProjectConfigGetMcuMpuTotalVersion(paramData, &length);
                            g_mcuParameterWriteCbFunc(recvDataPack->pDataBuffer[offsetLen], paramData, length);
                        }
                        }
                    }
                    else
                    {
                        g_mcuParameterReadCbFunc(recvDataPack->pDataBuffer[offsetLen], paramData, &length);
                    // if (recvDataPack->pDataBuffer[offsetLen] == E_ParamId_SN)
                    // {
                    //     for (uint16_t j = 0; j < recvDataPack->pDataBuffer[offsetLen + 1]; j++)
                    //     {
                    //         TBOX_PRINT("MCU SN Param Data[%d]: 0x%02X\r", j, recvDataPack->pDataBuffer[offsetLen + 2 + j]);
                    //     }
                    //     TBOX_PRINT("\r\n");
                    //     for (uint16_t j = 0; j < length; j++)
                    //     {

                    //         TBOX_PRINT("Local SN Param Data[%d]: 0x%02X\r", j, paramData[j]);
                    //     }
                    //     TBOX_PRINT("\r\n");
                    // }
                        if (IsValidParamData(recvDataPack->pDataBuffer[offsetLen],
                                         &(recvDataPack->pDataBuffer[offsetLen + 2]),
                                         recvDataPack->pDataBuffer[offsetLen + 1]) == 1)
                        {
                            // TBOX_PRINT("Param ID %d valid data received for write.\r\n", recvDataPack->pDataBuffer[offsetLen]);

                            if (memcmp(paramData, &(recvDataPack->pDataBuffer[offsetLen + 2]), recvDataPack->pDataBuffer[offsetLen + 1]) != 0)
                            {

                                g_mcuParameterWriteCbFunc(recvDataPack->pDataBuffer[offsetLen],
                                                        recvDataPack->pDataBuffer + offsetLen + 2,
                                                        recvDataPack->pDataBuffer[offsetLen + 1]);
                            }
                        }
                    // else
                    // {
                    //     TBOX_PRINT("Invalid Param ID %d received, write skipped.\r\n", recvDataPack->pDataBuffer[offsetLen]);
                    // }
                }
                offsetLen = offsetLen + 2 + recvDataPack->pDataBuffer[offsetLen + 1];
            }
            g_syncMpuParamResultFlag = 1;
            g_syncMpuParamTimeCount = 0;
        }
        else if ((recvDataPack->subcommand & 0x7F) == 1)
        {
//             TBOX_PRINT("ParamSync recv REQ (subcmd=1), g_syncMpuParamResultFlag=%d\r\n", g_syncMpuParamResultFlag);
            if (g_syncMpuParamResultFlag == 1)
            {
                ParameterSyncResponseGetParamPackage(recvDataPack);
                MpuHalTransmit(g_mpuHandle, &g_mpuDataPack, MPU_HAL_UART_MODE);
            }
        }
        else if ((recvDataPack->subcommand & 0x7F) == 4)
        {
            if (g_syncParamToMpuRequstFlag == 1)
            {
                if (recvDataPack->pDataBuffer[0] == 0)
                {
                    g_syncParamToMpuRequstFlag = 0;
                    g_syncParamToMpuRequstCount = 0;
                    g_syncParamToMpuTimeCount = 0;
                }
            }
        }
        else if ((recvDataPack->subcommand & 0x7F) == 3)
        {
            if (recvDataPack->pDataBuffer[0] < 26)
            {
                if (IsValidParamData(recvDataPack->pDataBuffer[0], &recvDataPack->pDataBuffer[2], recvDataPack->pDataBuffer[1]))
                {
                    g_mcuParameterWriteCbFunc(recvDataPack->pDataBuffer[0], &recvDataPack->pDataBuffer[2], recvDataPack->pDataBuffer[1]);
                    ParameterSyncResponseSyncParamPackage(1, recvDataPack->pDataBuffer[0]);
                }
                else
                {
                    ParameterSyncResponseSyncParamPackage(0, recvDataPack->pDataBuffer[0]);
                }
                MpuHalTransmit(g_mpuHandle, &g_mpuDataPack, MPU_HAL_UART_MODE);
            }
            else
            {
                ParameterSyncResponseSyncParamPackage(0, recvDataPack->pDataBuffer[0]);
                MpuHalTransmit(g_mpuHandle, &g_mpuDataPack, MPU_HAL_UART_MODE);
            }
        }
        else
        {
        }
    }
    if (g_syncMpuParamResultFlag == 0)
    {
        g_syncMpuParamTimeCount++;
        if (g_syncMpuParamRequstFlag == 0 || (g_syncMpuParamRequstFlag == 1 && g_syncMpuParamTimeCount >= (1000 / g_cycleTime)))
        {
            ParameterSyncRequstGetAllParamPackage();
            MpuHalTransmit(g_mpuHandle, &g_mpuDataPack, MPU_HAL_UART_MODE);
            g_syncMpuParamRequstFlag = 1;
            g_syncMpuParamTimeCount = 0;
        }
    }
    if (g_syncParamToMpuRequstFlag == 1)
    {
        g_syncParamToMpuTimeCount++;
        if (g_syncParamToMpuTimeCount > (1000 / g_cycleTime))
        {
            g_syncParamToMpuRequstCount++;
            if (g_syncParamToMpuRequstCount < 3)
            {
                MpuHalTransmit(g_mpuHandle, &g_syncParamToMpuPack, MPU_HAL_UART_MODE);
                g_syncParamToMpuTimeCount = 0;
            }
        }
    }

    return;
}
// void ParameterSyncSdkCycleProcess(MpuHalDataPack_t *recvDataPack)
// {
//     if (recvDataPack != NULL)
//     {
//         uint8_t subcommand = recvDataPack->subcommand & 0x7F;
//         uint8_t *pData = recvDataPack->pDataBuffer;
//         uint16_t dataLen = recvDataPack->dataLength;

//         switch (subcommand)
//         {
//         case 0x02:
//         {
//             if (dataLen < 1)
//                 break;

//             uint8_t count = pData[0];
//             uint16_t offset = 1;
//             uint8_t localDataBuffer[256];

//             for (uint8_t i = 0; i < count; i++)
//             {
//                 if (offset + 2 > dataLen)
//                     break;
//                 uint8_t paramId = pData[offset];
//                 uint8_t paramLen = pData[offset + 1];
//                 if (offset + 2 + paramLen > dataLen)
//                     break;
//                 uint8_t *mpuData = &pData[offset + 2];

//                 uint16_t localLen = 0;
//                 g_mcuParameterReadCbFunc(paramId, localDataBuffer, &localLen);
//                 if ((localLen != paramLen) || (memcmp(localDataBuffer, mpuData, paramLen) != 0))
//                 {
//                     g_mcuParameterWriteCbFunc(paramId, mpuData, paramLen);
//                 }
//                 offset += (2 + paramLen);
//             }
//             g_syncMpuParamResultFlag = 1;
//             g_syncMpuParamTimeCount = 0;
//             break;
//         }

//         case 0x01:
//         {
//             ParameterSyncResponseGetParamPackage(recvDataPack);
//             MpuHalTransmit(g_mpuHandle, &g_mpuDataPack, MPU_HAL_UART_MODE);
//             break;
//         }

//         case 0x03:
//         {

//             if (dataLen < 2)
//                 break;
//             uint8_t paramId = pData[0];
//             uint8_t paramLen = pData[1];

//             if (2 + paramLen > dataLen)
//                 break;

//             uint8_t *value = &pData[2];
//             uint8_t status = 0; // 0=success, 1=error
//             uint8_t readBackBuffer[256];
//             uint16_t readBackLen = 0;

//             if (g_mcuParameterWriteCbFunc(paramId, value, paramLen) != 0)
//             {
//                 status = 1;
//             }
//             g_mcuParameterReadCbFunc(paramId, readBackBuffer, &readBackLen);
//             ParameterSyncResponseSyncParamPackage(status, paramId, readBackBuffer, readBackLen);
//             MpuHalTransmit(g_mpuHandle, &g_mpuDataPack, MPU_HAL_UART_MODE);
//             break;
//         }
//         case 0x04:
//         {
//             if (g_syncParamToMpuRequstFlag == 1)
//             {
//                 if (dataLen < 4)
//                     break;

//                 uint8_t status = pData[0];
//                 uint8_t respId = pData[1];
//                 uint8_t respLen = pData[2];
//                 uint8_t *respData = &pData[3];

//                 if ((status == 0) &&
//                     (respId == g_syncParamToMpuPack.pDataBuffer[0]) &&
//                     (respLen == g_syncParamToMpuPack.pDataBuffer[1]) &&
//                     (memcmp(respData, &g_syncParamToMpuPack.pDataBuffer[2], respLen) == 0))
//                 {

//                     g_syncParamToMpuRequstFlag = 0;
//                     g_syncParamToMpuRequstCount = 0;
//                     g_syncParamToMpuTimeCount = 0;
//                 }
//             }
//             break;
//         }
//         }
//     }

//     if (g_syncMpuParamResultFlag == 0)
//     {
//         g_syncMpuParamTimeCount++;
//         if (g_syncMpuParamRequstFlag == 0 || (g_syncMpuParamTimeCount >= (1000 / g_cycleTime)))
//         {
//             ParameterSyncRequstGetAllParamPackage();
//             MpuHalTransmit(g_mpuHandle, &g_mpuDataPack, MPU_HAL_UART_MODE);
//             g_syncMpuParamRequstFlag = 1;
//             g_syncMpuParamTimeCount = 0;
//         }
//     }

//     if (g_syncParamToMpuRequstFlag == 1)
//     {
//         g_syncParamToMpuTimeCount++;
//         if (g_syncParamToMpuTimeCount > (5000 / g_cycleTime))
//         {
//             g_syncParamToMpuRequstCount++;
//             if (g_syncParamToMpuRequstCount < SYNC_PARAM_TO_MPU_REQUST_COUNT_MAX)
//             {
//                 MpuHalTransmit(g_mpuHandle, &g_syncParamToMpuPack, MPU_HAL_UART_MODE);
//                 g_syncParamToMpuTimeCount = 0;
//             }
//             else
//             {
//                 g_syncParamToMpuRequstFlag = 0;
//                 g_syncParamToMpuRequstCount = 0;
//                 g_syncParamToMpuTimeCount = 0;
//             }
//         }
//     }
// }
/*************************************************
  Function:       ParameterSyncSdkSetToCpu
  Description:    同步参数至MPU
  Input:          parameterId：需要同步至MPU的参数ID
                  pData:参数数据
                  length：参数数据长度
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:
*************************************************/
int16_t ParameterSyncSdkSetToCpu(uint8_t parameterId, uint8_t *pData, uint16_t length)
{
    int16_t ret = -1;
    uint16_t i = 0;
    uint8_t retryCount = 0;

    const uint8_t MAX_RETRY_TIMES = 2;
    
    const uint32_t POLL_INTERVAL_MS = 10;   
    const uint16_t WAIT_LOOP_COUNT = 50;    

    if (pData == NULL || length == 0)
    {
        return ret;
    }

    ParameterSyncRequstSyncParamPackage(parameterId);

    g_syncParamToMpuTimeCount = 0;
    g_syncParamToMpuRequstCount = 0;
    g_syncParamToMpuRequstFlag = 1;

    for (retryCount = 0; retryCount < MAX_RETRY_TIMES; retryCount++)
    {
        MpuHalTransmit(g_mpuHandle, &g_syncParamToMpuPack, MPU_HAL_UART_MODE);

        for (i = 0; i < WAIT_LOOP_COUNT; i++)
        {
            if (g_syncParamToMpuRequstFlag == 0)
            {
                return 0; 
            }
            vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
        }
        
    }

    if (g_syncParamToMpuRequstFlag != 0)
    {
        g_syncParamToMpuRequstFlag = 0;
    }

    return -1;
}

/*************************************************
  Function:       SyncParameterSdkGetFromCpuIsFinished
  Description:    获取参数同步的结果
  Input:          无
  Output:         无
  Return:         0：同步成功
                  -1：同步未成功
  Others:
*************************************************/
int16_t ParameterSyncSdkGetFromCpuIsFinished(void)
{
    return (g_syncMpuParamResultFlag == 1) ? 0 : -1;
}
