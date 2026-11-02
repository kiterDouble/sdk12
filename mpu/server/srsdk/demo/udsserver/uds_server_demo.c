#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include "uds_server.h"

typedef uint8_t (*udsProcessCb)(struct UDSServer *srv, UDSServerEvent_t event, const void *arg);

typedef struct
{
    int32_t cltid;
    int32_t srvid;
    int32_t seed;
    int32_t preChecked;
}UdsMsg_t;

typedef struct 
{
    UDSServerEvent_t event;
    udsProcessCb processCb;
}UdsEventProcess_t;


static ssize_t UdsSrvRecv(struct UDSTpHandle *hdl, uint8_t *buf, size_t count, uint32_t *tpAddrType)
{
    if (!hdl || !buf)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return RET_ERR;
    }

    UdsMsg_t *msg = (UdsMsg_t *)hdl->impl;
    ssize_t size = msgrcv(msg->srvid, buf, count, 0, IPC_NOWAIT);
    if (size > 0)
    {
        LOG_DebugHex(buf, size, __FUNCTION__);
    }
    return size;
}

static ssize_t UdsSrvSend(struct UDSTpHandle *hdl, const uint8_t *buf, size_t count, uint32_t tpAddrType)
{
    if (!hdl || !buf)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return RET_ERR;
    }

    {
        LOG_DebugHex(buf, count, __FUNCTION__);
    }
    UdsMsg_t *msg = (UdsMsg_t *)hdl->impl;
    int32_t ret = msgsnd(msg->cltid, buf, count, 0);
    if (ret == 0)
    {
        return count;
    }
    return ret;
}

static uint32_t UdsSrvPoll(struct UDSTpHandle *hdl)
{
    return 0;
}

static uint8_t udsDiagSessCtrl(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    UDSDiagSessCtrlArgs_t *ctlargs = (UDSDiagSessCtrlArgs_t *)arg;

    if (!srv || !ctlargs)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_REJECT;        
    }

    if (srv->sessionType == ctlargs->type)
    {
        return E_UDS_SRV_RESP_CODE_POSITIVE;
    }
    if ((ctlargs->type == E_UDS_SESSION_TYPE_PROGRAMMING) && (srv->sessionType != E_UDS_SESSION_TYPE_EXTENDED))
    {
		LOG_Error("%s E_UDS_SRV_RESP_CODE_CONDITIONS_NOT_CORRECT.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_CONDITIONS_NOT_CORRECT;
    }

    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

static uint8_t udsEcuReset(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

static uint8_t udsReadDataByIdent(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    UDSRDBIArgs_t *rdbiArgs = (UDSRDBIArgs_t *)arg;

    if (!srv || !rdbiArgs)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_REJECT;        
    }

    uint8_t msg[] = {0x01, 0x2a, 0x2f, 0x00,};//idx 0:moudle num, always 1.idex 1&2:prefix 0x2a 0x2f 
    uint32_t len = sizeof(msg);

    return rdbiArgs->copy(srv, (void *)msg, len); 
}

static uint8_t udsSecAccessRequestSeed(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    UDSSecAccessRequestSeedArgs_t *seedarg = (UDSSecAccessRequestSeedArgs_t *)arg;
    unsigned char seed[4] = {0x0,};
    if (!srv || !seedarg)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_REJECT;        
    }

    if((seedarg->level == srv->securityLevel1 ) && (srv->sessionType != E_UDS_SESSION_TYPE_EXTENDED))
    {
        return E_UDS_SRV_RESP_CODE_SECURITY_ACCESS_DENIED;
    }

    if ((seedarg->level == srv->securityLevel2) && (srv->sessionType != E_UDS_SESSION_TYPE_PROGRAMMING))
    {
        return E_UDS_SRV_RESP_CODE_SECURITY_ACCESS_DENIED;
    }

    UdsMsg_t *msg = (UdsMsg_t *)srv->tp->impl;
    //srand(time(NULL));
    msg->seed = 0x3b6c2d1e;//rand();
    LOG_Debug("%s called, seedint:%x", __FUNCTION__, msg->seed);
    seed[0] = (msg->seed >> 24) & 0xff;
    seed[1] = (msg->seed >> 16) & 0xff;
    seed[2] = (msg->seed >> 8) & 0xff;
    seed[3] = msg->seed & 0xff;
    seedarg->copySeed(srv, seed, sizeof(seed));
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

static uint32_t udsSeed2Key(uint32_t seed, uint32_t mask)
{
    uint32_t key = 0;
    int i = 0;

    if (seed != 0)
    {
        for (; i < 35; i++)
        {
            if (seed & 0x80000000)
            {
                seed = seed << 1;
                seed = seed ^ mask;
            }
            else
            {
                seed = seed << 1;
            }
            
        }
        key = seed;
    }

    return key;
}

static uint8_t udsSecAccessValidateKey(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    UDSSecAccessValidateKeyArgs_t *keyargs = (UDSSecAccessValidateKeyArgs_t *)arg;
    uint32_t mask;
    uint32_t keyint1;
    uint32_t keyint2;

    if (!srv || !keyargs)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_REJECT;        
    }

    UdsMsg_t *msg = (UdsMsg_t *)srv->tp->impl;

    mask = 0x19424F58;

    if (keyargs->level == srv->securityLevel2)
    {
        mask = 0x29424F58;
        if (msg->preChecked == 0)
        {
            return E_UDS_SRV_RESP_CODE_CONDITIONS_NOT_CORRECT;
        }

        if (srv->securityLevel != srv->securityLevel1)
        {
            return E_UDS_SRV_RESP_CODE_SECURITY_ACCESS_DENIED;
        }
    }

    keyint1 = udsSeed2Key(msg->seed, mask);
    keyint2 = (keyargs->key[0] & 0xff) << 24;
    keyint2 += (keyargs->key[1] & 0xff) << 16;
    keyint2 += (keyargs->key[2] & 0xff) << 8;
    keyint2 += (keyargs->key[3] & 0xff);

    if (keyint1 != keyint2)
    {
        return E_UDS_SRV_RESP_CODE_INVALID_KEY;              
    }

    srv->securityLevel = keyargs->level;
    if (keyargs->level == srv->securityLevel1)
    {
        msg->preChecked = 1;
    }

    return E_UDS_SRV_RESP_CODE_POSITIVE;  
}

static uint8_t udsWriteDataByIdent(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

static uint8_t UdsSrvProcess(UDSServer_t *srv, UDSServerEvent_t event, const void *arg)
{
    UdsEventProcess_t procs[] = 
    {
        {E_UDS_SRV_EVT_DiagSessCtrl, udsDiagSessCtrl},          // UDSDiagSessCtrlArgs_t * 0x10
        {E_UDS_SRV_EVT_EcuReset, udsEcuReset},              // UDSECUResetArgs_t * 0x11
        {E_UDS_SRV_EVT_ReadDataByIdent, udsReadDataByIdent},       // UDSRDBIArgs_t * 0x22
        {E_UDS_SRV_EVT_SecAccessRequestSeed, udsSecAccessRequestSeed},  // UDSSecAccessRequestSeedArgs_t * 0x27
        {E_UDS_SRV_EVT_SecAccessValidateKey, udsSecAccessValidateKey},  // UDSSecAccessValidateKeyArgs_t * 0x27
        {E_UDS_SRV_EVT_WriteDataByIdent, udsWriteDataByIdent},      // UDSWDBIArgs_t * 0x2e
    };

    int32_t len = sizeof(procs) / sizeof(UdsEventProcess_t);
    int32_t i = 0;

    for (; i < len; i++)
    {
        if (event == procs[i].event)
        {
            return procs[i].processCb(srv, event, arg);
        }
    }
    return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
}

static int32_t UdsMsgInit(UdsMsg_t *msg)
{
    if (!msg)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;
    }

    key_t key = ftok("/tmp", 's');
    if (key == -1)
    {
        LOG_Error("%s srv ftok fail.", __FUNCTION__);
        return RET_ERR;
    }

    int msgid = msgget(key, IPC_CREAT | 0644);
    if (msgid == -1)
    {
        LOG_Error("%s srv msgget fail.", __FUNCTION__);
        return RET_ERR;
    }
    msg->srvid = msgid;

    key = ftok("/tmp", 'c');
    if (key == -1)
    {
        LOG_Error("%s clt ftok fail.", __FUNCTION__);
        msgctl(msg->srvid, IPC_RMID, NULL);
        return RET_ERR;
    }

    msgid = msgget(key, IPC_CREAT | 0644);
    if (msgid == -1)
    {
        LOG_Error("%s clt msgget fail.", __FUNCTION__);
        msgctl(msg->srvid, IPC_RMID, NULL);
        return RET_ERR;
    }
    msg->cltid = msgid;
    msg->seed = 0;
    msg->preChecked = 0;
    return RET_OK;
}

static int32_t UdsMsgDeinit(UdsMsg_t *msg)
{
    if (!msg)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;
    }
    msgctl(msg->srvid, IPC_RMID, NULL);
    msgctl(msg->cltid, IPC_RMID, NULL);
    msg->srvid = -1;
    msg->cltid = -1;

    return RET_OK;
}

int main(int argc, char *argv[])
{
    UdsMsg_t msg;
    UDSServer_t server;
    LOG_INIT("demo");
    if (UdsMsgInit(&msg) < 0)
    {
        LOG_Error("%s UdsMsgInit fail.", __FUNCTION__);
        return RET_ERR;
    }

    UDSTpHandle_t udsTphdl = 
    {
        .send = UdsSrvSend,
        .recv = UdsSrvRecv,
        .poll = UdsSrvPoll,
        .impl = (void *) &msg
    };

    UDSServerConfig_t cfg = 
    {
        .EventProcessCb = UdsSrvProcess,
        .tp = &udsTphdl,
        .securityLevel1 = 0x01,
        .securityLevel2 = 0x05
    };

    if (UDSServerInit(&server, &cfg) < 0)
    {
        LOG_Error("%s UDSServerInit fail.", __FUNCTION__);
        UdsMsgDeinit(&msg);
        return RET_ERR;
    }

    while (1)
    {
        UDSServerPoll(&server);
        SleepMillis(1);
    }
    UdsMsgDeinit(&msg);
    return 0;
}
