#include "tsp_recorder.h"
#include "tsp_can.h"
#include "tsp_log.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

static void recorder_timer_cb(evutil_socket_t fd, short events, void *ctx);
static int recorder_open_file(tly_recorder_t *rec);
static void recorder_close_file(tly_recorder_t *rec);

static void recorder_timer_cb(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd;
    (void)events;
    tly_recorder_t *rec = (tly_recorder_t *)ctx;

    if (!rec->running) return;

    tly_can_frame_t frame;
    for (int ch = 0; ch < TLY_CAN_CHANNEL_MAX; ch++) {
        if (!(rec->channels & (1 << ch))) continue;

        int n = tly_can_read((uint8_t)ch, &frame, 1);
        for (int i = 0; i < n; i++) {
            tly_blf_write_can(rec->writer, &frame);
        }
    }

    if (tly_blf_writer_size(rec->writer) >= (long)rec->max_file_size) {
        recorder_close_file(rec);
        recorder_open_file(rec);
    }
}

static int ensure_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) return 0;
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", path);
    return system(cmd);
}

static int recorder_open_file(tly_recorder_t *rec)
{
    char path[512];
    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);

    snprintf(path, sizeof(path), "%s/CAN_%04d%02d%02d_%02d%02d%02d.blf",
             rec->base_dir,
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);

    rec->writer = tly_blf_writer_open(path);
    if (!rec->writer) {
        TLY_LOG_E("Recorder: failed to open %s", path);
        return -1;
    }

    rec->file_index++;
    TLY_LOG_I("Recorder: opened file #%u: %s", rec->file_index, path);
    return 0;
}

static void recorder_close_file(tly_recorder_t *rec)
{
    if (rec->writer) {
        tly_blf_writer_close(rec->writer);
        rec->writer = NULL;
    }
}

int tly_recorder_init(tly_recorder_t *rec, struct event_base *evbase)
{
    if (!rec || !evbase) return -1;
    memset(rec, 0, sizeof(*rec));
    rec->evbase = evbase;
    return 0;
}

int tly_recorder_start(tly_recorder_t *rec, const char *base_dir,
                        uint32_t max_file_size_mb, uint32_t sample_ms,
                        uint8_t channels)
{
    if (!rec || !base_dir) return -1;
    if (rec->running) return 0;

    strncpy(rec->base_dir, base_dir, sizeof(rec->base_dir) - 1);
    rec->max_file_size = max_file_size_mb * 1024 * 1024;
    rec->sample_ms     = sample_ms > 0 ? sample_ms : 10;
    rec->channels      = channels > 0 ? channels : 0x0F;

    if (ensure_dir(rec->base_dir) != 0) {
        TLY_LOG_E("Recorder: cannot create dir %s", rec->base_dir);
        return -1;
    }

    tly_can_init();

    if (recorder_open_file(rec) != 0) {
        return -1;
    }

    struct timeval tv;
    tv.tv_sec  = rec->sample_ms / 1000;
    tv.tv_usec = (rec->sample_ms % 1000) * 1000;

    rec->ev_timer = event_new(rec->evbase, -1,
                               EV_TIMEOUT | EV_PERSIST,
                               recorder_timer_cb, rec);
    if (!rec->ev_timer) {
        recorder_close_file(rec);
        return -1;
    }
    evtimer_add(rec->ev_timer, &tv);

    rec->running = 1;
    TLY_LOG_I("Recorder: started, dir=%s, max=%uMB, sample=%ums, ch=0x%02X",
              rec->base_dir, max_file_size_mb, rec->sample_ms, rec->channels);
    return 0;
}

void tly_recorder_stop(tly_recorder_t *rec)
{
    if (!rec || !rec->running) return;

    rec->running = 0;

    if (rec->ev_timer) {
        event_del(rec->ev_timer);
        event_free(rec->ev_timer);
        rec->ev_timer = NULL;
    }

    recorder_close_file(rec);
    TLY_LOG_I("Recorder: stopped, %u files written", rec->file_index);
}

void tly_recorder_destroy(tly_recorder_t *rec)
{
    if (!rec) return;
    tly_recorder_stop(rec);
    tly_can_deinit();
}