/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-09-04 09:50:23
 * @LastEditTime: 2019-09-04 09:51:09
 * @LastEditors: Please set LastEditors
 */
/**
 * @file uart_api.c
 * @author lujiefeng
 * @brief 
 * @version 0.1
 * @date 2019-01-16
 * 
 * @copyright Copyright (SiRun) 2019
 * 
 */

#include "uart_api.h"
#include "err_code.h"
#include "sr_log.h"
#include <glib.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

/**
 * @name UartSetSpeed
 * @brief 设置UART通信的波特率
 *        本函数通过给定的波特率速度值，配置UART通信的输入和输出波特率。如果给定的波特率值不在支持的范围内，
 *        则会打印警告日志并返回错误码。
 * 
 * @param options 指向struct termios结构的指针，该结构包含了UART的配置选项。
 * @param speed 指定要设置的波特率速度。
 * @return 如果成功设置波特率，则返回RET_OK（假设RET_OK为0）；如果波特率不受支持，则返回RET_ERR（假设RET_ERR为非0）。
 */
static int32_t UartSetSpeed(struct termios *options, uint32_t speed)
{
    uint32_t speedMap[][2] = {
        {UART_SET_SPEED_9600, B9600},
        {UART_SET_SPEED_38400, B38400},
        {UART_SET_SPEED_57600, B57600},
        {UART_SET_SPEED_115200, B115200},
        {UART_SET_SPEED_460800, B460800},
        {UART_SET_SPEED_576000, B576000},
        {UART_SET_SPEED_921600, B921600},
        {UART_SET_SPEED_1_5M, B1500000},
        {UART_SET_SPEED_2M, B2000000},
        {UART_SET_SPEED_2_5M, B2500000},
        {UART_SET_SPEED_3M, B3000000},
        {UART_SET_SPEED_3_5M, B3500000},
        {UART_SET_SPEED_4M, B4000000}};
    int32_t mapNum = ((int32_t)sizeof(speedMap) / (int32_t)sizeof(speedMap[0]));
    int32_t index = 0;

    for (index = 0; index < mapNum; index++)
    {
        if (speed == speedMap[index][0])
        {
            (void)cfsetispeed(options, speedMap[index][1]);
            (void)cfsetospeed(options, speedMap[index][1]);
            break;
        }
    }
    if (index >= mapNum)
    {
        LOG_Warn("UART Set Speed Error:Speed[%d] Not Support", speed);
        return RET_ERR;
    }

    LOG_Debug("UART Set Speed[%d] Success", speed);
    return RET_OK;
}

/**
 * @name UartSetFlowCtrl
 * @brief 设置UART的流控制方式
 *        该函数用于根据传入的流控制参数，配置UART的流控制模式。支持的流控制方式包括无流控制、硬件流控制和软件流控制。
 * @param options 指向struct termios结构体的指针，该结构体包含了UART的端口选项和属性。
 * @param flowCtrl 流控制方式的参数，取值自枚举类型，指定所需的流控制类型。
 * @return int32_t 函数执行状态码。RET_OK表示成功，RET_ERR表示错误。
 */
static int32_t UartSetFlowCtrl(struct termios *options, uint8_t flowCtrl)
{
    int32_t ret = RET_OK;
    /*setup control mode*/
    options->c_cflag |= (tcflag_t)CLOCAL;
    options->c_cflag |= (tcflag_t)CREAD;
    switch (flowCtrl)
    {
    case UART_SET_FLOWCTRL_NONE:
        options->c_cflag &= ~((tcflag_t)CRTSCTS);
        break;
    case UART_SET_FLOWCTRL_HARD:
        options->c_cflag |= (tcflag_t)CRTSCTS;
        break;
    case UART_SET_FLOWCTRL_SOFT:
        options->c_cflag |= (tcflag_t)IXON | (tcflag_t)IXOFF | (tcflag_t)IXANY;
        break;
    default:
        LOG_Error("UART Set flowCtrl Error: Wrong param [%d]", flowCtrl);
        ret = RET_ERR;
        break;
    }

    LOG_Debug("UART Set flowCtrl[%d] Success", flowCtrl);
    return ret;
}

/**
 * @name UartSetDataBits
 * @brief 设置UART数据位数
 *        此函数用于根据传入的数据位数参数，配置UART端口的数据位数。
 * @param options 指向struct termios结构体的指针，该结构体包含了UART端口的配置选项。
 * @param dataBit 指定的数据位数，可以是UART_SET_DATABITS_5、UART_SET_DATABITS_6、
 *                UART_SET_DATABITS_7或UART_SET_DATABITS_8中的一种。
 * @return int32_t 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
 */
static int32_t UartSetDataBits(struct termios *options, uint8_t dataBit)
{
    /*setup data bit*/
    int32_t ret = RET_OK;
    options->c_cflag &= ~((tcflag_t)CSIZE);
    switch (dataBit)
    {
    case UART_SET_DATABITS_5:
        options->c_cflag |= (tcflag_t)CS5;
        break;
    case UART_SET_DATABITS_6:
        options->c_cflag |= (tcflag_t)CS6;
        break;
    case UART_SET_DATABITS_7:
        options->c_cflag |= (tcflag_t)CS7;
        break;
    case UART_SET_DATABITS_8:
        options->c_cflag |= (tcflag_t)CS8;
        break;
    default:
        LOG_Error("UART Set dataBit Error: Wrong param [%d]", dataBit);
        ret  = RET_ERR;
        break;
    }
    LOG_Debug("UART Set dataBit[%d] Success", dataBit);
    return ret;
}

/**
 * @name UartSetDataBits
 * @brief 设置UART的停止位
 *        此函数用于根据传入的停止位参数配置UART的停止位。支持配置为1位或2位停止位。
 * @param options 指向struct termios结构体的指针，该结构体包含了UART的配置选项。
 * @param stopBit 指定要设置的停止位，可选值为UART_SET_STOPBIT_1（1位停止位）或UART_SET_STOPBIT_2（2位停止位）。
 * @return int32_t 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
 */
static int32_t UartSetStopBits(struct termios *options, uint8_t stopBit)
{
    /*setup stop bit */
    int32_t ret = RET_OK;
    switch (stopBit)
    {
    case UART_SET_STOPBIT_1:
        options->c_cflag &= ~((tcflag_t)CSTOPB);
        break;
    case UART_SET_STOPBIT_2:
        options->c_cflag |= (tcflag_t)CSTOPB;
        break;
    default:
        LOG_Error("UART Set stopBit Error: Wrong param [%d]", stopBit);
        ret = RET_ERR;
        break;
    }
    LOG_Debug("UART Set stopBit[%d] Success", stopBit);
    return ret;
}

/**
 * @name UartSetParity
 * @brief 设置UART的奇偶校验位。
 *        该函数用于根据传入的参数配置UART的奇偶校验位。支持的校验位选项包括无校验、奇校验和偶校验。
 *        配置完成后，会通过日志输出配置结果。
 * @param options 指向struct termios结构体的指针，该结构体包含了UART的配置选项。
 * @param parity 指定要设置的奇偶校验位，取值自定义的UART_SET_PARITY_NONE、UART_SET_PARITY_ODD或UART_SET_PARITY_EVEN。
 * @return int32_t 返回操作结果，成功返回RET_OK，错误返回RET_ERR。
 */
static int32_t UartSetParity(struct termios *options, uint8_t parity)
{
    int32_t ret = RET_OK;
    /*setup parity bit*/
    switch (parity)
    {
    case UART_SET_PARITY_NONE:
        options->c_cflag &= ~((tcflag_t)PARENB);
        options->c_iflag &= ~((tcflag_t)INPCK);
        break;
    case UART_SET_PARITY_ODD:
        options->c_cflag |= ((tcflag_t)PARODD | (tcflag_t)PARENB);
        options->c_iflag |= (tcflag_t)INPCK;
        break;
    case UART_SET_PARITY_EVEN:
        options->c_cflag |= (tcflag_t)PARENB;
        options->c_cflag &= ~((tcflag_t)PARODD);
        options->c_iflag |= (tcflag_t)INPCK;
        break;
    default:
        LOG_Error("UART Set parity Error: Wrong param [%d]", parity);
        ret = RET_ERR;
        break;
    }
    LOG_Debug("UART Set parity[%d] Success", parity);
    return ret;
}

/**
 * @name UartSetHexMode
 * @brief 设置UART为十六进制模式
 *        此函数用于配置UART的数据模式为非标准化的十六进制模式，通过修改传入的termios结构体选项来实现。
 *        配置包括禁用后置处理、非行方式输入输出、屏蔽软件流控制、禁用回车换行映射等。
 * @param options 指向termios结构体的指针，该结构体包含了UART的配置选项。
 * @return 返回RET_OK表示成功设置为十六进制模式。
 */
static int32_t UartSetHexMode(struct termios *options)
{
    //原始数据模式
    options->c_oflag &= ~((tcflag_t)OPOST);
    //行方式输出输入
    options->c_lflag &= ~((tcflag_t)ICANON | (tcflag_t)ECHO | (tcflag_t)ECHOE | (tcflag_t)ISIG);
    //屏蔽软件流控制
    options->c_iflag &= ~((tcflag_t)IXON | (tcflag_t)IXOFF | (tcflag_t)IXANY);
    //回车换行映射屏蔽
    options->c_iflag &= ~((tcflag_t)INLCR | (tcflag_t)ICRNL | (tcflag_t)IGNCR);
    options->c_oflag &= ~((tcflag_t)ONLCR | (tcflag_t)OCRNL);
    LOG_Debug("UART Set HexMode Success");
    return RET_OK;
}

/**
 * @name UartOpen
 * @brief 打开一个UART设备
 * @param uartDev 指向要打开的UART设备路径的字符串指针。
 * @return 返回打开的UART设备的文件描述符，若打开失败则返回错误码。
 */
static int32_t UartOpen(const char *uartDev)
{
    if (uartDev == NULL)
    {
        LOG_Error("UartDdev Path is NULL");
        return RET_ERR;
    }
    int32_t fd = open(uartDev, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        LOG_Error("Uart[%s] open Error[%s]", uartDev, g_strerror(errno));
        return RET_ERR;
    }
    LOG_Debug("Uart[%s] open success,fd[%d]", uartDev, fd);
    return fd;
}

/**
 * @name UartSet
 * @brief 设置UART的配置参数
 *        本函数用于通过文件描述符(fd)来设置UART的波特率、流控、数据位、停止位和校验位。 
 *        首先获取当前的UART配置，然后根据传入的参数修改配置，最后将修改后的配置应用到UART。 
 * @param fd        文件描述符，用于标识要设置的UART设备。
 * @param speed     指定的波特率。
 * @param flowCtrl  流控方式，0表示无流控，1表示软件流控，2表示硬件流控。
 * @param dataBit   数据位数，可选值为5、6、7、8。
 * @param stopBit   停止位数，可选值为1或2。
 * @param parity    校验位，0表示无校验，1表示奇校验，2表示偶校验。
 * @return          函数执行状态，RET_OK表示成功，RET_ERR表示失败。
 */
static int32_t UartSet(int32_t fd, uint32_t speed, uint8_t flowCtrl, uint8_t dataBit, uint8_t stopBit, uint8_t parity)
{
    struct termios options;
    if (tcgetattr(fd, &options) != 0)
    {
        LOG_Warn("UART Set Error:Get fd[%d] attr error[%s]", fd, g_strerror(errno));
        return RET_ERR;
    }
    if (UartSetSpeed(&options, speed) != RET_OK)
    {
        return RET_ERR;
    }
    if (UartSetFlowCtrl(&options, flowCtrl) != RET_OK)
    {
        return RET_ERR;
    }
    if (UartSetDataBits(&options, dataBit) != RET_OK)
    {
        return RET_ERR;
    }
    if (UartSetStopBits(&options, stopBit) != RET_OK)
    {
        return RET_ERR;
    }
    if (UartSetParity(&options, parity) != RET_OK)
    {
        return RET_ERR;
    }
    (void)UartSetHexMode(&options);
    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        LOG_Warn("UART Set fd[%d] Error:activate attr error[%s]", fd, g_strerror(errno));
        return RET_ERR;
    }
    LOG_Debug("Uart fd[%d] set Success", fd);
    return RET_OK;
}

/**
 * @name UartClose
 * @brief 关闭UART串口
 * @param fd 文件描述符，表示需要关闭的UART串口
 * @return 该函数不返回任何值。
 */
void UartClose(int32_t fd)
{
    LOG_Debug("Uart fd[%d] closed", fd);
    if (fd != -1)
    {
        (void)close(fd);
    }
}

/**
 * @name UartInit
 * @brief 初始化UART设备
 *        本函数用于初始化指定的UART设备，配置其传输速率、流控方式、数据位、停止位和校验位，并打开该设备。
 * @param uartDev 指向表示UART设备名称的字符串的指针。
 * @param speed 指定UART设备的传输速率。
 * @param flowCtrl 指定UART设备的流控方式。
 * @param dataBit 指定UART设备的数据位数。
 * @param stopBit 指定UART设备的停止位数。
 * @param parity 指定UART设备的校验位类型。
 * @return 返回打开的UART设备文件描述符，出错返回-1。
 */
int32_t UartInit(const char *uartDev, uint32_t speed, uint8_t flowCtrl, uint8_t dataBit, uint8_t stopBit, uint8_t parity)
{
    LOG_Debug("Init Uart:[%s],speed[%d],flowCtrl[%d],dataBit[%d],stopBit[%d],parity[%d]", uartDev, speed, flowCtrl, dataBit, stopBit, parity);
    int32_t fd = UartOpen(uartDev);
    if (fd != RET_ERR)
    {
        /*uart open success*/
        int32_t ret = UartSet(fd, speed, flowCtrl, dataBit, stopBit, parity);
        if (ret == RET_ERR)
        {
            UartClose(fd);
            fd = -1;
            LOG_Error("Uart[%s] Init Error", uartDev);
        }
        else
        {
            LOG_Debug("Uart[%s] Init Success", uartDev);
        }
    }
    return fd;
}

/**
 * @name UartFlush
 * @brief 清空UART的输入缓冲区
 *        该函数通过调用tcflush函数清空UART的输入缓冲区，确保在后续读取操作时不会收到之前的数据。
 * @param fd 文件描述符，指向UART设备
 * @return 该函数无返回值
 */
void UartFlush(int32_t fd)
{
    LOG_Debug("Uart Will be TCIFLUSH");
    (void)tcflush(fd, TCIFLUSH);
}

/**
 * @name UartSetCC
 * @brief 设置UART的读取超时和最小字符数
 *        该函数用于配置UART端口的读取特性，包括超时时间和最少读取字符数。
 * @param fd 文件描述符，指向要配置的UART端口
 * @param vTime 超时时间，单位为十分之一秒。0表示非阻塞，大于0表示最多等待的时间。
 * @param vMin 最小读取字符数。0表示只要有一个字符就返回，大于0表示至少要读取的字符数。
 */
void UartSetCC(int32_t fd, uint8_t vTime, uint8_t vMin)
{
    struct termios options;
    if (tcgetattr(fd, &options) != 0)
    {
        LOG_Warn("UART Set CC Error:Get fd attr error[%s]", g_strerror(errno));
        return;
    }
    options.c_cflag |= (tcflag_t)CLOCAL;
    options.c_cflag |= (tcflag_t)CREAD;
    options.c_cc[VTIME] = vTime;
    options.c_cc[VMIN] = vMin;
    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        LOG_Warn("UART Set CC Error:activate attr error[%s]", g_strerror(errno));
    }
    else
    {
        LOG_Debug("Set Uart CC vTime[%d] vMin[%d] OK", vTime, vMin);
    }
}

/**
 * @name UartSendData
 * @brief 通过UART发送数据
 * @param fd UART文件描述符
 * @param data 需要发送的数据缓冲区指针
 * @param dataLen 需要发送的数据长度
 * @return 发送成功的字节数，错误时返回负值
 */
int32_t UartSendData(int32_t fd, const uint8_t *data, int32_t dataLen)
{
    if (fd == -1)
    {
        LOG_Error("Uart fd is invalid,Please call init uart");
        return RET_ERR;
    }
    if (data == NULL)
    {
        LOG_Warn("Uart SendData Buff is NULL.fd[%d]", fd);
        return RET_ERR;
    }
    int32_t ret = write(fd, (const void*)data, (size_t)dataLen);
    if (ret < 0)
    {
        LOG_Warn("UartSendData Error:fd[%d]-[%d]-[%s]", fd, ret, g_strerror(errno));
    }
    return ret;
    
}

/**
 * @name UartRecvData
 * @brief 从UART接收数据
 *        通过指定的文件描述符（fd）从UART设备接收指定长度的数据，并将接收到的数据存储到提供的缓冲区中。
 * @param fd 文件描述符，用于标识UART设备。如果为-1，则表示UART设备未正确初始化。
 * @param data 指向接收缓冲区的指针，用于存储接收到的数据。
 * @param dataLen 指定接收缓冲区的长度，单位为字节。
 * @return 接收成功的数据长度，如果发生错误则返回RET_ERR（特定错误代码）。
 */
int32_t UartRecvData(int32_t fd,  uint8_t *data, int32_t dataLen)
{
    if (fd == -1)
    {
        LOG_Error("Uart fd is invalid,Please call init uart");
        return RET_ERR;
    }
    if (data == NULL)
    {
        LOG_Warn("Uart Recv Buff is NULL");
        return RET_ERR;
    }
    int32_t ret = read(fd, (void *)data, (size_t)dataLen);
    if (ret < 0)
    {
        LOG_Warn("UartRecvData Error:[%d]-[%s]", ret, g_strerror(errno));
        ret = RET_ERR;
    }
    return ret;
}
