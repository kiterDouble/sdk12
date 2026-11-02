
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "udsTp.h"
#include <string.h>
#include "logHal.h"
// 诊断状态管理
typedef enum
{
    E_REMOTE_STATE_FIRST_FRAME = 0,
    E_REMOTE_STATE_FLOW_CONTROL_FRAME,
    E_REMOTE_STATE_CONSECUTIVE_FRAME,
    E_REMOTE_STATE_FRAME_FINISHED
} RemoteEcuTpState_e;

// udsTp用来设置接收缓存结构体
typedef struct
{
    RemoteEcuTpState_e tpState;
    uint8_t *pTpBuffer;      // 接收buffer
    uint16_t tpBufferSize;   // Buffer大小
    uint16_t tpReceiveCount; // 接收到的实际长度
    uint32_t responseId;     // 诊断响应can ID
} FunctionEcu_t;

// udsTp结构体
// pending响应回调函数类型
typedef void (*UdsTpPendingCallback_t)(uint32_t responseId, uint8_t sid);

typedef struct
{
    int16_t canHandle;                // Can通信句柄
    uint32_t physicalRequestId;       // 物理寻址请求ID
    uint32_t physicalResponseId;      // 物理寻址响应ID
    uint32_t functionalRequestId;     // 功能寻址请求ID
    FunctionEcu_t *pFunctionalEcuBuf; // 接收buffer
    uint16_t functionalEcuBufSize;    // 接收buffer大小
    uint16_t functionalEcuCount;      // 接收计数
    uint32_t functionalResponseIdMax; // 功能寻址响应最小ID
    uint32_t functionalResponseIdMin; // 功能寻址响应最大ID
    UdsTpParameter_t *pParameter;     // CanTP参数设置
    UdsTpPendingCallback_t pendingCallback; // pending响应回调函数
} RemoteVariable_t;

#define UDS_TRANS_CANFD_ENABLE                   (0U)

#define REMOTE_DIAGNOSTIC_INSTANCE_NUM 6 // Can通道数量最大数

static RemoteVariable_t g_udsTp[REMOTE_DIAGNOSTIC_INSTANCE_NUM]; // 为每一路定义一个初始化TP结构体
static uint8_t g_handleOpenCount = 0;                            // 初始化TP结构体使用计数

/*************************************************
  Function:       UdsTpOpen
  Description:    转发诊断报文获取句柄
  Input:          canChanal:
                  pCanRxbuffer:
                  pBufferNum:
                  pTpParameter:
  Output:         无
  Return:         -1：失败
                  >0:有效句柄
  Others:
*************************************************/
int16_t UdsTpOpen(uint8_t canChanal, CanHalMsg_t *pCanRxbuffer, uint16_t pBufferNum, UdsTpParameter_t *pTpParameter)
{
    int16_t handle = -1;
    int16_t canHandle;

    if (g_handleOpenCount >= REMOTE_DIAGNOSTIC_INSTANCE_NUM)
    {
        return -1;
    }
    canHandle = CanHalOpen(canChanal);
    if (canHandle < 0)
    {
        return -2;
    }
    CanHalSetRxBuffer(canHandle, pCanRxbuffer, pBufferNum);
    handle = g_handleOpenCount;
    g_handleOpenCount++;

    g_udsTp[handle].canHandle = canHandle;
    g_udsTp[handle].pParameter = pTpParameter;
    g_udsTp[handle].pFunctionalEcuBuf = NULL;
    g_udsTp[handle].functionalEcuBufSize = 0;
    g_udsTp[handle].functionalEcuCount = 0;

    return handle;
}

/*************************************************
  Function:       UdsTpSetFunctionId
  Description:    设置功能寻址id
  Input:          handle：udsTp句柄
                  requestId：功能寻址请求ID
  Output:         无
  Return:         函数执行结果
                  0：成功
                  -1：失败
  Others:
*************************************************/
int16_t UdsTpSetFunctionId(int16_t handle, uint32_t requestId)
{
    g_udsTp[handle].functionalRequestId = requestId;

    return 0;
}

static int16_t UdsTpMultFramGetFlowControl(int16_t tpHandle, int16_t canHandle, CanHalMsg_t *pRxMsg, uint8_t *pSTmin, uint8_t *pBlocksize)
{
    int16_t ret, result;
    result = 0;
    while (1)
    {
        // ret = CanHalDiagnosticReceive(canHandle,pRxMsg, g_tpVariable[tpHandle].pTpParameter->N_Bs_enforced);//wait 150 ms
        ret = CanHalReceive(canHandle, pRxMsg, g_udsTp[tpHandle].pParameter->N_Bs_enforced);

        if (ret != 0)
        {
            result = -1;
            break; // time out
        }
        if (pRxMsg->dlc < 3)
        {
            result = -1;
            break; // error dlc
        }
        if (0x00 == (pRxMsg->canData[0] & 0xF0)) // single
        {
            if ((pRxMsg->canData[1] == 0x7F) && (pRxMsg->canData[1] == 0x78)) // busy state
            {
                continue;
            }
            else
            {
                result = -1;
                break; // not flow control frame
            }
        }
        if (0x30 != (pRxMsg->canData[0] & 0xF0))
        {
            result = -1;
            break; // not flow control frame
        }
        if (0 == (pRxMsg->canData[0] & 0x0F)) // success receive
        {
            break;
        }
        else if (1 == (pRxMsg->canData[0] & 0x0F)) // wait frame
        {
            continue;
        }
        else if (2 == (pRxMsg->canData[0] & 0x0F)) // over flow frmae
        {
            result = -1;
            break; // not flow control frame
        }
        else
        {
            result = -1;
            break; // not flow control frame
        }
    }
    if (result == 0)
    {
        *pBlocksize = pRxMsg->canData[1];
        *pSTmin = pRxMsg->canData[2] + 0; // 2ms
    }
    return result;
}

static int16_t UdsTpDataMultFramTransmit(int16_t tpHandle, int16_t canHandle, uint32_t txCanId, uint32_t multiFrameNum, uint8_t *txData, uint16_t txLength)
{
    uint32_t transmitCount = 0;
    int16_t ret;
    uint8_t txCanData[8];
    uint8_t txCanDlc;
    CanHalMsg_t rxCanMsg;

    uint8_t fillByte;
    int16_t result = 0;
    transmitCount += 6;
    int32_t txFrameCount = 0;

    // rxobj
    // canHandle = g_tpVariable[tpHandle].physicalCanHandle;
    // canHandle = g_udsTp[handle].canHandle;

    uint8_t blockSize = 0;
    uint8_t STmin = 0; // 2ms?????
    uint32_t frameInterval;

    fillByte = g_udsTp[tpHandle].pParameter->fillByte;

    // TBOX_PRINT("[UdsTp] MultiFrame transmit: txCanId=0x%X, totalFrames=%d, dataLen=%d\n", txCanId, multiFrameNum, txLength);

    ret = UdsTpMultFramGetFlowControl(tpHandle, canHandle, &rxCanMsg, &STmin, &blockSize);
    if (ret != 0)
    {
        TBOX_PRINT("[UdsTp] ERROR: Failed to get FlowControl frame\n");
        return -1;
    }
    if (STmin == 0)
    {
        frameInterval = 0;
    }
    else if (STmin < 128)
    {
        frameInterval = STmin;
    }
    else
    {
        frameInterval = 1;
    }
    multiFrameNum -= 1;
    for (txFrameCount = 0; txFrameCount < multiFrameNum; txFrameCount++)
    {
        int j;
        txCanData[0] = (0x20 | ((txFrameCount + 1) & 0x0f));
        if (txFrameCount < (multiFrameNum - 1))
        {
            txCanDlc = 8;
        }
        else
        {
            txCanDlc = ((txLength - 6 - txFrameCount * 7) + 1);
        }
        for (j = 0; j < (txCanDlc - 1); j++)
        {
            txCanData[j + 1] = txData[6 + txFrameCount * 7 + j];
        }

        for (; j < 7; j++)
        {
            txCanData[j + 1] = fillByte; // g_tpVariable[tpHandle].pTpParameter->fillByte;
        }

        if (frameInterval != 0)
        {
            vTaskDelay(pdMS_TO_TICKS(STmin));
        }
        ret = CanHalReceive(canHandle, &rxCanMsg, 0);
        // 检查是否收到pending响应 (7F SID 78)
        if (ret == 0 && rxCanMsg.dlc >= 4 && rxCanMsg.canData[1] == 0x7F && rxCanMsg.canData[3] == 0x78)
        {
            uint8_t sid = rxCanMsg.canData[2];
            // TBOX_PRINT("[UdsTp] Pending response received during CF transmit: 7F %02X 78\r\n", sid);
            // 调用pending回调函数，转发给MPU
            if (g_udsTp[tpHandle].pendingCallback != NULL)
            {
                g_udsTp[tpHandle].pendingCallback(g_udsTp[tpHandle].physicalResponseId, sid);
            }
        }
        memset(rxCanMsg.canData, 0, 8);
        ret = CanHalTransmitQueued(canHandle, txCanId, txCanData, 8, UDS_TRANS_CANFD_ENABLE, CAN_TX_PRIO_NORMAL);
        // TBOX_PRINT("can tp wait send %x\r\n",txCanId);
        if (ret != 0)
        {
            result = -1;
            break;
        }
        memset(rxCanMsg.canData, 0, 8);

        // transmitCount+
        if ((blockSize != 0) && ((txFrameCount + 1) >= blockSize))
        {
            // break;//block transmit finished
            // ret = CanHalDiagnosticReceive(canHandle,&rxCanMsg, m_flowControlTime);//wait 1000 ms
            // ret = CanHalDiagnosticReceive(canHandle,&rxCanMsg, g_udsTp[tpHandle].pTpParameter->N_Bs_enforced);//wait 150 ms
            ret = CanHalReceive(canHandle, &rxCanMsg, g_udsTp[tpHandle].pParameter->N_Bs_enforced);
            if (ret != 0)
            {
                result = -1;
                break; // time out
            }
            if (rxCanMsg.dlc < 3)
            {
                result = -1;
                break; // error dlc
            }
            if (0x30 != (rxCanMsg.canData[0] & 0xF0))
            {
                result = -1;
                break; // not flow control frame
            }
            else
            {
                blockSize += blockSize;
            }
        }
    }

    // canHandle = g_tpVariable[tpHandle].physicalCanHandle;
    // ret = CanHalDiagnosticReceive(canHandle,&rxCanMsg, 0);//wait 150 ms
    // memset(rxCanMsg.canData, 0, 8);
    // canHandle = g_tpVariable[tpHandle].functionalCanHandle;
    // ret = CanHalDiagnosticReceive(canHandle,&rxCanMsg, 0);//wait 150 ms
    // memset(rxCanMsg.canData, 0, 8);
    CanHalReceive(canHandle, &rxCanMsg, 0);

    /*if (txFrameCount >= multiFrameNum)
    {
        break;//transmit success
    }*/

    return result;
}

int16_t UdsTpTransmitRaw(int16_t udsTpHandle, uint32_t canId, uint8_t *txData, uint16_t dataLen)
{
    if (udsTpHandle < 0 || udsTpHandle >= REMOTE_DIAGNOSTIC_INSTANCE_NUM)
    {
        return -1;
    }

    int16_t canHandle = g_udsTp[udsTpHandle].canHandle;

    return CanHalTransmitQueued(canHandle, canId, txData, dataLen, UDS_TRANS_CANFD_ENABLE, CAN_TX_PRIO_NORMAL);
}

/*************************************************
  Function:       UdsTpTransmit
  Description:    远程诊断报文TP发送接口
  Input:          handle：诊断Tp据句柄
                  typeFlag：诊断报文类型标记 0：物理寻址 1：功能寻址
                  txData：Can数据
                  dataLen：数据长度
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:
*************************************************/
int16_t UdsTpTransmit(int16_t handle, uint8_t typeFlag, uint8_t *txData, uint16_t dataLen)
{
    int16_t canHandle;
    uint32_t txCanId;
    uint8_t fillByte;
    // uint8_t udsTransmitError = 0;
    // uint8_t m_transmitErrorFailed = 1;
    // uint8_t m_transmitOk = 0;
    uint8_t data[8];
    uint8_t dlc = 0;
    uint32_t multiFrameNum;
    uint16_t i = 0; // j= 0;
    int16_t ret = -1;
    // CanHalMsg_t rxCanMsg;
    // uint8_t blockSize = 0;
    // uint8_t STmin;;
    // uint32_t frameInterval;
    // TickType_t xLastWakeTime;
    // uint16_t

    canHandle = g_udsTp[handle].canHandle;
    fillByte = g_udsTp[handle].pParameter->fillByte;

    // 功能寻址？
    if (typeFlag == 1)
    {
        txCanId = g_udsTp[handle].functionalRequestId;
    }
    else
    {
        txCanId = g_udsTp[handle].physicalRequestId;
    }

    // 长度<8?
    if (dataLen < 8)
    {
        data[0] = dataLen;
        uint16_t i = 0;
        for (i = 0; i < dataLen; i++)
        {
            data[i + 1] = txData[i];
        }
        for (; i < 7; i++)
        {
            data[i + 1] = fillByte;
        }
        int16_t txRet = CanHalTransmitQueued(canHandle, txCanId, data, 8, UDS_TRANS_CANFD_ENABLE, CAN_TX_PRIO_NORMAL);
        // TBOX_PRINT("[UdsTp] CanHalTransmit return=%d\r\n", txRet);
        return txRet;
    }
    // 初始化及组装第一个包
    dlc = 0;
    // transmitCount = 0;
    multiFrameNum = (dataLen + 1) / (8 - 1);
    if ((dataLen + 1) % (8 - 1) != 0)
    {
        multiFrameNum += 1;
    }
    data[0] = (0x10 | (dataLen / 256));
    data[1] = (dataLen % 256);
    for (i = 0; i < 6; i++)
    {
        data[i + 2] = txData[i];
    }
    dlc = 8;
    ret = CanHalTransmitQueued(canHandle, txCanId, data, dlc, UDS_TRANS_CANFD_ENABLE, CAN_TX_PRIO_NORMAL);
    if (ret != 0)
    {
        return -1;
    }
    ret = UdsTpDataMultFramTransmit(handle, canHandle, txCanId, multiFrameNum, txData, dataLen);
    if (ret != 0)
    {
        return -1;
    }
    return 0;
}
/*
{
    int16_t canHandle;
    uint32_t txCanId;
    uint8_t fillByte;
    uint8_t udsTransmitError = 0;
    uint8_t m_transmitErrorFailed = 1;
    uint8_t m_transmitOk = 0;
    uint8_t data[8];
    uint8_t dlc = 0;
    uint32_t multiFrameNum;
    uint16_t i= 0,j= 0;
    int16_t ret = -1;
    CanHalMsg_t rxCanMsg;
    uint8_t blockSize = 0;
    uint8_t STmin;;
    uint32_t frameInterval;
    TickType_t xLastWakeTime;
    //uint16_t

    canHandle = g_udsTp[handle].canHandle;
    fillByte = g_udsTp[handle].pParameter->fillByte;

    //功能寻址？
    if(typeFlag == 1)
    {
        txCanId = g_udsTp[handle].functionalRequestId;
    }
    else
    {
        txCanId = g_udsTp[handle].physicalRequestId;
    }

    //长度<8?
    if(dataLen<8)
    {
        data[0] = dataLen;
        uint16_t i = 0;
        for(i=0;i<dataLen;i++)
        {
            data[i+1] = txData[i];
        }
        for(;i<7;i++)
        {
            data[i+1] = fillByte;
        }
        return CanHalTransmitQueued(canHandle,txCanId,data,8,0, CAN_TX_PRIO_NORMAL);
    }
    //初始化及组装第一个包
    dlc = 0;
    //transmitCount = 0;
    multiFrameNum = (dataLen+1)/(8-1);
    if((dataLen + 1) % (8 - 1) != 0)
    {
        multiFrameNum += 1;
    }
    data[0] = (0x10 | (dataLen / 256));
    data[1] = (dataLen % 256);
    for (i = 0; i < 6;i++ )
    {
        data[i+2] = txData[i];
    }
    dlc = 8;
    //发送第一个包
    ret = CanHalTransmitQueued(canHandle,txCanId,data,dlc,0, CAN_TX_PRIO_NORMAL);
    if(ret != 0)
    {
        udsTransmitError = m_transmitErrorFailed;
    }
    //transmitCount += 6;
    uint32_t txFrameCount = 0;
    xLastWakeTime = xTaskGetTickCount();
    while(1)
    {
        if(udsTransmitError != m_transmitOk)
        {
            break;
        }
        ret = CanHalReceive(canHandle,&rxCanMsg,g_udsTp[handle].pParameter->N_Bs_enforced);
        if(ret == 0)//接收成功
        {
            if(rxCanMsg.dlc < 3)//数据长度是否正确？
            {
                udsTransmitError = m_transmitErrorFailed;
                break;
            }
            //单包？
            if(0x00 == (rxCanMsg.canData[0] & 0xF0))
            {
                if((rxCanMsg.canData[1]==0x7F)&&(rxCanMsg.canData[2]==0x78))
                {
                    continue;
                }
                else
                {
                    udsTransmitError = m_transmitErrorFailed;
                    break;
                }
            }
            if(0x30 != (rxCanMsg.canData[0] & 0xF0))
            {
                udsTransmitError = m_transmitErrorFailed;
                break;
            }
            if(1 == (rxCanMsg.canData[0] & 0x0F))//wait frame?
            {
                continue;
            }
            else if(2 == (rxCanMsg.canData[0] & 0x0F))//over flow frmae?
            {
                udsTransmitError = m_transmitErrorFailed;
                break;
            }
            else if(0 == (rxCanMsg.canData[0] & 0x0F))            //接收成功
            {
                blockSize = rxCanMsg.canData[1];
                STmin = rxCanMsg.canData[2] + 0; //2ms修正值
                if (STmin == 0)
                {
                    frameInterval = 0;
                }
                else if(STmin < 128)
                {
                    frameInterval = STmin;
                }
                else
                {
                    frameInterval = 1;
                }
                multiFrameNum -= 1;
                for(txFrameCount = 0;txFrameCount < multiFrameNum;txFrameCount++)
                {
                    data[0]=(0x20| ((txFrameCount+1)&0x0f));
                    if (txFrameCount < (multiFrameNum - 1))
                    {
                        dlc = 8;
                    }
                    else
                    {
                        dlc = ((dataLen - 6 - txFrameCount * 7) + 1);
                    }
                    for (j = 0; j < (dlc-1); j++)
                    {
                        data[j + 1] = txData[6 + txFrameCount * 7 + j];
                    }
                    for (; j < 7; j++)
                    {
                        data[j + 1] = fillByte;
                    }
                    if(frameInterval != 0)
                    {
                        //RTOS_HalApiWait(STmin);
                        //RTOS_HalApiWaitUntil(&rtosTickCount,STmin);
                        vTaskDelayUntil(&xLastWakeTime,STmin);
                    }

                    CanHalReceive(canHandle,&rxCanMsg,0);
                    memset(rxCanMsg.canData, 0, 8);
                    ret=CanHalTransmitQueued(canHandle,txCanId,data,8,0, CAN_TX_PRIO_NORMAL);
                    if(ret != 0)
                    {
                        udsTransmitError = m_transmitErrorFailed;
                        break;
                    }
                    if((blockSize != 0) && ((txFrameCount+ 1) >= blockSize))
                    {
                        ret = CanHalReceive(canHandle,&rxCanMsg,g_udsTp[handle].pParameter->N_Bs_enforced);
                        if(ret != 0)
                        {
                            udsTransmitError = m_transmitErrorFailed;
                            break;
                        }
                        if(rxCanMsg.dlc < 3)
                        {
                            udsTransmitError = m_transmitErrorFailed;
                            break;
                        }
                        if(0x30!=(rxCanMsg.canData[0]&0xF0))
                        {
                            udsTransmitError = m_transmitErrorFailed;
                            break;
                        }
                        blockSize += blockSize;
                    }

                }
                CanHalReceive(canHandle,&rxCanMsg,0);
                if((udsTransmitError != m_transmitOk) || (txFrameCount >= multiFrameNum))
                {
                    break;
                }
            }
            else
            {
                udsTransmitError = m_transmitErrorFailed;
                break;
            }

        }
        else
        {
            udsTransmitError = m_transmitErrorFailed;
            break;
        }
    }
    if (udsTransmitError != m_transmitOk)
    {
        return -1;
    }
    return 0;
}
*/
/*************************************************
  Function:       UdsTpReceive
  Description:    获取mcu设置单个参数到CPU的请求包
  Input:          udsTpHandle：接收usd响应句柄
                  pData：接收响应数据
                  dataLen：接收数据长度
                  waitTime：接收超时时间设置 单位：ms
  Output:         无
  Return:         0：接收数据成功
                  -1：未接收到数据
  Others:
*************************************************/
static int16_t UdsTpSingleFrameRx(CanHalMsg_t *canRxMsg, uint8_t *udsData, uint16_t *udsLen)
{
    if ((canRxMsg->canData[0] & 0x0f) > 7)
    {
        return -1;
    }
    memcpy(udsData, &canRxMsg->canData[1], 7);
    *udsLen = canRxMsg->canData[0] & 0x0F;
    if (*udsLen > (canRxMsg->dlc - 1))
    {
        return -1;
    }

    return 0;
}

static int16_t UdsTpMultiFrameRxFLowControlTransmit(int16_t canHandle, int16_t udsTpHandle)
{
    uint8_t fillByte;
    uint8_t blockSize;
    uint8_t STmin;
    CanHalMsg_t canTxMsg;
    int16_t ret = -1;

    fillByte = g_udsTp[udsTpHandle].pParameter->fillByte;
    blockSize = 0x08;
    STmin = 0x14;

    canTxMsg.dlc = 8;
    canTxMsg.canData[0] = 0x30;
    canTxMsg.canData[1] = blockSize;
    canTxMsg.canData[2] = STmin;
    canTxMsg.canData[3] = fillByte;
    canTxMsg.canData[4] = fillByte;
    canTxMsg.canData[5] = fillByte;
    canTxMsg.canData[6] = fillByte;
    canTxMsg.canData[7] = fillByte;
    canTxMsg.canId = g_udsTp[udsTpHandle].physicalRequestId;

    ret = CanHalTransmitQueued(canHandle, canTxMsg.canId, canTxMsg.canData, canTxMsg.dlc, UDS_TRANS_CANFD_ENABLE, CAN_TX_PRIO_NORMAL);
    if (ret != 0)
    {
        return -1;
    }
    return 0;
}

static int16_t UdsTpMultiFrameRx(int16_t udsTpHandle, int16_t canHandle, uint16_t blockSize, uint16_t rxLen, uint8_t *udsData, CanHalMsg_t *pRxmsg)
{
    uint16_t FrameCount, i, rxLenLeft, CF_num, rxCount;
    int16_t ret;

    CF_num = 0x21;
    FrameCount = 0;
    rxLenLeft = rxLen;
    rxCount = 6;

    i = 0;
    while (i < 30)
    {
        if (FrameCount >= blockSize)
        {
            ret = UdsTpMultiFrameRxFLowControlTransmit(canHandle, udsTpHandle);
            if (0 != ret)
            {
                return -1;
            }
            FrameCount = 0;
        }
        if (rxLenLeft > 7)
        {
            // if((FrameCount >= blockSize) && (blockSize != 0))
            //{
            //     ret= TpDataMultiFrameRxFLowControlTransmit(tpHandle,canHandle);
            //     if(0 != ret)
            //     {
            //         return -1;
            //     }
            //     FrameCount = 0;
            // }

            // ret = CanHalDiagnosticReceive(canHandle,&canRxMsg,1000);N_Cr_enforced
            // ret = CanHalDiagnosticReceive(canHandle,pRxmsg,g_tpVariable[tpHandle].pTpParameter->N_Cr_enforced);
            ret = CanHalReceive(canHandle, pRxmsg, g_udsTp[udsTpHandle].pParameter->N_Cr_enforced);
            if (0 != ret)
            {
                return -1;
            }
            if ((((pRxmsg->canData[0] >> 4) & 0x0f) == 0) || ((pRxmsg->canData[0] & 0xF0) == 0X10))
            {
                return -2;
            }

            if (CF_num != pRxmsg->canData[0])
            {
                // return -1;
                if ((pRxmsg->canData[0] & 0xf0) == 0x20)
                {
                    return -1;
                }
                continue;
            }

            if (CF_num == 0x2F)
            {
                CF_num = 0x20;
            }
            else
            {
                CF_num++;
            }

            memcpy(udsData + rxCount, &pRxmsg->canData[1], (pRxmsg->dlc - 1));
            rxCount += (pRxmsg->dlc - 1);
            rxLenLeft -= 7;
            FrameCount++;
        }
        else
        {

            // ret = CanHalDiagnosticReceive(canHandle,&canRxMsg,1000);
            // ret = CanHalDiagnosticReceive(canHandle,pRxmsg,g_tpVariable[tpHandle].pTpParameter->N_Cr_enforced);
            ret = CanHalReceive(canHandle, pRxmsg, g_udsTp[udsTpHandle].pParameter->N_Cr_enforced);
            if (0 != ret)
            {
                return -1;
            }

            if ((((pRxmsg->canData[0] >> 4) & 0x0f) == 0) || ((pRxmsg->canData[0] & 0xF0) == 0x10))
            {
                return -2;
            }

            if (CF_num != pRxmsg->canData[0])
            {
                // return -1;
                if ((pRxmsg->canData[0] & 0xf0) == 0x20)
                {
                    return -1;
                }
                continue;
            }

            if (CF_num == 0x2F)
            {
                CF_num = 0x20;
            }
            else
            {
                CF_num++;
            }

            memcpy(udsData + rxCount, &pRxmsg->canData[1], (pRxmsg->dlc - 1));
            rxCount += (pRxmsg->dlc - 1);

            return rxCount;
        }
        i++;
    }
    return rxCount;
}

int16_t UdsTpReceiveRaw(int16_t udsTpHandle, uint8_t *pData, uint16_t *dataLen)
{
    if (udsTpHandle < 0 || udsTpHandle >= REMOTE_DIAGNOSTIC_INSTANCE_NUM)
        return -1;

    int16_t canHandle = g_udsTp[udsTpHandle].canHandle;
    //canHandle = g_udsTp[handle].canHandle;
    CanHalMsg_t canRxMsg;

    int16_t ret = CanHalReceive(canHandle, &canRxMsg, 0);

    if (ret == 0)
    {
        *dataLen = canRxMsg.dlc;
        memcpy(pData, canRxMsg.canData, *dataLen);
        return 0;
    }
    return -1;
}

int16_t UdsTpReceive(int16_t udsTpHandle, uint8_t *pData, uint16_t *dataLen, uint32_t waitTime)
{
    int16_t canHandle;
    CanHalMsg_t canRxMsg;
    // CanHalMsg_t  canTxMsg;
    int16_t ret;
    uint16_t rxLenLeft;
    uint8_t n_PCItype;
    uint8_t blockSize;
    // uint8_t STmin;
    // uint8_t FrameCount = 0;
    // uint8_t CF_num = 0x21;
    // uint8_t fillByte;
    uint16_t txCount = 0;
    // uint16_t i;

    canHandle = g_udsTp[udsTpHandle].canHandle;
    // fillByte = g_udsTp[udsTpHandle].pParameter->fillByte;
    blockSize = 0x08;
    // STmin = 0x14;

    if (pData == NULL || dataLen == NULL)
    {
        return -1;
    }
    ret = CanHalReceive(canHandle, &canRxMsg, waitTime);
    if (0 == ret)
    {
        do
        {
            n_PCItype = (canRxMsg.canData[0] >> 4) & 0x0F;
            if (0 == n_PCItype)
            {
                return UdsTpSingleFrameRx(&canRxMsg, pData, dataLen);
            }
            else if (1 == n_PCItype)
            {
                txCount = 0;
                rxLenLeft = (((uint16_t)canRxMsg.canData[0] & 0x0F) << 8) + canRxMsg.canData[1];
                *dataLen = rxLenLeft;
                if (rxLenLeft < 8)
                {
                    return -1;
                }
                memcpy(pData, &canRxMsg.canData[2], 6);
                rxLenLeft -= 6;
                txCount += 6;

                ret = UdsTpMultiFrameRxFLowControlTransmit(canHandle, udsTpHandle);
                if (0 != ret)
                {
                    return -1;
                }

                // ret = TpDataMultiFrameRx(tpHandle,canHandle,FrameSum,blockSize,rxLenLeft,udsData,&canRxMsg);
                ret = UdsTpMultiFrameRx(udsTpHandle, canHandle, blockSize, rxLenLeft, pData, &canRxMsg);
                if (ret < 0)
                {
                    if (-2 == ret) // receive again
                    {
                        continue;
                    }
                    else
                    {
                        return -1;
                    }
                }
                if (ret < *dataLen)
                {
                    return -1;
                }
                return 0;
            }
            else if ((2 == n_PCItype) || (3 == n_PCItype))
            {
                return -1;
            }
            return -1;
        } while (1);
    }
    else
    {
        return -1;
    }
}
/*
int16_t UdsTpReceive(int16_t udsTpHandle,uint8_t *pData,uint16_t *dataLen,uint32_t waitTime)
{
    int16_t canHandle;
    CanHalMsg_t canRxMsg;
    CanHalMsg_t  canTxMsg;
    int16_t ret;
    uint16_t rxLenLeft;
    uint8_t n_PCItype;
    uint8_t blockSize;
    uint8_t STmin;
    uint8_t FrameCount = 0;
    uint8_t CF_num = 0x21;
    uint8_t fillByte;
    uint16_t txCount = 0;
    uint16_t i;

    canHandle = g_udsTp[udsTpHandle].canHandle;
    fillByte = g_udsTp[udsTpHandle].pParameter->fillByte;
    blockSize = 0x08;
    STmin = 0x14;

    if(pData==NULL||dataLen==NULL)
    {
        return -1;
    }
    ret = CanHalReceive(canHandle,&canRxMsg,waitTime);
    if(ret != 0)
    {
        return -1;
    }

    while(1)
    {
        n_PCItype = (canRxMsg.canData[0]>>4)&0x0F;
        if(0 == n_PCItype)
        {
            if((canRxMsg.canData[0] & 0x0f) > 7)
            {
                return -1;
            }
            memcpy(pData,&canRxMsg.canData[1],7);
            *dataLen = canRxMsg.canData[0]&0x0F;
            if(*dataLen > (canRxMsg.dlc - 1))
            {
                return -1;
            }
            return 0;
        }
        else if((2 == n_PCItype) || (3 == n_PCItype))
        {
            return -1;
        }
        else if(1 == n_PCItype)
        {
            txCount = 0;
            rxLenLeft = (((uint16_t)canRxMsg.canData[0]&0x0F) << 8) + canRxMsg.canData[1];
            *dataLen = rxLenLeft;
            if(rxLenLeft < 8)
            {
                return -1;
            }
            memcpy(pData,&canRxMsg.canData[2], 6);
            rxLenLeft -= 6;
            txCount += 6;

            while(1)
            {
                canTxMsg.dlc = 8;
                canTxMsg.canData[0] = 0x30;
                canTxMsg.canData[1] = blockSize;
                canTxMsg.canData[2] = STmin;
                canTxMsg.canData[3] = fillByte;
                canTxMsg.canData[4] = fillByte;
                canTxMsg.canData[5] = fillByte;
                canTxMsg.canData[6] = fillByte;
                canTxMsg.canData[7] = fillByte;
                canTxMsg.canId=g_udsTp[udsTpHandle].physicalRequestId;

                ret = CanHalTransmitQueued(canHandle,canTxMsg.canId, canTxMsg.canData,canTxMsg.dlc,0, CAN_TX_PRIO_NORMAL);
                if(ret != 0)
                {
                    return -1;
                }

                for(i = 0;i<30;i++)
                {
                    if(rxLenLeft>7)
                    {
                        if(FrameCount >= blockSize)
                        {
                            canTxMsg.dlc = 8;
                            canTxMsg.canData[0] = 0x30;
                            canTxMsg.canData[1] = blockSize;
                            canTxMsg.canData[2] = STmin;
                            canTxMsg.canData[3] = fillByte;
                            canTxMsg.canData[4] = fillByte;
                            canTxMsg.canData[5] = fillByte;
                            canTxMsg.canData[6] = fillByte;
                            canTxMsg.canData[7] = fillByte;
                            canTxMsg.canId=g_udsTp[udsTpHandle].physicalRequestId;

                            ret = CanHalTransmitQueued(canHandle,canTxMsg.canId, canTxMsg.canData,canTxMsg.dlc,0, CAN_TX_PRIO_NORMAL);
                            if(ret != 0)
                            {
                                return -1;
                            }
                            FrameCount = 0;
                        }

                        ret = CanHalReceive(canHandle,&canRxMsg,g_udsTp[udsTpHandle].pParameter->N_Cr_enforced);
                        if(ret != 0)
                        {
                            return -1;
                        }
                        if((((canRxMsg.canData[0]>>4)&0x0f)==0)||((canRxMsg.canData[0]&0xF0)==0X10))
                        {
                            break;//////////////////////////////////////////////////
                        }
                        if(CF_num != canRxMsg.canData[0])
                        {
                            if((canRxMsg.canData[0] & 0xf0) == 0x20)
                            {
                                return -1;
                            }
                            else
                            {
                                continue;
                            }
                        }
                        else
                        {
                            if(CF_num == 0x2F)
                            {
                                CF_num = 0x20;
                            }
                            else
                            {
                                CF_num++;
                            }
                            memcpy(pData+txCount,&canRxMsg.canData[1],(canRxMsg.dlc - 1));
                            txCount += (canRxMsg.dlc - 1);
                            rxLenLeft -= 7;
                            FrameCount ++;
                        }
                    }
                    else
                    {
                        ret = CanHalReceive(canHandle,&canRxMsg,g_udsTp[udsTpHandle].pParameter->N_Cr_enforced);
                        if(ret != 0)
                        {
                            return -1;
                        }
                        if((((canRxMsg.canData[0]>>4)&0x0f)==0)||((canRxMsg.canData[0]&0xF0)==0X10))
                        {
                            break;///////////////////////////////////////////////
                        }
                        else
                        {
                            if(CF_num != canRxMsg.canData[0])
                            {
                                if((canRxMsg.canData[0] & 0xf0) == 0x20)
                                {
                                    return -1;
                                }
                                else
                                {
                                    continue;
                                }
                            }
                            else
                            {
                                if(CF_num == 0x2F)
                                {
                                    CF_num = 0x20;
                                }
                                else
                                {
                                    CF_num++;
                                }
                                memcpy(pData+txCount,&canRxMsg.canData[1],(canRxMsg.dlc - 1));
                                txCount += (canRxMsg.dlc - 1);

                                if(txCount < *dataLen)
                                {
                                    return -1;
                                }
                                else
                                {
                                    rxLenLeft = 0;
                                    return 0;
                                }
                            }
                        }
                    }
                }
                break;
            }

        }
        else
        {
            return -1;
        }

    }
    //return -1;
}
*/
/*************************************************
  Function:       UdsTpPhyAddressSetCanId
  Description:    USD TP物理寻址设置canID
  Input:          handle：udsTp句柄
                  reqCanId：诊断请求ID
                  respCanId：对应的响应ID
  Output:         无
  Return:
  Others:
*************************************************/
void UdsTpPhyAddressSetCanId(int16_t handle, uint32_t reqCanId, uint32_t respCanId)
{
    if (handle < 0 || handle >= REMOTE_DIAGNOSTIC_INSTANCE_NUM)
    {
        return;
    }
    g_udsTp[handle].physicalRequestId = reqCanId;
    g_udsTp[handle].physicalResponseId = respCanId;
    return;
}

/*************************************************
  Function:       UdsTpSetFunctionalResponseId
  Description:    设置功能寻址响应ID范围
  Input:          handle：udsTp句柄
                  responseIdMin：响应ID最小值
                  responseIdMax：响应ID最大值
  Output:         无
  Return:
  Others:
*************************************************/
void UdsTpSetFunctionalResponseId(int16_t handle, uint32_t responseIdMin, uint32_t responseIdMax)
{
    if (handle < 0 || handle >= REMOTE_DIAGNOSTIC_INSTANCE_NUM)
    {
        return;
    }
    g_udsTp[handle].functionalResponseIdMin = responseIdMin;
    g_udsTp[handle].functionalResponseIdMax = responseIdMax;
    // TBOX_PRINT("[UdsTp] Set functional response ID range: min=0x%X, max=0x%X\r\n", responseIdMin, responseIdMax);
    return;
}

/*************************************************
  Function:       UdsTpSetFilter
  Description:    UDS诊断设置filter
  Input:          handle：UdsTp 通信句柄
                  respType：诊断请求类型0：物理寻址 1：功能寻址
  Output:         无
  Return:
  Others:
*************************************************/
void UdsTpSetFilter(int16_t handle, uint8_t respType)
{
    CanHalMsgFilter_t canFilter;
    if (handle < 0 || handle >= REMOTE_DIAGNOSTIC_INSTANCE_NUM)
    {
        TBOX_PRINT("handle=%d, return!\n", handle);
        return;
    }
    // 功能寻址？
    if (respType == 1)
    {
        if (NULL == g_udsTp[handle].pFunctionalEcuBuf)
        {
            canFilter.useStdFilter = 0x00;
            canFilter.canIdStdMin = g_udsTp[handle].functionalResponseIdMin;
            canFilter.canIdStdMax = g_udsTp[handle].functionalResponseIdMax;
            canFilter.useExdtFilter = 0x00;
            canFilter.canIdExtdMin = 0x00;
            canFilter.canIdExtdMax = 0xFFFFFFFF;
        }
        else
        {
            canFilter.useStdFilter = 0x01;
            canFilter.canIdStdMin = g_udsTp[handle].functionalResponseIdMin;
            canFilter.canIdStdMax = g_udsTp[handle].functionalResponseIdMax;
            canFilter.useExdtFilter = 0x00;
            canFilter.canIdExtdMin = 0x00;
            canFilter.canIdExtdMax = 0xFFFFFFFF;
        }
    }
    else
    {
        // 根据responseId判断是标准帧还是扩展帧
        uint32_t responseId = g_udsTp[handle].physicalResponseId;
        if (responseId > 0x7FF)  // 扩展帧
        {
            canFilter.useStdFilter = 0x00;  // 不使用标准帧滤波
            canFilter.canIdStdMin = 0x00;
            canFilter.canIdStdMax = 0x7FF;
            canFilter.useExdtFilter = 0x01;  // 启用扩展帧滤波
            canFilter.canIdExtdMin = responseId;
            canFilter.canIdExtdMax = responseId;
            // TBOX_PRINT("[UdsTp] Set extended frame filter: responseId=0x%X\r\n", responseId);
        }
        else  // 标准帧
        {
            canFilter.useStdFilter = 0x01;  // 启用标准帧滤波
            canFilter.canIdStdMin = responseId;
            canFilter.canIdStdMax = responseId;
            canFilter.useExdtFilter = 0x00;  // 不使用扩展帧滤波
            canFilter.canIdExtdMin = 0x00;
            canFilter.canIdExtdMax = 0xFFFFFFFF;
            // TBOX_PRINT("[UdsTp] Set standard frame filter: responseId=0x%X\r\n", responseId);
        }
    }
    CanHalSetFilter(g_udsTp[handle].canHandle, &canFilter);
    return;
}

/*************************************************
  Function:       UdsTpClearRecvBuffer
  Description:    UDS诊断请求请求清空接收buf
  Input:          handle：UdsTp 通信句柄
  Output:         无
  Return:
  Others:
*************************************************/
void UdsTpClearRecvBuffer(int16_t handle)
{
    int16_t ret;
    int16_t canHandle;
    CanHalMsg_t canRxMsg;
    canHandle = g_udsTp[handle].canHandle;
    while (1)
    {
        ret = CanHalReceive(canHandle, &canRxMsg, 0);
        if (ret != 0)
        {
            break;
        }
    }
    return;
}

/*************************************************
  Function:       UdsTpSetPendingCallback
  Description:    设置pending响应回调函数
  Input:          handle：udsTp句柄
                  callback：回调函数指针
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         当收到pending响应(7F SID 78)时调用此回调
*************************************************/
int16_t UdsTpSetPendingCallback(int16_t handle, UdsTpPendingCallback_t callback)
{
    if (handle < 0 || handle >= REMOTE_DIAGNOSTIC_INSTANCE_NUM)
    {
        return -1;
    }
    g_udsTp[handle].pendingCallback = callback;
    return 0;
}
