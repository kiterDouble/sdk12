#include <iostream>
#include <future>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <ctime>
#include <chrono>
#include <cerrno>
#include "ota_master_timer.h"
#include "ota_master_log.h"

static OtaTaskTimeoutCallbackFunc g_timeoutCallback = nullptr; // OTA任务超时回调函数
static std::chrono::steady_clock::time_point g_endTime;        // OTA任务结束时间
static std::mutex g_timerMutex;                                // OTA任务超时计时器互斥锁
static std::condition_variable g_timerCond;                    // OTA任务超时计时器条件变量
static std::atomic<bool> g_isTimerStart(false);                // OTA任务超时计时器是否启动
static std::thread g_timerWorkThread;                          // OTA任务超时计时器工作线程

static void TimerWorkThread(void)
{
    std::unique_lock<std::mutex> locker(g_timerMutex);
    if (!g_isTimerStart.load()) {
        OTA_LOG_E("OTA task timer work thread started but timer is not running");
        return;
    }

    auto remaining = g_endTime - std::chrono::steady_clock::now();
    OTA_LOG_D("OTA task timer work thread started, remaining time: %lld seconds",
              std::chrono::duration_cast<std::chrono::seconds>(remaining).count());

    while (g_isTimerStart.load() && remaining > std::chrono::seconds(0)) {
        auto status = g_timerCond.wait_for(locker, remaining);
        if (status == std::cv_status::timeout) {
            OTA_LOG_D("OTA task timer expired");
            break;
        }
        remaining = g_endTime - std::chrono::steady_clock::now();
    }

    if (g_isTimerStart.load() && g_timeoutCallback) {
        OTA_LOG_D("OTA task timer expired, executing timeout callback");
        g_timeoutCallback();
    }

    g_isTimerStart.store(false);
    locker.unlock();
    OTA_LOG_D("OTA task timer work thread exit");
}

void StartOtaTaskTimeoutMonitor(uint32_t startTime, uint32_t duration, const OtaTaskTimeoutCallbackFunc callback)
{
    OTA_LOG_D("Start OTA task timeout monitor, start time: %u, duration: %u", startTime, duration);
    std::lock_guard<std::mutex> locker(g_timerMutex);
    if (g_isTimerStart.load()) {
        OTA_LOG_E("Start OTA task timeout monitor failed, OTA task timeout monitor is already started");
        return;
    }

    if (callback == nullptr) {
        OTA_LOG_E("Start OTA task timeout monitor failed, callback is nullptr");
        return;
    }

    // 清理旧线程
    if (g_timerWorkThread.joinable()) {
        OTA_LOG_E("Previous OTA task timeout monitor thread is still running, waiting for it to finish");
        g_timerCond.notify_all();
        g_timerWorkThread.join();
    }

    g_timeoutCallback = callback;
    g_endTime = std::chrono::steady_clock::now() + std::chrono::seconds(duration);
    g_isTimerStart.store(true);
    g_timerWorkThread = std::thread(TimerWorkThread);
}

void StopOtaTaskTimeoutMonitor(void)
{
    OTA_LOG_D("Stop OTA task timeout monitor");
    {
        std::lock_guard<std::mutex> locker(g_timerMutex);
        if (!g_isTimerStart.load()) {
            OTA_LOG_D("OTA task timeout monitor is not running, nothing to stop");
            return;
        }
        g_isTimerStart.store(false);
        g_timerCond.notify_all();
    }
    if (g_timerWorkThread.joinable()) {
        g_timerWorkThread.join();
    }
    g_timerWorkThread = std::thread();
    OTA_LOG_D("OTA task timeout monitor stopped");
}