#ifndef _TERMINAL_MANAGEMENT_DATA_REPORT_H
#define _TERMINAL_MANAGEMENT_DATA_REPORT_H

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 判断任务ID是否被取消
     * @param cmdId
     * @return
     */
    bool IsCanceledTask(const char *cmdId);

    /**
     * @brief 移除已取消的任务ID
     * @param cmdId
     */
    void RemoveCanceledTask(const char *cmdId);

    /**
     * @brief 向命令队列推送序列化后的命令数据（拷贝）
     * @param serialized_data 序列化后的 S3Command 二进制数据
     * @param size 数据长度
     */
    void PushCommandToQueue(const void *serialized_data, size_t size);

    /**
     * @brief 开始数据处理
     *
     * @return int
     */
    int DataProcessingStart();

    /**
     * @brief 结束数据处理
     *
     * @return int
     */
    int DataProcessingStop();

#ifdef __cplusplus
}
#endif

#endif // _TERMINAL_MANAGEMENT_DATA_REPORT_H
