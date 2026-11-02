
#include "sr_log.h"
#include "data_pool.h"
#include <stdlib.h>
#include <vector>
#include <deque>
#include <mutex>
#include <thread>
#include "hash_map"
#include <errno.h>
#include "err_code.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <queue>
#include <unordered_map>
#include "common.h"
#include "can_parser.h"
#include "sr_ipc.h"
#include "can_storage.h"
#include "data_pool.h"

using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

struct CanDataInfo_t
{
    uint8_t ch;
    uint32_t canId;
    std::vector<uint8_t> canData;
    CanDataInfo_t() : ch(0), canId(0), canData() {}
    CanDataInfo_t(uint8_t _ch, uint32_t _canId, const uint8_t* _canData, size_t _canDataLen)
        : ch(_ch), canId(_canId), canData(_canData, _canData + _canDataLen) {}
    CanDataInfo_t(CanDataInfo_t&& other) noexcept
        : ch(other.ch), canId(other.canId), canData(std::move(other.canData)) {}
    CanDataInfo_t(const CanDataInfo_t &other)
        : ch(other.ch), canId(other.canId), canData(other.canData) {}
    CanDataInfo_t &operator=(const CanDataInfo_t &other)
    {
        if (this != &other)
        {
            ch = other.ch;
            canId = other.canId;
            canData = other.canData;
        }
        return *this;
    }
};

static queue<CanDataInfo_t> g_canParserQue;

__gnu_cxx::hash_map<uint32_t ,std::vector<uint8_t>> g_CanIdMap;
__gnu_cxx::hash_map<uint32_t ,uint32_t> g_CanIdTimeOutMap;
// std::unordered_map<uint64_t ,std::vector<uint8_t>> g_CanDataHisMap;
int32_t canEncodeMode = CAN_ENCODE_MOTOROLA_MSB;

int32_t CanParserComplexCal(IN uint32_t idx, IN Dp_Value_u val, bool issigned)
{
    DpSetSignalValue(idx, val, issigned);
    return RET_OK;
}

static int32_t CanParserOri2MatrixArray(IN P_Cp_Ori_Data_t pOri, OUT P_Cp_Matrix_Data_t pData)
{
    pData->type = canEncodeMode;
    pData->ch = pOri->ch;
    pData->id = pOri->id;

    // LOG_DebugHex(pOri->data, 8, "pOri->data");
    for (uint32_t i = 0; i < 8; i++)
    {
        //这里的大小端表示整个dbc文件中所有的can消息的大小端 来自dbcjson文件中的"canEncodeMode"
        uint32_t idx = (canEncodeMode == CAN_ENCODE_INTEL) ? i : 7 - i;

        for (uint32_t j = 0; j < 8; j++)
        {
            pData->u8MatrixBuf[i * 8 + j] = (pOri->data[idx] >> j) & 0x01;
        }
    }
    // LOG_DebugHex(pData->u8MatrixBuf,64,"pData->u8MatrixBuf");
    return RET_OK;
}

static int32_t CanParserOri2MatrixArray_fd(IN P_Cp_Ori_Data_fd_t pOri, OUT P_Cp_Matrix_Data_fd_t pData)
{
    pData->type = canEncodeMode;
    pData->ch = pOri->ch;
    pData->id = pOri->id;
    for (uint32_t i = 0; i < 64; i++)
    {
        uint32_t idx = (canEncodeMode == CAN_ENCODE_INTEL) ? i : 63 - i;
        for (uint32_t j = 0; j < 8; j++)
        {
            pData->u8MatrixBuf[i * 8 + j] = (pOri->data[idx] >> j) & 0x01;
        }
    }
    return RET_OK;
}

static void CanParserSignalFromIndex(IN uint32_t index, IN P_Cp_Matrix_Data_t pMatrix)
{
    P_Dp_Signal_Data_t signalAttr;
    P_Dp_Signal_t signal;
    Dp_Value_u value_u;
    bool issigned = false;
    int32_t u8StartBit = 0;

    if ((RET_OK != DpGetSignalCheckIdx(index)) || !pMatrix)
    {
        LOG_Error("param is NULL! %d \n", index);
        return;
    }

    DpGetSignalAddr(index, &signal);
    signalAttr = &(signal->stSignal);

    if (CAN_ENCODE_INTEL == canEncodeMode)
    {
        u8StartBit = signalAttr->startBit;
    }
    else
    {
        u8StartBit = (((uint8_t)(signalAttr->startBit % 8)) * 2) + 56 - signalAttr->startBit;
        if (canEncodeMode != CAN_ENCODE_MOTOROLA_LSB)
        {
            u8StartBit = u8StartBit - signalAttr->dataLen + 1;
        }
    }

    // LOG_Debug("u8StartBit:%d",u8StartBit);

    uint32_t value = 0;
    for (int32_t i = 0; i < signalAttr->dataLen; i++)
    {
        if (pMatrix->u8MatrixBuf[u8StartBit + i] == 0x01)
        {
            value += (1 << i);
        }
    }


    switch (signalAttr->signalType)
    {
    case SIGNAL_TYPE_FLOAT:
        value_u.floatVal = (value * signalAttr->accuracy + signalAttr->offset);
        break;
    case SIGNAL_TYPE_UINT8:
    case SIGNAL_TYPE_UINT16:
    case SIGNAL_TYPE_UINT32:
    case SIGNAL_TYPE_UINT64:
        value_u.uint64Val = (value * signalAttr->accuracy + signalAttr->offset);
        break;
    case SIGNAL_TYPE_INT8:
    case SIGNAL_TYPE_INT16:
    case SIGNAL_TYPE_INT32:
    case SIGNAL_TYPE_INT64:
    {
        int64_t _val = value * signalAttr->accuracy + signalAttr->offset;
        if (_val < 0)
        {
            value_u.uint64Val = -_val;
            issigned = true;
        }
        else
        {
            value_u.uint64Val = _val;
        }
        break;
    }
    default:
        LOG_Error("unknown val Type!!!!!!!!!!");
        break;
    }
    // CanParserComplexCal(idx, value_u, issigned);
    setSignalValueAndSigned(signal, value_u, issigned);
}

static void CanParserSignalFromIndex_fd(IN uint32_t index, IN P_Cp_Matrix_Data_fd_t pMatrix)
{
    P_Dp_Signal_Data_t signalAttr;
    P_Dp_Signal_t signal;
    Dp_Value_u value_u;
    bool issigned = false;
    int32_t u8StartBit = 0;

    if ((RET_OK != DpGetSignalCheckIdx(index)) || !pMatrix)
    {
        LOG_Error("param is NULL! %d \n", index);
        return;
    }

    DpGetSignalAddr(index, &signal);
    signalAttr = &(signal->stSignal);

    if (CAN_ENCODE_INTEL == canEncodeMode)
    {
        u8StartBit = signalAttr->startBit;
    }
    else
    {
        u8StartBit = (((uint8_t)(signalAttr->startBit % 8)) * 2) + 504 - signalAttr->startBit;
        if (canEncodeMode != CAN_ENCODE_MOTOROLA_LSB)
        {
            u8StartBit = u8StartBit - signalAttr->dataLen + 1;
        }
    }

    uint32_t value = 0;
    for (int32_t i = 0; i < signalAttr->dataLen; i++)
    {
        if (pMatrix->u8MatrixBuf[u8StartBit + i] == 0x01)
        {
            value += (1 << i);
        }
    }

    switch (signalAttr->signalType)
    {
    case SIGNAL_TYPE_FLOAT:
        value_u.floatVal = (value * signalAttr->accuracy + signalAttr->offset);
        break;
    case SIGNAL_TYPE_UINT8:
    case SIGNAL_TYPE_UINT16:
    case SIGNAL_TYPE_UINT32:
    case SIGNAL_TYPE_UINT64:
        value_u.uint64Val = (value * signalAttr->accuracy + signalAttr->offset);
        break;
    case SIGNAL_TYPE_INT8:
    case SIGNAL_TYPE_INT16:
    case SIGNAL_TYPE_INT32:
    case SIGNAL_TYPE_INT64:
    {
        int64_t _val = value * signalAttr->accuracy + signalAttr->offset;
        if (_val < 0)
        {
            value_u.uint64Val = -_val;
            issigned = true;
        }
        else
        {
            value_u.uint64Val = _val;
        }
        break;
    }
    default:
        LOG_Error("unknown val Type!!!!!!!!!!");
        break;
    }
    setSignalValueAndSigned(signal, value_u, issigned);
}

static uint16_t CanParserSignal(IN P_Dp_Signal_t signal, IN P_Cp_Matrix_Data_t pMatrix)
{
    // LOG_Debug("CanParserSignal:idx:%d",signal->stSignal.sigIdx);

    Dp_Value_u value_u;
    bool issigned = false;
    // DpGetSignal(idx, &Signal);
    P_Dp_Signal_Data_t signalAttr = &(signal->stSignal);
    int32_t u8StartBit = 0;
    uint32_t multipleCount = 0;

    if (CAN_ENCODE_INTEL == canEncodeMode)
    {
        u8StartBit = signalAttr->startBit;
    }
    else
    {
        u8StartBit = (((uint8_t)(signalAttr->startBit % 8)) * 2) + 56 - signalAttr->startBit;
        if (canEncodeMode != CAN_ENCODE_MOTOROLA_LSB)
        {
            u8StartBit = u8StartBit - signalAttr->dataLen + 1;
        }
    }

    // LOG_Debug("u8StartBit:%d",u8StartBit);

    uint32_t value = 0;
    for (int32_t i = 0; i < signalAttr->dataLen; i++)
    {
        if (pMatrix->u8MatrixBuf[u8StartBit + i] == 0x01)
        {
            value += (1 << i);
        }
    }

    // LOG_Debug("canparser value: %x, signalAttr->accuracy :%f, signalAttr->offset:%d ", value, signalAttr->accuracy, signalAttr->offset);

    switch (signalAttr->signalType)
    {
    case SIGNAL_TYPE_FLOAT:
        value_u.floatVal = (value * signalAttr->accuracy + signalAttr->offset);
        break;
    case SIGNAL_TYPE_UINT8:
    case SIGNAL_TYPE_UINT16:
    case SIGNAL_TYPE_UINT32:
    case SIGNAL_TYPE_UINT64:
        value_u.uint64Val = (value * signalAttr->accuracy + signalAttr->offset);
        break;
    case SIGNAL_TYPE_INT8:
    case SIGNAL_TYPE_INT16:
    case SIGNAL_TYPE_INT32:
    case SIGNAL_TYPE_INT64:
    {
        int64_t _val = value * signalAttr->accuracy + signalAttr->offset;
        if (_val < 0)
        {
            value_u.uint64Val = -_val;
            issigned = true;
        }
        else
        {
            value_u.uint64Val = _val;
        }
        break;
    }
    default:
        LOG_Error("unknown val Type!!!!!!!!!!");
        break;
    }
    // CanParserComplexCal(idx, value_u, issigned);
    setSignalValueAndSigned(signal, value_u, issigned);

    for(multipleCount = 0; multipleCount < signalAttr->multiple; multipleCount++)
    {
        CanParserSignalFromIndex((value_u.uint64Val) + signalAttr->multipleIndex + multipleCount, pMatrix);
       // LOG_Debug("%d,%lf,Signal.multipleIndex:%d\n",((guint)doubleValue),doubleValue,Signal.multipleIndex);
    }

    if(multipleCount == signalAttr->multiple)
    {
        return (value_u.uint64Val & 0xFFFF);
    }
    else
    {
        return 0;
    }
}

static uint16_t CanParserSignal_fd(IN P_Dp_Signal_t signal, IN P_Cp_Matrix_Data_fd_t pMatrix)
{
    Dp_Value_u value_u;
    bool issigned = false;
    P_Dp_Signal_Data_t signalAttr = &(signal->stSignal);
    int32_t u8StartBit = 0;
    uint32_t multipleCount = 0;

    if (CAN_ENCODE_INTEL == canEncodeMode)
    {
        u8StartBit = signalAttr->startBit;
    }
    else
    {
        u8StartBit = (((uint8_t)(signalAttr->startBit % 8)) * 2) + 504 - signalAttr->startBit;
        if (canEncodeMode != CAN_ENCODE_MOTOROLA_LSB)
        {
            u8StartBit = u8StartBit - signalAttr->dataLen + 1;
        }
    }

    uint32_t value = 0;
    for (int32_t i = 0; i < signalAttr->dataLen; i++)
    {
        if (pMatrix->u8MatrixBuf[u8StartBit + i] == 0x01)
        {
            value += (1 << i);
        }
    }

    switch (signalAttr->signalType)
    {
    case SIGNAL_TYPE_FLOAT:
        value_u.floatVal = (value * signalAttr->accuracy + signalAttr->offset);
        break;
    case SIGNAL_TYPE_UINT8:
    case SIGNAL_TYPE_UINT16:
    case SIGNAL_TYPE_UINT32:
    case SIGNAL_TYPE_UINT64:
        value_u.uint64Val = (value * signalAttr->accuracy + signalAttr->offset);
        break;
    case SIGNAL_TYPE_INT8:
    case SIGNAL_TYPE_INT16:
    case SIGNAL_TYPE_INT32:
    case SIGNAL_TYPE_INT64:
    {
        int64_t _val = value * signalAttr->accuracy + signalAttr->offset;
        if (_val < 0)
        {
            value_u.uint64Val = -_val;
            issigned = true;
        }
        else
        {
            value_u.uint64Val = _val;
        }
        break;
    }
    default:
        LOG_Error("unknown val Type!!!!!!!!!!");
        break;
    }
    // CanParserComplexCal(idx, value_u, issigned);
    setSignalValueAndSigned(signal, value_u, issigned);

    for(multipleCount = 0; multipleCount < signalAttr->multiple; multipleCount++)
    {
        CanParserSignalFromIndex_fd((value_u.uint64Val) + signalAttr->multipleIndex + multipleCount, pMatrix);
       // LOG_Debug("%d,%lf,Signal.multipleIndex:%d\n",((guint)doubleValue),doubleValue,Signal.multipleIndex);
    }

    if(multipleCount == signalAttr->multiple)
    {
        return (value_u.uint64Val & 0xFFFF);
    }
    else
    {
        return 0;
    }
}

static void CanParserSignalTimeoutFromIndex(IN uint32_t index)
{
    P_Dp_Signal_Data_t signalAttr;
    P_Dp_Signal_t signal;
    Dp_Value_u value_u;
    bool issigned = false;
    int32_t u8StartBit = 0;

    if((RET_OK != DpGetSignalCheckIdx(index)))
    {
        LOG_Error("param is NULL! %d \n", index);
        return;
    }

    DpGetSignalAddr(index, &signal);
    signalAttr = &(signal->stSignal);

    if(signalAttr->setInvalid)
    {
        if(signalAttr->invalidValue < -0.0000001)
        {
            issigned = true;
            value_u.floatVal = -signalAttr->invalidValue;
        }
        else
        {
            value_u.floatVal = signalAttr->invalidValue;
        }
        setSignalValueAndSigned(signal, value_u, issigned);
    }

}

static void CanParserSignalTimeOut(IN P_Dp_Signal_t signal)
{
    bool issigned = false;
    Dp_Value_u value_u;
    P_Dp_Signal_Data_t signalAttr = &(signal->stSignal);
    uint32_t multipleCount = 0;

    if(signalAttr->setInvalid)
    {
        if(signalAttr->invalidValue < -0.0000001)
        {
            issigned = true;
            value_u.floatVal = -signalAttr->invalidValue;
        }
        else
        {
            value_u.floatVal = signalAttr->invalidValue;
        }
        setSignalValueAndSigned(signal, value_u, issigned);
    }

    for(multipleCount = 0; multipleCount < signalAttr->multipleMax; multipleCount++)
    {
        CanParserSignalTimeoutFromIndex(signalAttr->multipleIndex + multipleCount);
       // LOG_Debug("%d,%lf,Signal.multipleIndex:%d\n",((guint)doubleValue),doubleValue,Signal.multipleIndex);
    }

}

int32_t CanParserRecvCanData(uint8_t ch, uint32_t id, std::vector<uint8_t> &data)
{
    // 1.插入到g_CanIdMap
    uint32_t key = ((ch & 0x7) << 29) + id;
    g_CanIdMap[key] = data;
    uint16_t canDataSize = data.size();
    // 2.解析出信号值
    Cp_Ori_Data_t ori;
    Cp_Ori_Data_fd_t ori_fd;
    Cp_Matrix_Data_t matrix;
    Cp_Matrix_Data_fd_t matrix_fd;
    uint16_t canIdIndex = 0;
    uint64_t canIdKey = (key << 16);

    if(canDataSize <= 8)
    {
        ori.ch = ch & 0x7;
        ori.id = id;
        memcpy(ori.data, data.data(), canDataSize);
    }
    else
    {
        ori_fd.ch = ch & 0x7;
        ori_fd.id = id;
        memcpy(ori_fd.data, data.data(), canDataSize);
    }

    GSList *firstSignal = NULL;
    if (RET_OK == DpCanSignalForeach(key, &firstSignal))
    {
        P_Dp_Signal_t pTmp = (P_Dp_Signal_t)firstSignal->data;
        g_CanIdTimeOutMap[key] = pTmp->canCircle;
        // g_CanDataHisMap[canIdKey] = data;
        if(canDataSize <= 8)
        {
            CanParserOri2MatrixArray(&ori, &matrix);
        }
        else
        {
            CanParserOri2MatrixArray_fd(&ori_fd, &matrix_fd);
        }
        for (; firstSignal; firstSignal = firstSignal->next)
        {
            pTmp = (P_Dp_Signal_t)firstSignal->data;
            if (!pTmp)
            {
                break;
            }
            if ((pTmp->stSignal.id != key))
            {
                // 这里也可以改为break，减少循环，因为链表中相同id的信号是连续存放的
                //  continue;
                break;
            }
            if(canDataSize <= 8)
            {
                canIdIndex = CanParserSignal(pTmp, &matrix);
            }
            else
            {
                canIdIndex = CanParserSignal_fd(pTmp, &matrix_fd);
            }
            if(0 != canIdIndex)
            {
                canIdKey = (key << 16) | canIdIndex;
                // g_CanDataHisMap[canIdKey] = data;
            }
        }
    }
    return RET_OK;
}

int32_t CanParserTimeoutCanData(uint8_t ch, uint32_t id)
{
    GSList *firstSignal = NULL;
    uint32_t key = ((ch & 0x7) << 29) + id;
    if (RET_OK == DpCanSignalForeach(key, &firstSignal))
    {
        P_Dp_Signal_t pTmp = (P_Dp_Signal_t)firstSignal->data;
        for (; firstSignal; firstSignal = firstSignal->next)
        {
            pTmp = (P_Dp_Signal_t)firstSignal->data;
            if (!pTmp)
            {
                break;
            }
            if ((pTmp->stSignal.id != key))
            {
                // 这里也可以改为break，减少循环，因为链表中相同id的信号是连续存放的
                //  continue;
                break;
            }
            CanParserSignalTimeOut(pTmp);
        }
    }
    return RET_OK;
}

void RecvMcuCANMsgCallBack(const uint8_t* data, const uint16_t len, void* userdata)
{
    // LOG_DebugHex(data,len,"RecvMcuCANMsgCallBack");

    if (!data || len < 2)
    {
        LOG_Error("CAN msg too short: len=%u", len);
        return;
    }

    size_t index = 0;
    uint16_t num = (uint16_t)((data[index] << 8) | data[index + 1]);
    index += 2;

    for (uint16_t i = 0; i < num; i++)
    {
        if (index + 6 > (size_t)len)
        {
            LOG_Error("Truncated CAN record header: i=%u idx=%zu len=%u need=%zu",
                      i, index, len, (size_t)6);
            return;
        }
        uint8_t ch = data[index++];
        uint32_t canId =
            ((uint32_t)data[index]     << 24) |
            ((uint32_t)data[index + 1] << 16) |
            ((uint32_t)data[index + 2] << 8 ) |
            ((uint32_t)data[index + 3]);
        index += 4;
        uint8_t datalen = data[index++];

        if (datalen > 64)
        {
            LOG_Error("CAN data length > 64, ch:%u id:0x%X len:%u", ch, canId, datalen);
            return;
        }

        if (index + datalen > (size_t)len)
        {
            LOG_Error("Truncated CAN payload: ch:%u id:0x%X need=%zu have=%u",
                      ch, canId, index + datalen, len);
            return;
        }
        // canInfo.canData = 0;
        // memcpy((char *)&(canInfo.canData), (char *)&(data[index]), datalen);
        CanDataInfo_t canInfo(ch, canId, &(data[index]), datalen);
        index += datalen;
        // LOG_Debug("[recv mcu]canmsg ch:%d,id:%x", canInfo.ch, canInfo.canId);
        // LOG_DebugHex(canInfo.canData.data(),(uint16_t)canInfo.canData.size(),"[recv mcu]canmsg data");
        // CanParserRecvCanData(ch, id, candata);
        g_canParserQue.push(canInfo);
    }

    if (index != (size_t)len)
    {
        LOG_Warn("Extra %zu bytes after parsing", (size_t)len - index);
    }
}

void RecvMcuCanParserFilterReqCallBack(const uint8_t *data, const uint16_t len, void *userdata)
{
    LOG_DebugHex(data, len, "RecvMcuCanParserFilterReqCallBack");
    uint16_t reqIndex = 0;
    uint16_t reqNumber = 0;
    if (len == (sizeof(uint16_t) + sizeof(uint16_t)))
    {
        // 大端模式
        reqIndex = static_cast<uint16_t>(data[0]) << 8 | data[1];
        reqNumber = static_cast<uint16_t>(data[2]) << 8 | data[3];
    }
    uint32_t *plist = NULL;
    uint32_t arraylen = 0;
    DpGetFilterCanid(&plist, &arraylen);

    std::vector<uint8_t> dataResp;
    // 大端模式
    dataResp.push_back((reqIndex >> 8));
    dataResp.push_back((reqIndex & 0xFF));
    uint32_t respListSize = (arraylen < reqIndex + reqNumber) ? (arraylen - reqIndex) : reqNumber;
    dataResp.push_back((respListSize >> 8));
    dataResp.push_back((respListSize & 0xFF));
    for (uint32_t i = reqIndex; (i < arraylen) && ((i - reqIndex) < reqNumber); i++)
    {
        // Filter conf Bit0-Bit4:去重配置。预留。Bit5-Bit7:CAN 通道配置。取值0-7。
        // 通道
        dataResp.push_back((plist[i] >> 29) << 5);
        // CAN ID 四个字节
        dataResp.push_back((plist[i] >> 24) & 0x1f);
        dataResp.push_back((plist[i] >> 16) & 0xff);
        dataResp.push_back((plist[i] >> 8) & 0xff);
        dataResp.push_back(plist[i] & 0xff);
    }

    McuMesg_t dataToSend;
    // 1.消息类型
    dataToSend.type = SYS_QUE_TYPE_MCU_VEHICLE_CAN_ID_RES;
    // 2.数据长度
    dataToSend.payloadLen = dataResp.size();
    // 3.数据
    dataToSend.payload = dataResp.data();

    LOG_DebugHex(dataResp.data(), dataResp.size(), "send to mcu:");
    McuSrvSendMesg(&dataToSend);

    g_free(plist);
}

void *CanParserProcess(void *arg)
{
    CanDataInfo_t canInfo;
    uint64_t tmpData = 0;

    // InitCanDataMemMap();
    // LoadCanDataFromFile(g_CanDataHisMap);

    // for (const auto& item : g_CanDataHisMap) 
    // {
    //     tmpData = item.first;
    //     canInfo.ch = (tmpData >> 45) & 0x07;
    //     canInfo.canId = (tmpData >> 16) & 0x1FFFFFFF;
    //     canInfo.canData = item.second;
    //     g_canParserQue.push(canInfo);
    // }


    while(1)
    {
        if(g_canParserQue.empty())
        {
            usleep(10000);
            continue;
        }

        canInfo = g_canParserQue.front(); 
	    g_canParserQue.pop();
        // LOG_Debug("[CanParserProcess]canInfo.ch :%d", canInfo.ch);
        // LOG_Debug("[CanParserProcess]canInfo.canId :%lx", canInfo.canId);
        // LOG_DebugHex(canInfo.canData.data(),(uint16_t)canInfo.canData.size(),"[CanParserProcess]canInfo.canData ");
        CanParserAndStructDataLock();
        CanParserRecvCanData(canInfo.ch, canInfo.canId, canInfo.canData);
        CanParserAndStructDataUnlock();
    }
}



void *RecvCanDataTimeoutProcess(void *arg)
{
    CanDataInfo_t canInfo;
    uint32_t canId;
    uint32_t canCircle;
    // uint32_t count = 0;

    while(1)
    {
        for (const auto& item : g_CanIdTimeOutMap) 
        {
            canId = item.first;
            canCircle = item.second;
            g_CanIdTimeOutMap[canId] = --canCircle;
            if(0 == canCircle)
            {
                canInfo.ch = (canId >> 29) & 0x07;
                canInfo.canId = (canId) & 0x1FFFFFFF;
                // LOG_Debug("canInfo.ch :%d", canInfo.ch);
                // LOG_Debug("canInfo.id :%lx", canInfo.canId);
                CanParserTimeoutCanData(canInfo.ch, canInfo.canId);
            }
            else
            {
                canInfo.ch = (canId >> 29) & 0x07;
                canInfo.canId = (canId) & 0x1FFFFFFF;
                // LOG_Debug("canInfo.ch :%d", canInfo.ch);
                // LOG_Debug("canInfo.id :%lx", canInfo.canId);
                // LOG_Debug("canCircle :%d", canCircle);
            }
        }
        // count++;

        // if(count >= 500)
        // {
        //     SaveCanDataFromFile(g_CanDataHisMap);
        //     count = 0;
        // }
        usleep(10000);
    }
}


#ifdef __cplusplus
}
#endif
