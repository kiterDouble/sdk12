#ifndef __DOIP_TESTER_H__
#define __DOIP_TESTER_H__
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define DOIP_TP_IPADDR_LEN (16)

typedef enum
{
    E_DOIP_TESTER_CONNECT_FAIL = 0x0,//连接失败
    E_DOIP_TESTER_CONNECT_SUCCESS,//连接成功
    E_DOIP_TESTER_DISCONECTED,//连接断开
}DoipTesterConnectStatus_e;

typedef void* DoipTesterHandle_t;//tester操作句柄

/**
 * @name DoipTesterConnNotifyCb
 * @brief tester连接回调
 * @param targetLa 输入参数，目标逻辑地址 
 * @param status 输入参数，连接状态
 * @return 函数无返回值
 */
typedef void (*DoipTesterConnNotifyCb)(uint32_t targetLa, int32_t status);

/**
 * @name DoipTesterTxAckCb
 * @brief 诊断消息发送确认回调
 * @param targetLa 输入参数，目标逻辑地址     
 * @param payload 输入参数，接受到的负载数据
 * @param err 输入参数，Ack确认
 * @return 函数无返回值
 */
typedef void (*DoipTesterTxAckCb)(uint32_t targetLa, int32_t status); 

/**
 * @name DoipTesterUdsRxNotifyCb
 * @brief UDS数据接收回调
 * @param targetLa 输入参数，目标逻辑地址
 * @param payload 输入参数，接受到的负载数据
 * @return 函数无返回值
 */
typedef	void (*DoipTesterUdsRxNotifyCb)(uint32_t targetLa, const uint8_t* udsData, uint32_t udsDataLen);

/**
 * @name DoipTesterOnlineNotifyCb
 * @brief 节点上线通知回调，注意该回调函数内不允许调用tester提供的外部函数
 * @param targetLa 输入参数，目标逻辑地址
 * @return 函数无返回值
 */
typedef	void (*DoipTesterOnlineNotifyCb)(uint32_t targetLa); 

typedef struct {
    DoipTesterConnNotifyCb conFunc;//连接回调
    DoipTesterTxAckCb txFunc;//诊断数据发送确认回调
    DoipTesterUdsRxNotifyCb rxFunc;//数据接收回调
    DoipTesterOnlineNotifyCb onlineFunc;//节点上线通知回调
} DoipTesterCbs_t;

typedef struct 
{
    uint8_t ipAddr[DOIP_TP_IPADDR_LEN];//目标节点ip地址
    uint32_t activeType;//激活类型
    uint32_t functionalLa;//功能寻找
    uint32_t sourceLa;//tester逻辑地址
    uint32_t targetLa;//目标节点逻辑地址
    uint32_t maxPL;//支持最大的playload长度
}DoipTesterNode_t;

/**
 * @name DoipTesterInit
 * @brief 初始化DOIP测试设备。
 * @param hdl 指向DoipTesterHandle_t的指针，用于储存初始化后的测试设备句柄。
 * @param funs 指向DoipTesterCbs_t的指针，包含了测试设备需要的回调函数。
 * @return 成功返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipTesterInit(DoipTesterHandle_t *hdl, const DoipTesterCbs_t *funs);

/**
 * @name DoipTesterDeinit
 * @brief 对DOIP测试设备进行反初始化
 *        该函数用于释放DOIP测试设备在初始化时分配的资源，包括网络连接、事件基等。
 * @param hdl DOIP测试设备的句柄，用于标识和访问测试设备的私有数据。
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t DoipTesterDeinit(DoipTesterHandle_t hdl);

/**
 * @name DoipTesterConnect
 * @brief 连接到DOIP测试设备
 * 本函数用于通过提供的节点信息，建立到DOIP测试设备的连接。
 * @param hdl DOIP测试设备的句柄，用于标识和访问测试设备的私有数据。
 * @param node 指向包含要连接的节点信息的结构体的指针。
 * @return 返回连接操作的结果状态码。成功时返回RET_OK，错误时返回RET_ERR。
 */
int32_t DoipTesterConnect(DoipTesterHandle_t hdl, const DoipTesterNode_t *node);

/**
 * @name DoipTesterUdsRequest
 * @brief 执行UDS请求的函数
 *        该函数用于通过DOIP（Diagnostic Over IP）协议向目标设备发送一个UDS（Unified Diagnostic Services）请求。
 * @param hdl DoipTester的句柄，用于标识和访问测试设备的私有数据。
 * @param targetLa 目标逻辑地址，标识要向哪个网络节点发送请求。
 * @param udsData 指向包含UDS请求数据的缓冲区的指针。
 * @param udsDataLen UDS请求数据的长度。
 * @return 返回函数执行状态码。成功时返回RET_OK，错误时返回RET_ERR。
 */
int32_t DoipTesterUdsRequest(DoipTesterHandle_t hdl, uint32_t targetLa, const uint8_t *udsData, uint32_t udsDataLen);

/**
 * @name DoipTesterDisconnect
 * @brief 断开DOIP测试设备的连接。
 * @param hdl DoipTesterHandle_t 类型，表示DOIP测试设备的句柄。
 * @return 返回操作的结果状态码。成功时返回RET_OK，失败时返回RET_ERR。
 */
int32_t DoipTesterDisconnect(DoipTesterHandle_t hdl);
#ifdef __cplusplus
}
#endif
#endif