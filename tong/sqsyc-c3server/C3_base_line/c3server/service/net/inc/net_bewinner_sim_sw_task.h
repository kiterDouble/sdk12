/**
 * @file net_bewinner_sim_sw_task.h
 * @brief 北纬多网卡 SIM 卡切换任务对外接口
 *
 * 本头文件声明北纬外接多网卡切换任务的启动接口与切换接口，
 * 供 net_service.c 在加载 net.ini 配置后调用。
 */

#ifndef _NET_BEWINNER_SIM_SW_TASK_H_
#define _NET_BEWINNER_SIM_SW_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief 启动北纬多网卡切换后台任务
 *
 * 本函数会创建 GLib 工作线程，在线程内部完成：
 *   - 通过 ql_cmd 直接下发读卡信息 AT 指令
 *   - 解析 ICCID / 卡 ID
 *   - 进入等待状态，接收外部切换请求
 *
 * @return RET_OK 成功启动；RET_ERR 启动失败
 */
int BeWinnerSimTaskStart(void);

/**
 * @brief 请求切换到下一个运营商
 *
 * 按照 联通 -> 移动 -> 电信 -> 联通 的顺序自动切换下一个运营商。
 * 本接口为异步接口：把切换请求投递到任务队列后立即返回，
 * 实际切换动作由后台线程执行。调用前需确保任务已启动且已就绪。
 *
 * @return RET_OK 请求成功投递；RET_ERR 失败（任务未启动/未就绪）
 */
int BeWinnerSimSwitch(void);

#ifdef __cplusplus
}
#endif

#endif /* _NET_BEWINNER_SIM_SW_TASK_H_ */
