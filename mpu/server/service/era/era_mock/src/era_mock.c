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
    ECALL_TYPE_DUMP_INFO=98,            /*打印当前模块核心信息*/
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
    const char *filepath = "/tmp/eraKey";
    StrEcallEventControlCmd_t localEcallEventControlCmd = {0};
    memcpy(&localEcallEventControlCmd,ecallEventControlCmd,sizeof(StrEcallEventControlCmd_t));

    int fd = open(filepath, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        LOG_Error("%s : open error : %s ",__func__,strerror(errno));
        return RET_ERR;
    }
    close(fd);
    key_t msgSendKey = ftok(filepath, 'A');
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

typedef struct
{
    int cmdIdx;
    char *funcName;
} st_api_test_case;

st_api_test_case at_api_testcases[] = 
{
    {0, "print_help"},
    {1, "Send manual ecall by button SOS"},
    {2, "Send auto ecall by SRS"},
    {3, "Send auto ecall by CAN "},
    {4, "Send test ecall by SENSOR "},
    {5, "Send test ecall by TEST "},
    {6, "Hangup the call  "},
    {7, "Send Periodic automatic triggering "},
    {8, "dump ecallServer information"},
    {'q', "quilt the ecall dail "}
};

void print_help(void)
{
    int i;
    size_t array_size = sizeof(at_api_testcases) / sizeof(at_api_testcases[0]);

    printf("Supported test cases:\n");
    for (i = 0; i < array_size; i++)
    {
        if (at_api_testcases[i].cmdIdx == 'q')
        {
            break;
        }
        printf("%d:\t%s\n", at_api_testcases[i].cmdIdx, at_api_testcases[i].funcName);
    }
}

int main(int argc, char **argv)
{   
    char input[256] = {0};
    StrEcallEventControlCmd_t ecallEventControlCmd = {0};
    int cmdIdx = 0;
    
    LOG_INIT("test");
    
    while (1)
    {
        printf("########################### ECALL TRIGGER ##########################\n");
        print_help();
        printf("please input cmd index(q exit): ");
        memset(input, 0, sizeof(input));
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("Input error\n");
            continue;
        }
        input[strcspn(input, "\n")] = 0;  // 移除换行符
        if (input[0] == 'q' && strlen(input) == 1)
        {
            printf("eCall quit \n");
            break;
        }
        if (sscanf(input, "%d", &cmdIdx) != 1)
        {
            printf("Invalid input. Please input a number or 'q'.\n");
            continue;
        }
        if (cmdIdx == 1)
        {
            ecallEventControlCmd.ecallEventType = ECALL_TYPE_START_ECALL;
            ecallEventControlCmd.ecallTriggerType = ECALL_TRIGGER_SOS;
            printf("eCall Tirggered, Manual eCall by SOS\n");
            SendEventToEcallServer(&ecallEventControlCmd);
        }
        else if (cmdIdx == 2)
        {
            ecallEventControlCmd.ecallEventType = ECALL_TYPE_START_ECALL;
            ecallEventControlCmd.ecallTriggerType = ECALL_TRIGGER_SRS;
            printf("eCall Tirggered, Manual eCall by SRS\n");
            SendEventToEcallServer(&ecallEventControlCmd);
        }
        else if (cmdIdx == 3)
        {
            ecallEventControlCmd.ecallEventType = ECALL_TYPE_START_ECALL;
            ecallEventControlCmd.ecallTriggerType = ECALL_TRIGGER_CAN;
            printf("eCall Tirggered, Manual eCall by CAN\n");
            SendEventToEcallServer(&ecallEventControlCmd);
        }
        else if (cmdIdx == 4)
        {
            ecallEventControlCmd.ecallEventType = ECALL_TYPE_START_ECALL;
            ecallEventControlCmd.ecallTriggerType = ECALL_TRIGGER_ACCELERATION_SENSOR;
            printf("eCall Tirggered, Manual eCall by SENSOR\n");
            SendEventToEcallServer(&ecallEventControlCmd);
        }
        else if (cmdIdx == 5)
        {
            ecallEventControlCmd.ecallEventType = ECALL_TYPE_START_ECALL;
            ecallEventControlCmd.ecallTriggerType = ECALL_TRIGGER_TEST;
            printf("eCall Tirggered, Manual eCall by TEST\n");
            SendEventToEcallServer(&ecallEventControlCmd);
        }
        else if (cmdIdx == 6)
        {
            ecallEventControlCmd.ecallEventType = ECALL_TYPE_HUANGUP;
            printf("Hangup the ecall .\n");
            SendEventToEcallServer(&ecallEventControlCmd);
        }
        else if (cmdIdx == 7)
        {
            int seconds;
            trigger_period:
            printf("Please input the trigger period (in seconds): ");
            if (fgets(input, sizeof(input), stdin) == NULL)
            {
                printf("Input error\n");
                goto trigger_period;
            }
            input[strcspn(input, "\n")] = 0;  // 移除换行符
            if (sscanf(input, "%d", &seconds) != 1)
            {
                printf("Invalid input. Please input times in seconds.\n");
                goto trigger_period;
            }
            printf("Periodic eCall triggering every %d seconds\n", seconds);

            fd_set read_fds;
            struct timeval timeout;

            while (1)
            {
                ecallEventControlCmd.ecallEventType = ECALL_TYPE_START_ECALL;
                ecallEventControlCmd.ecallTriggerType = ECALL_TRIGGER_SRS;
                SendEventToEcallServer(&ecallEventControlCmd);
                printf("eCall Triggered, Auto eCall with period = %d seconds\n", seconds);

                for (int remaining = seconds; remaining > 0;)
                {
                    timeout.tv_sec = remaining;
                    timeout.tv_usec = 0;
                    FD_ZERO(&read_fds);
                    FD_SET(STDIN_FILENO, &read_fds);

                    int ret = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);
                    if (ret > 0 && FD_ISSET(STDIN_FILENO, &read_fds))
                    {
                        if (fgets(input, sizeof(input), stdin) == NULL)
                        {
                            printf("Input error\n");
                            continue;
                        }
                        input[strcspn(input, "\n")] = 0;
                        if (input[0] == 'q' && strlen(input) == 1)
                        {
                            printf("trigger period quit \n");
                            goto exit_loop;
                        }
                        else
                        {
                            remaining = timeout.tv_sec;
                            printf("timeout : %ld. \n",timeout.tv_sec);
                            continue;
                        }
                    }
                    else if (ret == -1)
                    {
                        perror("select");
                        goto exit_loop;
                    }
                    else
                    {
                        remaining = 0;
                    }
                }
            }
            exit_loop:;
        }
        else if (cmdIdx == 8)
        {
            ecallEventControlCmd.ecallEventType = ECALL_TYPE_DUMP_INFO;
            printf("dump ecallServer \n");
            SendEventToEcallServer(&ecallEventControlCmd);
        }
        cmdIdx = 0;
        printf("\n\n");
    }

    return RET_OK;
}
