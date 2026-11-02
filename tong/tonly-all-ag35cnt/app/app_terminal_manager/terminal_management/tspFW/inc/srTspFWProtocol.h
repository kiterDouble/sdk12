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

#ifndef SRMQTTPROTOCOL_H
#define SRMQTTPROTOCOL_H

#include "srTspFWMqttClient.h"
#include "srTspFWService.h"

#define SR_TOPIC_PREFIX_T2S_CONNECT_TBOX "t2s/proto/v1/connect/"
#define SR_TOPIC_PREFIX_T2S_REPORT "t2s/proto/v1/report/"
#define SR_TOPIC_PREFIX_T2S_EVENT "t2s/proto/v1/event/"
#define SR_TOPIC_PREFIX_T2S_REISSUE "t2s/proto/v1/reissue/"
#define SR_TOPIC_PREFIX_T2S_COMMAND "t2s/proto/v1/command/"

#define SR_TOPIC_PREFIX_S2T_CONNECT_PLATFORM "s2t/proto/v1/connect/"
#define SR_TOPIC_PREFIX_S2T_REPORT_PLATFORM "s2t/proto/v1/report/"
#define SR_TOPIC_PREFIX_S2T_REISSUE_PLATFORM "s2t/proto/v1/reissue/"
#define SR_TOPIC_PREFIX_S2T_COMMAND_PLATFORM "s2t/proto/v1/command/"

#define SR_BUILD_TOPIC(buffer, prefix, suffix) \
    snprintf((buffer), sizeof(buffer), "%s%s", (prefix), (suffix))

#endif
