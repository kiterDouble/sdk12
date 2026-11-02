/**
 * @file sr_gnss.h
 * @author jiajiangze (jiangze.jia@sirun.net)
 * @brief gnss sdk层接口头文件
 * @version 0.1
 * @date 2023-12-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef _RPC_GNSS_SDK_H
#define _RPC_GNSS_SDK_H

#include <stdint.h>
#include "gnss_ipc_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief gnss rpc server对外的位置信息RPC调用subcmd
 * 
 */
#define GNSSSERVER_MSGTYPE_SUBCMD_LOCATIONINFO (1)

/**
 * @brief 主动向gnss服务获取最新定位信息 基于RPC实现
 * 
 * @param data 主动获取到的最新定位信息 方向out
 */
void GetGnssLocation(GnssLocation_t *data);

#ifdef __cplusplus
}
#endif

#endif
