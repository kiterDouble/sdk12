#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "err_code.h"
#include "sr_log.h"
#include "doip_tester.h"


void ConnNotifyProc(uint32_t targetLa, int32_t status)
{
    LOG_Debug("%s called.", __FUNCTION__);
    
}
void TxAckProc(uint32_t targetLa, int32_t status)
{
    LOG_Debug("%s called.", __FUNCTION__);
}
void UdsRxNotifyProc(uint32_t targetLa, const uint8_t* udsData, uint32_t udsDataLen)
{
    LOG_DebugHex(udsData, udsDataLen, "UdsRxNotifyProc");
}

void OnlineNotifyProc(uint32_t targetLa)
{
    LOG_Debug("%s called.", __FUNCTION__);
}

int main(int argc, char *argv[])
{

    DoipTesterHandle_t client;
    DoipTesterCbs_t funs =
    {
        .conFunc = ConnNotifyProc,
        .txFunc = TxAckProc,
        .rxFunc = UdsRxNotifyProc,
        .onlineFunc = OnlineNotifyProc
    };
    DoipTesterNode_t node = 
    {
        .ipAddr = {0x0,},
        .activeType = 0xe2,
        .functionalLa = 0xe400,
        .sourceLa = 0x1001,
        .targetLa = 0x1002,
        .maxPL = 4096
    };

    LOG_INIT("demo");

    if (argc < 2)
    {
        LOG_Error("%s, argc < 2", __FUNCTION__);
        return -1;
    }
    memcpy(node.ipAddr, (void*)argv[1], sizeof(node.ipAddr));
    if (DoipTesterInit(&client, &funs) < 0)
    {
        LOG_Error("%s DoipTesterInit fail.", __FUNCTION__);
        return -1;
    }
    if (DoipTesterConnect(client, &node) < 0)
    {
        LOG_Error("%s DoipTesterConnect fail.", __FUNCTION__);
        return -1;
    }

    sleep(1);//wait activation
    while(1)
    {
        uint8_t udsdata[] = {0x22, 0xf1, 0xcc};
        if (DoipTesterUdsRequest(client, node.targetLa, udsdata, sizeof(udsdata)) < 0)
        {
            LOG_Error("%s DoipTesterUdsRequest fail.", __FUNCTION__);
        }        
        sleep(1);
    }
    
    return 0;
}
