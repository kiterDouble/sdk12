/**
 * @file can_parser.h
 * @author jiajiangze (jiangze.jia@sirun.net)
 * @brief 
 * @version 0.1
 * @date 2023-12-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef _UPGRADE_CAN_PARSER_H_
#define _UPGRADE_CAN_PARSER_H_
#include "common.h"
#include "data_pool.h"
#include "mcu_api.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define CAN_ENCODE_INTEL (0)//小端
#define CAN_ENCODE_MOTOROLA_MSB (1)//大端MSB
#define CAN_ENCODE_MOTOROLA_LSB (2)//大端LSB 大端MSB和LSB区别见https://blog.csdn.net/CJohn1994/article/details/124105213
#define CAN_ENCODE_NONE (3)

typedef struct 
{    
    uint8_t ch;//channel id
    uint32_t id;//can id
    uint8_t data[8];//can original data.
}Cp_Ori_Data_t,*P_Cp_Ori_Data_t;

typedef struct
{
    uint8_t type;//intel or motolLSB or motolMSB
    uint8_t ch;//channel id
    uint32_t id;//can id
    uint8_t u8MatrixBuf[64];//matrix bytes.
}Cp_Matrix_Data_t,*P_Cp_Matrix_Data_t;

/**
 * @brief 订阅mcu CAN数据透传,a02m02s00 IPC回调函数
 * 
 * @param data tbox mcu-mpu协议 Service Data 地址
 * @param len tbox mcu-mpu协议 Service Data 长度
 * @param userdata ipc 用户数据
 */
void RecvMcuCANMsgCallBack(const uint8_t* data, const uint16_t len, void* userdata);

/**
 * @brief 订阅mcu MCU向CPU请求透传CAN ID配置信息,a02m01s01 IPC回调函数
 * 
 * @param data tbox mcu-mpu协议 Service Data 地址
 * @param len tbox mcu-mpu协议 Service Data 长度
 * @param userdata ipc 用户数据
 */
void RecvMcuCanParserFilterReqCallBack(const uint8_t* data, const uint16_t len, void* userdata);

/**
 * @brief 解析CAN原始数据为相对应的信号值
 * 
 * @param ch can通道
 * @param id canid
 * @param data can数据
 * @return int32_t RET_OK:success RET_ERR:failed
 */
int32_t CanParserRecvCanData(uint8_t ch, uint32_t id, uint64_t data);


void *CanParserProcess(void *arg);


void *RecvCanDataTimeoutProcess(void *arg);


#ifdef __cplusplus
}
#endif
#endif
