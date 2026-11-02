#include "canTpSdk.h"
#include <string.h>
#include "FreeRTOS.h"
#include "timerHal.h"
#include "queue.h"

#define UDS_TP_MAX_NUMBER 4
#define UDS_FRAME_CANFD 1 // 0-CAN,1-CANFD

typedef struct
{
    const CanTpSdkParameter_t *pTpParameter;
    int16_t physicalCanHandle;
    int16_t functionalCanHandle;
    uint32_t physicalTransmitCanId;
    uint32_t frameInterval;
    uint8_t useFlag;
    uint8_t bootMode;
} CanTpVariable_t;

static CanTpVariable_t g_tpVariable[UDS_TP_MAX_NUMBER];

//static const uint32_t g_physicalReceiveMinCanId = 0x18DA00F1;
//static const uint32_t g_physicalReceiveMaxCanId = 0x18DAFFF1;

static int16_t GetValidTpHandle(void)
{
    uint8_t i;
    for (i = 0; i < UDS_TP_MAX_NUMBER; i++)
    {
        if (!g_tpVariable[i].useFlag)
        {
            return i;
        }
    }
    return -1;
}

int16_t CanTpSdkInitialize(uint8_t canChannle, const CanTpSdkParameter_t *pTpParameter, CanTpSdkBuffer_t *pTpBuffer)
{
    // int16_t ret;
    int16_t physicalCanHandle;
    int16_t functionCanHandle;
    int16_t tpHandle;
    if (pTpParameter == NULL)
    {
        return -1;
    }
    if (pTpBuffer == NULL)
    {
        return -1;
    }
    tpHandle = GetValidTpHandle();
    if (tpHandle < 0)
    {
        return -1;
    }
    g_tpVariable[tpHandle].pTpParameter = pTpParameter;
    physicalCanHandle = CanHalOpen(canChannle);
    if (physicalCanHandle < 0)
    {
        return -1;
    }
    g_tpVariable[tpHandle].physicalCanHandle = physicalCanHandle;
    CanHalSetRxBuffer(physicalCanHandle, pTpBuffer->physicalCanMsgBuf, sizeof(pTpBuffer->physicalCanMsgBuf) / sizeof(pTpBuffer->physicalCanMsgBuf[0]));
    functionCanHandle = CanHalOpen(canChannle);
    if (functionCanHandle < 0)
    {
        return -1;
    }
    g_tpVariable[tpHandle].functionalCanHandle = functionCanHandle;
    CanHalSetRxBuffer(functionCanHandle, pTpBuffer->functionCanMsgBuf, sizeof(pTpBuffer->functionCanMsgBuf) / sizeof(pTpBuffer->functionCanMsgBuf[0]));
    g_tpVariable[tpHandle].bootMode = 0;
    return 0;
}

int16_t CanTpSdkSetCanId(int16_t tpHandle, uint32_t pysicalRxId, uint32_t functionRxId, uint32_t txId)
{
    CanHalMsgFilter_t canFilter;

    if (tpHandle < 0)
    {
        return -1;
    }
    if (tpHandle >= UDS_TP_MAX_NUMBER)
    {
        return -1;
    }
    canFilter.useStdFilter = 0x00;
    canFilter.canIdStdMin = pysicalRxId;
    canFilter.canIdStdMax = pysicalRxId;

    canFilter.useExdtFilter = 0x01;
    canFilter.canIdExtdMin = pysicalRxId;
    canFilter.canIdExtdMax = pysicalRxId;
    CanHalSetFilter(g_tpVariable[tpHandle].physicalCanHandle, &canFilter);

    canFilter.useStdFilter = 0x00;
    canFilter.canIdStdMin = functionRxId;
    canFilter.canIdStdMax = functionRxId;

    canFilter.useExdtFilter = 0x01;
    canFilter.canIdExtdMin = functionRxId;
    canFilter.canIdExtdMax = functionRxId;
    CanHalSetFilter(g_tpVariable[tpHandle].functionalCanHandle, &canFilter);

    CanHalEnableTransmitLoopBack(g_tpVariable[tpHandle].functionalCanHandle);

    g_tpVariable[tpHandle].physicalTransmitCanId = txId;
    return 0;
}

static int16_t TpDataSingleFrameRx(CanHalMsg_t *pCanRxMsg, uint8_t *udsData, uint16_t *udsLen)
{
    if (pCanRxMsg->dlc <= 8)
    {
        if (((pCanRxMsg->canData[0] & 0x0f) > 7) || ((pCanRxMsg->canData[0] & 0x0f) < 1))
        {
            return -1;
        }

        memcpy(udsData, &pCanRxMsg->canData[1], 7);
        *udsLen = pCanRxMsg->canData[0] & 0x0F;
        if (*udsLen > (pCanRxMsg->dlc - 1))
        {
            return -1;
        }
    }
#if (UDS_FRAME_CANFD)
    else if ((pCanRxMsg->dlc > 8) && (pCanRxMsg->dlc <= 64))
    {
        if ((pCanRxMsg->canData[0] != 0) || (pCanRxMsg->canData[1] < 8) || (pCanRxMsg->canData[1] > 63))
        {
            return -1;
        }

        memcpy(udsData, &pCanRxMsg->canData[2], 62); // CANFD 单帧有效数据从byte2开始
        *udsLen = pCanRxMsg->canData[1];
        if (*udsLen > (pCanRxMsg->dlc - 2))
        {
            return -1;
        }
    }
#endif
    else
    {
        return -1;
    }

    return 0;
}

static int16_t TpDataMultiFrameRxFLowControlTransmit(int16_t tpHandle, int16_t canHandle)
{
    CanHalMsg_t canTxMsg;
    int16_t ret;

    canTxMsg.dlc = 8;
    canTxMsg.canData[0] = 0x30;
    if (tpHandle < 0 || tpHandle >= UDS_TP_MAX_NUMBER)
    {
        return -1;
    }
    if (g_tpVariable[tpHandle].bootMode)
    {
        canTxMsg.canData[1] = g_tpVariable[tpHandle].pTpParameter->blockSize_boot; // blockSize;
        canTxMsg.canData[2] = g_tpVariable[tpHandle].pTpParameter->STmin_boot;     // STmin;
    }
    else
    {
        canTxMsg.canData[1] = g_tpVariable[tpHandle].pTpParameter->blockSize_APP; // blockSize;
        canTxMsg.canData[2] = g_tpVariable[tpHandle].pTpParameter->STmin_APP;     // STmin;
    }
    canTxMsg.canData[3] = g_tpVariable[tpHandle].pTpParameter->fillByte;
    canTxMsg.canData[4] = g_tpVariable[tpHandle].pTpParameter->fillByte;
    canTxMsg.canData[5] = g_tpVariable[tpHandle].pTpParameter->fillByte;
    canTxMsg.canData[6] = g_tpVariable[tpHandle].pTpParameter->fillByte;
    canTxMsg.canData[7] = g_tpVariable[tpHandle].pTpParameter->fillByte;
    canTxMsg.canId = g_tpVariable[tpHandle].physicalTransmitCanId;

#if (UDS_FRAME_CANFD == 0)
    ret = CanHalTransmitQueued(canHandle, canTxMsg.canId, canTxMsg.canData, canTxMsg.dlc, 1, CAN_TX_PRIO_NORMAL);
#else
    ret = CanHalTransmitQueued(canHandle, canTxMsg.canId, canTxMsg.canData, canTxMsg.dlc, 1, CAN_TX_PRIO_NORMAL);
#endif
    if (0 != ret)
    {
        return -1;
    }
    return 0;
}

//#if (UDS_FRAME_CANFD == 0)
/*******************************************************
-2:receive new start frame
-1:receive error
0:rx success



*********************************************************/
static int16_t TpDataMultiFrameRx(int16_t tpHandle, int16_t canHandle, uint16_t FrameSum, uint16_t blockSize, uint16_t rxLen, uint8_t *udsData, CanHalMsg_t *pRxmsg)
{
    uint16_t FrameCount, i, rxLenLeft, CF_num, rxCount;
    int16_t ret;

    CF_num = 0x21;
    FrameCount = 0;
    rxLenLeft = rxLen;
    rxCount = 6;
    if (tpHandle >= UDS_TP_MAX_NUMBER || tpHandle < 0)
    {
        return -1;
    }

    i = 0;
    while (i < FrameSum)
    {
        if ((FrameCount >= blockSize) && (blockSize != 0))
        {
            ret = TpDataMultiFrameRxFLowControlTransmit(tpHandle, canHandle);
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
            ret = CanHalDiagnosticReceive(canHandle, pRxmsg, g_tpVariable[tpHandle].pTpParameter->N_Cr_enforced);
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
            ret = CanHalDiagnosticReceive(canHandle, pRxmsg, g_tpVariable[tpHandle].pTpParameter->N_Cr_enforced);
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
//#endif
#if 0
static int16_t TpDataMultiFrameRxFd(int16_t tpHandle, int16_t canHandle, uint16_t FrameSum, uint16_t blockSize, uint16_t rxLen, uint8_t *udsData, CanHalMsg_t *pRxmsg)
{
    uint16_t FrameCount, i, rxLenLeft, CF_num, rxCount;
    int16_t ret;

    CF_num = 0x21;
    FrameCount = 0;
    rxLenLeft = rxLen;
    rxCount = 62;
    if (tpHandle >= UDS_TP_MAX_NUMBER || tpHandle < 0)
    {
        return -1;
    }

    i = 0;
    while (i < FrameSum)
    {
        if ((FrameCount >= blockSize) && (blockSize != 0))
        {
            ret = TpDataMultiFrameRxFLowControlTransmit(tpHandle, canHandle);
            if (0 != ret)
            {
                return -1;
            }
            FrameCount = 0;
        }
        if (rxLenLeft > 63)
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
            ret = CanHalDiagnosticReceive(canHandle, pRxmsg, g_tpVariable[tpHandle].pTpParameter->N_Cr_enforced);
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
            rxLenLeft -= 63;
            FrameCount++;
        }
        else
        {

            // ret = CanHalDiagnosticReceive(canHandle,&canRxMsg,1000);
            ret = CanHalDiagnosticReceive(canHandle, pRxmsg, g_tpVariable[tpHandle].pTpParameter->N_Cr_enforced);
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
#endif
static int16_t TpDataReceive(int16_t tpHandle, int16_t canHandle, uint8_t *udsData, uint16_t *udsLen)
{
    CanHalMsg_t canRxMsg;
    int16_t ret;
    uint16_t rxLenLeft;
    uint8_t n_PCItype;
    uint8_t blockSize;
    //    uint8_t STmin;
    // uint8_t FrameCount;
    // uint8_t CF_num = 0x21;
    // uint8_t fillByte;
    uint8_t FrameSum = 0;

    if (tpHandle >= UDS_TP_MAX_NUMBER || tpHandle < 0)
    {
        return -1;
    }
    blockSize = g_tpVariable[tpHandle].pTpParameter->blockSize_APP;
    //    STmin = g_tpVariable[tpHandle].pTpParameter->STmin_APP;
    // fillByte = 0xcc;

    if (udsData == NULL)
    {
        return -1;
    }
    if (udsLen == NULL)
    {
        return -1;
    }

    ret = CanHalDiagnosticReceive(canHandle, &canRxMsg, 0);
    if (0 == ret)
    {
        do
        {
            if (canHandle == g_tpVariable[tpHandle].functionalCanHandle)
            {
                if ((canRxMsg.canData[0] >> 4) & 0x0F)
                {
                    return -1;
                }
            }

            n_PCItype = (canRxMsg.canData[0] >> 4) & 0x0F;
            if (0 == n_PCItype)
            {
                return TpDataSingleFrameRx(&canRxMsg, udsData, udsLen);
            }
            else if (1 == n_PCItype)
            {

                rxLenLeft = (((uint16_t)canRxMsg.canData[0] & 0x0F) << 8) + canRxMsg.canData[1];
                *udsLen = rxLenLeft;
                
                if (rxLenLeft < 8)
                {
                    return -1;
                }

                memcpy(udsData, &canRxMsg.canData[2], 6);
                rxLenLeft -= 6;

                FrameSum = rxLenLeft / 7;
                if ((rxLenLeft % 7) != 0)
                {
                    FrameSum += 1;
                }

                ret = TpDataMultiFrameRxFLowControlTransmit(tpHandle, canHandle);
                if (0 != ret)
                {
                    return -1;
                }
                
                ret = TpDataMultiFrameRx(tpHandle, canHandle, FrameSum, blockSize, rxLenLeft, udsData, &canRxMsg);
                
                if (ret < 0)
                {
                    if (-2 == ret)
                    {
                        continue;
                    }
                    else
                    {
                        return -1;
                    }
                }
                if (ret < *udsLen)
                {
                    return -1;
                }
                return 0;
            }
            else if (2 == n_PCItype)
            {
                return -1;
            }
            else if (3 == n_PCItype)
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

int16_t CanTpSdkDataReceive(int16_t tpHandle, uint8_t *udsData, uint16_t *udsLen, uint8_t *pFunctionFlag)
{
    int16_t ret;
    if (tpHandle < 0 || tpHandle >= UDS_TP_MAX_NUMBER)
    {
        return -1;
    }

    ret = TpDataReceive(tpHandle, g_tpVariable[tpHandle].physicalCanHandle, udsData, udsLen);
    if (ret == 0)
    {
        *pFunctionFlag = 0;
        return ret;
    }

    ret = TpDataReceive(tpHandle, g_tpVariable[tpHandle].functionalCanHandle, udsData, udsLen);
    if (ret == 0)
    {
        *pFunctionFlag = 1;
        // return ret;
    }
    return ret;
}

static int16_t CanTpDataMultFramGetFlowControl(int16_t tpHandle, int16_t canHandle, CanHalMsg_t *pRxMsg, uint8_t *pSTmin, uint8_t *pBlocksize)
{
    int16_t ret, result;
    result = 0;
    if (tpHandle < 0 || tpHandle >= UDS_TP_MAX_NUMBER)
    {
        return -1;
    }
    while (1)
    {
        ret = CanHalDiagnosticReceive(canHandle, pRxMsg, g_tpVariable[tpHandle].pTpParameter->N_Bs_enforced); // wait 150 ms

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
            if ((pRxMsg->canData[1] == 0x7F) && (pRxMsg->canData[3] == 0x78)) // busy state
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
        *pSTmin = pRxMsg->canData[2] + 2; // 2ms
    }
    return result;
}

#if (UDS_FRAME_CANFD == 0)
static int16_t CanTpDataMultFramTransmit(int16_t tpHandle, int16_t canHandle, uint32_t txCanId, uint32_t multiFrameNum, uint8_t *txData, uint16_t txLength)
{
    uint32_t transmitCount = 0;
    int16_t ret;
    uint8_t txCanData[8];
    uint8_t txCanDlc;
    CanHalMsg_t rxCanMsg;

    int16_t result = 0;
    transmitCount += 6;
    int32_t txFrameCount = 0;
    if (tpHandle < 0 || tpHandle >= UDS_TP_MAX_NUMBER)
    {
        return -1;
    }

    // rxobj
    canHandle = g_tpVariable[tpHandle].physicalCanHandle;

    uint8_t blockSize = 0;
    uint8_t STmin = 0; // 2ms?????
    uint32_t frameInterval;
    ret = CanTpDataMultFramGetFlowControl(tpHandle, canHandle, &rxCanMsg, &STmin, &blockSize);
    if (ret != 0)
    {
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
            txCanData[j + 1] = g_tpVariable[tpHandle].pTpParameter->fillByte;
        }

        if (frameInterval != 0)
        {
            vTaskDelay(pdMS_TO_TICKS(STmin));
        }
        ret = CanHalDiagnosticReceive(canHandle, &rxCanMsg, 0); // whl20180613
        memset(rxCanMsg.canData, 0, 8);
        ret = CanHalTransmitQueued(canHandle, txCanId, txCanData, 8, 1, CAN_TX_PRIO_NORMAL);
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
            ret = CanHalDiagnosticReceive(canHandle, &rxCanMsg, g_tpVariable[tpHandle].pTpParameter->N_Bs_enforced); // wait 150 ms
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

    canHandle = g_tpVariable[tpHandle].physicalCanHandle;
    ret = CanHalDiagnosticReceive(canHandle, &rxCanMsg, 0); // wait 150 ms
    memset(rxCanMsg.canData, 0, 8);
    canHandle = g_tpVariable[tpHandle].functionalCanHandle;
    ret = CanHalDiagnosticReceive(canHandle, &rxCanMsg, 0); // wait 150 ms
    memset(rxCanMsg.canData, 0, 8);

    /*if (txFrameCount >= multiFrameNum)
    {
        break;//transmit success
    }*/

    return result;
}
#endif

// static int16_t CanTpDataMultFramTransmitFd(int16_t tpHandle,int16_t canHandle,uint32_t txCanId,uint32_t multiFrameNum,uint8_t *txData,uint16_t txLength)
// {
//     uint32_t transmitCount = 0;
//     int16_t ret;
//     uint8_t txCanData[64];
//     uint8_t txCanDlc;
//     CanHalMsg_t rxCanMsg;

//     int16_t result = 0;
//     transmitCount += 6;
//     int32_t txFrameCount = 0;
//     if(tpHandle < 0 || tpHandle >= UDS_TP_MAX_NUMBER)
//     {
//     	return -1;
//     }

//     //rxobj
//     canHandle = g_tpVariable[tpHandle].physicalCanHandle;

//     uint8_t blockSize = 0;
//     uint8_t STmin = 0; //2ms?????
//     uint32_t frameInterval;
//     ret = CanTpDataMultFramGetFlowControl(tpHandle,canHandle,&rxCanMsg,&STmin,&blockSize);
//     if(ret!=0)
//     {
//         return -1;
//     }
//     if (STmin == 0)
//     {
//         frameInterval = 0;
//     }
//     else if (STmin < 128)
//     {
//         frameInterval = STmin;
//     }
//     else
//     {
//         frameInterval = 1;
//     }
//     multiFrameNum -= 1;
//     for (txFrameCount = 0; txFrameCount < multiFrameNum; txFrameCount++)
//     {
//         int j;
//         txCanData[0] = (0x20 | ((txFrameCount + 1) & 0x0f));
//         if (txFrameCount < (multiFrameNum - 1))
//         {
//             txCanDlc = 64;
//         }
//         else
//         {
//             txCanDlc = ((txLength - 62 - txFrameCount * 63) + 1);
//         }
//         for (j = 0; j < (txCanDlc-1); j++)
//         {
//             txCanData[j + 1] = txData[62 + txFrameCount * 63 + j];

//         }

//         for(;j<63;j++)
//         {
//             txCanData[j + 1] = g_tpVariable[tpHandle].pTpParameter->fillByte;
//         }

//         if(frameInterval != 0)
//         {
//             vTaskDelay( pdMS_TO_TICKS(STmin));
//         }
//         ret = CanHalDiagnosticReceive(canHandle,&rxCanMsg,0); //whl20180613
//         memset(rxCanMsg.canData, 0, 64);
//         ret = CanHalTransmitQueued(canHandle,txCanId,txCanData,64,0, CAN_TX_PRIO_NORMAL);
//         //TBOX_PRINT("can tp wait send %x\r\n",txCanId);
//         if (ret != 0)
//         {
//             result = -1;
//             break;
//         }
//         memset(rxCanMsg.canData, 0, 64);

//         //transmitCount+
//         if ((blockSize != 0) && ((txFrameCount + 1) >= blockSize))
//         {
//             //break;//block transmit finished
//             //ret = CanHalDiagnosticReceive(canHandle,&rxCanMsg, m_flowControlTime);//wait 1000 ms
//             ret = CanHalDiagnosticReceive(canHandle,&rxCanMsg, g_tpVariable[tpHandle].pTpParameter->N_Bs_enforced);//wait 150 ms
//             if (ret != 0)
//             {
//                 result = -1;
//                 break;//time out
//             }
//             if (rxCanMsg.dlc < 3)
//             {
//                 result = -1;
//                 break;//error dlc
//             }
//             if (0x30 != (rxCanMsg.canData[0] & 0xF0))
//             {
//                 result = -1;
//                 break;//not flow control frame
//             }
//             else
//             {
//                 blockSize += blockSize;
//             }
//          }
//     }

//     canHandle = g_tpVariable[tpHandle].physicalCanHandle;
//     ret = CanHalDiagnosticReceive(canHandle,&rxCanMsg, 0);//wait 150 ms
//     memset(rxCanMsg.canData, 0, 64);
//     canHandle = g_tpVariable[tpHandle].functionalCanHandle;
//     ret = CanHalDiagnosticReceive(canHandle,&rxCanMsg, 0);//wait 150 ms
//     memset(rxCanMsg.canData, 0, 64);

//     /*if (txFrameCount >= multiFrameNum)
//     {
//         break;//transmit success
//     }*/

//     return result;
// }
// static int16_t CanTpDataMultFramTransmitFd(int16_t tpHandle, int16_t canHandle, uint32_t txCanId, uint32_t multiFrameNum_UNUSED, uint8_t *txData, uint16_t txLength)
// {
//     int16_t ret;
//     uint8_t txCanData[64];                // CAN FD 帧数据缓冲区
//     uint8_t txCanDlc = 64;                // CAN FD 通常使用 64 字节 DLC
//     CanHalMsg_t rxCanMsg;                 // 用于接收流控帧
//     int16_t result = 0;                   // 函数返回值，默认为成功
//     uint32_t bytesSentAfterFF = 0;        // FF 之后已发送的数据字节数
//     uint32_t totalDataBytesToSendAfterFF; // FF 之后总共需要发送的数据字节数
//     uint8_t sequenceNumber = 1;           // CF 帧的序列号 (从 1 开始，0-F 循环)
//     uint8_t blockSize = 0;                // 从 FC 获取的块大小 (0 表示不分块)
//     uint8_t STmin = 0;                    // 从 FC 获取的最小帧间隔 (原始值)
//     uint32_t frameInterval = 0;           // 计算得到的实际帧间隔 (ms 或 ticks)
//     uint32_t currentBlockCount = 0;       // 当前块内已发送的 CF 帧数

//     // --- 1. 输入参数检查和句柄有效性检查 ---
//     if (tpHandle < 0 || tpHandle >= UDS_TP_MAX_NUMBER || txData == NULL || txLength <= 62)
//     {
//         // 如果句柄无效，或数据指针为空，或数据长度不足以构成多帧，则返回错误
//         // (理论上 txLength <= 62 不应在此函数被调用，应在 CanTpSdkDataTransmit 中处理)
//         return -1;
//     }
//     // FF 已经发送了 62 字节
//     totalDataBytesToSendAfterFF = txLength - 62;

//     // --- 2. 获取初始流控帧 (FC) ---
//     ret = CanTpDataMultFramGetFlowControl(tpHandle, canHandle, &rxCanMsg, &STmin, &blockSize);
//     if (ret != 0)
//     {
//         // 获取流控失败 (超时或收到无效帧)
//         return -1;
//     }

//     // --- 3. 计算帧间隔 (STmin) ---
//     // 根据 ISO 15765-2 标准解析 STmin 值
//     if (STmin == 0)
//         frameInterval = 0; // 0ms
//     else if (STmin <= 0x7F)
//         frameInterval = STmin; // 1ms - 127ms
//     else if (STmin >= 0xF1 && STmin <= 0xF9)
//         frameInterval = 1; // 100us - 900us, 向上取整为 1ms 以便 vTaskDelay 处理
//     else
//         frameInterval = 1; // 其他保留值或无效值按最小间隔处理 (或根据具体需求调整)

//     // --- 4. 循环发送连续帧 (CF) ---
//     while (bytesSentAfterFF < totalDataBytesToSendAfterFF)
//     {
//         // --- 4.1 检查是否需要等待下一个流控帧 (Block Size 控制) ---
//         if (blockSize != 0 && currentBlockCount >= blockSize)
//         {
//             // 当前块已发送完毕，等待下一个流控帧
//             ret = CanTpDataMultFramGetFlowControl(tpHandle, canHandle, &rxCanMsg, &STmin, &blockSize);
//             if (ret != 0)
//             {
//                 // 获取下一个流控失败
//                 result = -1;
//                 break;
//             }
//             // 重新计算帧间隔
//             if (STmin == 0)
//                 frameInterval = 0;
//             else if (STmin <= 0x7F)
//                 frameInterval = STmin;
//             else if (STmin >= 0xF1 && STmin <= 0xF9)
//                 frameInterval = 1;
//             else
//                 frameInterval = 1;

//             currentBlockCount = 0; // 重置块内计数
//         }

//         // --- 4.2 准备当前 CF 帧的数据 ---
//         memset(txCanData, g_tpVariable[tpHandle].pTpParameter->fillByte, sizeof(txCanData)); // 使用配置的填充字节填充缓冲区
//         txCanData[0] = (0x20 | (sequenceNumber & 0x0F));                                     // 设置 PCI: 类型=CF (0x2), SN=sequenceNumber

//         // 计算本帧能发送的数据字节数 (最多 63 字节)
//         uint16_t dataBytesInThisFrame = (totalDataBytesToSendAfterFF - bytesSentAfterFF >= 63) ? 63 : (totalDataBytesToSendAfterFF - bytesSentAfterFF);

//         // 计算数据在原始 txData 缓冲区中的起始偏移量
//         uint16_t dataOffsetInTxData = 62 + bytesSentAfterFF; // FF 发送了 62 字节，之后的数据从索引 62 开始

//         // 从原始数据缓冲区复制数据到当前 CAN 帧缓冲区
//         memcpy(&txCanData[1], &txData[dataOffsetInTxData], dataBytesInThisFrame);

//         // --- 4.3 等待最小帧间隔 (STmin) ---
//         if (frameInterval > 0)
//         {
//             // STmin > 0, 延时指定的毫秒数
//             vTaskDelay(pdMS_TO_TICKS(frameInterval));
//         }
//         else
//         {
//             // STmin = 0, 不延时，但让出 CPU 以免 starving 其他任务
//             taskYIELD();
//         }

//         // --- 4.4 发送当前 CF 帧 ---
//         ret = CanHalTransmitQueued(canHandle, txCanId, txCanData, txCanDlc, 1, CAN_TX_PRIO_NORMAL);
//         if (ret != 0)
//         {
//             // 底层 CAN 发送失败
//             result = -1;
//             break;
//         }

//         // --- 4.5 更新状态变量 ---
//         bytesSentAfterFF += dataBytesInThisFrame; // 更新已发送的总字节数
//         currentBlockCount++;                      // 更新当前块内发送的帧数

//         // 更新序列号 (SN)，处理 0x0F 到 0x00 的回绕
//         sequenceNumber = (sequenceNumber + 1) & 0x0F;

//     } // end while loop for sending CFs

//     // --- 5. 清理接收缓冲区 (可选，防止旧的/意外的 FC 帧干扰下次传输) ---
//     // CanHalReceive(canHandle, &rxCanMsg, 0);

//     return result; // 返回 0 表示成功, -1 表示失败
// }

static int16_t CanTpDataMultFramTransmitFd(int16_t tpHandle, int16_t canHandle, uint32_t txCanId, uint32_t multiFrameNum, uint8_t *txData, uint16_t txLength)
{
    int16_t ret;
    uint8_t txCanData[8]; 
    uint8_t txCanDlc;
    CanHalMsg_t rxCanMsg;

    int16_t result = 0;
    int32_t txFrameCount = 0;
    
    if(tpHandle < 0 || tpHandle >= UDS_TP_MAX_NUMBER)
    {
        return -1;
    }

    canHandle = g_tpVariable[tpHandle].physicalCanHandle;

    uint8_t blockSize = 0;
    uint8_t STmin = 0; 
    uint32_t frameInterval;
    
    ret = CanTpDataMultFramGetFlowControl(tpHandle, canHandle, &rxCanMsg, &STmin, &blockSize);
    if(ret != 0)
    {
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
            txCanData[j + 1] = g_tpVariable[tpHandle].pTpParameter->fillByte;
        }
        txCanDlc = 8; 

        if (frameInterval != 0)
        {
            vTaskDelay(pdMS_TO_TICKS(STmin));
        }
        
        ret = CanHalDiagnosticReceive(canHandle, &rxCanMsg, 0); 
        memset(rxCanMsg.canData, 0, 8);
        
        ret = CanHalTransmitQueued(canHandle, txCanId, txCanData, 8, 1, CAN_TX_PRIO_NORMAL);
        
        if (ret != 0)
        {
            result = -1;
            break;
        }
        
        if ((blockSize != 0) && ((txFrameCount + 1) >= blockSize))
        {
            ret = CanHalDiagnosticReceive(canHandle, &rxCanMsg, g_tpVariable[tpHandle].pTpParameter->N_Bs_enforced);
            if (ret != 0)
            {
                result = -1;
                break;
            }
            if (rxCanMsg.dlc < 3)
            {
                result = -1;
                break;
            }
            if (0x30 != (rxCanMsg.canData[0] & 0xF0))
            {
                result = -1;
                break;
            }
            else
            {
                blockSize += blockSize; 
            }
        }
    }

    canHandle = g_tpVariable[tpHandle].physicalCanHandle;
    ret = CanHalDiagnosticReceive(canHandle, &rxCanMsg, 0);
    
    canHandle = g_tpVariable[tpHandle].functionalCanHandle;
    ret = CanHalDiagnosticReceive(canHandle, &rxCanMsg, 0);

    return result;
}

int16_t CanTpSdkDataTransmit(int16_t tpHandle, uint32_t canId, uint8_t *txData, uint16_t txLength, uint8_t functionAddFlag)
{
    int16_t canHandle;
    uint32_t txCanId;
    txCanId = canId;
    if (tpHandle >= UDS_TP_MAX_NUMBER || tpHandle < 0)
    {
        return -1;
    }
    if (functionAddFlag)
    {
        canHandle = g_tpVariable[tpHandle].functionalCanHandle;
    }
    else
    {
        canHandle = g_tpVariable[tpHandle].physicalCanHandle;
    }
#if (UDS_FRAME_CANFD == 0)
    if (txLength < 8) // single frame
    {
        uint8_t data[8];
        data[0] = txLength;
        uint16_t i = 0;
        for (i = 0; i < txLength; i++)
        {
            data[i + 1] = txData[i];
        }
        for (; i < 7; i++)
        {
            data[i + 1] = g_tpVariable[tpHandle].pTpParameter->fillByte;
        }
        return CanHalTransmitQueued(canHandle, txCanId, data, 8, 1, CAN_TX_PRIO_NORMAL);
    }

    uint8_t txCanData[8];
    uint8_t txCanDlc;

    txCanDlc = 0;
    uint32_t multiFrameNum = (txLength + 1) / (8 - 1);
    if ((txLength + 1) % (8 - 1) != 0)
    {
        multiFrameNum += 1;
    }
    txCanData[0] = (0x10 | (txLength / 256));
    txCanData[1] = (txLength % 256);
    for (int i = 0; i < 6; i++)
    {
        txCanData[i + 2] = txData[i];
    }
    txCanDlc = 8;

    int16_t ret = CanHalTransmitQueued(canHandle, txCanId, txCanData, txCanDlc, 1, CAN_TX_PRIO_NORMAL);
    if (ret != 0)
    {
        return -1;
    }
    ret = CanTpDataMultFramTransmit(tpHandle, canHandle, txCanId, multiFrameNum, txData, txLength);
    if (ret != 0)
    {
        return -1;
    }
#else
    if (txLength < 8) // single frame
    {
        uint8_t data[8];
        data[0] = txLength;
        uint16_t i = 0;
        for (i = 0; i < txLength; i++)
        {
            data[i + 1] = txData[i];
        }
        for (; i < 7; i++)
        {
            data[i + 1] = g_tpVariable[tpHandle].pTpParameter->fillByte;
        }
        return CanHalTransmitQueued(canHandle, txCanId, data, 8, 1, CAN_TX_PRIO_NORMAL);
    }

    // if (txLength < 63) // single frame
    // {
    //     uint8_t data[64];
    //     uint8_t txCanDlc;

    //     if (txLength <= 10)
    //     {
    //         txCanDlc = 12;
    //     }
    //     else if ((txLength >= 11) && (txLength <= 14))
    //     {
    //         txCanDlc = 16;
    //     }
    //     else if ((txLength >= 15) && (txLength <= 18))
    //     {
    //         txCanDlc = 20;
    //     }
    //     else if ((txLength >= 19) && (txLength <= 22))
    //     {
    //         txCanDlc = 24;
    //     }
    //     else if ((txLength >= 23) && (txLength <= 30))
    //     {
    //         txCanDlc = 32;
    //     }
    //     else if ((txLength >= 31) && (txLength <= 46))
    //     {
    //         txCanDlc = 48;
    //     }
    //     else if ((txLength >= 47) && (txLength <= 62))
    //     {
    //         txCanDlc = 64;
    //     }

    //     data[0] = 0;
    //     data[1] = txLength;
    //     uint16_t i = 0;
    //     for (i = 0; i < txLength; i++)
    //     {
    //         data[i + 2] = txData[i];
    //     }
    //     for (; i < (txCanDlc - 2); i++)
    //     {
    //         data[i + 2] = g_tpVariable[tpHandle].pTpParameter->fillByte;
    //     }
    //     return CanHalTransmitQueued(canHandle, txCanId, data, txCanDlc, 1, CAN_TX_PRIO_NORMAL);
    // }

    uint8_t txCanData[8];
    uint8_t txCanDlc;

    uint32_t multiFrameNum = (txLength + 1) / (8 - 1);
    if ((txLength + 1) % (8 - 1) != 0)
    {
        multiFrameNum += 1;
    }

    txCanData[0] = (0x10 | (txLength / 256));
    txCanData[1] = (txLength % 256);
    for (int i = 0; i < 6; i++)
    {
        txCanData[i + 2] = txData[i];
    }
    txCanDlc = 8;

    int16_t ret = CanHalTransmitQueued(canHandle, txCanId, txCanData, txCanDlc, 1, CAN_TX_PRIO_NORMAL);
    if (ret != 0)
    {
        return -1;
    }

    ret = CanTpDataMultFramTransmitFd(tpHandle, canHandle, txCanId, multiFrameNum, txData, txLength);
    if (ret != 0)
    {
        return -1;
    }
#endif

    return 0;
}

void CanTpSdkSetBootMode(int16_t tpHandle, uint8_t bootModeFlag)
{
    if ((tpHandle < 0) || (tpHandle >= UDS_TP_MAX_NUMBER))
    {
        return;
    }
    g_tpVariable[tpHandle].bootMode = bootModeFlag;
}
