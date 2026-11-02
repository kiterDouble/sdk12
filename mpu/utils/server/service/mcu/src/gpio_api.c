#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "sr_log.h"
#include "err_code.h"
#include "gpio_api.h"

/**
 * @name ReadGpioValue
 * @brief 读取GOIP的值
 * @param pinn 输入参数，GPIO引脚号
 * @param value 输出参数，电平值
 * @return int32_t 执行结果，0成功，-1失败
 */
static int32_t ReadGpioValue(uint16_t pinn, uint8_t *value)
{
    int32_t ret = RET_ERR;
    char path[256] = {0x0,};

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pinn);
    int32_t fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        LOG_Error("%s open %s fail.", __FUNCTION__, path);
        return ret;
    }

    // 读取方向值
    char readBuf[64] = {0x0,};
    ssize_t readn = read(fd, readBuf, sizeof(readBuf));
    close(fd);

    if (readn <= 0)
    {
        LOG_Error("%s read %s fail.", __FUNCTION__, path);
        return ret;
    }

    *value = atoi(readBuf);
    LOG_Error("%s value invalid.", __FUNCTION__);
    return RET_OK;
}

/**
 * @name ReadGpioDirect
 * @brief 读取GPIO引脚电压输入输出
 * @param pinn 输入参数，GPIO引脚号
 * @param direct 输出参数，电压输入输出
 * @return int32_t 执行结果，0成功，-1失败
 */
static int32_t ReadGpioDirect(uint16_t pinn, uint8_t *direct)
{
    int32_t ret = RET_ERR;
    char path[256] = {0x0,};

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pinn);
    int32_t fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        LOG_Error("%s open %s fail.", __FUNCTION__, path);
        return ret;
    }

    char direction[64] = {0x0,};
    ssize_t readn = read(fd, direction, sizeof(direction) - 1);
    close(fd);

    if (readn <= 0)
    {
        LOG_Error("%s read %s fail.", __FUNCTION__, path);
        return ret;
    }

    if (strstr(direction, "out"))
    {
        *direct = GPIO_DIRECTION_OUT;
        return RET_OK;
    }
    else if (strstr(direction, "in"))
    {
        *direct = GPIO_DIRECTION_IN;
        return RET_OK;
    }

    LOG_Error("%s direction invalid.", __FUNCTION__);
    return ret;
}

/**
 * @name GpioFileExist
 * @brief 获取GPIO是否存在
 * @param pinn 输入参数，GPIO引脚号
 * @return int32_t 执行结果 0成功，-1失败
 */
static int32_t GpioFileExist(uint16_t pinn)
{
    int32_t ret = RET_ERR;
    char path[256] = {0x0,};

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pinn);

    if (access(path, F_OK) == 0)
    {
        return RET_OK;
    }

    LOG_Error("%s file not exist.", __FUNCTION__);
    return ret;
}
/**
 * @name GpioExport
 * @brief 声明GPIO
 * @param pinn  输入参数，GPIO引脚号
 * @return int32_t 执行结果，0成功，-1失败
 */
static int32_t GpioExport(uint16_t pinn)
{
    int32_t ret = RET_ERR;    

    if (RET_OK != GpioFileExist(pinn))
    {
        char cmd[256] = {0x0,};
        uint32_t count = 3;

        snprintf(cmd, sizeof(cmd), "echo %d > /sys/class/gpio/export", pinn);
        while (count > 0)
        {
            system(cmd);
            if (RET_OK == GpioFileExist(pinn))
            {
                break;
            }
            count--;
            usleep(100000);
        }

        if (0 == count)
        {
            LOG_Error("%s export gpio fail.", __FUNCTION__);
            return ret;
        }
    }

    ret = RET_OK;
    return ret;
}
/**
 * @name GpioSetDirection
 * @brief 设置GPIO引脚输入输出
 * @param pinn 输入参数，GPIO引脚号
 * @param direct 输入参数，GPIO引脚输入输出
 * @return int32_t 执行结果，0成功，-1失败
 */
static int32_t GpioSetDirection(uint16_t pinn, uint8_t direct)
{
    int32_t ret = RET_ERR;
    char cmd[256] = {0x0,};
    uint32_t count = 3;
    const char *directStr[] = {"in", "out"};
    uint8_t value = 0;

    snprintf(cmd, sizeof(cmd), "echo %s > /sys/class/gpio/gpio%d/direction", directStr[direct], pinn);
    while (count > 0)
    {
        system(cmd);
        if ((RET_OK == ReadGpioDirect(pinn, &value)) && (value == direct))
        {
            break;
        }
        count--;
        usleep(100000);
    }

    if (0 == count)
    {
        LOG_Error("%s set gpio direction fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}
/**
 * @name GpioInitLevel
 * @brief 初始化GPIO电平
 * @param pinn 输入参数，GPIO引脚号
 * @param level 输入参数，GPIO电平
 * @return int32_t 执行结果，0成功，-1失败
 */
static int32_t GpioInitLevel(uint16_t pinn, uint8_t level)
{
    int32_t ret = RET_ERR;
    char cmd[256] = {0x0,};
    uint32_t count = 3;
    uint8_t value = 0;

    snprintf(cmd, sizeof(cmd), "echo %d > /sys/class/gpio/gpio%d/value", level, pinn);
    while (count > 0)
    {
        system(cmd);
        if ((RET_OK == ReadGpioValue(pinn, &value)) && (value == level))
        {
            break;
        }
        count--;
        usleep(100000);
    }

    if (0 == count)
    {
        LOG_Error("%s set gpio value fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name GpioInit
 * brief 初始化GPIO,本函数用于初始化指定的GPIO引脚，并打开对应的文件描述符。
 * @param fd 指向int32_t的指针，用于保存打开的GPIO文件描述符。
 * @param pinNo GPIO引脚编号。
 * @param direct GPIO引脚方向，0为输入，1为输出。
 * @param level 当GPIO设置为输出时，指定的初始电平，0为低电平，1为高电平。
 * @return 返回RET_OK表示成功，否则返回错误码。
 */
int32_t GpioInit(int32_t *fd, uint16_t pinNo, uint8_t direct, uint8_t level)
{
    int32_t ret = RET_ERR;

    if (!fd || (direct > 1) || (level > 1))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (RET_OK != GpioExport(pinNo))
    {
        LOG_Error("%s GpioExport fail.", __FUNCTION__);
        return ret;        
    }

    if (RET_OK != GpioSetDirection(pinNo, direct))
    {
        LOG_Error("%s GpioSetDirection fail.", __FUNCTION__);
        return ret;             
    }

    if(GPIO_DIRECTION_OUT == direct)
    {
        if (RET_OK != GpioInitLevel(pinNo, level))
        {
            LOG_Error("%s GpioInitLevel fail.", __FUNCTION__);
            return ret;
        }
    }

    char path[256] = {0x0,};
    int32_t gpiofd = -1;
    int32_t flags = O_RDWR;

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%u/value", pinNo);
    if (GPIO_DIRECTION_OUT == direct)
    {
        flags = O_WRONLY;
    }
    gpiofd = open(path, flags);

    if (gpiofd == -1)
    {
        LOG_Error("%s  [%s ]open fail.", __FUNCTION__, path);
        return ret;
    }

    *fd = gpiofd;
    ret = RET_OK;
    return ret;
}

/**
 * @name GpioSetLevel
 * @brief 设置GPIO的电平值
 * @param fd GPIO文件描述符
 * @param level 要设置的电平值，0代表低电平，1代表高电平
 * @return 若设置成功返回RET_OK（假设为0），否则返回RET_ERR（假设为负值）
 */
int32_t GpioSetLevel(int32_t fd, uint8_t level)
{
    int32_t ret = RET_ERR;

    if ((fd < 0) || (level > 1))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;        
    }

    lseek(fd, 0, SEEK_SET);
    char writeBuf[32] = {0x0,};

    snprintf(writeBuf, sizeof(writeBuf), "%d", level);
    ssize_t writen = write(fd, writeBuf, strlen(writeBuf) + 1);
    if (writen <= 0)
    {
        LOG_Error("%s set fail.", __FUNCTION__);
        return ret;
    }
    fsync(fd);
    ret = RET_OK;
    return ret;
}

/**
 * @name GpioGetLevel
 * @brief 从GPIO设备文件中获取电平值
 * @param fd GPIO设备文件描述符
 * @param level 指向存储读取到的电平值的指针
 * @return 成功返回RET_OK（假设为0），失败返回RET_ERR（假设为负值）
 */
int32_t GpioGetLevel(int32_t fd, uint8_t *level)
{
    int32_t ret = RET_ERR;

    if ((fd < 0) || !level)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;        
    }

    lseek(fd, 0, SEEK_SET);
    char readBuf[32] = {0x0,};

    ssize_t readn = read(fd, readBuf, sizeof(readBuf));
    if (readn <= 0)
    {
        LOG_Error("%s get fail.", __FUNCTION__);
        return ret;
    }
    *level = atoi(readBuf);
    ret = RET_OK;
    return ret;
}

/**
 * @name GpioUnInit
 * @brief 释放GPIO资源, 该函数用于关闭GPIO文件描述符，并释放相关的资源。
 * @param fd GPIO的文件描述符。类型为int32_t，表示要释放的GPIO资源的文件描述符。
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t GpioUnInit(int32_t fd)
{
    int32_t ret = RET_ERR;
    if ((fd < 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;        
    }
    close(fd);
    ret = RET_OK;
    return ret;
}