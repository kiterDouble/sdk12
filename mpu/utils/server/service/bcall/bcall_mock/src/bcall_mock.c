#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "sr_log.h"
#include "err_code.h"

#define BCALL_MSG_SUBCMD (0x9007)

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
int32_t SendEventToBcallServer(StrBcallEventControlCmd_t *bcallEventControlCmd)
{
    int ret = 0;
    int32_t msgID = 0;
    const char *filepath = "/tmp/bcallKey";
    StrBcallEventControlCmd_t localBcallEventControlCmd = {0};
    memcpy(&localBcallEventControlCmd,bcallEventControlCmd,sizeof(StrBcallEventControlCmd_t));

    int fd = open(filepath, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        LOG_Error("%s : open error : %s ",__func__,strerror(errno));
        return RET_ERR;
    }
    close(fd);
    key_t msgSendKey = ftok(filepath, 'B');
    if (msgSendKey == -1) {
        LOG_Error("%s : ftok error : %s ",__func__,strerror(errno));
        return RET_ERR;
    }

    msgID = msgget(msgSendKey, 0666 | IPC_CREAT);
    if (msgID < 0)
    {
        LOG_Error("%s : SendEventToEcallServer msgget error : %s ", __func__,strerror(errno));
        return RET_ERR;
    }
    else
    {
        LOG_Info("%s : SendEventToEcallServer msgget successfull %s ", __func__ ,strerror(errno));
    }
    LOG_Info("msg : %s",__FUNCTION__);
    localBcallEventControlCmd.SubCmd = BCALL_MSG_SUBCMD;
    ret = msgsnd(msgID,&localBcallEventControlCmd, sizeof(StrBcallEventControlCmd_t),0);
    return ret;
}

int main()
{    
    StrBcallEventControlCmd_t bcallEventControlCmd = {0};
    
    LOG_INIT("test");
    
    bcallEventControlCmd.bcallEventType = BCALL_TYPE_START_ECALL;
    bcallEventControlCmd.bcallTriggerType = BCALL_TRIGGER_SOS;
    SendEventToBcallServer(&bcallEventControlCmd);
    
    return 0;
}
