#include "tsp_common.h"
#include "tsp_connect.h"
#include "tsp_dispatch.h"
#include "tsp_monitor.h"
#include "task_exc_handle.h"
#include "tsp_parser_diagnosis.h"
#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>
#include "tsp_oder_charging.h"
#include "sgmw_ipc.h"
#include "sr_ipc.h"

typedef void *(threadFun)(void *);

#define SEND_DIAGNOSIS_HEARTBEAT (9)

static int32_t TspAppCreateThread(pthread_t *threadId, threadFun fun, void *threadArg)
{
	pthread_attr_t attr;
	int32_t retValue = -1;

	if(NULL == threadId )
	{
		LOG_Error("%s argv threadId  is NULL", __FUNCTION__);
		return RET_ERR;
	}

	pthread_attr_init(&attr);

	//创建线程
	retValue = pthread_create(threadId, &attr, fun,(void *)threadArg);
	if(retValue < 0)
	{
		LOG_Error("pthread_create %s",strerror(errno));
		return RET_ERR;
	}

	LOG_Debug("tspapp  thread success");
	return RET_OK;
}

gpointer SendDiagnosisHeartbeat(gpointer pArg)
{
    while(1)
    {
        LOG_Debug("SendDiagnosisHeartbeat");
        DiagnosisHeartbeat();
        sleep(SEND_DIAGNOSIS_HEARTBEAT);
    }

}

void CheckParamsDB()
{
    uint8_t urlBuff[128] = {0}; 
    //从参数中获取tspToken url，若获取失败，则设置默认值；
    if (!GetTspTokenUrlUrl(urlBuff, sizeof(urlBuff) - 1))
    {
		ParamInfo_t tspTokenUrl;
		tspTokenUrl.id = E_ParamId_TspTokenUrl;
		memcpy(tspTokenUrl.name, "E_ParamId_TspTokenUrl", MIN(sizeof(tspTokenUrl.name) - 1, sizeof("E_ParamId_TspTokenUrl")));
		tspTokenUrl.type = 0;
		tspTokenUrl.min = 1;
		tspTokenUrl.max = 128;
		tspTokenUrl.paramSources = PARAM_SOURCES_USER;
		tspTokenUrl.rpcWriteEnable = true;
		memcpy(tspTokenUrl.value, "https://iov-prod.sgmwcloud.com.cn:443/sgmwcarshare/deviceMng/v1/device/login", MIN(sizeof(tspTokenUrl.value) - 1, sizeof("https://iov-prod.sgmwcloud.com.cn:443/sgmwcarshare/deviceMng/v1/device/login")));
		if (RET_ERR == AddUserParam(tspTokenUrl))
		{
			LOG_Debug("%s AddUserParam tspTokenUrl error", __FUNCTION__);
		}
		else
		{
			LOG_Debug("%s AddUserParam tspTokenUrl success", __FUNCTION__);
		}
    }
}

gint main(void)
{
	pthread_t ConnectTask;
	pthread_t DispatchTask;
	pthread_t OderChargingHandleTask;
	pthread_t CPUStatusTask;
	pthread_t MonitorThreadTask;


	LOG_INIT("tspApp");

	StartQueueMsgRcvThread("tsp_signal_handle"); //创建消息接收线程；
	InitOderChargingTime(ODER_CHARGING_INFO, strlen(ODER_CHARGING_INFO));
	InitSignalIpcCallback(); //订阅mcu返回的远控结果，并在回调中作出相应后续动作；
	
	TspAppCreateThread(&ConnectTask, ConnectTaskEntry, NULL);	//远控状态机，获取远控指令；
	TspAppCreateThread(&DispatchTask, DispatchTaskEntry, NULL);	//解析并执行指令；
	TspAppCreateThread(&OderChargingHandleTask, OderChargingHandleTaskEntry, NULL);	//预约充电处理；
	TspAppCreateThread(&CPUStatusTask, CPUStatusThread, NULL);
	TspAppCreateThread(&MonitorThreadTask, AccessMonitorEntry,NULL);   //监控九识远端服务器和内部网络节点
	




	sleep(30);
	CheckParamsDB(); //检查数据库；

	while(1)
	{
		sleep(1);
	}

	pthread_join(ConnectTask, NULL);
	pthread_join(DispatchTask, NULL);
	pthread_join(OderChargingHandleTask, NULL);
	pthread_join(CPUStatusTask, NULL);
	pthread_join(MonitorThreadTask,NULL);
    return 0;
}
