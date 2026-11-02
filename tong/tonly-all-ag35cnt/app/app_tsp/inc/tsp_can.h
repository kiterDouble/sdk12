#ifndef __TSP_CAN_H__
#define __TSP_CAN_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TLY_CAN_CHANNEL_MAX  4
#define TLY_CAN_DATA_LEN     8

typedef struct tly_can_frame {
    uint32_t id;
    uint8_t  channel;
    uint8_t  dlc;
    uint8_t  data[TLY_CAN_DATA_LEN];
    uint8_t  flags;
    uint64_t timestamp_us;
} tly_can_frame_t;

#define TLY_CAN_FLAG_EXT  0x01
#define TLY_CAN_FLAG_RTR  0x02
#define TLY_CAN_FLAG_EDL  0x04

int tly_can_init(void);
void tly_can_deinit(void);
int tly_can_read(uint8_t channel, tly_can_frame_t *buf, int max_count);

#ifdef __cplusplus
}
#endif

#endif /* __TSP_CAN_H__ */