#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "era_timer.h"
#include <stdio.h>
#include <stdio.h>

#include "sr_log.h"
#include "err_code.h"


struct EParamMsg {
    long mtype;
    char mtext[1024];
};

#define EPARAM_OUTQUE_PATH      "/tmp/ecallQue"
#define EPARAM_C2S_TYPE         1000
#define EPARAM_S2C_TYPE         1001
static int g_EParamMsgQue = -1;
static timer_t g_EParamTimer = {0};
int32_t SendEventToEcallServer(char *data)
{
    int ret = 0;
    struct EParamMsg msg;

    msg.mtype = EPARAM_C2S_TYPE;

    snprintf(msg.mtext, sizeof(msg.mtext), "%s", data);
    ret = msgsnd(g_EParamMsgQue, &msg, strlen(msg.mtext)+1, IPC_NOWAIT);

//    printf("SendEventToEcallServer msgsnd, g_EParamMsgQue=%d, ret = %d(%s)\n", g_EParamMsgQue, ret, strerror(errno));

    return ret;
}

void print_help(void);
int HelpCmdCallBack(char *cmd)
{
    print_help();
    return 0;
}

int KeylistCmdCallBack(char *cmd)
{
    return SendEventToEcallServer("keylist");
}

int KeyGetCmdCallBack(char *cmd)
{
    return SendEventToEcallServer(cmd);
}

int KeyPutCmdCallBack(char *cmd)
{
    if(NULL == cmd)
    {
        fprintf(stderr, "cmd == NULL\n");
        return -1;
    }

    if(NULL == strchr(cmd, '='))
    {
        fprintf(stderr, "parameter must has '='\n");
        return -1;
    }
    
    SendEventToEcallServer(cmd);

    return 0;
}

int ByeCmdCallBack(char *cmd)
{
    printf("exit finished!\n");
    exit(0);
    return 0;
}

typedef struct
{
    char *cmd;
    char *help;
    int (*callBack)(char *cmd);
} st_api_test_case;

st_api_test_case at_api_testcases[] = 
{
    {"help", "print_help", HelpCmdCallBack},
    {"keylist", "send query keylist", KeylistCmdCallBack},
    {"keyget", "send query keylist value", KeyGetCmdCallBack},
    {"keyput", "send write value", KeyPutCmdCallBack},
    {"bye", "quilt the ecall dail ", ByeCmdCallBack},
    {"quit", "quilt the ecall dail ", ByeCmdCallBack}
};

void print_help(void)
{
    int i;

    printf("Supported test cases:\n");
    for (i = 0; i<sizeof(at_api_testcases)/sizeof(at_api_testcases[0]); i++)
    {
        printf("%d. %s\t%s\n", i, at_api_testcases[i].cmd, at_api_testcases[i].help);
    }
}

int recv_flag = 0;
struct EParamMsg recv_msg;

static void EParamTimerCallBack(union sigval val)
{
    ssize_t len = 0;

    len = msgrcv(g_EParamMsgQue, &recv_msg, sizeof(recv_msg.mtext), EPARAM_S2C_TYPE, IPC_NOWAIT);
//    printf("recv length = %d\n", len);
    if(len > 0)
    {
//        printf("recv : %s\n", recv_msg.mtext); 
        recv_flag = 1;
    }
}

    
static int EParamQueInit(const char *msgPath)
{
    int fd = 0;
    key_t key = 0;

    fd = open(msgPath, O_CREAT|O_RDWR, 0666);
    if (fd < 0) {
        LOG_Error("open error : %s ",strerror(errno));
        return RET_ERR;
    }

    key = ftok(msgPath, 'E');
    if (key == -1) {
        LOG_Error("ftok error : %s ",strerror(errno));
        return RET_ERR;
    }

    LOG_Debug("ParamQue(%s) init success!", msgPath);

    fd = msgget(key, 0666|IPC_CREAT);

    return fd;
}


int main(int argc, char **argv)
{   
    char *rs = NULL;
    char save[256] = {0};
    char input[256] = {0};
    int i = 0;
    char *key = NULL;
    char *value = NULL;
    int pos = 0;
    
    LOG_INIT("test");
    g_EParamMsgQue = EParamQueInit(EPARAM_OUTQUE_PATH);
    InitTimer(&g_EParamTimer, EParamTimerCallBack);
    TimerXSecTimer(&g_EParamTimer, 1, 1);

    
    while (1)
    {
        printf("########################### ECALL CONFIG ##########################\n");
        print_help();
        printf("\e[33;1mplease input cmd : \e[0m");
        fflush(stdout);
        memset(input, 0, sizeof(input));
        rs = fgets(input, sizeof(input), stdin); 
        if(NULL==rs || 0==strlen(input))
            continue;

        pos = strlen(input);
        input[pos-1] = '\0';
        strcpy(save, input);
        key = strtok_r(input, " ", &value);
        if(NULL == key)
            continue;
        for(i=0; i < sizeof(at_api_testcases)/sizeof(at_api_testcases[0]); i++)
        {
            if(0 == strcmp(at_api_testcases[i].cmd, key))
            {
                if(0 != at_api_testcases[i].callBack(value))
                    fprintf(stderr, "cmd execute fail!\n");
                break;
            }
        }
        if(i == (sizeof(at_api_testcases)/sizeof(at_api_testcases[0])))
            fprintf(stderr, "unsupported cmd(%s)(%d)\n", save, strlen(save));
        for(i=0; i < 3; i++)
        {
            sleep(1);
            if(recv_flag)
            {
                printf("\e[33;1mrecv :\e[0m %s\n", recv_msg.mtext);
                recv_flag = 0;
                break;
            }
        }

        printf("\n\n");
    }

    return RET_OK;
}
