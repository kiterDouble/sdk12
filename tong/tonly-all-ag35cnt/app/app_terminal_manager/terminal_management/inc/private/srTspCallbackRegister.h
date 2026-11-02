#ifndef SR_TSP_CALLBACK_REGISTER_H
#define SR_TSP_CALLBACK_REGISTER_H
#include "terminal_management_data.h"
#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 注册 TSP 基础信息回调
     *
     * @param[in] cbs 回调结构体指针
     * @return 0 注册成功
     * @return -1 注册失败（cbs 为 NULL 或字段无效）
     */
    int TspBaseInfoRegisterCallbacks(const StBaseInfoCallbacks_t *cbs);

    /**
     * @brief 获取当前已注册的 TSP 回调
     *
     * @param[out] cbsOut 输出参数，保存当前注册的回调结构体
     * @return 0 获取成功
     * @return -1 获取失败（未注册或 cbsOut 为 NULL）
     */
    int TspBaseInfoGetCallbacks(StBaseInfoCallbacks_t *cbsOut);

    /**
     * @brief 检查当前注册的回调结构体是否全部有效（非 NULL）
     *
     * @return 0 全部字段有效
     * @return -1 存在空指针或未注册
     */
    int TspBaseInfoCheckCallbacksValid(void);

    /**
     * @brief 获取TBOX基础信息-SN
     * 主要用于topic拼接
     * @param buffer
     * @param len
     * @return int
     */
    int TspBaseInfoGetSn(char *buffer, int len);

#ifdef __cplusplus
}
#endif
#endif // SRTSPCALLBACKREGISTER_H
