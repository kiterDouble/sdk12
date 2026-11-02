#ifndef _UDS_EOL_TEST_SYNC_WITH_CPU_H_
#define _UDS_EOL_TEST_SYNC_WITH_CPU_H_

#include <stdint.h>
#include "stdlib.h"

#define SYNC_CPU_CONTROL_ITEM_TEST_MODE 1
#define SYNC_CPU_CONTROL_ITEM_APN_PING 2

#define SYNC_CPU_CONTROL_ITEM_WIFI_CONNECT 4

#define SYNC_CPU_CONTROL_ITEM_BLUETOOTH_CONNECT 6

#define SYNC_CPU_CONTROL_ITEM_CALL_TEST 8
#define SYNC_CPU_CONTROL_ITEM_SET_BLUE_TOOTH_NAME 10

#define SYNC_CPU_GET_INFO_ITEM_APN_PING_RESULT 3
#define SYNC_CPU_GET_INFO_ITEM_WIFI_CONNECT_RESULT 5
#define SYNC_CPU_GET_INFO_ITEM_BLUETOOTH_CONNECT_RESULT 7
#define SYNC_CPU_GET_INFO_ITEM_CALL_TEST_RESULT 9
#define SYNC_CPU_GET_INFO_ITEM_BLUE_TOOTH_NAME 11
#define SYNC_CPU_GET_INFO_ITEM_FLASH_CAPACITY 45
#define SYNC_CPU_GET_INFO_ITEM_4G_SIGNAL_VALUE 40
// #define SYNC_CPU_GET_INFO_ITEM_PIN_ECALL_STATE          2
#define SYNC_CPU_GET_INFO_ITEM_NET_TYPE 41
#define SYNC_CPU_GET_INFO_ITEM_APN_NUMBER 42
#define SYNC_CPU_GET_INFO_ITEM_NET_PROVIDER 43
#define SYNC_CPU_GET_INFO_ITEM_GNSS 44

#define SYNC_CPU_GET_INFO_ITEM_PKI_STATUS 46 // B262_cxl获取PKI状态的命令ID

int16_t EolTestSyncWithCpuGetPkiStatus(uint8_t *pPkiStatus); // B262_cxl

void EolTestSyncWithCpuInit(void);

int16_t EolTestSyncWithCpuTransmit(uint8_t item, uint8_t *pData, uint16_t length);
int16_t EolTestSyncWithCpuRecv(uint8_t item, uint8_t *pData, uint16_t *pLength);
/*******************************************************************************
 * Function:       CanPassthrough_RequestAndGetResponse
 * Description:    将UDS请求转发给MPU，并同步等待MPU的UDS响应
 * Input:          pUdsRequest:  UDS请求数据
 * reqLength:    请求数据的长度
 * Output:         pUdsResponse: 用于存储MPU返回的UDS响应的缓冲区
 * pRespLength:  用于存储响应数据长度的指针
 * Return:         0: 成功获取响应, -1: 失败或超时
 *******************************************************************************/
int16_t CanPassthrough_RequestAndGetResponse(const uint8_t *pUdsRequest, uint16_t reqLength,
                                             uint8_t *pUdsResponse, uint16_t *pRespLength);
#endif
