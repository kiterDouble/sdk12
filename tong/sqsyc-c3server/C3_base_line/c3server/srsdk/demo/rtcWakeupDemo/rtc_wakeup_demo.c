/*
 * rtc_wakeup_demo.c
 *
 * 用途：验证以下两点改动是否正常工作：
 *   1) HalModemPmWakeLockLockedEx / HalModemPmWakeLockUnlockedEx
 *      （fd 形式的独立 wakelock 接口）
 *   2) RTC 唤醒时序：PmCreateSleepAlarm -> RTC 触发 -> TimerThread 兜底
 *      持锁 -> PmModemWakeup 完成状态切换，主循环 PmModemSleep 不会
 *      抢先把系统按回 suspend
 *
 * 用法：直接运行 rtcWakeupDemo，根据菜单选择测试项。
 *   - 测试 1：Ex API 基本环回（lock/unlock，多 fd）
 *   - 测试 2：Ex API 与原 PmModemLock 共存（不破坏 gPmPriv）
 *   - 测试 3：RTC 唤醒端到端（推荐 30s 起步，挂在串口看日志）
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "sr_log.h"
#include "err_code.h"
#include "sr_sdk.h"
#include "pm_server_api.h"
#include "hal_modem_pm.h"

#define RTC_DEMO_EVENT_ID_BASE      (10)   /* CPU_APP_TIMER_START，避开系统预留 */
#define RTC_DEMO_DEFAULT_DELAY_SEC  (30)
#define RTC_DEMO_POLL_INTERVAL_MS   (500)
#define RTC_DEMO_EX_LOCK_NAME_A     "RtcDemoExLockA"
#define RTC_DEMO_EX_LOCK_NAME_B     "RtcDemoExLockB"
#define RTC_DEMO_EX_LOCK_NAME_C     "RtcDemoExLockC"
#define RTC_DEMO_REGULAR_LOCK_NAME  "RtcDemoRegLock"
#define RTC_DEMO_HOLDING_EVENT      "rtc_demo_holding"

/* 异常退出时也要清掉留在 pmServer 里的状态：
 * - g_active_alarm_id != 0 表示有一个调度中的 RTC 闹钟未销毁
 * - g_active_sleep_event_held = 1 表示有一个 sleep event 未清除 */
static volatile uint8_t g_active_alarm_id = 0;
static volatile uint8_t g_active_sleep_event_held = 0;

static void RtcDemoCleanupOnExit(void)
{
    if (g_active_alarm_id != 0)
    {
        (void)PmDestroySleepAlarm(g_active_alarm_id);
        g_active_alarm_id = 0;
    }
    if (g_active_sleep_event_held)
    {
        PmCleanSleepEvent(RTC_DEMO_HOLDING_EVENT);
        g_active_sleep_event_held = 0;
    }
}

static void RtcDemoSignalHandler(int sig)
{
    (void)sig;
    RtcDemoCleanupOnExit();
    _exit(130);
}

static void RtcDemoLogTs(const char *tag, const char *msg)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm tmv;
    localtime_r(&ts.tv_sec, &tmv);
    printf("[%02d:%02d:%02d.%03ld][%s] %s\n",
           tmv.tm_hour, tmv.tm_min, tmv.tm_sec,
           ts.tv_nsec / 1000000L, tag, msg);
    fflush(stdout);
}

/* ===================== 测试 1 ===================== */
static int RtcDemoTestExApiBasic(void)
{
    int fail = 0;

    RtcDemoLogTs("TEST1", "Ex API basic loop start");

    int32_t fdA = HalModemPmWakeLockLockedEx((const uint8_t *)RTC_DEMO_EX_LOCK_NAME_A);
    int32_t fdB = HalModemPmWakeLockLockedEx((const uint8_t *)RTC_DEMO_EX_LOCK_NAME_B);
    int32_t fdC = HalModemPmWakeLockLockedEx((const uint8_t *)RTC_DEMO_EX_LOCK_NAME_C);

    printf("    fdA=%d, fdB=%d, fdC=%d\n", fdA, fdB, fdC);
    if (fdA < 0 || fdB < 0 || fdC < 0)
    {
        RtcDemoLogTs("TEST1", "FAIL: LockedEx returned <0");
        fail = 1;
    }
    if (fdA == fdB || fdA == fdC || fdB == fdC)
    {
        RtcDemoLogTs("TEST1", "FAIL: Ex fds collided, expected distinct fds");
        fail = 1;
    }

    /* 失败入参防护 */
    int32_t fdNull = HalModemPmWakeLockLockedEx(NULL);
    if (fdNull >= 0)
    {
        RtcDemoLogTs("TEST1", "FAIL: LockedEx(NULL) should return <0");
        fail = 1;
    }
    int32_t retNeg = HalModemPmWakeLockUnlockedEx(-1);
    if (retNeg == RET_OK)
    {
        RtcDemoLogTs("TEST1", "FAIL: UnlockedEx(-1) should fail");
        fail = 1;
    }

    /* 正常释放 */
    if (fdA >= 0) (void)HalModemPmWakeLockUnlockedEx(fdA);
    if (fdB >= 0) (void)HalModemPmWakeLockUnlockedEx(fdB);
    if (fdC >= 0) (void)HalModemPmWakeLockUnlockedEx(fdC);

    /* 注意：UnlockedEx 内部已经 destroy 了 fd，不能用同一个 fd 再次调用
     * UnlockedEx 来「测试 double-free」——内核可能已经把这个 fd 复用给别处，
     * 重复调用会误删别人的锁。 */

    RtcDemoLogTs("TEST1", fail ? "TEST1 FAIL" : "TEST1 PASS");
    return fail ? RET_ERR : RET_OK;
}

/* ===================== 测试 2 ===================== */
static int RtcDemoTestExApiCoexist(void)
{
    int fail = 0;

    RtcDemoLogTs("TEST2", "Ex API coexist with regular lock");

    if (HalModemPmWakeLockCreate((const uint8_t *)RTC_DEMO_REGULAR_LOCK_NAME) != RET_OK)
    {
        RtcDemoLogTs("TEST2", "WARN: WakeLockCreate failed (可能已有别处占用 gPmPriv)，跳过本用例");
        return RET_OK;
    }
    (void)HalModemPmWakeLockLocked((const uint8_t *)RTC_DEMO_REGULAR_LOCK_NAME);

    /* 中间穿插 Ex 接口，应不影响 gPmPriv */
    int32_t fdEx = HalModemPmWakeLockLockedEx((const uint8_t *)RTC_DEMO_EX_LOCK_NAME_A);
    if (fdEx < 0)
    {
        RtcDemoLogTs("TEST2", "FAIL: LockedEx during regular lock returned <0");
        fail = 1;
    }
    (void)HalModemPmWakeLockUnlockedEx(fdEx);

    /* 还能正常用原接口解锁/销毁，说明 gPmPriv 没被 Ex 接口破坏 */
    if (HalModemPmWakeLockUnlocked((const uint8_t *)RTC_DEMO_REGULAR_LOCK_NAME) != RET_OK)
    {
        RtcDemoLogTs("TEST2", "FAIL: regular Unlocked failed after Ex use");
        fail = 1;
    }
    if (HalModemPmWakeLockDestroy((const uint8_t *)RTC_DEMO_REGULAR_LOCK_NAME) != RET_OK)
    {
        RtcDemoLogTs("TEST2", "FAIL: regular Destroy failed after Ex use");
        fail = 1;
    }

    RtcDemoLogTs("TEST2", fail ? "TEST2 FAIL" : "TEST2 PASS");
    return fail ? RET_ERR : RET_OK;
}

/* ===================== 测试 3 ===================== */
static int RtcDemoTestRtcWakeup(uint32_t delaySec, int allowSleep)
{
    char msg[256];

    snprintf(msg, sizeof(msg),
             "schedule RTC alarm in %us, allowSleep=%d", delaySec, allowSleep);
    RtcDemoLogTs("TEST3", msg);

    uint8_t eventId = (uint8_t)(RTC_DEMO_EVENT_ID_BASE);
    /* 同 ID 重复创建会失败，先清掉再建 */
    (void)PmDestroySleepAlarm(eventId);
    if (PmCreateSleepAlarm(eventId, delaySec, 0) != RET_OK)
    {
        RtcDemoLogTs("TEST3", "FAIL: PmCreateSleepAlarm failed");
        return RET_ERR;
    }
    g_active_alarm_id = eventId;  /* 信号/exit 兜底要用 */

    /* allowSleep=1 时清掉常驻 sleep event，让主循环允许休眠
     * 这是关键：只有真的进了 suspend，RTC 才会走 ALARM 唤醒路径，
     * 才能复现/验证 TimerThread 兜底 wakelock 的修复。 */
    if (allowSleep)
    {
        PmCleanSleepEvent(RTC_DEMO_HOLDING_EVENT);
        g_active_sleep_event_held = 0;
        RtcDemoLogTs("TEST3", "cleared sleep events, system may suspend now");
    }
    else
    {
        PmSetSleepEvent(RTC_DEMO_HOLDING_EVENT);
        g_active_sleep_event_held = 1;
        RtcDemoLogTs("TEST3", "holding sleep event, system will NOT suspend (仅验证定时回调)");
    }

    uint8_t lastSrc = 0xFF;
    uint32_t totalWaitMs = (delaySec + 10) * 1000U;  /* 多等 10s 观察派发 */
    uint32_t elapsed = 0;
    int gotWakeup = 0;
    while (elapsed < totalWaitMs)
    {
        uint8_t src = PmGetWakeupSrc();
        if (src != lastSrc)
        {
            snprintf(msg, sizeof(msg), "WakeupSrc changed: %u -> %u", lastSrc, src);
            RtcDemoLogTs("TEST3", msg);
            lastSrc = src;
        }
        if (src == PM_WAKEUP_SOURCE_TIMER)
        {
            RtcDemoLogTs("TEST3", "PASS: got PM_WAKEUP_SOURCE_TIMER, RTC dispatch OK");
            gotWakeup = 1;
            break;
        }
        usleep(RTC_DEMO_POLL_INTERVAL_MS * 1000);
        elapsed += RTC_DEMO_POLL_INTERVAL_MS;
    }

    /* 收尾：把这次测试期间留在 pmServer 里的状态全部清掉，
     * 避免离开 demo 后闹钟/事件继续生效 */
    (void)PmDestroySleepAlarm(eventId);
    g_active_alarm_id = 0;
    if (g_active_sleep_event_held)
    {
        PmCleanSleepEvent(RTC_DEMO_HOLDING_EVENT);
        g_active_sleep_event_held = 0;
    }

    if (!gotWakeup)
    {
        RtcDemoLogTs("TEST3", "FAIL: timeout, RTC wakeup never produced PM_WAKEUP_SOURCE_TIMER");
        return RET_ERR;
    }
    return RET_OK;
}

static void PrintMenu(void)
{
    printf("\n===== RTC 唤醒回归测试 =====\n");
    printf("1. Ex API 基本环回（lock/unlock 多 fd）\n");
    printf("2. Ex API 与原 PmModemLock 共存\n");
    printf("3. RTC 唤醒端到端（保持 sleep event，仅验回调）\n");
    printf("4. RTC 唤醒端到端（清 sleep event，验真实 suspend->resume）\n");
    printf("5. 全跑（1+2+3）\n");
    printf("q. 退出\n");
    printf("============================\n");
}

int main(void)
{
    char choice = 0;
    char buffer[64] = {0};
    uint32_t delay = RTC_DEMO_DEFAULT_DELAY_SEC;

    LOG_INIT("rtcDemo");
    InitSRSdk();

    /* 注册兜底清理：避免 Ctrl-C / 异常退出时把 RTC 闹钟和 sleep event
     * 留在 pmServer 里，导致系统后续要么被意外拉醒、要么永远进不了休眠 */
    atexit(RtcDemoCleanupOnExit);
    signal(SIGINT, RtcDemoSignalHandler);
    signal(SIGTERM, RtcDemoSignalHandler);

    do
    {
        PrintMenu();
        printf("选择: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        choice = buffer[0];

        switch (choice)
        {
        case '1':
            (void)RtcDemoTestExApiBasic();
            break;
        case '2':
            (void)RtcDemoTestExApiCoexist();
            break;
        case '3':
            printf("等待秒数(默认 %u): ", RTC_DEMO_DEFAULT_DELAY_SEC);
            if (fgets(buffer, sizeof(buffer), stdin) && buffer[0] != '\n')
                delay = (uint32_t)strtoul(buffer, NULL, 10);
            (void)RtcDemoTestRtcWakeup(delay ? delay : RTC_DEMO_DEFAULT_DELAY_SEC, 0);
            break;
        case '4':
            printf("等待秒数(默认 %u): ", RTC_DEMO_DEFAULT_DELAY_SEC);
            if (fgets(buffer, sizeof(buffer), stdin) && buffer[0] != '\n')
                delay = (uint32_t)strtoul(buffer, NULL, 10);
            (void)RtcDemoTestRtcWakeup(delay ? delay : RTC_DEMO_DEFAULT_DELAY_SEC, 1);
            break;
        case '5':
            (void)RtcDemoTestExApiBasic();
            (void)RtcDemoTestExApiCoexist();
            (void)RtcDemoTestRtcWakeup(RTC_DEMO_DEFAULT_DELAY_SEC, 0);
            break;
        case 'q':
            printf("退出\n");
            break;
        case '\n':
            break;
        default:
            printf("无效选择\n");
            break;
        }
    } while (choice != 'q');

    return 0;
}
