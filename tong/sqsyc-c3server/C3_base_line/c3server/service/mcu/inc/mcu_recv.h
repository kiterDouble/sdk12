#ifndef __MCU_RECV_H__
#define __MCU_RECV_H__
#include <stdint.h>
#include "mcu_data.h"

#ifdef __cplusplus
extern "C" {
#endif
gpointer UartDataHandleTask(gpointer arg);

gpointer SpiDataHandleTask(gpointer arg);

#ifdef __cplusplus
}
#endif
#endif
