#ifndef DEMO_INCLUDE_H
#define DEMO_INCLUDE_H

#include "can_signal_header_tonly.h"

/**
 * @brief 获取CAN信号原始数据
 *
 * @param outData
 * @return int
 */
int GetCanSignal(TonlyAllCanSignal_canVal_t *outData);

#endif
