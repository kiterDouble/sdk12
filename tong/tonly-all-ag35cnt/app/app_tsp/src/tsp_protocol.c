/**
 * tsp_protocol.c - 二进制协议编解码实现
 */

#include "tsp_protocol.h"
#include "tsp_log.h"
#include "tsp_app.h"
#include "tsp_tcp.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* ======================== BCC异或校验 ======================== */

uint8_t tly_bcc_calc(const uint8_t *data, uint32_t len)
{
    uint8_t checksum = 0;
    for (uint32_t i = 0; i < len; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

/* ======================== 时间编解码 ======================== */

int tly_time_encode(const struct tm *tm, uint8_t *buf)
{
    if (!tm || !buf) return TLY_ERR_PARAM;

    buf[0] = (uint8_t)(tm->tm_year % 100);  /* 年: 0~99 */
    buf[1] = (uint8_t)(tm->tm_mon + 1);     /* 月: 1~12 */
    buf[2] = (uint8_t)(tm->tm_mday);        /* 日: 1~31 */
    buf[3] = (uint8_t)(tm->tm_hour);        /* 时: 0~23 */
    buf[4] = (uint8_t)(tm->tm_min);         /* 分: 0~59 */
    buf[5] = (uint8_t)(tm->tm_sec);         /* 秒: 0~59 */
    return 0;
}

int tly_time_decode(const uint8_t *buf, struct tm *tm)
{
    if (!buf || !tm) return TLY_ERR_PARAM;

    memset(tm, 0, sizeof(*tm));
    tm->tm_year = (buf[0] < 100) ? buf[0] + 2000 : buf[0]; /* 两位年 → 四位 */
    tm->tm_mon  = buf[1] - 1;   /* 1~12 → 0~11 */
    tm->tm_mday = buf[2];
    tm->tm_hour = buf[3];
    tm->tm_min  = buf[4];
    tm->tm_sec  = buf[5];
    tm->tm_isdst = -1;
    return 0;
}

int tly_time_now(uint8_t *buf)
{
    if (!buf) return TLY_ERR_PARAM;

    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    return tly_time_encode(&tm_now, buf);
}

/* ======================== 通用数据包编解码 ======================== */

int tly_packet_encode(const tly_packet_t *pkt,
                      uint8_t *out_buf, uint32_t *out_len)
{
    if (!pkt || !out_buf || !out_len) return TLY_ERR_PARAM;
    if (pkt->data_len > TLY_PKT_MAX_DATA) return TLY_ERR_PARAM;

    uint8_t *p = out_buf;

    /* 起始符 "##" */
    p[0] = 0x23;
    p[1] = 0x23;
    p += 2;

    /* 命令单元(1B) */
    *p++ = pkt->cmd_id;

    /* VIN(17B) */
    memset(p, 0, TLY_PKT_VIN_LEN);
    size_t vin_len = strlen(pkt->vin);
    if (vin_len > TLY_PKT_VIN_LEN) vin_len = TLY_PKT_VIN_LEN;
    memcpy(p, pkt->vin, vin_len);
    p += TLY_PKT_VIN_LEN;

    /* 软件版本号(1B) */
    *p++ = pkt->sw_version;

    /* 加密方式(1B) */
    *p++ = pkt->encrypt_mode;

    /* 数据单元长度(2B, 大端) */
    tly_write_be16(p, pkt->data_len);
    p += 2;

    /* 数据单元(NB) */
    if (pkt->data_len > 0 && pkt->data) {
        memcpy(p, pkt->data, pkt->data_len);
        p += pkt->data_len;
    }

    /* BCC校验(1B): 从命令单元(偏移2)到数据单元最后一字节 */
    uint32_t bcc_len = (uint32_t)(p - out_buf) - TLY_PKT_START_LEN;
    *p = tly_bcc_calc(out_buf + TLY_PKT_START_LEN, bcc_len);
    p++;

    *out_len = (uint32_t)(p - out_buf);
    return 0;
}

int tly_packet_decode(const uint8_t *in_buf, uint32_t in_len,
                      tly_packet_t *pkt)
{
    if (!in_buf || !pkt || in_len < TLY_PKT_HEADER_LEN + 1)
        return TLY_ERR_PARAM;

    memset(pkt, 0, sizeof(*pkt));

    const uint8_t *p = in_buf;

    /* 校验起始符 */
    if (p[0] != 0x23 || p[1] != 0x23) {
        TLY_LOG_E("Invalid packet start: 0x%02X 0x%02X", p[0], p[1]);
        return TLY_ERR_PROTOCOL;
    }
    p += 2;

    /* 命令单元 */
    pkt->cmd_id = *p++;

    /* VIN(17B) */
    memcpy(pkt->vin, p, TLY_PKT_VIN_LEN);
    pkt->vin[TLY_PKT_VIN_LEN] = '\0';
    p += TLY_PKT_VIN_LEN;

    /* 版本号 */
    pkt->sw_version = *p++;

    /* 加密方式 */
    pkt->encrypt_mode = *p++;

    /* 数据单元长度 */
    pkt->data_len = tly_read_be16(p);
    p += 2;

    /* 检查总长度: 包头 + 数据 + 校验码 */
    if (in_len < (uint32_t)(TLY_PKT_HEADER_LEN + pkt->data_len + 1)) {
        TLY_LOG_E("Packet too short: expected %u, got %u",
                  TLY_PKT_HEADER_LEN + pkt->data_len + 1, in_len);
        return TLY_ERR_PROTOCOL;
    }

    /* 数据单元 */
    if (pkt->data_len > 0) {
        pkt->data = (uint8_t *)malloc(pkt->data_len);
        if (!pkt->data) return TLY_ERR_MEMORY;
        memcpy(pkt->data, p, pkt->data_len);
        p += pkt->data_len;
    }

    /* BCC校验 */
    uint32_t bcc_len = (uint32_t)(p - in_buf) - TLY_PKT_START_LEN;
    uint8_t expected_bcc = tly_bcc_calc(in_buf + TLY_PKT_START_LEN, bcc_len);
    if (*p != expected_bcc) {
        TLY_LOG_E("BCC mismatch: expected 0x%02X, got 0x%02X",
                  expected_bcc, *p);
        tly_packet_free(pkt);
        return TLY_ERR_CHECKSUM;
    }

    return 0;
}

void tly_packet_free(tly_packet_t *pkt)
{
    if (pkt) {
        free(pkt->data);
        pkt->data = NULL;
        pkt->data_len = 0;
    }
}

/* ======================== 车辆登入 (0x01) ======================== */

int tly_encode_vehicle_login(const tly_vehicle_login_t *login,
                             uint8_t *buf, uint32_t *len)
{
    if (!login || !buf || !len) return TLY_ERR_PARAM;

    /* 6字节时间 + 2字节流水号 + 20字节ICCID = 28字节 */
    const uint32_t total = 6 + 2 + TLY_ICCID_LEN;
    uint8_t *p = buf;

    memcpy(p, login->time, 6);
    p += 6;

    tly_write_be16(p, login->seq);
    p += 2;

    memset(p, 0, TLY_ICCID_LEN);
    size_t iccid_len = strlen(login->iccid);
    if (iccid_len > TLY_ICCID_LEN) iccid_len = TLY_ICCID_LEN;
    memcpy(p, login->iccid, iccid_len);
    p += TLY_ICCID_LEN;

    *len = total;
    return 0;
}

int tly_decode_vehicle_login(const uint8_t *buf, uint32_t len,
                             tly_vehicle_login_t *login)
{
    if (!buf || !login) return TLY_ERR_PARAM;
    if (len < 6 + 2 + TLY_ICCID_LEN) return TLY_ERR_PROTOCOL;

    const uint8_t *p = buf;

    memcpy(login->time, p, 6);
    p += 6;

    login->seq = tly_read_be16(p);
    p += 2;

    memcpy(login->iccid, p, TLY_ICCID_LEN);
    login->iccid[TLY_ICCID_LEN] = '\0';

    return 0;
}

/* ======================== 车辆登出 (0x04) ======================== */

int tly_encode_vehicle_logout(const tly_vehicle_logout_t *logout,
                              uint8_t *buf, uint32_t *len)
{
    if (!logout || !buf || !len) return TLY_ERR_PARAM;

    memcpy(buf, logout->time, 6);
    tly_write_be16(buf + 6, logout->seq);
    *len = 8;
    return 0;
}

int tly_decode_vehicle_logout(const uint8_t *buf, uint32_t len,
                              tly_vehicle_logout_t *logout)
{
    if (!buf || !logout) return TLY_ERR_PARAM;
    if (len < 8) return TLY_ERR_PROTOCOL;

    memcpy(logout->time, buf, 6);
    logout->seq = tly_read_be16(buf + 6);
    return 0;
}

/* ======================== 版本号上传 (0x78) ======================== */

#define TLY_VERSION_UPLOAD_LEN  18  /* time(6)+2+4+2+2+2 */

int tly_encode_version_upload(const tly_version_upload_t *ver,
                              uint8_t *buf, uint32_t *len)
{
    if (!ver || !buf || !len) return TLY_ERR_PARAM;

    memcpy(buf, ver->time, 6);
    tly_write_be16(buf + 6, ver->hw_version);
    tly_write_be32(buf + 8, ver->main_mcu_version);
    tly_write_be16(buf + 12, ver->sub_mcu_version);
    tly_write_be16(buf + 14, ver->non_stage4_version);
    tly_write_be16(buf + 16, ver->version_seq);

    *len = TLY_VERSION_UPLOAD_LEN;
    return 0;
}

int tly_decode_version_upload(const uint8_t *buf, uint32_t len,
                              tly_version_upload_t *ver)
{
    if (!buf || !ver) return TLY_ERR_PARAM;
    if (len < TLY_VERSION_UPLOAD_LEN) return TLY_ERR_PROTOCOL;

    memcpy(ver->time, buf, 6);
    ver->hw_version         = tly_read_be16(buf + 6);
    ver->main_mcu_version   = tly_read_be32(buf + 8);
    ver->sub_mcu_version    = tly_read_be16(buf + 12);
    ver->non_stage4_version = tly_read_be16(buf + 14);
    ver->version_seq        = tly_read_be16(buf + 16);

    return 0;
}

/* ======================== VCU版本号上传 (0x73) ======================== */

#define TLY_VCU_VERSION_LEN  12  /* time(6) + version(6) */

int tly_encode_vcu_version(const tly_vcu_version_t *ver,
                           uint8_t *buf, uint32_t *len)
{
    if (!ver || !buf || !len) return TLY_ERR_PARAM;

    memcpy(buf, ver->time, 6);
    memcpy(buf + 6, ver->version, 6);

    *len = TLY_VCU_VERSION_LEN;
    return 0;
}

int tly_decode_vcu_version(const uint8_t *buf, uint32_t len,
                           tly_vcu_version_t *ver)
{
    if (!buf || !ver) return TLY_ERR_PARAM;
    if (len < TLY_VCU_VERSION_LEN) return TLY_ERR_PROTOCOL;

    memcpy(ver->time, buf, 6);
    memcpy(ver->version, buf + 6, 6);

    return 0;
}

void tly_vcu_version_dump(const tly_vcu_version_t *ver)
{
    if (!ver) return;
    TLY_LOG_I("VCU version: time=20%02u-%02u-%02u %02u:%02u:%02u, version=%02X%02X%02X%02X%02X%02X",
              ver->time[0], ver->time[1], ver->time[2],
              ver->time[3], ver->time[4], ver->time[5],
              ver->version[0], ver->version[1], ver->version[2],
              ver->version[3], ver->version[4], ver->version[5]);
}

/* ======================== TCU版本号上传 (0x74) ======================== */

int tly_encode_tcu_version(const tly_tcu_version_t *ver,
                           uint8_t *buf, uint32_t *len)
{
    if (!ver || !buf || !len) return TLY_ERR_PARAM;
    if (ver->version_len == 0 || ver->version_len > TLY_TCU_VERSION_MAX_LEN) return TLY_ERR_PARAM;

    memcpy(buf, ver->version, ver->version_len);

    *len = ver->version_len;
    return 0;
}

int tly_decode_tcu_version(const uint8_t *buf, uint32_t len,
                           tly_tcu_version_t *ver)
{
    if (!buf || !ver) return TLY_ERR_PARAM;
    if (len == 0 || len > TLY_TCU_VERSION_MAX_LEN) return TLY_ERR_PROTOCOL;

    ver->version_len = len;
    memcpy(ver->version, buf, len);

    return 0;
}

void tly_tcu_version_dump(const tly_tcu_version_t *ver)
{
    if (!ver) return;
    TLY_LOG_I("TCU version: version_len=%u", ver->version_len);
    if (ver->version_len > 0) {
        TLY_LOG_Dump("TCU version data:", ver->version, ver->version_len);
    }
}

/* ======================== MCU版本号上传 (0x75) ======================== */
/* 数据单元格式: 数量(1B) + [mcu_type(1B) + 版本内容(10B)] × 数量 */

int tly_encode_mcu_version(const tly_mcu_version_t *ver,
                           uint8_t *buf, uint32_t *len)
{
    if (!ver || !buf || !len) return TLY_ERR_PARAM;
    if (ver->count == 0 || ver->count > TLY_MCU_VER_MAX_COUNT) return TLY_ERR_PARAM;

    uint8_t *p = buf;

    /* 数量(1B) */
    *p++ = ver->count;

    /* 每个MCU项: mcu_type(1B) + 版本内容(10B) */
    for (uint8_t i = 0; i < ver->count; i++) {
        *p++ = ver->items[i].mcu_type;
        memcpy(p, ver->items[i].version, TLY_MCU_VER_CONTENT_LEN);
        p += TLY_MCU_VER_CONTENT_LEN;
    }

    *len = (uint32_t)(p - buf);
    return 0;
}

int tly_decode_mcu_version(const uint8_t *buf, uint32_t len,
                           tly_mcu_version_t *ver)
{
    if (!buf || !ver) return TLY_ERR_PARAM;
    if (len < 1) return TLY_ERR_PROTOCOL;

    uint8_t count = buf[0];
    if (count > TLY_MCU_VER_MAX_COUNT) return TLY_ERR_PROTOCOL;
    if (len < (uint32_t)(1 + count * TLY_MCU_VER_ITEM_LEN)) return TLY_ERR_PROTOCOL;

    ver->count = count;
    const uint8_t *p = buf + 1;
    for (uint8_t i = 0; i < count; i++) {
        ver->items[i].mcu_type = *p++;
        memcpy(ver->items[i].version, p, TLY_MCU_VER_CONTENT_LEN);
        p += TLY_MCU_VER_CONTENT_LEN;
    }

    return 0;
}

void tly_mcu_version_dump(const tly_mcu_version_t *ver)
{
    if (!ver) return;
    TLY_LOG_I("MCU version: count=%u", ver->count);
    for (uint8_t idx = 0; idx < ver->count; idx++) {
        TLY_LOG_I("  mcu_type=0x%02X", ver->items[idx].mcu_type);
        TLY_LOG_Dump("  version:", ver->items[idx].version, TLY_MCU_VER_CONTENT_LEN);
    }
}

/* ======================== 位置信息 (0xAC) ======================== */

int tly_encode_location(const tly_location_t *loc,
                        uint8_t *buf, uint32_t *len)
{
    if (!loc || !buf || !len) return TLY_ERR_PARAM;

    uint8_t *p = buf;

    *p++ = loc->status;

    tly_write_be32(p, loc->longitude);  p += 4;
    tly_write_be32(p, loc->latitude);   p += 4;
    tly_write_be16(p, loc->altitude);   p += 2;
    *p++ = loc->gps_speed;
    tly_write_be16(p, loc->heading);    p += 2;
    tly_write_be16(p, loc->accel_x);    p += 2;
    tly_write_be16(p, loc->accel_y);    p += 2;
    tly_write_be16(p, loc->accel_z);    p += 2;
    *p++ = loc->dev_status;

    *len = (uint32_t)(p - buf);
    return 0;
}

int tly_decode_location(const uint8_t *buf, uint32_t len,
                        tly_location_t *loc)
{
    if (!buf || !loc) return TLY_ERR_PARAM;
    if (len < 23) return TLY_ERR_PROTOCOL;  /* 1+4+4+2+1+2+2+2+2+2+1=23 */

    const uint8_t *p = buf;

    loc->status     = *p++;
    loc->longitude  = tly_read_be32(p); p += 4;
    loc->latitude   = tly_read_be32(p); p += 4;
    loc->altitude   = tly_read_be16(p); p += 2;
    loc->gps_speed  = *p++;
    loc->heading    = tly_read_be16(p); p += 2;
    loc->accel_x    = tly_read_be16(p); p += 2;
    loc->accel_y    = tly_read_be16(p); p += 2;
    loc->accel_z    = tly_read_be16(p); p += 2;
    loc->dev_status = *p++;

    return 0;
}

/* ======================== 实时信息上报 (0x02) ======================== */

int tly_encode_realtime_report(const tly_realtime_report_t *rpt,
                               uint8_t *buf, uint32_t *len)
{
    if (!rpt || !buf || !len) return TLY_ERR_PARAM;

    uint8_t *p = buf;

    /* 时间(6B) */
    memcpy(p, rpt->time, 6);
    p += 6;

    /* 信息项: 标识(1B) + 内容(NB) */
    for (uint32_t i = 0; i < rpt->item_count; i++) {
        const tly_info_item_t *item = &rpt->items[i];
        *p++ = item->info_id;
        if (item->info_len > 0 && item->info_data) {
            memcpy(p, item->info_data, item->info_len);
            p += item->info_len;
        }
    }

    *len = (uint32_t)(p - buf);
    return 0;
}

int tly_decode_realtime_report(const uint8_t *buf, uint32_t len,
                               tly_realtime_report_t *rpt)
{
    if (!buf || !rpt || len < 7) return TLY_ERR_PARAM;

    memset(rpt, 0, sizeof(*rpt));

    const uint8_t *p = buf;
    const uint8_t *end = buf + len;

    /* 时间(6B) */
    memcpy(rpt->time, p, 6);
    p += 6;

    /* 预分配信息项数组 (按最大可能数量) */
    uint32_t max_items = (len - 6) / 2;  /* 每项至少2字节(1标识+1数据) */
    if (max_items == 0) return 0;

    rpt->items = (tly_info_item_t *)calloc(max_items, sizeof(tly_info_item_t));
    if (!rpt->items) return TLY_ERR_MEMORY;

    /* 逐个解析信息项 */
    while (p < end && rpt->item_count < max_items) {
        tly_info_item_t *item = &rpt->items[rpt->item_count];
        item->info_id = *p++;

        /* 根据信息标识确定数据长度 */
        uint32_t data_len = 0;
        switch (item->info_id) {
        case TLY_INFO_LOCATION:
            data_len = 23;
            break;
        case TLY_INFO_TIRE_PRESSURE:
            data_len = 8;
            break;
        case TLY_INFO_LOCK_STATUS:
            data_len = 7;  /* 标记帧(1)+其它帧(1)+状态帧(1)+预留(4) */
            break;
        case TLY_INFO_ALARM:
            data_len = 0;  /* 变长, 需要特殊处理 */
            break;
        case TLY_INFO_FAULT_CODE:
            data_len = 21; /* ECU版本(17)+故障码(4) */
            break;
        case TLY_INFO_FAST:
        case TLY_INFO_SLOW:
        case TLY_INFO_EV_FAST:
        case TLY_INFO_EV_SLOW:
            /* 快慢包为变长CAN数据, 需要特殊处理 */
            data_len = 0;
            break;
        default:
            /* 未知信息标识, 剩余全部作为一个数据块 */
            data_len = (uint32_t)(end - p);
            break;
        }

        /* 变长数据: 如果无法确定长度, 将剩余数据全部读入 */
        if (data_len == 0 || (p + data_len) > end) {
            data_len = (uint32_t)(end - p);
        }

        if (data_len > 0) {
            item->info_data = (uint8_t *)malloc(data_len);
            if (!item->info_data) {
                tly_realtime_report_free(rpt);
                return TLY_ERR_MEMORY;
            }
            memcpy(item->info_data, p, data_len);
            item->info_len = data_len;
        }

        p += data_len;
        rpt->item_count++;
    }

    return 0;
}

void tly_realtime_report_free(tly_realtime_report_t *rpt)
{
    if (!rpt) return;

    if (rpt->items) {
        for (uint32_t i = 0; i < rpt->item_count; i++) {
            free(rpt->items[i].info_data);
        }
        free(rpt->items);
        rpt->items = NULL;
    }
    rpt->item_count = 0;
}

/* ======================== 拆除报警 (0x05) ======================== */

int tly_encode_remove_alarm(const tly_remove_alarm_t *alarm,
                            uint8_t *buf, uint32_t *len)
{
    if (!alarm || !buf || !len) return TLY_ERR_PARAM;

    uint8_t *p = buf;

    memcpy(p, alarm->time, 6);         p += 6;
    tly_write_be16(p, alarm->seq);     p += 2;
    *p++ = alarm->status;
    tly_write_be32(p, alarm->longitude); p += 4;
    tly_write_be32(p, alarm->latitude);  p += 4;

    *len = (uint32_t)(p - buf);
    return 0;
}

int tly_decode_remove_alarm(const uint8_t *buf, uint32_t len,
                            tly_remove_alarm_t *alarm)
{
    if (!buf || !alarm) return TLY_ERR_PARAM;
    if (len < 17) return TLY_ERR_PROTOCOL;  /* 6+2+1+4+4=17 */

    const uint8_t *p = buf;

    memcpy(alarm->time, p, 6);              p += 6;
    alarm->seq      = tly_read_be16(p);     p += 2;
    alarm->status   = *p++;
    alarm->longitude = tly_read_be32(p);    p += 4;
    alarm->latitude  = tly_read_be32(p);

    return 0;
}

/* ======================== CANID列表获取-带通道号 (0x7B) ======================== */

int tly_encode_canid_request(uint8_t vehicle_type,
                             uint8_t *buf, uint32_t *len)
{
    if (!buf || !len) return TLY_ERR_PARAM;
    buf[0] = vehicle_type;
    *len = 1;
    return 0;
}

int tly_decode_canid_response(const uint8_t *buf, uint32_t len,
                              tly_canid_list_resp_t *resp)
{
    if (!buf || !resp) return TLY_ERR_PARAM;
    memset(resp, 0, sizeof(*resp));

    const uint8_t *p = buf;
    const uint8_t *end = buf + len;

    /* 快包采集频次(1B) */
    if (p >= end) return TLY_ERR_PROTOCOL;
    resp->fast_freq = *p++;

    /* 快包CANID个数(1B) */
    if (p >= end) return TLY_ERR_PROTOCOL;
    resp->fast_count = *p++;
    if (resp->fast_count > TLY_CANID_MAX) return TLY_ERR_PROTOCOL;

    /* 快包CANID集: 每个条目 = CANID(4B,小端) + 通道号(1B) = 5字节 */
    for (int i = 0; i < resp->fast_count; i++) {
        if (p + 5 > end) return TLY_ERR_PROTOCOL;
        resp->fast_ids[i].can_id = tly_read_le32(p); p += 4;
        resp->fast_ids[i].channel = *p++;
    }

    /* 慢包采集频次(1B) */
    if (p >= end) return TLY_ERR_PROTOCOL;
    resp->slow_freq = *p++;

    /* 慢包CANID个数(1B) */
    if (p >= end) return TLY_ERR_PROTOCOL;
    resp->slow_count = *p++;
    if (resp->slow_count > TLY_CANID_MAX) return TLY_ERR_PROTOCOL;

    /* 慢包CANID集: 每个条目 = CANID(4B,小端) + 通道号(1B) = 5字节 */
    for (int i = 0; i < resp->slow_count; i++) {
        if (p + 5 > end) return TLY_ERR_PROTOCOL;
        resp->slow_ids[i].can_id = tly_read_le32(p); p += 4;
        resp->slow_ids[i].channel = *p++;
    }

    return 0;
}

int tly_decode_canid_response_no_ch(const uint8_t *buf, uint32_t len,
                                    tly_canid_list_resp_t *resp)
{
    if (!buf || !resp) return TLY_ERR_PARAM;
    memset(resp, 0, sizeof(*resp));

    const uint8_t *p = buf;
    const uint8_t *end = buf + len;

    resp->fast_freq = *p++;
    resp->fast_count = *p++;
    if (resp->fast_count > TLY_CANID_MAX) return TLY_ERR_PROTOCOL;

    for (int i = 0; i < resp->fast_count; i++) {
        if (p + 4 > end) return TLY_ERR_PROTOCOL;
        resp->fast_ids[i].can_id = tly_read_le32(p); p += 4;
        resp->fast_ids[i].channel = 0;
    }

    if (p >= end) return TLY_ERR_PROTOCOL;
    resp->slow_freq = *p++;

    if (p >= end) return TLY_ERR_PROTOCOL;
    resp->slow_count = *p++;
    if (resp->slow_count > TLY_CANID_MAX) return TLY_ERR_PROTOCOL;

    for (int i = 0; i < resp->slow_count; i++) {
        if (p + 4 > end) return TLY_ERR_PROTOCOL;
        resp->slow_ids[i].can_id = tly_read_le32(p); p += 4;
        resp->slow_ids[i].channel = 0;
    }

    return 0;
}

void tly_canid_list_resp_dump(const tly_canid_list_resp_t *resp)
{
    if (!resp) return;

    TLY_LOG_I("CANID List Response:");
    TLY_LOG_I("  Fast: freq=%ds, count=%u", resp->fast_freq, resp->fast_count);
    for (int i = 0; i < resp->fast_count; i++) {
        TLY_LOG_I("    [%d] CANID=0x%08X, channel=%u",
                  i, resp->fast_ids[i].can_id, resp->fast_ids[i].channel);
    }

    TLY_LOG_I("  Slow: freq=%ds, count=%u", resp->slow_freq, resp->slow_count);
    for (int i = 0; i < resp->slow_count; i++) {
        TLY_LOG_I("    [%d] CANID=0x%08X, channel=%u",
                  i, resp->slow_ids[i].can_id, resp->slow_ids[i].channel);
    }
}

/* ======================== 电车快包 (信息标识 0xA0) ======================== */

int tly_decode_ev_fast_packet(const uint8_t *buf, uint32_t len,
                              tly_ev_fast_packet_t *pkt)
{
    if (!buf || !pkt || len < 1) return TLY_ERR_PARAM;
    memset(pkt, 0, sizeof(*pkt));

    const uint8_t *p = buf;
    const uint8_t *end = buf + len;

    pkt->frame_count = *p++;
    if (pkt->frame_count > TLY_EV_FRAME_MAX) return TLY_ERR_PROTOCOL;

    for (int i = 0; i < pkt->frame_count; i++) {
        if (p + 4 > end) return TLY_ERR_PROTOCOL;

        /* CANID(4B, 小端) */
        pkt->frames[i].can_id = tly_read_le32(p);
        p += 4;

        /* 报文状态(1B): bit0-3=报文长度, bit4-7=通道号 */
        if (p >= end) return TLY_ERR_PROTOCOL;
        uint8_t status = *p++;
        pkt->frames[i].msg_len = status & 0x0F;
        pkt->frames[i].channel = (status >> 4) & 0x0F;

        /* 报文长度为0则无原始报文 */
        if (pkt->frames[i].msg_len == 0) continue;

        if (p + pkt->frames[i].msg_len > end) return TLY_ERR_PROTOCOL;
        memcpy(pkt->frames[i].raw, p, pkt->frames[i].msg_len);
        p += pkt->frames[i].msg_len;
    }

    return 0;
}

void tly_ev_fast_packet_dump(const tly_ev_fast_packet_t *pkt)
{
    if (!pkt) return;

    TLY_LOG_I("  EV Fast Packet: %u frames", pkt->frame_count);
    for (int i = 0; i < pkt->frame_count; i++) {
        const tly_ev_can_frame_t *f = &pkt->frames[i];

        char hex[64] = {0};
        if (f->msg_len > 0) {
            int pos = 0;
            for (int j = 0; j < (int)f->msg_len && pos < 60; j++) {
                pos += snprintf(hex + pos, (size_t)(64 - pos), "%02X ", f->raw[j]);
            }
        }

        TLY_LOG_I("    [%d] CANID=0x%08X, ch=%u, len=%u, data=[%s]",
                  i, f->can_id, f->channel, f->msg_len,
                  f->msg_len > 0 ? hex : "(empty)");
    }
}

/* ======================== 应答包封装 ======================== */

int tly_encode_response(uint8_t cmd_id, const char *vin,
                        uint8_t resp_flag, uint8_t sw_ver,
                        uint8_t encrypt,
                        const uint8_t *data, uint32_t data_len,
                        uint8_t *out_buf, uint32_t *out_len)
{
    if (!vin || !out_buf || !out_len) return TLY_ERR_PARAM;

    (void)cmd_id;

    /* 应答包: byte 2 = 应答标志(非命令标识), 数据单元 = 原始数据 */
    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));

    pkt.cmd_id       = resp_flag;
    strncpy(pkt.vin, vin, TLY_PKT_VIN_LEN);
    pkt.sw_version   = sw_ver;
    pkt.encrypt_mode = encrypt;
    pkt.data_len     = data_len;

    if (data_len > 0 && data) {
        pkt.data = (uint8_t *)malloc(data_len);
        if (!pkt.data) return TLY_ERR_MEMORY;
        memcpy(pkt.data, data, data_len);
    }

    int rc = tly_packet_encode(&pkt, out_buf, out_len);
    free(pkt.data);
    return rc;
}

/* ======================== 终端控制 - 子命令分发 ======================== */

static int is_lock_cmd(uint8_t cmd)
{
    return cmd == TLY_CTRL_LOCK_EV   || cmd == TLY_CTRL_LOCK_WEICHAI ||
           cmd == TLY_CTRL_LOCK_YUCHAI || cmd == TLY_CTRL_LOCK_XICHAI ||
           cmd == TLY_CTRL_LOCK_CUMMINS || cmd == TLY_CTRL_LOCK_MAN;
}

static int is_ota_cmd(uint8_t cmd)
{
    if (cmd == TLY_OTA_TBOX || cmd == TLY_OTA_TBOX_EXT || cmd == TLY_OTA_ECU_WEICHAI) return 1;
    if (cmd >= 0x50 && cmd <= 0x54) return 1;
    if (cmd == 0x5A || cmd == 0x5B || cmd == 0x5C || cmd == 0x5D ||
        cmd == 0x5E || cmd == 0x5F) return 1;
    if (cmd >= 0x60 && cmd <= 0x6E) return 1;
    if (cmd >= 0xA0 && cmd <= 0xAC) return 1;
    return 0;
}

int tly_ctrl_classify(const uint8_t *data, uint32_t data_len)
{
    if (!data || data_len < 7) return TLY_CTRL_TYPE_OTHER;
    uint8_t cmd = data[6];
    if (is_lock_cmd(cmd)) return TLY_CTRL_TYPE_LOCK;
    if (is_ota_cmd(cmd))  return TLY_CTRL_TYPE_OTA;
    if (cmd == TLY_CTRL_BATTERY_HEAT) return TLY_CTRL_TYPE_BATTERY_HEAT;
    if (cmd == TLY_CTRL_SD_CARD)      return TLY_CTRL_TYPE_SD_CARD;
    if (cmd == TLY_CTRL_VIN_UNBIND)   return TLY_CTRL_TYPE_VIN_UNBIND;
    if (cmd == TLY_CTRL_TIME_SET)     return TLY_CTRL_TYPE_TIME_SYNC;
    if (cmd == TLY_CTRL_VIN_BIND)     return TLY_CTRL_TYPE_VIN_BIND;
    return TLY_CTRL_TYPE_OTHER;
}

/* ======================== 终端控制 - 电池加热 ======================== */

int tly_decode_battery_heat(const uint8_t *buf, uint32_t len, tly_battery_heat_t *heat)
{
    if (!buf || !heat) return TLY_ERR_PARAM;
    if (len < 11) return TLY_ERR_PROTOCOL;

    memset(heat, 0, sizeof(*heat));
    memcpy(heat->time, buf, 6);
    heat->cmd         = buf[6];
    heat->time_second = tly_read_le32(buf + 7);
    return 0;
}

void tly_battery_heat_dump(const tly_battery_heat_t *heat)
{
    if (!heat) return;
    TLY_LOG_I("Battery heat: cmd=0x%02X, time_second=%u, time=20%02u-%02u-%02u %02u:%02u:%02u",
              heat->cmd, heat->time_second,
              heat->time[0], heat->time[1], heat->time[2],
              heat->time[3], heat->time[4], heat->time[5]);
}

/* ======================== 终端控制 - VIN解绑 ======================== */

int tly_decode_vin_unbind(const uint8_t *buf, uint32_t len, tly_vin_unbind_t *vu)
{
    if (!buf || !vu) return TLY_ERR_PARAM;
    if (len < 7 + TLY_VIN_UNBIND_LEN) return TLY_ERR_PROTOCOL;

    memset(vu, 0, sizeof(*vu));
    memcpy(vu->time, buf, 6);
    vu->cmd = buf[6];
    memcpy(vu->target_vin, buf + 7, TLY_VIN_UNBIND_LEN);
    vu->target_vin[TLY_VIN_UNBIND_LEN] = '\0';
    return 0;
}

void tly_vin_unbind_dump(const tly_vin_unbind_t *vu)
{
    if (!vu) return;
    TLY_LOG_I("VIN unbind: cmd=0x%02X, target_vin=%s, time=20%02u-%02u-%02u %02u:%02u:%02u",
              vu->cmd, vu->target_vin,
              vu->time[0], vu->time[1], vu->time[2],
              vu->time[3], vu->time[4], vu->time[5]);
}

/* ======================== 终端控制 - VIN绑定 ======================== */

int tly_decode_vin_bind(const uint8_t *buf, uint32_t len, tly_vin_bind_t *vb)
{
    if (!buf || !vb) return TLY_ERR_PARAM;
    if (len < 7 + TLY_VIN_BIND_LEN) return TLY_ERR_PROTOCOL;

    memset(vb, 0, sizeof(*vb));
    memcpy(vb->time, buf, 6);
    vb->cmd = buf[6];
    memcpy(vb->target_vin, buf + 7, TLY_VIN_BIND_LEN);
    vb->target_vin[TLY_VIN_BIND_LEN] = '\0';
    return 0;
}

void tly_vin_bind_dump(const tly_vin_bind_t *vb)
{
    if (!vb) return;
    TLY_LOG_I("VIN bind: cmd=0x%02X, target_vin=%s, time=20%02u-%02u-%02u %02u:%02u:%02u",
              vb->cmd, vb->target_vin,
              vb->time[0], vb->time[1], vb->time[2],
              vb->time[3], vb->time[4], vb->time[5]);
}

/* ======================== 终端控制 - 主动授时 ======================== */

int tly_decode_time_sync(const uint8_t *buf, uint32_t len, tly_time_sync_t *ts)
{
    if (!buf || !ts) return TLY_ERR_PARAM;
    if (len < 11) return TLY_ERR_PROTOCOL;

    memset(ts, 0, sizeof(*ts));
    memcpy(ts->time, buf, 6);
    ts->cmd         = buf[6];
    ts->time_second = tly_read_le32(buf + 7);
    return 0;
}

void tly_time_sync_dump(const tly_time_sync_t *ts)
{
    if (!ts) return;
    TLY_LOG_I("Time sync: cmd=0x%02X, time_second=%u, time=20%02u-%02u-%02u %02u:%02u:%02u",
              ts->cmd, ts->time_second,
              ts->time[0], ts->time[1], ts->time[2],
              ts->time[3], ts->time[4], ts->time[5]);
}

/* ======================== 终端控制 - 锁车 ======================== */

int tly_decode_lock_control(const uint8_t *buf, uint32_t len,
                            tly_lock_control_t *lock)
{
    if (!buf || !lock) return TLY_ERR_PARAM;

    memset(lock, 0, sizeof(*lock));

    if (len < 7) return TLY_ERR_PROTOCOL;

    memcpy(lock->time, buf, 6);
    lock->ctrl_cmd  = buf[6];
    lock->lock_type = buf[7];

    uint32_t pos = 8;

    /* 扭矩字段仅玉柴(0x92)存在 */
    if (lock->ctrl_cmd == TLY_CTRL_LOCK_YUCHAI) {
        if (len < 12) return TLY_ERR_PROTOCOL;  /* 6+1+1+1+2+1 */
        lock->torque = buf[pos++];
    } else {
        if (len < 11) return TLY_ERR_PROTOCOL;  /* 6+1+1+2+1 */
    }

    lock->rpm       = tly_read_be16(buf + pos);
    pos += 2;
    lock->auto_lock = buf[pos];

    return 0;
}

void tly_lock_control_dump(const tly_lock_control_t *lock)
{
    if (!lock) return;

    struct tm tm;
    tly_time_decode(lock->time, &tm);

    const char *lock_str;
    switch (lock->lock_type) {
    case 0: lock_str = "UNLOCK"; break;
    case 1: lock_str = "LOCK"; break;
    case 2: lock_str = (lock->ctrl_cmd == TLY_CTRL_LOCK_YUCHAI) ?
                       "LOCK_LIMIT_TORQUE" : "ACTIVATE"; break;
    case 3: lock_str = (lock->ctrl_cmd == TLY_CTRL_LOCK_YUCHAI) ?
                       "ACTIVATE" : "DEACTIVATE"; break;
    case 4: lock_str = "DEACTIVATE"; break;
    default: lock_str = "UNKNOWN"; break;
    }

    TLY_LOG_I("Lock control: cmd=0x%02X, type=%s, torque=%u, rpm=%u, auto=%u, "
              "time=%04d-%02d-%02d %02d:%02d:%02d",
              lock->ctrl_cmd, lock_str, lock->torque, lock->rpm, lock->auto_lock,
              tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
              tm.tm_hour, tm.tm_min, tm.tm_sec);
}

/* ======================== 终端控制 - OTA升级 ======================== */

static uint32_t read_str_sep(const uint8_t *buf, uint32_t buf_len,
                             uint32_t pos, char *out, uint32_t out_max)
{
    uint32_t i = 0;
    while (pos < buf_len && buf[pos] != 0x3B && i < out_max - 1) {
        out[i++] = (char)buf[pos++];
    }
    out[i] = '\0';
    if (pos < buf_len && buf[pos] == 0x3B) pos++;
    return pos;
}

static uint32_t read_bin_sep(const uint8_t *buf, uint32_t buf_len,
                             uint32_t pos, uint8_t *out, uint32_t n)
{
    if (pos + n > buf_len) return pos;
    memcpy(out, buf + pos, n);
    pos += n;
    if (pos < buf_len && buf[pos] == 0x3B) pos++;
    return pos;
}

/* ======================== 终端控制 - SD卡数据获取 ======================== */

int tly_decode_sd_card(const uint8_t *buf, uint32_t len, tly_sd_card_t *sd)
{
    if (!buf || !sd) return TLY_ERR_PARAM;
    if (len < 8) return TLY_ERR_PROTOCOL;

    memset(sd, 0, sizeof(*sd));

    memcpy(sd->time, buf, 6);
    sd->cmd = buf[6];

    uint32_t pos = 7;
    if (pos < len && buf[pos] == 0x3B) pos++;

    pos = read_str_sep(buf, len, pos, sd->username, sizeof(sd->username));
    pos = read_str_sep(buf, len, pos, sd->password, sizeof(sd->password));

    pos = read_bin_sep(buf, len, pos, sd->ip, 6);

    uint8_t port_buf[2];
    pos = read_bin_sep(buf, len, pos, port_buf, 2);
    sd->port = tly_read_be16(port_buf);

    char skip[TLY_OTA_STR_MAX];
    pos = read_str_sep(buf, len, pos, skip, sizeof(skip));
    pos = read_str_sep(buf, len, pos, skip, sizeof(skip));
    pos = read_str_sep(buf, len, pos, skip, sizeof(skip));

    pos = read_str_sep(buf, len, pos, sd->url, sizeof(sd->url));

    uint8_t reuse[2];
    pos = read_bin_sep(buf, len, pos, reuse, 2);

    pos = read_bin_sep(buf, len, pos, sd->begin_time, 6);

    if (pos + 2 <= len) {
        sd->time_count = tly_read_be16(buf + pos);
    }

    return 0;
}

void tly_sd_card_dump(const tly_sd_card_t *sd)
{
    if (!sd) return;

    TLY_LOG_I("SD card: cmd=0x%02X, user=%s, pwd=%s, ip=%u.%u.%u.%u.%u.%u, port=%u, url=%s",
              sd->cmd, sd->username, sd->password,
              sd->ip[0], sd->ip[1], sd->ip[2],
              sd->ip[3], sd->ip[4], sd->ip[5],
              sd->port, sd->url);
    TLY_LOG_I("  begin_time=20%02u-%02u-%02u %02u:%02u:%02u, time_count=%us",
              sd->begin_time[0], sd->begin_time[1], sd->begin_time[2],
              sd->begin_time[3], sd->begin_time[4], sd->begin_time[5],
              sd->time_count);
}

int tly_decode_ota_info(const uint8_t *buf, uint32_t len,
                        tly_ota_info_t *ota)
{
    if (!buf || !ota) return TLY_ERR_PARAM;
    if (len < 8) return TLY_ERR_PROTOCOL;

    memset(ota, 0, sizeof(*ota));

    memcpy(ota->time, buf, 6);
    ota->ota_cmd = buf[6];

    uint32_t pos = 7;

    if (pos < len && buf[pos] == 0x3B) pos++;

    pos = read_str_sep(buf, len, pos, ota->username, sizeof(ota->username));
    pos = read_str_sep(buf, len, pos, ota->password, sizeof(ota->password));

    pos = read_bin_sep(buf, len, pos, ota->ip, 6);

    uint8_t port_buf[2];
    pos = read_bin_sep(buf, len, pos, port_buf, 2);
    ota->port = tly_read_be16(port_buf);

    pos = read_str_sep(buf, len, pos, ota->mid, sizeof(ota->mid));
    pos = read_str_sep(buf, len, pos, ota->yversion, sizeof(ota->yversion));
    pos = read_str_sep(buf, len, pos, ota->gversion, sizeof(ota->gversion));
    pos = read_str_sep(buf, len, pos, ota->url, sizeof(ota->url));

    if (pos + 2 <= len) {
        ota->upgrade_time = tly_read_be16(buf + pos);
    }

    return 0;
}

void tly_ota_info_dump(const tly_ota_info_t *ota)
{
    if (!ota) return;

    struct tm tm;
    tly_time_decode(ota->time, &tm);

    TLY_LOG_I("OTA upgrade: cmd=0x%02X, user=%s, pwd=%s, ip=%u.%u.%u.%u.%u.%u, port=%u",
              ota->ota_cmd, ota->username, ota->password,
              ota->ip[0], ota->ip[1], ota->ip[2],
              ota->ip[3], ota->ip[4], ota->ip[5],
              ota->port);
    TLY_LOG_I("  mid=%s, yver=%s, gver=%s, url=%s, time=%u",
              ota->mid, ota->yversion, ota->gversion, ota->url, ota->upgrade_time);
    TLY_LOG_I("  ota_time=%04d-%02d-%02d %02d:%02d:%02d",
              tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
              tm.tm_hour, tm.tm_min, tm.tm_sec);
}

/* ======================== 升级状态 (0xCF) ======================== */

#define TLY_UPGRADE_STATUS_LEN  13  /* time(6) + ecu(1) + process(1) + error(1) + download%(2) + upgrade%(2) */

int tly_encode_upgrade_status(const tly_upgrade_status_t *status,
                              uint8_t *buf, uint32_t *len)
{
    if (!status || !buf || !len) return TLY_ERR_PARAM;

    memcpy(buf, status->time, 6);
    buf[6] = status->ecu_type;
    buf[7] = status->process_status;
    buf[8] = status->error_status;
    buf[9]  = (uint8_t)(status->download_percent);
    buf[10] = (uint8_t)(status->download_percent >> 8);
    buf[11] = (uint8_t)(status->upgrade_percent);
    buf[12] = (uint8_t)(status->upgrade_percent >> 8);

    *len = TLY_UPGRADE_STATUS_LEN;
    return 0;
}

int tly_decode_upgrade_status(const uint8_t *buf, uint32_t len,
                              tly_upgrade_status_t *status)
{
    if (!buf || !status) return TLY_ERR_PARAM;
    if (len < TLY_UPGRADE_STATUS_LEN) return TLY_ERR_PROTOCOL;

    memcpy(status->time, buf, 6);
    status->ecu_type         = buf[6];
    status->process_status   = buf[7];
    status->error_status     = buf[8];
    status->download_percent = (uint16_t)buf[9] | ((uint16_t)buf[10] << 8);
    status->upgrade_percent  = (uint16_t)buf[11] | ((uint16_t)buf[12] << 8);

    return 0;
}

void tly_upgrade_status_dump(const tly_upgrade_status_t *status)
{
    if (!status) return;
    TLY_LOG_I("Upgrade status: ecu_type=0x%02X, process=%u, error=%u, download=%u%%, upgrade=%u%%, "
              "time=20%02u-%02u-%02u %02u:%02u:%02u",
              status->ecu_type, status->process_status,
              status->error_status, status->download_percent,
              status->upgrade_percent,
              status->time[0], status->time[1], status->time[2],
              status->time[3], status->time[4], status->time[5]);
}

int tly_encode_upgrade_fail(const tly_upgrade_fail_t *fail_info,
                            uint8_t *buf, uint32_t *len)
{
    if (!fail_info || !buf || !len) {
        return TLY_ERR_PARAM;
    }
    memcpy(buf, fail_info->time, 6);
    buf[6] = fail_info->result;
    buf[7] = fail_info->error_code;

    *len = 8;
    return 0;
}

int tly_decode_upgrade_fail(const uint8_t *buf, uint32_t len,
                            tly_upgrade_fail_t *fail_info)
{
    if (!buf || !fail_info) {
        return TLY_ERR_PARAM;
    }

    if (len < 8) {
        return TLY_ERR_PROTOCOL;
    }

    memcpy(fail_info->time, buf, 6);
    fail_info->result = buf[6];
    fail_info->error_code = buf[7];

    return 0;
}

int tly_ota_send_upgrade_fail(tly_app_t *app, uint8_t result, uint8_t error_code)
{
    if (!app || !tly_tcp_is_connected(app->tcp)) {
        return TLY_ERR_CLOSED;
    }

    tly_upgrade_fail_t fail_info;
    memset(&fail_info, 0, sizeof(fail_info));
    
    tly_time_now(fail_info.time);
    
    fail_info.result = result;        
    fail_info.error_code = error_code;

    uint8_t data_buf[8];
    uint32_t data_len = 0;
    int rc = tly_encode_upgrade_fail(&fail_info, data_buf, &data_len);
    if (rc != 0) {
        TLY_LOG_E("Failed to encode upgrade fail: %d", rc);
        return rc;
    }

    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_UPGRADE_FAIL; 
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = data_len;
    pkt.data         = data_buf;

    rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        TLY_LOG_I("Upgrade fail sent: result=%u, error_code=%u, time=20%02u-%02u-%02u %02u:%02u:%02u",
                  result, error_code,
                  fail_info.time[0], fail_info.time[1], fail_info.time[2],
                  fail_info.time[3], fail_info.time[4], fail_info.time[5]);
    } else {
        TLY_LOG_E("Upgrade fail send failed: %d", rc);
    }

    uint8_t raw_buf[TLY_PKT_MAX_SIZE];
    uint32_t raw_len = 0;
    if (tly_packet_encode(&pkt, raw_buf, &raw_len) == 0) {
        TLY_LOG_Dump("UPGRADE FAIL PKT RAW:", raw_buf, raw_len);
    }

    return rc;
}

/* ======================== 定时唤醒心跳位置包 (0x4F) ======================== */

int tly_encode_wakeup_location(const tly_wakeup_location_t *loc,
                               uint8_t *buf, uint32_t *len)
{
    if (!loc || !buf || !len) return TLY_ERR_PARAM;

    /* 21字节: status(1) + lon(4) + lat(4) + alt(2) + speed(1) + heading(2)
     *         + accel_x(2) + accel_y(2) + accel_z(2) + dev_status(1) */
    uint8_t *p = buf;

    *p++ = loc->gps_status;
    tly_write_be32(p, loc->longitude);  p += 4;
    tly_write_be32(p, loc->latitude);   p += 4;
    tly_write_be16(p, loc->altitude);   p += 2;
    *p++ = loc->speed;
    tly_write_be16(p, loc->heading);    p += 2;
    tly_write_be16(p, loc->accel_x);  p += 2;
    tly_write_be16(p, loc->accel_y);  p += 2;
    tly_write_be16(p, loc->accel_z);  p += 2;
    *p++ = loc->dev_status;

    *len = (uint32_t)(p - buf);
    return 0;
}
