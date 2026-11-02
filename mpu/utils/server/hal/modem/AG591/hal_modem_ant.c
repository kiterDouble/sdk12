#include "gpioSysfs.h"
#include "ql_adc.h"
#include "ql_gpio.h"
#include "sr_log.h"
#include "err_code.h"
#include "hal_modem_ant.h"


#define ANT_GPIO_PIN PINNAME_GPIO_42
#define HAL_MODEM_ANT_INITIALED (0)
#define HAL_MODEM_ANT_UNINITIALED (-1)

typedef struct
{
    int32_t initialed;
}HalModemAntPriv_t;

HalModemAntPriv_t gAntPriv =
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

    if(RES_OK != ql_gpio_init(ANT_GPIO_PIN, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_DISABLE))
    {
        //LOG_Error("%s Ql_GPIO_Init fail.", __FUNCTION__);
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

    *adcVal = ql_adc_show(ADC2);
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

    if(PINLEVEL_HIGH == ql_gpio_get_level(ANT_GPIO_PIN))
    {
        *ant = E_HAL_MODEM_ANT_INTERNAL;
    }
    else
    {
        *ant = E_HAL_MODEM_ANT_EXTERNEL;
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
        LOG_Debug("%s set value:%d.", __FUNCTION__, PINLEVEL_HIGH);
        ql_gpio_set_level(ANT_GPIO_PIN, PINLEVEL_HIGH);
    }
    else if (ant == E_HAL_MODEM_ANT_EXTERNEL)
    {
        LOG_Debug("%s set value:%d.", __FUNCTION__, PINLEVEL_LOW);
        ql_gpio_set_level(ANT_GPIO_PIN, PINLEVEL_LOW);
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

    if (RES_OK != ql_gpio_uninit(ANT_GPIO_PIN))
    {
        LOG_Error("%s Ql_GPIO_Uninit fail.", __FUNCTION__);
        return RET_ERR;        
    }

    gAntPriv.initialed = HAL_MODEM_ANT_UNINITIALED;
    return RET_OK;
}
