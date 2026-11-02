#include <glib.h>
#include "mcuinterbase.h"

#include "mcu_api.h"
#include "err_code.h"

static GMutex mcu_send_mutex;

gint SendRemoteControlDataToMcu(SYSQueMsgType_e type ,guint8 *controlDataBuff,gshort dataLen)
{
    if(NULL == controlDataBuff)
    {
        return RET_ERR;
    }
    
    g_mutex_lock(&mcu_send_mutex);
    
    McuMesg_t mesg = 
    {
        .type = type,
        .payloadLen = dataLen,
        .payload = controlDataBuff,
    };
    
    gint result = McuSrvSendMesg(&mesg);
    
    g_mutex_unlock(&mcu_send_mutex);
    
    return result;
}