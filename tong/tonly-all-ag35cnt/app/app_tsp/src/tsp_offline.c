/**
 * tsp_offline.c - 离线数据存储与补发
 *
 * 文件名格式: {epoch_sec}_{seq}.bin
 * 文件内容: 原始数据单元(与0x02 realtime report data一致)
 */

#include "tsp_offline.h"
#include "tsp_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

#define TLY_PENDING_DIR  "pending"

static int ensure_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) return 0;
        return -1;
    }
    if (mkdir(path, 0755) != 0) {
        TLY_LOG_E("Failed to create dir %s: %s", path, strerror(errno));
        return -1;
    }
    return 0;
}

int tly_offline_init(const char *base_dir)
{
    if (!base_dir) return -1;
    if (ensure_dir(base_dir) != 0) return -1;

    char path[512];
    snprintf(path, sizeof(path), "%s/%s", base_dir, TLY_PENDING_DIR);
    return ensure_dir(path);
}

int tly_offline_save(const char *base_dir, int max_days,
                     const uint8_t *data, uint32_t data_len)
{
    (void)max_days;
    if (!base_dir || !data || data_len == 0) return -1;

    static uint32_t save_seq = 0;
    save_seq++;

    time_t now = time(NULL);
    char path[512];
    snprintf(path, sizeof(path), "%s/%s/%ld_%u.bin",
             base_dir, TLY_PENDING_DIR, (long)now, save_seq);

    FILE *fp = fopen(path, "wb");
    if (!fp) {
        TLY_LOG_E("Failed to open offline file %s: %s", path, strerror(errno));
        return -1;
    }

    size_t written = fwrite(data, 1, data_len, fp);
    fclose(fp);

    if (written != data_len) {
        TLY_LOG_E("Failed to write offline file %s", path);
        remove(path);
        return -1;
    }

    TLY_LOG_D("Offline saved: %s (%u bytes)", path, data_len);
    return 0;
}

static int parse_filename(const char *name, time_t *out_ts)
{
    /* format: {epoch}_{seq}.bin */
    char *end = NULL;
    long ts = strtol(name, &end, 10);
    if (!end || *end != '_') return -1;
    if (out_ts) *out_ts = (time_t)ts;
    return 0;
}

static int cmp_str(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

/**
 * 扫描 pending 目录，返回排序后的文件名列表和数量
 * 调用者需 free(*out_names) 和每个 names[i]
 */
static int scan_pending(const char *base_dir, char ***out_names, int *out_count)
{
    char dirpath[512];
    snprintf(dirpath, sizeof(dirpath), "%s/%s", base_dir, TLY_PENDING_DIR);

    DIR *dir = opendir(dirpath);
    if (!dir) {
        *out_names = NULL;
        *out_count = 0;
        return 0;
    }

    int cap = 64;
    char **names = (char **)malloc(cap * sizeof(char *));
    int count = 0;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        size_t len = strlen(ent->d_name);
        if (len < 5 || strcmp(ent->d_name + len - 4, ".bin") != 0) continue;

        if (count >= cap) {
            cap *= 2;
            names = (char **)realloc(names, cap * sizeof(char *));
        }
        names[count++] = strdup(ent->d_name);
    }
    closedir(dir);

    if (count > 0) {
        qsort(names, count, sizeof(char *), cmp_str);
    }

    *out_names = names;
    *out_count = count;
    return 0;
}

int tly_offline_pop_oldest(const char *base_dir, int max_days,
                           tly_offline_record_t *record,
                           uint8_t *buf, uint32_t buf_size)
{
    if (!base_dir || !record || !buf) return -1;

    char **names = NULL;
    int count = 0;
    scan_pending(base_dir, &names, &count);

    if (count == 0) {
        free(names);
        return -1;
    }

    time_t now = time(NULL);
    time_t cutoff = now - (time_t)max_days * 86400;

    int result = -1;
    for (int i = 0; i < count; i++) {
        time_t ts = 0;
        if (parse_filename(names[i], &ts) != 0) {
            free(names[i]);
            continue;
        }

        /* 跳过过期记录 */
        if (ts < cutoff) {
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s/%s/%s",
                     base_dir, TLY_PENDING_DIR, names[i]);
            remove(fullpath);
            TLY_LOG_D("Expired offline record deleted: %s", names[i]);
            free(names[i]);
            continue;
        }

        /* 读取该文件 */
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s/%s",
                 base_dir, TLY_PENDING_DIR, names[i]);

        FILE *fp = fopen(fullpath, "rb");
        if (!fp) {
            free(names[i]);
            continue;
        }

        size_t nread = fread(buf, 1, buf_size, fp);
        fclose(fp);

        if (nread == 0) {
            free(names[i]);
            continue;
        }

        strncpy(record->filename, names[i], TLY_OFFLINE_NAME_MAX - 1);
        record->timestamp = ts;
        record->data_len = (uint32_t)nread;
        result = 0;

        /* 清理剩余的 names */
        for (int j = i; j < count; j++) free(names[j]);
        break;
    }

    free(names);
    return result;
}

int tly_offline_delete(const char *base_dir, const char *filename)
{
    if (!base_dir || !filename) return -1;

    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "%s/%s/%s",
             base_dir, TLY_PENDING_DIR, filename);

    if (remove(fullpath) == 0) {
        TLY_LOG_D("Offline record deleted: %s", filename);
        return 0;
    }
    return -1;
}

int tly_offline_get_stats(const char *base_dir, int max_days,
                          tly_offline_stats_t *stats)
{
    if (!base_dir || !stats) return -1;

    memset(stats, 0, sizeof(*stats));

    char **names = NULL;
    int count = 0;
    scan_pending(base_dir, &names, &count);

    time_t now = time(NULL);
    time_t cutoff = now - (time_t)max_days * 86400;

    for (int i = 0; i < count; i++) {
        time_t ts = 0;
        if (parse_filename(names[i], &ts) != 0 || ts < cutoff) {
            free(names[i]);
            continue;
        }
        if (stats->total_count == 0) stats->oldest_time = ts;
        stats->newest_time = ts;
        stats->total_count++;
        free(names[i]);
    }

    free(names);
    return 0;
}

int tly_offline_cleanup(const char *base_dir, int max_days)
{
    if (!base_dir) return 0;

    char **names = NULL;
    int count = 0;
    scan_pending(base_dir, &names, &count);

    time_t now = time(NULL);
    time_t cutoff = now - (time_t)max_days * 86400;
    int deleted = 0;

    for (int i = 0; i < count; i++) {
        time_t ts = 0;
        if (parse_filename(names[i], &ts) != 0) {
            free(names[i]);
            continue;
        }
        if (ts < cutoff) {
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s/%s/%s",
                     base_dir, TLY_PENDING_DIR, names[i]);
            if (remove(fullpath) == 0) deleted++;
        }
        free(names[i]);
    }

    free(names);

    if (deleted > 0) {
        TLY_LOG_I("Offline cleanup: deleted %d expired records (> %d days)", deleted, max_days);
    }
    return deleted;
}
