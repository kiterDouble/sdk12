/*************************************************
 Copyright © 2025 SiRun (AnHui). All rights reserved.
 File Name: autosarNmSdk.c
 Author: 
 Created Time: 
 Description: Implements AUTOSAR network management functionality, including network state management,
        CAN communication processing, timer management, bus error handling, etc.Provides APIs for 
        network request, release, communication control, etc., supporting the complete implementation 
        of the network management state machine.Main functionalities include network wake-up, sleep 
        management, repeat message handling, bus error recovery, etc.
 Others: This file implements the AUTOSAR network management protocol specification, supporting the 
        standard NM message format and state transition logic.It contains multiple static helper 
        functions and public API functions, supporting the complete lifecycle management of network 
        management.It supports multi-instance configuration and can manage multiple network management 
        channels simultaneously.
 *************************************************/

/****************************** include ***************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "timerHal.h"
#include "autosarNmSdk.h"
#include "secocSdk.h"
#include "peripheralHal.h"
#include "logHal.h"
#include "string.h"
#include "taskPowerManage.h"
#include "powerManageHal.h"
#include "canPeriodTask.h"
/****************************** Macro Definitions ******************************/
#define AUTOSAR_NET_MANAGE_MAX_INSTANCE         (1U)
#define NM_MESSAGE_PDU_LENGTH                   (8U)
#define NM_TRANS_CANFD_ENABLE                   (0U)
#define NM_E_OK                                 (0U)
#define NM_E_NOT_EXECUTED                       (2U)
#define NM_E_NOT_OK                             (3U)
#define NM_BUS_OFF_ERROR_RESET_MSG_ENABLE       (1U)

//Network management message config
#define NM_MESSAGE_DEFAULT_VALUE                (0x00)

//CBV byte default value                      
#define NM_CBV_DEFAULT_VALUE                    (NM_MESSAGE_DEFAULT_VALUE)

//Wake up reason byte
#define NM_WAKE_UP_REASON_DEFALUT_VALUE         (NM_MESSAGE_DEFAULT_VALUE)          
#define NM_CAN_MESSAGE_WAKEUP_VALUE             (0x01)
#define NM_IGN_ON_WAKEUP_VALUE                  (0x12)
#define NM_TSP_MESSAGE_WAKEUP_VALUE             (0x83)

//CAN ID valid range constants
#define NM_CAN_ID_LOWER_LIMIT                   (0U)       
#define NM_CAN_ID_MIDDLE_UPPER_LIMIT            (0x500U)    
#define NM_CAN_ID_UPPER_LIMIT                   (0x5FFU)    

//CAN ID valid range macro
#define NM_CAN_ID_VALID_RANGE(canId)            (((canId > NM_CAN_ID_LOWER_LIMIT) && (canId < NM_CAN_ID_MIDDLE_UPPER_LIMIT)) || (canId > NM_CAN_ID_UPPER_LIMIT))

/* CAN Sub-Network Wakeup Request Bit Masks */
#define NM_SUB_NET_WAKEUP_PCAN_MASK       (1U << 0U)  /* Bit 0: PCAN sub-network */
#define NM_SUB_NET_WAKEUP_ECAN_MASK       (1U << 1U)  /* Bit 1: ECAN sub-network */
#define NM_SUB_NET_WAKEUP_BCAN_MASK       (1U << 2U)  /* Bit 2: BCAN sub-network */
#define NM_SUB_NET_WAKEUP_ACAN_MASK       (1U << 3U)  /* Bit 3: ACAN sub-network */
#define NM_SUB_NET_WAKEUP_ICAN_MASK       (1U << 4U)  /* Bit 4: ICAN sub-network */
#define NM_SUB_NET_WAKEUP_TCAN_MASK       (1U << 5U)  /* Bit 5: TCAN sub-network */
#define NM_SUB_NET_WAKEUP_LCAN_MASK       (1U << 6U)  /* Bit 6: LCAN sub-network */

/* Helper macros to set/clear specific bits */
#define NM_SUB_NET_WAKEUP_SET(flags, mask)    ((flags) |= (mask))
#define NM_SUB_NET_WAKEUP_CLEAR(flags, mask)  ((flags) &= ~(mask))
#define NM_SUB_NET_WAKEUP_IS_SET(flags, mask) ((flags) & (mask))

#define NM_SUB_NET_WAKEUP_REQ_DEFAULT_VALUE     (NM_SUB_NET_WAKEUP_TCAN_MASK) /* Default: only wake up TCAN */

/* Common predefined wakeup configurations */
#define NM_SUB_NET_WAKEUP_ALL_SUBNETS           (NM_SUB_NET_WAKEUP_PCAN_MASK | NM_SUB_NET_WAKEUP_ECAN_MASK | \
                                               NM_SUB_NET_WAKEUP_BCAN_MASK | NM_SUB_NET_WAKEUP_ACAN_MASK | \
                                               NM_SUB_NET_WAKEUP_ICAN_MASK | NM_SUB_NET_WAKEUP_TCAN_MASK | \
                                               NM_SUB_NET_WAKEUP_LCAN_MASK)
#define NM_SUB_NET_WAKEUP_NO_SUBNETS            (0U) /* No subnets wake up */

#define NM_WUP_VALUE                            (0x4C)

// Global variable for sub-network wakeup request value
static uint8_t g_nmSubNetWakeupRequestValue = NM_SUB_NET_WAKEUP_REQ_DEFAULT_VALUE;
/****************************** Type Definitions ******************************/
typedef enum
{
	E_NETMANAGESTATE_IDLEMODE,
	E_NETMANAGESTATE_BUSSLEEPMODE,           
	E_NETMANAGESTATE_PREPAREBUSSLEEPMODE,    
	E_NETMANAGESTATE_STARTUPSTATE,              //GAC Rename to E_NETMANAGESTATE_STARTUPSTATE
	E_NETMANAGESTATE_NORMALOPERATIONSTATE,  
	E_NETMANAGESTATE_READYSLEEPSTATE,     
}NetManageAutosarState_e;

typedef struct
{
    NetManageAutosarState_e netManageState;
    int16_t canHandle;
    uint8_t nmMsgReceiveFlag;
    uint8_t networkRequestFlag;                  
    uint8_t passiveStartUpFlag;                   
    //uint8_t receiveNMpassiveStartUpFlag;           
    //uint8_t repeatMessageBitReceivedFlag;         //GAC don't need this flag  
    //uint8_t immediateCycleState;                  //GAC don't need this flag  
    uint8_t appEnableFlag;
    int16_t NMTimerHandle;
    int16_t MsgCycleTimerHandle;
    int16_t BusSleepTimerHandle;
    int16_t StartUpTimerHandle;                     //GAC rename RepeatMessageTimerHandle  to StartUpTimerHandle       
    //uint8_t CBVRepeatMessage;                     //GAC don't need this flag  
    //uint8_t CBVActiveWakeUp;                      //GAC don't need this flag                        
    uint32_t immediateCycleCount;                 
    //uint8_t repeatMessageRequestFlag;             //GAC don't need this flag         
    uint32_t busOffTimeCount;                      
    uint32_t busOffTimesCnt;
    uint8_t busOffEventOn;
    uint8_t passiveMode;                           
    uint8_t busOffTimerOnFlag;                     
    int16_t busOffRecoverTimerHandle;
    //  stru_can_hal_msg canRxBuffer[30];
    CanHalMsg_t canRxBuffer[5];
    int16_t receiveCheckCanHandle; 
    uint32_t checkTimeCount;
    uint8_t checkTimeoutFlag;                      
}AutosarNmVariable_t;

typedef enum
{
    NM_IGN_OFF = 0,
    NM_IGN_ON = 1,
}NetManageAutosarIgnStatus;
/****************************** Global Variables ******************************/
static AutosarNmVariable_t g_netManage[AUTOSAR_NET_MANAGE_MAX_INSTANCE];
static const AutosarNmParameter_t *g_pNmConfigure = NULL;
static uint8_t g_netManageNumber = 0;

/****************************** Function Declarations *************************/
static void StartNMTimer(uint8_t index);
static void StartWaitBusSleepTimer(uint8_t index);
static void StopWaitBusSleepTimer(uint8_t index);
static void StartMsgCycleTimer(uint8_t index);
static void StopMsgCycleTimer(uint8_t index);
static void StartStartUpTimer(uint8_t index);
static void StopStartUpTimer(uint8_t index);
static void MsgCycleTimeOutProcess(uint8_t index);
static void NetManageStateBussLeepMode(uint8_t index);
static void NetManageStateStartUp(uint8_t index);
static void NetManageStateReadySleep(uint8_t index);
static void NetManageStateNomalOperation(uint8_t index);
static void NetManageStatePrepareBusSleep(uint8_t index);
static void NetManageStateProcess(uint8_t index);
static void NetManageInializeHardware(uint8_t index);
static void CanHardwareControllerProcess(uint8_t index);
#if(NM_BUS_OFF_ERROR_RESET_MSG_ENABLE) == 1
static void BusOffErrorResetCanNMTransmit(uint8_t index);
#endif
static void NetManageTimerProcess(uint8_t index);
static void NetManageAllCanReceiveCheck(uint8_t index);
#if(0)
static void ApplicationMessageDisableTransmit(uint8_t index);
static void ApplicationMessageEnableTransmit(uint8_t index);
#endif
static void NetManageBusOffErrorCallBack(uint8_t index,uint8_t flag);
static int16_t AutosarNmNetworkRequest(const int16_t nmChannelHandle);
static int16_t AutosarNmPassiveStartUp(const int16_t nmChannelHandle);
static void CanReceiveMessageProcess(uint8_t index,uint32_t canId,uint8_t *canData);
static void NetManageStateCyclePrint(uint8_t nmId,uint8_t nmState,uint8_t requestFlag);

/****************************** Public Function Implementations ***************/
/*************************************************
   Function:        AutosarNmSdkConfig
   Description:     Configure the AUTOSAR Network Management module
   Input:           pNmConfigre - Pointer to the configuration parameters for AUTOSAR NM
                    NmNumber - Number of network management instances to configure
   Output:          None
   Return:          0 on success, -1 if NmNumber exceeds maximum supported instances
   Others:          Initializes network management states for each instance
                    Sets configuration parameters and global variables
 *************************************************/
int16_t AutosarNmSdkConfig(const AutosarNmParameter_t *pNmConfigre, uint8_t NmNumber)
{
    uint8_t i;
    CanHalMsgFilter_t canFilter;
    int16_t retVal = 0;
    if(NmNumber>sizeof(g_netManage)/sizeof(g_netManage[0]))
    {
        retVal = -1;
    }   
    else
    {
        g_pNmConfigure = pNmConfigre;
        g_netManageNumber = NmNumber;
        for(i = 0; i < g_netManageNumber; i++)
        {
            g_netManage[i].netManageState = E_NETMANAGESTATE_IDLEMODE;  
            g_netManage[i].canHandle = -1;
            g_netManage[i].nmMsgReceiveFlag = 0;
            g_netManage[i].networkRequestFlag = 0;
            g_netManage[i].passiveStartUpFlag = 0; 
            //g_netManage[i].receiveNMpassiveStartUpFlag = 0;     
            g_netManage[i].NMTimerHandle = -1;
            g_netManage[i].MsgCycleTimerHandle = -1;
            g_netManage[i].BusSleepTimerHandle = -1;
            g_netManage[i].StartUpTimerHandle = -1;
            g_netManage[i].busOffRecoverTimerHandle = -1;
            g_netManage[i].passiveMode = 0;
            g_netManage[i].busOffTimerOnFlag = 0;
            g_netManage[i].checkTimeCount = 0;
            g_netManage[i].checkTimeoutFlag = 1;
            g_netManage[i].appEnableFlag = 0;
        }   
        for(i = 0; i < g_netManageNumber; i++)
        {
            CanHalEnableCanWakeup(g_pNmConfigure[i].canChannel);
            CanHalCanBusOffProcessEnalbe(g_pNmConfigure[i].canChannel);
            g_netManage[i].canHandle = CanHalOpen(g_pNmConfigure[i].canChannel);
            canFilter.useStdFilter = 0x01;
            canFilter.canIdStdMin = g_pNmConfigure[i].BaseAddress;
            canFilter.canIdStdMax = g_pNmConfigure[i].BaseAddress+g_pNmConfigure[i].NodeIdMax;    
            canFilter.useExdtFilter = 0x00; 
            canFilter.canIdExtdMin = 0x01;
            canFilter.canIdExtdMax = 0x01;
            CanHalSetFilter(g_netManage[i].canHandle, &canFilter);      
            CanHalSetRxBuffer(g_netManage[i].canHandle, g_netManage[i].canRxBuffer, sizeof(g_netManage[0].canRxBuffer)/sizeof(g_netManage[0].canRxBuffer[0]));  
            g_netManage[i].receiveCheckCanHandle = CanHalOpen(g_pNmConfigure[i].canChannel);
            canFilter.useStdFilter = 0x01;
            canFilter.canIdStdMin = 0;
            canFilter.canIdStdMax = 0x800;    
            canFilter.useExdtFilter = 0x01; 
            canFilter.canIdExtdMin = 0x00;
            canFilter.canIdExtdMax = 0xFFFFFFFF;
            CanHalSetFilter(g_netManage[i].receiveCheckCanHandle, &canFilter);   
            g_netManage[i].NMTimerHandle = TimerHalOpen();
            g_netManage[i].MsgCycleTimerHandle = TimerHalOpen();
            g_netManage[i].BusSleepTimerHandle = TimerHalOpen();
            g_netManage[i].StartUpTimerHandle = TimerHalOpen();
            g_netManage[i].busOffRecoverTimerHandle = TimerHalOpen();
        }
    } 
    return retVal;
}

/*************************************************
 Function:        NetManageAutosarCycleProcess
 Description:     Main processing loop for AUTOSAR network management
 Input:           None
 Output:          None
 Return:          None
 Others:          Periodically prints network management state information
                  Processes received CAN NM messages
                  Calls state machine processing for each network management instance
                  Handles hardware controller events
                  Processes network management timers
                  Monitors CAN message reception status
 *************************************************/
void NetManageAutosarCycleProcess(void)
{
    static uint32_t printTimeCount = 0;
    int16_t ret;
    uint8_t i;
    CanHalMsg_t canMsgRx;

    printTimeCount++;
    if(printTimeCount >= 1000)
    {
        printTimeCount = 0;
        for(i = 0;i < g_netManageNumber; i++)
        {
            NetManageStateCyclePrint(i, g_netManage[i].netManageState, g_netManage[i].networkRequestFlag);
        }        
    }
    for(i = 0; i < g_netManageNumber; i++)
    {
        NetManageAllCanReceiveCheck(i);
    }
    for(i = 0;i < g_netManageNumber; i++)
    {             
        for(;;)
        {
            ret = CanHalNmReceive(g_netManage[i].canHandle,&canMsgRx,0);
            if (0 == ret && NM_CAN_ID_VALID_RANGE(canMsgRx.canId))
            {
                break;
            }
            if(ret == 0)    
            {
                CanReceiveMessageProcess(i,canMsgRx.canId,canMsgRx.canData);
            }
            else       
            {
                break;
            }  
        }
        NetManageStateProcess(i);
        CanHardwareControllerProcess(i);
    }
    for(i = 0;i < g_netManageNumber; i++)
    {
        NetManageTimerProcess(i);
    }
}

/*************************************************
   Function:        AutosarNmSdkNetworkRequest
   Description:     This function is used to request network communication or passive startup for all AUTOSAR NM instances
   Input:           LocalWakeup - Flag indicating whether to perform local wakeup (1) or passive startup (0)
   Output:          None
   Return:          NM_E_OK if all operations are successful, otherwise error code
   Others:          Iterates through all configured AUTOSAR NM instances and performs the corresponding operation
 *************************************************/
int16_t AutosarNmSdkNetworkRequest(uint8_t LocalWakeup)
{
    uint8_t i;
    int16_t ret;
    ret = NM_E_OK;
    for(i = 0; i < AUTOSAR_NET_MANAGE_MAX_INSTANCE; i++)
    {
        if(LocalWakeup)
        {
            ret = AutosarNmNetworkRequest(i); 
        }
        else
        {
            ret = AutosarNmPassiveStartUp(i);
        }
        if(ret != NM_E_OK)
        {
            break;
        }
    }
    return ret;
}

/*************************************************
   Function:        AutosarNmSdkNetworkRelease
   Description:     Release network communication request for specified NM channel
   Input:           nmChannelHandle - Network management channel handle
   Output:          None
   Return:          NM_E_OK on success, NM_E_NOT_OK if channel handle is invalid, NM_E_NOT_EXECUTED if not in valid state
   Others:          Clears network request flag only in REPEATMESSAGESTATE or NORMALOPERATIONSTATE
                    Used to signal completion of network communication needs
  *************************************************/
int16_t AutosarNmSdkNetworkRelease(const int16_t nmChannelHandle)
{
    int16_t retVal = NM_E_OK;
    if(nmChannelHandle >= AUTOSAR_NET_MANAGE_MAX_INSTANCE)
    {
        retVal = NM_E_NOT_OK;
    }
    else if(E_NETMANAGESTATE_STARTUPSTATE == g_netManage[nmChannelHandle].netManageState)
    {
        g_netManage[nmChannelHandle].networkRequestFlag = 0;
    }
    else if(E_NETMANAGESTATE_NORMALOPERATIONSTATE == g_netManage[nmChannelHandle].netManageState)
    {
        g_netManage[nmChannelHandle].networkRequestFlag = 0;
    }
    else 
    {
        retVal = NM_E_NOT_EXECUTED;
    }
    return retVal;
}

/*************************************************
   Function:        AutosarNmSdkEnableCommunication
   Description:     Enable network communication for specified NM channel
   Input:           nmID - Network management channel ID
   Output:          None
   Return:          NM_E_OK on success, NM_E_NOT_OK if channel ID is invalid
   Others:          Sets passiveMode flag to 0, enabling active communication mode
                    Allows normal network message transmission and reception
  *************************************************/
int16_t AutosarNmSdkEnableCommunication(uint8_t nmID)
{
  int16_t retVal = NM_E_OK;
  if(nmID >= AUTOSAR_NET_MANAGE_MAX_INSTANCE)
  {
    retVal = NM_E_NOT_OK;
  }
  else
  {
    g_netManage[nmID].passiveMode = 0;
  }
  return retVal;
}

/*************************************************
 Function:        AutosarNmSdkDisableCommunication
 Description:     Disable communication for the specified network management channel
 Input:           nmID - Network management channel identifier
 Output:          None
 Return:          NM_E_OK on success, NM_E_NOT_OK if nmID exceeds maximum supported instances
 Others:          Sets passive mode flag to 1 to prevent normal message transmission
 *************************************************/
int16_t AutosarNmSdkDisableCommunication(uint8_t nmID)
{
    int16_t retVal = NM_E_OK;
    if(nmID >= AUTOSAR_NET_MANAGE_MAX_INSTANCE)
    {
        retVal = NM_E_NOT_OK;
    }
    else
    {
        g_netManage[nmID].passiveMode = 1;
    }
    return retVal;
}

/*************************************************
 Function:        AutosarNmSdkGetSleepStatus
 Description:     Get the sleep status of the specified network management channel
 Input:           nmID - Network management channel identifier
 Output:          sleepState - Pointer to store sleep state: 1=sleep mode, 0=waiting to sleep, 2=re-wakeup
 Return:          NM_E_OK on success
 Others:          Checks network request flag and current network management state
 *************************************************/
int16_t AutosarNmSdkGetSleepStatus(uint8_t nmID,uint8_t *sleepState)
{
    int16_t ret;
    ret = NM_E_OK;
    *sleepState = 1;   
    if(E_NETMANAGESTATE_BUSSLEEPMODE == g_netManage[nmID].netManageState)
    {
        *sleepState = 1;
    }
    else if(g_netManage[nmID].networkRequestFlag)
    {
        *sleepState = 2;//rewake up the whole system
    }
    else
    {
        *sleepState = 0;//wait to sleep
    }
    return ret;
}

/*************************************************
 Function:        AutosarNmSdkGetCanStatus
 Description:     Get the CAN receive status of the specified network management channel
 Input:           nmID - Network management channel identifier
 Output:          ReceiveState - Pointer to store receive state: 1=normal, 0=timeout
 Return:          NM_E_OK on success
 Others:          Checks the timeout flag to determine if CAN messages are being received properly
 *************************************************/
int16_t AutosarNmSdkGetCanStatus(uint8_t nmID, uint8_t *ReceiveState)
{
    int16_t ret;
    ret = NM_E_OK;
    *ReceiveState = 1;
    if(0 != g_netManage[nmID].checkTimeoutFlag)
    {
        *ReceiveState = 0;
    } 
    return ret;
}

/*************************************************
 Function:        AutosarNmSdkGetNetManageReceiveStatus
 Description:     Get the receive status flag of the specified network management channel
 Input:           nmID - Network management channel identifier
                  ReceiveState - Pointer to store receive status
 Output:          ReceiveState - Returns receive status flag via pointer: 1=message received, 0=no message received
 Return:          NM_E_OK for successful operation, NM_E_NOT_OK if nmID exceeds range
 Others:          Used to check if NM messages have been received on the specified network management channel
 *************************************************/
int16_t AutosarNmSdkGetNetManageReceiveStatus(uint8_t nmID, uint8_t *ReceiveState)
{
    int16_t ret;
    ret = NM_E_OK;
    *ReceiveState = 0;
    if(g_netManage[nmID].nmMsgReceiveFlag)
    {
        *ReceiveState = 1;
    }
    return ret;
}

/*************************************************
 Function:        AutosarNmSdkGetNetManageReceiveStatusClear
 Description:     Clear the receive status flag of the specified network management channel
 Input:           nmID - Network management channel identifier
 Output:          None
 Return:          None
 Others:          Resets the NM message receive flag to 0
 *************************************************/
void AutosarNmSdkGetNetManageReceiveStatusClear(uint8_t nmID)
{ 
    g_netManage[nmID].nmMsgReceiveFlag = 0;
}

/*************************************************
 Function:        AutosarNmSdkGetNetManageCanChannel
 Description:     Get CAN channel information for specified network management instance
 Input:           nmID - Network management channel identifier
 Output:          channelFlag - Pointer to store CAN channel flag
 Return:          None
 Others:          Sets the corresponding bit in channelFlag based on the configured CAN channel
 *************************************************/
void AutosarNmSdkGetNetManageCanChannel(uint8_t nmID, uint8_t *channelFlag)
{
    if(g_pNmConfigure[nmID].canChannel < 8)
    {
        *channelFlag |= 0x01<<(g_pNmConfigure[nmID].canChannel);
    }
}
/*************************************************
 Function:        AutosarNmSdkGetBusState
 Description:     Get the overall bus state across all network management instances
 Input:           None
 Output:          None
 Return:          1 if any instance is in bus off state, 0 otherwise
 Others:          Iterates through all network management instances to check bus off events
 *************************************************/
uint8_t AutosarNmSdkGetBusState(void)
{
    uint8_t i = 0U;
    uint8_t retVal = 0U;
    for( ; i < g_netManageNumber; i++)
    {
        if(0x01 == g_netManage[i].busOffEventOn)
        {
            retVal = 1;
            break;
        }
    }
    return retVal;
}

/****************************** Private Function Implementations ***************/
/*************************************************
   Function:        AutosarNmNetworkRequest
   Description:     Request network management based on current network state
   Input:           nmChannelHandle - Network management channel handle
   Output:          None
   Return:          NM_E_OK on success, NM_E_NOT_OK if channel handle is invalid
   Others:          Sets network request flag and active wakeup flag based on network state
                    Handles various network states including bus sleep, prepare sleep, ready sleep, etc.
 *************************************************/
static int16_t AutosarNmNetworkRequest(const int16_t nmChannelHandle)
{
    int16_t retVal = NM_E_OK;
    if((nmChannelHandle  >= AUTOSAR_NET_MANAGE_MAX_INSTANCE) || (nmChannelHandle < 0))
    {
        retVal = NM_E_NOT_OK;
    }
    else
    {
        if(E_NETMANAGESTATE_BUSSLEEPMODE == g_netManage[nmChannelHandle].netManageState)
        {
            g_netManage[nmChannelHandle].networkRequestFlag = 1;
        }
        else if(E_NETMANAGESTATE_PREPAREBUSSLEEPMODE == g_netManage[nmChannelHandle].netManageState)
        {
            g_netManage[nmChannelHandle].networkRequestFlag = 1;
        }
        else if(E_NETMANAGESTATE_READYSLEEPSTATE == g_netManage[nmChannelHandle].netManageState)
        {
            g_netManage[nmChannelHandle].networkRequestFlag = 1;
        }
        else if(E_NETMANAGESTATE_STARTUPSTATE == g_netManage[nmChannelHandle].netManageState)
        {
            g_netManage[nmChannelHandle].networkRequestFlag = 1;
        }
        else if(E_NETMANAGESTATE_NORMALOPERATIONSTATE == g_netManage[nmChannelHandle].netManageState)
        {
            g_netManage[nmChannelHandle].networkRequestFlag = 1;
        }
    }
    return retVal;
}

/*************************************************
   Function:        AutosarNmPassiveStartUp
   Description:     Perform passive start-up for network management
   Input:           nmChannelHandle - Network management channel handle
   Output:          None
   Return:          NM_E_OK on success, NM_E_NOT_OK if channel handle is invalid
   Others:          Sets passive start-up flags when in bus sleep or prepare bus sleep state
                    Enables network communication without active wake-up signal
 *************************************************/
static int16_t AutosarNmPassiveStartUp( const int16_t nmChannelHandle )
{
    int16_t retVal = NM_E_OK;
	
	if(nmChannelHandle >= AUTOSAR_NET_MANAGE_MAX_INSTANCE)
	{
		retVal = NM_E_NOT_OK;
	}
	else
	{
		if(E_NETMANAGESTATE_BUSSLEEPMODE == g_netManage[nmChannelHandle].netManageState)
		{
			g_netManage[nmChannelHandle].passiveStartUpFlag = 1;
            
		}
		else if(E_NETMANAGESTATE_PREPAREBUSSLEEPMODE == g_netManage[nmChannelHandle].netManageState)
		{
			g_netManage[nmChannelHandle].passiveStartUpFlag = 1;
		}
	}
	return retVal;
}

/*************************************************
 Function:        StartNMTimer
 Description:     Start the network management timer for the specified index
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Uses the configured timeout time from the instance configuration
 *************************************************/
static void StartNMTimer(uint8_t index)
{
    TimerHalStartTime(g_netManage[index].NMTimerHandle, g_pNmConfigure[index].nmTimeOutTime);
}

/*************************************************
 Function:        StopNMTimer
 Description:     Stop the network management timer for the specified index
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Uses the configured timeout time from the instance configuration
 *************************************************/
// static void StopNMTimer(uint8_t index)
// {
//   TimerHalStopTime(g_netManage[index].NMTimerHandle);
// }

/*************************************************
 Function:        StartWaitBusSleepTimer
 Description:     Start the bus sleep waiting timer for the specified index
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Uses the configured wait time from the instance configuration
 *************************************************/
static void StartWaitBusSleepTimer(uint8_t index)
{
    TimerHalStartTime(g_netManage[index].BusSleepTimerHandle, g_pNmConfigure[index].waitBusSleepTime);
}

/*************************************************
 Function:        StopWaitBusSleepTimer
 Description:     Stop the bus sleep waiting timer for the specified index
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Halts the timer to prevent entering bus sleep mode
 *************************************************/
static void StopWaitBusSleepTimer(uint8_t index)
{
    TimerHalStopTime(g_netManage[index].BusSleepTimerHandle);
}

/*************************************************
 Function:        StartMsgCycleTimer
 Description:     Start the message cycle timer for the specified index
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Uses immediate cycle time if in immediate cycle state, otherwise uses regular message cycle time
 *************************************************/
static void StartMsgCycleTimer(uint8_t index)
{ 
    TimerHalStartTime(g_netManage[index].MsgCycleTimerHandle, g_pNmConfigure[index].msgCycleTime);
}

/*************************************************
 Function:        StopMsgCycleTimer
 Description:     Stop the message cycle timer for the specified index
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Halts the timer to stop message transmission cycles
 *************************************************/
static void StopMsgCycleTimer(uint8_t index)
{
    TimerHalStopTime(g_netManage[index].MsgCycleTimerHandle);
}

/*************************************************
 Function:        StartStartUpTimer
 Description:     Start the repeat message timer for the specified index
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Uses the configured repeat message time from the instance configuration
 *************************************************/
static void StartStartUpTimer(uint8_t index)
{
    TimerHalStartTime(g_netManage[index].StartUpTimerHandle, g_pNmConfigure[index].startUpTime);
}

/*************************************************
 Function:        StopStartUpTimer
 Description:     Stop the repeat message timer for the specified index
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Halts the timer to stop repeat message transmission
 *************************************************/
static void StopStartUpTimer(uint8_t index)
{
    TimerHalStopTime(g_netManage[index].StartUpTimerHandle);
}

/*************************************************
 Function:        CanReceiveMessageProcess
 Description:     Process received CAN messages for network management
 Input:           index - Index of the network management instance
                  canId - Received CAN message ID
                  canData - Pointer to received CAN message data
 Output:          None
 Return:          None
 Others:          Handles different network states based on received messages
                  Sets passive startup flags when in bus sleep or prepare bus sleep state
 *************************************************/
static void CanReceiveMessageProcess(uint8_t index, uint32_t canId, uint8_t *canData)
{
	g_netManage[index].nmMsgReceiveFlag = 1;  
	if(E_NETMANAGESTATE_BUSSLEEPMODE == g_netManage[index].netManageState)  
	{
		g_netManage[index].passiveStartUpFlag = 1;
	}
	else if(E_NETMANAGESTATE_PREPAREBUSSLEEPMODE == g_netManage[index].netManageState) 
	{
		g_netManage[index].passiveStartUpFlag = 1;
	}
	else if(E_NETMANAGESTATE_STARTUPSTATE == g_netManage[index].netManageState)
	{
		StartNMTimer(index);
	}  
    else if(E_NETMANAGESTATE_NORMALOPERATIONSTATE == g_netManage[index].netManageState)
    {
        StartNMTimer(index);
    }
    else if(E_NETMANAGESTATE_READYSLEEPSTATE == g_netManage[index].netManageState)
    {
        StartNMTimer(index);
    }
}

/*************************************************
 Function:        NetManageBusOffErrorCallBack
 Description:     Callback function for CAN bus off errors
 Input:           index - Index of the network management instance
                  flag - Error flag (0x00 for cleared, 0x01 for set)
 Output:          None
 Return:          None
 Others:          Calls the configured CAN bus error callback function if available
 *************************************************/
static void NetManageBusOffErrorCallBack(uint8_t index, uint8_t flag)
{
    if(g_pNmConfigure[index].canBusErrorCallBack != NULL)
    {
        g_pNmConfigure[index].canBusErrorCallBack(flag);
    }
}

/*************************************************
 Function:        MsgCycleTimeOutProcess
 Description:     Process message cycle timeout events
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Handles message transmission based on current network state
                  Manages immediate cycle state and count
                  Prepares and sends CAN messages according to AUTOSAR NM protocol
 *************************************************/
static void MsgCycleTimeOutProcess(uint8_t index)
{
	if((E_NETMANAGESTATE_BUSSLEEPMODE == g_netManage[index].netManageState)||
		(E_NETMANAGESTATE_PREPAREBUSSLEEPMODE == g_netManage[index].netManageState)||
		(E_NETMANAGESTATE_STARTUPSTATE == g_netManage[index].netManageState)||
		(E_NETMANAGESTATE_NORMALOPERATIONSTATE == g_netManage[index].netManageState)||
		(E_NETMANAGESTATE_READYSLEEPSTATE == g_netManage[index].netManageState))
	{
        int16_t     ret = 0U;
        uint32_t	canId;
        uint8_t 	canData[8];
        uint8_t 	CBV;
        uint8_t     wakeupSource;
        uint8_t     ignStatus = 0;
        uint8_t     canStatus = 0;

        StartMsgCycleTimer(index);
        StartNMTimer(index);
        ignStatus = PeripheralHalGetKl15Status();
        AutosarNmSdkGetCanStatus(index,&canStatus);

        if(g_netManage[index].passiveMode)
        {
            return;
        }
        canId = g_pNmConfigure[index].BaseAddress + g_pNmConfigure[index].NodeId; 
        memset(canData,NM_MESSAGE_DEFAULT_VALUE,sizeof(canData));

        //0:Source Node Identifier
        canData[0] = g_pNmConfigure[index].NodeId;  

        //1:Control Bit Vector
        CBV = NM_CBV_DEFAULT_VALUE;
        canData[1] = CBV;    //GAC reserve this byte 

        //2: Origin of the wake up 
        wakeupSource = PowerManageHalGetWakeupSource();
        if(ignStatus == NM_IGN_ON)                          //15 prio level 1
        {
            canData[2] = NM_IGN_ON_WAKEUP_VALUE;
        }
        else if(wakeupSource == PM_HAL_WAKEUP_SOURCE_MPU)
        {
            canData[2] = NM_TSP_MESSAGE_WAKEUP_VALUE;
        }
        else if(canStatus == 1)
        {
            canData[2] = NM_CAN_MESSAGE_WAKEUP_VALUE;
        }
        else if((wakeupSource != PM_HAL_WAKEUP_SOURCE_CAN2) && (wakeupSource != PM_HAL_WAKEUP_SOURCE_KL15))
        {
            canData[2] = wakeupSource;                      //for special situation find problem
        }

        //3:SubNet wake up request
        canData[3] = g_nmSubNetWakeupRequestValue;

        //4:Origin of keeping network awake,reserve
        canData[4] = ignStatus;

        //7.WUP: fixed to the value 0x4C
        canData[7] = NM_WUP_VALUE;

        ret = CanHalTransmitQueued(g_netManage[index].canHandle, canId, canData, NM_MESSAGE_PDU_LENGTH, NM_TRANS_CANFD_ENABLE, CAN_TX_PRIO_NORMAL);
        if(ret == 0)
        {
            g_netManage[index].busOffTimeCount = 0;
            //NetManageBusOffErrorCallBack(index,0x00);
        }
        else
        {
            TBOX_PRINT("net Manage CanDriverHalTransmit error,error code = %d\r\n",ret);
        }
	} 
}

#if(0)
/*************************************************
 Function:        ApplicationMessageDisableTransmit
 Description:     Disable application message transmission
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Calls HAL function to disable application messages and updates flag
 *************************************************/
static void ApplicationMessageDisableTransmit(uint8_t index)
{
    if(1 == g_netManage[index].appEnableFlag)
    {
        CanHalAppMsgDisable(g_netManage[index].canHandle);
        g_netManage[index].appEnableFlag = 0;
    }    
}

/*************************************************
 Function:        ApplicationMessageEnableTransmit
 Description:     Enable application message transmission
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Calls HAL function to enable application messages and updates flag
 *************************************************/
static void ApplicationMessageEnableTransmit(uint8_t index)
{
    if(0 == g_netManage[index].appEnableFlag)
    {
        CanHalAppMsgEnable(g_netManage[index].canHandle);
        g_netManage[index].appEnableFlag = 1;
    }    
}
#endif
/*************************************************
 Function:        NetManageStateBussLeepMode
 Description:     Handle network management in bus sleep mode
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Transitions to repeat message state when network is requested
                  Handles both active and passive wake-up scenarios
                  Enables application message transmission
 *************************************************/
static void NetManageStateBussLeepMode(uint8_t index)
{
    if(g_netManage[index].networkRequestFlag)    
    {
        TBOX_PRINT("Nm local wake\r\n");
        g_netManage[index].netManageState = E_NETMANAGESTATE_STARTUPSTATE;                    
        StopWaitBusSleepTimer(index);
        MsgCycleTimeOutProcess(index);
        StartStartUpTimer(index);	
        CanPeriodSendEnable(TBOX_CAN_CHANNEL_2);//only use can1
#if(SECOC_ENABLE == 1)
        SecocSdkWakeupResetState();
#endif
    }
    else if(g_netManage[index].passiveStartUpFlag)   
    {
        TBOX_PRINT("Nm can wake\r\n");
        g_netManage[index].netManageState = E_NETMANAGESTATE_STARTUPSTATE; 
        g_netManage[index].passiveStartUpFlag = 0;                     
        StopWaitBusSleepTimer(index);
        MsgCycleTimeOutProcess(index);
        StartStartUpTimer(index);
        CanPeriodSendEnable(TBOX_CAN_CHANNEL_2);
#if(SECOC_ENABLE == 1)
        SecocSdkWakeupResetState();
#endif
    }
}

/*************************************************
 Function:        NetManageStateStartUp
 Description:     Handle network management in repeat message state
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Transitions to normal operation state if network is requested
                  Transitions to ready sleep state if network is released
                  Restarts NM timer as needed
 *************************************************/
static void NetManageStateStartUp(uint8_t index)
{
    if((g_netManage[index].networkRequestFlag == 1) && (TimerHalIsTimeout(g_netManage[index].StartUpTimerHandle) == 0))  
    {   
        TBOX_PRINT("Nm SUM to NOM!\r\n");
        StopStartUpTimer(index);
        g_netManage[index].netManageState = E_NETMANAGESTATE_NORMALOPERATIONSTATE;   
    }
    else if((g_netManage[index].networkRequestFlag == 0) && (TimerHalIsTimeout(g_netManage[index].StartUpTimerHandle) == 0))   
    {
        TBOX_PRINT("Nm SUM to RSM!\r\n");
        StopStartUpTimer(index);
        g_netManage[index].netManageState = E_NETMANAGESTATE_READYSLEEPSTATE; 
    }
}

/*************************************************
 Function:        NetManageStateReadySleep
 Description:     Handle network management in ready sleep state
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Handles repeat message requests and bit received flags
                  Transitions back to normal operation state if network is requested
                  Transitions to prepare bus sleep mode when NM timer expires
 *************************************************/
static void NetManageStateReadySleep(uint8_t index)
{
    StopMsgCycleTimer(index);
    StopStartUpTimer(index);	
	if(g_netManage[index].networkRequestFlag == 1)    
	{
		StartNMTimer(index);
		StartMsgCycleTimer(index); 
        CanPeriodSendEnable(TBOX_CAN_CHANNEL_2);;
        TBOX_PRINT("Nm RSM to NOM!\r\n");
		g_netManage[index].netManageState = E_NETMANAGESTATE_NORMALOPERATIONSTATE; 
	}
	if(TimerHalIsTimeout(g_netManage[index].NMTimerHandle) == 0)  
	{
		StartWaitBusSleepTimer(index);
		CanPeriodSendDisable(TBOX_CAN_CHANNEL_2);
        TBOX_PRINT("Nm RSM to PBM!\r\n");
		g_netManage[index].netManageState = E_NETMANAGESTATE_PREPAREBUSSLEEPMODE;
	}
}

/*************************************************
 Function:        NetManageStateNomalOperation
 Description:     Handle network management in normal operation state
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Handles repeat message requests and bit received flags
                  Transitions to ready sleep state when network is released
                  Restarts NM timer as needed
 *************************************************/
static void NetManageStateNomalOperation(uint8_t index)
{
    if(g_netManage[index].networkRequestFlag == 0)   
    {
        TBOX_PRINT("Nm NOM to RSM!\r\n");
        g_netManage[index].netManageState = E_NETMANAGESTATE_READYSLEEPSTATE;     
    }
    if(TimerHalIsTimeout(g_netManage[index].NMTimerHandle) == 0)   //Condition 6 ,T_NM_TIMEROUT��ʱ
    {
        StartNMTimer(index);
    }
}

/*************************************************
 Function:        NetManageStatePrepareBusSleep
 Description:     Handle network management in prepare bus sleep mode
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Transitions back to repeat message state if network is requested or NM PDU is received
                  Transitions to bus sleep mode when wait bus sleep timer expires
 *************************************************/
static void NetManageStatePrepareBusSleep(uint8_t index)
{
    if(g_netManage[index].networkRequestFlag)    
    {
        TBOX_PRINT("Nm PBM local wake\r\n");
        g_netManage[index].netManageState = E_NETMANAGESTATE_STARTUPSTATE; 
        StopWaitBusSleepTimer(index);
        StartStartUpTimer(index);
        MsgCycleTimeOutProcess(index); 
        CanPeriodSendEnable(TBOX_CAN_CHANNEL_2);
    }
    else if(g_netManage[index].passiveStartUpFlag)   
    {
        TBOX_PRINT("Nm PBM can wake\r\n");
        g_netManage[index].passiveStartUpFlag = 0;
        g_netManage[index].netManageState = E_NETMANAGESTATE_STARTUPSTATE; 
        StopWaitBusSleepTimer(index);
        StartStartUpTimer(index);
        MsgCycleTimeOutProcess(index);
        CanPeriodSendEnable(TBOX_CAN_CHANNEL_2);
    }
    else if(TimerHalIsTimeout(g_netManage[index].BusSleepTimerHandle) == 0)   
    {
        TBOX_PRINT("Nm PBM to BSM!\r\n");
        StopWaitBusSleepTimer(index);
        g_netManage[index].netManageState = E_NETMANAGESTATE_BUSSLEEPMODE;
    }
}

/*************************************************
 Function:        NetManageStateProcess
 Description:     Main state machine processing for network management
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Routes to specific state handlers based on current network management state
                  Initializes to bus sleep mode if in idle mode
 *************************************************/
static void NetManageStateProcess(uint8_t index)
{
    if(E_NETMANAGESTATE_IDLEMODE == g_netManage[index].netManageState)
    {
        g_netManage[index].netManageState = E_NETMANAGESTATE_BUSSLEEPMODE;   
    }

    if(E_NETMANAGESTATE_BUSSLEEPMODE == g_netManage[index].netManageState)
    {
        NetManageStateBussLeepMode(index);
    }

	if(E_NETMANAGESTATE_STARTUPSTATE == g_netManage[index].netManageState)  
	{
        NetManageStateStartUp(index);
	}
	
	if(E_NETMANAGESTATE_READYSLEEPSTATE == g_netManage[index].netManageState) 
	{
		NetManageStateReadySleep(index);
	}
	
	if(E_NETMANAGESTATE_NORMALOPERATIONSTATE == g_netManage[index].netManageState)     
	{
        NetManageStateNomalOperation(index);
	}
	
	if(E_NETMANAGESTATE_PREPAREBUSSLEEPMODE == g_netManage[index].netManageState)    
	{
        NetManageStatePrepareBusSleep(index);
    }
}

/*************************************************
 Function:        NetManageInializeHardware
 Description:     Initialize network management hardware
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Resets CAN hardware using HAL functions
                  Includes a short delay between resets
 *************************************************/
static void NetManageInializeHardware(uint8_t index)
{
    CanHalResetHardware(g_netManage[index].canHandle);
    vTaskDelay( pdMS_TO_TICKS(5));
    CanHalResetHardware(g_netManage[index].canHandle);
}

/*************************************************
 Function:        CanHardwareControllerProcess
 Description:     Process CAN hardware controller events
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Handles CAN bus off errors with configurable recovery strategies
                  Manages error event counting and DTC reporting
                  Supports both quick and slow recovery timing
 *************************************************/
static void CanHardwareControllerProcess(uint8_t index)
{
    int16_t ret;
    ret = CanHalGetIsBusOffError(g_netManage[index].canHandle);
    if(ret == 0)
    {
        //TBOX_PRINT("busoff count %d\r\n",g_netManage[index].busOffTimeCount);
        if(g_netManage[index].busOffTimerOnFlag)
        {
            //do nothing
        }
        else
        {
            if(g_pNmConfigure[index].busOffQuickTimes != 0)
            {         
                if(g_netManage[index].busOffTimeCount >= (g_pNmConfigure[index].busOffQuickTimes - 1))
                {
                    if(g_pNmConfigure[index].busOffSlowTime != 0)
                    {
                        TimerHalStartTime(g_netManage[index].busOffRecoverTimerHandle, g_pNmConfigure[index].busOffSlowTime);
                        g_netManage[index].busOffTimerOnFlag = 1;
                    }
                    else
                    {
                        NetManageInializeHardware(index);
                    }
                }
                else
                {
                    if(g_pNmConfigure[index].busOffQuickTime != 0)
                    {
                        // start busoff recover quick timer
                        TimerHalStartTime(g_netManage[index].busOffRecoverTimerHandle, g_pNmConfigure[index].busOffQuickTime);
                        g_netManage[index].busOffTimerOnFlag = 1;
                        g_netManage[index].busOffTimeCount++;
                    }
                    else
                    {
                        NetManageInializeHardware(index);
                    }
                }
            }
            else//no quick time state
            {
                if(g_pNmConfigure[index].busOffSlowTime != 0)
                {
                    //start busoff recover slow timer
                    TimerHalStartTime(g_netManage[index].busOffRecoverTimerHandle,g_pNmConfigure[index].busOffSlowTime);
                    g_netManage[index].busOffTimerOnFlag = 1;
                    if(g_netManage[index].busOffTimeCount < g_pNmConfigure[index].busOffErrorEventLimitCount)
                    {
                        g_netManage[index].busOffTimeCount++;
                    }
                    //NM_MINA_DEBUG_PRINT("busoff count= %d\r\n",g_netManage[index].busOffTimeCount);
                }
                else
                {
                    NetManageInializeHardware(index);
                    g_netManage[index].busOffTimeCount++;       //busoff error count
                    //RTOS_HalApiWait(5);
                }
            }
        }
    }
    else
    {
        CanHalClearBusoffAppDisableFlag(g_netManage[index].canHandle);
    }
    if(g_netManage[index].busOffTimeCount >= g_pNmConfigure[index].busOffErrorEventLimitCount)
    {
        if(0x00 == g_netManage[index].busOffEventOn)
        {
            g_netManage[index].busOffEventOn = 0x01;
            NetManageBusOffErrorCallBack(index, 0x01);  
            //SetCanState(1);
            TBOX_PRINT("set busoff DTC\n");
        }
    }
    else
    {
        if(0x01 == g_netManage[index].busOffEventOn)
        {
            g_netManage[index].busOffEventOn = 0x00;
            NetManageBusOffErrorCallBack(index, 0x00);         
            //SetCanState(0);
            TBOX_PRINT("clear busoff DTC\n");
        }
    }
}
#if(NM_BUS_OFF_ERROR_RESET_MSG_ENABLE) == 1
/*************************************************
 Function:        BusOffErrorResetCanNMTransmit
 Description:     Reset CAN NM transmission after bus off error
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Starts message cycle timer and prepares/sends a CAN NM message
                  Follows AUTOSAR NM protocol for message formatting
                  Handles passive mode checks
 *************************************************/
static void BusOffErrorResetCanNMTransmit(uint8_t index)
{
    if((E_NETMANAGESTATE_STARTUPSTATE == g_netManage[index].netManageState)||
		(E_NETMANAGESTATE_NORMALOPERATIONSTATE == g_netManage[index].netManageState))
    {
        uint32_t	canId;
        uint8_t 	CBV;
        int16_t     ret = 0U;
        uint8_t 	canData[8];
        uint8_t     ignStatus = 0;
        uint8_t     canStatus = 0;
        uint8_t     wakeupSource;

        StartMsgCycleTimer(index);
        ignStatus = PeripheralHalGetKl15Status();
        AutosarNmSdkGetCanStatus(index,&canStatus);
        if(g_netManage[index].passiveMode)
        {
            return;
        }
        memset(canData,NM_MESSAGE_DEFAULT_VALUE,sizeof(canData));
        canId = g_pNmConfigure[index].BaseAddress + g_pNmConfigure[index].NodeId; 

        //0:Source Node Identifier
        canData[0] = g_pNmConfigure[index].NodeId;     

        //1:Control Bit Vector
        CBV = NM_CBV_DEFAULT_VALUE;
        canData[1] = CBV;    //GAC reserve this byte 

        //2: Origin of the wake up 
        wakeupSource = PowerManageGetLastWakeupSource();
        if(ignStatus == NM_IGN_ON)                          //15 prio level 1
        {
            canData[2] = NM_IGN_ON_WAKEUP_VALUE;
        }
        else if(wakeupSource == PM_HAL_WAKEUP_SOURCE_MPU)
        {
            canData[2] = NM_TSP_MESSAGE_WAKEUP_VALUE;
        }
        else if(canStatus == 1)
        {
            canData[2] = NM_CAN_MESSAGE_WAKEUP_VALUE;
        }
        else if((wakeupSource != PM_HAL_WAKEUP_SOURCE_CAN2) && (wakeupSource != PM_HAL_WAKEUP_SOURCE_KL15))
        {
            canData[2] = wakeupSource;                      //for special situation find problem
        }

        //3:SubNet wake up request
        canData[3] = g_nmSubNetWakeupRequestValue;

        //4:Origin of keeping network awake,reserve
        canData[4] = ignStatus;

        //7.WUP: fixed to the value 0x4C
        canData[7] = NM_WUP_VALUE;

        ret = CanHalTransmitQueued(g_netManage[index].canHandle, canId, canData, NM_MESSAGE_PDU_LENGTH, NM_TRANS_CANFD_ENABLE, CAN_TX_PRIO_NORMAL);
        if(ret == 0)
        {
            g_netManage[index].busOffTimeCount = 0;
            //NetManageBusOffErrorCallBack(index,0x00);
        }
        else
        {
            TBOX_PRINT("net Manage CanDriverHalTransmit error,error code = %d\r\n",ret);
        }
    }
}
#endif
/*************************************************
 Function:        NetManageTimerProcess
 Description:     Process network management timers for message cycles and bus off recovery
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Handles message cycle timer timeouts by calling MsgCycleTimeOutProcess
                  Manages bus off recovery by resetting hardware and restarting NM transmission
 *************************************************/
static void NetManageTimerProcess(uint8_t index)
{
	if(TimerHalIsTimeout(g_netManage[index].MsgCycleTimerHandle) == 0)
	{
        MsgCycleTimeOutProcess(index);
	}

	if(TimerHalIsTimeout(g_netManage[index].busOffRecoverTimerHandle) == 0)
	{
        TimerHalStopTime(g_netManage[index].busOffRecoverTimerHandle);
        CanHalResetHardware(g_netManage[index].canHandle);
        g_netManage[index].busOffTimerOnFlag = 0;
#if(NM_BUS_OFF_ERROR_RESET_MSG_ENABLE) == 1
        BusOffErrorResetCanNMTransmit(index);     
#endif
	}
}

/*************************************************
 Function:        NetManageAllCanReceiveCheck
 Description:     Check CAN message reception status for network management
 Input:           index - Index of the network management instance
 Output:          None
 Return:          None
 Others:          Monitors CAN message reception timeout
                  Sets checkTimeoutFlag when no messages received within threshold
                  Resets timeout flag and counter when messages are received
 *************************************************/
static void NetManageAllCanReceiveCheck(uint8_t index)
{
    if(CanHalRecieveIsTimeOut(g_netManage[index].receiveCheckCanHandle) == 0)
    {
        if(g_netManage[index].checkTimeCount >= (200/5))
        {
            g_netManage[index].checkTimeoutFlag = 1;
        }
        else
        {
            g_netManage[index].checkTimeCount++;      
        }
    }
    else
    {
        g_netManage[index].checkTimeoutFlag = 0;
        g_netManage[index].checkTimeCount = 0;  
    }  
}

/*************************************************
 Function:        NetManageStateCyclePrint
 Description:     Print network management state information for debugging
 Input:           nmId - Network management instance ID
                  nmState - Current network management state
                  requestFlag - Network request flag status
 Output:          None
 Return:          None
 Others:          Outputs debug information via TBOX_PRINT function
                  Displays instance ID, state, request flag, and timer handle
 *************************************************/
static void NetManageStateCyclePrint(uint8_t nmId, uint8_t nmState, uint8_t requestFlag) 
{
    TBOX_PRINT("net manage %d---state:%d,request:%d,handle:%d\r\n", nmId, nmState, requestFlag, g_netManage[nmId].StartUpTimerHandle);
}

/*************************************************
 Function:        AutosarNmSdkSetSubNetWakeupRequest
 Description:     Set CAN sub-network wakeup request configuration
 Input:           wakeupRequestFlags - Bitmask where each bit represents a sub-network wakeup request:
                   - Bit 0: PCAN sub-network
                   - Bit 1: ECAN sub-network
                   - Bit 2: BCAN sub-network
                   - Bit 3: ACAN sub-network
                   - Bit 4: ICAN sub-network
                   - Bit 5: TCAN sub-network
                   - Bit 6: LCAN sub-network
                   - Bit 7: Reserved (must be 0)
 Output:          None
 Return:          0 - Success, -1 - Invalid parameter (when bit 7 is set)
 Others:          This function is used to configure which sub-networks need to be woken up
 *************************************************/
int16_t AutosarNmSdkSetSubNetWakeupRequest(uint8_t wakeupRequestFlags)
{
    int16_t result = 0;
    
    /* Check if any reserved bits are set (only bits 0-6 are allowed) */
    if (wakeupRequestFlags & 0x80U)
    {
        result = -1;
    }
    else
    {
        /* Directly use the bitmask as the wakeup request value */
        g_nmSubNetWakeupRequestValue = wakeupRequestFlags;
    }
    
    return result;
}

/*************************************************
 Function:        AutosarNmSdkClearSubNetWakeupRequest
 Description:     Reset CAN sub-network wakeup request configuration to default value (only wake up TCAN)
 Input:           None
 Output:          None
 Return:          0 - Success
 Others:          This function resets the sub-network wakeup request to the default state, which only wakes up TCAN sub-network
 *************************************************/
int16_t AutosarNmSdkClearSubNetWakeupRequest(void)
{
    int16_t result = 0;
    
    g_nmSubNetWakeupRequestValue = NM_SUB_NET_WAKEUP_REQ_DEFAULT_VALUE;
    
    return result;
}

