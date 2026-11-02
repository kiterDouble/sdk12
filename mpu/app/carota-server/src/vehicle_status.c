#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include "sr_log.h"
#include "err_code.h"
#include "sr_ipc.h"
#include "sgmw_ipc.h"
#include "vehicle_status.h"
#include "canmsgstruct_canstruct.h"
// #include "struct_test.h"
// typedef struct
// {
//     uint8_t BMSCC2Sts;//电池管理系统检测快充连接状态
//     uint8_t CCSts;//慢充充电连接状态
//     uint8_t VecStatRdy;//整车高压状态
//     uint8_t VecChrgingSts;//充电线连接状态
//     uint8_t VecChrgStsIndOn;//充电状态
//     float VehSpdAvgDrvn;//车速
//     uint8_t VecSOC;//整车电池电量
//     float BatExtVol;//电池包外侧总电压
//     uint8_t VecActGearSts;//整车当前档位状态
//     uint8_t KyPstn;//整车钥匙状态
//     uint8_t HandBrkSts;//电子手刹状态
//     uint8_t AnThWaSt;//设防的警告状态
//     uint8_t SysPowMode;//整车电源模式
//     uint8_t EngRunAtv;//发动机运转状态
// } Data_Carota_canVal_t;//柯洛达所需信号项

typedef struct  
{
    gint8 arming;
    gdouble speed;
    gint32 soc;
    gdouble voltage;
    gchar vecStatRdy;
    gchar vecChrgingSts;
    gchar vecChrgStsIndOn;
    gchar ccStatus;
    gchar gear[4];
    gchar engine[6];
    gchar hb;
    gchar cc2Status;
    gint8 vcu_ota;
}VehicleStatus;

G_LOCK_DEFINE_STATIC(g_penetrateLock);

static VehicleStatus g_stvehicleData;

gint8 VehicleStatusGetOTA(void)
{
    LOG_Debug("g_stvehicleData.vcu_ota:%f",g_stvehicleData.vcu_ota);
    return g_stvehicleData.vcu_ota;
}

gdouble VehicleStatusGetSpeed(void)
{
    LOG_Debug("g_stvehicleData.speed:%f",g_stvehicleData.speed);
    return g_stvehicleData.speed;
}

gint32 VehicleStatusGetSOC(void)
{
    LOG_Debug("g_stvehicleData.soc:%d",g_stvehicleData.soc);
    return g_stvehicleData.soc;
}

gdouble VehicleStatusGetVoltage(void)
{
    LOG_Debug("g_stvehicleData.voltage:%f",g_stvehicleData.voltage);
    return g_stvehicleData.voltage;
}

gint32 VehicleStatusGetGear(gchar *gear,gint32 len)
{
    gint32 grarDataLen = strlen(g_stvehicleData.gear);
    if(len < grarDataLen && NULL == gear)
    {
        LOG_Error("gear Len is smart or gear is NULL");
        return RET_ERR;
    }
    G_LOCK(g_penetrateLock);
    memcpy(gear,g_stvehicleData.gear,grarDataLen);
    G_UNLOCK(g_penetrateLock);
    LOG_Debug("g_stvehicleData.gear:%s",g_stvehicleData.gear);
    return grarDataLen;
}

gint32 VehicleStatusGetEngine(gchar *engine, gint32 len)
{
    gint32 engineDataLen = strlen(g_stvehicleData.engine);
    if(len < engineDataLen && NULL == engine)
    {
        LOG_Error("engine Len is smart or engine is NULL");
        return RET_ERR;
    }
    memcpy(engine,g_stvehicleData.engine,engineDataLen);
    LOG_Debug("g_stvehicleData.engine:%s",g_stvehicleData.engine);

    return engineDataLen;
}

gint32 VehicleStatusGetPower(gchar *power, gint32 len)
{
    gint32 retValue = -1;

    if(NULL == power)
    {
        LOG_Debug("Fota API VehicleStatusGetPower Is NULL");
        return RET_ERR;
    }
    
    if(1 == g_stvehicleData.vecStatRdy)
    {
        retValue = snprintf(power, len, "Ready");
    }
    else
    {
        retValue = snprintf(power, len, "NotReady");
    }
    LOG_Debug("power :%s", power);
    return retValue;
}

gint32 VehicleStatusGetCharge(void)
{
    LOG_Debug("charge :%d", g_stvehicleData.ccStatus);
    return g_stvehicleData.ccStatus;
}


gint32 VehicleStatusGetHB(void)
{
    LOG_Debug("g_stvehicleData.hb:%d",g_stvehicleData.hb);
    if(g_stvehicleData.hb == 1)
    {
        return 0;
    }
    else
    {
        return 1;
    }
    
    //return g_stvehicleData.hb;
}

gint32 Fota_GetFastCharge(void)
{
    LOG_Debug("fast_charge :%d", g_stvehicleData.cc2Status);
    return g_stvehicleData.cc2Status;
}

gint8 VehicleStatusGetARMING(void)
{
    LOG_Debug("g_stvehicleData.arming:%d",g_stvehicleData.arming);
    return g_stvehicleData.arming;
}

typedef struct
{
    uint8_t value;
    char gear[3];
}VecValue2Gear_t;
static void VehicleStatusHandler(const uint8_t* data, const uint16_t datalen, void* userdata)
{
    if(NULL == data)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if(datalen != sizeof(Data_Carota_canVal_t))
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, datalen, Data_Carota_canVal_t_msgid);

    G_LOCK(g_penetrateLock);
    Data_Carota_canVal_t carOtaData;
    memcpy(&carOtaData, data, datalen);
    g_stvehicleData.speed = carOtaData.VehSpdAvgDrvn;
    g_stvehicleData.soc = carOtaData.VecSOC;
    g_stvehicleData.voltage = carOtaData.BatExtVol;
    g_stvehicleData.vecStatRdy = carOtaData.VecStatRdy;
    g_stvehicleData.vecChrgingSts = carOtaData.VecChrgingSts;
    g_stvehicleData.ccStatus = carOtaData.CCSts;
    // g_stvehicleData.vcu_ota = carOtaData.vcu_ota;
    LOG_Debug("carOtaData.VecActGearSts:%02x", carOtaData.VecActGearSts);
    switch (carOtaData.VecActGearSts)
    {
    case 0:
        g_stvehicleData.gear[0] = 'N';
        break;
    case 1:
        g_stvehicleData.gear[0] = 'D';
        break;   
    case 2:
        g_stvehicleData.gear[0] = 'R';
        break;
    case 3:
        g_stvehicleData.gear[0] = 'P';
        break;
    //适配新矩阵
    case 10:
        g_stvehicleData.gear[0] = 'P';
    break;
    case 11:
        g_stvehicleData.gear[0] = 'S';
    break;   
    case 12:
        g_stvehicleData.gear[0] = 'D';
    break;
    case 13:
        g_stvehicleData.gear[0] = 'N';
    break;
    case 14:
        g_stvehicleData.gear[0] = 'R';
break;
    default:
        break;
    }
    LOG_Debug("carOtaData.VecActGearSts:%02x", carOtaData.VecActGearSts);
    // VecValue2Gear_t vecValue2Gear[] = 
    // {
    //     {0x0d, "N"},
    //     {0x0c, "D"},
    //     {0x0e, "R"},
    //     {0x0a, "P"},
    // };
    // int32_t i = 0;
    // for (; i < sizeof(vecValue2Gear) / sizeof(vecValue2Gear[0]); i++)
    // {
    //     if (carOtaData.VecActGearSts == vecValue2Gear[i].value)
    //     {
    //         strcpy(g_stvehicleData.gear, vecValue2Gear[i].gear);
    //     }
    // }

    if (1 == carOtaData.KyPstn)
    {
        memcpy(g_stvehicleData.engine,"ACC",strlen("ACC") + 1);
    }
    else if (2 == carOtaData.KyPstn)
    {
        memcpy(g_stvehicleData.engine,"ON",strlen("ON") + 1);
    }
    else
    {
        memcpy(g_stvehicleData.engine,"OFF",strlen("OFF") + 1);
    }
    g_stvehicleData.arming = carOtaData.AnThWaSt;
    g_stvehicleData.hb = carOtaData.HandBrkSts;
    g_stvehicleData.cc2Status = carOtaData.BMSCC2Sts;
    G_UNLOCK(g_penetrateLock);
}

gint32 VehicleStatusInit(void)
{
    SubscriptionQueueMsg((SYSQueMsgType_e)Data_Carota_canVal_t_msgid, VehicleStatusHandler, NULL);

    return RET_OK;
}