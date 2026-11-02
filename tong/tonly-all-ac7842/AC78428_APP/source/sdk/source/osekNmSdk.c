#include "osekNmSdk.h"
#include "canHal.h"
#include "peripheralHal.h"
#include "powerManageHal.h"
#include "timerHal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "logHal.h"

/****************************************************
according to oseck.253 net management
    by xiewenming   2018.3


***************************************************/

#if(0)
#define OSEK_DEBUG_PRINT(var...)	TBOX_PRINT(var)
#else
#define OSEK_DEBUG_PRINT(var...)
#endif

#define OSEK_NET_MANAGE_MAX_INSTANCE            4
#define BUSOFF_CLEAR_CONFIRM_TIME_MS            100
#define TIME_MS_TO_TICKCOUNT(a)               (a/1)
#define NM_TRANS_CANFD_ENABLE                   1

/* OSEK NM日志总开关 */
#define OSEK_NM_LOG_ENABLE                    0

#if(OSEK_NM_LOG_ENABLE)
#define OSEK_NM_LOG(var...)                   TBOX_PRINT(var)
#else
#define OSEK_NM_LOG(var...)
#endif

#define OSEK_NET_MANAGE_DEBUG                 0
#define NET_MANAGE_STATE_RESET                0x02
#define NET_MANAGE_STATE_LIMHOME              0x03
#define NET_MANAGE_STATE_LIMHOME_PRESLEEP     0x04
#define NET_MANAGE_STATE_NORMAL               0x05
#define NET_MANAGE_STATE_NORMAL_PRESLEEP      0x06


#define NET_MANAGEE_OPCODE_BIT_MASK_ALIVE        (0x01<<0)
#define NET_MANAGEE_OPCODE_BIT_MASK_RING         (0x01<<1)
#define NET_MANAGEE_OPCODE_BIT_MASK_LIMPHOME     (0x01<<2)
#define NET_MANAGEE_OPCODE_BIT_MASK_SLEEP_IND    (0x01<<4)
#define NET_MANAGEE_OPCODE_BIT_MASK_SLEEP_ACK    (0x01<<5)



typedef enum
{
  netManageState_NMOff,
  netManageState_NMOn,
  netManageState_NMShutDown,
  //NMOn
  netManageState_NMInit,
  netManageState_NMAwake,
  netManageState_NMNormalPreSleep,//addtional
  netManageState_NMNormalInitBusSleep,
  netManageState_NMTwbsNormal,// wait Twait BusSleep  addtional
  //netManageState_NMBusSleep,
  netManageState_NMLimpHomePreSleep,//addtional
  netManageState_NMLimpHomeInitBusSleep,
  netManageState_NMTwbsLimpHome,//wait Twait BusSleep addtional
  netManageState_NMBusSleep,  
  netManageState_NMBusSleepWakeUp,//init for sleep wake up
  //NMAwkae
  netManageState_NMInitReset,//addtional
  netManageState_NMReset,
  netManageState_NMInitNormal,//addional
  netManageState_NMNormal,
  netManageState_NMInitLimpHome,//addtional
  netManageState_NMLimpHome,  
}enum_netManageState;

typedef enum
{
  messageType_limpHome,
  messageType_ring,
  messageType_alive,
  messageType_unknown,
}enum_messageType;

typedef struct
{
  uint8_t busSleep:1;
  uint8_t NMactive:1;
  uint8_t sleep_ind:1;
  uint8_t sleep_ack:1;
}struc_netWorkStatus;

typedef struct
{
  uint32_t canId;
  uint8_t data[8];
  uint8_t dlc;
}struc_canMsg;

typedef struct
{
  enum_netManageState netManageState;
  struc_netWorkStatus  netWorkStatus;
  int16_t canHandle;
  int16_t receiveCheckCanHandle;
  int16_t timerTtypeHandle;
  int16_t timerTmaxHandle;
  int16_t timerTerrorHandle;
  int16_t timerTtxHandle;
  int16_t timerTwbsHandle;
  int16_t busOffRecoverTimerHandle;
  int16_t busOffClearConfirmTimerHandle;
  CanHalMsg_t *canRxBuffer;
  uint8_t destinationId;
  uint8_t canInitFinished;
  uint8_t limpHomeACKFlag;
  uint8_t busOffTimerOnFlag;
  uint32_t NMrxcount;
  uint32_t NMtxcount;
  struc_canMsg canMsgTx;
  uint32_t busOffTimeCount;
  uint32_t checkTimeCount;
  uint8_t reWakeupFlag;
  uint8_t nmMsgReceiveFlag;
  uint8_t checkTimeoutFlag;
  uint8_t busOffEventOn;
  uint8_t networkRequested;
  uint8_t nmMsgReceived;
  uint8_t txEnabled;
  uint8_t busOffState;
  uint8_t busOffClearConfirmFlag;
}struc_osek_netManageVariable;

#ifdef _VARIABLE_DATA_IN_SYSRAM
/* Place following data in section SYS_RAM */
#pragma default_variable_attributes = @ "SYS_RAM"
#endif

static struc_osek_netManageVariable netManage[OSEK_NET_MANAGE_MAX_INSTANCE];//OSEK_NET_MANAGE_MAX_INSTANCE
CanHalMsg_t canRxBufferA[OSEK_NET_MANAGE_MAX_INSTANCE][4];

#ifdef _VARIABLE_DATA_IN_SYSRAM
/* Stop placing data in section SYS_RAM */
#pragma default_variable_attributes =
#endif

static const struc_netManageOsekConfigure* m_pNetManageConfigure = NULL;
static uint8_t m_netManageNumber = 0;
static uint32_t m_debugPrintTime = 0;
/*************************************************************************
index
  index of net manage
flag:
  0:limphome out
  1:limphome in
**************************************************************************/
static void NetManageLimpHomeState(uint8_t index,uint8_t flag)
{
  static uint8_t limphomeEventOn[6];
  if((flag==0x00)&&(limphomeEventOn[index]==0x00))
  {
    return;
  }
  if((flag)&&(limphomeEventOn[index]))
  {
    return;
  }
  limphomeEventOn[index] = flag;
  if(m_pNetManageConfigure[index].limpHomeEventCallBack!=NULL)
  {
    m_pNetManageConfigure[index].limpHomeEventCallBack(flag);
  }
}
static void NetManageBusOffErrorCallBack(uint8_t index,uint8_t flag)
{
  if(m_pNetManageConfigure[index].canBusErrorCallBack!=NULL)
  {
    m_pNetManageConfigure[index].canBusErrorCallBack(flag);
  }  
}

static uint8_t NetManageGetSuccessor(uint8_t source,uint8_t receiver,uint8_t oldSuccessor)
{
    uint8_t newSuccessor;
    newSuccessor = oldSuccessor;
    if(oldSuccessor==receiver)
    {
        newSuccessor = source;
    }
    else
    {
        if(oldSuccessor<receiver)
        {
            if(source<oldSuccessor)
            {
                newSuccessor = source;
            }
            else
            {
                if(source<receiver)
                {
                    //do nothing
                }
                else
                {
                    newSuccessor = source;
                }
            }
        }
        else
        {
            if(source<oldSuccessor)
            {
                if(source<receiver)
                {
                    //do nothing
                }
                else
                {
                    newSuccessor = source;
                }
            }
            else
            {
                //do nothing
            }			
                
        }
    }
    return newSuccessor;
}

static int16_t NetManageCheckSkiped(uint8_t source,uint8_t destination,uint8_t receiver)
{
    int16_t retValue;
    retValue = 0;
    if(destination<receiver)
    {
        if(source<destination)
        {
            //not skiped                    
        }
        else
        {
            if(source<receiver)
            {
                //skiped
                retValue = 1;
            }
            else
            {
                //not skiped
            }
        }
    }
    else
    {
        if(source<destination)
        {
            if(source<receiver)
            {
                //skiped
                retValue = 1; 
            }
            else
            {
                //not skiped
            }
        }
        else
        {
            //skiped
            retValue = 1; 
        }
    }
    return retValue;
}
/***********************************************
need to change according to different protocal
return value:
  messageType_limpHome,
  messageType_ring,
  messageType_alive,
  messageType_unknown
****************************************************/
static enum_messageType GetMessageTypeFromCanData(uint8_t *CanData)
{
  enum_messageType messageType;
  uint8_t opCode;
  opCode = CanData[1];
  if(opCode&NET_MANAGEE_OPCODE_BIT_MASK_ALIVE)
  {
     messageType = messageType_alive;  
  }
  else if(opCode&NET_MANAGEE_OPCODE_BIT_MASK_RING)
  {
     messageType = messageType_ring;  
  }
  else if(opCode&NET_MANAGEE_OPCODE_BIT_MASK_LIMPHOME)
  {
     messageType = messageType_limpHome;  
  }
  else
  {
    messageType = messageType_unknown;    
  }
  return messageType;
}

static void CancelAlarmTmax(uint8_t index)
{
    TimerHalStopTime(netManage[index].timerTmaxHandle);
}

static void CancelAlarmTtype(uint8_t index)
{
    TimerHalStopTime(netManage[index].timerTtypeHandle);
}

static void CancelAlarmTtx(uint8_t index)
{
    TimerHalStopTime(netManage[index].timerTtxHandle);
}

static void CancelAlarmTerror(uint8_t index)
{
    TimerHalStopTime(netManage[index].timerTerrorHandle);
}

static void SetAlarmTmax(uint8_t index)
{
    TimerHalStartTime(netManage[index].timerTmaxHandle,m_pNetManageConfigure[index].netManageTmax);
}

static void SetAlarmTtype(uint8_t index)
{
    TimerHalStartTime(netManage[index].timerTtypeHandle,m_pNetManageConfigure[index].netManageTtype);
}

static void SetAlarmTtx(uint8_t index)
{
    TimerHalStartTime(netManage[index].timerTtxHandle,m_pNetManageConfigure[index].netManageTtx);
}

static void SetAlarmTerror(uint8_t index)
{
    TimerHalStartTime(netManage[index].timerTerrorHandle,m_pNetManageConfigure[index].netManageTerror);
  
}

static void SetAlarmTwbs(uint8_t index)
{
    TimerHalStartTime(netManage[index].timerTwbsHandle,m_pNetManageConfigure[index].netManageTwaitBusSleep);
}

static void CancelAlarmTwbs(uint8_t index)
{
  TimerHalStopTime(netManage[index].timerTwbsHandle);
}

/**********************************************

return
0: BusOffError
1:no BusOffError
************************************************/
/*static int16_t NetManageIsBusOffError(uint8_t index)
{
  return CanHalGetIsBusOffError(netManage[index].canHandle);
}*/

static void NetManageAPPBusDisable(uint8_t index)
{
  CanHalAppMsgDisable(netManage[index].canHandle);
}

static void NetManageAppBusEnable(uint8_t index)
{
  CanHalAppMsgEnable(netManage[index].canHandle);
}

static void NetManageInializeHardware(uint8_t index)
{
  CanHalResetHardware(netManage[index].canHandle);
  vTaskDelay(5);
  CanHalResetHardware(netManage[index].canHandle);
}

static void CanTransmitSuccessful(uint8_t index)
{
  enum_messageType messageType;
  uint8_t opCode;
  //busSleep = 0;//for debug
  //messageType = messageType_ring;//for debug
  messageType = GetMessageTypeFromCanData(netManage[index].canMsgTx.data);
    
  opCode = netManage[index].canMsgTx.data[1];
  if(opCode&NET_MANAGEE_OPCODE_BIT_MASK_SLEEP_IND)
  {
    netManage[index].netWorkStatus.sleep_ind =  1;  
  }
  else
  {
      netManage[index].netWorkStatus.sleep_ind = 0;   
  }

  if(netManageState_NMNormal==netManage[index].netManageState)
  {
    netManage[index].NMtxcount = 0;
    if(messageType==messageType_ring)
    {
      //CancelAlarm(TTyp)
      CancelAlarmTtype(index);
      //CancelAlarm(Tmax)
      CancelAlarmTmax(index);
      
      /* 单节点判断: 不需要等待Tmax，直接重启Ttype周期发送Ring*/
      if(netManage[index].destinationId == m_pNetManageConfigure[index].netManageNodeId)
      {
        /* 单节点场景: 直接重启Ttype定时器，按Ttype周期发送Ring */
        SetAlarmTtype(index);
      }
      else
      {
        /* 多节点场景: 启动Tmax等待其他节点响应 */
        SetAlarmTmax(index);
        // OSEK_NM_LOG("[OSEK_NM] CanTransmitSuccessful! dest=%d\r\n", netManage[index].destinationId);
        
      }
      
      if(netManage[index].netWorkStatus.sleep_ind) 
      {
        if(netManage[index].netWorkStatus.busSleep)
        {
          //if(TimerHalIsTimeout(netManage[index].timerRequestSleepMinHandle)==0)
          {
            //sleep.ack = 1
             netManage[index].netWorkStatus.sleep_ack = 1;
            //NMNormalPreSleep
            netManage[index].netManageState = netManageState_NMNormalPreSleep;
            SetAlarmTtype(index);
          }
        }
        else
        {
          //NMNormal
        }
      }
      else
      {
        //NMNormal
      }
    }
    else
    {
      //NMNormal
    }
  }
  else if(netManageState_NMNormalPreSleep==netManage[index].netManageState)
  {
    //CancleAlarm(Tmax)
    CancelAlarmTmax(index);
    //NMInitBusSleep
    netManage[index].netManageState = netManageState_NMNormalInitBusSleep;    
  }
  else if(netManageState_NMLimpHome==netManage[index].netManageState)
  {
    if(messageType_limpHome==messageType)
    {
      //NMMerker.limphome :=1
    }
    //NMlimpHome
  }

}

static void CanReceiveMessageProcess(uint8_t index,uint32_t canId,uint8_t *canData)
{
    enum_messageType messageType;
    uint8_t destinationRx;
    uint8_t sourceRx;
    uint8_t NMMerker_limphome;
    uint8_t sleep_ack;
    uint8_t sleep_ind;
    
    NMMerker_limphome = 1;//if transmit sucess set this flag
   //clear NMrxcount
    netManage[index].NMrxcount = 0;
	  netManage[index].nmMsgReceiveFlag = 1;
    //messageType = destinationRx=  sourceRx=
    sourceRx = canId- m_pNetManageConfigure[index].netManageBaseAddress;//sourceRx
    destinationRx = canData[0];
    messageType = GetMessageTypeFromCanData(canData);
    
    // OSEK_NM_LOG("[OSEK_NM] RX msg, index=%d, src=%d, dest=%d, type=%d, opCode=0x%02x\r\n",
    //            index, sourceRx, destinationRx, messageType, canData[1]);
    if(canData[1]&NET_MANAGEE_OPCODE_BIT_MASK_SLEEP_IND)
    {
      //netWorkStatus.sleep_ind = 1;
      sleep_ind = 1;
    }
    else
    {
       //netWorkStatus.sleep_ind = 0;
        sleep_ind = 0;
    }
    
    if(canData[1]&NET_MANAGEE_OPCODE_BIT_MASK_SLEEP_ACK)
    {
      //netWorkStatus.sleep_ack = 1;
      sleep_ack = 1;
    }
    else
    {
       //netWorkStatus.sleep_ack = 0;
        sleep_ack = 0;
    }    
    
  if(netManageState_NMNormal==netManage[index].netManageState)
  {    
    if((messageType_alive==messageType || messageType_ring==messageType)
       && (netManage[index].netWorkStatus.NMactive == 0))
    {
      OSEK_NM_LOG("[OSEK_NM] Auto-recover NMactive in NMNormal, index=%d\r\n", index);
      netManage[index].netWorkStatus.NMactive = 1;
    }
    //
      if(messageType_limpHome==messageType)
      {
        //determine limphome configuration
        //add sender to config.limphome

        
      }
      else
      {
        //determine actual configuration
        //add sender to config.present

        //determine logical successor (only for valid message types: alive or ring)
        if(messageType_alive==messageType || messageType_ring==messageType)
        {
          uint8_t oldDestinationId = netManage[index].destinationId;
          netManage[index].destinationId = NetManageGetSuccessor(sourceRx,m_pNetManageConfigure[index].netManageNodeId,netManage[index].destinationId);//NetManageGetSuccessor(uint8_t source,uint8_t receiver,uint8_t oldSuccessor)
          OSEK_NM_LOG("[OSEK_NM] RX ring/alive update successor, index=%d, src=%d, receiver=%d, oldDest=%d, newDest=%d\n",
                     index, sourceRx, m_pNetManageConfigure[index].netManageNodeId, oldDestinationId, netManage[index].destinationId);
          if(oldDestinationId != netManage[index].destinationId)
          {
            OSEK_NM_LOG("[OSEK_NM] destinationId changed: %d -> %d !\n", oldDestinationId, netManage[index].destinationId);
          }
          if(messageType_ring==messageType)
          {
            OSEK_NM_LOG("[OSEK_NM] RX ring msg! src=%d, dest=%d, type=%d, opCode=0x%02x, myNode=%d, myDest=%d\r\n",
               sourceRx, destinationRx, messageType, canData[1], m_pNetManageConfigure[index].netManageNodeId, netManage[index].destinationId);
            if((destinationRx==m_pNetManageConfigure[index].netManageNodeId)||(destinationRx==sourceRx))//by xwm 2018.11.28
            {
              //-CancleAlarm(TTyp)
              CancelAlarmTtype(index);
              //-CancleAlarn(Tmax)
              CancelAlarmTmax(index);
              //SetAlarm(TTyp)
              SetAlarmTtype(index);
            }
            else//to another node
            {
              if(NetManageCheckSkiped(sourceRx,destinationRx,m_pNetManageConfigure[index].netManageNodeId))//skiped
              {
                uint8_t opCode;
                uint16_t ret;
                if(netManage[index].netWorkStatus.NMactive)//NMactive
                {
                    //own node skipped in logical ring
                    //transmit alive message
                    //canMsgTx.canId = netManageBaseAddress+netManageNodeId;
                    opCode = 0;
                    opCode |= NET_MANAGEE_OPCODE_BIT_MASK_ALIVE;
                    //add sleep status later
                    if(netManage[index].netWorkStatus.busSleep)
                    {
                      opCode |= NET_MANAGEE_OPCODE_BIT_MASK_SLEEP_IND;
                    }
                    netManage[index].destinationId = m_pNetManageConfigure[index].netManageNodeId;
                    netManage[index].canMsgTx.data[0] = netManage[index].destinationId;
                    netManage[index].canMsgTx.data[1] = opCode;
  #if OSEK_NET_MANAGE_DEBUG
                    netManage[index].canMsgTx.data[3] = NET_MANAGE_STATE_NORMAL;
  #endif
                    ret = CanHalNmTransmit(netManage[index].canHandle,netManage[index].canMsgTx.canId,netManage[index].canMsgTx.data,netManage[index].canMsgTx.dlc,NM_TRANS_CANFD_ENABLE);
  #if OSEK_NET_MANAGE_DEBUG
                    netManage[index].canMsgTx.data[3] = 0x00;
  #endif
                    if(ret==0)
                    {
                      //transmit successfull
                      CanTransmitSuccessful(index);
                    }
                    else if(ret==1)//buffer full need to modifyed later
                    {
                      
                      //start Ttx timer
                      SetAlarmTtx(index);
                    }
                    else
                    {
                      //transmit failed
                      netManage[index].NMtxcount++;
                    }
                      
                }

              }
              else//not skipped
              {
                //do nothing
              }
          }
          }
          else//alive message
          {
            OSEK_NM_LOG("[OSEK_NM] RX alive msg! src=%d, dest=%d, type=%d, opCode=0x%02x, myNode=%d, myDest=%d\r\n",
               sourceRx, destinationRx, messageType, canData[1], m_pNetManageConfigure[index].netManageNodeId, netManage[index].destinationId);
            //do nothing
          }
        }
        
        
      }
      
    //
      if(sleep_ack)
      {
        if(netManage[index].netWorkStatus.busSleep)
        {
         //CancelAlarm(Tmax)
         CancelAlarmTmax(index);
          //NMInitBusSleep
          OSEK_NM_LOG("[OSEK_NM] RX in NMNormal, sleep_ack -> NMNormalInitBusSleep, index=%d\r\n", index);
          netManage[index].netManageState = netManageState_NMNormalInitBusSleep;
        }
        else
        {
          //NMNormal
        }
      }
      else
      {
        //NMNormal
        
      }
  }
  else if(netManageState_NMLimpHome==netManage[index].netManageState)
  {
    do
    {
      if(netManage[index].netWorkStatus.NMactive)//NMactive==1
      {
        if(!NMMerker_limphome)//NMerker.limphome==0
        {
          break;
        }
      }
      if(PeripheralHalGetKl15Status())//!netManage[index].netWorkStatus.busSleep
      {
        //NMInitReset--5
        OSEK_NM_LOG("[OSEK_NM] RX in LimpHome, KL15 on -> NMInitReset, index=%d\r\n", index);
        netManage[index].netManageState = netManageState_NMInitReset;
        NetManageLimpHomeState(index,0);
        return;
      }
    }while(0);

    if(sleep_ack)
    {
      //disable application communication(D_Offline)
      NetManageAPPBusDisable(index);
      //SetAlarm(Twbs)
      SetAlarmTwbs(index);
      //NMTwbsLimpHome
      OSEK_NM_LOG("[OSEK_NM] RX in LimpHome, sleep_ack -> NMTwbsLimpHome, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMTwbsLimpHome;
      netManage[index].limpHomeACKFlag = 1;
    }
    else//sleep_ack==0
    {
        OSEK_NM_LOG("[OSEK_NM] RX in LimpHome, no sleep_ack -> NMInitReset, index=%d\r\n", index);
        netManage[index].netManageState = netManageState_NMInitReset;
        NetManageLimpHomeState(index,0);
       //NNInitReset--5
    }
  }
  else if(netManageState_NMLimpHomePreSleep==netManage[index].netManageState)
  {
    if(sleep_ack)
    {
      CancelAlarmTmax(index);
      NetManageAPPBusDisable(index);
      SetAlarmTwbs(index);
      //NMTwbsLimpHome
      OSEK_NM_LOG("[OSEK_NM] RX in LimpHomePreSleep, sleep_ack -> NMTwbsLimpHome, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMTwbsLimpHome;
      netManage[index].limpHomeACKFlag = 1;
    }
    else if(!sleep_ind)
    {
      //to do
      OSEK_NM_LOG("[OSEK_NM] RX in LimpHomePreSleep, no sleep_ind -> NMLimpHome, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMLimpHome;
    }
		else if(sleep_ind)
		{
			uint8_t opCode;

			opCode = 0x00;//canMsgTx.data[1]
			opCode |= NET_MANAGEE_OPCODE_BIT_MASK_SLEEP_IND;
			opCode |= NET_MANAGEE_OPCODE_BIT_MASK_LIMPHOME;
			netManage[index].canMsgTx.data[0] = netManage[index].destinationId;
			netManage[index].canMsgTx.data[1] = opCode;
			CanHalNmTransmit(netManage[index].canHandle,netManage[index].canMsgTx.canId,netManage[index].canMsgTx.data,netManage[index].canMsgTx.dlc,NM_TRANS_CANFD_ENABLE);
		}			
  }
  else if(netManageState_NMTwbsLimpHome==netManage[index].netManageState)
  {
    if(!sleep_ind)
    {
      //to do
      CancelAlarmTwbs(index);
      SetAlarmTerror(index);
      netManage[index].reWakeupFlag = 0x01;//19.1.31
      NetManageAppBusEnable(index);//19.1.31
      OSEK_NM_LOG("[OSEK_NM] RX in TwbsLimpHome, no sleep_ind -> NMLimpHome, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMLimpHome;
    }
  }
  else if(netManageState_NMNormalPreSleep==netManage[index].netManageState)
  {
    if((messageType_alive==messageType || messageType_ring==messageType)
       && (netManage[index].netWorkStatus.NMactive == 0))
    {
      OSEK_NM_LOG("[OSEK_NM] Auto-recover NMactive in NMNormalPreSleep, index=%d\r\n", index);
      netManage[index].netWorkStatus.NMactive = 1;
    }
      if(messageType_limpHome==messageType)
      {
        //determine limphome configuration
        //add sender to config.limphome        
      }
      else
      {
        //determine logical successor (only for valid message types: alive or ring)
        if(messageType_alive==messageType || messageType_ring==messageType)
        {
          netManage[index].destinationId = NetManageGetSuccessor(sourceRx,m_pNetManageConfigure[index].netManageNodeId,netManage[index].destinationId);//NetManageGetSuccessor(uint8_t source,uint8_t receiver,uint8_t oldSuccessor)
          if(messageType_ring==messageType)
          {
            if((destinationRx==m_pNetManageConfigure[index].netManageNodeId)||(destinationRx==sourceRx))//to me
            {
              //-CancleAlarm(TTyp)
              CancelAlarmTtype(index);
              //-CancleAlarn(Tmax)
              CancelAlarmTmax(index);
              //SetAlarm(TTyp)
              SetAlarmTtype(index);
              }
            else//to another node
            {
              if(NetManageCheckSkiped(sourceRx,destinationRx,m_pNetManageConfigure[index].netManageNodeId))//skiped
              {
                uint8_t opCode;
                uint16_t ret;
                if(netManage[index].netWorkStatus.NMactive)//NMactive
                {
                    //own node skipped in logical ring
                    //transmit alive message
                    //canMsgTx.canId = netManageBaseAddress+netManageNodeId;
                    opCode = 0;
                    opCode |= NET_MANAGEE_OPCODE_BIT_MASK_ALIVE;
                    //add sleep status later
                    if(netManage[index].netWorkStatus.busSleep)
                    {
                      opCode |= NET_MANAGEE_OPCODE_BIT_MASK_SLEEP_IND;
                    }   
                    netManage[index].destinationId = m_pNetManageConfigure[index].netManageNodeId;
                    netManage[index].canMsgTx.data[0] = netManage[index].destinationId;
                    netManage[index].canMsgTx.data[1] = opCode;
  #if OSEK_NET_MANAGE_DEBUG
                    netManage[index].canMsgTx.data[3] = NET_MANAGE_STATE_NORMAL_PRESLEEP;
  #endif
                    ret = CanHalNmTransmit(netManage[index].canHandle,netManage[index].canMsgTx.canId,netManage[index].canMsgTx.data,8,NM_TRANS_CANFD_ENABLE); 
  #if OSEK_NET_MANAGE_DEBUG
                    netManage[index].canMsgTx.data[3] = 0x00;
  #endif
                    if(ret==0)
                    {
                      //transmit successfull
                      CanTransmitSuccessful(index);
                    }
                    else if(ret==1)//buffer full need to modifyed later
                    {
                      
                      //start Ttx timer
                      SetAlarmTtx(index);                   
                    }
                    else
                    {
                      //transmit failed
                      netManage[index].NMtxcount++;
                    }
                      
                }

              }
              else//not skipped
              {
                //do nothing
              }
            }
          }
          else//alive message
          {
            //do nothing
          }          
        }

      }
    if(sleep_ack)
    {
      if(netManage[index].netWorkStatus.busSleep)
      {
         //CancelAlarm(Tmax)
         CancelAlarmTmax(index);
        //NMInitBusSleep
        netManage[index].netManageState = netManageState_NMNormalInitBusSleep;
      }
      else
      {
        //NMNormal
      }
    }  
    else if(sleep_ind)
    {
      //NMNormalPreSleep
      
    }
    else//sleep_ind==0
    {
      //NMNormal
      netManage[index].netManageState = netManageState_NMNormal;
      //TimerHalStartTime(netManage[index].timerRequestSleepMinHandle,netManageParameter[index]->netManageTrequestSleepMin);
    }
  }
  else if(netManageState_NMTwbsNormal==netManage[index].netManageState)
  {
    if(sleep_ind)
    {
      //NMTwbsNormal
         
    }
    else//sleep_ind==0
    {
      //CancelAlarm(Twbs)
      CancelAlarmTwbs(index);
      //NMInitReset
      OSEK_NM_LOG("[OSEK_NM] RX in TwbsNormal, no sleep_ind -> NMInitReset, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMInitReset;
    }
  }
  else if(netManageState_NMBusSleep==netManage[index].netManageState)
  {
    OSEK_NM_LOG("[OSEK_NM] RX in BusSleep, wakeup detected, index=%d\r\n", index);
    netManage[index].limpHomeACKFlag = 0;
    netManage[index].netWorkStatus.busSleep = 0;
    netManage[index].reWakeupFlag = 1;
  }
}

static void TtypeTimeOutProcess(uint8_t index)
{
    uint16_t ret;
    uint8_t opCode;
    
  OSEK_NM_LOG("[OSEK_NM] Ttype timeout, index=%d, state=%d, dest=%d, nodeId=%d\r\n", index, netManage[index].netManageState, netManage[index].destinationId, m_pNetManageConfigure[index].netManageNodeId);
  if(netManageState_NMNormal==netManage[index].netManageState)
  {
 //CancelAlarm(Tmax)
      //SetAlarm(Tmax)
      SetAlarmTmax(index);
    if(netManage[index].netWorkStatus.NMactive)//NMactive true
    {
	    opCode = 0x00;//canMsgTx.data[1]
	    opCode |= NET_MANAGEE_OPCODE_BIT_MASK_RING;
      
      if(netManage[index].netWorkStatus.busSleep)//bus sleep true
      {
        //if(TimerHalIsTimeout(netManage[index].timerRequestSleepMinHandle)==0)
        {
          //Send ring message with sleep.ind bit
          opCode |= NET_MANAGEE_OPCODE_BIT_MASK_SLEEP_IND;
        }
 
      }
      else//bus sleep false
      {
        //Send ring message
      }
      
      netManage[index].canMsgTx.data[0] = netManage[index].destinationId;
      netManage[index].canMsgTx.data[1] = opCode;
      OSEK_NM_LOG("[OSEK_NM] Ttype TX ring msg, index=%d, dest=%d, opCode=0x%02x\r\n", index,netManage[index].destinationId, opCode);
#if(OSEK_NET_MANAGE_DEBUG)
      netManage[index].canMsgTx.data[2]++;
      netManage[index].canMsgTx.data[3] = NET_MANAGE_STATE_NORMAL;//for debug
#endif
      ret = CanHalNmTransmit(netManage[index].canHandle,netManage[index].canMsgTx.canId,netManage[index].canMsgTx.data,netManage[index].canMsgTx.dlc,NM_TRANS_CANFD_ENABLE);
#if(OSEK_NET_MANAGE_DEBUG)
      netManage[index].canMsgTx.data[3] = 0x00;//for debug
#endif
      if(ret==0)
      {
        //transmit successful
        CanTransmitSuccessful(index);
      }
      else if(ret==1)
      {
            //tx buffer full
        SetAlarmTtx(index);
      }
      else
      {
            //transmit error
        netManage[index].NMtxcount++;
      }

    }
    if(netManage[index].NMtxcount>m_pNetManageConfigure[index].tx_limit)
    {
      //Enter state NMLimpHome
      CancelAlarmTmax(index);
      SetAlarmTerror(index);
      OSEK_NM_LOG("[OSEK_NM] Ttype tx_limit exceeded -> NMLimpHome, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMLimpHome;
      NetManageLimpHomeState(index,1);
      //TimerHalStartTime(netManage[index].timerRequestSleepMinHandle,m_pNetManageConfigure[index].netManageTrequestSleepMin);
    }
    else
    {
     /* if(NMMerker_stable)//NMMerker.stable = 1
      {
        //net workstatus.configuationstable := 1

      }
      else
      {
        //NMMerker.stable :=1
        
      }*/
      //NMormal
    }
  }
  else if(netManageState_NMNormalPreSleep==netManage[index].netManageState)
  {
    //CancelAlarm(Tmax)
    //SetAlarm(Tmax)
 	  SetAlarmTmax(index);
    if(netManage[index].netWorkStatus.NMactive)//NMacitve true
    {
      //send ring message with set sleep.ack bit
      opCode = 0x00;
      opCode |= NET_MANAGEE_OPCODE_BIT_MASK_RING;
      opCode |= NET_MANAGEE_OPCODE_BIT_MASK_SLEEP_IND;
      opCode |= NET_MANAGEE_OPCODE_BIT_MASK_SLEEP_ACK;
      netManage[index].canMsgTx.data[0] = netManage[index].destinationId;
      netManage[index].canMsgTx.data[1] = opCode;	
      
#if(NET_MANAGE_DEUBG)      
	  netManage[index].canMsgTx.data[3] = NET_MANAGE_STATE_NORMAL_PRESLEEP;//for debug
#endif
	  ret = CanHalNmTransmit(netManage[index].canHandle,netManage[index].canMsgTx.canId,netManage[index].canMsgTx.data,netManage[index].canMsgTx.dlc,NM_TRANS_CANFD_ENABLE);
#if(NET_MANAGE_DEUBG)  
	  netManage[index].canMsgTx.data[3] = 0x00;//for debug
#endif
	  if(ret==0)
	  {
	    //transmit successful
	    CanTransmitSuccessful(index);
	  }
	  else if(ret==1)
	  {
		//tx buffer full
		SetAlarmTtx(index);
	  }
	  else 
	  {
		//transmit error
		netManage[index].NMtxcount++;	  
	  }
	  
    }
  }

}

static void TmaxTimeOutProcess(uint8_t index)
{
  if(netManageState_NMNormal==netManage[index].netManageState)
  {
    if(netManage[index].destinationId == m_pNetManageConfigure[index].netManageNodeId)
    {      
      CancelAlarmTmax(index);
      SetAlarmTmax(index);
      SetAlarmTtype(index);
      OSEK_NM_LOG("[OSEK_NM] Tmax self-destination, restart Ttype, index=%d\r\n", index);
    }
    else{
      //CancelAlarm
      CancelAlarmTtype(index);
      CancelAlarmTmax(index);
      CancelAlarmTerror(index);
      CancelAlarmTtx(index);
      CancelAlarmTwbs(index);
      //NMReset
      OSEK_NM_LOG("[OSEK_NM] Tmax timeout -> NMReset, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMReset;
    }
  }
  else if(netManageState_NMNormalPreSleep==netManage[index].netManageState)
  {
    
    if(netManage[index].destinationId == m_pNetManageConfigure[index].netManageNodeId)
    {
      /* ???????: ??????InitBusSleep???Twbs??????? */
      CancelAlarmTmax(index);
      netManage[index].netManageState = netManageState_NMNormalInitBusSleep;
    }
    else
    {
      /* ????: ???Reset???????????? */
      CancelAlarmTtype(index);
      CancelAlarmTmax(index);
      CancelAlarmTerror(index);
      CancelAlarmTtx(index);
      CancelAlarmTwbs(index);
      netManage[index].netManageState = netManageState_NMReset;
    }
  }
  else if(netManageState_NMLimpHomePreSleep==netManage[index].netManageState)
  {
    //CancelAlarm(Tmax)
    CancelAlarmTmax(index);
    //NMLimpHomeInitBusSleep
    netManage[index].netManageState = netManageState_NMLimpHomeInitBusSleep;
  }
}

static void TerrorTimeOutProcess(uint8_t index)
{
  int16_t ret;
  uint8_t opCode;

  if(netManageState_NMLimpHome==netManage[index].netManageState)
  {
    //enable application communication(D_Online)
    
    if(netManage[index].netWorkStatus.busSleep)
    {
      //if(TimerHalIsTimeout(netManage[index].timerRequestSleepMinHandle)==0)
      {
          //SetAlarm(Tmax)
          SetAlarmTmax(index);
          if(netManage[index].netWorkStatus.NMactive)//NMactive
          {
            //send limphome message with set bit sleep_ind
            opCode = 0x00;//canMsgTx.data[1]
            opCode |= NET_MANAGEE_OPCODE_BIT_MASK_SLEEP_IND;
            opCode |= NET_MANAGEE_OPCODE_BIT_MASK_LIMPHOME;
            netManage[index].canMsgTx.data[0] = netManage[index].destinationId;
            netManage[index].canMsgTx.data[1] = opCode;
            OSEK_NM_LOG("[OSEK_NM] Terror TX limphome msg with sleep_ind, index=%d, dest=%d\r\n", index, 	netManage[index].destinationId);
            ret = CanHalNmTransmit(netManage[index].canHandle,netManage[index].canMsgTx.canId,netManage[index].canMsgTx.data,netManage[index].canMsgTx.dlc,NM_TRANS_CANFD_ENABLE);
            if(ret==0)
            {
              //transmit successful
              CanTransmitSuccessful(index);
            }
            else if(ret==1)//buffer
            {
              SetAlarmTtx(index);
            }
            else 
            {
              //transmit error
              netManage[index].NMtxcount++;
            }

          }
          //NMPreSleep
          netManage[index].netManageState = netManageState_NMLimpHomePreSleep;       
        
      }
      /*else//none sleep to process
      {
          //SetAlarm(TError)
          SetAlarmTerror(index);
          if(netManage[index].netWorkStatus.NMactive)//NMactive
          {
            //send limphome message
            opCode = 0x00;//canMsgTx.data[1]		
            opCode |= NET_MANAGEE_OPCODE_BIT_MASK_LIMPHOME;
            netManage[index].canMsgTx.data[0] = netManage[index].destinationId;
            netManage[index].canMsgTx.data[1] = opCode;
            ret = CanHalNmTransmit(netManage[index].canHandle,netManage[index].canMsgTx.canId,netManage[index].canMsgTx.data,netManage[index].canMsgTx.dlc,NM_TRANS_CANFD_ENABLE);
            if(ret==0)
            {
              //transmit successful
              CanTransmitSuccessful(index);
            }
            else if(ret==1)
            {
              
            }
            else 
            {
              //transmit error
            }
          }
          //NMlimpHome        
      }*/
      
    }
    else//bus sleep false
    {
      //SetAlarm(TError)
      SetAlarmTerror(index);
      if(netManage[index].netWorkStatus.NMactive)//NMactive
      {
        //send limphome message
        opCode = 0x00;//canMsgTx.data[1]
        opCode |= NET_MANAGEE_OPCODE_BIT_MASK_LIMPHOME;
        netManage[index].canMsgTx.data[0] = netManage[index].destinationId;
        netManage[index].canMsgTx.data[1] = opCode;
        OSEK_NM_LOG("[OSEK_NM] Terror TX limphome msg, index=%d, dest=%d\r\n", index, netManage[index].destinationId);
        ret = CanHalNmTransmit(netManage[index].canHandle,netManage[index].canMsgTx.canId,netManage[index].canMsgTx.data,netManage[index].canMsgTx.dlc,NM_TRANS_CANFD_ENABLE);
        if(ret==0)
        {
          //transmit successful
          CanTransmitSuccessful(index);
        }
        else if(ret==1)
        {
          
        }
        else 
        {
          //transmit error
        }
      }
      //NMlimpHome
    }
  }
}

static void TtxTimeOutProcess(uint8_t index)
{
  int16_t ret;
  //disable Ttx
  CancelAlarmTtx(index);
  ret = CanHalNmTransmit(netManage[index].canHandle,netManage[index].canMsgTx.canId,netManage[index].canMsgTx.data,netManage[index].canMsgTx.dlc,NM_TRANS_CANFD_ENABLE);
  if(ret==0)
  {
    //transmit successful
    CanTransmitSuccessful(index);
  }
  else if(ret==1)//tx buffer full
  {
    //Ttx start
    SetAlarmTtx(index);   
  }
  else
  {
    //transmit error
  }
}

static void TwbsTimeOutProcess(uint8_t index)
{
  OSEK_NM_LOG("[OSEK_NM] Twbs timeout, index=%d, state=%d\r\n", index, netManage[index].netManageState);
  if(netManageState_NMTwbsNormal==netManage[index].netManageState)
  {
    //CancelALarm(Twbs)
    CancelAlarmTwbs(index);
    //initialize the sleep mode of the bus hardware D_init(...;BusSleep)

    //NMBusSleep
    OSEK_NM_LOG("[OSEK_NM] Twbs timeout -> NMBusSleep, index=%d\r\n", index);
    netManage[index].netManageState = netManageState_NMBusSleep;
    
  }
  else if(netManageState_NMTwbsLimpHome==netManage[index].netManageState)
  {
    //initialize the sleep mode of the bus hardware D_init(...;BusSleep)

    //NMBusSleep
    OSEK_NM_LOG("[OSEK_NM] Twbs LimpHome timeout -> NMBusSleep, index=%d\r\n", index);
    netManage[index].netManageState = netManageState_NMBusSleep;
  }
}

static void CanHardwareControllerProcess(uint8_t index)
{
  int16_t ret;  
  ret = CanHalGetIsBusOffError(netManage[index].canHandle);
  if(ret==0)//bus off
  {
    // Cancel busoff clear confirm timer since bus is off again
    TimerHalStopTime(netManage[index].busOffClearConfirmTimerHandle);
    netManage[index].busOffClearConfirmFlag = 0;

    //CanHalResetHardware(netManage[index].canHandle);
    //busOffTimeCount;
    if(netManage[index].busOffTimerOnFlag)
    {
      //do nothing
    }
    else
    {
      if(m_pNetManageConfigure[index].busOffQuickTimes!=0)
      {
        if(netManage[index].busOffTimeCount>=m_pNetManageConfigure[index].busOffQuickTimes)//
        {
          if(m_pNetManageConfigure[index].busOffSlowTime!=0)
          {
            //start busoff recover slow timer
            TimerHalStartTime(netManage[index].busOffRecoverTimerHandle,m_pNetManageConfigure[index].busOffSlowTime);
            netManage[index].busOffTimerOnFlag = 1;            
          }
          else
          {
            NetManageInializeHardware(index);
            //vTaskDelay(5);
          }

        }
        else
        {
          if(m_pNetManageConfigure[index].busOffQuickTime!=0)
          {
            // start busoff recover quick timer
            TimerHalStartTime(netManage[index].busOffRecoverTimerHandle,m_pNetManageConfigure[index].busOffQuickTime);
            netManage[index].busOffTimerOnFlag = 1; 
            netManage[index].busOffTimeCount++;             
          }
          else
          {
            NetManageInializeHardware(index);
            //vTaskDelay(5);
          }
        }           
      }
      else//no quick time state
      {
          if(m_pNetManageConfigure[index].busOffSlowTime!=0)
          {
            //start busoff recover slow timer
            TimerHalStartTime(netManage[index].busOffRecoverTimerHandle,m_pNetManageConfigure[index].busOffSlowTime);
            netManage[index].busOffTimerOnFlag = 1;  
						if(netManage[index].busOffTimeCount<m_pNetManageConfigure[index].busOffErrorEventLimitCount)
						{
							netManage[index].busOffTimeCount++; 
						}		
						TBOX_PRINT("busoff occured %d\r\n", netManage[index].busOffTimeCount);
          }
          else
          {
            NetManageInializeHardware(index);
            //vTaskDelay(5);
          }
      }
   
    }

  }
  else
  {
    // CAN normal - start confirm timer if busoff has occurred, wait 100ms stable bus then reset counter
    if((netManage[index].busOffTimeCount > 0) && (0x00==netManage[index].busOffClearConfirmFlag))
    {
      TimerHalStartTime(netManage[index].busOffClearConfirmTimerHandle, BUSOFF_CLEAR_CONFIRM_TIME_MS);
      netManage[index].busOffClearConfirmFlag = 1;
    }
  }
 
  if(netManage[index].busOffTimeCount>=m_pNetManageConfigure[index].busOffErrorEventLimitCount)
  {
    if(0x00==netManage[index].busOffEventOn)
    {
      netManage[index].busOffEventOn = 0x01;
      NetManageBusOffErrorCallBack(index,0x01);
      TBOX_PRINT("set busOff DTC\r\n");
    }
  }
  
}

static void NetManageStateProcess(uint8_t index)
{
    int16_t ret;
    
    OSEK_NM_LOG("osek net state %d is %d\r\n",index, netManage[index].netManageState);
    if(netManageState_NMOff==netManage[index].netManageState)
    {
      //
      OSEK_NM_LOG("[OSEK_NM] State NMOff -> NMInit, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMInit;//
    
    }
    if(netManageState_NMInit==netManage[index].netManageState)//NMon
    {
      //clear the bits sleep.ind and sleep.ack
      netManage[index].netWorkStatus.sleep_ind = 0;//addtional for netManageState_NMInit
      netManage[index].netWorkStatus.sleep_ack = 0;
      
      //initialize the hardware by D_init(...;BusInit)
      //NetManageInializeHardware(index);
      //netManage[index].netManageState = netManageState_NMInitReset;
      OSEK_NM_LOG("[OSEK_NM] State NMInit -> NMBusSleep, index=%d\r\n", index);
      netManage[index].netManageState =  netManageState_NMBusSleep;//2018.11.05
    }
    if(netManageState_NMInitReset==netManage[index].netManageState)//Awake
    {
      netManage[index].NMrxcount = 0;
      netManage[index].NMtxcount = 0;
      
      //Cancel all alarm timers
      CancelAlarmTtype(index);
      CancelAlarmTmax(index);
      CancelAlarmTerror(index);
      CancelAlarmTtx(index);
      CancelAlarmTwbs(index);
      //enable application communication by D_Online
      
      NetManageAppBusEnable(index);
      
      OSEK_NM_LOG("[OSEK_NM] State NMInitReset -> NMReset, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMReset;
    }
    if(netManageState_NMReset==netManage[index].netManageState)//Awake
    {
      //uint32_t canId;
      //uint8_t data[8];
      uint8_t opCode;
      //reset the system specific default configuration

      //increment the reception counter
      netManage[index].NMrxcount++;
      //send an alive message,increment the transmission counter
      if(netManage[index].netWorkStatus.NMactive)
      {
        netManage[index].canMsgTx.canId = m_pNetManageConfigure[index].netManageBaseAddress+m_pNetManageConfigure[index].netManageNodeId;
        opCode = 0;
        opCode |= NET_MANAGEE_OPCODE_BIT_MASK_ALIVE;
        //add sleep status later
        
        netManage[index].destinationId = m_pNetManageConfigure[index].netManageNodeId;
        netManage[index].canMsgTx.data[0] = netManage[index].destinationId;
        netManage[index].canMsgTx.data[1] = opCode;
        OSEK_NM_LOG("[OSEK_NM] NMReset send alive msg, index=%d, nodeId=%d\r\n", index, m_pNetManageConfigure[index].netManageNodeId);
#if(OSEK_NET_MANAGE_DEBUG)
        netManage[index].canMsgTx.data[3] = NET_MANAGE_STATE_RESET;//for debug
#endif
        ret = CanHalNmTransmit(netManage[index].canHandle,netManage[index].canMsgTx.canId,netManage[index].canMsgTx.data,netManage[index].canMsgTx.dlc,NM_TRANS_CANFD_ENABLE);
#if(OSEK_NET_MANAGE_DEBUG)
        netManage[index].canMsgTx.data[3] = 0x00;//for debug
#endif
        if(ret==0)
        {
          //transmit successful
          CanTransmitSuccessful(index);
        }
        else if(ret==1)//tx buffer full
        {
          //start Ttx
          SetAlarmTtx(index);
        }
        else
        {
          //transmit failed
          netManage[index].NMtxcount++;
        }
      }

      //initialize the NMPDU(reserved area in the Opcode and the data field)

      //
      OSEK_NM_LOG("[OSEK_NM] State NMReset -> NMInitNormal, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMInitNormal;

      if(netManage[index].NMtxcount>m_pNetManageConfigure[index].tx_limit)
      {
        OSEK_NM_LOG("[OSEK_NM] tx_limit exceeded, NMReset -> NMInitLimpHome, index=%d\r\n", index);
        netManage[index].netManageState = netManageState_NMInitLimpHome;
        //NetManageLimpHomeState(index,1);
      }
      if(netManage[index].NMrxcount>m_pNetManageConfigure[index].rx_limit)
      {
        OSEK_NM_LOG("[OSEK_NM] rx_limit exceeded, NMReset -> NMInitLimpHome, index=%d\r\n", index);
        netManage[index].netManageState = netManageState_NMInitLimpHome;
        //NetManageLimpHomeState(index,1);
      }
    }
    if(netManageState_NMInitNormal==netManage[index].netManageState)
    {
      //start TTyp to transmit the 1st message
      SetAlarmTtype(index);
      //netManage[index].destinationId = netManageParameter[index]->netManageNodeId;
      OSEK_NM_LOG("[OSEK_NM] State NMInitNormal -> NMNormal, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMNormal;
      
      if(netManage[index].netWorkStatus.NMactive == 0)
      {
        OSEK_NM_LOG("[OSEK_NM] Force recover NMactive on enter NMNormal, index=%d\r\n", index);
        netManage[index].netWorkStatus.NMactive = 1;
      }
      //set request sleep min timer
      //TimerHalStartTime(netManage[index].timerRequestSleepMinHandle,m_pNetManageConfigure[index].netManageTrequestSleepMin);
    }
    if(netManageState_NMNormal==netManage[index].netManageState)
    {
      //determine present configuration(alive and ring message received)
      //determine logical successor
      //deternine the limp home configuration
      //clear NMrxcount,if any NM message is received
      //clear NMtxcount in case of the transmission of any NM mesage
      //increment NMtxcount,if a NM message has to be transmited
      //repeat the transmission request(TTx) in case of a rejection from the DLL,no effect to NMtxcount
      //send an alive message,if skipped in the logical ring
      //pass the ring message delayed(Ttyp)(source=destination or destination=own station),if there in no ring message on the bus
      if(netManage[index].NMtxcount>m_pNetManageConfigure[index].tx_limit)
      {
        //disable TTyp TMax TTx
        CancelAlarmTtype(index);
        CancelAlarmTtx(index);
        CancelAlarmTmax(index);
        //NMInitLimpHome
        OSEK_NM_LOG("[OSEK_NM] NMNormal tx_limit exceeded -> NMInitLimpHome, index=%d\r\n", index);
        netManage[index].netManageState = netManageState_NMInitLimpHome;
      }
      if(netManage[index].NMrxcount>m_pNetManageConfigure[index].rx_limit)
      {
        //
        CancelAlarmTtype(index);
        CancelAlarmTtx(index);
        CancelAlarmTmax(index);
        OSEK_NM_LOG("[OSEK_NM] NMNormal rx_limit exceeded -> NMInitLimpHome, index=%d\r\n", index);
        netManage[index].netManageState = netManageState_NMInitLimpHome;
      }
    }
    if(netManageState_NMInitLimpHome==netManage[index].netManageState)
    {
      //start TError to transmit the 1st limp home message
      SetAlarmTerror(index);
      OSEK_NM_LOG("[OSEK_NM] State NMInitLimpHome -> NMLimpHome, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMLimpHome;
      //TimerHalStartTime(netManage[index].timerRequestSleepMinHandle,m_pNetManageConfigure[index].netManageTrequestSleepMin);
      NetManageLimpHomeState(index,1);
    }
    if(netManageState_NMLimpHome==netManage[index].netManageState)
    {
      //send limphome message cyclically (TError)

      //enable cyclically(TError) application communication by D_Online

      //fatal Bus Error signalled by D_Status.ind(e.g.BusOff) jump to netManageState_NMInitLimpHome
      /*if(NetManageIsBusOffError(index)==0)//need complete later
      {

          //disable application communication by D_Offline
            NetManageBusOff(index);
          //initialize the hardware by D_Init(...;BusAwake)
          NetManageInializeHardware(index);
          NetManageBusOn(index);
          netManage[index].netManageState = netManageState_NMInitLimpHome;
      }*/
      //CanHardwareControllerProcess(index);
    }
    if(netManageState_NMNormalPreSleep==netManage[index].netManageState)
    {
      if(!netManage[index].netWorkStatus.busSleep)
      {
        OSEK_NM_LOG("[OSEK_NM] State NMNormalPreSleep -> NMNormal, index=%d\r\n", index);
        netManage[index].netManageState = netManageState_NMNormal;
        //TimerHalStartTime(netManage[index].timerRequestSleepMinHandle,netManageParameter[index]->netManageTrequestSleepMin);
      }
    }
    if(netManageState_NMNormalInitBusSleep==netManage[index].netManageState)
    {
      //disable application communication(D_Offline)
      CancelAlarmTtype(index);
      CancelAlarmTmax(index);
      NetManageAPPBusDisable(index);
      //SetAlarm(Twbs)
      SetAlarmTwbs(index);
      //NMTwbsNormal
      OSEK_NM_LOG("[OSEK_NM] State NMNormalInitBusSleep -> NMTwbsNormal, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMTwbsNormal;
    }
    if(netManageState_NMTwbsNormal==netManage[index].netManageState)
    {
      if(!netManage[index].netWorkStatus.busSleep)
      {
        NetManageAppBusEnable(index);
        CancelAlarmTwbs(index);
        //SetAlarmTmax(index);
        OSEK_NM_LOG("[OSEK_NM] State NMTwbsNormal -> NMInitReset, index=%d\r\n", index);
        netManage[index].netManageState = netManageState_NMInitReset;
      }
    }
    if(netManageState_NMLimpHomePreSleep==netManage[index].netManageState)
    {
      if(!netManage[index].netWorkStatus.busSleep)
      {
        OSEK_NM_LOG("[OSEK_NM] State NMLimpHomePreSleep -> NMLimpHome, index=%d\r\n", index);
        netManage[index].netManageState = netManageState_NMLimpHome;
        //TimerHalStartTime(netManage[index].timerRequestSleepMinHandle,m_pNetManageConfigure[index].netManageTrequestSleepMin);
      }
    }
    if(netManageState_NMLimpHomeInitBusSleep==netManage[index].netManageState)
    {
      //disable application communication(D_Offline)
      NetManageAPPBusDisable(index);
      //SetAlarm(Twbs)
      SetAlarmTwbs(index);
      //NMTwbsLimpHome
      CancelAlarmTerror(index);//19.1.31
      OSEK_NM_LOG("[OSEK_NM] State NMLimpHomeInitBusSleep -> NMTwbsLimpHome, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMTwbsLimpHome;
    }
    if(netManageState_NMTwbsLimpHome==netManage[index].netManageState)
    {
      if(netManage[index].limpHomeACKFlag)
      {
        //sleep state
      }
      else if(!netManage[index].netWorkStatus.busSleep)
      {
      
        if(netManage[index].destinationId == m_pNetManageConfigure[index].netManageNodeId)
        {
          /* ???????: ?????????????????NMReset??? */
          CancelAlarmTwbs(index);
          CancelAlarmTerror(index);
          netManage[index].NMrxcount = 0;
          netManage[index].NMtxcount = 0;
          OSEK_NM_LOG("[OSEK_NM] State NMTwbsLimpHome -> NMBusSleepWakeUp, index=%d\r\n", index);
          netManage[index].netManageState = netManageState_NMBusSleepWakeUp;
        }
        else
        {
          /* ????: ???LimpHome?? */
          NetManageAppBusEnable(index);
          CancelAlarmTwbs(index);
          SetAlarmTerror(index);
          OSEK_NM_LOG("[OSEK_NM] State NMTwbsLimpHome -> NMLimpHome, index=%d\r\n", index);
          netManage[index].netManageState = netManageState_NMLimpHome;
        }
      }
    }
    if(netManageState_NMBusSleep==netManage[index].netManageState)
    {
      if(netManage[index].limpHomeACKFlag)
      {
        //sleep state
      }
      else if(!netManage[index].netWorkStatus.busSleep)
      {
        OSEK_NM_LOG("[OSEK_NM] State NMBusSleep -> NMBusSleepWakeUp, index=%d\r\n", index);
        netManage[index].netManageState = netManageState_NMBusSleepWakeUp;
      }
    }
    if(netManageState_NMBusSleepWakeUp==netManage[index].netManageState)
    {
      //initialize the hardware by D_Init(...;BusAwake)
      NetManageInializeHardware(index);
      NetManageAppBusEnable(index);
      //clear limp home configuration
      netManage[index].NMrxcount = 0;
      netManage[index].NMtxcount = 0;
      netManage[index].destinationId = m_pNetManageConfigure[index].netManageNodeId;
     
      if(netManage[index].netWorkStatus.NMactive == 0)
      {
        OSEK_NM_LOG("[OSEK_NM] Force recover NMactive on BusSleepWakeUp, index=%d\r\n", index);
        netManage[index].netWorkStatus.NMactive = 1;
      }
      OSEK_NM_LOG("[OSEK_NM] State NMBusSleepWakeUp -> NMReset, index=%d\r\n", index);
      netManage[index].netManageState = netManageState_NMReset;
      netManage[index].busOffTimeCount = 0;
      NetManageLimpHomeState(index,0);
    }
}

static void NetManageTimerProcess(uint8_t index)
{
    CanHalMsg_t canMsgRx;
    int16_t ret;
    for(;;)//canr receive process
    {
        ret = CanHalNmReceive(netManage[index].canHandle,&canMsgRx,0);
        if(ret==0)//can receive successful
        {
          CanReceiveMessageProcess(index,canMsgRx.canId,canMsgRx.canData); 
        }
        else//not invalid message
        {
          break;
        }        
    }    
    if(TimerHalIsTimeout(netManage[index].timerTtypeHandle)==0)
    {
      TtypeTimeOutProcess(index);      
    }
    if(TimerHalIsTimeout(netManage[index].timerTmaxHandle)==0)
    {
      TmaxTimeOutProcess(index);
    }
    
    for(;;)//canr receive process
    {
        ret = CanHalNmReceive(netManage[index].canHandle,&canMsgRx,0);
        if(ret==0)//can receive successful
        {
          CanReceiveMessageProcess(index,canMsgRx.canId,canMsgRx.canData); 
        }
        else//not invalid message
        {
          break;
        }
    }
    
    if(TimerHalIsTimeout(netManage[index].timerTerrorHandle)==0)
    {
      TerrorTimeOutProcess(index);
    }

    if(TimerHalIsTimeout(netManage[index].timerTtxHandle)==0)
    {
      TtxTimeOutProcess(index);
    }

    if(TimerHalIsTimeout(netManage[index].timerTwbsHandle)==0)
    {
      TwbsTimeOutProcess(index);
    }
    if(TimerHalIsTimeout(netManage[index].busOffRecoverTimerHandle)==0)
    {
   uint8_t opCode;
      TimerHalStopTime(netManage[index].busOffRecoverTimerHandle);
      //CanHalResetHardware(netManage[index].canHandle);
      netManage[index].busOffTimerOnFlag = 0;
      
      OSEK_NM_LOG("[OSEK_NM] BusOff recover timer timeout, index=%d\r\n", index);
      //disable application communication by D_Offline
      NetManageAPPBusDisable(index);
      //initialize the hardware by D_Init(...;BusAwake)
        NetManageInializeHardware(index);
   if(netManageState_NMLimpHome!=netManage[index].netManageState )
   {
    OSEK_NM_LOG("[OSEK_NM] BusOff recover -> NMLimpHome, index=%d\r\n", index);
    netManage[index].netManageState = netManageState_NMLimpHome;
    //set Terror
    SetAlarmTerror(index);
   }
   OSEK_NM_LOG("[OSEK_NM] BusOff recover TX limphome msg, index=%d\r\n", index);
			//transmit limphome message
			opCode = 0x00;//canMsgTx.data[1]
			opCode |= NET_MANAGEE_OPCODE_BIT_MASK_LIMPHOME;
			netManage[index].canMsgTx.data[0] = netManage[index].destinationId;
			netManage[index].canMsgTx.data[1] = opCode;
			CanHalNmTransmit(netManage[index].canHandle,netManage[index].canMsgTx.canId,netManage[index].canMsgTx.data,netManage[index].canMsgTx.dlc,NM_TRANS_CANFD_ENABLE);
      //
			NetManageAppBusEnable(index);      
      if(!netManage[index].netWorkStatus.busSleep)
      {
        netManage[index].netManageState = netManageState_NMInitLimpHome;
        //vTaskDelay(5);
        //NetManageInializeHardware(index);        
      }

    }
    
    // BusOff DTC clear confirm timer: 100ms stable bus before clearing counter/DTC
    if(TimerHalIsTimeout(netManage[index].busOffClearConfirmTimerHandle)==0)
    {
      TimerHalStopTime(netManage[index].busOffClearConfirmTimerHandle);
      netManage[index].busOffClearConfirmFlag = 0;
      netManage[index].busOffTimeCount = 0;
      if(0x01==netManage[index].busOffEventOn)
      {
        netManage[index].busOffEventOn = 0x00;
        NetManageBusOffErrorCallBack(index,0x00);
        TBOX_PRINT("clear busOff DTC and counter\r\n");
      }
    }

}

static void NetManageAllCanReceiveCheck(uint8_t index)
{
  if( CanHalRecieveIsTimeOut(netManage[index].receiveCheckCanHandle)==0)
  {

    if(netManage[index].checkTimeCount>=(1000/5))
    {      
      netManage[index].checkTimeoutFlag = 1;
    }
    else
    {
      netManage[index].checkTimeCount++;      
    }
  }
  else
  {
    netManage[index].checkTimeoutFlag = 0;
    netManage[index].checkTimeCount = 0;  
  }  
}

static void NetCanTransmitSuccessCallBack(uint8_t index)
{
  netManage[index].busOffTimeCount = 0;
}

static void Net0CanTransmitSuccessCallBack()
{
  NetCanTransmitSuccessCallBack(0);
}

static void Net1CanTransmitSuccessCallBack()
{
  NetCanTransmitSuccessCallBack(1);
}

static void Net2CanTransmitSuccessCallBack()
{
  NetCanTransmitSuccessCallBack(2);
}
static void Net3CanTransmitSuccessCallBack()
{
  NetCanTransmitSuccessCallBack(3);
}

static void Net4CanTransmitSuccessCallBack()
{
  NetCanTransmitSuccessCallBack(4);
}

static const typeCanTxHalCallBackPtr  m_netCanTxSuccessFunList[] = 
{
  Net0CanTransmitSuccessCallBack,
  Net1CanTransmitSuccessCallBack,
  Net2CanTransmitSuccessCallBack,
  Net3CanTransmitSuccessCallBack, 
  Net4CanTransmitSuccessCallBack,
};

uint16_t NetManageOsekInit(const struc_netManageOsekConfigure* pNetManageConfigure,uint8_t netManageConfigureNumber)
{
    uint8_t i;
    CanHalMsgFilter_t canFilter;
    
    if(netManageConfigureNumber>sizeof(netManage)/sizeof(netManage[0]))
    {
      return -1;
    }
    m_pNetManageConfigure = pNetManageConfigure;
    m_netManageNumber = netManageConfigureNumber;
    
    for(i=0;i<m_netManageNumber;i++)
    {
      netManage[i].canRxBuffer = canRxBufferA[i];
      netManage[i].netWorkStatus.busSleep = 1;
      netManage[i].netWorkStatus.NMactive = 1;
      netManage[i].canMsgTx.canId = m_pNetManageConfigure[i].netManageBaseAddress+m_pNetManageConfigure[i].netManageNodeId;
      netManage[i].canMsgTx.dlc = 8;
      netManage[i].busOffTimerOnFlag = 0;
      netManage[i].busOffTimeCount = 0;
      netManage[i].limpHomeACKFlag = 0;
      netManage[i].reWakeupFlag = 0;
      netManage[i].receiveCheckCanHandle  = -1;
			netManage[i].checkTimeoutFlag = 1;
    }   
    for(int i=0;i<m_netManageNumber;i++)
    {
      CanHalEnableCanWakeup(m_pNetManageConfigure[i].canChannel);
      CanHalCanBusOffProcessDisalbe(m_pNetManageConfigure[i].canChannel);
      netManage[i].canHandle = CanHalOpen(m_pNetManageConfigure[i].canChannel);      
      canFilter.useStdFilter = 0x00;
      canFilter.canIdStdMin = 0x00;
      canFilter.canIdStdMax = 0x7FF;
      canFilter.useExdtFilter = 0x01;
      canFilter.canIdExtdMin = m_pNetManageConfigure[i].netManageBaseAddress;
      canFilter.canIdExtdMax = m_pNetManageConfigure[i].netManageBaseAddress+m_pNetManageConfigure[i].netManageNodeIdMax;
      CanHalSetFilter(netManage[i].canHandle,&canFilter);      
      CanHalSetRxBuffer(netManage[i].canHandle,netManage[i].canRxBuffer,sizeof(canRxBufferA[0])/sizeof(canRxBufferA[0][0]));
      CanHalRegisterTxFinishedCallBackFunction(netManage[i].canHandle,m_netCanTxSuccessFunList[i]);
      
      netManage[i].receiveCheckCanHandle = CanHalOpen(m_pNetManageConfigure[i].canChannel);//receive all message
      canFilter.useStdFilter = 0x01;
      canFilter.canIdStdMin = 0;
      canFilter.canIdStdMax = 0x800;    
      canFilter.useExdtFilter = 0x01; 
      canFilter.canIdExtdMin = 0x00;
      canFilter.canIdExtdMax = 0xFFFFFFFF;
      CanHalSetFilter(netManage[i].receiveCheckCanHandle,&canFilter); 
      
      netManage[i].timerTtypeHandle = TimerHalOpen();
      netManage[i].timerTmaxHandle = TimerHalOpen();
      netManage[i].timerTerrorHandle = TimerHalOpen();
      netManage[i].timerTtxHandle = TimerHalOpen();
      netManage[i].timerTwbsHandle = TimerHalOpen();
      //netManage[i].timerRequestSleepMinHandle = TimerHalOpen();
      
      netManage[i].busOffRecoverTimerHandle = TimerHalOpen();
      netManage[i].busOffClearConfirmTimerHandle = TimerHalOpen();
    }
    netManage[0].canInitFinished = 1;
    return 0;
}

static void NetManageStateDebugPrint(void)
{
  static uint32_t timeCount = 0;
  uint8_t i;
  if(m_debugPrintTime==0)
  {
    return;
  }
  timeCount++;
  if(timeCount<m_debugPrintTime)
  {
    return;
  }
  timeCount = 0;
  for(i=0;i<m_netManageNumber;i++)
  {
    TBOX_PRINT("osek net state %d is %d\r\n",i,netManage[i].netManageState);
  }
  
}

void NetManageOsekCycleProcess(void)
{
    uint8_t i;
    int16_t ret;
    CanHalMsg_t canMsgRx;
    for(i=0;i<m_netManageNumber;i++)
    {
        for(;;)
        {
            ret = CanHalNmReceive(netManage[i].canHandle,&canMsgRx,0);
            if(ret==0)//can receive successful
            {
              CanReceiveMessageProcess(i,canMsgRx.canId,canMsgRx.canData); 
            }
            else//not invalid message
            {
              break;
            }        
        }
        NetManageStateProcess(i); 
				CanHardwareControllerProcess(i);
    }      
    //vTaskDelay(5);//wait for 5ms
    for(i=0;i<m_netManageNumber;i++)
    {
      NetManageTimerProcess(i);
    }
    for(i=0;i<m_netManageNumber;i++)
    {
      NetManageAllCanReceiveCheck(i);
    }
    NetManageStateDebugPrint();
}

void NetManageOsekSetDebugPrint(uint32_t cycleTime_ms)
{
  m_debugPrintTime = cycleTime_ms;
}

/*********************
OSEK NM SDK API Functions
**********************/

int16_t OsekNmSdkEnableTx(uint8_t nmID)
{
  if(nmID>=m_netManageNumber)
  {
    return -1;
  }
  
  netManage[nmID].txEnabled = 1;
  netManage[nmID].netWorkStatus.NMactive = 1;
  return E_OK;
}

int16_t OsekNmSdkDisableTx(uint8_t nmID)
{
  if(nmID>=m_netManageNumber)
  {
    return -1;
  }
  
  netManage[nmID].txEnabled = 0;
  netManage[nmID].netWorkStatus.NMactive = 0;
  return E_OK;
}

int16_t OsekNmSdkGetSleepStatus(uint8_t nmID, uint8_t *sleepState)
{
  if(nmID>=m_netManageNumber || sleepState==NULL)
  {
    return -1;
  }
  
  if(netManage[nmID].netManageState==netManageState_NMBusSleep)
  {
    *sleepState = 1;
  }
  else if(netManage[nmID].reWakeupFlag)
  {
    netManage[nmID].reWakeupFlag = 0;
    *sleepState = 2;
  }
  else
  {
    *sleepState = 0;
  }
  
  return E_OK;
}

int16_t OsekNmSdkGetCanStatus(uint8_t nmID, uint8_t *ReceiveState)
{
  if(nmID>=m_netManageNumber || ReceiveState==NULL)
  {
    return -1;
  }
  
  *ReceiveState = netManage[nmID].nmMsgReceived;
  netManage[nmID].nmMsgReceived = 0;
  
  return E_OK;
}

uint8_t OsekNmSdkGetBusState(void)
{
  uint8_t i;
  uint8_t busState = 0;
  
  for(i=0;i<m_netManageNumber;i++)
  {
    if(netManage[i].busOffState)
    {
      busState = 1;
      break;
    }
  }
  
  return busState;
}

int16_t OsekNmSdkSetWakeupRequest(uint8_t wakeupRequestFlags)
{
  (void)wakeupRequestFlags;
  
  return E_OK;
}

int16_t OsekNmSdkClearWakeupRequest(void)
{
  return E_OK;
}

void OsekNmSdkGetNetManageCanChannel(uint8_t nmID, uint8_t *channelFlag)
{
  if(nmID<m_netManageNumber && channelFlag!=NULL)
  {
    *channelFlag = m_pNetManageConfigure[nmID].canChannel;
  }
}

void OsekNmSdkGetNetManageReceiveStatusClear(uint8_t nmID)
{
  if(nmID<m_netManageNumber)
  {
    netManage[nmID].nmMsgReceived = 0;
    netManage[nmID].checkTimeoutFlag = 1;
  }
}

int16_t OsekNmSdkGetNetManageReceiveStatus(uint8_t nmID, uint8_t *ReceiveState)
{
  if(nmID>=m_netManageNumber || ReceiveState==NULL)
  {
    return -1;
  }
  
  *ReceiveState = (netManage[nmID].checkTimeoutFlag==0) ? 1 : 0;
  
  return E_OK;
}

/*********************
core
**********************/
StatusType OsekNmStartNM(NetIdType NetId)
{
  return E_OK;
}
/**********************
core
***********************/
StatusType OsekNmStopNM(NetIdType NetId)
{
  return E_OK;
}

StatusType OsekNmGotoMode(NetIdType NetId,NMModeName NewMode)
{
  if(NetId>=m_netManageNumber)
  {
    return -1;
  }
  netManage[NetId].reWakeupFlag = 0;
  //RTOS_HalDisableScheduler();
  if(NMModeName_BusSleep==NewMode)
  {
    netManage[NetId].netWorkStatus.busSleep = 1;
  }
  else if(NMModeName_Awake==NewMode)
  {
    netManage[NetId].limpHomeACKFlag = 0;    
    netManage[NetId].netWorkStatus.busSleep = 0;
    //netManage[NetId].netManageState = netManageState_NMBusSleepWakeUp;
  }
  //RTOS_HalEnableScheduler();
  return E_OK;  
}
StatusType OsekNmTalkNM(NetIdType NetId)
{
  if(NetId>=m_netManageNumber)
  {
    return -1;
  }
  
  netManage[NetId].netWorkStatus.NMactive = 1;
  return E_OK;  
}
StatusType OsekNmSilentNM(NetIdType NetId)
{
  if(NetId>=m_netManageNumber)
  {
    return -1;
  }
  
  netManage[NetId].netWorkStatus.NMactive = 0;
  return E_OK;  
}
/************************************

return 
2:rewaked up by can
1:sleep
0:wait sleep
**************************************/
StatusType OsekNmGetStatusSleep(NetIdType NetId,uint8_t *sleepState)
{
  if(NetId>=m_netManageNumber)
  {
    return -1;
  }
  //RTOS_HalDisableScheduler();
  if(netManage[NetId].netManageState==netManageState_NMBusSleep)
  {
    *sleepState = 1;
  }
  else if(netManage[NetId].reWakeupFlag)
  {
    netManage[NetId].reWakeupFlag = 0;
    *sleepState = 2;
  }
  else 
  {
    *sleepState = 0;
  }
  //RTOS_HalEnableScheduler();
  return E_OK;
}

StatusType OsekNmAllStartNM(void)
{
  uint8_t i;
  StatusType retStatus;
  retStatus = E_OK;
  for(i=0;i<m_netManageNumber;i++)
  {
    retStatus = OsekNmStartNM(i);
    if(retStatus!=E_OK)
    {
      break;
    }
  }
  return retStatus;
}

StatusType OsekNmAllStopNM(void)
{
  uint8_t i;
  StatusType retStatus;
  retStatus = E_OK;
  for(i=0;i<m_netManageNumber;i++)
  {
    retStatus = OsekNmStopNM(i);
    if(retStatus!=E_OK)
    {
      break;
    }
  }
  return retStatus;
}

StatusType OsekNmAllGotoMode(NMModeName NewMode)
{
  uint8_t i;
  StatusType retStatus;
  retStatus = E_OK;
  for(i=0;i<m_netManageNumber;i++)
  {
    retStatus = OsekNmGotoMode(i,NewMode);
    if(retStatus!=E_OK)
    {
      break;
    }
  }
  return retStatus;
}

StatusType OsekNmAllTalkNM(void)
{
  uint8_t i;
  StatusType retStatus;
  retStatus = E_OK;
  for(i=0;i<m_netManageNumber;i++)
  {
    retStatus = OsekNmTalkNM(i);
    if(retStatus!=E_OK)
    {
      break;
    }
  }
  return retStatus;
}

StatusType OsekNmAllSilentNM(void)
{
  uint8_t i;
  StatusType retStatus;
  retStatus = E_OK;
  for(i=0;i<m_netManageNumber;i++)
  {
    retStatus = OsekNmSilentNM(i);
    if(retStatus!=E_OK)
    {
      break;
    }
  }
  return retStatus;
}

StatusType OsekNmAllGetStatusSleep(uint8_t *sleepState)
{
  uint8_t i;
  uint8_t sleepStatus;
  uint8_t allSleepStatus;
  StatusType retStatus;
  retStatus = E_OK;
  sleepStatus = 1;
  allSleepStatus = 1;
  
  for(i=0;i<m_netManageNumber;i++)
  {
    retStatus = OsekNmGetStatusSleep(i,&sleepStatus);
    if(retStatus!=E_OK)
    {
      break;
    }
    if(2==sleepStatus)
    {
      allSleepStatus = 2;
      break;
    }  
    else if(0==sleepStatus)
    {
      allSleepStatus = 0;
    }
  }
  *sleepState = allSleepStatus;
  return retStatus;
}
StatusType OsekLimpHomeSleepAck(uint8_t *sleepAck)
{
  int16_t ret;
  int16_t i;
  ret = 0;
  *sleepAck = 1;
  for(i=0;i<m_netManageNumber;i++)
  {
    if(!netManage[i].limpHomeACKFlag)
    {
      *sleepAck = 0;
      break;
    }
  }
  return ret;
}

StatusType OsekNm_GetAllCanStatus(uint8_t *ReceiveState)
{
  uint8_t i;
  StatusType ret;
  ret = E_OK;
  *ReceiveState = 0;
  for(i=0;i<m_netManageNumber;i++)
  {
    if(0==netManage[i].checkTimeoutFlag)
    {
      *ReceiveState = 1;
      break;
    }
  }   
  return ret;
}

StatusType OsekNm_GetNetManageReceiveStatus(uint8_t *ReceiveState)
{
  uint8_t i;
  StatusType ret;
  ret = E_OK;
  *ReceiveState = 0;
  for(i=0;i<m_netManageNumber;i++)
  {
    if(0==netManage[i].checkTimeoutFlag)
    {
      *ReceiveState = 1;
      break;
    }
  }   
  //*ReceiveState = 1;
  return ret;
}

void OsekNm_NetManageReceiveStatusClear(void)
{
  uint8_t i;  
  for(i=0;i<m_netManageNumber;i++)
  {
    netManage[i].checkTimeoutFlag = 1;
  }   
}

/*
StatusType OsekNm_GetNetManageReceiveStatus(uint8_t *ReceiveState)
{
  uint8_t i;
  StatusType ret;
  ret = E_OK;
  *ReceiveState = 0;
  for(i=0;i<OSEK_NET_MANAGE_MAX_INSTANCE;i++)
  {
    if(netManage[i].nmMsgReceiveFlag)
    {
      *ReceiveState = 1;
      break;
    }
  }   
  return ret;
}*/
/*
void OsekNm_NetManageReceiveStatusClear(void)
{
  uint8_t i;  
  for(i=0;i<OSEK_NET_MANAGE_MAX_INSTANCE;i++)
  {
    netManage[i].nmMsgReceiveFlag = 0;
  }   
}*/

void OsekNm_NetManageCanChannel(uint8_t *channelFlag)
{
  uint8_t i;
  for(i=0;i<m_netManageNumber;i++)
  {
    if(m_pNetManageConfigure[i].canChannel<8)
    {
      *channelFlag |= 0x01<<(m_pNetManageConfigure[i].canChannel);
    }
  }
}

/* Nm_AllEnableCommunication - Enable all NM communication */
StatusType Nm_AllEnableCommunication(void)
{
  uint8_t i;
  StatusType retStatus = E_OK;
  
  for(i=0;i<m_netManageNumber;i++)
  {
    retStatus = OsekNmTalkNM(i);
    if(retStatus!=E_OK)
    {
      break;
    }
  }
  
  return retStatus;
}

/* Nm_AllDisableCommunication - Disable all NM communication */
StatusType Nm_AllDisableCommunication(void)
{
  uint8_t i;
  StatusType retStatus = E_OK;
  
  for(i=0;i<m_netManageNumber;i++)
  {
    retStatus = OsekNmSilentNM(i);
    if(retStatus!=E_OK)
    {
      break;
    }
  }
  
  return retStatus;
}
