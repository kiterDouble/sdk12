#ifndef __CAN_FILE_SAVE_H__
#define __CAN_FILE_SAVE_H__


#ifdef __cplusplus
extern "C" {
#endif


// 定义一个结构体来表示 CAN 帧

typedef struct {
    unsigned char channel;            // CAN channel
    char direction;         // 'R' for RX, 'T' for TX
    unsigned int id;        // CAN ID
    unsigned char dlc;      // Data Length Code
    unsigned char data[8];  // Data bytes
    char dataflag;
    long timestamp; // 时间戳
} CAN_Frame;



#ifdef __cplusplus
}
#endif
#endif
