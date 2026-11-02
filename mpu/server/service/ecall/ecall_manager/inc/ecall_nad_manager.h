#ifndef _ECALL_NAD_MANAGER_H_
#define _ECALL_NAD_MANAGER_H_

#include <glib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ECALL_TYPE_START_INIT = 0,	        /*初始化状态*/
	ECALL_TYPE_START_ECALL = 1,	        /*触发ecall*/
	ECALL_TYPE_HUANGUP = 2,		        /*挂断ecall通信*/
	ECALL_TYPE_UPDATE_MSD = 3,		    /*更新ecall-msd数据*/
    ECALL_TYPE_PUSH_MSD = 4,		    /*推送ecall-msd数据*/
    ECALL_TYPE_ERR = 99,		        /*不能识别的事件*/    
}EnumEcallEventType;

typedef enum
{
    ECALL_TRIGGER_SOS = 0,	                        /*SOS按键手动触发*/
	ECALL_TRIGGER_SRS = 1,	                        /*SRS硬线自动触发*/
	ECALL_TRIGGER_CAN = 2,		                    /*CAN信号自动触发*/
	ECALL_TRIGGER_ACCELERATION_SENSOR = 3,		    /*加速度传感器自动触发*/
    ECALL_TRIGGER_TEST = 4,		                    /*测试模式触发*/
}EnumEcallTriggerType;

typedef struct{
    long SubCmd;
    EnumEcallEventType ecallEventType;            
    EnumEcallTriggerType ecallTriggerType;          
}StrEcallEventControlCmd_t;

/**
* @brief 将事件控制命令发送到ECALL SERVER处理线程
*
* @param[in] ecallEventControlCmd 存储事件控制命令的结构体
* @return 返回发送消息的ID
*/
extern int32_t SendEventToEcallServer(StrEcallEventControlCmd_t *ecallEventControlCmd);
/**
* @brief ECALL SERVER 线程
*
* @param[in] pArg 用户数据
* @return 无
*/
extern gpointer EcallServer(gpointer pArg);

#ifdef __cplusplus
}
#endif

#endif //_ECALL_NAD_MANAGER_H_E


