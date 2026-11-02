#ifndef __HAL_MODEM_COMM_H__
#define __HAL_MODEM_COMM_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t HalModemCommInit(void);
int32_t HalModemCommDeinit(void);
#ifdef __cplusplus
}
#endif

#endif

