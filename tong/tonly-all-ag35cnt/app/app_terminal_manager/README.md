# Terminal Manager — 终端管理服务

## 1. 概述

终端管理服务（Terminal Manager）是 TBOX 车联网终端的核心通信模块，负责终端与 TSP（Telematics Service Platform）平台之间的双向 MQTT 通信。服务基于 MQTT over TLS 协议，支持设备认证登录、状态监控与数据采集、事件上报、远程命令执行、日志采集、OTA 升级、远程 SSH 等功能。

- **作者**: SiRun (HeFei)
- **语言**: C / C++
- **构建系统**: CMake ≥ 3.0
- **MQTT 协议**: MQTT 3.1.1 over SSL/TLS
- **序列化**: Protocol Buffers (protobuf)

---

## 2. 目录结构

```
app_terminal_manager/
├── CMakeLists.txt                          # 顶层 CMake 构建文件
├── .gitlab-ci.yml                          # GitLab CI 配置
├── cmake/
│   └── build_env.cmake                     # 构建环境配置
│
├── terminal_management/                    # 核心库 (lib)
│   ├── config/                             # 运行时配置文件
│   ├── inc/
│   │   ├── public/                         # 对外公开 API 头文件
│   │   │   ├── terminal_management_api.h           # 主接口：初始化/启动/停止
│   │   │   ├── terminal_management_data.h          # 基础数据结构 & 回调类型定义
│   │   │   ├── terminal_management_data_encode.h   # Protobuf 编码接口
│   │   │   ├── terminal_management_data_monitor.h  # 监控数据采集
│   │   │   ├── terminal_management_data_packag.h   # 数据打包 & 发送队列
│   │   │   ├── terminal_management_data_report.h   # 数据上报处理
│   │   │   └── terminal_management_data_response.h # 命令响应处理
│   │   └── private/                        # 内部私有头文件
│   │       ├── srTspCallbackRegister.h             # 设备信息回调注册
│   │       ├── srTspEncryptAndDecrypt.h            # 加密/解密 (DES/AES/SM4)
│   │       ├── srTspLogin.h                        # 平台登录
│   │       ├── srTspPlatformInterface.h            # 状态机接口
│   │       └── srTspRemoteProcess.h                # 远程命令处理
│   ├── src/                                # 业务层实现
│   │   ├── srTspCallbackRegister.c
│   │   ├── srTspEncryptAndDecrypt.cpp
│   │   ├── srTspLogin.cpp
│   │   ├── srTspRemoteProcess.cpp
│   │   ├── srTspStateMachine.c                     # TSP 状态机
│   │   ├── terminal_management_api.cpp
│   │   ├── terminal_management_data_encode.cpp
│   │   ├── terminal_management_data_monitor.cpp
│   │   ├── terminal_management_data_packag.cpp
│   │   ├── terminal_management_data_report.cpp
│   │   └── terminal_management_data_response.cpp
│   ├── tspFW/                              # TSP 框架层 (MQTT 通信基础设施)
│   │   ├── inc/
│   │   │   ├── srTspFWCaCertHandler.h              # CA 证书处理
│   │   │   ├── srTspFWInterface.h                  # 框架接口汇总 & 状态枚举
│   │   │   ├── srTspFWLog.h                        # 日志
│   │   │   ├── srTspFWMqttClient.h                 # MQTT 客户端封装
│   │   │   ├── srTspFWMqttMsg.h                    # MQTT 消息结构
│   │   │   ├── srTspFWOs.h                         # OS 抽象层 (线程/队列/互斥锁)
│   │   │   ├── srTspFWProtocol.h                   # Topic 定义 & 协议注册
│   │   │   ├── srTspFWService.h                    # 服务层 (ACKQueue/ReplyQueue)
│   │   │   └── srTspFwPlatform.h                   # 平台连接封装
│   │   └── src/
│   │       ├── srTspCaCertHandler.c
│   │       ├── srTspFWMqttClient.c
│   │       ├── srTspFWMqttMsg.c
│   │       ├── srTspFWOs.c
│   │       ├── srTspFWPlatform.c
│   │       ├── srTspFWProtocol.c
│   │       └── srTspFWService.c
│   └── tspProto/                           # Protobuf 协议定义
│       ├── inc/
│       │   ├── command.pb.h                        # 远程命令
│       │   ├── common.pb.h                         # 公共定义
│       │   ├── config.pb.h                         # 配置
│       │   ├── connect.pb.h                        # 连接/登录
│       │   ├── device_status.pb.h                  # 设备状态
│       │   ├── event_report.pb.h                   # 事件上报
│       │   ├── filemanage.pb.h                     # 文件管理
│       │   ├── report.pb.h                         # 数据上报
│       │   ├── rssh.pb.h                           # 远程 SSH
│       │   └── upgrade.pb.h                        # OTA 升级
│       └── src/                                    # protobuf 源文件
│
└── terminal_management_main/               # 主程序入口
    └── src/
        └── terminal_management_main.cpp            # main() 启动流程
```

---

## 3. 架构设计

### 3.1 三层架构

```
┌─────────────────────────────────────────────────────┐
│               terminal_management_main              │  ← 应用层
│   (main 入口, 配置初始化, 回调注册, 模块启动)         │
├─────────────────────────────────────────────────────┤
│              terminal_management                    │  ← 业务层
│   (状态机, 登录, 远程控制, 数据采集, 上报, 加密)      │
├─────────────────────────────────────────────────────┤
│                    tspFW                            │  ← 框架层
│   (MQTT 客户端, 消息收发, 队列管理, TLS, OS 抽象)    │
└─────────────────────────────────────────────────────┘
```

### 3.2 MQTT Topic 设计

| 方向 | Topic 前缀 | 用途 |
|---|---|---|
| **上行** (TBOX→平台) | `t2s/proto/v1/connect/` | 设备登录 |
| 上行 | `t2s/proto/v1/report/` | 数据上报 |
| 上行 | `t2s/proto/v1/event/` | 事件上报 |
| 上行 | `t2s/proto/v1/reissue/` | 数据补发 |
| 上行 | `t2s/proto/v1/command/` | 命令执行结果 |
| **下行** (平台→TBOX) | `s2t/proto/v1/connect/` | 登录应答 |
| 下行 | `s2t/proto/v1/report/` | 上报应答 |
| 下行 | `s2t/proto/v1/reissue/` | 补发应答 |
| 下行 | `s2t/proto/v1/command/` | 远程命令下发 |

Topic 格式为 `{prefix}{sn}`，其中 `{sn}` 为设备唯一序列号。

### 3.3 TSP 状态机

```
       ┌──────────┐
       │   INIT   │  初始化
       └────┬─────┘
            ↓
     ┌────────────┐
     │ CONNECTING │  MQTT Broker 连接中
     └─────┬──────┘
           ↓ (成功)          ↑
     ┌────────────┐          │
     │ CONNECTED  │          │ 重连等待超时
     └─────┬──────┘          │
           ↓            ┌────┴──────────┐
     ┌────────────┐     │ RECONNECT_WAIT│  等待重连
     │ SUBSCRIBED │     └───────────────┘
     └─────┬──────┘          ↑
           ↓                 │
     ┌──────────┐            │
     │  LOGIN   │  平台登录  │
     └─────┬────┘            │
       ↓ (成功)    ↓ (失败)   │
     ┌──────┐  ┌────────┐    │
     │ RECV │  │ LOGOUT │    │
     └──┬───┘  └───┬────┘    │
        │          ↓         │
        │      ┌────────┐    │
        │      │ ERROR  │────┘
        │      └────────┘
        │
        └── 正常接收/处理数据 (主循环)
```

状态说明：

| 状态 | 说明 |
|---|---|
| `INIT` | 初始化 MQTT 客户端 |
| `CONNECTING` | 连接 Broker（SSL） |
| `CONNECTED` | TCP/TLS 已连接 |
| `SUBSCRIBED` | 订阅下行 Topic 完成 |
| `LOGIN` | 发送登录报文，等待平台认证 |
| `LOGOUT` | 登录失败，进入错误 |
| `RECV` | 正常通信（接收 command + 处理上报应答） |
| `ERROR` | 通信异常，断开连接 |
| `RECONNECT_WAIT` | 等待 5 秒后重连 |
| `DEINIT` | 状态机退出 |

---

## 4. 核心模块

### 4.1 tspFW — 框架层

框架层封装 MQTT 通信基础设施，提供 OS 抽象能力。

| 模块 | 文件 | 功能 |
|---|---|---|
| **MQTT Client** | `srTspFWMqttClient.[ch]` | 基于 Paho MQTT 库封装，提供 connect/publish/subscribe |
| **Platform** | `srTspFWPlatform.[ch]` | 平台连接管理，封装 Broker 连接/断开/发送/状态 |
| **Protocol** | `srTspFWProtocol.[ch]` | Topic 路由表，将下行消息按 Topic 分发到对应处理器 |
| **Service** | `srTspFWService.[ch]` | 队列管理：ACKQueue (同步应答) + ReplyQueue (异步接收) |
| **OS** | `srTspFWOs.[ch]` | OS 抽象层：线程、队列、互斥锁、定时器 |
| **Msg** | `srTspFWMqttMsg.[ch]` | MQTT 消息结构体定义与操作 |
| **CA Cert** | `srTspFWCaCertHandler.[ch]` | TLS 证书管理 |
| **Log** | `srTspFWLog.h` | 日志宏定义 |

#### 队列机制

```
ACKQueue (同步应答队列)
┌──────────────────────────────────────────────────────────┐
│  生产者: SRMqttProtocolSyncRecvResponseHandle()           │
│          → SRTspServiceActivateAck()                      │
│  消费者: SRTspServiceExecute()  阻塞等待 + 超时重试       │
│  处理消息: connect / report / reissue 的平台应答           │
└──────────────────────────────────────────────────────────┘

ReplyQueue (异步接收队列)
┌──────────────────────────────────────────────────────────┐
│  生产者: SRMqttProtocolAsyncRecvResponseHandle()          │
│          → SRTspServiceActivateReply()                    │
│  消费者: SRTspServiceRecData()    状态机被动轮询接收       │
│  处理消息: command 平台主动下发的远程命令                  │
└──────────────────────────────────────────────────────────┘
```

| 队列 | 模式 | 处理的消息 | 生产函数 | 消费函数 |
|---|---|---|---|---|
| **ACKQueue** | 同步请求-响应 | connect / report / reissue 应答 | `SRTspServiceActivateAck()` | `SRTspServiceExecute()` |
| **ReplyQueue** | 异步推送接收 | command 命令下发 | `SRTspServiceActivateReply()` | `SRTspServiceRecData()` |

### 4.2 业务层

| 模块 | 文件 | 功能 |
|---|---|---|
| **State Machine** | `srTspStateMachine.c` | TSP 状态机主循环，驱动连接/登录/重连流程 |
| **Login** | `srTspLogin.[cpp/h]` | 平台登录认证，含加密协商 |
| **Remote Process** | `srTspRemoteProcess.[cpp/h]` | 远程命令解析与分发（重启/日志/SSH/升级等） |
| **Callback Register** | `srTspCallbackRegister.[c/h]` | 设备基础信息回调注册（SN/IMEI/VIN 等） |
| **Encrypt** | `srTspEncryptAndDecrypt.[cpp/h]` | 报文加解密（DES-CBC / AES-128-CBC / SM4-CBC） |
| **API** | `terminal_management_api.[cpp/h]` | 对外主接口 |
| **Data Monitor** | `terminal_management_data_monitor.[cpp/h]` | 车况数据采集与事件监控 |
| **Data Packag** | `terminal_management_data_packag.[cpp/h]` | 数据打包与发送队列管理 |
| **Data Report** | `terminal_management_data_report.[cpp/h]` | 上行数据处理线程（从队列取数据→上报） |
| **Data Encode** | `terminal_management_data_encode.[cpp/h]` | Protobuf 序列化编码 |
| **Data Response** | `terminal_management_data_response.[cpp/h]` | 命令执行结果响应 |

#### 发送队列（优先级调度）

```
优先级 高 → 低:
  P0: 故障类事件 (触发/恢复)
  P1: 异常类事件 (触发/恢复)
  P2: 正常类事件
  P3: 周期数据
  P4: 补发数据
```

数据发送时按优先级从 P0→P4 依次消费，确保故障类紧急数据优先上报。

---

## 5. 对外接口

### 5.1 初始化与生命周期

```c
// 配置地址初始化
int ConfigAddressInit(const char *dataFilePath,
                      const char *taskConfigFilePath,
                      const char *crtStorePath,
                      const char *downloadFolder,
                      const char *unzipFolder);

// 注册设备基础信息回调
int RegisterQueryBaseInfoCallback(const StBaseInfoCallbacks_t *callback);

// 启动 MQTT 客户端 (创建状态机线程)
int TspClientStart(void);

// 停止终端管理服务
int ServiceStop(void);
```

### 5.2 设备信息回调结构体

```c
typedef struct {
    QueryBaseInfoCallbackFunc getSn;                     // SN 序列号
    QueryBaseInfoCallbackFunc getSim;                    // SIM 卡号
    QueryBaseInfoCallbackFunc getImei;                   // IMEI
    QueryBaseInfoCallbackFunc getIccid;                  // ICCID
    QueryBaseInfoCallbackFunc getProjectName;            // 项目名称
    QueryBaseInfoCallbackFunc getVin;                    // VIN 码
    QueryBaseInfoCallbackFunc getHwVersion;              // 硬件版本
    QueryBaseInfoCallbackFunc getSwVersionMain;          // 主软件版本
    QueryBaseInfoCallbackFunc getSwVersionSecondary;     // 次软件版本
    QueryBaseInfoCallbackFunc getSwVersionConfiguration; // 配置版本
} StBaseInfoCallbacks_t;
```

### 5.3 数据上报

```c
// 启动数据组包线程
int DataPackagingStart(void);

// 启动数据处理线程
int DataProcessingStart(void);
```

### 5.4 加密

| 类型 | 值 | 算法 |
|---|---|---|
| `NON` | 0 | 不加密 |
| `DES` | 1 | DES-CBC |
| `AES` | 2 | AES-128-CBC |
| `SM4` | 3 | SM4-CBC |

```c
void EncryptionSetSession(int32_t encryptType, const uint8_t iv[16], const uint8_t key[16]);
int EncryptPayload(const uint8_t *plaintext, size_t plainLen, uint8_t **output);
int DecryptPayload(const uint8_t *ciphertext, size_t cipherLen, uint8_t **output);
```

---

## 6. 启动流程

`main()` 函数（`terminal_management_main.cpp`）的启动步骤：

```
Step 1: ConfigAddressInit()
        → 初始化数据/配置/证书/下载/解压路径

Step 2: RegisterQueryBaseInfoCallback()
        → 注册 10 个设备基础信息回调函数
          (SN, SIM, IMEI, ICCID, ProjectName, VIN, HW, SW×3)

Step 3: InitTriggerEventCallback() + InitSignalIpcCallback()
        → 初始化事件触发回调 & IPC 信号监听

Step 4: DataPackagingStart()
        → 启动周期数据组包线程 (10s 间隔)

Step 5: TspClientStart()
        → 创建状态机线程，启动 MQTT 连接流程

Step 6: DataProcessingStart()
        → 启动上下行数据处理线程

Step 7: while(1) { sleep(25); }
        → 主线程保活
```

---

## 7. 数据流

### 7.1 上行数据流（TBOX → 平台）

```
┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│ Monitor 模块  │───→│ Packag 模块  │───→│  Send Queue  │───→│ MQTT Publish │
│ (数据采集)    │    │ (编码/打包)   │    │ (P0→P4 优先级)│    │ (发送)       │
└──────────────┘    └──────────────┘    └──────────────┘    └──────┬───────┘
                                                                    │
                                                                    ↓
                                                             ┌──────────────┐
                                                             │  TSP 平台    │
                                                             └──────┬───────┘
                                                                    │ 应答
                                                                    ↓
                                                             ┌──────────────┐
                                                             │  ACKQueue    │
                                                             │ (同步等待)    │
                                                             └──────────────┘
```

### 7.2 下行数据流（平台 → TBOX）

```
┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│  TSP 平台    │───→│ MQTT Callback│───→│  Protocol    │───→│ ReplyQueue   │
│ (主动下发)    │    │ (消息到达)    │    │ (Topic 路由)  │    │ (异步入队)   │
└──────────────┘    └──────────────┘    └──────────────┘    └──────┬───────┘
                                                                    │
                                                                    ↓
                                                             ┌──────────────┐
                                                             │ StateMachine │
                                                             │ (RECV 状态)  │
                                                             │ SRTspService │
                                                             │ RecData()    │
                                                             └──────┬───────┘
                                                                    │
                                                                    ↓
                                                             ┌──────────────┐
                                                             │ TspRemote    │
                                                             │ Process()    │
                                                             │ (命令执行)    │
                                                             └──────────────┘
```

---

## 8. 支持的远程命令

通过 `command.pb.h` 协议定义的远程命令类型：

| 命令 | 说明 |
|---|---|
| 远程重启 | MCU 重启 / CPU 重启 / TBOX 整机重启 |
| 日志采集 | 远程触发日志上传至平台 |
| OTA 升级 | 固件远程升级 |
| 远程 SSH | 远程调试登录 |
| 文件管理 | 远程文件操作 |
| 配置下发 | 远程参数配置 |

---

## 9. 构建

### 9.1 依赖

| 依赖 | 用途 |
|---|---|
| Eclipse Paho MQTT C | MQTT 客户端 |
| Protocol Buffers | 消息序列化 |
| OpenSSL | TLS 加密通信 |
| SQLite3 | 本地数据缓存 |
| GLib | 基础数据结构 |
| POSIX Threads | 多线程 |

### 9.2 编译

```bash
mkdir build && cd build
cmake ..
make
```

---

## 10. 配置参数

| 参数 | 默认值 | 说明 |
|---|---|---|
| `SR_TSP_SEND_RETRY_COUNT` | 3 | ACKQueue 同步等待重试次数 |
| `RECONNECT_INTERVAL_SEC` | 5 | 断线重连间隔 (秒) |
| `HEARTBEAT_INTERVAL_SEC` | 30 | 心跳间隔 (秒) |
| `INTERVAL_MS` | 1000 | 数据采集更新间隔 (毫秒) |
| `PACKAGE_INTERVAL_MS` | 10000 | 周期数据组包间隔 (毫秒) |
| `MAX_RECORDS_PER_REISSUE_DB` | 5000 | 补发数据库单表最大记录数 |
| `MAX_REISSUE_BACKUP_TOTAL_BYTES` | 500MB | 补发备份总容量上限 |
| `MAX_HISTORY_FAULT_BACKUP_TOTAL_BYTES` | 100MB | 历史故障备份总容量上限 |

### Broker 地址

```c
static const char *g_broker = "ssl://121.40.204.137:8883";
```

---

## 11. 数据缓存策略

- **补发数据库**: 上报失败的数据持久化到本地 SQLite，断网恢复后自动补发（优先级 P4）
- **历史故障数据库**: 故障事件记录到独立数据库，用于故障溯源
- **容量保护**: 补发备份最大 500MB，历史故障备份最大 100MB，超出容量限制后自动清理
