#ifndef __CAN_FILE_SAVE_H__
#define __CAN_FILE_SAVE_H__
#include <stdint.h>
#include "time_status.h"
#ifdef __cplusplus
extern "C"
{
#endif

#pragma pack(push, 1)

    typedef struct
    {
        uint64_t timestamp; // 时间戳 微秒
        uint8_t channel;
        uint32_t can_id;
        uint8_t direction; // 'R' for RX, 'T' for TX
        uint8_t flags;     // bit0=FD, bit1=TX/RX, bit2=BRS, bit3=ESI
        uint8_t dlc;
        uint8_t data[8];
    } Can_frame_t;

#pragma pack(pop)

typedef struct
{
    uint64_t start_time; // 开始时间戳
    uint64_t end_time;   // 结束时间戳
} TimeRange_t;
#ifdef __cplusplus
}
#endif
#endif
