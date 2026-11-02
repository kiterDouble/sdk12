/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-02-07
 * Description: 数据结构定义
 *********************************************************/
#ifndef _TERMINAL_MANAGEMENT_DATA_H
#define _TERMINAL_MANAGEMENT_DATA_H

#include <stdint.h>

/**
 * @brief 获取设备信息的回调函数指针类型
 *
 * 回调函数签名为：
 *   int callback(char* buffer, int len);
 * 要求：
 *   - buffer：用于填充信息的缓冲区，调用方分配。
 *   - len：buffer 的最大长度。
 * 返回值：
 *   - 0：获取成功。
 *   - 非0：获取失败。
 */
typedef int (*QueryBaseInfoCallbackFunc)(char *buffer, int len);

/**
 * @brief TSP 基础信息回调结构体
 *
 * 包含所有需要获取的基础信息回调接口。
 * 每个字段必须注册有效的回调函数，否则系统会判定为无效。
 */
typedef struct
{
    QueryBaseInfoCallbackFunc getSn;                     ///< 获取 SN 编号
    QueryBaseInfoCallbackFunc getSim;                    ///< 获取 SIM 卡号
    QueryBaseInfoCallbackFunc getImei;                   ///< 获取 IMEI 编号
    QueryBaseInfoCallbackFunc getIccid;                  ///< 获取 ICCID 编号
    QueryBaseInfoCallbackFunc getProjectName;            ///< 获取项目名称
    QueryBaseInfoCallbackFunc getVin;                    ///< 获取 VIN 码
    QueryBaseInfoCallbackFunc getHwVersion;              ///< 获取硬件版本
    QueryBaseInfoCallbackFunc getSwVersionMain;          ///< 获取主软件版本
    QueryBaseInfoCallbackFunc getSwVersionSecondary;     ///< 获取次软件版本
    QueryBaseInfoCallbackFunc getSwVersionConfiguration; ///< 获取配置软件版本
} StBaseInfoCallbacks_t;

#endif // _TERMINAL_MANAGEMENT_DATA_H
