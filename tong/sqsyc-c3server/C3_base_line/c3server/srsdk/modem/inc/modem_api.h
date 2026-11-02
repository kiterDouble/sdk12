#ifndef __MODEM_API_H__
#define __MODEM_API_H__
#include "modem_ipc_msg.h"
#include "sr_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 从Modem服务获取IMSI（国际移动用户识别码）
 * @param imsi 指向存储IMSI字符串的缓冲区的指针。该缓冲区应由调用者分配。
 * @return 返回操作的结果状态码。RET_OK表示成功，其他值表示错误。
 */
int32_t ModemSrvGetImsi(uint8_t *imsi);

/**
 * @brief 从调制解调器服务获取ICCID（集成电路卡识别码）
 * @param iccid 指向接收ICCID的缓冲区的指针。该缓冲区应能容纳最长的ICCID字符串。
 * @return 返回操作的结果状态码。RET_OK表示成功，其他值表示错误。
 */
int32_t ModemSrvGetIccid(uint8_t *iccid);

/**
 * @brief 从Modem服务获取IMEI号
 * @param imei 用于存储从Modem服务获取到的IMEI号的缓冲区，IMEI号以字符串形式存储。
 * @return 返回操作的结果状态码。RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvGetImei(uint8_t *imei);

/**
 * @brief 获取飞行模式的状态,该函数用于通过远程过程调用（RPC）的方式，从Modem服务器获取当前的飞行模式状态。 
 * @param mode 指向一个uint8_t类型的变量的指针，用于接收飞行模式的状态。状态为0代表非飞行模式，为1代表飞行模式。
 * @return 返回一个int32_t类型的值。如果调用成功，返回RET_OK；如果调用失败，返回其他错误码。
 */
int32_t ModemSrvGetAirplaneMode(uint8_t *mode);

/**
 * @brief 设置飞行模式,该函数用于通过远程过程调用（RPC）的方式设置模组的飞行模式状态。
 * @param mode 飞行模式的状态，通常为0表示关闭，非0表示开启。
 * @return 返回操作的结果状态码。成功时返回RET_OK（假设为0），失败时返回其他错误码。
 */
int32_t ModemSrvSetAirplaneMode(uint8_t mode);

/**
 * @brief 获取Modem服务的QooS启用状态
 * @param enable 指向一个uint8_t类型的变量的指针，用于接收QoS是否启用的状态。如果启用了QooS，其值为1；否则为0。
 * @return 返回一个int32_t类型的值。如果成功获取了QooS启用状态，则返回RET_OK；否则返回RET_ERR。
 */
int32_t ModemSrvGetQoosEnable(uint8_t *enable);

/**
 * @brief 设置QoS使能状态,本函数用于通过RPC调用设置Modem服务的QooS使能状态。
 *        调用成功后，会根据输入的使能状态设置QooS。
 * @param enable 一个uint8_t类型的值，用于指定是否使能QoS。若为1，则表示使能QooS；若为0，则表示禁用QooS。
 * @return 返回一个int32_t类型的值。成功时返回RET_OK（具体定义见相关错误码），失败时返回其他错误码。
 */
int32_t ModemSrvSetQoosEnable(uint8_t enable);

/**
 * @brief 从Modem服务获取QoS配置。
 * @param qoosCfg 指向存储QoS配置信息的结构体的指针。
 * @return 函数执行状态，RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvGetQoosCfg(ModemSrvQoosCfg_t *qoosCfg);

/**
 * @brief 设置Modem的服务（QooS）配置。
 * @param qoosCfg 指向ModemSrvQoosCfg_t结构体的指针，包含了需要设置的QooS配置信息。
 * @return 返回操作的结果状态码。成功时返回RET_OK，错误时返回RET_ERR。
 */
int32_t ModemSrvSetQoosCfg(ModemSrvQoosCfg_t *qoosCfg);

/**
 * @brief 获取扩展信号信息,本函数用于通过调用MODEM服务器的RPC子命令获取扩展信号信息。函数会将获取到的信息
 *        存储到提供的sigInfo参数指向的结构体中。
 * @param sigInfo 指向ModemSrvExtSigInfo_t结构体的指针，用于存放获取到的扩展信号信息。
 * @return 返回执行结果的状态码。RET_OK表示成功，其他值表示错误。
 */
int32_t ModemSrvGetExtSignalInfo(ModemSrvExtSigInfo_t *sigInfo);
/**
 * @brief 获取当前Modem服务所连接的运营商信息。
 * @param provider 指向接收运营商名称的缓冲区的指针。该缓冲区应由调用者分配。
 *                 运营商名称将被复制到此缓冲区。
 * @return 返回操作的结果状态码。RET_OK表示成功，其他值表示错误。
 *         如果成功，operator参数将被填充为运营商名称。
 */
int32_t ModemSrvGetOperator(uint8_t*provider);
/**
 * @brief 通过调用MODEM服务发送短信消息 
 * @param phoneNum 发送短信的目标手机号码，类型为uint8_t*，指向存储手机号码字符串的内存地址
 * @param textContent 要发送的短信内容，类型为uint8_t*，指向存储短信文本的内存地址
 * @param textLen 短信内容的长度，类型为uint32_t，指定textContent中有效字符的数量
 * @return 函数执行状态，成功返回RET_OK，失败返回RET_ERR
 */
int32_t ModemSrvSendSmsMesg(uint8_t* phoneNum, uint8_t* textContent, uint32_t textLen);

/**
 * @brief 启动呼叫,该函数用于通过调制解调器服务发起一个电话呼叫。
 * @param phoneNum 指向要拨打的电话号码的指针。电话号码应为字符串格式。
 * @param callId 指向接收呼叫ID的指针。呼叫ID将在呼叫成功启动时被填充。
 * @return 函数返回一个整型值，表示操作的结果状态。RET_OK表示成功，其他值表示错误。
 */
int32_t ModemSrvStartCall(uint8_t* phoneNum, int32_t *callId);

/**
 * @brief 接听来电，该函数用于处理Modem服务的来电接听请求。它通过RPC（远程过程调用）同步方式发送接听来电的指令，
 *        并等待响应以获取操作结果。
 * @param callId 拨打的电话ID，类型为int32_t。
 * @return 函数执行结果，成功返回RET_OK（假设为正数），失败返回RET_ERR（假设为负数）。
 */
int32_t ModemSrvAcceptCall(int32_t callId);

/**
 * @brief 挂断电话函数
 * @param callId 呼叫的ID，用于标识要停止的呼叫
 * @return 返回操作的结果状态码。成功时返回RET_OK，错误时返回其他错误码。
 */
int32_t ModemSrvStopCall(int32_t callId);

/**
 * @brief 反初始化模式服务作业，该函数用于向模组服务器发送反初始化作业的请求，并同步等待响应，最终返回操作结果。
 * @param idx 作业索引标识，用于指定需要反初始化的作业。
 * @return int32_t 操作结果，成功返回RET_OK，失败返回RET_ERR。
 */
int32_t ModemSrvDeinitJob(uint8_t idx);

/**
 * @brief 从调制解调器服务获取电话号码（集成电路卡识别码）
 * @param iccid 指向接收phoneNum的缓冲区的指针。该缓冲区应能容纳最长的phoneNum字符串。
 * @return 返回操作的结果状态码。RET_OK表示成功，其他值表示错误。
 */
int32_t ModemSrvGetPhoneNum(uint8_t *phoneNum);

/**
 * @brief 获取Modem CPU温度和占用率
 * @param cpuInfo 指向ModemSrvCpuInfo_t结构体的指针，用于接收CPU温度和占用率。
 * @return 返回操作的结果状态码。RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvGetCpuInfo(ModemSrvCpuInfo_t *cpuInfo);
#ifdef __cplusplus
}
#endif
#endif