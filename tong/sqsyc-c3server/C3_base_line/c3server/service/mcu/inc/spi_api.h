#ifndef __SPI_API_H__
#define __SPI_API_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name SpiInit
 * @brief 初始化SPI接口
 *        本函数用于初始化SPI设备，配置其模式、位宽、速度，并对相关GPIO进行初始化。
 * @param devName 指向设备名称字符串的指针，用于打开SPI设备。
 * @param mode SPI工作模式，具体模式由参数决定。
 * @param bits 通信的位宽，例如8位或16位。
 * @param speed SPI通信的最大速度，单位为Hz。
 * @param cpinn MPU数据触发控制PIN编号
 * @param mpinn MCU数据触发控制PIN编号
 * @return 返回一个指向SpiApiPriv_t结构体的指针，若初始化失败则返回-1。
 */
int32_t SpiInit(const uint8_t *devName, uint32_t mode, uint32_t bits, uint32_t speed, uint32_t cpinn, uint32_t mpinn);


/**
 * @name SpiRead
 * @brief 通过SPI接口从指定设备读取数据
 * @param fd 文件描述符，指向SPI设备的私有数据结构
 * @param data 指向接收数据缓冲区的指针
 * @param dataLen 要读取的数据长度
 * @return 读取成功返回实际读取的数据长度，失败返回-1
 */
int32_t SpiRead(int32_t fd, uint8_t *data, int32_t dataLen);

/**
 * @name SpiWrite
 * @brief 通过SPI接口进行写操作
 * @param fd 文件描述符，实际上是指向SpiApiPriv_t结构体的指针
 * @param data 要写入的数据指针
 * @param dataLen 要写入的数据长度
 * @return 实际写入的数据长度，出错返回-1
 */
int32_t SpiWrite(int32_t fd, const uint8_t *data, int32_t dataLen);

/**
 * @name SpiClose
 * @brief 关闭SPI设备连接，该函数主要用于清理与SPI设备相关的资源，包括线程、管道、缓冲区和设备文件描述符等
 * @param fd 文件描述符，指向SPI设备的私有数据结构
 * @return 无返回值
 */
void SpiClose(int32_t fd);
#ifdef __cplusplus
}
#endif
#endif