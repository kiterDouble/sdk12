#ifndef _BCALL_NAD_MANAGER_H_
#define _BCALL_NAD_MANAGER_H_

#include <glib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    BCALL_TYPE_START_INIT = 0,	        /*初始化状态*/
	BCALL_TYPE_START_ECALL = 1,	        /*触发bcall*/
	BCALL_TYPE_HUANGUP = 2,		        /*挂断bcall通信*/
	BCALL_TYPE_UPDATE_MSD = 3,		    /*更新bcall-msd数据*/
    BCALL_TYPE_PUSH_MSD = 4,		    /*推送bcall-msd数据*/
    BCALL_TYPE_ERR = 99,		        /*不能识别的事件*/    
}EnumBcallEventType;

typedef enum
{
    BCALL_TRIGGER_SOS = 0,	                        /*SOS按键手动触发*/
	BCALL_TRIGGER_SRS = 1,	                        /*SRS硬线自动触发*/
	BCALL_TRIGGER_CAN = 2,		                    /*CAN信号自动触发*/
	BCALL_TRIGGER_ACCELERATION_SENSOR = 3,		    /*加速度传感器自动触发*/
}EnumBcallTriggerType;

typedef struct{
    long SubCmd;
    EnumBcallEventType bcallEventType;            
    EnumBcallTriggerType bcallTriggerType;          
}StrBcallEventControlCmd_t;

/**
* @brief 将事件控制命令发送到BCALL SERVER处理线程
*
* @param[in] bcallEventControlCmd 存储事件控制命令的结构体
* @return 返回发送消息的ID
*/
extern int32_t SendEventToBcallServer(StrBcallEventControlCmd_t *bcallEventControlCmd);
/**
* @brief ECALL SERVER 线程
*
* @param[in] pArg 用户数据
* @return 无
*/
extern gpointer BcallServer(gpointer pArg);

#ifdef __cplusplus
}
#endif

#endif //_BCALL_NAD_MANAGER_H_E


