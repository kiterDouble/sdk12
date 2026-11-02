#include "ql_adc.h"
#include "ql_gpio.h"
#include "sr_log.h"
#include "err_code.h"
#include "hal_modem_ant.h"


#define HAL_MODEM_ANT_INITIALED (0)
#define HAL_MODEM_ANT_UNINITIALED (-1)

typedef struct
{
    int32_t initialed;
}HalModemAntPriv_t;

static HalModemAntPriv_t gAntPriv =
{
    .initialed = HAL_MODEM_ANT_UNINITIALED
};


int32_t HalModemAntInit(void)
{
    if (HAL_MODEM_ANT_INITIALED == gAntPriv.initialed)
    {
        LOG_Debug("%s have initialed.", __FUNCTION__);
        return RET_OK;
    }

    if(RES_OK != ql_gpio_init(PINNAME_GPIO_40, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_DISABLE))
    {
        LOG_Error("%s Ql_GPIO_Init fail.", __FUNCTION__);
        return RET_ERR;
    }

    gAntPriv.initialed = HAL_MODEM_ANT_INITIALED;
    return RET_OK;
}

int32_t HalModemAntGetAdc(int32_t *adcVal)
{
    if (!adcVal)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;        
    }

    *adcVal = ql_adc_show(ADC0);
    return RET_OK;
}

int32_t HalModemSecAntGetAdc(int32_t *adcVal)
{
    if (!adcVal)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;        
    }

    *adcVal = ql_adc_show(ADC1);
    return RET_OK;
}

int32_t HalModemAntGetCurrent(HalModemAntType_e *ant)
{
    if (!ant)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;        
    }

    if (HAL_MODEM_ANT_INITIALED != gAntPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return RET_ERR;
    }

    if((int)PINLEVEL_HIGH == ql_gpio_get_level(PINNAME_GPIO_40))
    {
        // *ant = E_HAL_MODEM_ANT_INTERNAL;
        *ant = E_HAL_MODEM_ANT_EXTERNEL;
    }
    else
    {
        // *ant = E_HAL_MODEM_ANT_EXTERNEL;
        *ant = E_HAL_MODEM_ANT_INTERNAL;
    }

    LOG_Debug("%s ant value:%d.", __FUNCTION__, *ant);
    return RET_OK;
}

int32_t HalModemAntChange(HalModemAntType_e ant)
{
    if (HAL_MODEM_ANT_INITIALED != gAntPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return RET_ERR;
    }

    if (ant == E_HAL_MODEM_ANT_INTERNAL)
    {
        LOG_Debug("%s set value:%d.", __FUNCTION__, PINLEVEL_LOW);
        // (void)ql_gpio_set_level(PINNAME_GPIO_40, PINLEVEL_HIGH);
        (void)ql_gpio_set_level(PINNAME_GPIO_40, PINLEVEL_LOW);
    }
    else if (ant == E_HAL_MODEM_ANT_EXTERNEL)
    {
        LOG_Debug("%s set value:%d.", __FUNCTION__, PINLEVEL_HIGH);
        (void)ql_gpio_set_level(PINNAME_GPIO_40, PINLEVEL_HIGH);
    }
    else
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;
    }
    
    return RET_OK;
}

int32_t HalModemAntDeinit(void)
{
    if (HAL_MODEM_ANT_INITIALED != gAntPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return RET_ERR;
    }

    if (RES_OK != ql_gpio_uninit(PINNAME_GPIO_40))
    {
        LOG_Error("%s Ql_GPIO_Uninit fail.", __FUNCTION__);
        return RET_ERR;        
    }

    gAntPriv.initialed = HAL_MODEM_ANT_UNINITIALED;
    return RET_OK;
}
