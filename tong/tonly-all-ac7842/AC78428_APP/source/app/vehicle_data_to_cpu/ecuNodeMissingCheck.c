#include <string.h>

#include "taskDtcProcess.h"
#include "canHal.h"
#include "peripheralHal.h"
#include "logHal.h"

#include "ecuNodeMissingCheck.h"

typedef int16_t (* NodeMissingEnableFun_t)(void);
#define SIZE_OF_ARRAY(name)     (sizeof(name)/sizeof(name[0]))

typedef struct
{
  uint32_t canId;
  uint32_t timeOut;//ms
  uint32_t recoverTime;  
  DtcItem_e dtcItem;
  NodeMissingEnableFun_t nodeMissingEnableCallBack;
}EcuNodeConfigure_t;

typedef struct
{
  uint32_t recoverCount;  
  uint32_t timeCount;
  uint8_t missingFlag;
}EcuNodeBuffer_t;


//230
//static int16_t NodeMsgMissingAdasConfig(void)
//{
//    return 0;
//}

#define CAN_1_USED 1
#define CAN_2_USED 0
#define CAN_3_USED 0
#define CAN_4_USED 0
#define CAN_5_USED 0
#define CAN_6_USED 0

#if CAN_1_USED
static const EcuNodeConfigure_t g_can1EcuNodeConfigure[] = 
{
    /***id  ,   timeout(ms),recovertime(ms) event call back**********************/
  //{0x1D2      ,250	,100    ,E_DTC_ITEM_NODE_MISSING_1D2 ,NodeMsgMissingAdasConfig      },//20ms
  //{0x329      ,1050	,500    ,E_DTC_ITEM_NODE_MISSING_329 ,NULL      },//100ms 
    // {0x310      ,1050	,500    ,E_DTC_ITEM_NODE_MISSING_310 ,NULL      },//100ms 
};
#endif

#if CAN_2_USED
static const EcuNodeConfigure_t g_can2EcuNodeConfigure[] = 
{
  /***id  ,   timeout(ms),recovertime(ms) event call back**********************/
  // {0x319      ,525	,1000   ,E_DTC_ITEM_NODE_MISSING_BCM1_319		,NULL},//50ms
  // {0x29D      ,1050	,1000   ,E_DTC_ITEM_NODE_MISSING_AC1_29D		,NULL},//10*100ms
  // {0x295      ,525	,1000   ,E_DTC_ITEM_NODE_MISSING_PEPS2_295	,NULL},//10*50ms
  // {0x2BB      ,1050	,1000   ,E_DTC_ITEM_NODE_MISSING_GWFD1_2BB	,NULL},//10*100ms
  // {0x27F      ,525  ,1000   ,E_DTC_ITEM_NODE_MISSING_IP2_27F		,NULL},//10*50ms
  // {0x341      ,5250	,1000   ,E_DTC_ITEM_NODE_MISSING_TPMS1_341	,NULL},//10*500ms
  // {0x2A8      ,1050	,1000   ,E_DTC_ITEM_NODE_MISSING_EEM1_2A8		,NULL},//10*100ms
  {0x310      ,1050	,500    ,E_DTC_ITEM_NODE_MISSING_310 ,NULL      },//100ms 
};
#endif
#if CAN_3_USED
static const EcuNodeConfigure_t g_can3EcuNodeConfigure[] = 
{
    /***id  ,   timeout(ms),recovertime(ms) event call back**********************/
    //{0x1E9      ,250	,100    ,E_DTC_ITEM_NODE_MISSING_1E9  ,NULL                   },//20ms 
    //{0x17D      ,1050	,500    ,E_DTC_ITEM_NODE_MISSING_17D  ,NULL                   },//100ms
};
#endif
#if CAN_4_USED
static const EcuNodeConfigure_t g_can4EcuNodeConfigure[] = 
{
    ///***id  ,   timeout(ms),recovertime(ms) event call back**********************/
//  {0x287      ,525	,250    ,E_DTC_ITEM_NODE_MISSING_287  ,NULL                   },//50ms
//  {0x297      ,525	,250    ,E_DTC_ITEM_NODE_MISSING_297  ,NULL                   },//50ms
};
#endif
#if CAN_5_USED
static const EcuNodeConfigure_t g_can5EcuNodeConfigure[] = 
{
    /***id  ,   timeout(ms),recovertime(ms) event call back**********************/
    //{0x120      ,1050	,500    ,E_DTC_ITEM_NODE_MISSING_120  ,NULL                   },//100ms
    //{0x332      ,1050	,500    ,E_DTC_ITEM_NODE_MISSING_332  ,NULL    },//100ms
};
#endif
#if CAN_6_USED
static const EcuNodeConfigure_t g_can6EcuNodeConfigure[] = 
{
    /***id  ,   timeout(ms),recovertime(ms) event call back**********************/
    //{0x120      ,1050	,500    ,E_DTC_ITEM_NODE_MISSING_120  ,NULL                   },//100ms
    //{0x332      ,1050	,500    ,E_DTC_ITEM_NODE_MISSING_332  ,NULL    },//100ms
};
#endif
static uint8_t g_missingCheckRestFlag= 0;
#if CAN_1_USED
static EcuNodeBuffer_t g_can1EcuNodeBuffer[sizeof(g_can1EcuNodeConfigure)/sizeof(g_can1EcuNodeConfigure[0])];
#endif
#if CAN_2_USED
static EcuNodeBuffer_t g_can2EcuNodeBuffer[sizeof(g_can2EcuNodeConfigure)/sizeof(g_can2EcuNodeConfigure[0])];
#endif
#if CAN_3_USED
static EcuNodeBuffer_t g_can3EcuNodeBuffer[sizeof(g_can3EcuNodeConfigure)/sizeof(g_can3EcuNodeConfigure[0])];
#endif
#if CAN_4_USED
static EcuNodeBuffer_t g_can4EcuNodeBuffer[sizeof(g_can4EcuNodeConfigure)/sizeof(g_can4EcuNodeConfigure[0])];
#endif
#if CAN_5_USED
static EcuNodeBuffer_t g_can5EcuNodeBuffer[sizeof(g_can5EcuNodeConfigure)/sizeof(g_can5EcuNodeConfigure[0])];
#endif
#if CAN_6_USED
static EcuNodeBuffer_t g_can6EcuNodeBuffer[sizeof(g_can5EcuNodeConfigure)/sizeof(g_can5EcuNodeConfigure[0])];
#endif

void EcuNodeMissingCheckCanId(uint8_t canChannel,uint32_t canId)
{

  uint32_t i;
  uint32_t num = 0;
  EcuNodeBuffer_t* pEcuBuffer = NULL;
  const EcuNodeConfigure_t* pEcuConfig = NULL;
  
  if(TBOX_CAN_CHANNEL_1==canChannel)
  {
#if CAN_1_USED
      pEcuBuffer = g_can1EcuNodeBuffer;
      pEcuConfig = g_can1EcuNodeConfigure;
      num = SIZE_OF_ARRAY(g_can1EcuNodeConfigure);
#endif
  }
  else if(TBOX_CAN_CHANNEL_2==canChannel)
  {
#if CAN_2_USED
      pEcuBuffer = g_can2EcuNodeBuffer;
      pEcuConfig = g_can2EcuNodeConfigure;
      num = SIZE_OF_ARRAY(g_can2EcuNodeConfigure);
#endif
  }
  else if(TBOX_CAN_CHANNEL_3==canChannel)
  {
#if CAN_3_USED
      pEcuBuffer = g_can3EcuNodeBuffer;
      pEcuConfig = g_can3EcuNodeConfigure;
      num = SIZE_OF_ARRAY(g_can3EcuNodeConfigure);
#endif
  }
  else if(TBOX_CAN_CHANNEL_4==canChannel)
  {
#if CAN_4_USED
      pEcuBuffer = g_can4EcuNodeBuffer;
      pEcuConfig = g_can4EcuNodeConfigure;
      num = SIZE_OF_ARRAY(g_can4EcuNodeConfigure);
#endif
  }
  else if(TBOX_CAN_CHANNEL_5==canChannel)
  {
#if CAN_5_USED
      pEcuBuffer = g_can5EcuNodeBuffer;
      pEcuConfig = g_can5EcuNodeConfigure;
      num = SIZE_OF_ARRAY(g_can5EcuNodeConfigure);
#endif  
  }
  else if(TBOX_CAN_CHANNEL_6==canChannel)
  {
#if CAN_6_USED
      pEcuBuffer = g_can6EcuNodeBuffer;
      pEcuConfig = g_can6EcuNodeConfigure;
      num = SIZE_OF_ARRAY(g_can6EcuNodeConfigure);
#endif
  }
  for(i=0;i<num;i++)
  {
    if(pEcuConfig[i].canId==canId)
    {
      pEcuBuffer[i].timeCount = 0x00;
      if(pEcuBuffer[i].recoverCount<100)
      {
        pEcuBuffer[i].recoverCount++;
      }      
    }
  }
}

uint8_t EcuNodeMissingStatus(uint8_t canChannel,uint32_t canId)
{

  uint32_t i;
  uint32_t num = 0;
  EcuNodeBuffer_t* pEcuBuffer = NULL;
  const EcuNodeConfigure_t* pEcuConfig = NULL;
  
  if(TBOX_CAN_CHANNEL_1==canChannel)
  {
#if CAN_1_USED
      pEcuBuffer = g_can1EcuNodeBuffer;
      pEcuConfig = g_can1EcuNodeConfigure;
      num = SIZE_OF_ARRAY(g_can1EcuNodeConfigure);
#endif
  }
  else if(TBOX_CAN_CHANNEL_2==canChannel)
  {
#if CAN_2_USED
      pEcuBuffer = g_can2EcuNodeBuffer;
      pEcuConfig = g_can2EcuNodeConfigure;
      num = SIZE_OF_ARRAY(g_can2EcuNodeConfigure);
#endif
  }
  else if(TBOX_CAN_CHANNEL_3==canChannel)
  {
#if CAN_3_USED
      pEcuBuffer = g_can3EcuNodeBuffer;
      pEcuConfig = g_can3EcuNodeConfigure;
      num = SIZE_OF_ARRAY(g_can3EcuNodeConfigure);
#endif
  }
  else if(TBOX_CAN_CHANNEL_4==canChannel)
  {
#if CAN_4_USED
      pEcuBuffer = g_can4EcuNodeBuffer;
      pEcuConfig = g_can4EcuNodeConfigure;
      num = SIZE_OF_ARRAY(g_can4EcuNodeConfigure);
#endif
  }
  else if(TBOX_CAN_CHANNEL_5==canChannel)
  {
#if CAN_5_USED
      pEcuBuffer = g_can5EcuNodeBuffer;
      pEcuConfig = g_can5EcuNodeConfigure;
      num = SIZE_OF_ARRAY(g_can5EcuNodeConfigure);
#endif  
  }
  else if(TBOX_CAN_CHANNEL_6==canChannel)
  {
#if CAN_6_USED
      pEcuBuffer = g_can6EcuNodeBuffer;
      pEcuConfig = g_can6EcuNodeConfigure;
      num = SIZE_OF_ARRAY(g_can6EcuNodeConfigure);
#endif
  }
  for(i=0;i<num;i++)
  {
    if(pEcuConfig[i].canId==canId)
    {
        if(pEcuBuffer[i].missingFlag == 0)
        {
            return 0;
        } 
    }
  }

  return 1;
}

//#define  NODE_MISS_CNT    (10)

//static uint32_t m_testCount = 0;

static void NodeMissingCheckTimeOutOnCan(const EcuNodeConfigure_t *pCheckConfigure,EcuNodeBuffer_t *pCheckBuffer,uint32_t configureSize,uint32_t cycleTime)
{
    uint32_t i;
    for(i=0;i<configureSize;i++)
    {       
        /*if(pCheckConfigure[i].canId==0x4C6)
        {
                m_testCount++;
        }*/
        if(pCheckConfigure[i].nodeMissingEnableCallBack!=NULL)
        {
          if(pCheckConfigure[i].nodeMissingEnableCallBack()!=0)//disabled
          {
            pCheckBuffer[i].timeCount = 0;
            pCheckBuffer[i].recoverCount = 100;
          }              
        }
        if((pCheckBuffer[i].timeCount) >= pCheckConfigure[i].timeOut)
        {         
             pCheckBuffer[i].recoverCount = 0;
            if(!pCheckBuffer[i].missingFlag)
            {
                pCheckBuffer[i].missingFlag = 0x01;
                SetDtcFaultState(pCheckConfigure[i].dtcItem);                              
            }                                      
        }
        else
        {
            pCheckBuffer[i].timeCount += cycleTime;
            if(pCheckBuffer[i].timeCount >= (pCheckConfigure[i].recoverTime * 2 / 5))
            {
            	/*if(pCheckConfigure[i].canId==0x1D2)
            	{
            		TBOX_PRINT("1D2 recoverCount clear %d,%d\r\n",pCheckBuffer[i].timeCount,RTOS_HalGetTickCount());
            	}*/
                pCheckBuffer[i].recoverCount = 0;
            }
            
            if(pCheckBuffer[i].recoverCount >= 5 )
            {
                if(pCheckBuffer[i].missingFlag)
                { 
                    pCheckBuffer[i].missingFlag = 0x00;
                    ClearDtcFaultState(pCheckConfigure[i].dtcItem);                 
                }      
            }
        }
    }
}

static void ClearNodeMissingDtcOnCan(const EcuNodeConfigure_t *pCheckConfigure,EcuNodeBuffer_t *pCheckBuffer,uint32_t configureSize)
{
	uint32_t i = 0;
    for(i = 0; i < configureSize; i++)
    {
        ClearDtcFaultState(pCheckConfigure[i].dtcItem);
    }
}

static void ClearAllNodeMissingDtc(void)
{
#if CAN_1_USED
    ClearNodeMissingDtcOnCan(g_can1EcuNodeConfigure,g_can1EcuNodeBuffer,SIZE_OF_ARRAY(g_can1EcuNodeConfigure));
#endif
#if CAN_2_USED
    ClearNodeMissingDtcOnCan(g_can2EcuNodeConfigure,g_can2EcuNodeBuffer,SIZE_OF_ARRAY(g_can2EcuNodeConfigure));
#endif
#if CAN_3_USED
    ClearNodeMissingDtcOnCan(g_can3EcuNodeConfigure,g_can3EcuNodeBuffer,SIZE_OF_ARRAY(g_can3EcuNodeConfigure));
#endif
#if CAN_4_USED
    ClearNodeMissingDtcOnCan(g_can4EcuNodeConfigure,g_can4EcuNodeBuffer,SIZE_OF_ARRAY(g_can4EcuNodeConfigure));
#endif
#if CAN_5_USED
    ClearNodeMissingDtcOnCan(g_can5EcuNodeConfigure,g_can5EcuNodeBuffer,SIZE_OF_ARRAY(g_can5EcuNodeConfigure));
#endif
}

void NodeMissingCheckTimeOut(uint32_t cycleTime)
{
    static uint32_t TimeCnt = 0;
    static uint8_t FirstEnter = 0;

    if(g_missingCheckRestFlag)
    {
        g_missingCheckRestFlag = 0;
        ClearNodeMissingCheckConfig();
    }
    if(PeripheralHalGetKl15Status() && (FirstEnter == 0))
    {
        TimeCnt += cycleTime;
        if(TimeCnt <= 1000)
        {
            ClearAllNodeMissingDtc();
            return;        
        }
        
        TimeCnt = 0;
        FirstEnter = 1;
        ClearNodeMissingCheckConfig();
    }

    if((!PeripheralHalGetKl15Status()))
    {
        if(FirstEnter!=0)
        {
            ClearAllNodeMissingDtc();
        }
        TimeCnt = 0;
        FirstEnter = 0;
        return;
    }
     
    if(!GetDtcProcessSetStatus())
    {
        return;
    }

#if CAN_1_USED
    NodeMissingCheckTimeOutOnCan(g_can1EcuNodeConfigure,g_can1EcuNodeBuffer,SIZE_OF_ARRAY(g_can1EcuNodeConfigure),cycleTime);
#endif
#if CAN_2_USED
     NodeMissingCheckTimeOutOnCan(g_can2EcuNodeConfigure,g_can2EcuNodeBuffer,SIZE_OF_ARRAY(g_can2EcuNodeConfigure),cycleTime);
#endif
#if CAN_3_USED
     NodeMissingCheckTimeOutOnCan(g_can3EcuNodeConfigure,g_can3EcuNodeBuffer,SIZE_OF_ARRAY(g_can3EcuNodeConfigure),cycleTime);
#endif
#if CAN_4_USED
     NodeMissingCheckTimeOutOnCan(g_can4EcuNodeConfigure,g_can4EcuNodeBuffer,SIZE_OF_ARRAY(g_can4EcuNodeConfigure),cycleTime);
#endif
#if CAN_5_USED
     NodeMissingCheckTimeOutOnCan(g_can5EcuNodeConfigure,g_can5EcuNodeBuffer,SIZE_OF_ARRAY(g_can5EcuNodeConfigure),cycleTime);
#endif

}

static void ClearNodeMissingCheckConfigOnCan(const EcuNodeConfigure_t *pCheckConfigure,EcuNodeBuffer_t *pCheckBuffer,uint32_t configureSize)
{
    uint32_t i = 0;
    for(i = 0; i < configureSize; i++)
    {
        pCheckBuffer[i].missingFlag = 1;
        pCheckBuffer[i].timeCount = 0;  
        pCheckBuffer[i].recoverCount = 5;  
    }
}

void ClearNodeMissingCheckConfig(void)
{
#if CAN_1_USED
    ClearNodeMissingCheckConfigOnCan(g_can1EcuNodeConfigure,g_can1EcuNodeBuffer,SIZE_OF_ARRAY(g_can1EcuNodeConfigure));
#endif
#if CAN_2_USED
    ClearNodeMissingCheckConfigOnCan(g_can2EcuNodeConfigure,g_can2EcuNodeBuffer,SIZE_OF_ARRAY(g_can2EcuNodeConfigure));
#endif
#if CAN_3_USED
    ClearNodeMissingCheckConfigOnCan(g_can3EcuNodeConfigure,g_can3EcuNodeBuffer,SIZE_OF_ARRAY(g_can3EcuNodeConfigure));
#endif
#if CAN_4_USED
    ClearNodeMissingCheckConfigOnCan(g_can4EcuNodeConfigure,g_can4EcuNodeBuffer,SIZE_OF_ARRAY(g_can4EcuNodeConfigure));
#endif
#if CAN_5_USED
    ClearNodeMissingCheckConfigOnCan(g_can5EcuNodeConfigure,g_can5EcuNodeBuffer,SIZE_OF_ARRAY(g_can5EcuNodeConfigure));
#endif
}

void NodeMissingCheckReset(void)
{
    g_missingCheckRestFlag= 1;
}


