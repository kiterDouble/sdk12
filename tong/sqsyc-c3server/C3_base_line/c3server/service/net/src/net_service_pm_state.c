#include <stdint.h>
#include <string.h>

#include "sr_log.h"
#include "net_service.h"

/**
 * @name NetPmChangeCallback
 * @brief 从给定数据中获取PM电源状态
 * @param data 指向包含PM电源状态数据的缓冲区的指针。
 * @param len 数据缓冲区的长度。
 * @param userdata 用户自定义数据的指针，本函数中传入 NetSrvCtx_t * 时使用。
 * @return 无返回值
 */
void NetPmChangeCallback(const uint8_t *data, const uint16_t len, void *userdata)
{
    NetSrvCtx_t* ctx = (NetSrvCtx_t*)userdata;

    if (sizeof(MPUPmStateChange_t) == len && NULL != data)
    {
        memcpy(ctx->pmState, data, len);
        NetBackoffOnPmStateChange(*(ctx->pmState));
    }
    else
    {
        LOG_Debug("%s len error!",__FUNCTION__);
    }
}
