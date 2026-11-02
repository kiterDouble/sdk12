#ifndef _TERMINAL_MANAGEMENT_DATA_PACKAG_H
#define _TERMINAL_MANAGEMENT_DATA_PACKAG_H

#include <stdio.h>
#include "terminal_management_data_monitor.h"
#include "srTspFWMqttClient.h"
#include "srTspFWOs.h"
#include <sqlite3.h>

#define MAX_RECORDS_PER_REISSUE_DB (5000)                          // 每个补发数据库的最大记录数
#define MAX_REISSUE_BACKUP_TOTAL_BYTES (500LL * 1024 * 1024)       // 补发数据备份文件最大可存储容量：500MB
#define MAX_HISTORY_FAULT_BACKUP_TOTAL_BYTES (100LL * 1024 * 1024) // 历史故障备份文件最大可存储容量：100MB

typedef struct
{
    /*最高优先级*/
    SRQueue_t *P0; // 故障类<触发/恢复>数据
    SRQueue_t *P1; // 异常类<触发/恢复>数据
    SRQueue_t *P2; // 正常类<触发/恢复>数据
    SRQueue_t *P3; // 周期数据
    SRQueue_t *P4; // 补发数据
    /*最低优先级*/
} SendQueueCollection_t;

typedef struct
{
    uint8_t data[1024];
    size_t len;
    int priority;
} LoadData_t;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 创建终端管理平台的发送队列
     *
     * @return int
     */
    int InitSendQueue(void);

    /**
     * @brief 销毁终端管理平台的所有发送队列
     */
    void DeinitSendQueue(void);

    /**
     * @brief 获取待发送队列中暂存的数据条数
     *
     * @param queueIdx 队列优先级
     * @return int
     */
    int GetQueueDataCnt(int queueIdx);

    /**
     * @brief 获取待发送队列中的数据
     *
     * @param loadData buff缓存待发送队列中的数据
     * @param buffLen buff数据长度
     * @param queueIdx 队列优先级
     * @return int
     */
    int GetQueueData(LoadData_t *loadData, int buffLen, int queueIdx);

    /**
     * @brief 向待发送队列中插入数据
     *
     * @param loadData 待发送的数据
     * @param queueIdx 队列优先级
     * @return int
     */
    int AddQueueData(LoadData_t loadData, int queueIdx);

    /**
     * @brief 向指定数据库添加一条数据
     *
     * @param loadData 待缓存的数据
     * @param type     优先级 0:P0故障 1:P1异常 2:P2正常 3:P3周期（仅在 dbType=0 时生效）
     * @param dbType   数据库类型：0-补发数据库，1-历史故障数据库
     */
    void CacheAdd(LoadData_t loadData, int type, int dbType);

    /**
     * @brief 从时间戳最早的数据库（包含当前库及所有备份库）中取一条补发数据
     * @details 按优先级 P0 > P1 > P2 > P3 遍历所有数据库文件，优先从高优先级、最早的文件中获取数据。
     * @param loadData
     * @return RET_OK: 数据库无数据或获取到实际数据(需自行根据入参进行判断)，RET_ERR: 操作出错
     */
    int CacheGet(LoadData_t *loadData);

    /**
     * @brief 初始化所有数据库（补发数据库 + 历史故障数据库）
     *
     * @param dbFolder 数据库存放目录
     */
    void InitAllDatabases(const char *dbFolder);

    /**
     * @brief 关闭所有数据库（补发 + 历史故障）
     */
    void CloseAllDatabases(void);

    /**
     * @brief 终端管理平台周期数据组包
     *
     * @param data
     */
    void PeriodicPack(MonitoringData_t rawData);

    /*<start>/<end>事件组包函数-1期*/
    void BatteryLowVoltageTriggerEventPack(const MonitoringData_t data);
    void BatteryLowVoltageRecoveryEventPack(const MonitoringData_t data);

    void GpsSignalLostTriggerEventPack(const MonitoringData_t data);
    void GpsSignalLostRecoveryEventPack(const MonitoringData_t data);

    void CanBusCommInterruptTriggerEventPack(const MonitoringData_t data);
    void CanBusCommInterruptRecoveryEventPack(const MonitoringData_t data);

    void VehicleUnexpectedHvPowerDownTriggerEventPack(const MonitoringData_t rawData);
    void VehicleUnexpectedHvPowerDownRecoveryEventPack(const MonitoringData_t rawData);

    void VehicleCanCommFailTriggerEventPack(const MonitoringData_t data);
    void VehicleCanCommFailRecoveryEventPack(const MonitoringData_t rawData);

    void GpsModuleFaultTriggerEventPack(const MonitoringData_t data);
    void GpsModuleFaultRecoveryEventPack(const MonitoringData_t rawData);

    void BatteryChargeFailTriggerEventPack(const MonitoringData_t rawData);
    void BatteryChargeFailRecoveryEventPack(const MonitoringData_t rawData);

    void VehicleStartTriggerEventPack(const MonitoringData_t data);

    void VehicleSpeedGreater0TriggerEventPack(const MonitoringData_t data);

    void VehicleSpeedGreater100TriggerEventPack(const MonitoringData_t data);

    /*<start>/<end>事件组包函数-2期*/
    // 暂无

#ifdef __cplusplus
}
#endif

#endif // _TERMINAL_MANAGEMENT_DATA_PACKAG_H
