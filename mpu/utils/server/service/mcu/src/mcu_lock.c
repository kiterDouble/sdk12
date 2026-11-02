#include "mcu_lock.h"

pthread_mutex_t g_mcuSendLock = PTHREAD_MUTEX_INITIALIZER;
static uint8_t g_sendDataFlag = 0;

void setSendDataFlag(uint8_t flag)
{
    pthread_mutex_lock(&g_mcuSendLock);
    g_sendDataFlag = flag;
    pthread_mutex_unlock(&g_mcuSendLock);
}

uint8_t getSendDataFlag()
{
    uint8_t flag = 0;

    pthread_mutex_lock(&g_mcuSendLock);
    flag = g_sendDataFlag;
    pthread_mutex_unlock(&g_mcuSendLock);

    return flag;
}