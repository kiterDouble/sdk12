#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <glib.h>

#include "err_code.h"
#include "sr_log.h"
#include "sr_ipc.h"
#include "mcu_can_filter.h"

/*
 * CAN ID过滤表请求/应答格式（signalServer 现有协议，signalServer 无需修改）:
 *   REQ(SYS_QUE_TYPE_MCU_VEHICLE_CAN_ID_REQ): reqIndex(2B BE) + reqNumber(2B BE)
 *   RES(SYS_QUE_TYPE_MCU_VEHICLE_CAN_ID_RES): reqIndex 回显(2B BE) + respListSize(2B BE)
 *                                             + N×5B(1B 通道<<5 + 4B 29bit CAN ID)
 * signalServer 的 RES 经 McuSrvSendMesg RPC 回送至本进程 mcu_send.c，
 * 截获后调用 McuCanFilterSaveFromRes 全局保存，不再下发 MCU。
 *
 * 包长约束（取两者较小者留余量）:
 *   - IPC 单包有效载荷上限 SYSQUE_MSG_DATA_MAX_LEN = 7KB (queue_msg.h)
 *   - mcu_send.c 组帧缓冲 5000B，payload 上限 4990B
 *   每页 900 条 -> RES = 4 + 900×5 = 4504B，同步线程逐页拉取，收到短页(<900)即完成。
 */
#define CAN_FILTER_REQ_LEN            (4U)
#define CAN_FILTER_RESP_HEAD_LEN      (4U)
#define CAN_FILTER_RESP_ENTRY_LEN     (5U)
#define CAN_FILTER_PAGE_NUM           (900U)
#define CAN_FILTER_MAX_NUM            (4096U)  /* 存储总上限: 树内无参考值，实际配置几十条，取4页余量 */
#define CAN_FILTER_SYNC_INTERVAL_US    (3U * G_USEC_PER_SEC) /* 页请求/重试统一间隔3s */
#define CAN_FILTER_SYNC_START_DELAY_US (1U * G_USEC_PER_SEC) /* 启动后先等待signalServer就绪再拉取 */
#define CAN_FILTER_SYNC_EMPTY_RETRY_US (5U * G_USEC_PER_SEC) /* 空表或失败后重新拉取间隔5s */

/* CAN_RAW 报文格式(与 canLogger 解析一致): 2B num(大端) + 帧数据
 * 定长格式: num×80B(CanFrameRecord_t)，变长格式: num×(16B头+dlc) */
#define CAN_RAW_BATCH_HEAD_LEN        (2U)
#define CAN_RAW_FRAME_HEAD_LEN        (16U)
#define CAN_RAW_FIXED_FRAME_LEN       (80U)
#define CAN_RAW_MAX_DLC               (64U)
#define CAN_RAW_FORWARD_BUF_LEN       (5000U)  /* 转发组包缓冲: 输出必短于输入, 按MCU接收帧上限分配 */

/**
 * @name McuCanFilterCtx_t
 * @brief CAN ID过滤表模块上下文，全局唯一实例 g_canFilterCtx
 */
typedef struct
{
    uint32_t filterList[CAN_FILTER_MAX_NUM]; /* 条目编码: canId | (channel << 29) */
    uint32_t filterNum;                      /* 已保存条目总数 */
    uint32_t lastPageIndex;                  /* 最近成功保存页的reqIndex */
    uint32_t lastPageSeq;                    /* 保存成功次数(页应答判重用) */
    uint16_t lastPageNum;                    /* 最近成功保存页条目数 */
    uint8_t  valid;                          /* 全部页拉完(收到短页)后置1 */
    uint8_t  failed;                         /* 存储溢出等不可恢复错误置1 */
    pthread_mutex_t lock;                    /* 上下文互斥锁 */
} McuCanFilterCtx_t;

static McuCanFilterCtx_t g_canFilterCtx = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
};

int32_t McuCanFilterSaveFromRes(const uint8_t *data, uint32_t len)
{
    int32_t ret = RET_ERR;
    uint16_t reqIndex = 0;
    uint16_t respNum = 0;
    uint32_t totalNum = 0;
    uint32_t channel = 0;
    uint32_t canId = 0;
    uint32_t i = 0;
    const uint8_t *entryData = NULL;

    do
    {
        /* 入参校验：空指针或连4字节包头都不够，直接丢弃 */
        if ((data == NULL) || (len < CAN_FILTER_RESP_HEAD_LEN))
        {
            LOG_Error("McuCanFilterSaveFromRes invalid len=%u", len);
            break;
        }

        /* 解析大端包头：本页起始条目号 + 本页实际条目数 */
        reqIndex = (uint16_t)(((uint16_t)data[0] << 8U) | data[1]);
        respNum = (uint16_t)(((uint16_t)data[2] << 8U) | data[3]);

        /* 包体完整性校验：按 respNum 推算的理论长度不能超过实际包长，
         * 防止对端异常（如respListSize字段错误）导致越界读 */
        if (len < (CAN_FILTER_RESP_HEAD_LEN + (uint32_t)respNum * CAN_FILTER_RESP_ENTRY_LEN))
        {
            LOG_Error("McuCanFilterSaveFromRes len mismatch: len=%u respNum=%u", len, respNum);
            break;
        }

        /* 存储上限校验：防止越界写全局数组。
         * 溢出属不可恢复错误，置 failed 让同步线程退出，避免无限重试 */
        if (((uint32_t)reqIndex + (uint32_t)respNum) > CAN_FILTER_MAX_NUM)
        {
            LOG_Error("McuCanFilterSaveFromRes filter num overflow: index=%u num=%u max=%u",
                      reqIndex, respNum, CAN_FILTER_MAX_NUM);
            pthread_mutex_lock(&g_canFilterCtx.lock);
            g_canFilterCtx.failed = 1;
            pthread_mutex_unlock(&g_canFilterCtx.lock);
            break;
        }

        pthread_mutex_lock(&g_canFilterCtx.lock);
        /* 逐条解析5字节条目并写入全局表：
         * byte0      : 高3位为CAN通道号(即 bit5-7)
         * byte1-4    : 29bit CAN ID(大端，byte1仅低5位有效)
         * 还原为与 signalServer 数据池一致的编码: canId | (channel << 29) */
        for (i = 0; i < (uint32_t)respNum; i++)
        {
            entryData = &data[CAN_FILTER_RESP_HEAD_LEN + i * CAN_FILTER_RESP_ENTRY_LEN];
            channel = (uint32_t)(entryData[0] >> 5U);
            canId = ((uint32_t)(entryData[1] & 0x1FU) << 24U) | ((uint32_t)entryData[2] << 16U) |
                    ((uint32_t)entryData[3] << 8U) | (uint32_t)entryData[4];
            g_canFilterCtx.filterList[(uint32_t)reqIndex + i] = (channel << 29U) | canId;
        }
        /* 更新总条数：分页按序到达，取最大覆盖范围即可 */
        if (((uint32_t)reqIndex + (uint32_t)respNum) > g_canFilterCtx.filterNum)
        {
            g_canFilterCtx.filterNum = (uint32_t)reqIndex + (uint32_t)respNum;
        }
        /* 记录本页页号/条数/序号，供同步线程判定"本页应答已到达"(seq判重) */
        g_canFilterCtx.lastPageIndex = reqIndex;
        g_canFilterCtx.lastPageNum = respNum;
        g_canFilterCtx.lastPageSeq++;
        totalNum = g_canFilterCtx.filterNum;
        pthread_mutex_unlock(&g_canFilterCtx.lock);

        /* 先解锁再打日志，缩短持锁时间 */
        LOG_Info("can id filter page saved: index=%u num=%u total=%u", reqIndex, respNum, totalNum);
        ret = RET_OK;
    } while (0);

    return ret;
}

int32_t McuCanFilterGetList(uint32_t *buf, uint32_t *num)
{
    int32_t ret = RET_ERR;
    uint32_t copyNum = 0;

    /* num 为必带出参；buf 可为 NULL，表示调用方仅查询条目数 */
    if (num == NULL)
    {
        LOG_Error("McuCanFilterGetList param invalid.");
        return ret;
    }

    pthread_mutex_lock(&g_canFilterCtx.lock);
    /* 仅在全表同步完成(valid)后对外提供，避免调用方拿到未拉完的半截表 */
    if (g_canFilterCtx.valid != 0U)
    {
        if (buf != NULL)
        {
            /* 缓冲区容量不足时按容量截断拷贝，num 始终返回实际总条数 */
            copyNum = (*num < g_canFilterCtx.filterNum) ? *num : g_canFilterCtx.filterNum;
            memcpy(buf, g_canFilterCtx.filterList, copyNum * sizeof(uint32_t));
        }
        *num = g_canFilterCtx.filterNum;
        ret = RET_OK;
    }
    pthread_mutex_unlock(&g_canFilterCtx.lock);

    return ret;
}

uint8_t McuCanFilterIsReady(void)
{
    uint8_t ready = 0;

    pthread_mutex_lock(&g_canFilterCtx.lock);
    ready = g_canFilterCtx.valid;
    pthread_mutex_unlock(&g_canFilterCtx.lock);

    return ready;
}

int32_t McuCanFilterIsMatch(uint8_t channel, uint32_t canId)
{
    int32_t ret = RET_ERR;
    uint32_t target = 0;
    uint32_t i = 0;

    /* 组合与 signalServer 数据池一致的key: (channel << 29) | canId
     * canId 的 bit31 为扩展帧标志，过滤表为29位ID空间，不参与比对 */
    target = (((uint32_t)channel & 0x7U) << 29U) | (canId & 0x1FFFFFFFU);

    pthread_mutex_lock(&g_canFilterCtx.lock);
    if (g_canFilterCtx.valid != 0U)
    {
        /* 线性查找：过滤表实际为几十条量级，开销可忽略 */
        for (i = 0; i < g_canFilterCtx.filterNum; i++)
        {
            if (g_canFilterCtx.filterList[i] == target)
            {
                ret = RET_OK;
                break;
            }
        }
    }
    pthread_mutex_unlock(&g_canFilterCtx.lock);

    return ret;
}

void McuCanFilterForwardRawToData(const uint8_t *payload, uint16_t srvLen)
{
    uint16_t rawNum = 0;
    uint16_t outNum = 0;
    uint16_t i = 0;
    uint16_t outLen = 0;
    uint32_t off = 0;
    uint32_t dataOff = 0;
    uint32_t frameAdv = 0;
    uint32_t canId = 0;
    uint8_t channel = 0;
    uint8_t dlc = 0;
    uint8_t isFixed = 0;
    /* 输出必短于输入(每帧16B/80B头缩为6B)，缓冲按接收上限分配即可 */
    uint8_t outBuf[CAN_RAW_FORWARD_BUF_LEN] = {0};

    do
    {
        /* 入参校验：空指针或连2字节批头都不够，直接返回 */
        if ((payload == NULL) || (srvLen < CAN_RAW_BATCH_HEAD_LEN))
        {
            LOG_Error("McuCanFilterForwardRawToData invalid param, len=%u", srvLen);
            break;
        }

        /* 过滤表未加载完成则不转发(signalServer暂无过滤依据)。
         * CAN_RAW频率高，仅打Debug避免刷日志 */
        if (McuCanFilterIsReady() == 0U)
        {
            // LOG_Debug("can filter not ready, skip CAN_RAW forward");
            break;
        }

        rawNum = (uint16_t)(((uint16_t)payload[0] << 8U) | payload[1]);

        /* 与 canLogger 一致的格式判定：定长(80B/帧)优先，长度不足按变长(16B头+dlc) */
        if ((CAN_RAW_BATCH_HEAD_LEN + (uint32_t)rawNum * CAN_RAW_FIXED_FRAME_LEN) <= (uint32_t)srvLen)
        {
            isFixed = 1;
        }

        off = CAN_RAW_BATCH_HEAD_LEN;
        outLen = CAN_RAW_BATCH_HEAD_LEN; /* 头部2B预留，末尾回填实际帧数 */

        for (i = 0; i < rawNum; i++)
        {
            /* 变长格式需逐帧校验头长；定长格式总长度已在判定范围内，无需再查 */
            if ((isFixed == 0U) && ((off + CAN_RAW_FRAME_HEAD_LEN) > (uint32_t)srvLen))
            {
                LOG_Error("CAN_RAW truncated frame head: i=%u off=%u len=%u", i, off, srvLen);
                outNum = 0;
                break;
            }

            /* 帧头16B布局: channel(1) flags(1) dlc(1) rsv(1) canId(4大端) tsMs(4) tsUs(4) */
            channel = payload[off];
            dlc = payload[off + 2U];
            canId = ((uint32_t)payload[off + 4U] << 24U) | ((uint32_t)payload[off + 5U] << 16U) |
                    ((uint32_t)payload[off + 6U] << 8U) | (uint32_t)payload[off + 7U];
            dataOff = off + CAN_RAW_FRAME_HEAD_LEN;

            if (dlc > CAN_RAW_MAX_DLC)
            {
                LOG_Error("CAN_RAW dlc error: i=%u ch=%u id=0x%x dlc=%u", i, channel, canId, dlc);
                outNum = 0;
                break;
            }

            if (isFixed != 0U)
            {
                /* 定长80B/帧：16B头 + 64B数据区，dlc标记有效字节数 */
                frameAdv = CAN_RAW_FIXED_FRAME_LEN;
            }
            else
            {
                frameAdv = CAN_RAW_FRAME_HEAD_LEN + dlc;
                if ((dataOff + dlc) > (uint32_t)srvLen)
                {
                    LOG_Error("CAN_RAW truncated frame data: i=%u off=%u dlc=%u len=%u", i, off, dlc, srvLen);
                    outNum = 0;
                    break;
                }
            }

            /* 命中过滤表才组 CAN_DATA 记录: 1B通道 + 4B ID(29bit大端) + 1B长度 + 数据
             * CAN ID 高3位可能带扩展帧等标志，清掉后发送，与过滤表编码及signalServer key计算保持一致 */
            if (McuCanFilterIsMatch(channel, canId) == RET_OK)
            {
                uint32_t sendCanId = canId & 0x1FFFFFFFU;

                outBuf[outLen] = channel;
                outLen++;
                outBuf[outLen] = (uint8_t)((sendCanId >> 24U) & 0xFFU);
                outLen++;
                outBuf[outLen] = (uint8_t)((sendCanId >> 16U) & 0xFFU);
                outLen++;
                outBuf[outLen] = (uint8_t)((sendCanId >> 8U) & 0xFFU);
                outLen++;
                outBuf[outLen] = (uint8_t)(sendCanId & 0xFFU);
                outLen++;
                outBuf[outLen] = dlc;
                outLen++;
                (void)memcpy(&outBuf[outLen], &payload[dataOff], dlc);
                outLen += (uint16_t)dlc;
                outNum++;
            }

            off += frameAdv;
        }

        /* 解析出错(outNum清0)或无命中帧，均不广播 */
        if (outNum == 0U)
        {
            break;
        }

        /* 回填实际帧数，按 signalServer 的 CAN_DATA 格式广播 */
        outBuf[0] = (uint8_t)((outNum >> 8U) & 0xFFU);
        outBuf[1] = (uint8_t)(outNum & 0xFFU);
        if (SendMsgToAllQueue(SYS_QUE_TYPE_MCU_VEHICLE_CAN_DATA, outBuf, (int32_t)outLen) != RET_OK)
        {
            LOG_Error("CAN_DATA forward fail, num=%u len=%u", outNum, outLen);
        }
        else
        {
            // LOG_Debug("CAN_DATA forward: rawNum=%u outNum=%u outLen=%u", rawNum, outNum, outLen);
        }
    } while (0);
}

/**
 * @name McuCanFilterRequest
 * @brief 通过IPC发布一页CAN ID过滤表请求，由signalServer应答
 * @param reqIndex 起始条目号
 * @return RET_OK 成功，RET_ERR 失败(如signalServer未启动，无订阅者)
 */
static int32_t McuCanFilterRequest(uint32_t reqIndex)
{
    int32_t ret = RET_ERR;
    uint8_t req[CAN_FILTER_REQ_LEN] = {0};

    /* 组4字节大端请求包：起始条目号(2B) + 每页请求条数(2B) */
    req[0] = (uint8_t)((reqIndex >> 8U) & 0xFFU);
    req[1] = (uint8_t)(reqIndex & 0xFFU);
    req[2] = (uint8_t)((CAN_FILTER_PAGE_NUM >> 8U) & 0xFFU);
    req[3] = (uint8_t)(CAN_FILTER_PAGE_NUM & 0xFFU);

    /* 发布到IPC，signalServer 订阅该类型后按分页协议应答 */
    ret = SendMsgToAllQueue(SYS_QUE_TYPE_MCU_VEHICLE_CAN_ID_REQ, req, (int32_t)sizeof(req));
    return ret;
}

/**
 * @name McuCanFilterSyncThread
 * @brief 逐页拉取过滤表直到收到短页(全部拉完)。
 *        每 3s 请求一页，未收到应答按同间隔原页重试；
 *        若同步失败或加载结果为空，5s 后从头重新拉取。
 *        注意：MCU 固件若也发 REQ，其应答同样会被截获保存（内容一致，仅可能影响
 *        "短页即完成"的判定时机，不影响表内容正确性）。
 */
static gpointer McuCanFilterSyncThread(gpointer arg)
{
    uint32_t reqIndex = 0;
    uint32_t seqBefore = 0;
    uint32_t totalNum = 0;
    uint16_t pageNum = 0;
    uint8_t pageArrived = 0;
    uint8_t syncFailed = 0;

    (void)arg;

    /* 启动后稍等片刻再开始请求，降低mcuServer先于signalServer启动时首包丢失、
     * 随后把MCU固件提前REQ换回的empty应答误当作本进程应答的概率 */
    g_usleep(CAN_FILTER_SYNC_START_DELAY_US);

    while (1)
    {
        /* 失败时等待5s后重置上下文并重新从头拉取 */
        pthread_mutex_lock(&g_canFilterCtx.lock);
        syncFailed = g_canFilterCtx.failed;
        pthread_mutex_unlock(&g_canFilterCtx.lock);

        if (syncFailed != 0U)
        {
            LOG_Info("can id filter sync failed, retry after 5s");
            g_usleep(CAN_FILTER_SYNC_EMPTY_RETRY_US);
            pthread_mutex_lock(&g_canFilterCtx.lock);
            g_canFilterCtx.failed = 0;
            g_canFilterCtx.valid = 0;
            g_canFilterCtx.filterNum = 0;
            g_canFilterCtx.lastPageIndex = 0;
            g_canFilterCtx.lastPageNum = 0;
            g_canFilterCtx.lastPageSeq = 0;
            pthread_mutex_unlock(&g_canFilterCtx.lock);
            reqIndex = 0;
            continue;
        }

        /* 读出本页判重基准序号，避免多次持锁 */
        pthread_mutex_lock(&g_canFilterCtx.lock);
        seqBefore = g_canFilterCtx.lastPageSeq;
        pthread_mutex_unlock(&g_canFilterCtx.lock);

        /* 请求一页后统一等待3s：
         * signalServer 在线时应答毫秒级到达，3s已足够富余；
         * 掉线时该间隔即重试间隔 */
        (void)McuCanFilterRequest(reqIndex);
        g_usleep(CAN_FILTER_SYNC_INTERVAL_US);

        /* 醒后判定本页应答是否到达：seq 有变化 且 页号匹配，
         * 防止把旧应答或MCU固件自身REQ换来的应答误算成本页 */
        pthread_mutex_lock(&g_canFilterCtx.lock);
        if ((g_canFilterCtx.lastPageSeq != seqBefore) && (g_canFilterCtx.lastPageIndex == reqIndex))
        {
            pageArrived = 1;
            pageNum = g_canFilterCtx.lastPageNum;
        }
        else
        {
            pageArrived = 0;
        }
        pthread_mutex_unlock(&g_canFilterCtx.lock);

        if (pageArrived != 0U)
        {
            /* 短页(不足一页)说明对端已无更多条目，全表拉取完成 */
            if (pageNum < CAN_FILTER_PAGE_NUM)
            {
                pthread_mutex_lock(&g_canFilterCtx.lock);
                totalNum = g_canFilterCtx.filterNum;
                pthread_mutex_unlock(&g_canFilterCtx.lock);

                /* 空表不认为完成，5s后从头重新拉取 */
                if (totalNum == 0U)
                {
                    LOG_Info("can id filter empty, retry after 5s");
                    g_usleep(CAN_FILTER_SYNC_EMPTY_RETRY_US);
                    pthread_mutex_lock(&g_canFilterCtx.lock);
                    g_canFilterCtx.valid = 0;
                    g_canFilterCtx.filterNum = 0;
                    g_canFilterCtx.lastPageIndex = 0;
                    g_canFilterCtx.lastPageNum = 0;
                    g_canFilterCtx.lastPageSeq = 0;
                    pthread_mutex_unlock(&g_canFilterCtx.lock);
                    reqIndex = 0;
                }
                else
                {
                    pthread_mutex_lock(&g_canFilterCtx.lock);
                    g_canFilterCtx.valid = 1;
                    pthread_mutex_unlock(&g_canFilterCtx.lock);
                    LOG_Info("can id filter sync done, total=%u", totalNum);
                    break;
                }
            }
            else
            {
                /* 满页说明可能还有后续条目，翻到下一页继续拉 */
                reqIndex += CAN_FILTER_PAGE_NUM;
            }
        }
        else
        {
            /* 无应答仅打Debug：signalServer 长期不在时不刷日志，
             * 需要排查时把日志级别开到DEBUG即可见 */
            LOG_Debug("can id filter no response, retry");
        }
    }

    /* 退出前再读一次标志，便于定位是正常完成还是失败退出 */
    pthread_mutex_lock(&g_canFilterCtx.lock);
    totalNum = g_canFilterCtx.filterNum;
    syncFailed = g_canFilterCtx.failed;
    pthread_mutex_unlock(&g_canFilterCtx.lock);
    LOG_Info("can id filter sync thread exit, total=%u failed=%u", totalNum, syncFailed);
    return NULL;
}

int32_t McuCanFilterInit(void)
{
    int32_t ret = RET_ERR;
    GThread *tid = NULL;

    /* 创建过滤表同步线程 */
    tid = g_thread_new("canFilterSync", McuCanFilterSyncThread, NULL);
    if (tid == NULL)
    {
        LOG_Error("McuCanFilterInit create sync thread fail");
        return ret;
    }
    /* unref 置为分离线程，退出后由glib自动回收资源，主线程无需join */
    g_thread_unref(tid);

    ret = RET_OK;
    return ret;
}
