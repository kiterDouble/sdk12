#ifndef __CLOCK_SERVER_H
#define __CLOCK_SERVER_H

#include <glib.h>
#include "msg_queue.h"

/* 时间管理配置文件路径 */
#define TIME_SYNC_CFG_PATH          "/oemapp/config/time.ini"
#define TIME_SYNC_CFG_PATH_PRIMARY  "/oemdata/config/time.ini"

/** NTP 服务器地址缓冲区最大长度 */
#define TIME_SOURCE_NTP_HOST_BUF_MAX_SIZE       (128)
/** 最大可配置 NTP 服务器数量 */
#define TIME_SOURCE_NTP_HOST_MAX_NUM            (10)
/** 默认授时活性时间（单位：秒），10 分钟 */
#define TIME_SOURCE_DEFAULT_ACTIVATION_SEC      (60*10)
/** 默认异常时钟跳变检测阈值（单位：秒） */
#define TIME_JUMP_DEFAULT_THRESHOLD             (60)

/**
 * @brief 优先级与活性时间配置结构体
 *
 * 用于配置各授时源的优先级和有效期。
 */
typedef struct
{
    int32_t priority;           /**< 优先级，数值越大优先级越高 */
    int32_t activation;         /**< 活性时间，单位秒 */
}TimePriorityActivationInfo_t;

/**
 * @brief 时间管理服务器配置结构体
 *
 * 从 time.ini 配置文件解析得到，包含 TSP/GNSS/NTP/RTC 各段配置。
 */
typedef struct
{
    struct{
        TimePriorityActivationInfo_t priorityActivation;    /**< TSP 优先级和活性时间 */
    }tspCfg;
    struct{
        TimePriorityActivationInfo_t priorityActivation;    /**< GNSS 优先级和活性时间 */
    }gnssCfg;
    struct{
        int hostNum;                                            /**< 实际配置的 NTP 服务器数量 */
        struct{
            char name[TIME_SOURCE_NTP_HOST_BUF_MAX_SIZE];       /**< NTP 服务器地址 */
        }host[TIME_SOURCE_NTP_HOST_MAX_NUM];                    /**< NTP 服务器列表 */
        TimePriorityActivationInfo_t priorityActivation;        /**< NTP 优先级和活性时间 */
    }ntpCfg;
    int32_t timeJumpEventThreshold;                             /**< 时间异常跳变阈值，单位秒 */
    char rtcDev[32];                                            /**< RTC 设备节点路径（如 "/dev/rtc0"） */
}TimeServerConfig_t;

/**
 * @brief 时间同步任务线程数据结构体
 *
 * 每个工作线程（GNSS/NTP/Broadcast/Manage）各持有一个实例，用于线程生命周期控制。
 */
typedef struct
{
    gint taskQuitFlag;          /**< 线程退出标志，使用 g_atomic_int 访问 */
    GThread* taskThread;        /**< GLib 线程句柄 */
}TimeSyncTaskThreadData_t;

/**
 * @brief 时间管理服务全局上下文结构体
 *
 * 主线程创建并初始化，所有工作线程共享读取。
 * 其中 gint 类型字段必须使用 g_atomic_int_get/set 进行线程安全访问。
 */
typedef struct
{
    gint pmState;                                   /**< 当前电源管理状态（PM_STATE_NORMAL 等） */
    gint timeSyncSource;                            /**< 当前授时源（E_TIME_SYNC_SRC_NONE/NTP/GNSS/TSP） */
    gint timeSyncSourceAboutToExpire;               /**< 当前授时源即将过期标志（1=即将过期） */
    gint priority;                                  /**< 当前授时源优先级 */
    MsgQueue_t timeSyncEventQueue;                  /**< 时间同步事件队列（Manage 线程消费） */
    TimeServerConfig_t config;                      /**< 时间服务配置信息 */
    void* broadcastTaskThreadData;                  /**< Broadcast 线程数据指针 */
    void* gnssTaskThreadData;                       /**< GNSS 线程数据指针 */
    void* ntpTaskThreadData;                        /**< NTP 线程数据指针 */
    void* manageTaskThreadData;                     /**< Manage 线程数据指针 */
}TimeSrvCtx_t;


/**
 * @brief 解析时间管理配置文件
 *
 * 使用 GKeyFile 解析 time.ini，读取 [timeJumpEvent]、[tsp]、[ntp]、[gnss]、[rtc] 各段配置。
 *
 * @param[in]  cfgPath 配置文件路径
 * @param[out] cfg     输出配置结构体
 * @return int32_t 0 表示成功，非 0 表示失败
 */
int32_t TimeCfgParse(const char* cfgPath, TimeServerConfig_t* cfg);

/**
 * @brief 获取指定授时源的配置活性时间
 *
 * @param[in] cfg        配置结构体指针
 * @param[in] timeSource 授时源类型（E_TIME_SYNC_SRC_NTP/GNSS/TSP）
 * @return int32_t 活性时间（秒），默认返回 TIME_SOURCE_DEFAULT_ACTIVATION_SEC
 */
int32_t TimeCfgGetSyncSourceActivation(TimeServerConfig_t* cfg, uint8_t timeSource);

/**
 * @brief 获取指定授时源的配置优先级
 *
 * @param[in] cfg        配置结构体指针
 * @param[in] timeSource 授时源类型（E_TIME_SYNC_SRC_NTP/GNSS/TSP）
 * @return int32_t 优先级，默认返回 0
 */
int32_t TimeCfgGetSyncSourcePriority(TimeServerConfig_t* cfg, uint8_t timeSource);

/**
 * @brief RPC 总线服务器回调函数
 *
 * 注册到 RPC Server，根据 subcmd 分发到具体处理函数。
 * 当前仅处理 E_RPC_TIME_RSEVER_REPORT_TSP_TIME（subcmd=0x0）。
 *
 * @param[in]  subcmd  RPC 子命令
 * @param[in]  reqData 请求数据指针
 * @param[in]  reqLen  请求数据长度
 * @param[out] respData 响应数据缓冲区
 * @param[out] respLen  响应数据长度
 * @param[in]  userdata 用户数据（指向 TimeSrvCtx_t）
 */
void TimeRpcBusServerCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata);


/**
 * @brief 初始化 GNSS 时间同步线程
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功，非 0 表示失败
 */
int InitTimeSyncGnssTask(void* arg);

/**
 * @brief 释放 GNSS 时间同步线程
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功
 */
int ReleaseTimeSyncGnssTask(void* arg);


/**
 * @brief 初始化 NTP 时间同步线程
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功，非 0 表示失败
 */
int InitTimeSyncNtpTask(void* arg);

/**
 * @brief 释放 NTP 时间同步线程
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功
 */
int ReleaseTimeSyncNtpTask(void* arg);


/**
 * @brief 初始化时间同步广播线程
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功，非 0 表示失败
 */
int InitTimeSyncBroadcastTask(void* arg);

/**
 * @brief 释放时间同步广播线程
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功
 */
int ReleaseTimeSyncBroadcastTask(void* arg);

/**
 * @brief 初始化同步时间管理线程
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功，非 0 表示失败
 */
int InitTimeSyncManageTask(void* arg);

/**
 * @brief 释放同步时间管理线程
 *
 * @param[in] arg 指向 TimeSrvCtx_t 的指针
 * @return int 0 表示成功
 */
int ReleaseTimeSyncManageTask(void* arg);



#endif
