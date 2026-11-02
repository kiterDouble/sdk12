/**
 * @file uart_api.h
 * @author lujiefeng
 * @brief 
 * @version 0.1
 * @date 2019-01-16
 * 
 * @copyright Copyright (SiRun) 2019
 * 
 */
#ifndef _UART_API_H
#define _UART_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <termios.h>

/*uart set speed*/
#define UART_SET_SPEED_9600 (9600)
#define UART_SET_SPEED_38400 (38400)
#define UART_SET_SPEED_57600 (57600)
#define UART_SET_SPEED_115200 (115200)
#define UART_SET_SPEED_460800 (460800)
#define UART_SET_SPEED_576000 (576000)
#define UART_SET_SPEED_921600 (921600)
#define UART_SET_SPEED_1_5M (1500000)
#define UART_SET_SPEED_2M (2000000)
#define UART_SET_SPEED_2_5M (2500000)
#define UART_SET_SPEED_3M (3000000)
#define UART_SET_SPEED_3_5M (3500000)
#define UART_SET_SPEED_4M (4000000)

/*uart set flowctrl*/
#define UART_SET_FLOWCTRL_NONE (0)
#define UART_SET_FLOWCTRL_HARD (1)
#define UART_SET_FLOWCTRL_SOFT (2)

/*uart set databit*/
#define UART_SET_DATABITS_5 (5)
#define UART_SET_DATABITS_6 (6)
#define UART_SET_DATABITS_7 (7)
#define UART_SET_DATABITS_8 (8)

/*uart set stopbit*/
#define UART_SET_STOPBIT_1 (1)
#define UART_SET_STOPBIT_2 (2)

/*uart set parity*/
#define UART_SET_PARITY_NONE (0)
#define UART_SET_PARITY_ODD (1)
#define UART_SET_PARITY_EVEN (2)

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
int32_t UartInit(const char *uartDev, uint32_t speed, uint8_t flowCtrl, uint8_t dataBit, uint8_t stopBit, uint8_t parity);

/**
 * @name UartClose
 * @brief 关闭UART串口
 * @param fd 文件描述符，表示需要关闭的UART串口
 * @return 该函数不返回任何值。
 */
void UartClose(int32_t fd);

/**
 * @name UartSendData
 * @brief 通过UART发送数据
 * @param fd UART文件描述符
 * @param data 需要发送的数据缓冲区指针
 * @param dataLen 需要发送的数据长度
 * @return 发送成功的字节数，错误时返回负值
 */
int32_t UartSendData(int32_t fd, const uint8_t *data, int32_t dataLen);

/**
 * @name UartRecvData
 * @brief 从UART接收数据
 *        通过指定的文件描述符（fd）从UART设备接收指定长度的数据，并将接收到的数据存储到提供的缓冲区中。
 * @param fd 文件描述符，用于标识UART设备。如果为-1，则表示UART设备未正确初始化。
 * @param data 指向接收缓冲区的指针，用于存储接收到的数据。
 * @param dataLen 指定接收缓冲区的长度，单位为字节。
 * @return 接收成功的数据长度，如果发生错误则返回RET_ERR（特定错误代码）。
 */
int32_t UartRecvData(int32_t fd, uint8_t *data, int32_t dataLen);

/**
 * @name UartFlush
 * @brief 清空UART的输入缓冲区
 *        该函数通过调用tcflush函数清空UART的输入缓冲区，确保在后续读取操作时不会收到之前的数据。
 * @param fd 文件描述符，指向UART设备
 * @return 该函数无返回值
 */
void UartFlush(int32_t fd);

/**
 * @name UartSetCC
 * @brief 设置UART的读取超时和最小字符数
 *        该函数用于配置UART端口的读取特性，包括超时时间和最少读取字符数。
 * @param fd 文件描述符，指向要配置的UART端口
 * @param vTime 超时时间，单位为十分之一秒。0表示非阻塞，大于0表示最多等待的时间。
 * @param vMin 最小读取字符数。0表示只要有一个字符就返回，大于0表示至少要读取的字符数。
 */
void UartSetCC(int32_t fd, uint8_t vTime, uint8_t vMin);

#ifdef __cplusplus
}
#endif

#endif
