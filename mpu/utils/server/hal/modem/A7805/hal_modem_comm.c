#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_comm.h"
#include "Sunsea_AT.h"

#define HAL_MODEM_COMM_INITIALED (0)
#define HAL_MODEM_COMM_UNINITIALED (-1)
static int32_t gCommInited = HAL_MODEM_COMM_UNINITIALED;

int32_t HalModemCommInit(void)
{
    int32_t ret = RET_ERR;
    if (gCommInited == HAL_MODEM_COMM_INITIALED)
    {
        ret = RET_OK;
        return ret;
    }

    if (Sunsea_AT_Init() < 0)
    {
        LOG_Error("Sunsea_AT_Init failed");
        return ret;
    }
    
    gCommInited = HAL_MODEM_COMM_INITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemCommDeinit(void)
{
    int32_t ret = RET_ERR;
    if (gCommInited == HAL_MODEM_COMM_UNINITIALED)
    {
        ret = RET_OK;
        return ret;
    }

    Sunsea_AT_Deinit();
    gCommInited = HAL_MODEM_COMM_UNINITIALED;
    ret = RET_OK;
    return ret;    
}