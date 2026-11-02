/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-02-07
 * Description: 应用层接口
 *********************************************************/
#ifndef _TERMINAL_MANAGEMENT_API_H
#define _TERMINAL_MANAGEMENT_API_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "terminal_management_data.h"

    /**
     * 配置地址初始化
     *
     * @param[in] dataFilePath          数据文件路径(配置监管平台数据库缓存位置)
     * @param[in] taskConfigFilePath    配置文件路径
     * @param[in] crtStorePath          证书存储路径
     * @param[in] downloadFolder        下载目录
     * @param[in] unzipFolder           解压目录
     *
     * @return
     *      RET_OK：初始化成功
     *      RET_ERR：初始化失败
     */
    int ConfigAddressInit(const char *dataFilePath,
                          const char *taskConfigFilePath,
                          const char *crtStorePath,
                          const char *downloadFolder,
                          const char *unzipFolder);

    /**
     * @brief 注册信息查询回调函数
     *
     * 该函数用于注册一组基础信息查询回调函数，供模块在需要获取设备相关信息时调用。
     * 回调函数集合通常包括获取 SN、IMEI、软件版本、硬件版本等信息的接口。
     *
     * @param[in] callback  指向信息查询回调函数集合的指针，类型为 StBaseInfoCallbacks_t。
     *                      该结构体内包含多个函数指针，每个函数对应一种基础信息查询。
     *                      调用方需确保该指针及其内部的函数指针有效，并在注册期间保持生命周期。
     *
     * @retval RET_OK   注册成功。
     * @retval RET_ERR  注册失败，例如传入参数为 NULL 或注册过程内部出错。
     *
     * @note
     * - 本函数通常在初始化阶段调用，仅需注册一次。
     * - 注册成功后，将通过已注册的回调函数获取设备基础信息。
     * - 如果需要更换回调函数集合，可再次调用该函数进行覆盖注册。
     */
    int RegisterQueryBaseInfoCallback(const StBaseInfoCallbacks_t *callback);

    /**
     * 启动 MQTT 客户端
     *
     * @param 无
     *
     * @return
     *      RET_OK：启动成功
     *      RET_ERR：启动失败
     */
    int TspClientStart(void);

    /**
     * 停止终端管理服务
     *
     * @param 无
     *
     * @return
     *      RET_OK：停止成功
     *      RET_ERR：停止失败
     */
    int ServiceStop(void);

#ifdef __cplusplus
}
#endif
#endif // _TERMINAL_MANAGEMENT_API_H
