#include "data_pool.h"
#include "can_parser.h"
#include "../json/inc/json.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <hash_map>
#include "sr_ipc.h"
#include "queue_msg.h"

using namespace std;

static Dp_Data_t g_dpData;
static pthread_rwlock_t g_dataPoolRWLock = PTHREAD_RWLOCK_INITIALIZER;
static pthread_mutex_t structDataLock = PTHREAD_MUTEX_INITIALIZER;

template <typename T>
void copyDataAndIncrementIndex(T val, std::vector<uint8_t> &sendData)
{
    sendData.insert(sendData.end(), reinterpret_cast<uint8_t *>(&val), reinterpret_cast<uint8_t *>(&val) + sizeof(T));
}

#ifdef __cplusplus
extern "C"
{
#endif

extern __gnu_cxx::hash_map<uint32_t ,uint64_t> g_CanIdMap;
extern __gnu_cxx::hash_map<uint32_t ,uint32_t> g_CanIdTimeOutMap;
extern int32_t canEncodeMode;

#define DBC_JSON_FILE_PATH "/oemapp/config/dbc.json"
#define STRUCT_JSON_FILE_PATH "/oemapp/config/struct.json"

typedef struct
{
    uint8_t signalType;//用户设置的需要发送的数据类型"float"==1,"uint8"==2,"uint16"==3,"uint32"==4,"uint64"==5,"int8"==6,"int16"==7,"int32"==8,"int64"==9
    double userfactor;//用户发送时需要额外设置的用户因子
    double useroffset;//用户发送时需要额外设置的用户偏移
    __gnu_cxx::hash_map<int,int> usermap;//用户发送时需要额外设置的用户map
    P_Dp_Signal_t signal;
}CallbackSignalData_t;

struct CallbackData_t
{
    std::string msgname;
    std::vector<uint32_t> canIdList;
    std::vector<CallbackSignalData_t> signalsData;
    long msgid;
    CallbackData_t(std::string s, std::vector<uint32_t> c, std::vector<CallbackSignalData_t> d, long m) : msgname(s), canIdList(c), signalsData(d), msgid(m) {}
};

CallbackData_t *g_realDataUpload = NULL;

static void dealSourceSignalIsNull(const CallbackSignalData_t &iter, std::vector<uint8_t> &sendData)
{
    uint64_t value_unsigned = 0;
    switch (iter.signalType)
    {
    case SIGNAL_TYPE_FLOAT:copyDataAndIncrementIndex<float>(static_cast<float>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_UINT8:copyDataAndIncrementIndex<uint8_t>(static_cast<uint8_t>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_UINT16:copyDataAndIncrementIndex<uint16_t>(static_cast<uint16_t>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_UINT32:copyDataAndIncrementIndex<uint32_t>(static_cast<uint32_t>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_UINT64:copyDataAndIncrementIndex<uint64_t>(static_cast<uint64_t>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_INT8:copyDataAndIncrementIndex<int8_t>(static_cast<int8_t>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_INT16:copyDataAndIncrementIndex<int16_t>(static_cast<int16_t>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_INT32:copyDataAndIncrementIndex<int32_t>(static_cast<int32_t>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_INT64:copyDataAndIncrementIndex<int64_t>(static_cast<int64_t>(value_unsigned), sendData);break;
    default:break;
    }
}

static void dealSourceSignalIsFloat(const CallbackSignalData_t &iter, std::vector<uint8_t> &sendData)
{
    //处理用户因子与偏移
    double factor = (iter.userfactor != DBL_MAX) ? iter.userfactor : 1.0;
    double offset = (iter.useroffset != DBL_MAX) ? iter.useroffset : 0.0;
    pthread_rwlock_rdlock(&g_dataPoolRWLock);
    float value_float = iter.signal->value.floatVal * factor + offset;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    //float的值，再设置usermap是无效的
    //如果原始值是float 但是用户设置了一个整数，那么就取浮点数的整数部分
    switch (iter.signalType)
    {
    case SIGNAL_TYPE_FLOAT:copyDataAndIncrementIndex<float>(value_float, sendData);break;
    //用户设置了无符号数，但实际值为负值，则设置为0
    case SIGNAL_TYPE_UINT8:copyDataAndIncrementIndex<uint8_t>(static_cast<uint8_t>(value_float<0?0:value_float), sendData);break;
    case SIGNAL_TYPE_UINT16:copyDataAndIncrementIndex<uint16_t>(static_cast<uint16_t>(value_float<0?0:value_float), sendData);break;
    case SIGNAL_TYPE_UINT32:copyDataAndIncrementIndex<uint32_t>(static_cast<uint32_t>(value_float<0?0:value_float), sendData);break;
    case SIGNAL_TYPE_UINT64:copyDataAndIncrementIndex<uint64_t>(static_cast<uint64_t>(value_float<0?0:value_float), sendData);break;
    case SIGNAL_TYPE_INT8:copyDataAndIncrementIndex<int8_t>(static_cast<int8_t>(value_float), sendData);break;
    case SIGNAL_TYPE_INT16:copyDataAndIncrementIndex<int16_t>(static_cast<int16_t>(value_float), sendData);break;
    case SIGNAL_TYPE_INT32:copyDataAndIncrementIndex<int32_t>(static_cast<int32_t>(value_float), sendData);break;
    case SIGNAL_TYPE_INT64:copyDataAndIncrementIndex<int64_t>(static_cast<int64_t>(value_float), sendData);break;
    default:break;
    }
}

static void dealSourceSignalIsUint(const CallbackSignalData_t &iter, std::vector<uint8_t> &sendData)
{
    //处理用户因子与偏移
    double factor = (iter.userfactor != DBL_MAX) ? iter.userfactor : 1.0;
    double offset = (iter.useroffset != DBL_MAX) ? iter.useroffset : 0.0;
    //小端模式 按照最大的类型提取数值
    pthread_rwlock_rdlock(&g_dataPoolRWLock);
    uint64_t value_unsigned = iter.signal->value.uint64Val * factor + offset;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    //如果有usermap 处理
    if(!iter.usermap.empty())
    {
        auto it = iter.usermap.find(value_unsigned);
        if (it != iter.usermap.end())
        {
            value_unsigned = it->second;
        }
    }
    switch (iter.signalType)
    {
    case SIGNAL_TYPE_FLOAT:copyDataAndIncrementIndex<float>(static_cast<float>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_UINT8:copyDataAndIncrementIndex<uint8_t>(static_cast<uint8_t>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_UINT16:copyDataAndIncrementIndex<uint16_t>(static_cast<uint16_t>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_UINT32:copyDataAndIncrementIndex<uint32_t>(static_cast<uint32_t>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_UINT64:copyDataAndIncrementIndex<uint64_t>(static_cast<uint64_t>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_INT8:copyDataAndIncrementIndex<int8_t>(static_cast<int8_t>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_INT16:copyDataAndIncrementIndex<int16_t>(static_cast<int16_t>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_INT32:copyDataAndIncrementIndex<int32_t>(static_cast<int32_t>(value_unsigned), sendData);break;
    case SIGNAL_TYPE_INT64:copyDataAndIncrementIndex<int64_t>(static_cast<int64_t>(value_unsigned), sendData);break;
    default:break;
    }
}

static void dealSourceSignalIsInt(const CallbackSignalData_t &iter, std::vector<uint8_t> &sendData)
{
    //处理用户因子与偏移
    double factor = (iter.userfactor != DBL_MAX) ? iter.userfactor : 1.0;
    double offset = (iter.useroffset != DBL_MAX) ? iter.useroffset : 0.0;
    pthread_rwlock_rdlock(&g_dataPoolRWLock);
    uint64_t _val = iter.signal->value.uint64Val * factor + offset;
    int64_t value_signed = (iter.signal->issigned)?(-_val):_val;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    // 如果有usermap 处理
    if (!iter.usermap.empty())
    {
        auto it = iter.usermap.find(value_signed);
        if (it != iter.usermap.end())
        {
            value_signed = it->second;
        }
    }
    switch (iter.signalType)
    {
    case SIGNAL_TYPE_FLOAT:copyDataAndIncrementIndex<float>(static_cast<float>(value_signed), sendData);break;
    //原始值为有符号的，但是用户设置为无符号。当原始值为负值时，强转会数据变为一个无意义的数，特殊处理成0
    case SIGNAL_TYPE_UINT8:copyDataAndIncrementIndex<uint8_t>(static_cast<uint8_t>(value_signed<0?0:value_signed), sendData);break;
    case SIGNAL_TYPE_UINT16:copyDataAndIncrementIndex<uint16_t>(static_cast<uint16_t>(value_signed<0?0:value_signed), sendData);break;
    case SIGNAL_TYPE_UINT32:copyDataAndIncrementIndex<uint32_t>(static_cast<uint32_t>(value_signed<0?0:value_signed), sendData);break;
    case SIGNAL_TYPE_UINT64:copyDataAndIncrementIndex<uint64_t>(static_cast<uint64_t>(value_signed<0?0:value_signed), sendData);break;
    case SIGNAL_TYPE_INT8:copyDataAndIncrementIndex<int8_t>(static_cast<int8_t>(value_signed), sendData);break;
    case SIGNAL_TYPE_INT16:copyDataAndIncrementIndex<int16_t>(static_cast<int16_t>(value_signed), sendData);break;
    case SIGNAL_TYPE_INT32:copyDataAndIncrementIndex<int32_t>(static_cast<int32_t>(value_signed), sendData);break;
    case SIGNAL_TYPE_INT64:copyDataAndIncrementIndex<int64_t>(static_cast<int64_t>(value_signed), sendData);break;
    default:break;
    }
}

int32_t CanParserAndStructDataLock()
{
    pthread_mutex_lock(&structDataLock);
}

int32_t CanParserAndStructDataUnlock()
{
    pthread_mutex_unlock(&structDataLock);
}

static gboolean timer_callback(gpointer data)
{
    pthread_mutex_lock(&structDataLock);
    CallbackData_t *callbackData = static_cast<CallbackData_t *>(data);
    LOG_Debug("Timer callback called, msg id: 0x%lx,msgname:%s", callbackData->msgid, callbackData->msgname.c_str());

    std::vector<uint8_t> sendData;

    //1.首先处理是否加入原始can值
    for (auto &iter : callbackData->canIdList)
    {
        uint64_t value = g_CanIdMap[iter];
        if (value != 0)
        {
            sendData.insert(sendData.end(), reinterpret_cast<uint8_t *>(&iter), reinterpret_cast<uint8_t *>(&iter) + 4);
            sendData.insert(sendData.end(), reinterpret_cast<uint8_t *>(&value), reinterpret_cast<uint8_t *>(&value) + 8);
        }
        else
        {
            if (g_CanIdMap.find(iter) == g_CanIdMap.end())
            {
                LOG_Error("Original CAN value not found");
                sendData.insert(sendData.end(), 12, 0); // 插入12个0
            }
            else
            {
                sendData.insert(sendData.end(), reinterpret_cast<uint8_t *>(&iter), reinterpret_cast<uint8_t *>(&iter) + 4);
                sendData.insert(sendData.end(), 8, 0); // 插入8个0
            }
        }
    }
    //2.处理信号值
    for (const auto &iter : callbackData->signalsData)
    {
        if(NULL == iter.signal)
        {
            dealSourceSignalIsNull(iter,sendData);
            continue;
        }
        switch (iter.signal->stSignal.signalType)
        {
        case SIGNAL_TYPE_FLOAT://float
        {
            dealSourceSignalIsFloat(iter,sendData);
            break;
        }
        case SIGNAL_TYPE_UINT8://uint8_t
        case SIGNAL_TYPE_UINT16://uint16_t
        case SIGNAL_TYPE_UINT32://uint32_t
        case SIGNAL_TYPE_UINT64://uint64_t
        {
            dealSourceSignalIsUint(iter,sendData);
            break;
        }
        case SIGNAL_TYPE_INT8://int8_t
        case SIGNAL_TYPE_INT16://int16_t
        case SIGNAL_TYPE_INT32://int32_t
        case SIGNAL_TYPE_INT64://int64_t
        {
            dealSourceSignalIsInt(iter,sendData);
            break;
        }
        default:
            break;
        }
    }
    pthread_mutex_unlock(&structDataLock);
    LOG_Debug("pub id:0x%lx",callbackData->msgid);
    LOG_DebugHex(sendData.data(),sendData.size(),"pub data");
    SendMsgToAllQueue(static_cast<SYSQueMsgType_e>(callbackData->msgid), sendData.data(), sendData.size());
    return TRUE;
}

static void Dp_list(GSList *pList)
{
    if (!pList)
    {
        LOG_Debug("param invalid");
        return;
    }

    GSList *iterator = pList;
    LOG_Debug("----------------------------");
    for (; iterator; iterator = iterator->next) 
    {
        P_Dp_Signal_t pTmp = (P_Dp_Signal_t)iterator->data;
        LOG_Debug("id:%x,encode:%d,sigIdx:%d,startBit:%d,dataLen:%d,offset:%lf,accuracy:%f,defaultValue:%d", pTmp->stSignal.id,
            pTmp->stSignal.encode,
            pTmp->stSignal.sigIdx,
            pTmp->stSignal.startBit,
            pTmp->stSignal.dataLen,
            pTmp->stSignal.offset,
            pTmp->stSignal.accuracy,
            pTmp->stSignal.defaultValue);
    }
}

static void Dp_Init(void)
{
    if (g_dpData.pSpeedTable)
    {
        g_hash_table_destroy(g_dpData.pSpeedTable);
        g_dpData.pSpeedTable = NULL;
    }
    if (g_dpData.pSigNameTable)
    {
        g_hash_table_destroy(g_dpData.pSigNameTable);
        g_dpData.pSigNameTable = NULL;
    }
    if (g_dpData.pParserView)
    {
        g_hash_table_destroy(g_dpData.pParserView);
        g_dpData.pParserView = NULL;
    }
    if (g_dpData.pAll)
    {
        g_slist_free_full(g_dpData.pAll, g_free);
        g_dpData.pAll = NULL;
    }
    if (g_dpData.canIdFilter)
    {
        g_array_free(g_dpData.canIdFilter, TRUE);
        g_dpData.canIdFilter = NULL;
    }
    memset(&g_dpData, 0x0, sizeof(g_dpData));
}

static void setSignalTypeAndValue(P_Dp_Signal_t tmp)
{
    std::string valType(tmp->stSignal.signalTypeName);
    if (valType == "float")
    {
        tmp->stSignal.signalType = SIGNAL_TYPE_FLOAT;
        tmp->value.floatVal = static_cast<float>(tmp->stSignal.defaultValue);
        // LOG_Debug("float:%f,issigned:%d",tmp->value.floatVal,tmp->issigned);
    }
    else if (valType.substr(0, 4) == "uint")
    {
        int bitNum = std::stoi(valType.substr(4));
        if (bitNum >= 8 && bitNum <= 64)
        {
            tmp->stSignal.signalType = log(bitNum) / log(2) - 1;
        }
        else
        {
            tmp->stSignal.signalType = SIGNAL_TYPE_UINT64; // uint64
        }
        tmp->value.uint64Val = static_cast<uint64_t>(tmp->stSignal.defaultValue);
        // LOG_Debug("uint64Val:%lld,issigned:%d",tmp->value.uint64Val,tmp->issigned);
    }
    else if (valType.substr(0, 3) == "int")
    {
        int bitNum = std::stoi(valType.substr(3));
        if (bitNum >= 8 && bitNum <= 64)
        {
            tmp->stSignal.signalType = log(bitNum) / log(2) + 3;
        }
        else
        {
            tmp->stSignal.signalType = SIGNAL_TYPE_INT64; // int64
        }
        int64_t _val = static_cast<int64_t>(tmp->stSignal.defaultValue);
        tmp->value.uint64Val = _val < 0 ? (-_val) : _val;
        tmp->issigned = _val < 0 ? true : false;
        // LOG_Debug("uint64Val:%lld,issigned:%d",tmp->value.uint64Val,tmp->issigned);
    }
    else
    {
        LOG_Error("unknown val Type");
    }
}

static int32_t DpLoadConfigFromDb(IN gpointer pArg)
{
    P_Dp_Data_t pDpData = static_cast<P_Dp_Data_t>(pArg);
    printf("DpLoadConfigFromDb \n");
    if (!pDpData)
    {
        LOG_Error("pArg is NULL!");
        return RET_ERR;
    }
    //解析dbcjson
    std::ifstream is(DBC_JSON_FILE_PATH, ios::binary);
    if (!is.is_open())
    {
        LOG_Error("open json file failed.");
        return RET_ERR;
    }
     printf("open \n");
    Json::Reader reader;
    Json::Value root;
    printf("parse \n");
    if(reader.parse(is, root))
    {
        if (root["version"].isString())
        {
            stpcpy(g_dpData.version, root["version"].asCString());
        }
        else
        {
            int version = root["version"].asInt();
            std::string verStr = std::to_string(version);
            stpcpy(g_dpData.version, verStr.c_str());
        }
        LOG_Debug("Start Load Config: %d",root["data"]["canItems"].size());
    }
    else
    {
        LOG_Error("parse json file failed.");
        return RET_ERR;
    }
    printf("isMember \n");
    // 检查是否存在指定的key
    if (!(root.isMember("data") && root["data"].isMember("canItems")))
        return RET_ERR;
    printf("canEncodeMode \n");
    canEncodeMode = root["data"]["canEncodeMode"].asInt();

    P_Dp_Signal_t pCanFirst = NULL;
    for (const auto& canItem : root["data"]["canItems"])
    {
        // id为channelid和canid的组合，当接收到mcu的消息后，会根据接受到mcu的消息里面的id找到对应的解析需要的startbit等数据
        // 此处channelId不是标准DBC里面的数据，所以在生成的json中channelid默认写为了1，后续再考虑具体解决办法
        uint32_t canId = canItem["canId"].asUInt();
        uint32_t channelId = canItem["channelId"].asUInt();
        printf("canCircle \n");
        uint32_t canCircle = canItem["canCircle"].asUInt();
        uint32_t id = canId | (channelId << 29);
        pCanFirst = NULL;
        for (const auto& signal : canItem["signals"]) 
        {
            P_Dp_Signal_t tmp = static_cast<P_Dp_Signal_t>(g_try_malloc0(sizeof(Dp_Signal_t)));
            if (!tmp)
            {
                LOG_Error("malloc fail!");
        		return RET_ERR;
            }
            tmp->stSignal.id = id;
            tmp->canCircle = canCircle;
            g_CanIdTimeOutMap[id] = canCircle;
            printf("multipleMax \n");
            tmp->stSignal.multipleMax = signal["multipleMax"].asInt();
            printf("multiplexer \n");
            tmp->stSignal.multiple = signal["multiplexer"].asInt();
            printf("multiplexer_ids \n");
            tmp->stSignal.multipleIndex = signal["multiplexer_ids"].asInt();
            printf("realUpload \n");
            tmp->stSignal.realUpload = signal["realUpload"].asBool();
            printf("setInvalid \n");
            tmp->stSignal.setInvalid = signal["setInvalid"].asBool();
            printf("invalidValue \n");
            tmp->stSignal.invalidValue = signal["invalidValue"].asInt();
            //sigIdx是一个自定义的数据，每一个信号都不同，它不是标准DBC里面的数据，所以在生成的json中每个信号的sigIdx默认从1递增
            //接收到mcu的can消息后，会根据id查找所有符合的信号，然后根据这些信号中的index拿startbit等数据去计算信号val
            tmp->stSignal.sigIdx = signal["signalIndex"].asInt();
            std::string byte_order(signal["byte_order"].asCString());
            tmp->stSignal.encode = (byte_order == "big_endian") ? 1 : 0;
            tmp->stSignal.startBit = signal["startBit"].asInt();
            tmp->stSignal.dataLen = signal["length"].asInt();
            memset(tmp->stSignal.signalTypeName,0,7);
            stpcpy(tmp->stSignal.signalTypeName, signal["value_type"].asCString());
            //物理值=原始值*因子+偏移量
            //偏移量
            tmp->stSignal.offset = signal["offset"].asDouble();
            //因子
            tmp->stSignal.accuracy = signal["factor"].asDouble();
            //defaultValue，默认值，标准DBC里没有这个，这里取最小值的整数部分--- 有些项目上需要只用默认值，添加defaulutValue字段
            printf("defaultValue \n");
            tmp->stSignal.defaultValue = static_cast<int32_t>(std::floor(signal["defaultValue"].asDouble()));

            //按照f501c里面的sitech.json来看的话，所有信号的multipleIndex和multiple都是0
            //这里先暂时写死0，后续再优化
            // tmp->stSignal.multipleIndex = 0;
            // tmp->stSignal.multiple = 0;
            printf("signalName \n");
            const char *signalName = signal["signalName"].asCString();
            printf("end\n");
            setSignalTypeAndValue(tmp);

            // 所有的数据都会在pAll里面保存一份，它是一个单项链表
            pDpData->pAll = g_slist_append(pDpData->pAll, tmp);

            LOG_Debug("canid:%d,encode:%d,sigIdx:%d,startBit:%d,dataLen:%d,signalTypeName:%s,offset:%lf,accuracy:%f,defaultValue:%d",
                    tmp->stSignal.id & 0x1FFF,
                    tmp->stSignal.encode,
                    tmp->stSignal.sigIdx,
                    tmp->stSignal.startBit,
                    tmp->stSignal.dataLen,
                    tmp->stSignal.signalTypeName,
                    tmp->stSignal.offset,
                    tmp->stSignal.accuracy,
                    tmp->stSignal.defaultValue);
            // 存放到pParserView里面的数据结构为：
            // key：dbcjson里面每个信号的"channelId"<<29 | "canId"
            // value：假设该id下有8个信号，那么value就代表这个id下的第1个信号在pAll中的保存位置
            if (!pCanFirst && (tmp->stSignal.id != 0))
            {
                gboolean ret;
                pCanFirst = tmp;
                ret = g_hash_table_insert(pDpData->pParserView, &pCanFirst->stSignal.id, g_slist_find(pDpData->pAll, pCanFirst));
                int32_t len = g_hash_table_size(g_dpData.pParserView);
                LOG_Debug("len:%d", len);
                LOG_Debug("insert to hash:%s,key:%d, value:%x", ret?"yes":"no", pCanFirst->stSignal.id,g_slist_find(pDpData->pAll, pCanFirst));
            }

            // 所有的数据都会在pSpeedTable里面保存一份
            // 存放到pSpeedTable里面的数据结构为：
            // key：dbcjson里面每个信号的"signalIndex"
            // value：dbcjson里面这个"signalIndex"所在的信号的所有信号信息，以及一个uint64_t类型的VALUE值
            //          该VALUE用来存放当CAN消息到来时根据信号信息解析好的数值
            if(!g_hash_table_insert(pDpData->pSpeedTable, &tmp->stSignal.sigIdx, tmp))
            {
                LOG_Debug("speed table insert fail!");
            }
            // key：dbcjson里面每个信号的信号名
            // value：dbcjson里面这个"signalIndex"所在的信号的所有信号信息，以及一个uint64_t类型的VALUE值
            //          该VALUE用来存放当CAN消息到来时根据信号信息解析好的数值
            if (!g_hash_table_insert(pDpData->pSigNameTable, g_strdup(signalName), tmp))
            {
                LOG_Debug("pSigNameTable insert fail!");
            }
        }
    }
    is.close();
    LOG_Debug("DpLoadConfigFromDb end");
    return RET_OK;
}

int32_t DpLoadConfig(void)
{
	int32_t ret = RET_ERR;
    pthread_rwlock_wrlock(&g_dataPoolRWLock);
    Dp_Init();
    g_dpData.pParserView = g_hash_table_new(g_int_hash, g_int_equal);
    g_dpData.pSpeedTable = g_hash_table_new(g_int_hash, g_int_equal);
    g_dpData.pSigNameTable = g_hash_table_new(g_str_hash, g_str_equal);
    g_dpData.canIdFilter = g_array_new(FALSE, FALSE, sizeof(uint32_t));
    ret = DpLoadConfigFromDb(&g_dpData);
	if(ret == RET_ERR)
	{
        pthread_rwlock_unlock(&g_dataPoolRWLock);
		return RET_ERR;
	}

    pthread_rwlock_unlock(&g_dataPoolRWLock);
    Dp_list(g_dpData.pAll);
    return RET_OK;
}

static void dealCanIdFilterId(Json::Value &root)
{
    for (const auto &canid : root["canIdFilter"])
    {
        uint32_t id = 0;
        if (canid.isString())
        {
            std::string canidstr = canid.asString();
            id = std::stoul(canidstr, nullptr, 16);
        }
        else if (canid.isInt())
        {
            id = canid.asInt();
        }
        else
        {
        }
        LOG_Debug("canIdFilter add:%x", id);
        g_array_append_val(g_dpData.canIdFilter, id);
    }
}

static void setUserPubSignalType(std::string &signaltype, CallbackSignalData_t &data)
{
    // 用户设置的发送的数据类型
    if (signaltype == "float")
    {
        data.signalType = 1;
    }
    else if (signaltype.substr(0, 4) == "uint")
    {
        int bitNum = std::stoi(signaltype.substr(4));
        data.signalType = ((bitNum >= 8 && bitNum <= 64) ? (log(bitNum) / log(2) - 1) : 5);
    }
    else if (signaltype.substr(0, 3) == "int")
    {
        int bitNum = std::stoi(signaltype.substr(3));
        data.signalType = ((bitNum >= 8 && bitNum <= 64) ? (log(bitNum) / log(2) + 3) : 9);
    }
    else
    {
        data.signalType = 99;
    }
}

int32_t dealSendCanSignalMsgs(void)
{    
    //加载structjson文件
    std::ifstream is(STRUCT_JSON_FILE_PATH, ios::binary);
    if (!is.is_open())
    {
        LOG_Error("open json file failed.");
        return RET_ERR;
    }

    Json::Reader reader;
    Json::Value root;

    if(reader.parse(is, root))
    {
        LOG_Debug("start parse canMsgStruct.json");
    }
    else
    {
        LOG_Debug("parse json file failed.");
        return RET_ERR;
    }

    dealCanIdFilterId(root);

    /**
     * @brief Construct a new Subscription Queue Msg object:请求CAN过滤列表
     * 
     */
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_VEHICLE_CAN_ID_REQ, RecvMcuCanParserFilterReqCallBack, NULL);

    for (const auto& msg : root["structs"]) 
    {
        std::string msgname = msg["msgname"].asString();
        LOG_Debug("parse msgname:");
        //1.首先处理是否需要携带can原始数据
        std::vector<uint32_t> canIdList;
        if (msg.isMember("rawCan") && msg["rawCan"]["is"].asBool())
        {
            //需要包含canid的情况
            //canId可以同时支持以十六进制和十进制的写法 如"canId": "0x283" 或 "canId": 123
            for (const auto& rawCanData : msg["rawCan"]["data"])
            {
                if(rawCanData["canId"].isString())
                {
                    std::string canidstr = rawCanData["canId"].asString();
                    uint64_t canid = std::stoul(canidstr, nullptr, 16);
                    canIdList.push_back(canid);
                }
                else if(rawCanData["canId"].isInt())
                {
                    canIdList.push_back(rawCanData["canId"].asInt());
                }
                else
                {
                    canIdList.push_back(0);
                }
            }
        }
        //2.处理信号值
        std::vector<CallbackSignalData_t> signalDatas;
        for (const auto& signal : msg["SignalContents"])
        {
            std::string signalname = "";
            if(!signal["signal"].isNull())
            {
                signalname = signal["signal"].asString();
            }
            std::string signaltype = signal["type"].asString();
            CallbackSignalData_t data;
            data.signal = nullptr;

            setUserPubSignalType(signaltype, data);

            //用户发送时需要额外设置的用户因子
            data.userfactor = signal.get("userfactor", DBL_MAX).asDouble();
            //用户发送时需要额外设置的用户偏移
            data.useroffset = signal.get("useroffset", DBL_MAX).asDouble();
            //用户发送时需要额外设置的用户map
            if (signal.isMember("usermap"))
            {
                for (const auto &entry : signal["usermap"].getMemberNames())
                {
                    int key = std::stoi(entry);
                    int value = signal["usermap"][entry].asInt();
                    data.usermap[key] = value;
                }
            }
            else
            {
                data.usermap.clear();
            }

            //查找信号保存的位置指针
            data.signal = (P_Dp_Signal_t)g_hash_table_lookup(g_dpData.pSigNameTable, signalname.c_str());
            LOG_Debug("push_back msg:%s and signal:%s, signal type:%d", msgname.c_str(), signalname.c_str(),data.signalType);
            signalDatas.push_back(data);
        }
        int period = msg["period"].asInt();
        long msgid = 0;

        if (msg["msgid"].isString())
        {
            std::string msgidstr = msg["msgid"].asString();
            msgid = std::stoi(msgidstr, nullptr, 16);
        }
        else
        {
            msgid = msg["msgid"].asInt();
        }

        CallbackData_t *callbackData = new CallbackData_t(msgname, canIdList, signalDatas, msgid);
        
        if(0 == strncmp("realdataupload", msgname.c_str(), strlen("realdataupload")))
        {
            g_realDataUpload = callbackData;
        }
        else
        {
            g_timeout_add(period, timer_callback, callbackData);
        }
    }
    is.close();
    return RET_OK;
}

GMutex g_realDataUploadLock;
GCond g_realDataUploadCond;

int32_t SignalRealDataUploadCond()
{
    g_mutex_lock(&g_realDataUploadLock);
    g_cond_signal(&g_realDataUploadCond);
    g_mutex_unlock(&g_realDataUploadLock);

    return RET_OK;
}

int32_t WaitRealDataUploadTimeOut(uint32_t secTimeOut)
{
    int32_t retValue = RET_ERR;
    gint64 endTIme = 0;

    g_mutex_lock(&g_realDataUploadLock);
    
    endTIme = g_get_monotonic_time() + G_TIME_SPAN_SECOND * secTimeOut;
    if (g_cond_wait_until(&g_realDataUploadCond, &g_realDataUploadLock, endTIme))
    {
        retValue = RET_OK;
    }

    g_mutex_unlock(&g_realDataUploadLock);
    return retValue;
}


void *RealDataUploadHandle(void *arg)
{
    int32_t retValue = 0;

    g_mutex_init(&g_realDataUploadLock);
    g_cond_init(&g_realDataUploadCond);

    while(1)
    {
        retValue = WaitRealDataUploadTimeOut(3);
        if(RET_OK == retValue)
        {
            timer_callback(g_realDataUpload);
            LOG_Debug("realDataUpload");
        }
    }
}

int32_t DpGetCanid(OUT uint32_t **pData, OUT uint32_t *pLen)
{
    if (!pData || !pLen)
    {
        LOG_Error("param is NULL!");
        return RET_ERR;
    }

    pthread_rwlock_rdlock(&g_dataPoolRWLock);

    if (!g_dpData.pParserView || g_hash_table_size(g_dpData.pParserView) == 0)
    {
        LOG_Error("Dp have not init!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }

    GList *pList = g_hash_table_get_keys(g_dpData.pParserView);
    if (!pList)
    {
        LOG_Error("pList is null!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }

    uint32_t len = g_list_length(pList);
    uint32_t *ptr = static_cast<uint32_t *>(g_try_malloc0(len * sizeof(uint32_t)));
    if (!ptr)
    {
        LOG_Error("malloc fail!");
        g_list_free(pList);
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }

    uint32_t i = 0;
    for (GList *iterator = pList; iterator && i < len; iterator = iterator->next, i++)
    {
        const uint32_t *pTmp = static_cast<uint32_t *>(iterator->data);
        if (pTmp)
        {
            ptr[i] = *pTmp;
        }
    }

    g_list_free(pList);
    pthread_rwlock_unlock(&g_dataPoolRWLock);

    *pData = ptr;
    *pLen = len;
    return RET_OK;
}

int32_t DpGetFilterCanid(OUT uint32_t **pData, OUT uint32_t *pLen)
{
    if (!pData || !pLen)
    {
        LOG_Error("param is NULL!");
        return RET_ERR;
    }

    pthread_rwlock_rdlock(&g_dataPoolRWLock);

    if (!g_dpData.canIdFilter || g_dpData.canIdFilter->len == 0)
    {
        LOG_Error("Dp have not init!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }

    uint32_t *ptr = static_cast<uint32_t *>(g_try_malloc0(g_dpData.canIdFilter->len * sizeof(uint32_t)));
    if (!ptr)
    {
        LOG_Error("malloc fail!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }

    for (uint32_t i = 0; i < g_dpData.canIdFilter->len; i++)
    {
        uint32_t value = g_array_index(g_dpData.canIdFilter, uint32_t, i);
        ptr[i] = value;
    }

    *pData = ptr;
    *pLen = g_dpData.canIdFilter->len;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}

int32_t DpGetCanEncodeType(IN uint8_t ch, IN uint32_t canid, OUT uint8_t *pType)
{
    if (!pType || !g_dpData.pParserView)
    {
        LOG_Error("param is NULL!");
        return RET_ERR;
    }

    int key = ((ch & 0x7) << 29) + canid;
    pthread_rwlock_rdlock(&g_dataPoolRWLock);

    GSList *iterator = (GSList *)g_hash_table_lookup(g_dpData.pParserView, &key);

    if (!iterator || !iterator->data)
    {
        LOG_Error("Not Find CanMsg!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }

    P_Dp_Signal_t pTmp = (P_Dp_Signal_t)iterator->data;
    *pType = pTmp->stSignal.encode;

    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}

int32_t DpCanSignalForeach(IN uint32_t key, OUT GSList **firstSignal)
{
    pthread_rwlock_rdlock(&g_dataPoolRWLock);
    if (!g_dpData.pParserView)
    {
        LOG_Error("Dp have not init!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }
    //找到这个canid在pAll中的首个信号位置，然后轮询判断该信号后续链表中的指定id的信号数据并处理
    GSList *iterator = static_cast<GSList *>(g_hash_table_lookup(g_dpData.pParserView, &key));
    if(iterator == NULL)
    {
        LOG_Error("key:0x%x not found",key);
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }
    *firstSignal = iterator;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}

int32_t DpGetSignalCheckIdx(IN uint32_t idx)
{
    pthread_rwlock_rdlock(&g_dataPoolRWLock);
    int32_t ret = g_hash_table_contains(g_dpData.pSpeedTable, &idx) ? RET_OK : RET_ERR;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return ret;
}

int32_t DpGetSignalId(IN uint32_t idx, OUT uint32_t *pId)
{
    pthread_rwlock_rdlock(&g_dataPoolRWLock);
    P_Dp_Signal_t pTmp = (P_Dp_Signal_t)g_hash_table_lookup(g_dpData.pSpeedTable, &idx);
    if (!pId || !pTmp)
    {
        LOG_Error("param error!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }

    *pId = pTmp->stSignal.id;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}

int32_t DpGetSignalEncode(IN uint32_t idx, OUT int32_t *pEncode)
{
    pthread_rwlock_rdlock(&g_dataPoolRWLock);
    P_Dp_Signal_t pTmp = (P_Dp_Signal_t)g_hash_table_lookup(g_dpData.pSpeedTable, &idx);
    if (!pEncode || !pTmp)
    {
        LOG_Error("param error!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }

    *pEncode = pTmp->stSignal.encode;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}

int32_t DpGetSignalStartBit(IN uint32_t idx, OUT int32_t *pStartBit)
{
    pthread_rwlock_rdlock(&g_dataPoolRWLock);
    P_Dp_Signal_t pTmp = (P_Dp_Signal_t)g_hash_table_lookup(g_dpData.pSpeedTable, &idx);
    if (!pStartBit || !pTmp)
    {
        LOG_Error("param error!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }

    *pStartBit = pTmp->stSignal.startBit;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}

int32_t DpGetSignalDataLen(IN uint32_t idx, OUT int32_t *pDataLen)
{
    pthread_rwlock_rdlock(&g_dataPoolRWLock);
    P_Dp_Signal_t pTmp = (P_Dp_Signal_t)g_hash_table_lookup(g_dpData.pSpeedTable, &idx);
    if (!pDataLen || !pTmp)
    {
        LOG_Error("param error!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }

    *pDataLen = pTmp->stSignal.dataLen;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}

int32_t DpGetSignalOffset(IN uint32_t idx, OUT double *pType)
{
    pthread_rwlock_rdlock(&g_dataPoolRWLock);  
    P_Dp_Signal_t pTmp = (P_Dp_Signal_t) g_hash_table_lookup(g_dpData.pSpeedTable, &idx);
    if (!pType || !pTmp)
    {
        LOG_Error("param error!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }

    *pType = pTmp->stSignal.offset;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}

int32_t DpGetSignalAccuracy(IN uint32_t idx, OUT double *pType)
{
    pthread_rwlock_rdlock(&g_dataPoolRWLock);  
    P_Dp_Signal_t pTmp = (P_Dp_Signal_t) g_hash_table_lookup(g_dpData.pSpeedTable, &idx);
    if (!pType || !pTmp)
    {
        LOG_Error("param error!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }

    *pType = pTmp->stSignal.accuracy;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}

int32_t DpGetSignalDefaultValue(IN uint32_t idx, OUT int32_t *pType)
{
    pthread_rwlock_rdlock(&g_dataPoolRWLock);  
    P_Dp_Signal_t pTmp = (P_Dp_Signal_t) g_hash_table_lookup(g_dpData.pSpeedTable, &idx);
    if (!pType || !pTmp)
    {
        LOG_Error("param error!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }

    *pType = pTmp->stSignal.defaultValue;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}

int32_t DpGetSignalValue(IN uint32_t idx, OUT Dp_Value_u *pValue, bool *issigned)
{
    pthread_rwlock_rdlock(&g_dataPoolRWLock);  
    P_Dp_Signal_t pTmp = (P_Dp_Signal_t) g_hash_table_lookup(g_dpData.pSpeedTable, &idx);
    if (!pValue || !pTmp)
    {
        LOG_Error("param error!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }

    *pValue = pTmp->value;
    *issigned = pTmp->issigned;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}

int32_t DpGetSignalAddr(IN uint32_t idx, OUT P_Dp_Signal_t *signalData)
{
    P_Dp_Signal_t pTmp = NULL;
    pthread_rwlock_rdlock(&g_dataPoolRWLock);
    pTmp = (P_Dp_Signal_t) g_hash_table_lookup(g_dpData.pSpeedTable, &idx);
    if (!pTmp)
    {
        LOG_Error("param error!\n");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }
    *signalData = pTmp;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}


int32_t DpSetSignalValue(IN uint32_t idx, IN Dp_Value_u val, bool issigned)
{
    pthread_rwlock_wrlock(&g_dataPoolRWLock);
    P_Dp_Signal_t pTmp = (P_Dp_Signal_t) g_hash_table_lookup(g_dpData.pSpeedTable, &idx);
    if (!pTmp)
    {
        LOG_Error("param error!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }
    pTmp->value = val;
    pTmp->issigned = issigned;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}

int32_t setSignalValueAndSigned(IN P_Dp_Signal_t signal, IN Dp_Value_u val, bool issigned)
{
    pthread_rwlock_wrlock(&g_dataPoolRWLock);

    if(signal->stSignal.realUpload)
    {
        if(val.floatVal != signal->value.floatVal || val.uint64Val != signal->value.uint64Val)
        {
            signal->value = val;
            signal->issigned = issigned;
            pthread_rwlock_unlock(&g_dataPoolRWLock);
            SignalRealDataUploadCond();
            LOG_Debug("realDataUpload");
            return RET_OK;
        }
    }
    signal->value = val;
    signal->issigned = issigned;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}

int32_t DpGetSignal(IN uint32_t idx, OUT Dp_Signal_Data_t *signalData)
{
    pthread_rwlock_rdlock(&g_dataPoolRWLock);
    P_Dp_Signal_t pTmp = (P_Dp_Signal_t)g_hash_table_lookup(g_dpData.pSpeedTable, &idx);
    if (!pTmp)
    {
        LOG_Error("param error!");
        pthread_rwlock_unlock(&g_dataPoolRWLock);
        return RET_ERR;
    }
    *signalData = pTmp->stSignal;
    pthread_rwlock_unlock(&g_dataPoolRWLock);
    return RET_OK;
}

#ifdef __cplusplus
}
#endif
