#ifndef __LOG_PACK_CFG_H__
#define __LOG_PACK_CFG_H__

#include <stdint.h>
#include "log_pack.h"

/**
 * @brief 从 ini 文件加载日志打包配置
 *
 * @param cfgPath ini 文件路径
 * @param ctx     日志服务器上下文，函数内会分配 tasks 数组
 * @return 0 成功，其他失败
 */
int32_t LogPackLoadCfg(const char* cfgPath, LogSrCtx_t* ctx);

#endif /* __LOG_PACK_CFG_H__ */
