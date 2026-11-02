#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sr_log.h"
#include "err_code.h"
#include "sr_sdk.h"
#include "pm_server_api.h"

void printMenu() {
    printf("\n===== 电源管理函数选择 =====\n");
    printf("1. 获取唤醒源\n");
    printf("2. 创建睡眠定时器\n");
    printf("3. 创建每日休眠定时器\n");
    printf("4. 销毁睡眠定时器\n");
    printf("5. 设置睡眠事件\n");
    printf("6. 清除睡眠事件\n");
    printf("q. 退出程序\n");
    printf("============================\n");
}

int main() {
    char choice = 0;
    char buffer[256] = {0};
    uint8_t eventId = 0;
    uint32_t second = 0;
    uint8_t hour = 0, minute = 0;
    uint16_t wakeupOption = 0;

    LOG_INIT("demo");

    InitSRSdk();

    do {
        printMenu();

        printf("请选择功能（输入编号）： ");
        fgets(buffer, sizeof(buffer), stdin);
        choice = buffer[0];

        switch (choice) {
            case '1': {
                uint8_t wakeupSrc = PmGetWakeupSrc();
                printf("当前唤醒源：%u\n", wakeupSrc);
                break;
            }
            case '2': {
                printf("请输入睡眠定时器的相关参数：\n");
                printf("请输入事件编号： ");
                scanf("%hhu", &eventId);
                printf("请输入休眠时间（秒）： ");
                scanf("%u", &second);

                PmCreateSleepAlarm(eventId, second, wakeupOption);
                break;
            }
            case '3': {
                printf("请输入每日休眠定时器的相关参数：\n");
                printf("请输入事件编号： ");
                scanf("%hhu", &eventId);
                printf("请输入休眠时间（小时）： ");
                scanf("%hhu", &hour);
                printf("请输入休眠时间（分钟）： ");
                scanf("%hhu", &minute);

                PmCreateSleepAlarmEveryDay(eventId, hour, minute, wakeupOption);
                break;
            }
            case '4': {
                printf("请输入要销毁的睡眠定时器的编号： ");
                scanf("%hhu", &eventId);
                PmDestroySleepAlarm(eventId);
                break;
            }
            case '5': {
                printf("请输入要设置的睡眠事件： ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0';
                PmSetSleepEvent(buffer);
                break;
            }
            case '6': {
                printf("请输入要清除的睡眠事件： ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0';
                PmCleanSleepEvent(buffer);
                break;
            }
            case 'q':
                printf("退出程序\n");
                break;
            default:
                printf("无效的选择，请重新输入。\n");
                break;
        }
    } while (choice != 'q');

    return 0;
}
