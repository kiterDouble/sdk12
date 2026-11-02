#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <poll.h>
#include <time.h>

#include "sr_log.h"
#include "err_code.h"
#include "ring_buffer.h"
#include "spi_api.h"
#include "gpio_api.h"
#include "crcTool.h"   /* 提供 Crc16(..., Crc16Ccitt) */

/* ========================= 配置与常量 ========================= */
#define RINGBUFFER_MAX_SIZE (4096)   /* 发送侧缓冲 */
#define SPI_CHUNK_MAX       (2048)   /* 仅用于分块读（接收端） */
#define SPI_DUMMY_BYTE      (0x00)

/* 线上 SPI 帧头：55 BB + LEN(2B,BE) + CRC16(2B,BE) */
#define SPI_HDR0     (0x55)
#define SPI_HDR1     (0xBB)
#define SPI_HEAD_LEN (6)
#define SPI_DIRTY_MAX (2048)

#define SPI_TX_BODY_MAX   (1024) /* 每次发出的最大 payload，避免占总线太久 */

/* 事件驱动参数 */
#define PIN1_GUARD_US        (100)  /* 117 拉高后，读 SPI 前保护窗：50~300us 可调 */
#define RX_HDR_RETRY_CNT     (3)    /* 帧头读取重试次数 */
#define RX_HDR_RETRY_GAP_US  (100)  /* 重试间隔 */

#define SPI_RX_WATCHDOG_MS   (3000U)
#define SPI_KICK_MIN_GAP_MS  (1000U)
#define SPI_KICK_PULSE_US    (100U)
#define SPI_RX_FINISH_RETRY_CNT  (20U)
#define SPI_RX_FINISH_GAP_US     (200U)
#ifndef RET_TIMEOUT
#define RET_TIMEOUT (1)
#endif

/* ========================= 私有结构体 ========================= */
typedef struct
{
    uint8_t  devname[128];
    uint32_t mode;
    uint32_t bits;
    uint32_t speed;
} SpiDevInfo_t;

typedef struct
{
    SpiDevInfo_t devInfo;
    int32_t fd;           /* SPI 设备 */
    int32_t sock_rx;      /* 本地 socket 读端（对外 read） */
    int32_t sock_tx;      /* 本地 socket 写端（RX 线程写 payload） */
    int32_t cpinnfd;      /* PIN2：输出，我方“有数据可读”提示（master=120） */
    int32_t mpinnfd;      /* PIN1：输入，对端拉高表示它要发（slave=117） */
    RingBuffer_t rbuf;    /* 仅发送侧缓存 payload */
    pthread_t threadId;
    uint32_t threadExit;
    uint64_t lastRxOkMs;  /* 最近一次RX成功的monotonic */
    uint64_t lastKickMs;  /* 最近一次主动kick REQd的时间 */
} SpiApiPriv_t;

/* ========================= 小工具 ========================= */
static int32_t SleepMillis(uint32_t tms)
{
    struct timespec ts;
    ts.tv_sec  = (time_t)(tms / 1000U);
    ts.tv_nsec = (long)((tms % 1000U) * 1000000UL);
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {}
    return 0;
}

/* us 级小延时（用于就绪保护窗/头重试间隔） */
static inline void BusyWaitUs(uint32_t us)
{
    struct timespec t0, t1, step = {0, 1000}; /* 1us 步进 */
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (;;) {
        nanosleep(&step, NULL);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        uint64_t d = (t1.tv_sec - t0.tv_sec) * 1000000ULL + (t1.tv_nsec - t0.tv_nsec) / 1000ULL;
        if (d >= us) break;
    }
}

static int32_t SpiIo(SpiApiPriv_t *priv, const uint8_t *tx, uint8_t *rx, size_t len)
{
    if (!priv) return RET_ERR;
    struct spi_ioc_transfer tr;
    (void)memset(&tr, 0, sizeof(tr));
    tr.tx_buf        = (unsigned long)tx;  /* 可为 0 */
    tr.rx_buf        = (unsigned long)rx;  /* 可为 0 */
    tr.len           = len;
    tr.speed_hz      = priv->devInfo.speed;
    tr.bits_per_word = priv->devInfo.bits;
    tr.delay_usecs   = 0;
    if (ioctl(priv->fd, SPI_IOC_MESSAGE(1), &tr) < 0)
    {
        LOG_Error("SpiIo ioctl fail len=%zu errno=%d", len, errno);
        return RET_ERR;
    }
    return RET_OK;
}

/* 一次性写（不读取 MISO）——用于把“头+payload”合并一次发出 */
static int32_t SpiWriteOnce(SpiApiPriv_t *priv, const uint8_t *buf, size_t len)
{
    return SpiIo(priv, buf, NULL, len);
}

/* 分块读：用于从对端取回 payload（需要 MISO） */
static int32_t SpiReadBytes(SpiApiPriv_t *priv, uint8_t *rx, size_t len)
{
    static uint8_t dummy[SPI_CHUNK_MAX];
    if (!priv || !rx) return RET_ERR;
    (void)memset(dummy, SPI_DUMMY_BYTE, sizeof(dummy));
    size_t remain = len; uint8_t *p = rx;
    while (remain > 0U)
    {
        size_t chunk = (remain > SPI_CHUNK_MAX) ? SPI_CHUNK_MAX : remain;
        if (SpiIo(priv, dummy, p, chunk) != RET_OK) return RET_ERR;
        p += chunk; remain -= chunk;
    }
    return RET_OK;
}

static ssize_t WriteAll(int fd, const uint8_t *buf, size_t len)
{
    size_t off = 0;
    while (off < len)
    {
        ssize_t n = write(fd, buf + off, len - off);
        if (n > 0) { off += (size_t)n; continue; }
        if (n == -1 && errno == EINTR) continue;
        if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) continue;
        return -1;
    }
    return (ssize_t)off;
}

/*
 * MCU uses SPI DMA TX and keeps PIN1/IEQ high until TX_FINISH is raised.
 * Keep supplying clocks after the payload is read so the DMA can complete,
 * but discard the returned bytes instead of writing the same frame twice.
 */
static void FinishMcuRxTransfer(SpiApiPriv_t *priv)
{
    if (!priv) return;

    uint8_t discard[SPI_HEAD_LEN] = {0U};
    uint8_t level = 0U;
    if (GpioGetLevel(priv->mpinnfd, &level) == RET_OK &&
        level == GPIO_LEVEL_LOW)
    {
        return;
    }

    for (uint32_t i = 0U; i < SPI_RX_FINISH_RETRY_CNT; ++i)
    {
        if (SpiReadBytes(priv, discard, sizeof(discard)) != RET_OK)
        {
            break;
        }
        if (GpioGetLevel(priv->mpinnfd, &level) == RET_OK &&
            level == GPIO_LEVEL_LOW)
        {
            return;
        }
        BusyWaitUs(SPI_RX_FINISH_GAP_US);
    }

    // LOG_Warn("MCU IEQ stayed high after one SPI payload");
}

static int32_t GetPin1Level(SpiApiPriv_t *priv, uint8_t *level)
{
    if (!priv || !level) return RET_ERR;
    return (GpioGetLevel(priv->mpinnfd, level) == RET_OK) ? RET_OK : RET_ERR;
}

/* ========================= 接收：读头(6B)->读payload->写socket ========================= */
static int32_t RxHandleOnce(SpiApiPriv_t *priv)
{
    if (!priv) return RET_ERR;

    uint8_t hdr[SPI_HEAD_LEN] = {0};
    int hdrOk = 0;

    for (int tries = 0; tries < RX_HDR_RETRY_CNT; ++tries)
    {
        if (tries == 0) BusyWaitUs(PIN1_GUARD_US); /* 117 拉高后给 slave 就绪窗口 */
        else            BusyWaitUs(RX_HDR_RETRY_GAP_US);

        if (SpiReadBytes(priv, hdr, sizeof(hdr)) == RET_OK &&
            hdr[0] == SPI_HDR0 && hdr[1] == SPI_HDR1)
        { hdrOk = 1; break; }
    }
    if (!hdrOk)
    {
        // LOG_Warn("RX header not ready or dirty: %02X %02X", hdr[0], hdr[1]);
        /* 可选：丢弃一段脏流帮助重新同步 */
        uint8_t *drop = (uint8_t *)malloc(SPI_DIRTY_MAX);
        if (drop) { (void)SpiReadBytes(priv, drop, SPI_DIRTY_MAX); free(drop); }
        return RET_ERR;
    }

    // LOG_DebugHex(hdr, 6, "RX header");
    uint16_t bodyLen = (uint16_t)((hdr[2] << 8) | hdr[3]);
    // uint16_t crcHdr  = (uint16_t)((hdr[4] << 8) | hdr[5]);

    if (bodyLen == 0U)
    {
        /* 空包直接返回 OK，必要时可在此写 ACK */
        return RET_OK;
    }

    uint8_t *payload = (uint8_t *)malloc(bodyLen);
    if (!payload) return RET_ERR;

    if (SpiReadBytes(priv, payload, bodyLen) != RET_OK)
    {
        LOG_Error("RX read payload fail");
        free(payload);
        return RET_ERR;
    }

    /* 如需 CRC 校验，解注释 */
    // uint16_t crcCal = Crc16((void*)payload, (uint32_t)bodyLen, Crc16Ccitt);
    // if (crcCal != crcHdr) {
    //     LOG_Warn("RX crc mismatch hdr=0x%04X cal=0x%04X", crcHdr, crcCal);
    //     free(payload);
    //     return RET_ERR;
    // }

    if (WriteAll(priv->sock_tx, payload, bodyLen) < 0)
    {
        LOG_Error("socket write payload fail");
        free(payload);
        return RET_ERR;
    }

    // LOG_DebugHex(payload, bodyLen, "RX-PAYLOAD->SOCK");
    free(payload);
    return RET_OK;
}

/* ========================= 发送：一次 transfer 发送(头+payload) ========================= */
static int32_t TxHandleOnce(SpiApiPriv_t *priv)
{
    if (!priv) return RET_ERR;

    uint32_t avai = 0;
    if (RingBufferGetAvailableSize(priv->rbuf, &avai) != RET_OK || avai == 0U)
        return RET_ERR;

    /* 接收优先：对端正在发则我方先不发 */
    uint8_t pin1 = 0;
    (void)GetPin1Level(priv, &pin1);
    if (pin1 == GPIO_LEVEL_HIGH)
        return RET_ERR;

    uint16_t bodyLen = (uint16_t)((avai > SPI_TX_BODY_MAX) ? SPI_TX_BODY_MAX : avai);
    if (bodyLen == 0U) return RET_ERR;

    uint8_t *payload = (uint8_t *)malloc(bodyLen);
    if (!payload) return RET_ERR;

    if (RingBufferPeek(priv->rbuf, payload, bodyLen) != RET_OK)
    {
        free(payload);
        return RET_ERR;
    }

    /* 计算 CRC16-CCITT(0xFFFF) */
    uint16_t crc16 = Crc16((void*)payload, (uint32_t)bodyLen, Crc16Ccitt);

    /* 组“头+payload”到一块连续 buffer，单次 transfer 发送 */
    size_t  total = (size_t)SPI_HEAD_LEN + bodyLen;
    uint8_t *frame = (uint8_t *)malloc(total);
    if (!frame) { free(payload); return RET_ERR; }

    frame[0] = SPI_HDR0; frame[1] = SPI_HDR1;
    frame[2] = (uint8_t)(bodyLen >> 8);
    frame[3] = (uint8_t)(bodyLen & 0xFF);
    frame[4] = (uint8_t)(crc16   >> 8);
    frame[5] = (uint8_t)(crc16   & 0xFF);
    (void)memcpy(frame + SPI_HEAD_LEN, payload, bodyLen);

    /* 拉高 cpinn(120) → 一次性发送整帧 */
    (void)GpioSetLevel(priv->cpinnfd, GPIO_LEVEL_HIGH);
    if (SpiWriteOnce(priv, frame, total) != RET_OK)
    {
        LOG_Error("TX single-transfer fail");
        (void)GpioSetLevel(priv->cpinnfd, GPIO_LEVEL_LOW);
        free(frame); free(payload);
        return RET_ERR;
    }

    /* 发送成功，实际出队 */
    if (RingBufferDequeue(priv->rbuf, payload, bodyLen) != RET_OK)
    {
        LOG_Error("TX dequeue fail");
        (void)GpioSetLevel(priv->cpinnfd, GPIO_LEVEL_LOW);
        free(frame); free(payload);
        return RET_ERR;
    }

    (void)GpioSetLevel(priv->cpinnfd, GPIO_LEVEL_LOW);
    // LOG_DebugHex(payload, bodyLen, "TX-PAYLOAD");
    free(frame);
    free(payload);
    return RET_OK;
}

/* ========================= 线程与对外 API ========================= */
static uint64_t GetMonotonicMs(void)
{
    struct timespec ts;
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
}

/* 主动产生一次 REQ HIGH->LOW 脉冲。
 * - MCU 刚重启 (IRQ cnt==0): 下降沿把 spi_ready bit0 置位，恢复 TX。
 * - MCU 正常运行 (cnt>0):    上升沿复位 DMA RX，下降沿 BufferLen==0 空操作。
 */
static void KickMcuReq(SpiApiPriv_t *priv)
{
    if (!priv) return;
    (void)GpioSetLevel(priv->cpinnfd, GPIO_LEVEL_HIGH);
    BusyWaitUs(SPI_KICK_PULSE_US);
    (void)GpioSetLevel(priv->cpinnfd, GPIO_LEVEL_LOW);
}

static void *SpiThreadHandler(void *arg)
{
    SpiApiPriv_t *priv = (SpiApiPriv_t *)arg;
    if (!priv) return NULL;

    uint8_t rxPending = 0U;

    (void)GpioSetLevel(priv->cpinnfd, GPIO_LEVEL_LOW);
    priv->lastRxOkMs = GetMonotonicMs();
    priv->lastKickMs = 0;

    while (priv->threadExit == 0U)
    {
        /* 先等 117 上升沿事件；每 50ms 超时一次以便处理 TX */
        int rc = GpioWaitEdge(priv->mpinnfd, 50);
        if (rc == RET_OK)
        {
            rxPending = 1U;
        }
        else if (rc == RET_ERR)
        {
            /* GPIO 异常：避免空转 */
            SleepMillis(1);
        }

        /*
         * PIN1/IEQ stays high until the MCU DMA completion callback runs.
         * Reading again while the level is high can fetch the same static MCU
         * TX buffer again, so one rising edge authorizes exactly one payload.
         * Keep rxPending set on a failed read so that transfer can be retried.
         */
        /* Recover an already-high IEQ left over from an interrupted transfer. */
        if (rxPending == 0U)
        {
            uint8_t pin1 = 0U;
            if (GetPin1Level(priv, &pin1) == RET_OK && pin1 == GPIO_LEVEL_HIGH)
            {
                rxPending = 1U;
            }
        }

        if (rxPending != 0U)
        {
            if (RxHandleOnce(priv) == RET_OK)
            {
                rxPending = 0U;
                priv->lastRxOkMs = GetMonotonicMs();
                FinishMcuRxTransfer(priv);
            }
            else
            {
                uint8_t pin1 = 0U;
                if (GpioGetLevel(priv->mpinnfd, &pin1) == RET_OK &&
                    pin1 == GPIO_LEVEL_LOW)
                {
                    rxPending = 0U;
                }
            }
            continue;
        }

        uint32_t txAvail = 0;
        (void)RingBufferGetAvailableSize(priv->rbuf, &txAvail);
        if (txAvail > 0U)
        {
            (void)TxHandleOnce(priv);
            continue;
        }

        /* 空闲小睡，降低占用 */
        (void)SleepMillis(1U);

        uint64_t now = GetMonotonicMs();
        if ((now - priv->lastRxOkMs) > SPI_RX_WATCHDOG_MS &&
            (now - priv->lastKickMs) > SPI_KICK_MIN_GAP_MS)
        {
            LOG_Warn("MCU SPI RX idle %llums, kick REQ to recover MCU handshake",
                     (unsigned long long)(now - priv->lastRxOkMs));
            KickMcuReq(priv);
            priv->lastKickMs = now;
        }
    }
    return NULL;
}

int32_t SpiInit(const uint8_t *devName, uint32_t mode, uint32_t bits, uint32_t speed, uint32_t cpinn, uint32_t mpinn)
{
    SpiApiPriv_t *priv;
    int32_t ret;

    LOG_Debug("%s enter.", __FUNCTION__);
    if (devName == NULL)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return -1;
    }

    priv = (SpiApiPriv_t *)calloc(1, sizeof(SpiApiPriv_t));
    if (!priv)
    {
        LOG_Error("%s calloc fail.", __FUNCTION__);
        return -1;
    }

    (void)strncpy((char *)priv->devInfo.devname, (const char *)devName, sizeof(priv->devInfo.devname) - 1U);
    priv->devInfo.devname[sizeof(priv->devInfo.devname) - 1U] = '\0';
    priv->devInfo.mode = mode;
    priv->devInfo.bits = bits;
    priv->devInfo.speed = speed;
    priv->fd = -1;
    priv->sock_rx = -1;
    priv->sock_tx = -1;
    priv->threadId = (pthread_t)0;

    /* GPIO：PIN2 输出低，PIN1 输入 */
    if ((ret = GpioInit(&priv->cpinnfd, (uint16_t)cpinn, GPIO_DIRECTION_OUT, GPIO_LEVEL_LOW)) != RET_OK)
    {
        LOG_Error("cpinn GpioInit fail.");
        free(priv);
        return -1;
    }
    if ((ret = GpioInit(&priv->mpinnfd, (uint16_t)mpinn, GPIO_DIRECTION_IN, GPIO_LEVEL_LOW)) != RET_OK)
    {
        LOG_Error("mpinn GpioInit fail.");
        (void)GpioUnInit(priv->cpinnfd);
        free(priv);
        return -1;
    }

    /* 关键：设置 mpinn(=117) 的边沿为 rising（sysfs: .../gpio117/edge） */
    (void)GpioSetEdge((uint16_t)mpinn, GPIO_EDGE_RISING);

    /* 打开并配置 SPI */
    priv->fd = open((const char *)devName, O_RDWR);
    if (priv->fd < 0)
    {
        LOG_Error("%s can't open device", __FUNCTION__);
        (void)GpioUnInit(priv->mpinnfd);
        (void)GpioUnInit(priv->cpinnfd);
        free(priv);
        return -1;
    }
    if (ioctl(priv->fd, SPI_IOC_WR_MODE, &priv->devInfo.mode) == -1 ||
        ioctl(priv->fd, SPI_IOC_WR_BITS_PER_WORD, &priv->devInfo.bits) == -1 ||
        ioctl(priv->fd, SPI_IOC_WR_MAX_SPEED_HZ, &priv->devInfo.speed) == -1)
    {
        LOG_Error("%s spi config fail.", __FUNCTION__);
        (void)close(priv->fd);
        (void)GpioUnInit(priv->mpinnfd);
        (void)GpioUnInit(priv->cpinnfd);
        free(priv);
        return -1;
    }

    /* 发送侧环形缓冲 */
    if (RingBufferCreate(&priv->rbuf, RINGBUFFER_MAX_SIZE) == RET_ERR)
    {
        LOG_Error("%s RingBufferCreate(tx) fail.", __FUNCTION__);
        (void)close(priv->fd);
        (void)GpioUnInit(priv->mpinnfd);
        (void)GpioUnInit(priv->cpinnfd);
        free(priv);
        return -1;
    }

    /* 本地 socketpair */
    int sv[2] = {-1, -1};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1)
    {
        LOG_Error("%s socketpair fail.", __FUNCTION__);
        (void)RingBufferDestroy(priv->rbuf);
        (void)close(priv->fd);
        (void)GpioUnInit(priv->mpinnfd);
        (void)GpioUnInit(priv->cpinnfd);
        free(priv);
        return -1;
    }
    priv->sock_rx = sv[0];
    priv->sock_tx = sv[1];

    /* 启线程 */
    ret = pthread_create(&priv->threadId, NULL, SpiThreadHandler, (void *)priv);
    if (ret != 0)
    {
        LOG_Error("%s pthread_create fail.", __FUNCTION__);
        (void)close(priv->sock_rx);
        (void)close(priv->sock_tx);
        (void)RingBufferDestroy(priv->rbuf);
        (void)close(priv->fd);
        (void)GpioUnInit(priv->mpinnfd);
        (void)GpioUnInit(priv->cpinnfd);
        free(priv);
        return -1;
    }

    return (int32_t)priv;
}

int32_t SpiRead(int32_t fd, uint8_t *data, int32_t dataLen)
{
    if ((fd == -1) || (data == NULL) || (dataLen <= 0))
    {
        LOG_Error("%s param invalid.,fd:%d", __FUNCTION__, fd);
        return -1;
    }
    SpiApiPriv_t *priv = (SpiApiPriv_t *)fd;

    ssize_t n = read(priv->sock_rx, data, (size_t)dataLen);
    if (n < 0)
    {
        if (errno == EINTR) return 0;
        LOG_Error("SpiRead socket read fail: %d", errno);
        return -1;
    }
    if (n == 0) return 0; /* 暂无数据 */
    // LOG_DebugHex(data, (uint32_t)n, "SpiRead(rx-payload)");
    return (int32_t)n;
}

int32_t SpiWrite(int32_t fd, const uint8_t *data, int32_t dataLen)
{
    LOG_Debug("SpiWrite");
    if ((fd == -1) || (data == NULL) || (dataLen <= 0))
    {
        LOG_Error("%s param invalid.fd:%d", __FUNCTION__, fd);
        return -1;
    }

    SpiApiPriv_t *priv = (SpiApiPriv_t *)fd;
    uint32_t freeLen = 0;
    if (RingBufferGetFreeSize(priv->rbuf, &freeLen) == RET_ERR)
    {
        LOG_Error("%s RingBufferGetFreeSize fail.", __FUNCTION__);
        return -1;
    }

    uint32_t writeLen = ((uint32_t)dataLen <= freeLen) ? (uint32_t)dataLen : freeLen;
    if (writeLen == 0U)
    {
        LOG_Error("txRbuf full drop %d bytes", dataLen);
        return 0;
    }

    // LOG_DebugHex(data, writeLen, "SpiWrite(tx-payload)");
    if (RingBufferEnqueue(priv->rbuf, data, writeLen) == RET_ERR)
    {
        LOG_Error("%s RingBufferEnqueue fail.", __FUNCTION__);
        return -1;
    }
    return (int32_t)writeLen;
}

void SpiClose(int32_t fd)
{
    LOG_Debug("SpiClose");
    if (fd == -1)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    SpiApiPriv_t *priv = (SpiApiPriv_t *)fd;
    priv->threadExit = 1U;
    (void)pthread_join(priv->threadId, NULL);

    if (priv->rbuf) (void)RingBufferDestroy(priv->rbuf);
    if (priv->sock_rx >= 0) (void)close(priv->sock_rx);
    if (priv->sock_tx >= 0) (void)close(priv->sock_tx);
    if (priv->fd != -1) (void)close(priv->fd);

    (void)GpioUnInit(priv->mpinnfd);
    (void)GpioUnInit(priv->cpinnfd);

    free(priv);
}
