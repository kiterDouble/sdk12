#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "err_code.h"
#include "sr_log.h"
#include "uds_client.h"

typedef struct
{
    int32_t cltid;
    int32_t srvid;
    uint32_t seed;
}UdsMsg_t;

static ssize_t UdsCltRecv(struct UDSTpHandle *hdl, uint8_t *buf, size_t count, uint32_t *tpAddrType)
{
    if (!hdl || !buf)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return RET_ERR;
    }

    UdsMsg_t *msg = (UdsMsg_t *)hdl->impl;
    ssize_t size = msgrcv(msg->cltid, buf, count, 0, 0);

    if (size > 0)
    {
        LOG_DebugHex(buf, size, __FUNCTION__);
    }
    return size;
}

static ssize_t UdsCltSend(struct UDSTpHandle *hdl, const uint8_t *buf, size_t count, uint32_t tpAddrType)
{
    if (!hdl || !buf)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return RET_ERR;
    }

    UdsMsg_t *msg = (UdsMsg_t *)hdl->impl;
    int32_t ret = 0;
    ret = msgsnd(msg->srvid, buf, count, IPC_NOWAIT);

    LOG_DebugHex(buf, count, __FUNCTION__);

    if (ret < 0)
    {
        LOG_Error("%s errno:%s", __FUNCTION__, strerror(errno));
        return ret;
    }

    
    return count;
}

static uint32_t UdsCltPoll(struct UDSTpHandle *hdl)
{
    return RET_OK;
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

    int32_t msgid = msgget(key, IPC_CREAT | 0644);
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

static UDSSeqState_t AwaitPositiveResponse(UDSClient_t *client)
{
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (E_UDS_CLT_REQUEST_STATE_IDLE == client->state)
    {
        LOG_Debug("got positive response");
        return E_UDS_SEQ_STATE_GOTONEXT;
    }
    else
    {
        return E_UDS_SEQ_STATE_RUNNING;
    }
}

UDSSeqState_t RequestReadIdentify(struct UDSClient *client)
{
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    const uint16_t didList[] = {0xf1cb};
    UDSSendRDBI(client, didList, 1);
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

static UDSSeqState_t PrintTheData(UDSClient_t *client)
{
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    uint8_t buf[64];
    uint16_t offset = 0;
    UDSErr_t err;

    err = UDSUnpackRDBIResponse(client, 0xf1cb, buf, 4, &offset);
    if (err)
    {
        client->err = err;
        return E_UDS_SEQ_STATE_DONE;
    }

    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t EnterExtendSession(struct UDSClient *client)
{
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    UDSSendDiagSessCtrl(client, E_UDS_SESSION_TYPE_EXTENDED);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t RequestSecurityAccessSeed(struct UDSClient *client)
{
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    UDSSendSecurityAccess(client, 0x01, (void*) 0, 0);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    return E_UDS_SEQ_STATE_GOTONEXT;
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

UDSSeqState_t UnpackSecurityAccessSeed(struct UDSClient *client)
{
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    SecurityAccessResponse_t resp;
    UDSUnpackSecurityAccessResponse(client, &resp);

    uint32_t seed = ((resp.securitySeed[0] & 0xff) << 24) 
                    + ((resp.securitySeed[1] & 0xff) << 16) 
                    + ((resp.securitySeed[2] & 0xff) << 8) 
                    + (resp.securitySeed[3] & 0xff);
    UdsMsg_t *msg = (UdsMsg_t *)client->tp->impl;
    msg->seed = seed;

    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t SendSecurityAccessKey(struct UDSClient *client)
{
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    UdsMsg_t *msg = (UdsMsg_t *)client->tp->impl;
    uint8_t bytes[4] = {0x0,};
    uint32_t key = 0;
    key = udsSeed2Key(msg->seed, 0x19424F58);
    bytes[0] = (key >> 24) & 0xff;
    bytes[1] = (key >> 16) & 0xff;
    bytes[2] = (key >> 8) & 0xff;
    bytes[3] = key & 0xff;
    UDSSendSecurityAccess(client, 0x02, bytes, sizeof(bytes));
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    return E_UDS_SEQ_STATE_GOTONEXT;  
}
int main(int argc, char *argv[])
{
    UdsMsg_t msg;
    
    LOG_INIT("demo");
    if (UdsMsgInit(&msg) < 0)
    {
        LOG_Error("%s UdsMsgInit fail.", __FUNCTION__);
        return RET_ERR;
    }

    UDSTpHandle_t udsTphdl = 
    {
        .send = UdsCltSend,
        .recv = UdsCltRecv,
        .poll = UdsCltPoll,
        .impl = (void *) &msg
    };
    UDSClient_t client;
    UDSClientConfig_t cfg = 
    {
        .tp = &udsTphdl
    };

    if (UDSClientInit(&client, &cfg) < 0)
    {
        LOG_Error("%s UDSClientInit fail.", __FUNCTION__);
        UdsMsgDeinit(&msg);
        return RET_ERR;
    }

    static UDSClientCallback callbacks[] =
    {
        RequestReadIdentify,
        AwaitPositiveResponse,
        PrintTheData,

        EnterExtendSession, 
        AwaitPositiveResponse,

        RequestSecurityAccessSeed,
        AwaitPositiveResponse,
        UnpackSecurityAccessSeed,
        SendSecurityAccessKey,
        AwaitPositiveResponse,

        NULL
    };

    client.cbList = callbacks;
    client.cbIdx = 0;

    int running = 1;
    do
    {
        running = UDSClientPoll(&client);
        SleepMillis(10);
    }
    while (running);

    LOG_Debug("sequence completed %d callbacks with client err: %d", client.cbIdx, client.err);
    UDSClientDeInit(&client);
    UdsMsgDeinit(&msg);
    return 0;
}
