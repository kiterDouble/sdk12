#ifndef __TSP_BLF_H__
#define __TSP_BLF_H__

#include "tsp_can.h"
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tly_blf_writer tly_blf_writer_t;

tly_blf_writer_t *tly_blf_writer_open(const char *path);
int  tly_blf_writer_close(tly_blf_writer_t *w);
int  tly_blf_write_can(tly_blf_writer_t *w, const tly_can_frame_t *frame);
long tly_blf_writer_size(const tly_blf_writer_t *w);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_BLF_H__ */