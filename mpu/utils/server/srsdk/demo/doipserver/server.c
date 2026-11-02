#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "err_code.h"
#include "sr_log.h"
#include "doip_entity.h"

static int32_t gfds[2] = {-1, -1};

typedef struct
{
    uint32_t testerLa;
    uint32_t dataLen;
    uint8_t data[0];
}UdsSrvData_t;

void UdsRequestProcess(uint32_t testerLa, const uint8_t *udsData, uint32_t dataLen)
{
    LOG_Debug("%s called.", __FUNCTION__);
    UdsSrvData_t *srvData;
    uint8_t buffer[4096 + sizeof(UdsSrvData_t)] = {0x0,};
    uint32_t len = sizeof(UdsSrvData_t) + dataLen;

    srvData = (UdsSrvData_t*) buffer;
    srvData->testerLa = testerLa;
    srvData->dataLen = dataLen;

    memcpy(srvData->data, udsData, dataLen);
    
    int32_t writen = write(gfds[0], buffer, len);
    LOG_Debug("writen:%d", writen);
    return;
}

int main(int argc, char *argv[])
{
    DoipEntityCfg_t cfg = 
    {
        .activeType = 0xe2,
        .testerNum = 1,
        .testerLas = {0x1001,},
        .entityLa = 0x1002,
        .functionLa = 0xe400,
        .eid = {0x02, 0x47, 0x57, 0x4d, 0x00, 0x97},
        .gid = {0x02, 0x47, 0x57, 0x4d, 0x00, 0x99},
        .vin = {0x0,}      
    };

    uint8_t resp[] = {0x7f, 0x22, 0x30};
    DoipEntityHandle_t server;
    LOG_INIT("demo");
    if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, gfds)) 
    {
        LOG_Error("%s socketpair fail.", __FUNCTION__);
        return -1;
    }

    DoipEntityInit(&server, &cfg, UdsRequestProcess);

    while (1)
    {
        UdsSrvData_t *srvData;
        uint8_t buffer[4096 + sizeof(UdsSrvData_t)] = {0x0,};
        srvData = (UdsSrvData_t*) buffer;

        int32_t readn = read(gfds[1], buffer, sizeof(buffer));

        if (readn > 0)
        {
            DoipEntityUdsResponse(server, srvData->testerLa, resp, sizeof(resp));
        }

    }

    return 0;
}