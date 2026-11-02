
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

#define BYTES_TO_UINT16(b0, b1) \
    (((uint16_t)(b0) << 8) | (uint16_t)(b1))

#define BYTES_TO_UINT32(b0, b1, b2, b3) \
    (((uint32_t)(b0) << 24) | ((uint32_t)(b1) << 16) | ((uint32_t)(b2) << 8) | (uint32_t)(b3))

static void LBATSyncCallBack1(const uint8_t* data, const uint16_t len, void* userdata)
{
    uint8_t state = 0;             /* 电池工作状态 */
    uint16_t voltage = 0;          /* 备用电池电压 单位 mV */
    int16_t temp = 0;              /* 备用电池温度 */
    float tempf = 0;              /* 备用电池温度 */
    uint8_t soc = 0;               /* 电池电量百分比 */
    uint8_t voltageAlarm = 0;      /* 电压异常告警 */
    uint8_t tempAlarm = 0;         /* 温度异常告警 */
    uint8_t status = 0;            /* 电池连接状态 */
    uint32_t chargingDuration = 0; /* 备用电池充电时长 获取，单位 s */

    if (data == NULL || len < 13)
    {
        LOG_Debug("%s called, len:%u, data invalid.", __FUNCTION__, len);
        return;
    }

    /* data[0]: 电池工作状态 0: (idle 状态) 1: (充电状态) 2: (使能/放电状态) */
    state = data[0];

    /* data[1..2]: 备用电池电压 单位 mV */
    voltage = BYTES_TO_UINT16(data[1], data[2]);

    /* data[3..4]: 备用电池温度 0.1°C 单位  */
    temp = (int16_t)BYTES_TO_UINT16(data[3], data[4]);
    tempf = (temp - 400)*0.1f;

    /* data[5]: BatterySdkGetBatteySoc() -> 备用电池电量百分比，单位 % */
    soc = data[5];

    /* data[6]: 备用电池电压异常告警 0: normal 1: 低压警告 2: 高压警告 */
    voltageAlarm = data[6];

    /* data[7]: 备用电池温度异常告警 0: normal 1: 低温警告 2: 高温警告 */
    tempAlarm = data[7];

    /* data[8]: 备用电池连接状态 0: 已连接  1: 未连接 */
    status = data[8];

    /* data[9..12]: 备用电池充电时长 */
    chargingDuration = BYTES_TO_UINT32(data[9], data[10], data[11], data[12]);

    LOG_Debug("%s called, len:%u, state:%u, voltage:%u mV, temp:%0.1f(C), soc:%u%%, voltageAlarm:%u, tempAlarm:%u, status:%u, chargingDuration:%us",
              __FUNCTION__, len,
              state,
              voltage,
              tempf,
              soc,
              voltageAlarm,
              tempAlarm,
              status,
              chargingDuration);
}

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
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_LBAT, LBATSyncCallBack1, NULL);//将用户参数传入

    while(1)
    {
        sleep(1);
    }
}