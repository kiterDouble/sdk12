#include <glib.h>
#include <glib/gprintf.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "sr_log.h"
#include "err_code.h"
#include "param_database.h"
#include "param_sync.h"
#include "param_util.h"

static const char *MPU_VERSION_TXT_PATH = "/oemapp/version.txt";   //MPU版本号文件路径

uint8_t flagParamSyncAck[E_ParamId_UserEnd + 1] = {0};
uint8_t flagGetMcuParas[E_ParamId_UserEnd + 1] = {0};
extern const char *element[E_ParamId_BaseEnd + 1][8];

void* UpdateParamFromMcu(void* pArg)
{
    //统计来自MCU的参数数量
    uint16_t mcuParaNum = 0;
    uint8_t reqData[E_ParamId_UserEnd + 2] = {0};
    
    LOG_Debug("UpdateParamFromMcu");

    // 需同步项
    // E_ParamId_CustomSW_Version
    // E_ParamId_SW_Version
    // E_ParamId_HW_Version
    // E_ParamId_VIN
    // E_ParamId_SN
    // E_ParamId_ParatNumber

    /* 吉利VE21: 外部版本号(param10)以MCU为数据源, 由MCU侧F195同步; 陕汽/同力保持DB来源不变 */
    if (MPU_VE21_PROJECT_NUM == ParamUtilGetMpuProjectNo() &&
        PARAM_SOURCES_MCU != GetParamsSources(E_ParamId_SW_Version_External))
    {
        LOG_Debug("VE21 set E_ParamId_SW_Version_External from mcu");
        SetParamsSources(E_ParamId_SW_Version_External, PARAM_SOURCES_MCU);
    }
    usleep(500000);
	if (PARAM_SOURCES_MCU != GetParamsSources(E_ParamId_SW_Version))
    {
        LOG_Debug("set E_ParamId_SW_Version from mcu");
        SetParamsSources(E_ParamId_SW_Version, PARAM_SOURCES_MCU);
    }
    usleep(500000);

    if (PARAM_SOURCES_MCU != GetParamsSources(E_ParamId_VIN))
    {
        LOG_Debug("set E_ParamId_HW_Version from mcu");
        SetParamsSources(E_ParamId_VIN, PARAM_SOURCES_MCU);
    }
    usleep(500000);
    
    if (PARAM_SOURCES_MCU != GetParamsSources(E_ParamId_SN))
    {
        LOG_Info("set E_ParamId_SN from mcu");
        SetParamsSources(E_ParamId_SN, PARAM_SOURCES_MCU);
        LOG_Info("E_ParamId_SN source set to: %d", GetParamsSources(E_ParamId_SN));
    }
    usleep(500000);

    if (PARAM_SOURCES_MCU != GetParamsSources(E_ParamId_HW_Version))
    {
        LOG_Info("set E_ParamId_HW_Version from mcu");
        SetParamsSources(E_ParamId_HW_Version, PARAM_SOURCES_MCU);
        LOG_Info("E_ParamId_HW_Version source set to: %d", GetParamsSources(E_ParamId_HW_Version));
    }
    usleep(500000);
    
    for(int index = 0; index < E_ParamId_UserEnd + 1; index++)
    {
        if(PARAM_SOURCES_MCU == GetParamsSources(index))
        {
            mcuParaNum++;
            LOG_Debug("i:%d, mcuParaNum:%d", index, mcuParaNum);
        }
    }
#if 0
    if (PARAM_SOURCES_MCU != GetParamsSources(E_ParamId_CustomSW_Version))
    {
        LOG_Debug("set E_ParamId_CustomSW_Version from mcu");
        SetParamsSources(E_ParamId_CustomSW_Version, PARAM_SOURCES_MCU);
    }
    usleep(500000);    
    if (PARAM_SOURCES_MCU != GetParamsSources(E_ParamId_HW_Version))
    {
        LOG_Debug("set E_ParamId_HW_Version from mcu");
        SetParamsSources(E_ParamId_HW_Version, PARAM_SOURCES_MCU);
    }
    usleep(500000);
    if (PARAM_SOURCES_MCU != GetParamsSources(E_ParamId_SN))
    {
        LOG_Debug("set E_ParamId_HW_Version from mcu");
        SetParamsSources(E_ParamId_SN, PARAM_SOURCES_MCU);
    }
    usleep(500000);   
    if (PARAM_SOURCES_MCU != GetParamsSources(E_ParamId_ParatNumber))
    {
        LOG_Debug("set E_ParamId_HW_Version from mcu");
        SetParamsSources(E_ParamId_ParatNumber, PARAM_SOURCES_MCU);
    }
#endif    
    
    while (1)
    {
        //当前已向MCU同步的参数数量
        uint16_t curSyncNum = 0;
        uint16_t reqParaLen = 1;
        memset(reqData, 0x00, sizeof(reqData));
        for (int index = 0; index < E_ParamId_UserEnd + 1; index++)
        {
            if (PARAM_SOURCES_MCU == GetParamsSources(index))
            {
                LOG_Debug("i:%d is from mcu", index);
                //如果当前参数已同步
                if(flagGetMcuParas[index] == 1)
                {
                    curSyncNum++;
                    LOG_Debug("i:%d, curSyncNum:%d", index, curSyncNum);
                }
                else
                {
                    //本次传输的参数个数
                    reqData[0]++;
                    reqData[reqParaLen++] = index;
                    LOG_Debug("req param to mcu:param index:%d",index);
                }
            }
        }
        if(curSyncNum == mcuParaNum)
        {
            break;
        }
        else
        {
            // 向MCU获取参数
            McuMesg_t sendToMcu;
            sendToMcu.type = SYS_QUE_TYPE_MCU_SYNC_TBOX_READ_REQ;
            // 参数的索引值数组，最大32。如果有超过32个MCU参数的情况，那么每次只请求当前还未获取到的前32个参数
            sendToMcu.payloadLen = (reqParaLen > 33 ? 33 : reqParaLen);
            sendToMcu.payload = reqData;
            McuSrvSendMesg(&sendToMcu);
            usleep(500000);
        }
    }
    return NULL;
}

void *UpdateParamFromModem(void *pArg)
{
    char iccid[32] = {0};
    char imei[32] = {0};
    char imsi[32] = {0};

    GetParamsValue(E_ParamId_IMEI, imei);
    GetParamsValue(E_ParamId_IMSI, imsi);
    GetParamsValue(E_ParamId_ICCID, iccid);

    LOG_Debug("get imei from db :%s", imei);
    LOG_Debug("get imsi from db :%s", imsi);
    LOG_Debug("get iccid from db :%s", iccid);

    LOG_Debug("start get imei from modem");
    char imeiTemp[32] = {0};
    while (RET_OK != ModemSrvGetImei((uint8_t *)imeiTemp))
    {
        LOG_Error("param get imei from At error");
        memset(imeiTemp, 0, sizeof(imeiTemp));
        sleep(1);
    }
    LOG_Debug("get imei from at :%s", imeiTemp);
    if(strcmp(imeiTemp, imei) != 0)
    {
        SetParamsValue(E_ParamId_IMEI, imeiTemp, true);
    }

    LOG_Debug("start get iccid from modem");
    char iccidTemp[32] = {0};
    while (RET_OK != ModemSrvGetIccid((uint8_t *)iccidTemp))
    {
        LOG_Error("param get iccid from At error");
        memset(iccidTemp, 0, sizeof(iccidTemp));
        sleep(3);
    }
    LOG_Debug("get iccid from at :%s", iccidTemp);
    if(strcmp(iccidTemp, iccid) != 0)
    {
        SetParamsValue(E_ParamId_ICCID, iccidTemp, true);
    }

    LOG_Debug("start get imsi from modem");
    char imsiTemp[32] = {0};
    while (RET_OK != ModemSrvGetImsi((uint8_t *)imsiTemp))
    {
        LOG_Error("param get imsi from At error");
        memset(imsiTemp, 0, 25);
        sleep(3);/* sleep 3s, Prevent affecting the normal sleep mode of the modem*/
    }
    LOG_Debug("get imsi from at :%s", imsiTemp);
    if(strcmp(imsiTemp, imsi) != 0)
    {
        SetParamsValue(E_ParamId_IMSI, imsiTemp, true);
    }

    return NULL;
}

void HandleMcuReadParamReq(const uint8_t *data, const uint16_t len, void *userdata)
{
    LOG_DebugHex(data,len,"HandleMcuReadParamReq");

    int32_t ret;

    char paramValue[100];
    int32_t paramValueInt = 0;
    uint8_t type = 0;

    uint32_t paramNum = data[0];

    uint8_t paramDataSendToMCU[1024];
    int32_t paramDataLenSum = 0;
    paramDataSendToMCU[paramDataLenSum++] = paramNum;
    LOG_Debug("[param sync]  reqest %d params ", paramNum);
    for (int32_t i = 0; i < paramNum; i++)
    {
        /*get param value*/
        memset(paramValue, 0, sizeof(paramValue));
        DeviceParamEnum paramIndex = (DeviceParamEnum)(data[i + 1]);
        if (paramIndex > E_ParamId_UserEnd)
        {
            return;
        }
        LOG_Debug("[param sync] index %d ", paramIndex);

        if (GetParamsValueType(paramIndex, &type) == RET_ERR)
        {
            LOG_Debug("[param sync] the param [%d] can not be set", paramIndex);
            return;
        }

        if (type == PARAM_TYPE_INT)
        {
            ret = GetParamsValue(paramIndex, &paramValueInt);
            if (ret != RET_OK)
            {
                LOG_Error("[param sync ] get param errorn ");
            }
            else
            {
                LOG_Debug("[param sync] get param value %d ", paramValueInt);
            }
            /*add param index(1 byte) into buffer*/
            paramDataSendToMCU[paramDataLenSum++] = data[i + 1];

            /*add param len(1 byte) into buffer*/
            paramDataSendToMCU[paramDataLenSum++] = 4;

            /*add param value into buffer*/
            paramDataSendToMCU[paramDataLenSum++] = paramValueInt >> 24;
            paramDataSendToMCU[paramDataLenSum++] = paramValueInt >> 16;
            paramDataSendToMCU[paramDataLenSum++] = paramValueInt >> 8;
            paramDataSendToMCU[paramDataLenSum++] = paramValueInt;
        }
        else
        {
            ret = GetParamsValue(paramIndex, paramValue);
            if (ret != RET_OK)
            {
                LOG_Error("[param sync] get param errorn ");
            }
            else
            {
                LOG_Debug("[param sync] get param value %s ", paramValue);
            }
            /*add param index(1 byte) into buffer*/
            paramDataSendToMCU[paramDataLenSum++] = data[i + 1];
            /*add param len(1 byte) into buffer*/
            uint8_t paramLen = strlen(paramValue);

            /* If the internal version number is less than 12 digits, fill in 3 '0' to the end */
            if (paramIndex == E_ParamId_SW_Version && 9 == paramLen)
            {
                strncat(paramValue, "000", 3);
                paramLen += 3;
            }

            paramDataSendToMCU[paramDataLenSum++] = paramLen;
            /*add param value into buffer*/
            memcpy(&paramDataSendToMCU[paramDataLenSum], paramValue, paramLen);
            paramDataLenSum += paramLen;
            LOG_Debug("[param sync] paramlen %d  paramsumlen %d ", paramLen, paramDataLenSum);

            // check iccid all is 0
            if (paramIndex == E_ParamId_ICCID)
            {
                if (strcmp(element[E_ParamId_ICCID][2], paramValue) == 0)
                {
                    LOG_Error("param sync iccid all is 0,not response to MCU");
                    return;
                }
            }

            // check imsi all is 0
            if (paramIndex == E_ParamId_IMSI)
            {
                if (strcmp(element[E_ParamId_IMSI][2], paramValue) == 0)
                {
                    LOG_Error("param sync imsi all is 0,not response to MCU");
                    return;
                }
            }

            // check imei is 0
            if (paramIndex == E_ParamId_IMEI)
            {
                if (strcmp(element[E_ParamId_IMEI][2], paramValue) == 0)
                {
                    LOG_Error("param sync imei all is 0,not response to MCU");
                    return;
                }
            }
        }
    }

    /*res : send params to MCU a01m03s02*/
    McuMesg_t sendToMcu;
    sendToMcu.type = SYS_QUE_TYPE_MCU_SYNC_TBOX_READ_RES;
    sendToMcu.payloadLen = paramDataLenSum;
    sendToMcu.payload = paramDataSendToMCU;
    ret = McuSrvSendMesg(&sendToMcu);
    // ret = SendMcuMsg(MSG_QUEUE_SYNC_PARAM, paramDataLenSum, PARAM_SYNC_RES, paramDataSendToMCU);

    if (ret != RET_OK)
    {
        LOG_Debug("[param sync] sent data to MCU erron ");
        return;
    }
    else
    {
        LOG_Debug("[param sync]send to mcu type %d len %x ", SYS_QUE_TYPE_MCU_SYNC_TBOX_READ_RES, paramDataLenSum);
    }
}

void HandleMcuSyncParam(const uint8_t *data, const uint16_t len, void *userdata)
{
    LOG_DebugHex(data,len,"HandleMcuSyncParam");

    int32_t ret;

    DeviceParamEnum paramIndex = E_ParamId_ICCID;
    
    char paramValue[100] = {0};
    int32_t paramValueInt = 0;
    uint8_t type = 0;

    uint32_t syncAckParamNum = 0;
    uint8_t paramDataSendToMCU[1024];

    int32_t paramDataLenSum = 0;
    paramIndex = (DeviceParamEnum)(data[paramDataLenSum++]);
    /*is check param valid*/
    if (paramIndex > E_ParamId_UserEnd || paramIndex == E_ParamId_ICCID ||
        paramIndex == E_ParamId_IMSI || paramIndex == E_ParamId_IMEI)
    {
        LOG_Debug("[param sync] the param [%d] can not be set ", paramIndex);
        return;
    }

    uint32_t paramLen = data[paramDataLenSum++];

    memset(paramValue, 0, sizeof(paramValue));
    memcpy(paramValue, &data[paramDataLenSum], paramLen);

    if (GetParamsValueType(paramIndex, &type) == RET_ERR)
    {
        LOG_Debug("[param sync] the param [%d] can not be set ", paramIndex);
        return;
    }
    if (type == PARAM_TYPE_INT)
    {

        paramValueInt = (paramValue[0] << 24) +
                        (paramValue[1] << 16) +
                        (paramValue[2] << 8) + paramValue[3];

        ret = SetParamsValue(paramIndex, &paramValueInt, false);
    }
    else if (type == PARAM_TYPE_STR)
    {
            // 内部版本号最后三位代表MCU版本，需要保留MPU前9位只更新后3位
            if (paramIndex == E_ParamId_SW_Version)
            {
                ParamInfo_t paramInfo = {.id = E_ParamId_SW_Version};
                GetParam(&paramInfo);
                char version[16] = {0};
                strncat(version, paramInfo.value, 9);
                strncat(version + 9, paramValue + 9, 3);
                LOG_Debug("VERSION(sync):%s", version);
                memset(paramValue, 0, sizeof(paramValue));
                memcpy(paramValue, version, strlen(version));
            }
            ret = SetParamsValue(paramIndex, paramValue, false);
        }
    else
    {
        LOG_Debug("[param sync] the param [%d] can not be set ", paramIndex);
        return;
    }

    if (ret != RET_OK)
    {
        LOG_Debug("[param sync] erron : %x ", ret);
    }

    paramDataSendToMCU[syncAckParamNum++] = (ret == RET_OK ? 0 : 1);

    memset(paramValue, 0, sizeof(paramValue));
    if (type == PARAM_TYPE_INT)
    {
        ret = GetParamsValue(paramIndex, &paramValueInt);
        if (ret != RET_OK)
        {
            LOG_Error("[param sync ] get param errorn ");
        }
        else
        {
            LOG_Debug("[param sync] get param value %d ", paramValueInt);
        }
        /*add param index(1 byte) into buffer*/
        paramDataSendToMCU[syncAckParamNum++] = paramIndex;

        /*add param len(1 byte) into buffer*/
        paramDataSendToMCU[syncAckParamNum++] = sizeof(uint32_t);

        /*add param value into buffer*/
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt >> 24;
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt >> 16;
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt >> 8;
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt;
    }
    else
    {
        ret = GetParamsValue(paramIndex, paramValue);
        if (ret != RET_OK)
        {
            LOG_Error("[param sync ] get param errorn ");
        }
        else
        {
            LOG_Debug("[param sync] get param value %s ", paramValue);
        }
        /*add param index(1 byte) into buffer*/
        paramDataSendToMCU[syncAckParamNum++] = paramIndex;

        /*add param len(1 byte) into buffer*/
        paramLen = strlen(paramValue);
        paramDataSendToMCU[syncAckParamNum++] = paramLen;

        /*add param value into buffer*/
        memcpy(&paramDataSendToMCU[syncAckParamNum], paramValue, paramLen);
        syncAckParamNum += paramLen;
        LOG_Debug("[param sync] paramlen %d  paramsumlen %d ", paramLen, syncAckParamNum);
    }

    McuMesg_t sendToMcu;
    sendToMcu.type = SYS_QUE_TYPE_MCU_SYNC_TBOX_MCU_WRITE_RES;
    sendToMcu.payloadLen = syncAckParamNum;
    sendToMcu.payload = paramDataSendToMCU;
    ret = McuSrvSendMesg(&sendToMcu);

    if (ret != RET_OK)
    {
        LOG_Debug("[param sync] sent data to MCU erron ");
    }
}

void HandleMcuRespParam(const uint8_t *data, const uint16_t len, void *userdata)
{
    LOG_DebugHex(data,len,"HandleMcuRespParam");

    uint32_t datalen = 0;
    uint8_t number_ = data[datalen++];
    for (int i = 0; i < number_; i++)
    {
        if(datalen >= len)
        {
            LOG_Error("[param] datalen is longer than the received length!");
            break;
        }
        uint8_t index_ = data[datalen++];
        uint8_t len_ = data[datalen++];

        uint8_t type;
        GetParamsValueType(index_, &type);
        int32_t ret;
        if ((type == PARAM_TYPE_INT) && (len_ == sizeof(uint32_t)))
        {
            int32_t value = 0;
            value |= ((int32_t)(data[datalen++])) << 24;
            value |= ((int32_t)(data[datalen++])) << 16;
            value |= ((int32_t)(data[datalen++])) << 8;
            value |= ((int32_t)(data[datalen++]));
            ret = SetParamsValue((DeviceParamEnum)index_, (void *)&value, false);
        }
        else
        {
            uint8_t value[256] = {0};
            memcpy(value, &(data[datalen]), len_);
            datalen += len_;

            // 内部版本号最后三位代表MCU的流水号，这里只需要更新最后三位
            // 209001009009
            if (index_ == E_ParamId_SW_Version)
            {
                ParamInfo_t paramInfo = {.id = E_ParamId_SW_Version};
                GetParam(&paramInfo);
                char version[16] = {0};
                strncat(version, paramInfo.value, 9);
                strncat(version + 9, value + 9, 3);
                LOG_Debug("VERSION:%s", version);
                memset(value, 0, sizeof(value));
                memcpy(value, version, strlen(version));
            }
            ret = SetParamsValue((DeviceParamEnum)index_, (void *)value, false);
        }
        if (ret != RET_OK)
        {
            LOG_Error("[param] set param fail!");
        }
        else
        {
            flagGetMcuParas[index_] = 1;
            LOG_Debug("[param] sync param from mcu success, param index: %d!", index_);
        }
    }
}

void HandleMcuRespSyncParam(const uint8_t *data, const uint16_t len, void *userdata)
{
    LOG_DebugHex(data,len,"HandleMcuRespSyncParam");

    uint16_t index = 0;
    uint8_t status = data[index++];
    uint8_t paraIndex = data[index++];
    if(0 == status)
    {
        flagParamSyncAck[paraIndex] = 0; /*应答清零*/
        LOG_Debug("[param sync]##### flagParamSyncAck[%d] = [0]", paraIndex);
    }
    else
    {
        LOG_Debug("[param sync]##### Param sync to MCU failed. index:[%d]", paraIndex);
    }
}

void SyncParamToMcu(DeviceParamEnum paramName, void* paramValue)
{
    uint8_t type;
    GetParamsValueType(paramName, &type);
    McuMesg_t sendToMcu;
    sendToMcu.type = SYS_QUE_TYPE_MCU_SYNC_TBOX_MCU_WRITE_REQ;

    uint32_t syncAckParamNum = 0;
    uint8_t paramDataSendToMCU[300];

    if (type == PARAM_TYPE_INT)
    {
        int32_t paramValueInt = *(int32_t *)paramValue;
        paramDataSendToMCU[syncAckParamNum++] = paramName;
        paramDataSendToMCU[syncAckParamNum++] = sizeof(uint32_t);
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt >> 24;
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt >> 16;
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt >> 8;
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt;
    }
    else
    {
        paramDataSendToMCU[syncAckParamNum++] = paramName;
        uint16_t paramLen = strlen(paramValue);
        paramDataSendToMCU[syncAckParamNum++] = paramLen;
        memcpy(&paramDataSendToMCU[syncAckParamNum], paramValue, paramLen);
        syncAckParamNum += paramLen;
    }
    sendToMcu.payloadLen = syncAckParamNum;
    sendToMcu.payload = paramDataSendToMCU;
    McuSrvSendMesg(&sendToMcu);
    flagParamSyncAck[(uint32_t)paramName] = 1;
}

bool CheckParamSyncAck(void *user_data)
{
    for (uint32_t i = 0; i < E_ParamId_UserEnd + 1; i++)
    {
        /*无应答重发*/
        if (flagParamSyncAck[i] == 1)
        {
            LOG_Debug("sync mcu timer:Sync param to MCU. id:%d",i);
            uint8_t type;
            GetParamsValueType(i, &type);

            if (type == PARAM_TYPE_INT)
            {
                int32_t paramValueInt = 0;
                GetParamsValue(i, &paramValueInt);
                SyncParamToMcu(i, &paramValueInt);
            }
            else
            {
                uint8_t paramValue[256] = {0};
                GetParamsValue(i, paramValue);
                SyncParamToMcu(i, paramValue);
            }
        }
    }
    return TRUE;
}

void LoadMpuVersionToParam(void)
{
    char version[16] = {0};
    FILE *file = fopen(MPU_VERSION_TXT_PATH, "r");
    if (file == NULL) {
        LOG_Error("Open %s failed, error[%d]: %s\n", MPU_VERSION_TXT_PATH, errno, strerror(errno));
        return;
    }

    char *ret = fgets(version, sizeof(version), file);
    fclose(file);
    if (ret == NULL){
        LOG_Error("Error or end of file[%s] encountered", MPU_VERSION_TXT_PATH);
        return;
    }
    LOG_Debug("Read MPU version[%s]", version);
    if (strlen(version) < 9) {
        return;
    }

    // 235000001001 - 内部版本号
    ParamInfo_t paramInfo = {.id = E_ParamId_SW_Version};
    GetParam(&paramInfo);
    char versionMpu[16] = {0};
    strncat(versionMpu, version, 9);
    strncat(versionMpu + 9, paramInfo.value + 9, 3);
    LOG_Debug("VERSION MPU:%s", versionMpu);

    SetParamsValue(E_ParamId_SW_Version, (void *)versionMpu, false);

    /* 吉利VE21: E_ParamId_SW_Version_External(外部版本=F195 16字节)以MCU为数据源
       陕汽/同力保持原有拼装逻辑不变 */
    if (MPU_VE21_PROJECT_NUM == ParamUtilGetMpuProjectNo())
    {
        LOG_Debug("VE21: skip external version(param10), sourced from MCU");
        return;
    }

    // 外部版本号: 将前9位的后3位转成16进制8位，version.txt后3位转成16进制4位
    // 组合格式: V[前9位后3位16进制8位][version.txt后3位16进制4位] = V000000010001

    char cpuLast3[4] = {0};
    strncpy(cpuLast3, version + 6, 3);  // 取前9位的后3位 (位置6-8)
    int hexCpuLast3 = atoi(cpuLast3);   // 转16进制8位
    
    char mcuLast3[4] = {0};
    strncpy(mcuLast3, version + 9, 3);  // 取version.txt后3位 (位置9-11)
    int hexMcuLast3 = atoi(mcuLast3);    // 转16进制4位
    
    char versionExt[24] = {0};
    snprintf(versionExt, sizeof(versionExt), "V%08X%04X", hexCpuLast3, hexMcuLast3);
    LOG_Debug("VERSION External:%s (from version.txt cpu=%s->%08X, mcu=%s->%04X)", versionExt, cpuLast3, hexCpuLast3, mcuLast3, hexMcuLast3);

    SetParamsValue(E_ParamId_SW_Version_External, (void *)versionExt, false);
}
