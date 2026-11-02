//
// Created by rfge on 2024/01/24.
//

#ifndef AIKIT_DIAG_H
#define AIKIT_DIAG_H

#include "aikit_biz_type.h"

namespace AIKIT {

/**
 * 主动触发重新解析DNS流程(异步接口，通过事件回调接口抛出解析结果)
 * @param host 需要解析的域名列表；
 *             如果只有一个域名则直接传入，例如："aaa.xf-yun.com"
 *             如果有多个则传入JSON Array，例如："[\"aaa.xf-yun.com\",\"bbb.xf-yun.com\"]"
 *             如果为空则表示解析全部预埋域名
 * @return 结果错误码，0:执行结束(不代表解析成功)，其他:执行报错
 */
AIKITAPI int32_t AIKIT_ResolveDNS(const char *host);

}  // namespace AIKIT

#endif  //AIKIT_DIAG_H
