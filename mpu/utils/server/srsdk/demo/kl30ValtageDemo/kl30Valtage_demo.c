
#include <unistd.h>
#include "sr_log.h"
#include "err_code.h"
#include "sr_ipc.h"

typedef struct
{
    uint8_t kl15status;
    uint32_t kl30voltage;
    uint16_t fastCharge;
}HardLineSyncInfo_t;

static void HardLineSyncCallBack1(const uint8_t* data, const uint16_t len, void* userdata)
{
    HardLineSyncInfo_t hardLine;

    hardLine.kl15status = data[0];
    hardLine.kl30voltage = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
    hardLine.fastCharge = (data[5] << 8) | data[6];
    LOG_Debug("%s called, kl30valtage:%u mv.", __FUNCTION__, hardLine.kl30voltage);
}

int32_t main(void)
{
    // uint8_t buffer[128] = {0x0,};
    // int32_t ret;
    LOG_INIT("demo");

    StartQueueMsgRcvThread("test_kl30");
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_KLINE, HardLineSyncCallBack1, NULL);//将用户参数传入

    while(1)
    {
        sleep(1);
    }
}