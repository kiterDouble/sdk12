//#include "rtos_api_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "batteryHal.h"
#include "timerHal.h"
#include "batteryHal.h"
#include "powerManageSdk.h"
#include "batterySdk.h"


static const uint32_t g_cellNumber                = 3;                   //备用电池单体个数
static const uint32_t g_cellMaxVoltage            = 1340;                //单体电池最大电压单位：mv
static const uint32_t g_cellMinVoltage            = 1240;                //单体电池最小电压单位：mv
//static const uint32_t g_lowVoltageChargeTime      = 6*60*60*1000;        //检测到电压过低后备用电池放电时长，单位ms
static const uint32_t g_middleVoltageChargeTime   = 6*60*60*1000;        //备用电池充电时长，单位ms
static const uint32_t g_temperatureRMap[]         = {                    //温度--电压对应值，单位°C
    173200,    //-40 单位：°C
    164200,    // -39
    155700,    // -38
    147700,    // -37
    140200,    // -36
    133200,    // -35
    126600,    // -34
    120400,    // -33
    114500,    // -32
    108900,    // -31
    103700,    // -30
    98800,     // -29
    94160,     // -28
    89770,     // -27
    85600,     // -26
    81660,     // -25
    77920,     // -24
    74380,     // -23
    71010,     // -22
    67820,     // -21
    64790,     // -20
    61930,     // -19
    59210,     // -18
    56620,     // -17
    54150,     // -16
    51790,     // -15
    49550,     // -14
    47410,     // -13
    45370,     // -12
    43430,     // -11
    41570,     // -10
    39810,     // -9
    38130,     // -8
    36530,     // -7
    34990,     // -6
    33530,     // -5
    32130,     // -4
    30790,     // -3
    29500,     // -2
    28280,     // -1
    27110,     // 0
    25990,     // 1
    24920,     // 2
    23900,     // 3
    22920,     // 4
    21990,     // 5
    21100,     // 6
    20250,     // 7
    19440,     // 8
    18660,     // 9
    17920,     // 10
    17220,     // 11
    16540,     // 12
    15890,     // 13
    15270,     // 14
    14680,     // 15
    14120,     // 16
    13580,     // 17
    13060,     // 18
    12560,     // 19
    12090,     // 20
    11630,     // 21
    11200,     // 22
    10780,     // 23
    10380,     // 24
    10000,     // 25
    9634,      // 26
    9283,      // 27
    8946,      // 28
    8623,      // 29
    8314,      // 30
    8017,      // 31
    7733,      // 32
    7460,      // 33
    7198,      // 34
    6946,      // 35
    6705,      // 36
    6474,      // 37
    6251,      // 38
    6038,      // 39
    5832,      // 40
    5635,      // 41
    5446,      // 42
    5264,      // 43
    5089,      // 44
    4921,      // 45
    4759,      // 46
    4603,      // 47
    4454,      // 48
    4310,      // 49
    4171,      // 50
    4037,      // 51
    3909,      // 52
    3785,      // 53
    3666,      // 54
    3551,      // 55
    3440,      // 56
    3334,      // 57
    3231,      // 58
    3132,      // 59
    3036,      // 60
    2944,      // 61
    2855,      // 62
    2769,      // 63
    2686,      // 64
    2606,      // 65
    2529,      // 66
    2454,      // 67
    2382,      // 68
    2313,      // 69
    2245,      // 70
    2181,      // 71
    2118,      // 72
    2057,      // 73
    1999,      // 74
    1942,      // 75
    1887,      // 76
    1834,      // 77
    1783,      // 78
    1734,      // 79
    1686,      // 80
    1640,      // 81
    1595,      // 82
    1552,      // 83
    1510,      // 84
    1469,      // 85
    1430,      // 86
    1392,      // 87
    1355,      // 88
    1319,      // 89
    1285,      // 90
    1251,      // 91
    1219,      // 92
    1187,      // 93
    1157,      // 94
    1127,      // 95
    1099,      // 96
    1071,      // 97
    1044,      // 98
    1018,      // 99
    992        // 100---992.40
};

const static uint32_t g_FDKPercentTable[] = {3100,3340,3539,3617,3662,3695,3716,3733,3750,3765,3774,3787,3795,3798,3813,3821,3828,3837,3845,3853,3863,3873,3882,3892,3894,3914,3958,3986,4018,4062,4117,4200};
const static uint32_t g_XYSRPercentTable[] = {3100,3358,3401,3494,3533,3560,3586,3606,3624,3637,3650,3660,3670,3680,3688,3698,3706,3713,3721,3730,3738,3739,3746,3760,3761,3768,3769,3775,3779,3795,3801,3815,3824,3837,3854,3876,3910,3940,3990,4030,4100,4200};
    
static int16_t g_sleepStateHandle = -1;                         //获取休眠状态handle
static int16_t g_batterySdkTimerHandle= -1;                      //timer句柄
static const BatteryConfigure_t *g_BatteryConfigure = NULL;    //注册异常处理回调接口
static uint32_t g_batteryTemperature = 0;                       //备用电池温度
static uint32_t g_batteryVoltage = 0;                           //备用电池电压
static uint8_t g_sleepState = 0;                                //休眠状态，0：休眠1：唤醒
static uint16_t g_cycleTime = 10;                               //>0，单位ms
static uint8_t g_getTempSuccessFlag = 0;                        //获取温度成功标记0：未成功 1：成功
static uint8_t g_getVolSuccessFlag = 0;                         //获取电压成功标记 0：未成功，1：成功
static uint16_t g_adGetDelayTimeCount = 0;                      //重新获取状态时间计数
static uint8_t g_adGetStateFlag = 0;                            //重新获取状态标记，0：需要重新获取，1：不需要获取
static BatteryState_e g_batteryState = E_BatteryState_Init;     //电池管理状态机
static uint8_t g_testModeFlag = 0;                              //测试模式标记 0：非测试模式1：测试模式
static SemaphoreHandle_t g_mutexHandle = NULL;                  //互斥锁句柄
static uint8_t g_batteryConnectFlag = 1;                        //备用电池连接状态 0：连接   1：未连接
static BatteryType_E g_batteryTpye = E_BATTERY_XYSR;            //备用电池电池型号
static uint8_t g_batteryVoltageAlarmFlag = 0;                   //备用电池电压异常警告标记  0：normal 1:低压警告 2：高压警告
static uint8_t g_batteryTempAlarmFlag = 0;                      //备用电池温度异常警告标记  0：normal 1：低温警告 2；高温警告

/*************************************************
  Function:       BatterySdkAdConversion
  Description:    模块内部接口，将AD采集的电压数据转换为温感的电阻值
  Input:          ntcValue：采集到的电压
  Output:         resisterOut： 温感电阻值
  Return:         无
  Others:         
*************************************************/
static void BatterySdkAdConversion(uint32_t ntcValue,uint32_t *resisterOut)
{
    int32_t refVoltage;
    uint32_t resisterPull;
    refVoltage = 3300;//3300mv
    resisterPull = 20000;//20K
    
    if(ntcValue>=refVoltage)
    {
        ntcValue = refVoltage - 1;
    }
    *resisterOut = (resisterPull*ntcValue)/(refVoltage-ntcValue);
    
    return;
}

/*************************************************
  Function:       ResisterToTemperature
  Description:    模块内部接口，温感的电阻值转换为温度值
  Input:          resisterIn：电阻值
  Output:         tmpValue： 温度值  0.01C
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
static int16_t ResisterToTemperature(uint32_t resisterIn,int32_t *tmpValue)
{
  int32_t size;
  int32_t i;
  int32_t j;
  int32_t half;
  //uint32_t tem;
  
  if(resisterIn>=g_temperatureRMap[0])
  {
    *tmpValue = -5000;
    return 0;
  }
  
  size = sizeof(g_temperatureRMap)/sizeof(g_temperatureRMap[0]);
  i = 0;
  j = size;

  while(1)
  {
    if(j-i<2)
    {
      break;
    }
    half = (i+j)/2;
    if(resisterIn>g_temperatureRMap[half])
    {
      j = half;
    }
    else if(resisterIn==g_temperatureRMap[half])
    {
    	j = half;
    	i = half;
    }
    else
    {
      i = half;
    }
  }
  if(i==j)
  {
    *tmpValue = (i-40)*100;//unit:0.01C
  }
  else
  {
    //tem = 
    *tmpValue = (i*100)+((j-i)*100)*(g_temperatureRMap[i]-resisterIn)/(g_temperatureRMap[i]-g_temperatureRMap[j]);
    *tmpValue -= 40*100;//unit:0.01C
  }
  
  return 0;
}

static void BatterySdkTmpErrorProcess()
{
    uint8_t lowFlag = 0;
    uint8_t highFlag = 0;

    if(g_batteryTemperature>=g_temperatureRMap[0])//<=-40C
    {
        lowFlag = 1; //电池未连接
        g_batteryConnectFlag = 1;
    }
    else
    {
        if(g_batteryConnectFlag == 1)
        {
            g_batteryConnectFlag = 0;
        }
    }
    if(g_BatteryConfigure == NULL)
    {
        return;
    }
    if(g_batteryTemperature<=g_temperatureRMap[g_BatteryConfigure->tempHighErrorValue+40])//>=50
    {
        highFlag = 1;
    } 

    if(g_batteryTemperature < g_temperatureRMap[g_BatteryConfigure->tempHighAlarmValue+40]) //>70,温度报警
    {
        g_batteryTempAlarmFlag = 2;
    }
    else
    {
        g_batteryTempAlarmFlag = 0;
    }
    
    if(g_BatteryConfigure->batteryNoPlugInErrorFun != NULL)
    {
        g_BatteryConfigure->batteryNoPlugInErrorFun(lowFlag);
    }
    if(g_BatteryConfigure->batteryTemperatureHighErrorFun!=NULL)
    {
        g_BatteryConfigure->batteryTemperatureHighErrorFun(highFlag);
    }
    return ;
}

static void BatterySdkVoltageErrorProcess(uint32_t volValue)
{
    uint8_t lowFlag = 0;
    uint8_t highFlag = 0;

    if(g_BatteryConfigure!=NULL)
    {
        if(volValue>g_BatteryConfigure->voltageHighErrorValue)    //>5V
        {
            highFlag = 1;
            g_batteryVoltageAlarmFlag = 2;
        }
        if(volValue<g_BatteryConfigure->voltageLowErrorValue)     //200mv
        {
            lowFlag = 1;
            g_batteryVoltageAlarmFlag = 1;
        }
        else
        {
            g_batteryVoltageAlarmFlag = 0;
        }

        if(g_BatteryConfigure->batteryVoltageHighErrorFun != NULL)
        {
            g_BatteryConfigure->batteryVoltageHighErrorFun(highFlag);
        }
        if(g_BatteryConfigure->batteryVoltageLowErrorFun!=NULL)
        {
            g_BatteryConfigure->batteryVoltageLowErrorFun(lowFlag);
        }
    }
    return ;
}

static void BatterySdkChargeStateTemperatureCheck(void)
{
    if(g_getTempSuccessFlag == 1)
    {
        if(g_batteryTemperature>=g_temperatureRMap[0+40]||g_batteryTemperature<=g_temperatureRMap[50+40])
        {
            g_batteryState = E_BatteryState_Wait5MinDelay;
            //开始5min计时 
            TimerHalStartTime(g_batterySdkTimerHandle,5*60*1000);
        }
        
    }
    else if(g_sleepState == 0)
    {
        g_batteryState = E_BatteryState_StopCharge;
        BatteryHalDisableCharge();
    }
}

static void BatterySdkChargeStateWait5MinDelay(void)
{
     if(TimerHalIsTimeout(g_batterySdkTimerHandle)== 0)
     {
         TimerHalStopTime(g_batterySdkTimerHandle);
         if(g_sleepState == 0)
         {
             g_batteryState = E_BatteryState_StopCharge;
             BatteryHalDisableCharge();
         }
         else
         {
             g_batteryState = E_BatteryState_BatteryTemperatureCheck;
         }
     }
     else if(g_sleepState == 0)
     {
         g_batteryState = E_BatteryState_StopCharge;
         BatteryHalDisableCharge();
     }
}

static void BatterySdkChargeStateVoltageCheck(void)
{
    if(g_getVolSuccessFlag == 1)
    {
        uint32_t batteryMaxVoltage;
        uint32_t batteryMinVoltage;

        batteryMaxVoltage = g_cellMaxVoltage * g_cellNumber;
        batteryMinVoltage = g_cellMinVoltage * g_cellNumber;
        
        if(g_batteryVoltage>batteryMaxVoltage)
        {
            g_batteryState = E_BatteryState_HealthCheck;
        }
        else
        {
            if(g_batteryVoltage>batteryMinVoltage)
            {
                g_batteryState = E_BatteryState_MiddleVoltageCharge;
            }
            else
            {
                g_batteryState = E_BatteryState_LowVoltageCharge;
            }
            //开始6H计时 
            TimerHalStartTime(g_batterySdkTimerHandle,g_middleVoltageChargeTime);
            //开始充电
            BatteryHalEnableCharge();
        }
    }
    else if(g_sleepState == 0)
    {
        g_batteryState = E_BatteryState_StopCharge;
        BatteryHalDisableCharge();
    }
}

static void BatterySdkChargeStateMiddleVoltageCharge(void)
{
    //timer是否超时
    if(TimerHalIsTimeout(g_batterySdkTimerHandle)== 0)
    {
        //停止计时
        TimerHalStopTime(g_batterySdkTimerHandle);
        //停止充电
        BatteryHalDisableCharge();
        g_batteryState = E_BatteryState_TrickleCharge;
    }
    else if(g_sleepState == 0)
    {
        g_batteryState = E_BatteryState_StopCharge;
        //停止充电
        BatteryHalDisableCharge();
    }
    else
    {
        g_batteryState = E_BatteryState_MiddleVoltageChargeCheckTem;
    }
}

static void BatterySdkChargeStateMiddleVoltageChargeCheckTem(void)
{
    if(g_getTempSuccessFlag)
    {
        if(g_batteryTemperature>=g_temperatureRMap[0+40]||g_batteryTemperature<=g_temperatureRMap[60+40])
        {
            g_batteryState = E_BatteryState_MiddleVoltageChargeFinish;
            //停止充电
            BatteryHalDisableCharge();
        }
        else if(g_sleepState == 0)
        {
            g_batteryState = E_BatteryState_StopCharge;
            //停止充电
            BatteryHalDisableCharge();
            //停止计时
            TimerHalStopTime(g_batterySdkTimerHandle);
        }
        else
        {
            g_batteryState = E_BatteryState_MiddleVoltageCharge;
        }
    }
    else if(g_sleepState == 0)
    {
        g_batteryState = E_BatteryState_StopCharge;
        //停止充电
        BatteryHalDisableCharge();
    }
}

static void BatterySdkChargeStateLowVoltageCharge(void)
{
    if(TimerHalIsTimeout(g_batterySdkTimerHandle)== 0)
    {
        TimerHalStopTime(g_batterySdkTimerHandle);
        //停止充电
        BatteryHalDisableCharge();
        g_batteryState = E_BatteryState_LowVoltageChargeFinish;
    }
    else if(g_sleepState == 0)
    {
        g_batteryState = E_BatteryState_StopCharge;
        //停止充电
        BatteryHalDisableCharge();
    }
    else
    {
        g_batteryState = E_BatteryState_LowVoltageChargeCheckTem;
    }
}

static void BatterySdkChargeStateLowVoltageChargeCheckTem(void)
{
    if(g_getTempSuccessFlag == 1)
    {
        if(g_batteryTemperature>=g_temperatureRMap[0+40]||g_batteryTemperature<=g_temperatureRMap[60+40])
        {
            g_batteryState = E_BatteryState_StopCharge;
            BatteryHalDisableCharge();
            TimerHalStopTime(g_batterySdkTimerHandle);
        }
        else
        {
            g_batteryState = E_BatteryState_LowVoltageCharge;
        }
    }
    else if(g_sleepState == 0)
    {
        g_batteryState = E_BatteryState_StopCharge;
        //停止充电
        BatteryHalDisableCharge();
    }
}

static void BatterySdkChargeStateChargeFinishCheckTem(void)
{
    if(g_getTempSuccessFlag == 1)
    {
        if(g_batteryTemperature>=g_temperatureRMap[0+40]||g_batteryTemperature<=g_temperatureRMap[60+40])
        {
            g_batteryState = E_BatteryState_ChargeFinishDelay10Min;
            //开始10min计时 
            TimerHalStartTime(g_batterySdkTimerHandle,10*60*1000);
        }
        else if(g_sleepState == 0)
        {
            g_batteryState = E_BatteryState_StopCharge;
            //停止充电
            BatteryHalDisableCharge();
        }
        else
        {
            g_batteryState = E_BatteryState_ChargeFinishCheckTem;
        }
    }
    else if(g_sleepState == 0)
    {
        g_batteryState = E_BatteryState_StopCharge;
        //停止充电
        BatteryHalDisableCharge();
    }
}

static void BatterySdkChargeStateChargeFinishDelay10Min(void)
{
    if(TimerHalIsTimeout(g_batterySdkTimerHandle)== 0)
    {
        TimerHalStopTime(g_batterySdkTimerHandle);
        //停止充电
        BatteryHalDisableCharge();
        g_batteryState = E_BatteryState_ChargeFinishCheckTem;
    }
    else if(g_sleepState == 0)
    {
        g_batteryState = E_BatteryState_StopCharge;
        //停止充电
        BatteryHalDisableCharge();
    }
}

static void BatterySdkChargeStateInit(void)
{
    if(g_getTempSuccessFlag == 1)
    {
        g_batteryState = E_BatteryState_BatteryTemperatureCheck;
    }
    else if(g_sleepState == 0)
    {
        g_batteryState = E_BatteryState_StopCharge;
        BatteryHalDisableCharge();
    }
}

static void BatterySdkChargeStateManage()
{
    if(g_batteryState == E_BatteryState_Init)
    {
        BatterySdkChargeStateInit();
    }
    else if(g_batteryState == E_BatteryState_BatteryTemperatureCheck)
    {
        BatterySdkChargeStateTemperatureCheck();
    }
    else if(g_batteryState == E_BatteryState_Wait5MinDelay)
    {
        BatterySdkChargeStateWait5MinDelay();
    }
    else if(g_batteryState == E_BatteryState_BatteryVoltageCheck)
    {
        BatterySdkChargeStateVoltageCheck();
    }
    else if(g_batteryState == E_BatteryState_MiddleVoltageCharge)
    {
        BatterySdkChargeStateMiddleVoltageCharge();
    }
    else if(g_batteryState == E_BatteryState_MiddleVoltageChargeCheckTem)
    {
        BatterySdkChargeStateMiddleVoltageChargeCheckTem();
    }
    else if(g_batteryState == E_BatteryState_MiddleVoltageChargeFinish)
    {
        if(g_sleepState == 0)
        {
            g_batteryState = E_BatteryState_StopCharge;
            //停止充电
            BatteryHalDisableCharge();
        }
    }
    else if(g_batteryState == E_BatteryState_LowVoltageCharge)
    {
        BatterySdkChargeStateLowVoltageCharge();
    }
    else if(g_batteryState == E_BatteryState_LowVoltageChargeCheckTem)
    {
        BatterySdkChargeStateLowVoltageChargeCheckTem();
    }
    else if(g_batteryState == E_BatteryState_LowVoltageChargeFinish)
    {
        if(g_sleepState == 0)
        {
            g_batteryState = E_BatteryState_StopCharge;
            //停止充电
            BatteryHalDisableCharge();
        }
    }
    else if(g_batteryState == E_BatteryState_HealthCheck)
    {
        g_batteryState = E_BatteryState_TrickleCharge;
    }
    else if(g_batteryState == E_BatteryState_TrickleCharge)
    {
        g_batteryState = E_BatteryState_ChargeFinishCheckTem;
    }
    else if(g_batteryState == E_BatteryState_ChargeFinishCheckTem)
    {
        BatterySdkChargeStateChargeFinishCheckTem();
    }
    else if(g_batteryState == E_BatteryState_ChargeFinishDelay10Min)
    {
        BatterySdkChargeStateChargeFinishDelay10Min();
    }
    else if(g_batteryState == E_BatteryState_StopCharge)
    {
        //唤醒
        if(g_sleepState == 1)
        {
            g_batteryState = E_BatteryState_Init;
        }
    }
}

/*************************************************
  Function:       BatterySdkInit
  Description:    SDK备用电池管理模块初始化接口
  Input:          pBatteryConfigure：错误处理回调注册
                  cycleTime：设置process周期调用接口调用周期
                  batteryType：电池型号
  Output:         函数执行结果
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkInit(const BatteryConfigure_t *pBatteryConfigure,uint16_t cycleTime,BatteryType_E batteryType)
{
    const char modulName[] = {'B','a','t','t','e','r','y','S','d','k'};
    if(pBatteryConfigure != NULL)
    {
        g_BatteryConfigure = pBatteryConfigure;
    }
    if(cycleTime > 0)
    {
        g_cycleTime = cycleTime;
    }
    g_batterySdkTimerHandle = TimerHalOpen();
    g_sleepStateHandle = PowerManageSdkOpenHandle(modulName);
    g_batteryTpye = batteryType;
    
    g_mutexHandle = xSemaphoreCreateMutex();
    return 0;
}

/*************************************************
  Function:       BatterySdkCycleProcess
  Description:    备用电池管理周期调用接口，该接口需要以一定周期进行循环调用
  Input:          无
  Output:         无
  Return:         
  Others:         
*************************************************/
void BatterySdkCycleProcess(void)
{
    int16_t ret = -1;
    uint32_t tmpValue = 0;
    uint32_t volValue = 0;
    
    //是否测试模式
    if(g_testModeFlag == 1)
    {
        //休眠状态设置为唤醒
        g_sleepState = 1;
    }
    else
    {
        g_sleepState=PowerManageSdkGetSleepState(g_sleepStateHandle);
        //是否休眠状态
        if(g_sleepState == 0)
        {
            //获取休眠状态成功响应
            PowerManageSdkSetSleepAck(g_sleepStateHandle);
        }
    
    }
    if(g_getTempSuccessFlag==0 || g_adGetStateFlag==0)
    {
        //获取NTC电压
        ret = BatteryHalGetNtc(&tmpValue);
        if(ret == 0)
        {
            //计算真实温度值
            xSemaphoreTake(g_mutexHandle, portMAX_DELAY);
            BatterySdkAdConversion(tmpValue,&g_batteryTemperature);
            xSemaphoreGive(g_mutexHandle);
            g_getTempSuccessFlag = 1;
            g_adGetStateFlag = 1;
            //温度异常处理
            BatterySdkTmpErrorProcess();
        }
    }
    if((g_getVolSuccessFlag==0) || (g_adGetStateFlag==0))
    {
        //获取电压
        ret = BatteryHalGetVoltage(&volValue);
        if(ret == 0)
        {
            xSemaphoreTake(g_mutexHandle, portMAX_DELAY);
            g_batteryVoltage = volValue;
            g_getVolSuccessFlag = 1;
            g_adGetStateFlag = 1;
            //电压异常处理
            BatterySdkVoltageErrorProcess(g_batteryVoltage);
            xSemaphoreGive(g_mutexHandle);
        }
    }
    
    if(g_adGetStateFlag == 1)
    {
        g_adGetDelayTimeCount++;
        
        //10s获取一次
        if(g_adGetDelayTimeCount>(1000/g_cycleTime))
        {
            //重新获取状态数据
            g_adGetStateFlag = 0;
            g_getTempSuccessFlag = 0;
            g_getVolSuccessFlag = 0;
            g_adGetDelayTimeCount = 0;
        }
    }
    
    //是否正常模式
    if(g_testModeFlag == 0)
    {
        //备用电池充电状态管理
        BatterySdkChargeStateManage();
    }
}

/*************************************************
  Function:       BatterySdkGetVoltage
  Description:    获取备用电池电压接口
  Input:          无
  Output:         pVoltage：电压值，单位mv
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkGetVoltage(uint32_t *pVoltage)
{
    if(g_getVolSuccessFlag == 0)
    {
        return -1;//value not valid
    }
    //RTOS_HalDisableScheduler();
    xSemaphoreTake(g_mutexHandle, portMAX_DELAY);
    *pVoltage = g_batteryVoltage;  
    //RTOS_HalEnableScheduler();
    xSemaphoreGive(g_mutexHandle);
    return 0;
}

/*************************************************
  Function:       BatterySdkGetTemperature
  Description:    获取备用电池温度接口
  Input:          无
  Output:         pTemperature：备用电池温度值，单位：0.001摄氏度
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkGetTemperature(int32_t *pTemperature)
{
    uint32_t resister;
    if(g_getTempSuccessFlag == 0)
    {
        return -1;//value not valid
    }
    //RTOS_HalDisableScheduler();
    xSemaphoreTake(g_mutexHandle, portMAX_DELAY);
    resister = g_batteryTemperature;
    //RTOS_HalEnableScheduler();
    xSemaphoreGive(g_mutexHandle);
    ResisterToTemperature(resister,pTemperature);
    return 0; 
}

/*************************************************
  Function:       BatterySdkSetTestMode
  Description:    设置备用电池进入测试模式接口
  Input:          无
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkSetTestMode(void)
{
    g_testModeFlag = 1;
    return 0;
}

/*************************************************
  Function:       BatterySdkSetNormalMode
  Description:    设置备用电池从测试模式进入正常模式接口
  Input:          无
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkSetNormalMode(void)
{
    //退出测试模式
    g_testModeFlag = 0;
    
    //放电失能
    BatteryHalDisableOut();
    
    return 0;
}

/*************************************************
  Function:       BatterySdkTestModeChargeStart
  Description:    设置测试模式下备用电池充电接口
  Input:          无
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkTestModeChargeStart(void)
{
    //是否测试模式
    if(g_testModeFlag == 1)
    {
        //充电使能
        BatteryHalEnableCharge();
        return 0;
    }
    
    return -1;
}

/*************************************************
  Function:       BatterySdkTestModeChargeStop
  Description:    设置测试模式下备用电池停止充电接口
  Input:          无
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkTestModeChargeStop(void)
{
    //是否测试模式
    if(g_testModeFlag == 1)
    {
        //充电失能
        BatteryHalDisableCharge();
        return 0;
    }
    
    return -1;
}

/*************************************************
  Function:       BatterySdkTestModeDischargeStart
  Description:    设置测试模式下备用电池放电接口
  Input:          无
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkTestModeDischargeStart(void)
{
    //是否测试模式
    if(g_testModeFlag == 1)
    {
        //放电使能
        BatteryHalEnableOut();
        return 0;
    }
    
    return -1;
}

/*************************************************
  Function:       BatterySdkTestModeDischargeStop
  Description:    设置测试模式下备用电池停止放电接口
  Input:          无
  Output:         无
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t BatterySdkTestModeDischargeStop(void)
{
    //是否测试模式
    if(g_testModeFlag == 1)
    {
        //放电失能
        BatteryHalDisableOut();
        return 0;
    }
    
    return -1;
}

/*************************************************
  Function:       BatterySdkGetBatteyStatus
  Description:    获取备用电池的连接状态
  Input:          无
  Output:         无
  Return:         0：连接
                  1：未连接
  Others:         
*************************************************/
uint8_t BatterySdkGetBatteyStatus(void)
{
    return g_batteryConnectFlag;
}

/*************************************************
  Function:       BatterySdkGetBatteySoc
  Description:    获取备用电池的电池电量接口
  Input:          无
  Output:         无
  Return:         电池电量百分比
  Others:         
*************************************************/
uint32_t BatterySdkGetBatteySoc(void)
{
    uint32_t i = 0;
    uint32_t percentTableSize;
    const uint32_t *pTable = NULL; 
    uint32_t section = 0;
    uint32_t voltage = 0;
    
    if(g_batteryTpye == E_BATTERY_FDK)
    {
        percentTableSize = sizeof(g_FDKPercentTable) / sizeof(g_FDKPercentTable[0]);
        pTable = g_FDKPercentTable;
    }
    else if(g_batteryTpye == E_BATTERY_XYSR)
    {
        percentTableSize = sizeof(g_XYSRPercentTable) / sizeof(g_XYSRPercentTable[0]);
        pTable = g_XYSRPercentTable;
    }
    else
    {
        return 0;
    }
    section = (100 * 1000) / (percentTableSize - 1);
    
    if(BatterySdkGetVoltage(&voltage) != 0)
    {
        return 0;
    }
	if(voltage < pTable[0])
    {
		return 0;
    }
    
	for(i=0; i< percentTableSize; i++){
		if(voltage < pTable[i])
			return (i*section - (section * (uint32_t)(pTable[i] - voltage)) / 
			(uint32_t)(pTable[i] - pTable[i-1]))/1000;
	}
 
	return 100;    
}

uint8_t BatterySdkGetBatteyVoltageError(void)
{
    return g_batteryVoltageAlarmFlag;
}

/*************************************************
  Function:       BatterySdkGetBatteyVoltageAlarm
  Description:    获取备用电池的电量过低报警状态
  Input:          无
  Output:         0：nomal
                  1：电量过低警告
  Others:         
*************************************************/
uint8_t BatterySdkGetBatteyVoltageAlarm(void)
{
    if(g_BatteryConfigure != NULL)
    {
        if(BatterySdkGetBatteySoc() < g_BatteryConfigure->batterySocAlarmValue)
        {
            return 1;
        }
    }
    return 0;
}

/*************************************************
  Function:       BatterySdkGetBatteyTempAlarm
  Description:    获取备用电池的温度报警状态
  Input:          无
  Output:         0：nomal
                  1：预留
                  2：高温警告
  Others:         
*************************************************/
uint8_t BatterySdkGetBatteyTempAlarm(void)
{
    return g_batteryTempAlarmFlag;
}

/*************************************************
  Function:       BatterySdkGetBatteyChargingDuration
  Description:    获取备用电池的充电时长,单位s
  Input:          无
  Output:         0：未充电
                  >0:充电时长
  Others:         
*************************************************/
uint32_t BatterySdkGetBatteyChargingDuration(void)
{
    uint32_t time = 0;
    if(BatteryHalGetState() == 1) // 充电
    {
        if(g_batterySdkTimerHandle < 0)
        {
            return 0;
        }
        time = TimerHalGetRemainder(g_batterySdkTimerHandle);
        if(time == 0)
        {
            return 0;
        }
        return (g_middleVoltageChargeTime - time)/1000;
    }
    return 0;
}


