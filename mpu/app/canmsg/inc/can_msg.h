/*
 * Filename: CanMsg.h
 * Created Date: Wednesday, September 3rd 2025, 6:13:07 pm
 * Author: shiping.sun@carota.ai
 * Company: Carota.ai
 * Copyright (c) 2025 Carota.ai
 * Description: CAN消息模块头文件
 */

#ifndef __CAN_MSG_H__
#define __CAN_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CAN_MAX_PAYLOAD_LEN 8
#define CANFD_MAX_PAYLOAD_LEN 64
#define CAN_MSG_MAX_PAYLOAD_LEN CANFD_MAX_PAYLOAD_LEN
//can总线定义
typedef enum
{
    CAN_BUS_0 = 0,
    CAN_BUS_1,
    CAN_BUS_2,
    CAN_BUS_3,
    CAN_BUS_4,
    CAN_BUS_5,
    CAN_BUS_6,
    CAN_BUS_MAX
} CanBusType;

//错误类型定义
typedef enum
{
    CAN_ERROR_NONE = 0,
    CAN_ERROR_BUS_OFF,                                  //CAN总线关闭
    CAN_ERROR_OVERRUN,                                  //CAN消息溢出
    CAN_ERROR_TIMEOUT,                                  //超时
    CAN_ERROR_FILTER,                                   //过滤器设置错误
    CAN_ERROR_HARDWARE,                                 //硬件异常
    CAN_ERROR_INVALID_DLC,                              //无效数据长度
    CAN_ERROR_UNKNOWN                                    //未知无错误
}CanError_t;

//CAN消息定义
typedef struct
{
    CanBusType bus;                                     //CAN总线类型
    uint32_t id;                                        //CAN ID
    uint8_t data[CAN_MSG_MAX_PAYLOAD_LEN];              //数据域
    uint8_t dlc;                                        //CAN数据长度
    uint8_t is_fd;                                      //是否为CANFD消息（0：CAN消息，1：CANFD消息）
    uint64_t timestamp;                                 //时间戳,精确到微秒
} CanMsg_t;

typedef struct {
    CanBusType bus;                                     //CAN总线类型
    uint32_t* ids;                                      //CAN ID列表
    uint32_t id_count;                                  //CAN ID列表长度
} CanFilter_t;

typedef void (*CanMsgRecvIndicationCbk)(CanMsg_t* msgs, uint32_t msg_count);
typedef void (*CanErrorIndicationCbk)(CanError_t error);

typedef struct {
    CanFilter_t* filters;                               //CAN过滤器列表
    uint32_t filter_count;                              //CAN过滤器列表长度
    uint32_t buffer_size;                               //CAN消息最大接收缓存大小
    uint32_t flush_cycle;                               //CAN消息刷新周期,单位ms
    CanMsgRecvIndicationCbk recv_indication_cbk;        //CAN消息接收回调函数
    CanErrorIndicationCbk error_indication_cbk;         //CAN错误回调函数
} CanMsgConfig_t;


/**
 * @brief CAN消息模块初始化
 * @param config CAN消息配置参数
 * @return 0:成功, 其他:失败
 */
int CanMsg_Init(const CanMsgConfig_t* config);

/**
 * @brief CAN消息模块启动接收
 * @return 0:成功, 其他:失败
 */
int CanMsg_Start(void);

/**
 * @brief CAN消息模块停止接收
 * @return 无
 */
void CanMsg_Stop(void);

/**
 * @brief CAN消息发送函数
 * @param msg CAN消息结构体指针
 * @return 0:成功, 其他:失败
 */
int CanMsg_Transmit(CanMsg_t* msg);

/**
 * @brief CAN消息模块反初始化
 * @return 无
 */
void CanMsg_Deinit(void);


#ifdef __cplusplus
}
#endif

#endif /* __CAN_MSG_H__ */