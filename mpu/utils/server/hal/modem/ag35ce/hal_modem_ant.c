#include "ql_adc.h"
#include "ql_gpio.h"
#include "ql_error.h"
#include "sr_log.h"
#include "err_code.h"
#include "hal_modem_ant.h"


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

/**
 * @name HalModemAntInit
 * @brief 模组天线金接口初始化
 *        本函数用于初始化模组的天线部分。在初始化过程中，会首先检查天线是否已经初始化过，
 *        如果已经初始化，则直接返回成功；如果未初始化，则尝试初始化GPIO引脚来控制天线电源，
 *        并标记为已初始化。
 * @param null
 * @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
 */
int32_t HalModemAntInit(void)
{
    if (HAL_MODEM_ANT_INITIALED == gAntPriv.initialed)
    {
        LOG_Debug("%s have initialed.", __FUNCTION__);
        return RET_OK;
    }

    if(RES_OK != Ql_GPIO_Init(PINNAME_OTG_PWR_EN, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_DISABLE))
    {
        LOG_Error("%s Ql_GPIO_Init fail.", __FUNCTION__);
        return RET_ERR;
    }

    gAntPriv.initialed = HAL_MODEM_ANT_INITIALED;
    return RET_OK;
}

/**
 * @name HalModemAntGetAdc
 * @brief 获取模天线（ADC）的值
 *        本函数用于通过ADC0获取当前的模拟输入值。
 * @param adcVal 指向一个整数的指针，用于接收ADC转换后的数字值。
 * @return 如果函数执行成功，返回RET_OK；如果参数无效，返回RET_ERR。
 */
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
/**
 * @name HalModemSecAntGetAdc
 * @brief 获取模天线（ADC）的值
 *        本函数用于通过ADC2获取当前的模拟输入值。
 * @param adcVal 指向一个整数的指针，用于接收ADC转换后的数字值。
 * @return 如果函数执行成功，返回RET_OK；如果参数无效，返回RET_ERR。
 */
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
/**
 * @name HalModemAntGetCurrent
 * @brief 获取当前Modem当前使用的天线
 *        本函数用于查询Modem当前所使用的天线是内部天线还是外部天线。
 * @param ant 指向HalModemAntType_e类型的指针，函数将通过这个指针返回当前天线的使用状态。
 *            可能的取值为E_HAL_MODEM_ANT_INTERNAL（内部天线）或E_HAL_MODEM_ANT_EXTERNEL（外部天线）。
 * @return 返回函数执行状态。RET_OK表示成功，RET_ERR表示错误。
 */
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

    if(PINLEVEL_HIGH == Ql_GPIO_GetLevel(PINNAME_OTG_PWR_EN))
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

/**
 * @name HalModemAntChange
 * @brief 天线切换
 *        该函数用于根据输入参数选择内部或外部天线，初始化后才能调用此函数。
 * @param ant 天线类型枚举，指定使用内部还是外部天线
 * @return int32_t 返回操作结果，成功返回RET_OK，失败返回RET_ERR
 */
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
        Ql_GPIO_SetLevel(PINNAME_OTG_PWR_EN, PINLEVEL_HIGH);
    }
    else if (ant == E_HAL_MODEM_ANT_EXTERNEL)
    {
        LOG_Debug("%s set value:%d.", __FUNCTION__, PINLEVEL_LOW);
        Ql_GPIO_SetLevel(PINNAME_OTG_PWR_EN, PINLEVEL_LOW);
    }
    else
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;
    }
    
    return RET_OK;
}

/**
 * @name HalModemAntDeinit
 * @brief Modem天线反初始化
 *        该函数用于反初始化Modem的天线部分。它首先检查天线是否已经初始化，如果未初始化则直接返回错误。
 * @param null
 * @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
 */
int32_t HalModemAntDeinit(void)
{
    if (HAL_MODEM_ANT_INITIALED != gAntPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return RET_ERR;
    }

    if (RES_OK != Ql_GPIO_Uninit(PINNAME_OTG_PWR_EN))
    {
        LOG_Error("%s Ql_GPIO_Uninit fail.", __FUNCTION__);
        return RET_ERR;        
    }

    gAntPriv.initialed = HAL_MODEM_ANT_UNINITIALED;
    return RET_OK;
}
