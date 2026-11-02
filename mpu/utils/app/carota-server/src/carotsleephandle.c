#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "pm_server_api.h"
#include "carotsleephandle.h"
#include "sr_log.h"
#include "carotaHttpClient.h"

static volatile guint8 carota_pmState = PM_STATE_SLEEP;
static pthread_rwlock_t carota_PmStateMutex = PTHREAD_RWLOCK_INITIALIZER;

uint8_t CarotaGetPmState()
{
    pthread_rwlock_rdlock(&carota_PmStateMutex);
    uint8_t pmState = carota_pmState;
    pthread_rwlock_unlock(&carota_PmStateMutex);
    return pmState;
}

void CarotaSetPmState(uint8_t pmState)
{
    pthread_rwlock_wrlock(&carota_PmStateMutex);
    carota_pmState = pmState;
    LOG_Info("%s, carota_pmState:%d", __FUNCTION__, carota_pmState);
    pthread_rwlock_unlock(&carota_PmStateMutex);
}

void* carotaTimerThread(void* arg)
{

    LOG_Debug("carotaTimerThread start ");
    int carotasleepfla = 0;
    while (1)
    {

        if(CarotaGetPmState() == PM_STATE_SLEEP && carotasleepfla == 0)
        {
            LOG_Debug("%s ,system is sleeping, send wake request",__FUNCTION__);
            NotifyFOTASleepMode(0);  // 0表示唤醒模式
            carotasleepfla = 1;
        }

        if(CarotaGetPmState() == PM_STATE_NORMAL && carotasleepfla == 1)
        {   
            LOG_Debug("%s system is normal, send sleep request",__FUNCTION__);
            NotifyFOTASleepMode(1);  // 1表示休眠模式
            SendDOTAChangeDomainRequest();
            carotasleepfla = 0;
        }
        LOG_Debug("%s GetPmState : %d  ",__FUNCTION__,CarotaGetPmState());
        sleep(1);
    }

    return NULL;
}