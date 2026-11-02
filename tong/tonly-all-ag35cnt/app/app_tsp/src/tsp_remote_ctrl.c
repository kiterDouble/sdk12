/**
 * @file    tsp_remote_ctrl.c
 * @brief   远控命令处理实现
 * 
 * 整体链路分为4步：
 * 1. 连接线程收到远控下行后调用预处理
 * 2. 预处理完成判重、FIFO入队
 * 3. 远控线程逐条取队列，按MCU协议打包并等待ACK/Result
 * 4. 执行结果上报到平台
 */

#include "tsp_remote_ctrl.h"
#include "tsp_log.h"
#include "tsp_app.h"
#include "sr_ipc.h"
#include "queue_msg.h"
#include "mcu_api.h"
#include "tsp_protocol.h"
#include "tsp_common.h"

/* PM Server SDK - for RTC wakeup handling */
#include "pm_server_api.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

/* ======================== 常量定义 ======================== */

/** 远控缓存最大条数 */
#define TLY_REMOTE_CTRL_CACHE_MAX       25

/** 等待MCU ACK的超时时间，单位毫秒 */
#define TLY_REMOTE_CTRL_MCU_ACK_TIMEOUT_MS      3000U

/** MCU Result额外等待保护时间，单位毫秒 */
#define TLY_REMOTE_CTRL_MCU_REPLY_EXTRA_MS      500U

/** 远控线程空队列时的轮询间隔，单位毫秒 */
#define TLY_REMOTE_CTRL_QUEUE_WAIT_MS           200U

/** MCU命令重发次数 */
#define TLY_REMOTE_CTRL_MCU_RETRY_COUNT         3

/** MCU命令重发间隔，单位毫秒 */
#define TLY_REMOTE_CTRL_MCU_RETRY_INTERVAL_MS   100U

/**
 * 远控执行期间阻止休眠的时长，单位毫秒
 *
 * 按最坏异常场景计算：
 * - MCU ACK等待：3秒 × 3次重试 = 9秒
 * - MCU Result等待：10秒 × 3次重试 = 30秒
 * - 额外保护时间：500ms × 3次 = 1.5秒
 * - 重试间隔：100ms × 2次 = 0.2秒
 * - 唤醒稳定时间：1秒
 * - 结果上报保护时间：5秒
 * 总计约：47秒，取整60秒确保覆盖最坏情况
 */
#define TLY_REMOTE_CTRL_SLEEP_EVENT_TIMEOUT_MS  60000U

/** 休眠事件名称 */
#define TLY_SLEEP_EVENT_REMOTE_CTRL             "tly_rmt"

/* ======================== 模块内部数据结构 ======================== */

/**
 * @brief MCU通信上下文
 * 封装MCU ACK/Result队列、互斥锁和就绪状态
 */
typedef struct {
    GAsyncQueue *ack_queue;      /* MCU ACK(0x6E)异步队列 */
    GAsyncQueue *res_queue;      /* MCU Result(0x6F)异步队列 */
    GMutex       queue_mutex;    /* 保护队列初始化和就绪状态 */
    GMutex       io_mutex;       /* 串行化MCU通信 */
    bool         ready;          /* MCU订阅是否就绪 */
} remote_ctrl_mcu_ctx_t;

/**
 * @brief 远控业务上下文
 * 封装缓存、执行状态和线程管理
 */
typedef struct {
    tly_remote_ctrl_cache_t      cache;       /* FIFO请求缓存 */
    tly_remote_ctrl_exec_state_t exec_state;  /* 当前执行状态 */
    volatile gboolean            running;     /* 模块运行标志 */
    GThread                     *worker;      /* 工作线程 */
} remote_ctrl_business_ctx_t;

/* ======================== 模块单例实例 ======================== */

static remote_ctrl_mcu_ctx_t       g_mcu_ctx       = {0};
static remote_ctrl_business_ctx_t  g_business_ctx  = {0};
static tly_app_t                  *g_app_ctx       = NULL;  /* 应用上下文，用于上报结果 */

/* ======================== 内部函数声明 ======================== */

static void remote_ctrl_ensure_preprocess_ready(void);
static void remote_ctrl_ensure_mcu_ready(void);
static void remote_ctrl_deinit_mcu(void);
static void remote_ctrl_drain_mcu_queue(GAsyncQueue *queue);
static void remote_ctrl_free_mcu_msg(tly_mcu_queue_msg_t *msg);

static void on_mcu_ack(const uint8_t *data, const uint16_t dataLen, void *userdata);
static void on_mcu_res(const uint8_t *data, const uint16_t dataLen, void *userdata);

static bool remote_ctrl_is_same_command(const tly_remote_ctrl_req_t *lhs,
                                        const tly_remote_ctrl_req_t *rhs);
static bool remote_ctrl_is_duplicate_in_cache(const tly_remote_ctrl_req_t *req);
static bool remote_ctrl_is_executing_same(const tly_remote_ctrl_req_t *req);
static void remote_ctrl_set_executing(bool executing, const tly_remote_ctrl_req_t *req);
static int remote_ctrl_cache_push(const tly_remote_ctrl_req_t *req);
static int remote_ctrl_take_next_req(tly_remote_ctrl_req_t *req);

static int remote_ctrl_send_mcu_command(const uint8_t *data, uint16_t dataLen,
                                        uint8_t *rxData, uint16_t rxDataCap,
                                        uint16_t *rxLen, uint32_t timeoutMs);
static int remote_ctrl_parse_mcu_response(const uint8_t *data, uint16_t len, 
                                          tly_mcu_response_t *resp);
static int remote_ctrl_build_mcu_cmd(const tly_remote_ctrl_req_t *req, 
                                     tly_mcu_command_t *mcuCmd);
static void remote_ctrl_execute_single(const tly_remote_ctrl_req_t *req);


/* ======================== 消息释放 ======================== */

static void remote_ctrl_free_mcu_msg(tly_mcu_queue_msg_t *msg)
{
    if (msg == NULL) {
        return;
    }
    if (msg->payload != NULL) {
        g_free(msg->payload);
        msg->payload = NULL;
    }
    g_free(msg);
}

/* ======================== 队列清空 ======================== */

static void remote_ctrl_drain_mcu_queue(GAsyncQueue *queue)
{
    tly_mcu_queue_msg_t *msg = NULL;
    if (queue == NULL) {
        return;
    }
    while ((msg = (tly_mcu_queue_msg_t *)g_async_queue_try_pop(queue)) != NULL) {
        remote_ctrl_free_mcu_msg(msg);
    }
}

/* ======================== 预处理准备 ======================== */

static void remote_ctrl_ensure_preprocess_ready(void)
{
    /* 预处理可能在接收线程先发生，也可能远控线程先启动，因此这里按
     * lazy-init方式初始化缓存和执行态锁，避免依赖固定启动顺序。 */
    if (g_business_ctx.cache.inited == false) {
        (void)pthread_mutex_init(&g_business_ctx.cache.mutex, NULL);
        g_business_ctx.cache.inited = true;
    }
    if (g_business_ctx.exec_state.inited == false) {
        (void)pthread_mutex_init(&g_business_ctx.exec_state.mutex, NULL);
        g_business_ctx.exec_state.inited = true;
    }
}

/* ======================== MCU准备 ======================== */

static void remote_ctrl_ensure_mcu_ready(void)
{
    g_mutex_lock(&g_mcu_ctx.queue_mutex);
    if (g_mcu_ctx.ready == false) {
        g_mcu_ctx.ack_queue = g_async_queue_new();
        g_mcu_ctx.res_queue = g_async_queue_new();
        if ((g_mcu_ctx.ack_queue != NULL) && (g_mcu_ctx.res_queue != NULL)) {
            /* 订阅MCU ACK和Result消息 */
            (void)SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_REMOTE_CONTROL_ACK,
                                       on_mcu_ack, NULL);
            (void)SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_REMOTE_CONTROL_RES,
                                       on_mcu_res, NULL);
            g_mcu_ctx.ready = true;
        }
    }
    g_mutex_unlock(&g_mcu_ctx.queue_mutex);
}

static void remote_ctrl_deinit_mcu(void)
{
    g_mutex_lock(&g_mcu_ctx.queue_mutex);
    if (g_mcu_ctx.ready == true) {
        (void)UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_REMOTE_CONTROL_ACK);
        (void)UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_REMOTE_CONTROL_RES);
        remote_ctrl_drain_mcu_queue(g_mcu_ctx.ack_queue);
        remote_ctrl_drain_mcu_queue(g_mcu_ctx.res_queue);
        if (g_mcu_ctx.ack_queue != NULL) {
            g_async_queue_unref(g_mcu_ctx.ack_queue);
            g_mcu_ctx.ack_queue = NULL;
        }
        if (g_mcu_ctx.res_queue != NULL) {
            g_async_queue_unref(g_mcu_ctx.res_queue);
            g_mcu_ctx.res_queue = NULL;
        }
        g_mcu_ctx.ready = false;
    }
    g_mutex_unlock(&g_mcu_ctx.queue_mutex);
}

/* ======================== MCU回调 ======================== */

static void on_mcu_ack(const uint8_t *data, const uint16_t dataLen, void *userdata)
{
    tly_mcu_queue_msg_t *msg = NULL;
    (void)userdata;
    if ((data == NULL) || (dataLen == 0U) || (g_mcu_ctx.ack_queue == NULL)) {
        return;
    }
    /* 回调来自IPC线程，不能在这里做复杂逻辑，只把原始payload复制入队，
     * 由远控执行线程按顺序消费。 */
    msg = (tly_mcu_queue_msg_t *)g_malloc0(sizeof(*msg));
    if (msg == NULL) {
        return;
    }
    msg->payload = (uint8_t *)g_malloc((gsize)dataLen);
    if (msg->payload == NULL) {
        g_free(msg);
        return;
    }
    msg->payload_len = (uint32_t)dataLen;
    (void)memcpy(msg->payload, data, (size_t)dataLen);
    g_async_queue_push(g_mcu_ctx.ack_queue, msg);
}

static void on_mcu_res(const uint8_t *data, const uint16_t dataLen, void *userdata)
{
    tly_mcu_queue_msg_t *msg = NULL;
    (void)userdata;
    if ((data == NULL) || (dataLen == 0U) || (g_mcu_ctx.res_queue == NULL)) {
        return;
    }
    /* MCU Result也先转存到异步队列，避免在IPC回调上下文里阻塞。 */
    msg = (tly_mcu_queue_msg_t *)g_malloc0(sizeof(*msg));
    if (msg == NULL) {
        return;
    }
    msg->payload = (uint8_t *)g_malloc((gsize)dataLen);
    if (msg->payload == NULL) {
        g_free(msg);
        return;
    }
    msg->payload_len = (uint32_t)dataLen;
    (void)memcpy(msg->payload, data, (size_t)dataLen);
    g_async_queue_push(g_mcu_ctx.res_queue, msg);
}

/* ======================== 电池加热RTC唤醒处理 ======================== */

/**
 * @brief 处理电池加热RTC唤醒
 * @param[in] pmData PM电源状态数据
 * @description 当检测到电池加热定时器唤醒时，触发MCU唤醒并发送电池加热命令
 */
void remote_ctrl_handle_battery_heat_wakeup(const MPUPmStateChange_t *pmData, tly_app_t *app)
{
    if (pmData == NULL) {
        TLY_LOG_E("remote_ctrl_handle_battery_heat_wakeup: invalid params");
        return;
    }

    /* 检查是否是定时器唤醒 */
    if (pmData->WakeupSrc == PM_WAKEUP_SOURCE_TIMER_MCU) {
        /* 检查是否是电池加热的RTC闹钟事件 */
        if (pmData->SleepAlarmEventId == CPU_APP_TIMER_BATTERY_HEAT) {
            TLY_LOG_I("Battery heat RTC wakeup received, alarmId=%u, sending command to MCU",
                      pmData->SleepAlarmEventId);

            /* 发送电池加热命令到MCU */
            tly_lock_control_t battery_heat_lock;
            memset(&battery_heat_lock, 0, sizeof(battery_heat_lock));
            battery_heat_lock.ctrl_cmd = TLY_CTRL_BATTERY_HEAT;
            battery_heat_lock.lock_type = 1; /* 电池加热类型 */
            send_remote_cmd(NULL, &battery_heat_lock);
        } else {
            TLY_LOG_D("PM state change: TIMER wakeup but not battery heat alarm (eventId=%u)",
                      pmData->SleepAlarmEventId);
        }
    }
}

/* ======================== 判重逻辑 ======================== */

static bool remote_ctrl_is_same_command(const tly_remote_ctrl_req_t *lhs,
                                        const tly_remote_ctrl_req_t *rhs)
{
    if ((lhs == NULL) || (rhs == NULL)) {
        return false;
    }
    /* 比较控制指令和锁车类型 */
    return (lhs->ctrl_cmd == rhs->ctrl_cmd) && (lhs->lock_type == rhs->lock_type);
}

static bool remote_ctrl_is_duplicate_in_cache(const tly_remote_ctrl_req_t *req)
{
    uint32_t idx = 0U;
    uint32_t i = 0U;
    
    pthread_mutex_lock(&g_business_ctx.cache.mutex);
    idx = g_business_ctx.cache.head;
    for (i = 0U; i < g_business_ctx.cache.count; i++) {
        if (remote_ctrl_is_same_command(req, &g_business_ctx.cache.nodes[idx].req)) {
            pthread_mutex_unlock(&g_business_ctx.cache.mutex);
            return true;
        }
        idx = (idx + 1U) % TLY_REMOTE_CTRL_CACHE_MAX;
    }
    pthread_mutex_unlock(&g_business_ctx.cache.mutex);
    return false;
}

static bool remote_ctrl_is_executing_same(const tly_remote_ctrl_req_t *req)
{
    bool same = false;
    pthread_mutex_lock(&g_business_ctx.exec_state.mutex);
    if (g_business_ctx.exec_state.executing) {
        same = remote_ctrl_is_same_command(req, &g_business_ctx.exec_state.current_req);
    }
    pthread_mutex_unlock(&g_business_ctx.exec_state.mutex);
    return same;
}

static void remote_ctrl_set_executing(bool executing, const tly_remote_ctrl_req_t *req)
{
    /* 执行完成后必须清空executing状态，否则后续同命令会被永久误判。 */
    pthread_mutex_lock(&g_business_ctx.exec_state.mutex);
    g_business_ctx.exec_state.executing = executing;
    if (executing && (req != NULL)) {
        (void)memcpy(&g_business_ctx.exec_state.current_req, req, sizeof(*req));
    } else {
        (void)memset(&g_business_ctx.exec_state.current_req, 0,
                     sizeof(g_business_ctx.exec_state.current_req));
    }
    pthread_mutex_unlock(&g_business_ctx.exec_state.mutex);
}

/* ======================== FIFO缓存操作 ======================== */

static int remote_ctrl_cache_push(const tly_remote_ctrl_req_t *req)
{
    int rc = 0;
    /* 使用固定长度环形队列实现25条缓存上限，符合需求里的FIFO约束。 */
    pthread_mutex_lock(&g_business_ctx.cache.mutex);
    if (g_business_ctx.cache.count >= TLY_REMOTE_CTRL_CACHE_MAX) {
        rc = -1;
    } else {
        (void)memcpy(&g_business_ctx.cache.nodes[g_business_ctx.cache.tail].req,
                     req, sizeof(*req));
        g_business_ctx.cache.tail = (g_business_ctx.cache.tail + 1U) % TLY_REMOTE_CTRL_CACHE_MAX;
        g_business_ctx.cache.count++;
    }
    pthread_mutex_unlock(&g_business_ctx.cache.mutex);
    return rc;
}

static int remote_ctrl_take_next_req(tly_remote_ctrl_req_t *req)
{
    int rc = -1;
    if (req == NULL) {
        return -1;
    }
    /* 取队列和标记executing必须放在同一临界区内完成，避免"刚出队还未标记"
     * 的瞬间又有新请求进来，从而漏判同命令执行中。 */
    pthread_mutex_lock(&g_business_ctx.exec_state.mutex);
    pthread_mutex_lock(&g_business_ctx.cache.mutex);
    if (g_business_ctx.cache.count > 0U) {
        (void)memcpy(req, &g_business_ctx.cache.nodes[g_business_ctx.cache.head].req,
                     sizeof(*req));
        g_business_ctx.cache.head = (g_business_ctx.cache.head + 1U) % TLY_REMOTE_CTRL_CACHE_MAX;
        g_business_ctx.cache.count--;
        g_business_ctx.exec_state.executing = true;
        (void)memcpy(&g_business_ctx.exec_state.current_req, req, sizeof(*req));
        rc = 0;
    }
    pthread_mutex_unlock(&g_business_ctx.cache.mutex);
    pthread_mutex_unlock(&g_business_ctx.exec_state.mutex);
    return rc;
}

/* ======================== MCU命令构建 ======================== */

static int remote_ctrl_build_mcu_cmd(const tly_remote_ctrl_req_t *req,
                                     tly_mcu_command_t *mcuCmd)
{
    if ((req == NULL) || (mcuCmd == NULL)) {
        return -1;
    }
    memset(mcuCmd, 0, sizeof(*mcuCmd));
    
    /* MCU命令格式: 4字节
     * data0: ctrl_cmd (控制指令/发动机厂商标识/功能码)
     * data1: 0x01 (命令类型标识)
     * data2: lock_type/子命令 (根据ctrl_cmd不同含义不同)
     * data3: 预留
     */
    uint8_t mcu_lock_type = req->lock_type;
    
    /* 根据ctrl_cmd类型进行不同的处理 */
    switch (req->ctrl_cmd) {
    case TLY_CTRL_LOCK_YUCHAI:  /* 玉柴: 平台值与MCU值一致，直接透传 */
        mcu_lock_type = req->lock_type;
        break;
        
    case TLY_CTRL_LOCK_WEICHAI: /* 潍柴 */
    case TLY_CTRL_LOCK_XICHAI:  /* 锡柴 */
    case TLY_CTRL_LOCK_CUMMINS: /* 康明斯 */
    case TLY_CTRL_LOCK_MAN:     /* 曼 */
    case TLY_CTRL_LOCK_EV:      /* 电车: 与潍柴/锡柴等使用相同的转换逻辑 */
        /* 平台: 0解锁, 1锁车, 2激活, 3取消激活
         * MCU:  0解锁, 1锁车, 2限扭矩, 3激活, 4取消激活
         * 差异: 平台没有"限扭矩"类型，2和3需要映射到MCU的3和4
         */
        switch (req->lock_type) {
        case 0: /* 解锁 */
            mcu_lock_type = 0;
            break;
        case 1: /* 锁车 */
            mcu_lock_type = 1;
            break;
        case 2: /* 激活 */
            mcu_lock_type = 3;
            break;
        case 3: /* 取消激活 */
            mcu_lock_type = 4;
            break;
        default:
            TLY_LOG_W("Unknown lock_type %u for ctrl_cmd 0x%02X, using as-is",
                      req->lock_type, req->ctrl_cmd);
            mcu_lock_type = req->lock_type;
            break;
        }
        break;
        
    case TLY_CTRL_BATTERY_HEAT: /* 电池包加热: 直接透传lock_type作为子命令 */
        /* 电池加热的lock_type直接作为子命令传递给MCU
         * 例如: 1=开始加热, 0=停止加热(如果有)
         */
        mcu_lock_type = req->lock_type;
        TLY_LOG_I("Battery heat command: ctrl_cmd=0x%02X, sub_cmd=%u",
                  req->ctrl_cmd, mcu_lock_type);
        break;
        
    case TLY_CTRL_SD_CARD:      /* SD卡数据获取 */
    case TLY_CTRL_TIME_SET:     /* 主动授时 */
    case TLY_CTRL_VIN_UNBIND:   /* VIN解绑 */
        /* 这些命令直接透传lock_type作为子命令 */
        mcu_lock_type = req->lock_type;
        TLY_LOG_I("Other ctrl command: ctrl_cmd=0x%02X, sub_cmd=%u",
                  req->ctrl_cmd, mcu_lock_type);
        break;
        
    default:
        /* 其他未知命令: 直接透传 */
        mcu_lock_type = req->lock_type;
        TLY_LOG_W("Unknown ctrl_cmd 0x%02X, passing through lock_type=%u",
                  req->ctrl_cmd, mcu_lock_type);
        break;
    }
    
    mcuCmd->cmd_data[0] = req->ctrl_cmd;
    mcuCmd->cmd_data[1] = 0x01; /* 命令类型: 0x01=请求 */
    mcuCmd->cmd_data[2] = mcu_lock_type;
    mcuCmd->cmd_data[3] = 0x00; /* 预留 */
    
    TLY_LOG_I("MCU cmd built: ctrl_cmd=0x%02X, cmd_type=0x01, platform_lock_type=%u -> mcu_lock_type=%u",
              mcuCmd->cmd_data[0], req->lock_type, mcuCmd->cmd_data[2]);
    return 0;
}

/* ======================== MCU响应解析 ======================== */

static int remote_ctrl_parse_mcu_response(const uint8_t *data, uint16_t len,
                                          tly_mcu_response_t *resp)
{
    /* MCU响应格式(4字节): data[0]=ctrl_cmd, data[1]=cmd_type(0x03), data[2]=result, data[3]=fail_reason
     * 必须收到完整的4个字节 */
    if ((data == NULL) || (len != 4) || (resp == NULL)) {
        return -1;
    }
    memset(resp, 0, sizeof(*resp));
    resp->result = data[2];
    resp->fail_reason = data[3];
    return 0;
}

/* ======================== MCU通信 ======================== */

static int remote_ctrl_send_mcu_command_once(const uint8_t *data, uint16_t dataLen,
                                             uint8_t *rxData, uint16_t rxDataCap,
                                             uint16_t *rxLen, uint32_t timeoutMs)
{
    McuMesg_t mcuMsg;
    tly_mcu_queue_msg_t *ackMsg = NULL;
    tly_mcu_queue_msg_t *resMsg = NULL;
    int rc = -1;
    
    /* 清空队列，避免收到旧消息 */
    remote_ctrl_drain_mcu_queue(g_mcu_ctx.ack_queue);
    remote_ctrl_drain_mcu_queue(g_mcu_ctx.res_queue);
    
    memset(&mcuMsg, 0, sizeof(mcuMsg));
    mcuMsg.type = SYS_QUE_TYPE_MCU_REMOTE_CONTROL_REQ;
    mcuMsg.payload = (uint8_t *)data;
    mcuMsg.payloadLen = dataLen;

    TLY_LOG_Dump("MCU cmd send:", data, dataLen);
    
    if (McuSrvSendMesg(&mcuMsg) != 0) {
        TLY_LOG_E("Remote ctrl send MCU msg failed");
        return -1;
    }
    
    /* MCU正常交互顺序固定为：REQ -> ACK(0x6E) -> RESULT(0x6F)。
     * 命令格式(4字节): data[0]=ctrl_cmd, data[1]=cmd_type, data[2]=data[3]=预留/结果
     * cmd_type: 0x01=请求, 0x02=ACK回复, 0x03=Result回复
     */
    ackMsg = (tly_mcu_queue_msg_t *)g_async_queue_timeout_pop(
            g_mcu_ctx.ack_queue,
            (guint64)TLY_REMOTE_CTRL_MCU_ACK_TIMEOUT_MS * 1000U);
    if (ackMsg == NULL) {
        TLY_LOG_W("Remote ctrl wait MCU ACK timeout");
        return -1;
    }
    
    /* 打印收到的ACK原始数据 */
    TLY_LOG_Dump("MCU ACK recv:", ackMsg->payload, ackMsg->payload_len);

    resMsg = (tly_mcu_queue_msg_t *)g_async_queue_timeout_pop(
            g_mcu_ctx.res_queue,
            (guint64)timeoutMs * 1000U);
    if (resMsg == NULL) {
        TLY_LOG_E("Remote ctrl wait MCU result timeout");
        remote_ctrl_free_mcu_msg(ackMsg);
        return -1;
    }
    
    /* 打印收到的Result原始数据 */
    TLY_LOG_Dump("MCU Result recv:", resMsg->payload, resMsg->payload_len);

    *rxLen = (uint16_t)((resMsg->payload_len > rxDataCap) ? rxDataCap : resMsg->payload_len);
    (void)memcpy(rxData, resMsg->payload, *rxLen);
    remote_ctrl_free_mcu_msg(ackMsg);
    remote_ctrl_free_mcu_msg(resMsg);
    return 0;
}

static int remote_ctrl_send_mcu_command(const uint8_t *data, uint16_t dataLen,
                                        uint8_t *rxData, uint16_t rxDataCap,
                                        uint16_t *rxLen, uint32_t timeoutMs)
{
    int rc = -1;
    int retry = 0;
    
    if ((data == NULL) || (dataLen == 0U) || (rxData == NULL) || (rxLen == NULL)) {
        return -1;
    }
    if (g_mcu_ctx.ready == false) {
        return -1;
    }
    
    /* ACK/Result队列是全局共享的，同一时刻只允许一条远控请求在途，
     * 否则不同命令的ACK/Result会互相串扰。 */
    g_mutex_lock(&g_mcu_ctx.io_mutex);
    
    /* 重发机制：最多尝试TLY_REMOTE_CTRL_MCU_RETRY_COUNT次 */
    for (retry = 0; retry < TLY_REMOTE_CTRL_MCU_RETRY_COUNT; retry++) {
        if (retry > 0) {
            TLY_LOG_I("Remote ctrl retry %d/%d", retry, TLY_REMOTE_CTRL_MCU_RETRY_COUNT);
            g_usleep(TLY_REMOTE_CTRL_MCU_RETRY_INTERVAL_MS * 1000U);
        }
        
        rc = remote_ctrl_send_mcu_command_once(data, dataLen, rxData, rxDataCap, rxLen, timeoutMs);
        if (rc == 0) {
            /* 发送成功，跳出重发循环 */
            break;
        }
    }
    
    g_mutex_unlock(&g_mcu_ctx.io_mutex);
    return rc;
}

/* ======================== 结果上报 ======================== */

void remote_ctrl_report_result(const tly_remote_ctrl_req_t *req,
                                      const tly_process_result_t *result)
{
    /* 记录日志 */
    TLY_LOG_I("Remote ctrl result: ctrl_cmd=0x%02X, lock_type=%u, code=%u, fail_reason=%u",
              req->ctrl_cmd, req->lock_type, result->code, result->fail_reason);
    
    /* 上报结果到平台 */
    if (g_app_ctx != NULL) {
        /* 构造响应数据: 1字节ctrl_cmd + 6字节时间 + 1字节结果 + 1字节失败原因 */
        uint8_t resp_data[9];
        resp_data[0] = req->ctrl_cmd;  /* 最开头加ctrl_cmd */
        tly_time_now(&resp_data[1]);   /* 时间从第2个字节开始 */
        resp_data[7] = (result->code == TLY_REMOTE_CTRL_RESULT_SUCCESS) ? 0x00 : 0x01;
        resp_data[8] = result->fail_reason;
        
        /* 远控命令的cmd_id是0x82 */
        int rc = tly_app_send_response(g_app_ctx, 0x82,
                                       (result->code == TLY_REMOTE_CTRL_RESULT_SUCCESS) ? 0x00 : 0x01,
                                       resp_data, sizeof(resp_data));
        if (rc == 0) {
            TLY_LOG_I("Remote ctrl result reported to platform: success=%d",
                      result->code == TLY_REMOTE_CTRL_RESULT_SUCCESS);
        } else {
            TLY_LOG_E("Failed to report remote ctrl result: rc=%d", rc);
        }
    } else {
        TLY_LOG_W("Cannot report result: app context not set");
    }
}


/* ======================== 单条执行 ======================== */

static void remote_ctrl_execute_single(const tly_remote_ctrl_req_t *req)
{
    tly_process_result_t result;
    tly_mcu_command_t mcuCmd;
    tly_mcu_response_t resp;
    uint8_t rxBuf[16] = {0};
    uint16_t rxLen = 0U;
    /* 使用请求中指定的超时时间，若未指定则使用默认值 */
    uint32_t timeoutMs = (req->timeout > 0) ? req->timeout : 10000U;
    
    memset(&result, 0, sizeof(result));
    
    /* 远控是典型唤醒型业务，执行前先设置休眠阻止，避免系统进入低功耗 */
    (void)PmSetSleepEvent(TLY_SLEEP_EVENT_REMOTE_CTRL);
    
    /* 给系统一点时间完成唤醒和稳定 */
    g_usleep(1000U * 1000U);
    
    /* 构建MCU命令 */
    if (remote_ctrl_build_mcu_cmd(req, &mcuCmd) != 0) {
        result.code = TLY_REMOTE_CTRL_RESULT_TIMEOUT;
        result.fail_reason = 0;
        goto report;
    }
    
    /* 发送给MCU并等待响应 */
    if (remote_ctrl_send_mcu_command(mcuCmd.cmd_data, 4,
                                     rxBuf, sizeof(rxBuf), &rxLen,
                                     timeoutMs) != 0) {
        result.code = TLY_REMOTE_CTRL_RESULT_TIMEOUT;
        result.fail_reason = 0;
        goto report;
    }
    
    /* 解析响应 */
    if (remote_ctrl_parse_mcu_response(rxBuf, rxLen, &resp) != 0) {
        result.code = TLY_REMOTE_CTRL_RESULT_TIMEOUT;
        result.fail_reason = 0;
        goto report;
    }
    
    result.code = resp.result;
    result.fail_reason = resp.fail_reason;
    
report:
    /* 无论MCU执行成功还是失败，都必须先清executing，再回平台，
     * 否则上行过程中又收到新命令时会把后续请求误判成"正在执行"。 */
    remote_ctrl_set_executing(false, NULL);
    if (req->ctrl_cmd != TLY_CTRL_BATTERY_HEAT) {
        remote_ctrl_report_result(req, &result);
    }
    
    /* 远控执行完成，立即清除休眠阻止 */
    (void)PmCleanSleepEvent(TLY_SLEEP_EVENT_REMOTE_CTRL);
}

/* ======================== 对外接口 ======================== */

int remote_ctrl_init(tly_app_t *app)
{
    if (app == NULL) {
        TLY_LOG_E("Remote ctrl init failed: app is NULL");
        return -1;
    }
    
    TLY_LOG_I("Remote ctrl module initializing...");
    
    /* 保存应用上下文 */
    g_app_ctx = app;
    
    /* 初始化互斥锁 */
    g_mutex_init(&g_mcu_ctx.queue_mutex);
    g_mutex_init(&g_mcu_ctx.io_mutex);
    
    /* 确保预处理相关结构已初始化 */
    remote_ctrl_ensure_preprocess_ready();
    
    /* 确保MCU订阅已就绪 */
    remote_ctrl_ensure_mcu_ready();
    
    /* 启动工作线程 */
    g_business_ctx.running = TRUE;
    g_business_ctx.worker = g_thread_new("remote_ctrl", remote_ctrl_exec_thread, NULL);
    if (g_business_ctx.worker == NULL) {
        TLY_LOG_E("Failed to create worker thread");
        return -1;
    }
    
    TLY_LOG_I("Remote ctrl module initialized");
    return 0;
}

void remote_ctrl_deinit(void)
{
    TLY_LOG_I("Remote ctrl module deinitializing...");
    
    g_business_ctx.running = FALSE;
    
    /* 等待线程结束 */
    if (g_business_ctx.worker != NULL) {
        g_thread_join(g_business_ctx.worker);
        g_business_ctx.worker = NULL;
    }
    
    /* 反初始化MCU订阅 */
    remote_ctrl_deinit_mcu();
    
    /* 清理互斥锁 */
    g_mutex_clear(&g_mcu_ctx.queue_mutex);
    g_mutex_clear(&g_mcu_ctx.io_mutex);
    
    TLY_LOG_I("Remote ctrl module deinitialized");
}

/* ======================== 执行线程 ======================== */

gpointer remote_ctrl_exec_thread(gpointer data)
{
    tly_remote_ctrl_req_t req;
    (void)data;
    
    TLY_LOG_I("Remote ctrl exec thread started");
    remote_ctrl_ensure_mcu_ready();
    
    while (g_business_ctx.running) {
        /* 从FIFO缓存取出下一条待执行请求 */
        if (remote_ctrl_take_next_req(&req) != 0) {
            /* 队列为空，等待一段时间再试 */
            g_usleep(TLY_REMOTE_CTRL_QUEUE_WAIT_MS * 1000U);
            continue;
        }
        
        TLY_LOG_I("Executing remote ctrl: ctrl_cmd=0x%02X, lock_type=%u", req.ctrl_cmd, req.lock_type);
        
        /* 执行单条远控 */
        remote_ctrl_execute_single(&req);
    }
    
    TLY_LOG_I("Remote ctrl exec thread exited");
    return NULL;
}

/* ======================== 对外接口 ======================== */

void send_remote_cmd(tly_app_t *app, const tly_lock_control_t *lock)
{
    tly_remote_ctrl_req_t req;
    
    (void)app;
    
    if (!lock) {
        TLY_LOG_E("Invalid lock parameter");
        return;
    }
    
    /* 构造请求 */
    memset(&req, 0, sizeof(req));
    req.ctrl_cmd = lock->ctrl_cmd;
    req.lock_type = lock->lock_type;
    req.timeout = 15000; /* 默认15秒超时 */
    
    /* 判重检查 - 检查缓存中是否已有同类命令 */
    if (remote_ctrl_is_duplicate_in_cache(&req)) {
        TLY_LOG_W("Duplicate command in cache");
        return;
    }
    
    /* 检查是否已有同类命令正在执行 */
    if (remote_ctrl_is_executing_same(&req)) {
        TLY_LOG_W("Same command is executing");
        return;
    }
    
    /* 入队 */
    if (remote_ctrl_cache_push(&req) != 0) {
        TLY_LOG_E("Cache full, command dropped");
        return;
    }
    
    TLY_LOG_I("Remote cmd cached: ctrl_cmd=0x%02X, lock_type=%u", req.ctrl_cmd, req.lock_type);
}
