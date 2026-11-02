  
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "sr_log.h"
#include "err_code.h"
#include "ring_buffer.h"
#include "spi_api.h"
#include "gpio_api.h"

#define RINGBUFFER_MAX_SIZE (4096)
#define SPI_BUFFER_SIZE (2048)
#define SPI_VALID_SIZE_MAX (2044)
#define SPI_VALID_DATA_OFFSET (0)
typedef struct
{
    uint8_t devname[128];//spi dev name.
    uint32_t mode;//mode
    uint32_t bits;//bits per word.
    uint32_t speed;//speed hz
}SpiDevInfo_t;

typedef struct
{
    SpiDevInfo_t devInfo;
    int32_t fd;
    int32_t fds[2];
    int32_t cpinnfd;
    int32_t mpinnfd;
    RingBuffer_t rbuf;
    pthread_t threadId;
    uint32_t threadExit;
}SpiApiPriv_t;

static int32_t SleepMillis(uint32_t tms)
{
    struct timespec ts;
    int ret;

    ts.tv_sec = tms / 1000;
    ts.tv_nsec = (tms % 1000) * 1000000;
    do
    {
        ret = nanosleep(&ts, &ts);
    } 
    while (ret && (errno == EINTR));

    return ret;
}
/**
 * @name SpiCheckGpioLevel
 * @brief 检查SPI设备的GPIO电平是否达到指定状态
 * @param fd SPI设备的文件描述符
 * @param level 需要检查的GPIO电平状态（0或1）
 * @return 如果GPIO电平达到指定状态，返回RET_OK；如果在指定时间内未达到，返回RET_ERR。
 */
static int32_t SpiCheckGpioLevel(int32_t fd, uint8_t level)
{
    uint8_t count = 0;

    while(count++ < 10)
    {
        uint8_t value;
        GpioGetLevel(fd, &value);
        if (level == value)
        {
            return RET_OK;
        }
        SleepMillis(1);
    }

    return RET_ERR;
}

/**
 * @name LocalSendSpiData
 * @brief 通过local socket接口发送SPI数据
 *        本函数用于通过local socket接口发送指定长度的SPI数据。这是一个局部静态函数，仅在特定上下文中调用。
 * @param priv 指向SpiApiPriv_t结构体的指针，包含SPI设备的文件描述符等私有信息。
 * @param rxBuf 指向要发送的数据缓冲区的指针。
 * @param len 要发送的数据长度，单位为字节。
 */
static void LocalSendSpiData(SpiApiPriv_t *priv, uint8_t *rxBuf, size_t len)
{
    if (!priv || !rxBuf || (len == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    write(priv->fds[1], (void *)rxBuf, len);
    return;
    
}

/**
 * @name SpiTransfer
 * @brief 通过SPI进行数据传输
 * @param priv 指向SpiApiPriv_t结构体的指针，包含SPI设备的私有信息
 * @param txBuf 指向发送缓冲区的指针
 * @param rxBuf 指向接收缓冲区的指针
 * @param len 要传输的数据长度
 * @return 若传输成功返回RET_OK，否则返回RET_ERR
 */
static int32_t SpiTransfer(SpiApiPriv_t *priv, uint8_t *txBuf, uint8_t *rxBuf, size_t len)
{
    int32_t ret = RET_ERR;

    if (!priv || !txBuf || !rxBuf)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)txBuf,
        .rx_buf = (unsigned long)rxBuf,
        .len = len,
        .speed_hz = priv->devInfo.speed,
        .delay_usecs = 0,
        .bits_per_word = priv->devInfo.bits,
    };

    if (ioctl(priv->fd, SPI_IOC_MESSAGE(1), &tr) < 0)
    {
        //perror("SPI transfer failed");
        LOG_Error("%s transfer failed.", __FUNCTION__);
        return ret;
    }

    LOG_DebugHex(rxBuf,len,"SpiTransfer");
    ret = RET_OK;
    return ret;
}

/**
 * @name SpiThreadHandler
 * @brief SPI线程处理函数
 *        该函数是一个无限循环的线程函数，主要用于处理SPI通信。它会不断检查GPIO引脚的电平，根据电平状态进行SPI传输，并处理传输后的数据。
 * @param arg 指向SpiApiPriv_t结构体的指针，包含了SPI操作所需的各种参数和状态
 * @return void* 总是返回NULL
 */
static void* SpiThreadHandler(void* arg)
{  
    SpiApiPriv_t *priv = (SpiApiPriv_t *) arg;
    static uint8_t txBuf[SPI_BUFFER_SIZE];
    static uint8_t rxBuf[SPI_BUFFER_SIZE];
    
    if(!priv)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return NULL;
    }

    while (!priv->threadExit)
    {
        uint32_t avaiLen = 0;
        uint16_t validDataLen = 0;
        uint16_t reversDataeLen = 0;
        uint8_t level;

        memset(txBuf, 0x0, sizeof(txBuf));
        memset(rxBuf, 0x0, sizeof(rxBuf));
        
        RingBufferGetAvailableSize(priv->rbuf, &avaiLen);
        GpioGetLevel(priv->mpinnfd, &level);
        if (avaiLen >= SPI_VALID_SIZE_MAX)
        {
            avaiLen = SPI_VALID_SIZE_MAX;
        }
        validDataLen = avaiLen;
        reversDataeLen = ~avaiLen;

        if (GPIO_LEVEL_HIGH == level)
        {
            GpioSetLevel(priv->cpinnfd, GPIO_LEVEL_HIGH);
            usleep(1000);
            txBuf[0] = (reversDataeLen >> 8) & 0xFF;
            txBuf[1] = reversDataeLen & 0xFF;
            txBuf[2] = (validDataLen >> 8) & 0xFF;
            txBuf[3] = validDataLen & 0xFF;
            RingBufferDequeue(priv->rbuf, txBuf + 4, avaiLen);
            SpiTransfer(priv, txBuf, rxBuf, sizeof(txBuf));
            GpioSetLevel(priv->cpinnfd, GPIO_LEVEL_LOW);
            SpiCheckGpioLevel(priv->mpinnfd, GPIO_LEVEL_LOW);
            reversDataeLen = (rxBuf[SPI_VALID_DATA_OFFSET + 0] << 8) | rxBuf[SPI_VALID_DATA_OFFSET + 1];
            validDataLen = (rxBuf[SPI_VALID_DATA_OFFSET + 2] << 8) | rxBuf[SPI_VALID_DATA_OFFSET + 3];

            LOG_Debug("reversDataeLen :%x, validDataLen :%x, result:%d", reversDataeLen, validDataLen, (~validDataLen) == reversDataeLen);
            if(((~validDataLen) & 0xffff) == reversDataeLen)
            {
                LocalSendSpiData(priv, (rxBuf + SPI_VALID_DATA_OFFSET + 4), validDataLen);
                LOG_DebugHex((rxBuf + SPI_VALID_DATA_OFFSET + 4),validDataLen,"LocalSendSpiData");
            }
        }
        else
        {
            if (avaiLen == 0)
            {
                SleepMillis(1);
                continue;
            }
            GpioSetLevel(priv->cpinnfd, GPIO_LEVEL_HIGH);
            if(RET_OK == SpiCheckGpioLevel(priv->mpinnfd, GPIO_LEVEL_HIGH))
            {
                txBuf[0] = (reversDataeLen >> 8) & 0xFF;
                txBuf[1] = reversDataeLen & 0xFF;
                txBuf[2] = (validDataLen >> 8) & 0xFF;
                txBuf[3] = validDataLen & 0xFF;
                RingBufferDequeue(priv->rbuf, txBuf + 4, avaiLen);
                SpiTransfer(priv, txBuf, rxBuf, sizeof(txBuf));
                GpioSetLevel(priv->cpinnfd, GPIO_LEVEL_LOW);
                SpiCheckGpioLevel(priv->mpinnfd, GPIO_LEVEL_LOW);
                reversDataeLen = (rxBuf[SPI_VALID_DATA_OFFSET + 0] << 8) | rxBuf[SPI_VALID_DATA_OFFSET + 1];
                validDataLen = (rxBuf[SPI_VALID_DATA_OFFSET + 2] << 8) | rxBuf[SPI_VALID_DATA_OFFSET + 3];
                LOG_Debug("reversDataeLen :%x, validDataLen :%x, result:%d", reversDataeLen, validDataLen, (~validDataLen) == reversDataeLen);

                if(((~validDataLen) & 0xffff) == reversDataeLen)
                {
                    LOG_DebugHex((rxBuf + SPI_VALID_DATA_OFFSET + 4),validDataLen,"LocalSendSpiData");

                    LocalSendSpiData(priv, (rxBuf + SPI_VALID_DATA_OFFSET + 4), validDataLen);
                }
            }
            else
            {
                GpioSetLevel(priv->cpinnfd, GPIO_LEVEL_LOW);
                SpiCheckGpioLevel(priv->mpinnfd, GPIO_LEVEL_LOW);
            }
        }
    }
    return NULL;  
}

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
int32_t SpiInit(const uint8_t *devName, uint32_t mode, uint32_t bits, uint32_t speed, uint32_t cpinn, uint32_t mpinn)
{
    SpiApiPriv_t *priv;

    if (!devName)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return -1;
    }

    priv = (SpiApiPriv_t *) calloc(1, sizeof(SpiApiPriv_t));
    if (!priv)
    {
        LOG_Error("%s calloc fail.", __FUNCTION__);
        return -1;
    }

    strncpy((void *)priv->devInfo.devname, (void *)devName, strlen((void *)devName));
    priv->devInfo.mode = mode;
    priv->devInfo.bits = bits;
    priv->devInfo.speed = speed;
    priv->fd = -1;
    priv->threadId = -1;

    int32_t ret = GpioInit(&priv->cpinnfd, cpinn, GPIO_DIRECTION_OUT, GPIO_LEVEL_LOW);

    if (RET_OK != ret)
    {
        LOG_Error("%s cpinn GpioInit fail.", __FUNCTION__);
        free(priv);
		return -1;
    }

    ret = GpioInit(&priv->mpinnfd, mpinn, GPIO_DIRECTION_IN, GPIO_LEVEL_LOW);

    if (RET_OK != ret)
    {
        LOG_Error("%s mpinn GpioInit fail.", __FUNCTION__);
        GpioUnInit(priv->cpinnfd);
        free(priv);
		return -1;
    }

    priv->fd = open((void*)devName, O_RDWR);
    if (priv->fd < 0)
	{
        LOG_Error("%s can't open device ", __FUNCTION__);
        GpioUnInit(priv->mpinnfd);
        GpioUnInit(priv->cpinnfd);
        free(priv);
		return -1;
	}

    /*
    * spi mode
    */
    ret = ioctl(priv->fd, SPI_IOC_WR_MODE, &priv->devInfo.mode);
    if (ret == -1)
    {
        LOG_Error("%s spi mode set fail.", __FUNCTION__);
        close(priv->fd);
        GpioUnInit(priv->mpinnfd);
        GpioUnInit(priv->cpinnfd);
        free(priv);
        return -1;
    }

    /*
    * bits per word
    */
    ret = ioctl(priv->fd, SPI_IOC_WR_BITS_PER_WORD, &priv->devInfo.bits);
    if (ret == -1)
    {
        LOG_Error("%s bits per word set fail.", __FUNCTION__);
        close(priv->fd);
        GpioUnInit(priv->mpinnfd);
        GpioUnInit(priv->cpinnfd);
        free(priv);
        return -1;        
    }

    /*
    * max speed hz
    */
    ret = ioctl(priv->fd, SPI_IOC_WR_MAX_SPEED_HZ, &priv->devInfo.speed);
    if (ret == -1)
    {
        LOG_Error("%s max speed hz set fail.", __FUNCTION__);
        close(priv->fd);
        GpioUnInit(priv->mpinnfd);
        GpioUnInit(priv->cpinnfd);
        free(priv);
        return -1;
    }

    if (RET_ERR == RingBufferCreate(&priv->rbuf, RINGBUFFER_MAX_SIZE))
    {
        LOG_Error("%s RingBufferCreate fail.", __FUNCTION__);
        close(priv->fd);
        GpioUnInit(priv->mpinnfd);
        GpioUnInit(priv->cpinnfd);
        free(priv);
        return -1;        
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, priv->fds) == -1)
    {
        LOG_Error("%s socketpair fail.", __FUNCTION__);  
        RingBufferDestroy(priv->rbuf);
        close(priv->fd);
        GpioUnInit(priv->mpinnfd);
        GpioUnInit(priv->cpinnfd);
        free(priv);        
        return -1;
    }  

    ret = pthread_create(&priv->threadId, NULL, SpiThreadHandler, (void*)priv);  
    if (ret != 0)
    {
        LOG_Error("%s pthread_create fail.", __FUNCTION__); 
        close(priv->fds[1]);
        close(priv->fds[0]);
        RingBufferDestroy(priv->rbuf);
        close(priv->fd);
        GpioUnInit(priv->mpinnfd);
        GpioUnInit(priv->cpinnfd);
        free(priv);
        return -1;
    }

    return ((int32_t) priv);
}

/**
 * @name SpiRead
 * @brief 通过SPI接口从指定设备读取数据
 * @param fd 文件描述符，指向SPI设备的私有数据结构
 * @param data 指向接收数据缓冲区的指针
 * @param dataLen 要读取的数据长度
 * @return 读取成功返回实际读取的数据长度，失败返回-1
 */
int32_t SpiRead(int32_t fd, uint8_t *data, int32_t dataLen)
{
    SpiApiPriv_t *priv;
    if((fd <= 0) || !data)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return -1;
    }

    priv = (SpiApiPriv_t *) fd;

    return read(priv->fds[0], data, dataLen);
}

/**
 * @name SpiWrite
 * @brief 通过SPI接口进行写操作
 * @param fd 文件描述符，实际上是指向SpiApiPriv_t结构体的指针
 * @param data 要写入的数据指针
 * @param dataLen 要写入的数据长度
 * @return 实际写入的数据长度，出错返回-1
 */
int32_t SpiWrite(int32_t fd, const uint8_t *data, int32_t dataLen)
{
    SpiApiPriv_t *priv;
    if((fd <= 0) || !data)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return -1;
    }

    priv = (SpiApiPriv_t *) fd;
    uint32_t writeLen = 0;
    if (RET_ERR == RingBufferGetFreeSize(priv->rbuf, &writeLen))
    {
        LOG_Error("%s RingBufferGetFreeSize fail.", __FUNCTION__);
        return -1;
    }

    if (dataLen <= writeLen)
    {
        writeLen = dataLen;
    }

    if (RET_ERR == RingBufferEnqueue(priv->rbuf, data, writeLen))
    {
        LOG_Error("%s RingBufferEnqueue fail.", __FUNCTION__);
        return -1;
    }

    return writeLen;
}

/**
 * @name SpiClose
 * @brief 关闭SPI设备连接，该函数主要用于清理与SPI设备相关的资源，包括线程、管道、缓冲区和设备文件描述符等
 * @param fd 文件描述符，指向SPI设备的私有数据结构
 * @return 无返回值
 */
void SpiClose(int32_t fd)
{
    SpiApiPriv_t *priv;

    if (fd <= 0)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    priv = (SpiApiPriv_t *) fd;
    priv->threadExit = 1;
    pthread_join(priv->threadId, NULL);
    if(priv->fds[1] != -1)
    {
        close(priv->fds[1]);
    }
    
    if(priv->fds[1] != -1)
    {
        close(priv->fds[0]);
    }
    
    if (priv->rbuf)
    {
        RingBufferDestroy(priv->rbuf);        
    }

    if(priv->fd != -1)
    {
        close(priv->fd);
    }

    GpioUnInit(priv->mpinnfd);
    GpioUnInit(priv->cpinnfd);
    free(priv);
}