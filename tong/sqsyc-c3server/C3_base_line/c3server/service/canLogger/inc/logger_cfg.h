/**
 * @file logger_cfg.h
 * @brief canLogger 运行期配置结构与加载接口
 *
 * @details 配置文件为 GLib GKeyFile (INI) 格式，默认段名为 [canLogger]。
 * 加载失败时由调用方回退到结构体默认值。
 */
#ifndef _LOGGER_CFG_H_
#define _LOGGER_CFG_H_

#include <cstdint>
#include <string>

/**
 * @brief canLogger 运行期配置
 *
 * @details 所有字段在 main() 启动时一次性读取，进程运行中不可变。
 */
struct LoggerCfg
{
    bool         enable          = true;                 ///< 是否启用录制，false 则进程立即退出
    std::string  filePath        = "/oemdata/log/can/";  ///< BLF 输出目录（不存在会自动 mkdir）
    std::string  filePrefix      = "can_";               ///< 文件名前缀，最终格式 prefix+时间戳+序号.blf
    uint32_t     maxSizeMB       = 100u;                 ///< 单文件大小上限（MB），超出触发轮转
    uint32_t     maxFileCount    = 10u;                  ///< 目录内最大保留文件数，超出删最旧
    uint32_t     maxTotalSizeMB  = 5120u;                ///< 目录内 BLF 总大小上限（MB）
    uint32_t     flushIntervalMs = 500u;                 ///< 配置保留字段，实际 flush 由写线程超时驱动
    uint32_t     channelMask     = 0xFFFFFFFFu;          ///< 通道掩码，bit n=1 表示录通道 n（仅 channel<32）
    std::string  logRule         = "canLogger";          ///< zlog 规则名（保留字段，当前未使用）
};

/**
 * @brief 从 INI 文件加载配置
 * @param[out] cfg  输出配置，加载失败时保持默认值不变
 * @param[in]  path INI 文件绝对路径
 * @return 0 成功，-1 失败（文件不存在 / 解析失败）
 *
 * @note 单字段缺失时使用 cfg 中已有默认值，不返回错误；
 *       仅文件无法打开或非 INI 格式时返回 -1。
 */
int LoadLoggerConfig(LoggerCfg &cfg, const std::string &path);

#endif
