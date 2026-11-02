#ifndef __MCUINTERBASE_H__
#define __MCUINTERBASE_H__
#include <glib.h>

#include "sdk/mcu_api.h"
#include "queue_msg.h"
#include "err_code.h"
#ifdef __cplusplus
extern "C" {
#endif

gint SendRemoteControlDataToMcu(SYSQueMsgType_e type ,guint8 *controlDataBuff,gshort dataLen);


#ifdef __cplusplus
}
#endif

#endif