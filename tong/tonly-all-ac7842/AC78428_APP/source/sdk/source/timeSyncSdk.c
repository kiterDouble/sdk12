#include <stdlib.h>
#include <stdint.h>

#include "powerManageSdk.h"
#include "timerHal.h"
#include "timeSyncSdk.h"



typedef struct
{
    uint32_t timestamp; //0-4294967295, UNIX时间戳，单位S ，UTC时间.时间未同步时填0
    uint8_t timeZone; //时区 1-25
    uint8_t timeSrc;  //时间源，0-无效 1-NTP  2-GNSS
    uint8_t timeSyncStat; //时间同步状态 0：未同步 1：同步
    uint8_t timeValidity;  //数据有效性，0：无效 1有效
}MpuTime_t;

const static uint8_t g_monTable[12]={31,28,31,30,31,30,31,31,30,31,30,31};
#define RTC_TIME_START_YEAR      (2000)

//static int16_t g_mpuHandle = -1;  //MPU通信句柄
static int16_t g_sleepStateHandle = -1;   //休眠状态获取句柄
static MpuTime_t g_timeData;      //时间数据保存
//static uint16_t g_cycleTime = 0;


/*************************************************
  Function:       TimeSyncSdkInit
  Description:    sdk层时间同步模块初始化接口
  Input:          mpuHandle:mpu的通信句柄
                  cycleTime：周期接口调用周期设置
  Output:         无
  Return:         无
  Others:         
*************************************************/
void TimeSyncSdkInit(int16_t mpuHandle,uint16_t cycleTime)
{
    const char moduleName[] = {'t','i','m','e','s','y','n','c','s','d','k'};
    //g_mpuHandle = mpuHandle;
    //g_cycleTime = cycleTime;
    g_timeData.timeValidity = 0;
    g_sleepStateHandle = PowerManageSdkOpenHandle(moduleName);
}

/*************************************************
  Function:       TimeSyncSdkCycleProcess
  Description:    sdk层时间同步模块初始化接口
  Input:          pRxData：接收到的MPU的时间同步数据
                  rxLength：接收到的MPU时间同步数据长度
  Output:         无
  Return:         无
  Others:         
*************************************************/
void TimeSyncSdkCycleProcess(MpuHalDataPack_t *msgData)
{
    int16_t sleepFlag = 1; //0:休眠 1：唤醒
    uint8_t *pRxData = NULL;
    
    sleepFlag = PowerManageSdkGetSleepState(g_sleepStateHandle);
    if(sleepFlag == 0)
    {
        PowerManageSdkSetSleepAck(g_sleepStateHandle);
        g_timeData.timeValidity = 0;
    }
    else
    {
        if(msgData != NULL && msgData->pDataBuffer != NULL  &&  msgData->dataLength >0)
        {
            pRxData = msgData->pDataBuffer;
            /**************
            Timestamp	UINT32
            TimeZone	UINT8
            Status	UINT8  Bit0-Bit3 时间同步状态：0-未同步1-已同步
                           Bit4-Bit7  时间源：0-无效 1-NTP 2-GNSS
            **************/
            if(((pRxData[5]&0x0F) != 0) && (((pRxData[5]>>4)&0x0F) != 0))
            {
                g_timeData.timeSyncStat = pRxData[5] & 0x0F;
                g_timeData.timeSrc = (pRxData[5]>>4) & 0x0F;
                g_timeData.timestamp = (pRxData[0]<<24)+(pRxData[1]<<16)+(pRxData[2]<<8) + pRxData[3];
                g_timeData.timeValidity = 1;
                TimerHalSetRtcTime(g_timeData.timestamp);
            }
        }
    }
    
    return;
}

/**********************************************
* 函数名 : TimeSyncGetLocalTimeFromRtc
* 描  述 : 将2000年后的RTC时间转换为年月日时分秒
* 输  入 : rtc 2000年以后的秒数
* 输  出 : 当前的年月日时分秒
* 返回值 : 0,成功;其他:错误代码.
**********************************************/
static int16_t TimeSyncGetLocalTimeFromRtc(uint32_t rtc,uint8_t timeZone, uint32_t *year, uint8_t *mon, uint8_t *day, uint8_t *hour, uint8_t *min, uint8_t *sec)
{
	uint64_t temp=0;
	uint32_t temp1=0;	  
  
	if(rtc < 946656000)
	{
		return -1;
	}
    rtc =rtc - 946656000 + (timeZone * 60 *60);
	temp=rtc/86400;	//得到天数(秒钟数对应的)

	temp1=2000; //从1970年开始
	while(temp>=365)
	{				 
		if((temp1%400 == 0)||(temp1%4 == 0 && temp1%100 != 0))//判断是否为闰年，年号能被400整除或年号能被4整除，而不能被100整除为闰年
		{
			if(temp>=366)
            {
                temp-=366;//闰年的秒钟数
            }
			else 
            {
                //temp1++;
                break;
            }  
		}
		else
        {
            temp-=365;   //平年 
        }
		temp1++;  
        if(temp1 > 2099)
        {
            return -1;//大于2099年，退出
        }
	}	
	*year = temp1;//得到年份
	temp1=0;
	while(temp>=28)//超过了一个月
	{
		if(((*year%400 == 0)||(*year%4 == 0 && *year%100 != 0))&&(temp1==1))//当年是不是闰年/2月份
		{
			if(temp>=29)
            {
                temp-=29;//闰年的秒钟数
            }
			else
            {
                break;
            }
		}
		else 
		{
			if(temp>=g_monTable[temp1])
            {
                temp-=g_monTable[temp1];//平年
            }
			else
            {
                break;
            }
		}
		temp1++;
	}
	*mon=temp1+1;	//得到月份
	*day=temp+1;	//得到日期 
	temp=rtc%86400;			//得到秒钟数		   
	*hour=temp/3600;		//小时
	*min=(temp%3600)/60;	//分钟	
	*sec=(temp%3600)%60;	//秒钟

    return 0;
}

/**********************************************
* 函数名 : RTC_GetSecFrom2000
* 描  述 : 得到2000年以后的秒数
* 输  入 : 当前的年月日时分秒
* 输  出 : None
* 返回值 : 2000年以后的秒数  
**********************************************/
#if 0
static uint32_t TimeSyncGetRtcFromLocalTime(uint16_t year,uint8_t month,uint8_t day,uint8_t hour,uint8_t min, uint8_t sec)
{
  uint32_t seccount = 0;//一定要初始化
  uint16_t t;

  if(year < RTC_TIME_START_YEAR || year > 2099)
	return 0;	  
  
  for(t= RTC_TIME_START_YEAR;t<year;t++)  
  {
  	if(RTC_IsLeapYear(t))
  	  seccount += 31622400;
  	else 
  	  seccount += 31536000; 	   
  }
  month-=1;
  for(t=0;t<month;t++)	  
  {
  	seccount += (uint32_t)mon_table[t]*86400;
  	if(RTC_IsLeapYear(year)&&t==1)
  	  seccount+=86400;
  }
  seccount += (uint32_t)(day-1)*86400;
  seccount += (uint32_t)hour*3600;
  seccount += (uint32_t)min*60;   //分钟秒钟数
  seccount += sec;

  seccount += 946656000;
  return seccount;
}
#endif


/*************************************************
  Function:       TimeSyncSdkGetRealTime
  Description:    获取有格式的时间
  Input:          无
  Output:         timeZone：时间差
                  pYear：年
                  pMonth：月
                  pDay：日
                  pHour：时
                  pMin：分
                  pSecond：秒
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t TimeSyncSdkGetRealTime(uint8_t timeZone, TimeSevice_t *pTimeOut)
{
    uint32_t time = 0U;
    int8_t ret = 0U;
    if ((pTimeOut == NULL) || (g_timeData.timeValidity == 0U))
    {
        ret = -1;
    }
    else
    {
        TimerHalGetRtcTime(&time);
        TimeSyncGetLocalTimeFromRtc(time, timeZone,
                                 &pTimeOut->year,
                                 &pTimeOut->month,
                                 &pTimeOut->day,
                                 &pTimeOut->hour,
                                 &pTimeOut->minute,
                                 &pTimeOut->second);
    }
    return ret;
}

/*************************************************
  Function:       TimeSyncSdkGetUtcTime
  Description:    获取有格式的时间
  Input:          无
  Output:         pTime:时间戳
  Return:         0：成功
                  -1：失败
  Others:         
*************************************************/
int16_t TimeSyncSdkGetUtcTime(uint32_t *pTime)
{
    uint32_t time = 0;
    
    if(g_timeData.timeValidity == 0)
    {
        return -1;
    }
    
    TimerHalGetRtcTime(&time);
    *pTime = time;
    return 0;
}



