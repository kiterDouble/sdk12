#include "can_msg.h"
// #include "can_tcp_proxy.h"
#include "err_code.h"
#include "sr_log.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#define  Print_Debug 1
// 消息接收回调函数
void msg_recv_callback(CanMsg_t* msgs, uint32_t msg_count)
{

    for (uint32_t i = 0; i < msg_count; i++) 
    {
#if Print_Debug
        printf("收到消息%d：总线=%d, ID=0x%x, 长度=%d ",msg_count,msgs[i].bus, msgs[i].id, msgs[i].dlc);
        printf("Payload: ");
        for (int j = 0; j < msgs[i].dlc; j++) {
            printf("%02x ", msgs[i].data[j]);
        }
        printf("(时间戳: %lld us)\n", msgs[i].timestamp);
#else
        LOG_Debug("recv: bus=%d, ID=0x%x, len=%d ",msgs[i].bus, msgs[i].id, msgs[i].dlc);
        LOG_DebugHex(msgs[i].data, msgs[i].dlc, "can msg recv:data");
        LOG_Debug("(time: %lld us)\n", msgs[i].timestamp);
#endif
    }

}

// 错误回调函数
void error_callback(CanError_t error)
{
    printf("返回：");
    switch(error) {
        case CAN_ERROR_NONE: printf("无错误\n"); break;
        case CAN_ERROR_BUS_OFF: printf("总线关闭\n"); break;//00
        case CAN_ERROR_OVERRUN: printf("消息溢出\n"); break;
        case CAN_ERROR_TIMEOUT: printf("超时（未收到响应）\n"); break;
        case CAN_ERROR_FILTER: printf("过滤器错误\n"); break;
        case CAN_ERROR_HARDWARE: printf("硬件异常\n"); break;
        case CAN_ERROR_INVALID_DLC: printf("无效数据长度\n"); break;//00
        default: printf("未知错误\n"); break;
    }
}

// 构造并发送测试消息（区分SUB=0x01和0x03类型）
void send_test_messages()
{
    // 测试消息：包含SUB=0x01（ID最高位0）和SUB=0x03（ID最高位1）
    CanMsg_t tx_msgs[10] = {
        // SUB=0x01类型（需等待0x02响应）
        {
            .bus = CAN_BUS_1,
            .id = 0x702,  // 最高位为0
            .dlc = 4,
            .is_fd = 0,
            .data = {0x01, 0x02, 0x03, 0x04}
        },
        // SUB=0x03类型（需等待0x04响应）
        {
            .bus = CAN_BUS_1,
            .id = 0x701,  // 最高位为1（0x80000000 | 0x712）
            .dlc = 8,
            .is_fd = 1,
            .data = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22}
        },
        // 再次发送SUB=0x01类型
        {
            .bus = CAN_BUS_1,
            .id = 0x791,
            .dlc = 5,
            .is_fd = 0,
            .data = {0xAB, 0xCD,0xFF,0x23,0x33}
        },
        // 再次发送SUB=0x03类型
        {
            .bus = CAN_BUS_0,
            .id = 0x772,  // 最高位为1
            .dlc = 3,
            .is_fd = 0,
            .data = {0x11, 0x22, 0x33}
        },
        // 再次发送SUB=0x03类型
        {
            .bus = CAN_BUS_1,
            .id = 0x666,  // 最高位为1
            .dlc = 3,
            .is_fd = 0,
            .data = {0x11, 0x22, 0x33}
        },
        // 再次发送SUB=0x03类型
        {
            .bus = CAN_BUS_0,
            .id = 0x777,  // 最高位为1
            .dlc = 3,
            .is_fd = 0,
            .data = {0x11, 0x22, 0x33}
        },
        // 再次发送SUB=0x03类型
        {
            .bus = CAN_BUS_1,
            .id = 0x128,  // 最高位为1
            .dlc = 3,
            .is_fd = 0,
            .data = {0xAA, 0x22, 0x33}
        },
        // 再次发送SUB=0x03类型
        {
            .bus = CAN_BUS_1,
            .id = 0x999,  // 最高位为1
            .dlc = 3,
            .is_fd = 0,
            .data = {0xBB, 0x22, 0x33}
        },
        // 再次发送SUB=0x03类型
        {
            .bus = CAN_BUS_0,
            .id = 0x123,  // 最高位为1
            .dlc = 3,
            .is_fd = 0,
            .data = {0x66, 0x22, 0x33}
        },
        // 再次发送SUB=0x03类型
        {
            .bus = CAN_BUS_1,
            .id = 0x456,  // 最高位为1
            .dlc = 3,
            .is_fd = 0,
            .data = {0x11, 0x22, 0x44}
        }
    };

    // 为每条消息设置时间戳并发送
    struct timeval tv;
    for (int i = 0; i < 10; i++) {
        gettimeofday(&tv, NULL);
        tx_msgs[i].timestamp = (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
        
        int ret = CanMsg_Transmit(&tx_msgs[i]);
        if (ret == 0) {
            uint8_t sub_type = 0x01;
#if !Print_Debug
            printf("发送成功：SUB=0x%02x, 总线=%d, ID=0x%x, 长度=%d\n",sub_type, tx_msgs[i].bus, tx_msgs[i].id, tx_msgs[i].dlc);
#else
            LOG_Debug("Send OK SUB=0x%02x, bus=%d, ID=0x%x, len=%d\n",sub_type, tx_msgs[i].bus, tx_msgs[i].id, tx_msgs[i].dlc);
#endif
        } else {
            printf("发送失败：总线=%d, ID=0x%x, 错误码=%d\n",tx_msgs[i].bus, tx_msgs[i].id, ret);
        }
        // usleep(1000);
        
    }
}

int main()
{
    // 配置过滤器
    uint32_t filter_ids[] = {0x702,0x711,0x772,0x76A,0x732,0x730,0x779,0x123,0x233,0x456};
    CanFilter_t filter = {
        .bus = CAN_BUS_0,
        .ids = filter_ids,
        .id_count = 10
    };
    // uint32_t filter_ids[] = {0x76A};
    // CanFilter_t filter = {
    //     .bus = CAN_BUS_0,
    //     .ids = filter_ids,
    //     .id_count = 1
    // };

    // 配置参数
    CanMsgConfig_t config = {
        .filters = &filter,
        .filter_count = 1,
        .buffer_size = 100,
        .flush_cycle = 10,
        .recv_indication_cbk = msg_recv_callback,
        .error_indication_cbk = error_callback
    };
    // LOG_INIT("canmsgDemo");
    // LOG_Debug("config.flush_cycle:%d\r\n",config.flush_cycle);
    // 初始化并启动CAN模块
    if (CanMsg_Init(&config) == 0) {
        printf("CAN模块初始化成功4\n");
        if (CanMsg_Start() == 0) {
            printf("CAN接收启动成功，运行60秒...\n");
            // // 每隔5秒发送一次
            for (int i = 0; i <12; i++) {
                sleep(1);                
                send_test_messages();
            }
            CanMsg_Stop();
        } else {
            printf("CAN接收启动失败\n");
        }
        CanMsg_Deinit();
    } else {
        printf("CAN模块初始化失败\n");
    }

    return 0;
}