#ifndef __GPIO_API_H__
#define __GPIO_API_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_DIRECTION_OUT (1)
#define GPIO_DIRECTION_IN (0)
#define GPIO_LEVEL_HIGH (1)
#define GPIO_LEVEL_LOW (0)

typedef enum {
    GPIO_EDGE_NONE = 0,
    GPIO_EDGE_RISING,
    GPIO_EDGE_FALLING,
    GPIO_EDGE_BOTH,
} GpioEdge_e;

/**
 * @name GpioInit
 * @brief GPIO初始化函数
 * @param fd 输出参数，文件描述符
 * @param pinNo 输入参数，引脚号
 * @param direct 输入参数，引脚输入输出
 * @param level 输入参数，电平
 * @return int32_t 返回初始化结果，0成功，-1失败
 */
int32_t GpioInit(int32_t *fd, uint16_t pinNo, uint8_t direct, uint8_t level);

/**
 * @name GpioSetLevel
 * @brief 设置电平 
 * @param fd 输入参数，文件描述符
 * @param level 输入参数，电平
 * @return int32_t 执行结果，0成功，-1失败
 */
int32_t GpioSetLevel(int32_t fd, uint8_t level);

/**
 * @name GpioGetLevel
 * @brief 获取电平
 * @param fd 输入参数，文件描述符
 * @param level 输出参数，电平
 * @return int32_t 执行结果，0成功，-1失败
 */
int32_t GpioGetLevel(int32_t fd, uint8_t *level);
/**
 * @name GpioUnInit
 * @brief GPIO反初始化
 * @param fd 输入参数，文件描述符
 * @return int32_t 执行结果，0成功，-1失败
 */
int32_t GpioUnInit(int32_t fd);

int32_t GpioSetEdge(uint16_t pinNo, GpioEdge_e edge);

int32_t GpioWaitEdge(int32_t fd, int timeout_ms);

#ifdef __cplusplus
}
#endif
#endif