#ifndef __MODEM_DATA_H__
#define __MODEM_DATA_H__
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <glib.h>
#include "hal_modem_sim.h"
#include "hal_modem_nw.h"
#include "hal_modem_ant.h"
#include "hal_modem_call.h"
#include "hal_modem_sms.h"
#include "hal_modem_dms.h"
#include "modem_ipc_msg.h"
#include "err_code.h"
#include "sr_log.h"
#include "sr_ipc.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t (*ModemSrvCallback)(void);

typedef enum
{
    E_MODEM_SRV_JOB_STATUS_UNINITIALED = 0x0,
    E_MODEM_SRV_JOB_STATUS_INITIALED
}ModemSrvJobStatus_e;

typedef struct
{
    ModemSrvJobStatus_e status;
    ModemSrvCallback initCb;
    ModemSrvCallback deinitCb;
}ModemSrvJob_t;

typedef struct
{
    //ant
    int32_t antShort; 
    int32_t antNormal;
    int32_t antNormalMax;
    int32_t antOpen;
    //sec
    int32_t secAntShort; 
    int32_t secAntNormal;
    int32_t secAntNormalMax;
    int32_t secAntOpen;
}ModemIniConfig_t;

typedef struct
{
    pthread_mutex_t mutex;
    ModemSrvNotify_t notify;
    ModemIniConfig_t cfg;
    ModemSrvJob_t jobs[E_MODEM_SRV_JOB_TYPE_MAX];
}ModemSrvData_t;

typedef int32_t (*ModemSrvRpcSubProcCallback)(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

typedef struct
{
    ModemSrvRpcSubCmd_e sub;
    ModemSrvRpcSubProcCallback procCb;
}ModemSrvSubProc_t;

#ifdef __cplusplus
}
#endif
#endif