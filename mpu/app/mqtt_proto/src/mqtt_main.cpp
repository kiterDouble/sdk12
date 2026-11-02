#include "mqtt_common.h"
#include "mqtt_log_coord.h"
#include "mqtt_connect.h"
#include "mqtt_dispatch.h"
#include "task_exc_handle.h"
#include "mqtt_evet_handle.h"
#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>
#include "mqtt_get_modem_info.h"
#include "mqtt_gateway.h"
#include "task_exc_handle.h"
#include "mqtt_connect.h"
#include "mqtt_srlog_upload.h"
#include "mqtt_log_sync.h"
#include "mqtt_get_mcu_version.h"
#include "mqtt_qos.h"
#include "mqtt_emmc_ro.h"
#include <sys/resource.h>

typedef void *(threadFun)(void *);

#define SEND_DIAGNOSIS_HEARTBEAT (9)

// Set process to high priority by adjusting nice value
int set_process_high_priority(int nice_value) {
    // Core call: adjust process nice value using setpriority
    int ret = setpriority(PRIO_PROCESS, 0, nice_value);
    if (ret == -1) {
        LOG_Debug("Failed to set process high priority: %s\n");
        LOG_Debug("Hint: need to run with sudo (nice value < 0 requires root permission)\n");
        return -1;
    }
    LOG_Debug("Process priority set to nice=%d (higher priority)\n", nice_value);
    return 0;
}


static int32_t MQTTCreateThread(pthread_t *threadId, threadFun fun, void *threadArg,uint8_t priority)
{
    pthread_attr_t attr;
    struct sched_param param;
    int32_t retValue = -1;

    if(NULL == threadId) {
        LOG_Error("%s argv threadId is NULL", __FUNCTION__);
        return RET_ERR;
    }

    pthread_attr_init(&attr);
    
    // 尝试设置实时调度策略（失败时回退到默认）
    if (priority >= 1 && priority <= 99 && pthread_attr_setschedpolicy(&attr, SCHED_RR) == 0 ) {
        // 设置合理的优先级
        param.sched_priority = priority;
        if (pthread_attr_setschedparam(&attr, &param) == 0) {
            pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
			LOG_Debug("created with priority: %d",param.sched_priority);
        }else{
			LOG_Debug("pthread_attr_setschedparam failed");
		}		
    }

    retValue = pthread_create(threadId, &attr, fun, (void *)threadArg);
    pthread_attr_destroy(&attr);
    
    if(retValue < 0) {
        LOG_Error("pthread_create %s", strerror(errno));
        return RET_ERR;
    }

	LOG_Debug("tspapp  thread success");
	return RET_OK;
}

//ADB 开关配置
//开:touch /tmp/adbd
//关:rm -rf /tmp/adbd
static uint8_t s_adbCurrentStatus = 0xFF; // 记录当前ADB状态，0xFF表示初始未知状态

static void McuSyncAdbSwitchToMpuCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata){
    if (recvData == NULL || dataLen < 1) {
        LOG_Error("McuSyncAdbSwitchToMpuCallback: Invalid data");
        return;
    }
    
    const uint8_t *data = (const uint8_t *)recvData;
    uint8_t adbOpen = data[0];
    
    LOG_Debug("McuSyncAdbSwitchToMpuCallback: ADB_Open = %d, CurrentStatus = %d", adbOpen, s_adbCurrentStatus);
    
    // 检查状态是否发生变化，避免重复执行
    if (adbOpen == s_adbCurrentStatus) {
        LOG_Debug("ADB status unchanged, skip execution");
        return;
    }
    
    if (adbOpen == 1) {
        // 开启ADB
        int ret = SrGetSystemRetValue("touch /tmp/adbd");
        if (ret == 0) {
            s_adbCurrentStatus = 1;
            LOG_Debug("ADB enabled successfully");
        } else {
            LOG_Error("Failed to enable ADB, ret=%d", ret);
        }
    } else if (adbOpen == 0) {
        // 关闭ADB
        int ret = SrGetSystemRetValue("rm -rf /tmp/adbd");
        if (ret == 0) {
            s_adbCurrentStatus = 0;
            LOG_Debug("ADB disabled successfully");
        } else {
            LOG_Error("Failed to disable ADB, ret=%d", ret);
        }
    } else {
        LOG_Error("McuSyncAdbSwitchToMpuCallback: Invalid ADB_Open value = %d", adbOpen);
    }
}

gint main(void)
{
        // 启动日志协同HTTP服务器（端口20006），用于主副模组间通信
    StartLogCoordinationHttpServer();
    int ret = -1;
	pthread_t ConnectTask;
	pthread_t SendPackageDataTask;
	pthread_t getModemInfo;
	pthread_t contrlGateway;
	pthread_t canFileUpload;
    pthread_t networkMonitor;
    pthread_t culrGetUpgradeStatus;
    pthread_t uartSendData;
    // pthread_t qosPingDetect;
    // pthread_t qosBwAdjust;
    pthread_t emmcRoTask;
    pthread_t syncLogFile; // 日志同步到主模组线程
    pthread_t qosLatencyDetect;
    int qosSwitch = 0;

	LOG_INIT("mqtt_proto");

	ExceptionHookInit();

	StartQueueMsgRcvThread("mqtt_signal_handle"); //创建消息接收线程；
	
	InitSignalIpcCallback(); //接收来自MCU的event消息，并进行上报

    ret = QoS_Init();
	if (ret != 0) {
		LOG_Error("QoS_Init fail");
        return -1;
	}

    if (0 != InitMcuSwitchEventHandler())
    {
        LOG_Error("Failed to initialize MCU switch event handler");
    }

    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, PmStatusCallback, NULL);   //订阅电源状态变化
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW, KLINEStatusCallback, (void *)(SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW));   //订阅车辆硬线状态变化
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CTROL_SIMCARD_INFO_TO_MPU, SimCardInfoToMpuCallback, NULL);
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CTROL_GATEWAY_REQ, McuControlMpuGatewayCallback, NULL);
	SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CTROL_SWITCH_POWERUP_TO_MPU, McuSyncSwitchPowerUpCallback, NULL);
	SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CTROL_SWITCH_POWERDOWN_TO_MPU, McuSyncSwitchPowerDownCallback, NULL);
	SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CTROL_SWITCH_RESET_TO_MPU, McuSyncSwitchResetCallback, NULL);
	SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CTROL_VERSION_SYNC_TO_MPU, McuSyncVersionToMpuCallback, NULL);
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_DATABASE_CONTENT_SYNC_TO_MPU, McuSyncDatabaseToMpuCallback, NULL);
    SubscriptionQueueMsg((SYSQueMsgType_e)SYS_QUE_TYPE_MCU_ADB_SWITCH_SYNC_TO_MPU, McuSyncAdbSwitchToMpuCallback, NULL);

	MQTTCreateThread(&ConnectTask, ConnectTaskEntry, NULL,0);	//维持与车况推送平台的交互，与TSP平台基本相同
	MQTTCreateThread(&getModemInfo, GetModemInfoThread, NULL,0);	//九识 车况信息推送线程
	MQTTCreateThread(&contrlGateway, ControlGatewayThread, NULL,0);	//九识 车况信息推送线程
	MQTTCreateThread(&canFileUpload, UploadCanFileThread, NULL,0);	//九识 车况信息推送线程
    MQTTCreateThread(&SendPackageDataTask, SendPackageDataTaskEntry, NULL,0);	//九识 车况信息推送线程
    // 启动网络状态监测线程
    MQTTCreateThread(&networkMonitor, NetworkMonitorThread, NULL,0);    // 网络状态监测线程
    // 副模组启动curl 周期2s同步升级状态
    //if(0 != access("/dev/mmcblk0", F_OK)){//副模组
    MQTTCreateThread(&culrGetUpgradeStatus,curlGetUpgradeStatusThread, NULL,0);
    //}

    // 启动串口数据发送线程（优先级 50）
    MQTTCreateThread(&uartSendData, UartSendDataThread, NULL, 50);    // 串口数据发送线程，高优先级确保及时发送

    // QoS线程
    // MQTTCreateThread(&qosPingDetect, QoS_PingDetectThread, NULL, 0);
    // MQTTCreateThread(&qosBwAdjust, QoS_BwAdjustThread, NULL, 0);
    MQTTCreateThread(&emmcRoTask, DetectEMMCRoTHread, NULL, 0);

    // 日志同步线程
    MQTTCreateThread(&syncLogFile, SyncLogFileThread, NULL, 0); //九识 sirunlog同步线程

    MQTTCreateThread(&qosLatencyDetect, QoS_LatencyAdjust, NULL, 0);

	if(InitTboxDataBaseCache() != 0){ //初始化数据库关键数据缓存
        LOG_Error("%s InitTboxDataBaseCache NULL ERR", __FUNCTION__);
    }
    set_process_high_priority(-20);//设置高优先级

    sleep(10); // 3->10

    qosSwitch = QoS_CheckSwitch();
    if (qosSwitch == 1) {
        QoS_Enable();
        SetQoSFlag(1);
    } else if (qosSwitch == 0) {
        QoS_Disable();
        SetQoSFlag(0);
    } else {
        LOG_Error("QoS_CheckSwitch fail");
    }

	while(1)
	{
		sleep(1);
	}

	pthread_join(contrlGateway, NULL);
	pthread_join(getModemInfo, NULL);
	pthread_join(ConnectTask, NULL);
	pthread_join(SendPackageDataTask, NULL);
    pthread_join(networkMonitor, NULL);
    pthread_join(uartSendData, NULL);

    // pthread_join(qosPingDetect, NULL);
    // pthread_join(qosBwAdjust, NULL);
    pthread_join(qosLatencyDetect, NULL);

    QoS_DeInit();

    //if(0 != access("/dev/mmcblk0", F_OK)){
    pthread_join(culrGetUpgradeStatus, NULL);
    //}

    return 0;
}
