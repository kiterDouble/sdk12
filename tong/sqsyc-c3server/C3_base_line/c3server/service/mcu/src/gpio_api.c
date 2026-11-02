#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
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

    (void)snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pinn);
    int32_t fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        LOG_Error("%s open %s fail.", __FUNCTION__, path);
        return ret;
    }

    // 读取方向值
    char readBuf[64] = {0x0,};
    ssize_t readn = read(fd, readBuf, sizeof(readBuf));
    (void)close(fd);

    if (readn <= 0)
    {
        LOG_Error("%s read %s fail.", __FUNCTION__, path);
        return ret;
    }

    *value = (uint8_t)atoi(readBuf);
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

    (void)snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pinn);
    int32_t fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        LOG_Error("%s open %s fail.", __FUNCTION__, path);
        return ret;
    }

    char direction[64] = {0x0,};
    ssize_t readn = read(fd, direction, sizeof(direction) - 1U);
    (void)close(fd);

    if (readn <= 0)
    {
        LOG_Error("%s read %s fail.", __FUNCTION__, path);
        return ret;
    }

    if (NULL != strstr(direction, "out"))
    {
        *direct = GPIO_DIRECTION_OUT;
        return RET_OK;
    }
    else if (NULL != strstr(direction, "in"))
    {
        *direct = GPIO_DIRECTION_IN;
        return RET_OK;
    }
    else
    {
        //do nothing
        ;
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

    (void)snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pinn);

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
    if (RET_OK == GpioFileExist(pinn))
    {
        return RET_OK;
    }

    int32_t sfd = open("/sys/class/gpio/export", O_WRONLY);
    if (sfd < 0)
    {
        LOG_Error("%s open export fail.", __FUNCTION__);
        return RET_ERR;
    }

    char num[16];
    (void)snprintf(num, sizeof(num), "%u", pinn);
    size_t len = strlen(num);
    if ((ssize_t)len != write(sfd, num, len))
    {
        LOG_Error("%s write export fail.", __FUNCTION__);
        (void)close(sfd);
        return RET_ERR;
    }
    (void)close(sfd);

    /* 等待节点出现 */
    for (int i = 0; i < 10; ++i)
    {
        if (RET_OK == GpioFileExist(pinn))
            return RET_OK;
        usleep(1000);
    }
    LOG_Error("%s export gpio timeout.", __FUNCTION__);
    return RET_ERR;
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
    char path[256] = {0x0,};

    (void)snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pinn);
    int32_t fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        LOG_Error("%s open %s fail.", __FUNCTION__, path);
        return RET_ERR;
    }

    const char *s = (direct == GPIO_DIRECTION_OUT) ? "out" : "in";
    size_t len = strlen(s);
    if ((ssize_t)len != write(fd, s, len))
    {
        LOG_Error("%s write direction fail.", __FUNCTION__);
        (void)close(fd);
        return RET_ERR;
    }
    (void)close(fd);

    /* 校验 */
    uint8_t chk = 0xFF;
    if (RET_OK != ReadGpioDirect(pinn, &chk) || chk != direct)
    {
        LOG_Error("%s direction verify fail.", __FUNCTION__);
        return RET_ERR;
    }
    return RET_OK;
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
    char path[256] = {0x0,};

    (void)snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pinn);
    int32_t fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        LOG_Error("%s open %s fail.", __FUNCTION__, path);
        return RET_ERR;
    }

    const char *v = level ? "1" : "0";
    if (write(fd, v, 1) != 1)
    {
        LOG_Error("%s write value fail.", __FUNCTION__);
        (void)close(fd);
        return RET_ERR;
    }
    (void)fsync(fd);
    (void)close(fd);

    uint8_t chk = 2;
    if (RET_OK != ReadGpioValue(pinn, &chk) || chk != level)
    {
        LOG_Error("%s value verify fail.", __FUNCTION__);
        return RET_ERR;
    }
    return RET_OK;
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
    if (!fd || (direct > 1U) || (level > 1U))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;
    }

    if (RET_OK != GpioExport(pinNo))
    {
        LOG_Error("%s GpioExport fail.", __FUNCTION__);
        return RET_ERR;
    }
    if (RET_OK != GpioSetDirection(pinNo, direct))
    {
        LOG_Error("%s GpioSetDirection fail.", __FUNCTION__);
        return RET_ERR;
    }
    if ((uint32_t)GPIO_DIRECTION_OUT == direct)
    {
        if (RET_OK != GpioInitLevel(pinNo, level))
        {
            LOG_Error("%s GpioInitLevel fail.", __FUNCTION__);
            return RET_ERR;
        }
    }

    char path[256] = {0x0,};
    int32_t gpiofd = -1;
    int32_t flags = ((uint32_t)GPIO_DIRECTION_OUT == direct) ? O_WRONLY : O_RDWR;

    (void)snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pinNo);
    gpiofd = open(path, flags);
    if (gpiofd == -1)
    {
        LOG_Error("%s  [%s ]open fail.", __FUNCTION__, path);
        return RET_ERR;
    }

    *fd = gpiofd;
    return RET_OK;
}

/**
 * @name GpioSetLevel
 * @brief 设置GPIO的电平值
 * @param fd GPIO文件描述符
 * @param level 要设置的电平值，0代表低电平，1代表高电平
 * @return 若设置成功返回RET_OK，否则返回RET_ERR
 */
int32_t GpioSetLevel(int32_t fd, uint8_t level)
{
    if ((fd < 0) || (level > 1U))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;
    }
    (void)lseek(fd, 0, SEEK_SET);
    const char *s = level ? "1" : "0";
    if (write(fd, s, 1) != 1)
    {
        LOG_Error("%s write fail.", __FUNCTION__);
        return RET_ERR;
    }
    (void)fsync(fd);
    return RET_OK;
}

/**
 * @name GpioGetLevel
 * @brief 从GPIO设备文件中获取电平值
 * @param fd GPIO设备文件描述符
 * @param level 指向存储读取到的电平值的指针
 * @return 成功返回RET_OK，失败返回RET_ERR
 */
int32_t GpioGetLevel(int32_t fd, uint8_t *level)
{
    int32_t ret = RET_ERR;

    if ((fd < 0) || !level)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    (void)lseek(fd, 0, SEEK_SET);
    char readBuf[32] = {0x0,};

    ssize_t readn = read(fd, readBuf, sizeof(readBuf));
    if (readn <= 0)
    {
        LOG_Error("%s get fail.", __FUNCTION__);
        return ret;
    }
    *level = (uint8_t)atoi(readBuf);
    ret = RET_OK;
    return ret;
}

/**
 * @name GpioUnInit
 * @brief 释放GPIO资源
 */
int32_t GpioUnInit(int32_t fd)
{
    int32_t ret = RET_ERR;
    if ((fd < 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    (void)close(fd);
    ret = RET_OK;
    return ret;
}

/**
 * @name GpioSetEdge
 * @brief 配置GPIO边沿触发（sysfs: /sys/class/gpio/gpioX/edge）
 */
int32_t GpioSetEdge(uint16_t pinNo, GpioEdge_e edge)
{
    const char *edge_str = "none";
    switch (edge) {
        case 1: edge_str = "rising";  break;
        case 2: edge_str = "falling"; break;
        case 3: edge_str = "both";    break;
        default: edge_str = "none";   break;
    }

    char path[256] = {0};
    (void)snprintf(path, sizeof(path), "/sys/class/gpio/gpio%u/edge", pinNo);
    int32_t fd = open(path, O_WRONLY);
    if (fd < 0) {
        LOG_Error("%s open %s fail.", __FUNCTION__, path);
        return RET_ERR;
    }

    size_t len = strlen(edge_str);
    if ((ssize_t)len != write(fd, edge_str, len)) {
        LOG_Error("%s write edge fail(%s).", __FUNCTION__, edge_str);
        (void)close(fd);
        return RET_ERR;
    }
    (void)close(fd);
    return RET_OK;
}

/**
 * @name GpioWaitEdge
 * @brief 阻塞等待GPIO边沿事件发生（对 value fd 做 poll）
 * @param fd     GpioInit 打开的 value 文件fd（输入脚 O_RDWR）
 * @param timeout_ms  超时，-1 永久等待，0 立即返回，>0 毫秒
 * @return RET_OK=有事件；RET_TIMEOUT=超时；RET_ERR=失败
 */
#ifndef RET_TIMEOUT
#define RET_TIMEOUT (1)
#endif
int32_t GpioWaitEdge(int32_t fd, int timeout_ms)
{
    if (fd < 0) {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;
    }

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLPRI | POLLERR;   // sysfs GPIO 事件语义

    /* 读取一次以清理上次状态：sysfs 语义要求读才能复位“边沿触发” */
    (void)lseek(fd, 0, SEEK_SET);
    char dummy[8];
    (void)read(fd, dummy, sizeof(dummy));

    int r = poll(&pfd, 1, timeout_ms);
    if (r == 0) {
        return RET_TIMEOUT;
    } else if (r < 0) {
        if (errno == EINTR) return RET_TIMEOUT;
        LOG_Error("%s poll fail(errno=%d).", __FUNCTION__, errno);
        return RET_ERR;
    }

    if (pfd.revents & (POLLPRI | POLLERR)) {
        // 事件就绪后再读一次清中断
        (void)lseek(fd, 0, SEEK_SET);
        (void)read(fd, dummy, sizeof(dummy));
        return RET_OK;
    }
    return RET_TIMEOUT;
}
