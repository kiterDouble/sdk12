#include "tsp_blf.h"
#include "tsp_log.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zlib.h>

#define BLF_FILE_SIGNATURE     0x47474F4C   /* "LOGG" little-endian */
#define BLF_OBJ_SIGNATURE     0x4A424F4C   /* "LOBJ" little-endian */
#define BLF_HEADER_SIZE       144
#define BLF_HEADER_VERSION_V0 0
#define BLF_OBJ_HEADER_SIZE   16
#define BLF_OBJ_HEADER_V1     1
#define BLF_OBJ_FLAG_TIME_ONE_NANS 0x00000002
#define BLF_CONTAINER_TYPE    10
#define BLF_CAN_MSG_TYPE      1
#define BLF_CAN_FD_TYPE       86
#define BLF_COMPRESSION_NONE  0
#define BLF_COMPRESSION_ZLIB  2
#define BLF_CONTAINER_MAX     (128 * 1024)

/* DLC to length conversion table for CAN FD */
static const uint8_t dlc_to_len[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64
};

static const uint8_t len_to_dlc_table[65] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  /* 0-8: direct mapping */
    9,  10, 11, 12,                      /* 9-12 */
    13, 14, 15,                          /* 13-15: reserved */
    16,                                 /* 16 */
    17, 18, 19,                          /* 17-19: reserved */
    20,                                 /* 20 */
    21, 22, 23,                          /* 21-23: reserved */
    24,                                 /* 24 */
    25, 26, 27, 28, 29, 30, 31,          /* 25-31: reserved */
    32,                                 /* 32 */
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, /* 33-48: reserved */
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,    /* 49-63: reserved */
    64                                  /* 64 */
};

#pragma pack(push, 1)

typedef struct {
    uint32_t signature;
    uint32_t header_size;
    uint8_t  app_id;
    uint8_t  app_major;
    uint8_t  app_minor;
    uint8_t  app_build;
    uint8_t  bin_log_major;
    uint8_t  bin_log_minor;
    uint8_t  bin_log_build;
    uint8_t  bin_log_patch;
    uint64_t file_size;
    uint64_t uncompressed_size;
    uint32_t object_count;
    uint32_t objects_read;
    uint16_t time_start_year;
    uint16_t time_start_month;
    uint16_t time_start_day_of_week;
    uint16_t time_start_day;
    uint16_t time_start_hour;
    uint16_t time_start_minute;
    uint16_t time_start_second;
    uint16_t time_start_millisecond;
    uint16_t time_stop_year;
    uint16_t time_stop_month;
    uint16_t time_stop_day_of_week;
    uint16_t time_stop_day;
    uint16_t time_stop_hour;
    uint16_t time_stop_minute;
    uint16_t time_stop_second;
    uint16_t time_stop_millisecond;
} blf_file_header_t;

typedef struct {
    uint32_t signature;
    uint16_t header_size;
    uint16_t header_version;
    uint32_t object_size;
    uint32_t object_type;
} blf_obj_header_t;

typedef struct {
    uint32_t flags;
    uint16_t client_index;
    uint16_t object_version;
    uint64_t timestamp;
} blf_obj_header_v1_t;

typedef struct {
    uint16_t compression_method;
    uint8_t  reserved[6];
    uint32_t uncompressed_size;
    uint8_t  reserved2[4];
} blf_container_header_t;

typedef struct {
    uint16_t channel;
    uint8_t  flags;
    uint8_t  dlc;
    uint32_t id;
    uint8_t  data[8];
} blf_can_msg_t;

typedef struct {
    uint16_t channel;
    uint8_t  flags;
    uint8_t  dlc;
    uint32_t id;
    uint32_t frame_length;
    uint8_t  bit_count;
    uint8_t  fd_flags;
    uint8_t  valid_data_bytes;
    uint8_t  reserved[5];
    uint8_t  data[64];
} blf_can_fd_msg_t;

#pragma pack(pop)

struct tly_blf_writer {
    FILE           *fp;
    long           file_size;
    uint32_t       object_count;
    uint8_t       *container_buf;
    uint32_t       container_pos;
    uint16_t       start_year, start_mon, start_day;
    uint16_t       start_hour, start_min, start_sec, start_ms;
    uint16_t       stop_year, stop_mon, stop_day;
    uint16_t       stop_hour, stop_min, stop_sec, stop_ms;
    int            has_start;
};

static void fill_systemtime_now(uint16_t *year, uint16_t *mon, uint16_t *day,
                               uint16_t *hour, uint16_t *min,
                               uint16_t *sec, uint16_t *ms)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm tm;
    localtime_r(&ts.tv_sec, &tm);
    *year = (uint16_t)(tm.tm_year + 1900);
    *mon  = (uint16_t)(tm.tm_mon + 1);
    *day  = (uint16_t)(tm.tm_mday);
    *hour = (uint16_t)(tm.tm_hour);
    *min  = (uint16_t)(tm.tm_min);
    *sec  = (uint16_t)(tm.tm_sec);
    *ms   = (uint16_t)(ts.tv_nsec / 1000000);
}

static uint8_t len_to_dlc(uint8_t len)
{
    if (len > 64) return 15;
    return len_to_dlc_table[len];
}

static void flush_container(tly_blf_writer_t *w)
{
    if (w->container_pos == 0) return;

    uLongf compressed_size = compressBound(w->container_pos);
    uint8_t *compressed = (uint8_t *)malloc(compressed_size);
    if (!compressed) return;

    int rc = compress2(compressed, &compressed_size,
                       w->container_buf, w->container_pos, Z_DEFAULT_COMPRESSION);
    if (rc != Z_OK) {
        free(compressed);
        TLY_LOG_E("BLF zlib compress failed: %d", rc);
        return;
    }

    uint32_t data_size = (uint32_t)(sizeof(blf_container_header_t) + compressed_size);
    uint32_t padded_size = (sizeof(blf_obj_header_t) + data_size + 3) & ~3u;

    blf_obj_header_t oh;
    oh.signature      = BLF_OBJ_SIGNATURE;
    oh.header_size    = BLF_OBJ_HEADER_SIZE;
    oh.header_version = BLF_HEADER_VERSION_V0;
    oh.object_size    = padded_size;
    oh.object_type    = BLF_CONTAINER_TYPE;

    blf_container_header_t ch;
    ch.compression_method = BLF_COMPRESSION_ZLIB;
    memset(ch.reserved, 0, sizeof(ch.reserved));
    ch.uncompressed_size  = w->container_pos;
    memset(ch.reserved2, 0, sizeof(ch.reserved2));

    fwrite(&oh, sizeof(oh), 1, w->fp);
    fwrite(&ch, sizeof(ch), 1, w->fp);
    fwrite(compressed, 1, compressed_size, w->fp);

    uint32_t pad = padded_size - sizeof(blf_obj_header_t) - data_size;
    if (pad > 0) {
        uint8_t zero[4] = {0};
        fwrite(zero, 1, pad, w->fp);
    }

    w->file_size += padded_size;
    w->object_count++;
    w->container_pos = 0;

    free(compressed);
}

tly_blf_writer_t *tly_blf_writer_open(const char *path)
{
    tly_blf_writer_t *w = (tly_blf_writer_t *)calloc(1, sizeof(*w));
    if (!w) return NULL;

    w->container_buf = (uint8_t *)malloc(BLF_CONTAINER_MAX);
    if (!w->container_buf) {
        free(w);
        return NULL;
    }

    w->fp = fopen(path, "wb");
    if (!w->fp) {
        TLY_LOG_E("BLF: cannot open %s", path);
        free(w->container_buf);
        free(w);
        return NULL;
    }

    blf_file_header_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.signature       = BLF_FILE_SIGNATURE;
    hdr.header_size     = BLF_HEADER_SIZE;
    hdr.bin_log_major   = 2;
    hdr.bin_log_minor   = 8;
    hdr.file_size       = BLF_HEADER_SIZE;
    hdr.uncompressed_size = BLF_HEADER_SIZE;

    fwrite(&hdr, sizeof(hdr), 1, w->fp);
    /* write remaining padding to reach 144 bytes */
    uint8_t zero_pad[BLF_HEADER_SIZE];
    memset(zero_pad, 0, sizeof(zero_pad));
    fwrite(zero_pad + sizeof(hdr), 1, BLF_HEADER_SIZE - sizeof(hdr), w->fp);
    w->file_size = BLF_HEADER_SIZE;

    fill_systemtime_now(&w->start_year, &w->start_mon, &w->start_day,
                        &w->start_hour, &w->start_min, &w->start_sec, &w->start_ms);
    w->has_start = 1;

    TLY_LOG_I("BLF: opened %s", path);
    return w;
}

int tly_blf_writer_close(tly_blf_writer_t *w)
{
    if (!w) return -1;

    flush_container(w);

    fill_systemtime_now(&w->stop_year, &w->stop_mon, &w->stop_day,
                       &w->stop_hour, &w->stop_min, &w->stop_sec, &w->stop_ms);

    blf_file_header_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.signature         = BLF_FILE_SIGNATURE;
    hdr.header_size        = BLF_HEADER_SIZE;
    hdr.bin_log_major      = 2;
    hdr.bin_log_minor      = 8;
    hdr.file_size          = (uint64_t)w->file_size;
    hdr.uncompressed_size  = (uint64_t)w->file_size;
    hdr.object_count       = w->object_count;
    hdr.objects_read        = w->object_count;
    hdr.time_start_year = w->start_year;   hdr.time_start_month = w->start_mon;
    hdr.time_start_day_of_week = 0;
    hdr.time_start_day  = w->start_day;   hdr.time_start_hour  = w->start_hour;
    hdr.time_start_minute = w->start_min; hdr.time_start_second = w->start_sec;
    hdr.time_start_millisecond = w->start_ms;
    hdr.time_stop_year = w->stop_year;    hdr.time_stop_month = w->stop_mon;
    hdr.time_stop_day_of_week = 0;
    hdr.time_stop_day   = w->stop_day;    hdr.time_stop_hour  = w->stop_hour;
    hdr.time_stop_minute = w->stop_min;   hdr.time_stop_second = w->stop_sec;
    hdr.time_stop_millisecond = w->stop_ms;

    fseek(w->fp, 0, SEEK_SET);
    fwrite(&hdr, sizeof(hdr), 1, w->fp);
    uint8_t zero_pad[BLF_HEADER_SIZE];
    memset(zero_pad, 0, sizeof(zero_pad));
    fwrite(zero_pad + sizeof(hdr), 1, BLF_HEADER_SIZE - sizeof(hdr), w->fp);
    fclose(w->fp);

    TLY_LOG_I("BLF: closed, %u objects, %ld bytes", w->object_count, w->file_size);

    free(w->container_buf);
    free(w);
    return 0;
}

int tly_blf_write_can(tly_blf_writer_t *w, const tly_can_frame_t *frame)
{
    if (!w || !frame) return -1;

    uint8_t dlc_bytes = frame->dlc;
    if (dlc_bytes > 64) {
        dlc_bytes = 64;
    }

    const bool is_ext = (frame->flags & TLY_CAN_FLAG_EXT) != 0;
    const bool is_rtr = (frame->flags & TLY_CAN_FLAG_RTR) != 0;
    const bool is_brs = (frame->flags & 0x08) != 0;

    uint8_t raw_dlc;
    uint32_t obj_data_size;
    uint32_t object_type;

    if (dlc_bytes <= 8) {
        raw_dlc = dlc_bytes;
        obj_data_size = sizeof(blf_can_msg_t);
        object_type = BLF_CAN_MSG_TYPE;
    } else {
        raw_dlc = len_to_dlc(dlc_bytes);
        obj_data_size = sizeof(blf_can_fd_msg_t);
        object_type = BLF_CAN_FD_TYPE;
    }

    uint32_t total_obj_size = sizeof(blf_obj_header_t) + sizeof(blf_obj_header_v1_t) + obj_data_size;
    uint32_t padded_size = (total_obj_size + 3) & ~3u;

    if (w->container_pos + padded_size > BLF_CONTAINER_MAX) {
        flush_container(w);
    }

    blf_obj_header_t oh;
    oh.signature      = BLF_OBJ_SIGNATURE;
    oh.header_size    = BLF_OBJ_HEADER_SIZE + sizeof(blf_obj_header_v1_t);
    oh.header_version = BLF_OBJ_HEADER_V1;
    oh.object_size    = padded_size;
    oh.object_type    = object_type;

    blf_obj_header_v1_t ohv;
    ohv.flags          = BLF_OBJ_FLAG_TIME_ONE_NANS;
    ohv.client_index   = frame->channel;
    ohv.object_version = 0;
    ohv.timestamp      = frame->timestamp_us;

    uint8_t *p = w->container_buf + w->container_pos;
    memcpy(p, &oh, sizeof(oh));        p += sizeof(oh);
    memcpy(p, &ohv, sizeof(ohv));      p += sizeof(ohv);

    uint32_t can_id = frame->id & 0x1FFFFFFFU;
    if (is_ext) can_id |= 0x80000000U;

    if (dlc_bytes <= 8) {
        blf_can_msg_t msg;
        memset(&msg, 0, sizeof(msg));
        msg.channel = frame->channel;
        msg.flags   = is_rtr ? 0x01 : 0x00;
        msg.dlc     = raw_dlc;
        msg.id      = can_id;
        memcpy(msg.data, frame->data, dlc_bytes);
        memcpy(p, &msg, sizeof(msg));   p += sizeof(msg);
    } else {
        blf_can_fd_msg_t msg;
        memset(&msg, 0, sizeof(msg));
        msg.channel          = frame->channel;
        msg.flags            = is_rtr ? 0x01 : 0x00;
        msg.dlc              = raw_dlc;
        msg.id               = can_id;
        msg.frame_length     = dlc_bytes;
        msg.bit_count        = dlc_bytes * 8;
        msg.fd_flags         = (0x01 | (is_brs ? 0x02 : 0x00));
        msg.valid_data_bytes = dlc_bytes;
        memcpy(msg.data, frame->data, dlc_bytes);
        memcpy(p, &msg, sizeof(msg));   p += sizeof(msg);
    }

    uint32_t pad = padded_size - total_obj_size;
    if (pad > 0) {
        memset(p, 0, pad);
    }

    w->container_pos += padded_size;
    return 0;
}

long tly_blf_writer_size(const tly_blf_writer_t *w)
{
    if (!w) return 0;
    return w->file_size;
}