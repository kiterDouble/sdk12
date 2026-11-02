/****************************************************************************
 * Copyright (c) 2025 <SRTL>. All rights reserved.
 *
 * This file is part of <TSP>.
 *
 * Licensed under the  Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: RunJie Xu
 ****************************************************************************/
#ifndef __SR_TSP_FW_LOG_H__
#define __SR_TSP_FW_LOG_H__

#include "sr_log.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /*
     * 标准日志调用：
     *   SR_TSP_LOG_I("TSP_FW_LOG", "Connecting to broker: %s", broker);
     *   SR_TSP_LOG_D("TSP_FW_LOG", "Debug value: %d", value);
     *   SR_TSP_LOG_W("TSP_FW_LOG", "Warning: %s", message);
     *   SR_TSP_LOG_E("TSP_FW_LOG", "Error code: %d", errorCode);
     *
     * 注意：
     * - 第一个参数 tag，推荐模块或子系统名，比如 "TSP_FW_LOG"
     * - 第二个参数 fmt 是格式化字符串
     */

#define SR_TSP_LOG_I(fmt, ...) \
    LOG_Info(fmt "\n", ##__VA_ARGS__)

#define SR_TSP_LOG_D(fmt, ...) \
    LOG_Debug(fmt "\n", ##__VA_ARGS__)

#define SR_TSP_LOG_W(fmt, ...) \
    LOG_Warn(fmt "\n", ##__VA_ARGS__)

#define SR_TSP_LOG_E(fmt, ...) \
    LOG_Error(fmt "\n", ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // __SR_TSP_FW_LOG_H__
