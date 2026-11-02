
/* ======================== 头文件 ======================== */

#include "tsp_vin.h"
#include "tsp_log.h"
#include "tsp_common.h"
#include "tsp_remote_ctrl.h"
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "sr_param.h"
#include "sr_ipc.h"
#include "queue_msg.h"
#include "mcu_api.h"
#include "err_code.h"

/* ======================== 常量定义 ======================== */

/** @brief VIN相关的CAN ID列表 */
#define VIN_CAN_ID_COUNT    2
static const uint32_t VIN_CAN_IDS[VIN_CAN_ID_COUNT] = {
    0x18FEEC10,  /* VCU广播VIN报文 */
    0x18F117D1   /* 钥匙开关 */
};

/** @brief 比对线程轮询间隔(毫秒) */
#define VIN_COMPARE_POLL_MS         (1000u)

/** @brief 钥匙ON触发后最大执行时间(秒) */
#define VIN_COMPARE_TIMEOUT_S       (30u)

/** @brief can_id_2 钥匙位置字节 */
#define VIN_CAN2_KEY_POS_BYTE       0

#define VIN_KEY_POS_KEY_OFF         0x00
#define VIN_KEY_POS_KEY_ON          0x01

/** @brief can_id_1 VIN帧序号 (data[0]低3位) */
#define VIN_CAN1_SEQ_BYTE       0
#define VIN_CAN1_SEQ_FRAME1     1
#define VIN_CAN1_SEQ_FRAME2     2
#define VIN_CAN1_SEQ_FRAME3     3

/** @brief can_id_1 VIN数据起始偏移 */
#define VIN_CAN1_DATA_OFFSET    1

/* ======================== 上下文 ======================== */

/** @brief CAN ID1 三帧缓存 */
#define VIN_CAN1_FRAME_COUNT    3

typedef struct {
    uint8_t seq;       /* 帧序号 (1,2,3) */
    uint8_t dlc;       /* 数据长度 */
    uint8_t data[8];   /* CAN数据 */
} vin_frame_t;

typedef struct {
    bool         initialized;
    bool         compare_thread_run;
    pthread_t    compare_thread;

    /* CAN ID1 三帧缓存 */
    uint32_t     can_id_1;
    uint8_t      frame_batch_id;                     /* 当前批次号，每收Frame1递增 */
    vin_frame_t  frames_1[VIN_CAN1_FRAME_COUNT];     /* Frame1/2/3 */
    uint8_t      frame_mask_1;                       /* 已接收帧掩码 (bit0=Frame1, bit1=Frame2, bit2=Frame3) */

    /* CAN ID2 数据 */
    uint32_t     can_id_2;
    uint8_t      data_2[8];
    uint8_t      dlc_2;

    /* 同步 */
    pthread_mutex_t mutex;
    uint64_t       last_sample_ms;
} vin_ctx_t;

typedef struct {
    uint32_t canId;
    uint8_t  channel;
    uint8_t  dlc;
    uint8_t  canData[8];
} vin_can_frame_t;

static vin_ctx_t g_ctx = {0};

/* ======================== 辅助函数 ======================== */

/**
 * @brief 检查CAN ID是否为VIN相关ID
 */
bool vin_is_vin_can_id(uint32_t can_id)
{
    for (uint8_t i = 0; i < VIN_CAN_ID_COUNT; i++) {
        if (can_id == VIN_CAN_IDS[i]) {
            return true;
        }
    }
    return false;
}

/**
 * @brief 检查CAN VIN是否全为FF（空数据）
 */
static inline bool vin_is_all_ff(const char *vin, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        if ((uint8_t)vin[i] != 0xFF) {
            return false;
        }
    }
    return true;
}

/**
 * @brief 检查VIN是否全为0（空数据）
 */
bool vin_is_all_zero(const char *vin, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        if (vin[i] != '0') {
            return false;
        }
    }
    return true;
}

/**
 * @brief 解析CAN帧缓存为VIN字符串
 * @param out_vin  输出的VIN字符串缓冲区（至少17字节）
 * @return true 解析成功, false 三帧未收齐或数据无效
 *
 * @details 数据格式:
 *   Frame1: data[1]=VIN[0], data[2]=VIN[1], ..., data[7]=VIN[6]
 *   Frame2: data[1]=VIN[7], data[2]=VIN[8], ..., data[7]=VIN[13]
 *   Frame3: data[1]=VIN[14], data[2]=VIN[15], data[3]=VIN[16]
 */
static bool vin_parse_can_vin(char *out_vin)
{
    if (out_vin == NULL) {
        return false;
    }

    /* 检查三帧是否都收到 */
    const uint8_t expected_mask = (1 << 0) | (1 << 1) | (1 << 2);  /* 0x07 */
    if ((g_ctx.frame_mask_1 & expected_mask) != expected_mask) {
        return false;
    }

    /* 从三帧组合VIN */
    const vin_frame_t *f1 = &g_ctx.frames_1[0];
    const vin_frame_t *f2 = &g_ctx.frames_1[1];
    const vin_frame_t *f3 = &g_ctx.frames_1[2];

    /* 检查数据是否全为0 */
    bool data_not_all_zero = false;
    for (uint8_t i = 0; i < f1->dlc; i++) {
        if (f1->data[i] != 0) { data_not_all_zero = true; break; }
    }
    if (!data_not_all_zero) {
        for (uint8_t i = 0; i < f2->dlc; i++) {
            if (f2->data[i] != 0) { data_not_all_zero = true; break; }
        }
    }
    if (!data_not_all_zero) {
        for (uint8_t i = 0; i < f3->dlc; i++) {
            if (f3->data[i] != 0) { data_not_all_zero = true; break; }
        }
    }
    if (!data_not_all_zero) {
        TLY_LOG_W("vin_parse_can_vin: all frames data is zero");
        return false;
    }

    /* 清零输出缓冲区 */
    memset(out_vin, 0, VIN_CAN1_VIN_LENGTH + 1);

    /* Frame1: VIN[0]~VIN[6] <- data[1]~data[7] */
    for (uint8_t i = 0; i < 7; i++) {
        if (i + 1 < f1->dlc) {
            out_vin[i] = f1->data[VIN_CAN1_DATA_OFFSET + i];
        }
    }

    /* Frame2: VIN[7]~VIN[13] <- data[1]~data[7] */
    for (uint8_t i = 0; i < 7; i++) {
        if (i + 1 < f2->dlc) {
            out_vin[7 + i] = f2->data[VIN_CAN1_DATA_OFFSET + i];
        }
    }

    /* Frame3: VIN[14]~VIN[16] <- data[1]~data[3] */
    for (uint8_t i = 0; i < 3; i++) {
        if (i + 1 < f3->dlc) {
            out_vin[14 + i] = f3->data[VIN_CAN1_DATA_OFFSET + i];
        }
    }

    return true;
}

/**
 * @brief 更新实时数据
 */
void vin_update_frame(uint32_t can_id, uint8_t channel,
                            const uint8_t *data, uint8_t dlc)
{
    (void)channel;

    if (can_id == VIN_CAN_IDS[0]) {
        if (data == NULL || dlc <= VIN_CAN1_SEQ_BYTE) {
            return;
        }
        uint8_t seq = data[VIN_CAN1_SEQ_BYTE] & 0x07;
        if (seq < VIN_CAN1_SEQ_FRAME1 || seq > VIN_CAN1_SEQ_FRAME3) {
            return;
        }

        uint8_t idx = seq - 1;
        uint8_t store_dlc = (dlc > 8) ? 8 : dlc;

        pthread_mutex_lock(&g_ctx.mutex);

        /* Frame1: 批次号递增，清空缓存，重置mask */
        if (seq == VIN_CAN1_SEQ_FRAME1) {
            g_ctx.frame_batch_id++;
            memset(g_ctx.frames_1, 0, sizeof(g_ctx.frames_1));
            g_ctx.frame_mask_1 = 0;
        }

        g_ctx.can_id_1 = can_id;
        g_ctx.frames_1[idx].seq = seq;
        g_ctx.frames_1[idx].dlc = store_dlc;
        memcpy(g_ctx.frames_1[idx].data, data, store_dlc);
        g_ctx.frame_mask_1 |= (1 << idx);

        pthread_mutex_unlock(&g_ctx.mutex);
    } else if (can_id == VIN_CAN_IDS[1]) {
        uint8_t store_dlc = (dlc > 8) ? 8 : dlc;

        pthread_mutex_lock(&g_ctx.mutex);
        g_ctx.can_id_2 = can_id;
        g_ctx.dlc_2 = store_dlc;
        if (data != NULL) {
            memcpy(g_ctx.data_2, data, store_dlc);
        }
        pthread_mutex_unlock(&g_ctx.mutex);
    }
}

/**
 * @brief VIN比对线程
 */
static void *vin_compare_thread(void *arg)
{
    tly_app_t *app = (tly_app_t *)arg;
    TLY_LOG_I("vin_compare_thread: compare thread started");
    uint8_t key_pos = 0;
    bool key_on_triggered = false;  /* 标志位，false表示未触发，true表示已触发 */
    uint32_t elapsed_seconds = 0;   /* 触发后已流逝的秒数 */
    bool can_vin_valid = false;
    char can_vin[VIN_CAN1_VIN_LENGTH + 1] = {0};
    tly_remote_ctrl_req_t req = {.ctrl_cmd = TLY_CTRL_BATTERY_HEAT};
    tly_process_result_t result = {.code = TLY_REMOTE_CTRL_RESULT_SUCCESS, .fail_reason = 0};
    static uint8_t prev_key_pos = VIN_KEY_POS_KEY_OFF;  /* 记录上一次的钥匙位置状态 */

    while (g_ctx.compare_thread_run) {
        /* 持锁拷贝CAN数据 */
        pthread_mutex_lock(&g_ctx.mutex);
        if (g_ctx.dlc_2 > VIN_CAN2_KEY_POS_BYTE) {
            key_pos = g_ctx.data_2[VIN_CAN2_KEY_POS_BYTE] & 0x07;
        }
        can_vin_valid = vin_parse_can_vin(can_vin);
        pthread_mutex_unlock(&g_ctx.mutex);

        /* 等待钥匙ON (0x01)，首次触发后不再受钥匙状态影响 */
        if (!key_on_triggered && key_pos == VIN_KEY_POS_KEY_ON) {
            key_on_triggered = true;
            elapsed_seconds = 0;
            TLY_LOG_I("vin_compare_thread: key ON detected, start compare");
        }

        if (key_pos == VIN_KEY_POS_KEY_OFF) {
            if (key_on_triggered)
                TLY_LOG_I("vin_compare_thread: key OFF detected, stop compare");
            key_on_triggered = false;
            elapsed_seconds = 0;

            if (app->battery_heat_alarm_on == 1 && tly_tcp_is_connected(app->tcp)) {
                TLY_LOG_I("vin_compare_thread: battery heat alarm on, send remote result to IOT");
                remote_ctrl_report_result(&req, &result);
                app->battery_heat_alarm_on = 2;

                tly_app_set_battery_heat_alarm(app);
            }
        }

        /* 检测钥匙状态转换 */
        if (app->battery_heat_alarm_on == 2) {
            if (prev_key_pos != key_pos) {
                if (prev_key_pos == VIN_KEY_POS_KEY_OFF && key_pos == VIN_KEY_POS_KEY_ON) {
                    /* 钥匙从OFF切换到ON，删除电池加热闹钟 */
                    TLY_LOG_I("vin_compare_thread: key OFF -> ON, delete battery heat alarm");
                    tly_app_delete_battery_heat_alarm(app);
                } else if (prev_key_pos == VIN_KEY_POS_KEY_ON && key_pos == VIN_KEY_POS_KEY_OFF) {
                    /* 钥匙从ON切换到OFF，重新设置电池加热闹钟 */
                    TLY_LOG_I("vin_compare_thread: key ON -> OFF, set battery heat alarm");
                    tly_app_set_battery_heat_alarm(app);
                }
                prev_key_pos = key_pos;
            }
        }

        /* 钥匙ON触发后，持续执行比对逻辑，超时30秒后停止 */
        if (key_on_triggered) {
            if (elapsed_seconds <= VIN_COMPARE_TIMEOUT_S) {
                TLY_LOG_W("vin_parse_can_vin: mask=0x%02X", g_ctx.frame_mask_1);
                if (can_vin_valid) {
                    /* 从数据库读取TBOX VIN */
                    char db_vin[VIN_CAN1_VIN_LENGTH + 1] = {0};
                    ParamInfo_t param = {0};
                    param.id = E_ParamId_VIN;
                    if (GetParam(&param) == RET_OK) {
                        memcpy(db_vin, param.value, VIN_CAN1_VIN_LENGTH);
                    } else {
                        TLY_LOG_E("GetParam VIN failed");
                    }

                    bool can_vin_empty = vin_is_all_ff(can_vin, VIN_CAN1_VIN_LENGTH);
                    bool tbox_vin_empty = vin_is_all_zero(db_vin, VIN_CAN1_VIN_LENGTH);

                    if (!can_vin_empty) {
                        TLY_LOG_I("vin_compare_thread: can_vin is valid, can_vin=%s", can_vin);
                    }

                    /* 情况1: TBOX无VIN VCU无VIN */
                    if (tbox_vin_empty && can_vin_empty) {
                        TLY_LOG_W("vin_compare_thread: TBOX and VCU vin all empty, request from server");
                        tly_app_get_vin(app);
                    }

                    /* 情况2: TBOX有VIN VCU无VIN */
                    if (!tbox_vin_empty && can_vin_empty) {
                        TLY_LOG_W("vin_compare_thread: VCU vin is empty but TBOX vin is not empty");

                        McuMesg_t McuMsg = {0};
                        McuMsg.type = SYS_QUE_TYPE_MCU_SYNC_VIN_STATUS;
                        McuMsg.payloadLen = 2;
                        uint8_t payLoad[2] = {0};
                        payLoad[0] = 0x00;  /* 非强制锁存 */
                        payLoad[1] = 0x01;  /* VIN不一致 */
                        McuMsg.payload = payLoad;
                        McuSrvSendMesg(&McuMsg);
                    }

                    /* 情况3: TBOX无VIN VCU有VIN */
                    if (tbox_vin_empty && !can_vin_empty) {
                        TLY_LOG_I("vin_compare_thread: TBOX vin is empty but VCU vin is not empty, save VCU vin to config");
                        if (strncmp(db_vin, can_vin, VIN_CAN1_VIN_LENGTH) != 0) {
                            UpdateParam(E_ParamId_VIN, (uint8_t *)can_vin, VIN_CAN1_VIN_LENGTH);
                        }

                        McuMesg_t McuMsg = {0};
                        McuMsg.type = SYS_QUE_TYPE_MCU_SYNC_VIN_STATUS;
                        McuMsg.payloadLen = 2;
                        uint8_t payLoad[2] = {0};
                        payLoad[0] = 0x00;  /* 非强制锁存 */
                        payLoad[1] = 0x01;  /* VIN不一致 */
                        McuMsg.payload = payLoad;
                        if (RET_ERR == McuSrvSendMesg(&McuMsg))
                        {
                            TLY_LOG_E(" %s:McuSrvSendMesg faild!", __func__);
                        }
                    }

                    /* 情况4: TBOX有VIN VCU有VIN */
                    if (!tbox_vin_empty && !can_vin_empty) {
                        if (strncmp(db_vin, can_vin, VIN_CAN1_VIN_LENGTH) == 0) {
                            TLY_LOG_I("vin_compare_thread: compare is same, db=%s can=%s", db_vin, can_vin);

                            McuMesg_t McuMsg = {0};
                            McuMsg.type = SYS_QUE_TYPE_MCU_SYNC_VIN_STATUS;
                            McuMsg.payloadLen = 2;
                            uint8_t payLoad[2] = {0};
                            payLoad[0] = 0x00;  /* 非强制锁存 */
                            payLoad[1] = 0x00;  /* VIN一致 */
                            McuMsg.payload = payLoad;
                            if (RET_ERR == McuSrvSendMesg(&McuMsg))
                            {
                                TLY_LOG_E(" %s:McuSrvSendMesg faild!", __func__);
                            }
                        } else {
                            TLY_LOG_W("vin_compare_thread: compare is different, db=%s can=%s", db_vin, can_vin);
                            tly_app_get_vin(app);
                        }
                    }
                }
            }
        }

        usleep(VIN_COMPARE_POLL_MS * 1000);
        if (key_on_triggered) {
            elapsed_seconds += VIN_COMPARE_POLL_MS / 1000;
            if (elapsed_seconds >= VIN_COMPARE_TIMEOUT_S) {
                elapsed_seconds = VIN_COMPARE_TIMEOUT_S + 1;
            }
        }
    }

    TLY_LOG_I("VIN: compare thread exited");
    return NULL;
}


/* ======================== 生命周期 ======================== */
int tsp_vin_data_init(tly_app_t *app)
{
    TLY_LOG_I("VIN_DATA: init start, with_app=%s", app ? "yes" : "no");
    
    if (g_ctx.initialized) {
        TLY_LOG_W("VIN_DATA: already initialized");
        return 0;
    }

    /* 清零上下文 */
    memset(&g_ctx, 0, sizeof(g_ctx));

    /* 初始化互斥锁 */
    pthread_mutex_init(&g_ctx.mutex, NULL);

    /* 启动VIN码绑定线程 */
    if (app != NULL) {
        g_ctx.compare_thread_run = true;
        int ret = pthread_create(&g_ctx.compare_thread, NULL, vin_compare_thread, app);
        if (ret != 0) {
            TLY_LOG_E("VIN_DATA: failed to create compare thread, ret=%d", ret);
            g_ctx.compare_thread_run = false;
        } else {
            TLY_LOG_I("VIN_DATA: compare thread started");
        }
    }

    g_ctx.last_sample_ms = tly_monotonic_time_ms();
    g_ctx.initialized = true;
    TLY_LOG_I("VIN_DATA: init success, filter_ids={0x%X,0x%X}",
              VIN_CAN_IDS[0], VIN_CAN_IDS[1]);
    return 0;
}

void tsp_vin_data_deinit(void)
{
    if (!g_ctx.initialized) {
        return;
    }

    /* 停止比对线程 */
    if (g_ctx.compare_thread_run) {
        g_ctx.compare_thread_run = false;
        pthread_join(g_ctx.compare_thread, NULL);
        TLY_LOG_I("VIN_DATA: compare thread stopped");
    }

    g_ctx.initialized = false;
    pthread_mutex_destroy(&g_ctx.mutex);
    memset(&g_ctx, 0, sizeof(g_ctx));
    TLY_LOG_I("VIN_DATA: deinit success");
}
