#ifndef __TSP_RECORDER_H__
#define __TSP_RECORDER_H__

#include "tsp_common.h"
#include "tsp_blf.h"
#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tly_recorder {
    struct event_base *evbase;
    tly_blf_writer_t  *writer;
    char              base_dir[256];
    uint32_t          max_file_size;
    uint32_t          sample_ms;
    uint8_t           channels;
    int               running;
    struct event     *ev_timer;
    uint32_t          file_index;
} tly_recorder_t;

int  tly_recorder_init(tly_recorder_t *rec, struct event_base *evbase);
int  tly_recorder_start(tly_recorder_t *rec, const char *base_dir,
                         uint32_t max_file_size_mb, uint32_t sample_ms,
                         uint8_t channels);
void tly_recorder_stop(tly_recorder_t *rec);
void tly_recorder_destroy(tly_recorder_t *rec);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_RECORDER_H__ */