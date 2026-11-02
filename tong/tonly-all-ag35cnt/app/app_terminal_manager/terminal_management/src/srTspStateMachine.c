#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <sys/prctl.h>
#include "srTspFWInterface.h"
#include "srTspRemoteProcess.h"
#include "srTspPlatformInterface.h"
#include "srTspFWLog.h"
#include "srTspLogin.h"
#include "srTspCallbackRegister.h"

/* 连接失败后的重连间隔（秒） */
#define RECONNECT_INTERVAL_SEC 5
/* 心跳发送间隔（秒） */
#define HEARTBEAT_INTERVAL_SEC 30

/* 当前MQTT状态 */
atomic_int g_mqttState = ATOMIC_VAR_INIT(MQTT_STATE_INIT);
/* 状态访问锁 */
static pthread_mutex_t g_mqttStateLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_tspIsRunning = PTHREAD_MUTEX_INITIALIZER;
/* 上一次重连时间记录 */
static time_t g_lastRetryTime = 0;
/* 上一次心跳发送时间记录 */
static time_t g_lastHeartbeatTime = 0;

/* MQTT Broker地址 */
static const char *g_broker = "ssl://121.40.204.137:8883"; // "ssl://mqtt-bdf.conn.link:8883"
static const char *g_username = NULL;
static const char *g_password = NULL;

/* 状态机线程ID */
static pthread_t g_tspTid;

static bool g_isRunning = false; // 是否正在运行

/* 函数声明 */
static void *StartRunTspStateMachine(void *args);
static void TspStateModify(SRTspState_e state);

/**
 * @brief 修改TSP状态（线程安全）
 */
static void TspStateModify(SRTspState_e state)
{
    pthread_mutex_lock(&g_mqttStateLock);
    if (atomic_load(&g_mqttState) != MQTT_STATE_DEINIT)
    { /* 如果已经在退出状态，不允许修改 */
        atomic_store(&g_mqttState, state);
    }
    pthread_mutex_unlock(&g_mqttStateLock);
}
static void TspStateSetToDeInit()
{
    pthread_mutex_lock(&g_mqttStateLock);
    atomic_store(&g_mqttState, MQTT_STATE_DEINIT);
    pthread_mutex_unlock(&g_mqttStateLock);
}

/**
 * @brief 停止TSP状态机
 */
int32_t StopTspStateMachine()
{
    pthread_mutex_lock(&g_tspIsRunning);

    if (!g_isRunning)
    {
        pthread_mutex_unlock(&g_tspIsRunning);
        return RET_OK; // 已经停止，无需重复操作
    }
    TspStateSetToDeInit();
    // 等待线程退出
    int ret = pthread_join(g_tspTid, NULL);
    if (ret != 0)
    {
        SR_TSP_LOG_E("StopTspStateMachine Failed to join thread, ret = %d", ret);
        pthread_mutex_unlock(&g_tspIsRunning);
        return RET_ERR;
    }
    g_isRunning = false;
    pthread_mutex_unlock(&g_tspIsRunning);
    // 等待线程退出（如果需要的话，可以加 pthread_join）
    return RET_OK;
}

/*
 * @brief 设置ca保存路径
 */
int32_t SRTspServiceSetCaSavePath(const char *dirPath)
{
    int32_t ret = RET_OK;
    if (SRTspWriteCaCrt(dirPath) != RET_OK)
        ret = RET_ERR;
    if (SRTspWriteClientCrt(dirPath) != RET_OK)
        ret = RET_ERR;
    if (SRTspWriteClientKey(dirPath) != RET_OK)
        ret = RET_ERR;
    if (ret != RET_OK)
    {
        SR_TSP_LOG_E("Failed to write CaCrt/clientCrt/clientKey to path: %s", dirPath);
        return RET_ERR;
    }
    return RET_OK;
}
/**
 * @brief 启动TSP状态机线程
 */
int32_t StartTspStateMachine()
{
    pthread_mutex_lock(&g_tspIsRunning);
    if (TspBaseInfoCheckCallbacksValid() != RET_OK)
    {
        return RET_ERR;
    }
    if (g_isRunning)
    {
        SR_TSP_LOG_E("StartTspStateMachine thread already running = %d", g_isRunning);
        pthread_mutex_unlock(&g_tspIsRunning);
        return RET_OK; // 已经启动，无需重复操作
    }

    g_isRunning = true;

    if (pthread_create(&g_tspTid, NULL, StartRunTspStateMachine, NULL) != 0)
    {
        g_isRunning = false;
        pthread_mutex_unlock(&g_tspIsRunning);
        return RET_ERR; // 创建线程失败
    }
    pthread_mutex_unlock(&g_tspIsRunning);
    return RET_OK;
}
/* ---------------- SN 校验函数 ---------------- */
static int IsSnValid(const char *sn)
{
    if ((sn == NULL) || (sn[0] == '\0'))
    {
        return RET_ERR; /* 空指针或空字符串 */
    }

    size_t len = strlen(sn);
    if (len == 0U)
    {
        return RET_ERR;
    }

    /* 检查是否全是 '0' */
    for (size_t i = 0; i < len; i++)
    {
        if (sn[i] != '0')
        {
            return RET_OK; /* 合法 */
        }
    }
    return RET_ERR; /* 全是 '0' */
}

/**
 * @brief TSP状态机线程主函数
 */
static void *StartRunTspStateMachine(void *args)
{
    prctl(PR_SET_NAME, "TspStateMachine");

    /* 初始化状态 */
    pthread_mutex_lock(&g_mqttStateLock);
    atomic_store(&g_mqttState, MQTT_STATE_INIT);
    pthread_mutex_unlock(&g_mqttStateLock);

    char snBuffer[64] = {0};
    /* 循环直到 SN 有效 */
    while (1)
    {
        TspBaseInfoGetSn(snBuffer, sizeof(snBuffer));
        if (atomic_load(&g_mqttState) == MQTT_STATE_DEINIT)
        {
            return NULL;
        }
        if (IsSnValid(snBuffer) == RET_OK)
        {
            SR_TSP_LOG_I("SN valid: %s\n", snBuffer);
            break; /* SN 有效，跳出循环 */
        }
        else
        {
            SR_TSP_LOG_E("SN invalid, retry...\n");
            sleep(1); /* 无效则延迟 1 秒再取 */
        }
    }

    while (1)
    {
        switch (atomic_load(&g_mqttState))
        {
        case MQTT_STATE_INIT:
        {
            /* 初始化MQTT客户端 */
            if (SRTspServiceInit(snBuffer, g_broker))
            {
                TspStateModify(MQTT_STATE_CONNECTING);
            }
            else
            {
                sleep(2); /* 初始化失败，延迟重试 */
            }
        }
        break;

        case MQTT_STATE_CONNECTING:
        {
            /* 尝试连接MQTT服务器 */
            if (SRTspServiceConnect(g_username, g_password) == 0)
            {
                TspStateModify(MQTT_STATE_CONNECTED);
            }
            else
            {
                // SR_TSP_LOG_I("connect failed, reconnect wait...");
                TspStateModify(MQTT_STATE_RECONNECT_WAIT);
                g_lastRetryTime = time(NULL); /* 记录重连起点时间 */
            }
        }
        break;

        case MQTT_STATE_CONNECTED:
        {
            /* 连接成功，进入订阅阶段 */
            TspStateModify(MQTT_STATE_SUBSCRIBED);
        }
        break;

        case MQTT_STATE_SUBSCRIBED:
        {
            /* 订阅必要的主题 */
            char topic[128];
            SR_BUILD_TOPIC(topic, SR_TOPIC_PREFIX_S2T_CONNECT_PLATFORM, snBuffer);
            SRTspServiceSubscribe(topic, MQTT_QOS_1);
            SR_BUILD_TOPIC(topic, SR_TOPIC_PREFIX_S2T_REPORT_PLATFORM, snBuffer);
            SRTspServiceSubscribe(topic, MQTT_QOS_1);
            SR_BUILD_TOPIC(topic, SR_TOPIC_PREFIX_S2T_REISSUE_PLATFORM, snBuffer);
            SRTspServiceSubscribe(topic, MQTT_QOS_1);
            SR_BUILD_TOPIC(topic, SR_TOPIC_PREFIX_S2T_COMMAND_PLATFORM, snBuffer);
            SRTspServiceSubscribe(topic, MQTT_QOS_1);
            /* 重置心跳时间 */
            g_lastHeartbeatTime = time(NULL);

            TspStateModify(MQTT_STATE_LOGIN);
        }
        break;
        case MQTT_STATE_LOGIN:
        {
            // 发送并等待登陆的报文
            if (SRTspPlatformLogin() == RET_OK)
            {
                TspStateModify(MQTT_STATE_RECV);
            }
            else
            {
                TspStateModify(MQTT_STATE_LOGOUT);
            }
        }
        break;
        case MQTT_STATE_LOGOUT:
        {
            // 发送并等待登陆的报文
            TspStateModify(MQTT_STATE_ERROR);
        }
        break;
        case MQTT_STATE_RECV:
        {
            /* 正常接收数据阶段 */
            SRMqttMessage_t pRxData = {0};

            /* 等待接收数据，超时时间2秒 */
            if (SRTspServiceRecData(&pRxData, 2000) != RET_OK)
            {
                /* 检查是否断线 */
                if (SRTspServiceGetStatus() == SR_TSP_SERVICE_NOT_CONNECT)
                {
                    TspStateModify(MQTT_STATE_ERROR);
                    SRThreadSleepMs(500); /* 小延时，避免忙等 */
                }
                // SR_TSP_LOG_I("LISTENING ...%d", SRTspServiceGetStatus());
            }
            else
            {
                /* 收到下行数据，分发到APP处理 */
                SR_TSP_LOG_I("ProcessTspDownwardData %s! %s", pRxData.topicName, pRxData.payload);
                TspRemoteProcess(&pRxData);
            }

            /* 定时发送心跳或执行心跳处理逻辑 */
            if (time(NULL) - g_lastHeartbeatTime >= HEARTBEAT_INTERVAL_SEC)
            {
                g_lastHeartbeatTime = time(NULL);
                // 心跳数据处理位置（TODO）
            }
        }
        break;

        case MQTT_STATE_ERROR:
        {
            /* 错误处理，断开连接 */
            SRTspServiceDisconnect();
            TspStateModify(MQTT_STATE_RECONNECT_WAIT);
            g_lastRetryTime = time(NULL); /* 准备下一次重连 */
        }
        break;

        case MQTT_STATE_RECONNECT_WAIT:
        {
            /* 检查是否到达重连等待时间 */
            if (time(NULL) - g_lastRetryTime >= RECONNECT_INTERVAL_SEC)
            {
                // SR_TSP_LOG_I("retrying connect...");
                TspStateModify(MQTT_STATE_CONNECTING);
            }
            else
            {
                sleep(1); /* 每秒检查一次是否可以重连 */
            }
        }
        break;

        default:
            break;
        }

        /* 检查是否收到退出指令 */
        if (atomic_load(&g_mqttState) == MQTT_STATE_DEINIT)
        {
            SRTspServiceDeInit(); /* 清理资源 */
            // SR_TSP_LOG_I("Tsp DeInit...");
            break; /* 退出线程 */
        }
    }
    return NULL;
}
