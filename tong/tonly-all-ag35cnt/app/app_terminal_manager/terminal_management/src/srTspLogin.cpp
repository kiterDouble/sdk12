#include "time.h"
#include "srTspLogin.h"
#include "terminal_management_data_monitor.h"
#include "terminal_management_data_encode.h"
#include <string>
#include <string.h>
#include <json/json.h>
#include <fstream>
#include "srTspFWInterface.h"
#include "srTspFWLog.h"
#include "srTspEncryptAndDecrypt.h"
#include "srTspCallbackRegister.h"

static char g_connectId[256] = {0};

/**
 * @brief 序列化 S3Connect 消息到内存 buffer
 *
 * @param[out] buffer 序列化后的输出缓冲区
 * @return true  序列化成功
 * @return false 序列化失败
 */
static int32_t SerializeS3ConnectMessage(std::string &buffer)
{
    // 1. 确保静态信息已初始化
    if (InitTboxStaticInfo() != RET_OK)
    {
        SR_TSP_LOG_E("InitTboxStaticInfo failed");
        return RET_ERR;
    }

    // 2. 获取最新的监控数据
    MonitoringData_t rawData;
    if (GetMonitoringData(&rawData) != RET_OK)
    {
        SR_TSP_LOG_E("Failed to get monitoring data");
        return RET_ERR;
    }

    // 3. 构建 S3Connect 消息并填充
    net::sirun::tboxs3::connect::S3Connect connectMsg;
    EncodeS3Connect(&connectMsg, rawData.tboxDbInfo);

    // 4. 序列化
    if (!connectMsg.SerializeToString(&buffer))
    {
        SR_TSP_LOG_E("Failed to serialize S3Connect message");
        return RET_ERR;
    }

    SR_TSP_LOG_I("S3Connect serialized, size = %zu bytes", buffer.size());
    return RET_OK;
}

/**
 * @brief 构造 MQTT 消息对象
 *
 * @param[out] msg MQTT 消息对象
 * @param[in]  buffer 已序列化的 payload 数据
 * @return true  构造成功
 * @return false 构造失败（如 payload 过大）
 */
static int32_t BuildLoginMessage(SRMqttMessage_t &msg, const std::string &buffer)
{
    if (buffer.size() > sizeof(msg.payload))
    {
        SR_TSP_LOG_E("Payload size too large: %zu bytes", buffer.size());
        return RET_ERR;
    }

    char snBuffer[64] = {0};
    TspBaseInfoGetSn(snBuffer, sizeof(snBuffer));

    memcpy(msg.payload, buffer.data(), buffer.size());
    msg.payloadLen = buffer.size();
    snprintf(msg.topicName, sizeof(msg.topicName), "%s%s", SR_TOPIC_PREFIX_T2S_CONNECT_TBOX, snBuffer);
    msg.topicLen = strlen(msg.topicName);

    return RET_OK;
}

/**
 * @brief 发送 MQTT 消息并接收响应
 *
 * @param[in]  reqMsg 发送的请求消息
 * @param[out] rspMsg 接收到的响应消息
 * @return true  发送并接收成功
 * @return false 失败
 */
static int32_t SendAndReceiveMqttMessage(SRMqttMessage_t &reqMsg, SRMqttMessage_t &rspMsg)
{
    if (SRTspServiceExecute(&reqMsg, &rspMsg, 5000) != RET_OK)
    {
        SR_TSP_LOG_E("Service execution failed");
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @brief 解析 ConnectAck 响应消息
 *
 * @param[in] rspMsg 接收到的响应消息
 * @return true  解析成功
 * @return false 解析失败
 */
static int32_t ParseConnectAckWithHandler(const SRMqttMessage_t &rspMsg)
{
    bool isConnect = (strncmp(rspMsg.topicName, SR_TOPIC_PREFIX_S2T_CONNECT_PLATFORM,
                              strlen(SR_TOPIC_PREFIX_S2T_CONNECT_PLATFORM)) == 0);
    if (!isConnect)
    {
        LOG_Error("ConnectAck unexpected topicName:%s", rspMsg.topicName);
        return RET_ERR;
    }

    net::sirun::tboxs3::connect::ConnectAck ackMsg;
    if (!ackMsg.ParseFromArray(rspMsg.payload, rspMsg.payloadLen))
    {
        SR_TSP_LOG_E("Failed to parse ConnectAck message");
        return RET_ERR;
    }
    int32_t encryptType = static_cast<int32_t>(ackMsg.encrypt_type());
    const std::string &ivStr = ackMsg.iv();
    SR_TSP_LOG_I("ConnectAck received: encrypt_type=%d, iv=%s",
                 encryptType, ivStr.c_str());

    /* ---- 建立加密会话 ---- */
    if (encryptType != static_cast<int32_t>(::net::sirun::tboxs3::connect::DataEncryption::NON))
    {
        /* 1) 解析 IV */
        // 根据加密类型确定 iv 和 key 的长度
        size_t expectedLen;
        std::string defaultKey;
        switch (encryptType)
        {
        case static_cast<int32_t>(::net::sirun::tboxs3::connect::DataEncryption::DES):
            expectedLen = 8U; // DES 块大小 64 位
            defaultKey = "SirunAsm";
            break;
        case static_cast<int32_t>(::net::sirun::tboxs3::connect::DataEncryption::AES):
        case static_cast<int32_t>(::net::sirun::tboxs3::connect::DataEncryption::SM4):
            expectedLen = 16U; // AES/SM4 块大小 128 位
            defaultKey = "SirunAsmsKey2026";
            break;
        default:
            SR_TSP_LOG_E("ConnectAck unsupported encryptType=%d", encryptType);
            return RET_ERR;
        }

        uint8_t ivBytes[16] = {0};
        if (ivStr.size() >= expectedLen)
        {
            memcpy(ivBytes, ivStr.data(), expectedLen);
        }
        else
        {
            SR_TSP_LOG_E("ConnectAck IV parse failed: encryptType=%d, ivSize=%zu, expectedLen=%zu",
                         encryptType, ivStr.size(), expectedLen);
            return RET_ERR;
        }

        /* 2) 密钥key：优先从本地配置下的 monitor_config.json 文件中加载 */
        uint8_t sessionKey[16] = {0};
        bool keyLoaded = false;
        {
            MonitorConfigPath cfgPath = {0};
            MonitorConfig_Get(&cfgPath);
            char configPath[256] = {0};
            snprintf(configPath, sizeof(configPath), "%smonitor_config.json", cfgPath.taskConfigFilePath);

            Json::Value root;
            Json::Reader reader;
            std::ifstream ifs(configPath);
            if (ifs.is_open() && reader.parse(ifs, root, false))
            {
                std::string keyStr = root.get("key", "").asString();
                if (keyStr.size() >= expectedLen)
                {
                    memcpy(sessionKey, keyStr.data(), expectedLen);
                    keyLoaded = true;
                }
                else
                {
                    SR_TSP_LOG_E("monitor_config.json 'key' too short: %zu (need %d)",
                                 keyStr.size(), expectedLen);
                }
            }
            else
            {
                SR_TSP_LOG_E("Failed to load monitor_config.json from %s", configPath);
            }
        }

        if (!keyLoaded)
        {
            // 配置加载失败或文件不存在，使用默认密钥
            memcpy(sessionKey, defaultKey.c_str(), defaultKey.size());
            SR_TSP_LOG_I("Encryption key not available, using default key:%s", defaultKey.c_str());
        }

        /* 3) 保存加密配置 */
        EncryptionSetSession(encryptType, ivBytes, sessionKey);
        SR_TSP_LOG_I("Encryption session established: type=%d", encryptType);
    }
    else
    {
        SR_TSP_LOG_I("ConnectAck: encryption not enabled (NON)");
    }

    return RET_OK;
}

/**
 * @brief 主入口：执行 TSP 平台登录流程
 *
 * @return int RET_OK 成功, RET_ERR 失败
 */
int SRTspPlatformLogin(void)
{
    // GOOGLE_PROTOBUF_VERIFY_VERSION;

    std::string buffer;
    if (SerializeS3ConnectMessage(buffer) != RET_OK)
    {
        return RET_ERR;
    }

    SRMqttMessage_t reqMsg = SRMqttMessage_initializer;
    if (BuildLoginMessage(reqMsg, buffer) != RET_OK)
    {
        return RET_ERR;
    }

#ifndef TEST_NOACK
    SRMqttMessage_t rspMsg = SRMqttMessage_initializer;
    if (SendAndReceiveMqttMessage(reqMsg, rspMsg) != RET_OK)
    {
        return RET_ERR;
    }

    if (ParseConnectAckWithHandler(rspMsg) != RET_OK)
    {
        return RET_ERR;
    }
#else
    SRMqttMessage_t rspMsg = SRMqttMessage_initializer;
    SendAndReceiveMqttMessage(reqMsg, rspMsg);
#endif
    SR_TSP_LOG_I("TSP platform login successful");
    return RET_OK;
}
