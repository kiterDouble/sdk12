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

#define ECALL_MSG_SUBCMD (0x8007)

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

int32_t SendEventToEcallServer(StrEcallEventControlCmd_t *ecallEventControlCmd)
{
    int ret = 0;
    int32_t msgID = 0;
    const char *filepath = "/tmp/ecallKey";
    StrEcallEventControlCmd_t localEcallEventControlCmd = {0};
    memcpy(&localEcallEventControlCmd,ecallEventControlCmd,sizeof(StrEcallEventControlCmd_t));

    int fd = open(filepath, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        LOG_Error("%s : open error : %s ",__func__,strerror(errno));
        return RET_ERR;
    }
    close(fd);
    key_t msgSendKey = ftok(filepath, 'E');
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
    LOG_Info("msg SendEventToEcallServer ");
    localEcallEventControlCmd.SubCmd = ECALL_MSG_SUBCMD;
    ret = msgsnd(msgID,&localEcallEventControlCmd, sizeof(StrEcallEventControlCmd_t),0);
    return ret;
}

int main()
{    
    StrEcallEventControlCmd_t ecallEventControlCmd = {0};
    
    LOG_INIT("test");
    
    ecallEventControlCmd.ecallEventType = ECALL_TYPE_START_ECALL;
    ecallEventControlCmd.ecallTriggerType = ECALL_TRIGGER_SOS;
    SendEventToEcallServer(&ecallEventControlCmd);
    
    return 0;
}
