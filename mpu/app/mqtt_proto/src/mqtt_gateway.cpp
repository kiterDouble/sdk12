#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <queue>
#include <atomic>
#include "err_code.h"
#include "sr_log.h"
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ota_api.h"
#include "common_functions.h"
#include "mqtt_evet_handle.h"
#include "net_service_api.h"
#include "hal_modem_sim.h"
#include "modem_ipc_msg.h"
#include "modem_api.h"
#include "mqtt_connect.h"
#include "mcu_api.h"
#include "mqtt_get_modem_info.h"

typedef enum
{
    SWITCH_EVENT_NONE = 0,
    SWITCH_EVENT_POWER_UP = 1,
    SWITCH_EVENT_POWER_DOWN = 2,
    SWITCH_EVENT_RESET = 3
} SwitchEventType_e;

typedef struct
{
    SwitchEventType_e eventType;
} SwitchEvent_t;

static std::queue<SwitchEvent_t> g_switchEventQueue;
static pthread_mutex_t g_eventLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_eventCond = PTHREAD_COND_INITIALIZER;
static pthread_t g_switchEventHandlerThreadId = 0;
static std::atomic_uint8_t g_switchSta;/* 0 init  1: up 2:down */


static pthread_mutex_t g_controlGatewayLock = PTHREAD_MUTEX_INITIALIZER;
static uint8_t g_controlGatewayBuff[4] = {0};

/*************************************************
Function:    ProcessSwitchPowerUpEvent
Description: Process switch power up event
Input:       None
Output:      None
Return:      None
Others:      None
*************************************************/
static void ProcessSwitchPowerUpEvent(void)
{
    if (2 != g_switchSta.load())
    {
        SrGetSystemRetValue("echo  1 > /sys/class/net/eth0/rx_clock_rdy");
        //SrGetSystemRetValue("ifconfig eth0 down");
        //usleep(100000);
        //SrGetSystemRetValue("ifconfig eth0 up");

        LOG_Debug("%s power up switch and enable eth0 ", __FUNCTION__);
    }
}

/*************************************************
Function:    ProcessSwitchPowerDownEvent
Description: Process switch power down event
Input:       None
Output:      None
Return:      None
Others:      None
*************************************************/
static void ProcessSwitchPowerDownEvent(void)
{
    SrGetSystemRetValue("echo  0 > /sys/class/net/eth0/rx_clock_rdy");
    g_switchSta = 1;

    LOG_Debug("%s power down switch and disable eth0 ", __FUNCTION__);
}

/*************************************************
Function:    ProcessSwitchResetEvent
Description: Process switch reset event
Input:       None
Output:      None
Return:      None
Others:      None
*************************************************/
static void ProcessSwitchResetEvent(void)
{
    SrGetSystemRetValue("echo  0 > /sys/class/net/eth0/rx_clock_rdy");
    g_switchSta = 1;

    LOG_Debug("%s reset switch and restart eth0 ", __FUNCTION__);
}

int32_t ClearSwitchUpStatus()
{
    g_switchSta = 1;
    LOG_Debug("pm is sleep and reset upstatus %s", __FUNCTION__);
    return 0;
}

/*************************************************
Function:    ReadDevmemValue
Description: Execute devmem command and read hex value
Input:       addr - memory address string
Output:      pValue - pointer to store the read value
Return:      0 on success, -1 on failure
Others:      Uses popen to capture command output
*************************************************/
static int32_t ReadDevmemValue(const char *addr, uint32_t *pValue)
{
    char cmdBuff[128] = {0};
    char outputBuff[64] = {0};
    FILE *fileFp = NULL;
    int32_t retValue = -1;

    if ((NULL == addr) || (NULL == pValue))
    {
        LOG_Error("ReadDevmemValue: invalid params");
        return -1;
    }

    snprintf(cmdBuff, sizeof(cmdBuff), "devmem %s", addr);

    fileFp = popen(cmdBuff, "r");
    if (NULL == fileFp)
    {
        LOG_Error("ReadDevmemValue: popen failed: %s", strerror(errno));
        return -1;
    }

    if (NULL != fgets(outputBuff, sizeof(outputBuff), fileFp))
    {
        outputBuff[strcspn(outputBuff, "\r\n")] = '\0';
        *pValue = (uint32_t)strtoul(outputBuff, NULL, 16);
        retValue = 0;
        LOG_Debug("ReadDevmemValue: addr=%s, value=0x%08X", addr, *pValue);
    }
    else
    {
        LOG_Error("ReadDevmemValue: fgets failed");
        retValue = -1;
    }

    pclose(fileFp);
    return retValue;
}

/*************************************************
Function:    CheckSwitchStatusIsError
Description: Check device status 
Input:       None
Output:      None
Return:      1 is error, 0 is OK, -1 is error
Others:      Checks 0x23058004
*************************************************/
int32_t CheckSwitchStatusIsError()
{
    uint32_t value1 = 0U;
    uint32_t value2 = 0U;
    int32_t retValue = -1;

    /* Read status twice, check if both are 0x00000109 (abnormal) */
    retValue = ReadDevmemValue("0x23058004", &value1);
    if (0 != retValue)
    {
        LOG_Error("CheckAndResetDeviceStatus: first read failed");
        return -1;
    }

    retValue = ReadDevmemValue("0x23058004", &value2);
    if (0 != retValue)
    {
        LOG_Error("CheckAndResetDeviceStatus: second read failed");
        return -1;
    }  
    return ((0x00000109U == value1) & (0x00000109U == value2));
}
/*************************************************
Function:    CheckAndResetDeviceStatus
Description: Check device status and perform reset if abnormal
Input:       None
Output:      None
Return:      0 on success (normal), -1 on failure
Others:      Checks 0x23058004, resets via 0xFFAC08 if abnormal
*************************************************/
int32_t CheckAndResetDeviceStatus(void)
{
    int32_t retValue = -1;

    /* Step 1: Read status twice, check if both are 0x00000109 (abnormal) */
    retValue = CheckSwitchStatusIsError();
    if(retValue == 0)
    {
        LOG_Debug("CheckAndResetDeviceStatus: status is normal \n");
        retValue =  0;
    }
    else
    {
        /* Step 2: Perform reset sequence*/
        LOG_Warn("CheckAndResetDeviceStatus: status is error and Reset");
        ProcessSwitchResetEvent();
        usleep(10000);
        ProcessSwitchPowerUpEvent();
    }
    return retValue;
}


/*************************************************
Function:    McuSwitchEventHandlerThread
Description: Worker thread that processes switch events from queue
Input:       pArg - thread argument (unused)
Output:      None
Return:      NULL
Others:      Blocks on queue wait, processes events sequentially
*************************************************/
void *McuSwitchEventHandlerThread(void *pArg)
{
    SwitchEvent_t event = {SWITCH_EVENT_NONE};

    LOG_Debug("McuSwitchEventHandlerThread: started");
    /* check switch 3 times after system boot */
    for(int i = 0;i < 3 ;i++)
    {
        if(CheckAndResetDeviceStatus() == 0)
        {
            break;
        }
        /* 检查失败时等待100ms再重试，避免频繁调用 devmem */
        if (i < 2)  /* 最后一次不需要等待 */
        {
            usleep(100000);
        }
    }

    while (1)
    {
        pthread_mutex_lock(&g_eventLock);

        while (g_switchEventQueue.empty())
        {
            pthread_cond_wait(&g_eventCond, &g_eventLock);
        }

        event = g_switchEventQueue.front();
        g_switchEventQueue.pop();
        pthread_mutex_unlock(&g_eventLock);

        switch (event.eventType)
        {
            case SWITCH_EVENT_POWER_UP:
                /* 优化：如果已经是上电完成状态，跳过所有操作
                 * 避免每秒周期性事件导致的频繁调用
                 */
                if (g_switchSta.load() == 2)
                {
                    LOG_Debug("SWITCH_EVENT_POWER_UP: already powered up, skip");
                    break;
                }
                
                ProcessSwitchPowerUpEvent();
                for(int i = 0; i < 3; i++)
                {
                    if(CheckAndResetDeviceStatus() == 0)
                    {
                        g_switchSta = 2;
                        break;
                    }
                    /* 检查失败时等待100ms再重试，避免频繁调用 devmem */
                    if (i < 2)  /* 最后一次不需要等待 */
                    {
                        usleep(100000);
                    }
                }
                break;
            case SWITCH_EVENT_POWER_DOWN:
                ProcessSwitchPowerDownEvent();
                break;
            case SWITCH_EVENT_RESET:
                ProcessSwitchResetEvent();
                break;
            default:
                LOG_Error("McuSwitchEventHandlerThread: unknown event type %d", event.eventType);
                break;
        }
    }

    return NULL;
}

/*************************************************
Function:    InitMcuSwitchEventHandler
Description: Initialize the switch event handler thread
Input:       None
Output:      None
Return:      0 on success, -1 on failure
Others:      Creates the worker thread
*************************************************/
int32_t InitMcuSwitchEventHandler(void)
{
    int32_t retValue = 0;

    if (0 != g_switchEventHandlerThreadId)
    {
        LOG_Debug("InitMcuSwitchEventHandler: already initialized");
        return 0;
    }

    retValue = pthread_create(&g_switchEventHandlerThreadId, NULL, McuSwitchEventHandlerThread, NULL);
    if (0 != retValue)
    {
        LOG_Error("InitMcuSwitchEventHandler: pthread_create failed: %s", strerror(errno));
        return -1;
    }

    LOG_Debug("InitMcuSwitchEventHandler: thread created successfully");
    return 0;
}

/*************************************************
Function:    EnqueueSwitchEvent
Description: Add event to queue and signal condition variable
Input:       eventType - type of event
Output:      None
Return:      0 on success, -1 on failure
Others:      Thread-safe, wakes up handler thread
*************************************************/
static int32_t EnqueueSwitchEvent(SwitchEventType_e eventType)
{
    SwitchEvent_t event = {SWITCH_EVENT_NONE};

    event.eventType = eventType;

    pthread_mutex_lock(&g_eventLock);
    g_switchEventQueue.push(event);
    pthread_cond_signal(&g_eventCond);
    pthread_mutex_unlock(&g_eventLock);

    LOG_Debug("EnqueueSwitchEvent: event %d enqueued", eventType);
    return 0;
}

void McuControlMpuGatewayCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{

    if (NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    LOG_DebugHex(recvData, dataLen, "gateway_debug MCU Data");
    pthread_mutex_lock(&g_controlGatewayLock);
    memcpy(g_controlGatewayBuff, recvData, 4);
    pthread_mutex_unlock(&g_controlGatewayLock);
}

uint8_t GetGatewayLocation()
{
  return g_controlGatewayBuff[1];
}

void McuSyncSwitchPowerUpCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    if (NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if (4 != dataLen)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if (0 != EnqueueSwitchEvent(SWITCH_EVENT_POWER_UP))
    {
        LOG_Error("%s failed to enqueue event", __FUNCTION__);
    }
}

void McuSyncSwitchPowerDownCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    if (NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if (4 != dataLen)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if (0 != EnqueueSwitchEvent(SWITCH_EVENT_POWER_DOWN))
    {
        LOG_Error("%s failed to enqueue event", __FUNCTION__);
    }
}

void McuSyncSwitchResetCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    if (NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if (4 != dataLen)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if (0 != EnqueueSwitchEvent(SWITCH_EVENT_RESET))
    {
        LOG_Error("%s failed to enqueue event", __FUNCTION__);
    }
}

int32_t SendControlGatewayResData(const uint8_t *recvData, const uint16_t dataLen)
{
    McuMesg_t sendToMcu;

    if ((NULL == recvData) || (dataLen <= 0))
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    sendToMcu.type = (SYSQueMsgType_e)(SYS_QUE_TYPE_MCU_VEHICLE_CTROL_GATEWAY_RES);
    sendToMcu.payloadLen = dataLen;
    sendToMcu.payload = (uint8_t *)recvData;

    LOG_Debug("sendToMcu.type : %8x", sendToMcu.type);
    return McuSrvSendMesg(&sendToMcu);
}

void *ControlGatewayThread(void *pArg)
{
    int32_t ret = -1;
    uint8_t module_index = 0;
    uint8_t net_switch = 0;
    bool mmc_flag = false;
    bool gateway_flag = false;

    if(!access("/dev/mmcblk0", F_OK))
    {
      mmc_flag = true;
    }

    while (1)
    {
        pthread_mutex_lock(&g_controlGatewayLock);
        module_index = g_controlGatewayBuff[0];
        net_switch = g_controlGatewayBuff[1];
        pthread_mutex_unlock(&g_controlGatewayLock);
        
        //检测数据有效性
        if(module_index > 1 || net_switch > 2 || net_switch == 0)
        {
          LOG_Error("%s gateway_debug invalid value from mcu, module_index: %d, net_switch: %d", __FUNCTION__, module_index, net_switch);
          sleep(1);
          continue;
        }
  
        //检查数据是否发错模组
        if(mmc_flag == module_index)
        {
          LOG_Error("%s gateway_debug delivered to wrong module");
          sleep(1);
          continue;
        }

        //模组与待配置网关模组一致则设置网关
        if((net_switch - 1) == module_index)
        {
            //如果当前已设置网关，跳过设置命令
            if(gateway_flag)
            {
              sleep(1);
              continue;
            }

            ret = SrGetSystemRetValue("ifconfig eth0:1 192.168.1.1 netmask 255.255.255.0 broadcast 192.168.1.255 up");
            if (0 != ret)
            {
                LOG_Error("%s gateway_debug, switch gateway %d failed", __FUNCTION__, module_index);
            }
            gateway_flag = true;

            SrGetSystemRetValue("timeout  10 arping -U -I eth0:1 -c 3 192.168.1.1 ");
        }
        //模组与待设置模组不一致，则取消当前网关地址
        else
        {            
            //如果当前已处于取消网关状态，跳过设置操作
            if(!gateway_flag){
              sleep(1);
              continue;
            }

            ret = SrGetSystemRetValue("ifconfig eth0:1 down");
            if (0 != ret)
            {
                LOG_Error("%s gateway_debug, deactivate gateway %d failed", __FUNCTION__, module_index);
            }
            gateway_flag = false;
            
            if (0 != module_index)
            {
                SrGetSystemRetValue("timeout  10 arping -U -I bridge2 -c 3 192.168.1.86 ");
            }
            else
            {
                SrGetSystemRetValue("timeout  10 arping -U -I bridge2 -c 3 192.168.1.88 ");
            }
        }

        sleep(1);
    }
}
