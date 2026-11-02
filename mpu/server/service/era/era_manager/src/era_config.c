#include "era_config.h"
#include <glib.h>
#include "sr_log.h"
#include <stdio.h>
#include <string.h>
#include "sr_param.h"
#include "hal_modem_era.h"
#include "err_code.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "era_timer.h"
#include "ECallMessage.h"

#define VIN_LENGTH (17)

#define EPARAM_OUTQUE_PATH     "/tmp/ecallQue"
static int g_EParamMsgQue = -1;
static timer_t g_EParamTimer = {0};

static int EParamQueInit(const char *msgPath)
{
    int fd = 0;
    key_t key = 0;

    fd = open(msgPath, O_CREAT|O_RDWR, 0666);
    if (fd < 0) {
        LOG_Error("open error : %s ",strerror(errno));
        return RET_ERR;
    }
    close(fd);

    key = ftok(msgPath, 'E');
    if (key == -1) {
        LOG_Error("ftok error : %s ",strerror(errno));
        return RET_ERR;
    }

    fd = msgget(key, 0666|IPC_CREAT);

    LOG_Debug("ParamQue(%s)(%d) init success!", msgPath, fd);

    return fd;
}

#define EPARAM_C2S_TYPE 1000
#define EPARAM_S2C_TYPE 1001

int EParamQueSend(int fd, long mtype, char *data, int len)
{
    struct EParamMsg msg = {.mtype=mtype};

    snprintf(msg.mtext, sizeof(msg.mtext), "%s", data);

    return msgsnd(g_EParamMsgQue, &msg, sizeof(msg.mtext), 0);
}
#if 0
struct EParamEntry {
    char *name;
    int paramId;
    char value[256];
} EParamTable[] =
{
    { "TEST_MODE",                          E_ParamId_ecallConfigTestMode, "0"},
    { "CALL_AUTO_ANSWER_TIME",              E_ParamId_ecallConfigCallAutoAnswerTime, "20"},
    { "POST_TEST_REGISTRATION_TIME",        E_ParamId_ecallConfigPosTestRegistrationTime, "120"},
    { "TEST_MODE_END_DISTANCE",             E_ParamId_ecallConfigTestModeEndDistance, "300"},
    { "ECALL_TEST_NUMBER",                  E_ParamId_ecallConfigTestNum, "112"},
    { "INT_MEM_TRANSMIT_INTERVAL",          E_ParamId_ecallConfigIntMemTransmitInterval, "60"},
    { "INT_MEM_TRANSMIT_ATTEMPTS",          E_ParamId_ecallConfigIntMemTransmitAttempts, "10"},
    { "CCFT",                               E_ParamId_ecallConfigCCFT, "60"},
    { "MSD_MAX_TRANSMISSION_TIME",          E_ParamId_ecallConfigMSDMaxTransmissionTime, "20"},
    { "NAD_DEREGISTRATION_TIME",            E_ParamId_ecallConfigNADDeregistrationTime, "120"},
    { "ECALL_NO_AUTOMATIC_TRIGGERING",      E_ParamId_ecallConfigEcallNoAutomaticTriggering, "0"},
    { "ECALL_DIAL_DIRATION",                E_ParamId_ecallConfigEcallDialDiration, "5"},
    { "ECALL_AUTO_DIAL_ATTEMPTS",           E_ParamId_ecallConfigEcallAutoDialAttempts, "10"},
    { "ECALL_MANUAL_DIAL_ATTEMPTS",         E_ParamId_ecallConfigEcallManualDialAttempts, "10"}, 
    { "ECALL_MANUAL_CAN_CANCEL",            E_ParamId_ecallConfigEcallManualCanCancel, "1"},
    { "ECALL_SMS_FALLBACK_NUMBER",          E_ParamId_ecallConfigEcallSMSFallbackNumber, "112"},
    { "TEST_REGISTRATION_PERIOD",           E_ParamId_ecallConfigTestRegistrationPeriod, "5"},
    { "GNSS_DATA_RATE",                     E_ParamId_ecallConfigGNSSDataRate, "1"},
    { "VIN",                                E_ParamId_VIN, "ECALLEXAMPLE02013"},
    { "VEHICLE_TYPE",                       E_ParamId_ecallConfigVehicleType, "M1"},
    { "VEHICLE_PROPULSION_STORAGE_TYPE",    E_ParamId_ecallConfigVehiclePropulsionStorageType, "0"},
    { "ERA_AMP_GAIN",                       255, "0"},
};
#else

int EParamAmpGainCallBack(char *key, char *value)
{
    SendMcuEcallAmpGain(atoi(value));

    return 0;
}

int EParamECallCallBack(char *key, char *value)
{

    return 0;
}


int EParamModuleCallBack(char *key, char *value)
{

    HalEraVoiceSetRegulationsInfo(key, value);

    return 0;
}

struct EParamEntry{
    char *name;
    int paramId;
    char value[256];
    int (* EParamCallBack)(char *key, char *value);
} EParamTable[] =
{
    { "TEST_MODE",                          E_ParamId_ecallConfigTestMode, "0", EParamECallCallBack},
    { "CALL_AUTO_ANSWER_TIME",              E_ParamId_ecallConfigCallAutoAnswerTime, "20", EParamModuleCallBack},
    { "POST_TEST_REGISTRATION_TIME",        E_ParamId_ecallConfigPosTestRegistrationTime, "120", EParamModuleCallBack},
    { "ECALL_TEST_NUMBER",                  E_ParamId_ecallConfigTestNum, "112", EParamModuleCallBack},
    { "INT_MEM_TRANSMIT_INTERVAL",          E_ParamId_ecallConfigIntMemTransmitInterval, "60", EParamModuleCallBack},
    { "INT_MEM_TRANSMIT_ATTEMPTS",          E_ParamId_ecallConfigIntMemTransmitAttempts, "10", EParamModuleCallBack},
    { "CCFT",                               E_ParamId_ecallConfigCCFT, "60", EParamModuleCallBack},
    { "MSD_MAX_TRANSMISSION_TIME",          E_ParamId_ecallConfigMSDMaxTransmissionTime, "20", EParamModuleCallBack},
    { "NAD_DEREGISTRATION_TIME",            E_ParamId_ecallConfigNADDeregistrationTime, "3600", EParamModuleCallBack},
    { "ECALL_AUTO_DIAL_ATTEMPTS",           E_ParamId_ecallConfigEcallAutoDialAttempts, "10", EParamModuleCallBack},
    { "GNSS_DATA_RATE",                     E_ParamId_ecallConfigGNSSDataRate, "1", EParamECallCallBack},
    { "VIN",                                E_ParamId_VIN, "ECALLEXAMPLE02013", EParamECallCallBack},
    { "VEHICLE_TYPE",                       E_ParamId_ecallConfigVehicleType, "M1", EParamECallCallBack},
    { "VEHICLE_PROPULSION_STORAGE_TYPE",    E_ParamId_ecallConfigVehiclePropulsionStorageType, "1", EParamECallCallBack},
    { "ERA_AMP_GAIN",                       255, "0", EParamAmpGainCallBack},
};
#endif

struct EParamEntry *EParamKeyFind(const char *key)
{
    int i = -1;

    for(i=0; i < sizeof(EParamTable)/sizeof(EParamTable[0]); i++)
    {
        if(0 == strcmp(key, EParamTable[i].name))
        {
            return &EParamTable[i];
        }
    }

    return NULL;
}


void EParamKeyListSend(void)
{
    char data[1024];
    int i = 0;
    int ret = 0;

    for(i=0; i < sizeof(EParamTable)/sizeof(EParamTable[0]); i++)
        ret += snprintf(data+ret, sizeof(data)-ret, "%s;", EParamTable[i].name);
    data[ret-1] = '\0';

    LOG_Debug("EParam KeyList(%s)", data);
    EParamQueSend(g_EParamMsgQue, EPARAM_S2C_TYPE, data, strlen(data));
}

void EParamKeyListValueSend(char *cmd)
{
    char data[1024] = "";
    int i = 0;
    int ret = 0;
    char *key = NULL;
    char *save = NULL;

    for(key=strtok_r(cmd, ";", &save); NULL != key; key=strtok_r(NULL, ";", &save))
    {
//        printf("%s # key = %s\n", __func__, key);
        for(i=0; i < sizeof(EParamTable)/sizeof(EParamTable[0]); i++)
        {
            if(0 == strcmp(key, EParamTable[i].name))
            {
                ret += snprintf(data+ret, sizeof(data)-ret, "%s=%s;", EParamTable[i].name, EParamTable[i].value);
                break;
            }
        }
    }
    if(ret > 0)
        data[ret-1] = '\0';

    LOG_Debug("EParam KeyListValue(%s)", data);
    EParamQueSend(g_EParamMsgQue, EPARAM_S2C_TYPE, data, strlen(data));
}

void EParamKeyValueWrite(char *cmd)
{
    char save[1024];
    int i = 0;
    char *key = NULL;
    char *value = NULL;

    LOG_Debug("EParam cmd(%s) split!\n", cmd);
    snprintf(save, sizeof(save), "%s", cmd);
    key = strtok_r(cmd, "=", &value);
    if(NULL==key || NULL==value)
    {
        LOG_Error("EParam cmd(%s) split error!\n", cmd);
        return;
    }

    for(i=0; i < sizeof(EParamTable)/sizeof(EParamTable[0]); i++)
    {
        if(0 == strcmp(key, EParamTable[i].name))
        {
            snprintf(EParamTable[i].value, sizeof(EParamTable[i].value), "%s", value);
            if(EParamTable[i].EParamCallBack)
                EParamTable[i].EParamCallBack(key, value);
            EParamQueSend(g_EParamMsgQue, EPARAM_S2C_TYPE, save, strlen(save)+1);
            LOG_Debug("EParam write %s=%s success!", key, value);
            return;
        }
    }

    snprintf(save, sizeof(save), "error=unsupported\n"); 
    EParamQueSend(g_EParamMsgQue, EPARAM_S2C_TYPE, save, strlen(save)+1);
    LOG_Debug("EParam write %s=%s fail!", key, value);
}

static void EParamTimerCallBack(union sigval val)
{
    ssize_t len = 0;
    struct EParamMsg msg = {.mtype = EPARAM_C2S_TYPE };
    char cmd[1024];

    len = msgrcv(g_EParamMsgQue, &msg, sizeof(msg.mtext), EPARAM_C2S_TYPE, IPC_NOWAIT);
//    printf("recv length = %d\n", len);
    if(len > 0)
    {
        LOG_Warn("EParam cmd : %s", msg.mtext);
        snprintf(cmd, sizeof(cmd), "%s", msg.mtext);
        if(0 == strcmp(cmd, "keylist"))         /* 读取所有key列表 */
            EParamKeyListSend();
        else if(NULL != strchr(cmd, '='))       /* 写入参数值 */
            EParamKeyValueWrite(cmd);
        else                                    /* 读取指定key值 */
            EParamKeyListValueSend(cmd);
    }
}


int EParamInit(void)
{
    g_EParamMsgQue = EParamQueInit(EPARAM_OUTQUE_PATH);
    InitTimer(&g_EParamTimer, EParamTimerCallBack);
    TimerXSecTimer(&g_EParamTimer, 1, 1);

    return 0;
}





#if 1
GKeyFile *g_KeyObject = NULL;

#define ECONFIG_FILEPATH        "/data/ecall.ini"
#define ECFG_DEFAULT_GROUP      "default"
#define ECFG_TEST_MODE          "testMode"
#define ECFG_TEST_NUMBER        "testNumber"
void EConfigPrint(GKeyFile * pObj)
{
}


void EConfigInit(void)
{
    GKeyFile *pObj = NULL;
    gboolean ret;
    GError *err = NULL;
    
    EParamInit();       /* 参数消息队列初始化 */

    pObj = g_key_file_new();
    if(NULL == pObj)
    {
        LOG_Error("key_file new fail!");
        return;
    }

    ret = g_key_file_load_from_file(pObj, ECONFIG_FILEPATH, G_KEY_FILE_NONE, &err);
    if(TRUE != ret)
    {
        if(err != NULL)
        {
            LOG_Error("key_file load fail, %s", err->message);
            g_error_free(err);
        }
        else
        {
            LOG_Error("key_file load fail, no error message available");
        }
        goto errLoad;
    }

    EConfigPrint(pObj);

    g_KeyObject = pObj;


    return;
errLoad:
    return;
}

int EConfigECallTestMode(void)
{
    gboolean value = FALSE;     /* 默认非测试模式 */
    GError *err = NULL;

    if(g_KeyObject)
    {
        value = g_key_file_get_boolean(g_KeyObject, ECFG_DEFAULT_GROUP, ECFG_TEST_MODE, &err);
        if(err)
        {
            LOG_Debug("econfig: testMode fail - %s", err->message);
            g_error_free(err);
        }
        else
            LOG_Debug("econfg: testMode(%s) success!", (value && NULL==err)? "yes": "no");
    }

    return value;
}

void EConfigECallTestNumberFill(char *buff, int len)
{
    gchar *value = NULL;

    if(g_KeyObject)
    {
        value = g_key_file_get_string(g_KeyObject, ECFG_DEFAULT_GROUP, ECFG_TEST_NUMBER, NULL);
        if(value)
        {
            snprintf(buff, len, value);
            LOG_Debug("econfig: testNumber(%s) get success!", buff);
        }
    }

    return;
}

const char *EConfigECallTestNumber(void)
{
    static char testNumber[ERA_PHONE_NUM_LEN] = "112";

    EConfigECallTestNumberFill(testNumber, ERA_PHONE_NUM_LEN);

    return testNumber;
}

int EConfigVINFill(char *buff, int len)
{
    int ret = RET_ERR;
    int i = 0;

    memset(buff, 0, len);
    /*
    ParamInfo_t param = {0};
    param.id = 4; //vin 码的参数ID
    int32_t ret = GetParam(&param);
    if(ret == RET_OK)
    {
        memcpy(msdInfo.VIN, param.value, VIN_LENGTH);
    }
    else
    {
        memset(msdInfo.VIN, 0, VIN_LENGTH);
    }
    */
    
    for(i=0; i < sizeof(EParamTable)/sizeof(EParamTable[0]); i++)
    {
        if(0 == strcmp("VIN", EParamTable[i].name))
        {
            memcpy(buff, EParamTable[i].value,VIN_LENGTH);
            ret = RET_OK;
            break;
        }
    }
    
    return ret;
}

typedef struct {
    const char *str;         
    msd_VehicleType type;
} VehicleTypeMapping;

static const VehicleTypeMapping vehicleTypeTable[] = {
    {"M1", EU_MSD_VEHICLE_PASSENGER_M1},
    {"M2", EU_MSD_VEHICLE_BUS_M2},
    {"M3", EU_MSD_VEHICLE_BUS_M3},
    {"N1", EU_MSD_VEHICLE_COMMERCIAL_N1},
    {"N2", EU_MSD_VEHICLE_HEAVY_N2},
    {"N3", EU_MSD_VEHICLE_HEAVY_N3},
    {"L1E", EU_MSD_VEHICLE_MOTORCYCLE_L1E},
    {"L2E", EU_MSD_VEHICLE_MOTORCYCLE_L2E},
    {"L3E", EU_MSD_VEHICLE_MOTORCYCLE_L3E},
    {"L4E", EU_MSD_VEHICLE_MOTORCYCLE_L4E},
    {"L5E", EU_MSD_VEHICLE_MOTORCYCLE_L5E},
    {"L6E", EU_MSD_VEHICLE_MOTORCYCLE_L6E},
    {"L7E", EU_MSD_VEHICLE_MOTORCYCLE_L7E}
};

static msd_VehicleType ECallVehicleTypeStringToEnum(char *vehicleType)
{
    for (int i = 0; i < sizeof(vehicleTypeTable) / sizeof(vehicleTypeTable[0]); i++) {
        if (strcmp(vehicleType, vehicleTypeTable[i].str) == 0) {
            return vehicleTypeTable[i].type;
        }
    }
    // 如果没有匹配项，返回默认值
    return EU_MSD_VEHICLE_PASSENGER_M1;
}

int EConfigVehicleTypeFill(msd_VehicleType *vehicleType)
{
    int ret = RET_ERR;
    int i = 0;

    for(i=0; i < sizeof(EParamTable)/sizeof(EParamTable[0]); i++)
    {
        if(0 == strcmp("VEHICLE_TYPE", EParamTable[i].name))
        {
            LOG_Debug("%s : VehicleType is - [%s]",__FUNCTION__,EParamTable[i].value);
            *vehicleType = ECallVehicleTypeStringToEnum(EParamTable[i].value);
            ret = RET_OK;
            break;
        }
    }
    
    return ret;
}
static msd_VehiclePropulsionStorageType VehiclePropulsionIntToStruct(char *vehiclePropulsion)
{
    msd_VehiclePropulsionStorageType mVehilcePropulsion = {0};

    if(strcmp("1", vehiclePropulsion) == 0)
        mVehilcePropulsion.gasolineTankPresent = 1;
    else if(strcmp("2", vehiclePropulsion) == 0)
        mVehilcePropulsion.dieselTankPresent = 1;
    else if(strcmp("3", vehiclePropulsion) == 0)
        mVehilcePropulsion.compressedNaturalGas = 1;
    else if(strcmp("4", vehiclePropulsion) == 0)
        mVehilcePropulsion.liquidPropaneGas = 1;
    else if(strcmp("5", vehiclePropulsion) == 0)
        mVehilcePropulsion.electricEnergyStorage = 1;
    else if(strcmp("6", vehiclePropulsion) == 0)
        mVehilcePropulsion.hydrogenStorage = 1;
    else if(strcmp("7", vehiclePropulsion) == 0)
        mVehilcePropulsion.otherStorage = 1;
    else
        mVehilcePropulsion.gasolineTankPresent = 1;

    return mVehilcePropulsion;
}
int VehiclePropulsionStorageTypeFill(msd_VehiclePropulsionStorageType *vehiclePropulsion)
{
    int ret = RET_ERR;
    int i = 0;

    for(i=0; i < sizeof(EParamTable)/sizeof(EParamTable[0]); i++)
    {
        if(0 == strcmp("VEHICLE_PROPULSION_STORAGE_TYPE", EParamTable[i].name))
        {
            LOG_Debug("%s : VehiclePropulsion is - [%s]",__FUNCTION__,EParamTable[i].value);
            *vehiclePropulsion = VehiclePropulsionIntToStruct(EParamTable[i].value);
            ret = RET_OK;
            break;
        }
    }
    
    return ret;
}
#endif
