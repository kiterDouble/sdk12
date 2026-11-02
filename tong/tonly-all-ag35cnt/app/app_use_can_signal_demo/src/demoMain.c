#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "sr_ipc.h"
#include "err_code.h"

#include "can_signal_header_tonly.h"

TonlyAllCanSignal_canVal_t g_canSignal;
static pthread_rwlock_t g_canSignalDataRwlock = PTHREAD_RWLOCK_INITIALIZER;

/**
 * @brief Get the Can Signal object
 *
 * @param outData
 * @return int
 */
int GetCanSignal(TonlyAllCanSignal_canVal_t *outData)
{
    pthread_rwlock_wrlock(&g_canSignalDataRwlock);
    memcpy(outData, &g_canSignal, sizeof(TonlyAllCanSignal_canVal_t));
    pthread_rwlock_unlock(&g_canSignalDataRwlock);
    return RET_OK;
}

/**
 * @brief IPC回调：获取电源管理数据
 * @param
 * @return
 */
static void CanSignalCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    if (NULL == recvData)
    {
        printf("%s() argv is invalid\n", __FUNCTION__);
        return;
    }
    if (dataLen != sizeof(TonlyAllCanSignal_canVal_t))
    {
        printf("%s() argv is invalid\n", __FUNCTION__);
        return;
    }
    printf("%s(), recv DataLen:%d, type:%08x\n", __FUNCTION__, dataLen, (SYSQueMsgType_e)TonlyAllCanSignal_canVal_t_msgid);

    const TonlyAllCanSignal_canVal_t *tmpRecvData = (const TonlyAllCanSignal_canVal_t *)recvData;
    pthread_rwlock_wrlock(&g_canSignalDataRwlock);
    memcpy(&g_canSignal, tmpRecvData, sizeof(TonlyAllCanSignal_canVal_t));
    pthread_rwlock_unlock(&g_canSignalDataRwlock);
}

int main(int argc, char *argv[])
{
    // IPC初始化
    StartQueueMsgRcvThread("use_can_signal_demo");
    printf("IPC init status:%d\n",IpcIsInitialized());
    // IPC数据订阅
    SubscriptionQueueMsg((SYSQueMsgType_e)TonlyAllCanSignal_canVal_t_msgid, CanSignalCallback, NULL);

    while (1)
    {
        sleep(1);

        pthread_rwlock_wrlock(&g_canSignalDataRwlock);

        printf("Q2V_SOC: %f\n", g_canSignal.Q2V_SOC);
        printf("BCM_SOC: %f\n", g_canSignal.BMS_SOC);
        printf("Gear: %u\n", g_canSignal.gear);
        printf("VehicleSpeed: %f\n", g_canSignal.vehicle_speed);
        printf("AccPedal: %f\n", g_canSignal.acc_pedal);
        printf("BrakePedal: %f\n", g_canSignal.brake_pedal);
        printf("PKB_Status: %u\n", g_canSignal.PKB_status);
        printf("KeyPosition: %u\n", g_canSignal.key_position);
        printf("BatteryVoltage: %f\n", g_canSignal.battery_voltage);
        printf("EngineSpeed: %f\n", g_canSignal.engine_speed);
        printf("MotorSpeed: %d\n", g_canSignal.motor_speed);
        printf("CtrlBusVoltage1: %f\n", g_canSignal.controller_bus_voltage_1);
        printf("CtrlBusVoltage2: %f\n", g_canSignal.controller_bus_voltage_2);
        printf("\n");

        pthread_rwlock_unlock(&g_canSignalDataRwlock);
    }

    UnSubscriptionQueueMsg((SYSQueMsgType_e)TonlyAllCanSignal_canVal_t_msgid);
}
