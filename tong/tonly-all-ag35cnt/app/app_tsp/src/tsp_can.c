#include "tsp_can.h"
#include "tsp_log.h"
#include "tsp_common.h"

#include <string.h>

static const uint32_t g_sim_ids[TLY_CAN_CHANNEL_MAX] = {
    0x18FFE117, 0x18FFF1D0, 0x18FF0CF2, 0x0CF00400
};

static const uint8_t g_sim_dlc[TLY_CAN_CHANNEL_MAX] = { 8, 8, 8, 8 };

int tly_can_init(void)
{
    TLY_LOG_I("[STUB] tly_can_init: 4-channel CAN stub ready");
    return 0;
}

void tly_can_deinit(void)
{
    TLY_LOG_I("[STUB] tly_can_deinit");
}

int tly_can_read(uint8_t channel, tly_can_frame_t *buf, int max_count)
{
    if (channel >= TLY_CAN_CHANNEL_MAX || !buf || max_count <= 0)
        return 0;

    tly_can_frame_t *f = &buf[0];
    memset(f, 0, sizeof(*f));

    f->id = g_sim_ids[channel];
    f->channel = channel;
    f->dlc = g_sim_dlc[channel];
    f->flags = TLY_CAN_FLAG_EXT;
    f->timestamp_us = tly_monotonic_time_us();

    for (int i = 0; i < f->dlc && i < TLY_CAN_DATA_LEN; i++) {
        f->data[i] = (uint8_t)(f->timestamp_us + i);
    }

    return 1;
}