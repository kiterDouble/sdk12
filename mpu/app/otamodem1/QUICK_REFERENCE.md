# otamodem1 快速参考

## 📌 核心功能
✅ HTTP监听（端口8080）  
✅ 接收升级包大小  
✅ 接收升级包内容  
✅ 磁盘空间检查  
✅ 解压升级包  
✅ MD5校验  
✅ **多线程并行升级** ⭐ 新增  
✅ 升级状态管理  

## 📁 关键文件

### 主逻辑
- **otamodem1.cpp** - HTTP监听和数据接收主逻辑

### 接口层
- **file_interface.cpp** - 文件读写操作
- **log_interface.cpp** - 日志清理和管理
- **mtd_interface.cpp** - 磁盘空间管理
- **system_interface.cpp** - 系统命令执行
- **upgrade_interface.cpp** - 升级功能（含多线程并行升级）⭐ 增强
- **md5_interface.cpp** - MD5计算和校验

## 🔑 关键函数

```cpp
// 主循环
main() 
  ↓
WaitForSocketConnection()    // 等待连接（10秒超时）
  ↓
ReceiveFileSize()            // 接收文件大小
  ↓
CheckDiskSpace()             // 检查磁盘空间
  ↓
ReceiveAndWriteFile()        // 接收文件内容
  ↓
ota_unzip_file()             // 解压并校验
  ↓
【新增】DoUpgrade()          // 并行升级（MCU + MPU）
  ↓
SendUpgradeResult()          // 统一发送结果
```

### 解压和校验函数

```cpp
// 解压升级包（最多重试3次）
int ota_unzip_file(char *upgradefilepath);

// MD5校验单个文件
int VerifyFileMd5(const char* filepath, const char* filename, int lineNum);

// 计算文件MD5
int CalculateMd5(const char *filename, unsigned char output[MD5_DIGEST_LENGTH]);

// 十六进制转字节
void HexStringToBytes(const char *hex_string, unsigned char *bytes, size_t length);
```

### 并行升级函数 ⭐ 

```cpp
// 并行升级协调器（主入口）
int DoUpgrade();

// MCU升级线程函数
void* mcu_upgrade_thread(void* arg);

// MPU升级线程函数
void* mpu_upgrade_thread(void* arg);

// MCU升级执行（带重试）
int LordMcuUpgrade();

// MPU升级执行
int LordMpuUpgrade();

// 等待下一步指令（重启或切换分区）
int WaitForNextCommand(int socket_fd, int timeout_seconds);
```

## 📊 状态定义
```
0 - IDLE     (空闲)
1 - RUNNING  (升级中)
2 - SUCCESS  (成功)
3 - FAILED   (失败)
4 - BACKUP   (备份)
```

## 📂 关键路径
```
/oemdata/upgrade/update.zip              # 升级文件
/oemdata/upgrade/md5.txt                 # MD5校验文件
/oemdata/upgrade/mcu_1.bin               # MCU升级文件1
/oemdata/upgrade/mcu_2.bin               # MCU升级文件2
/oemdata/parameters/SlaveModemUpProgress # 进度
/oemdata/parameters/SlaveModemUpgradeStatus # 状态
```

## 🔧 编译运行
```bash
cd app/build
cmake ..
make modem_a
./modem_a
```

**依赖库**: libcrypto, libssl (OpenSSL), pthread

## 🧪 测试脚本
见 `TEST.md` 中的Python测试脚本

## ⚙️ 配置常量
``c
PORT:           8080
BUFFER_SIZE:    20480 (20KB)
MAX_RETRY:      3 (解压重试次数)
MCU_MAX_RETRY:  3 (MCU升级重试次数)
MCU_TIMEOUT:    300秒 (5分钟)
MPU_TIMEOUT:    600秒 (10分钟)
TOTAL_TIMEOUT:  900秒 (15分钟)
COMMAND_TIMEOUT: 120秒 (等待指令超时) ⭐ 新增
```

## 📝 协议格式

### 旧协议
```
[8 bytes: double MB]
```

### 新协议
```
[8 bytes: double MB][1 byte: type][8 bytes: double bytes]
```

### 控制指令协议 ⭐ 新增
```
发送升级结果后，客户端可发送以下指令：

重启指令:
"reboot" 或 "reboot_begin"

分区切换指令:
"switch_partition" 或 "switch_partition_begin"

响应:
- 收到有效指令 → 立即执行
- 收到未知指令 → 忽略并继续等待
- 120秒无指令 → 超时关闭连接
```

## 🔐 MD5校验流程

```
md5.txt 文件格式:
Line 1: MCU1 BIN的MD5值 (32字符十六进制)
Line 2: MCU2 BIN的MD5值 (32字符十六进制)
Line 3: Tbox (update.zip) 的MD5值 (32字符十六进制)

校验步骤:
1. 读取对应行的MD5字符串
2. 转换为字节数组 (HexStringToBytes)
3. 计算文件实际MD5 (CalculateMd5)
4. 比较两个MD5值 (memcmp)
5. 不匹配返回-3
```

## 🗜️ 解压流程

```
1. 清理旧文件 (ota_delete_file)
2. 执行 tar -zxvf (最多3次重试)
3. 验证文件存在性:
   ✓ md5.txt
   ✓ mcu_1.bin
   ✓ mcu_2.bin
   ✓ update.zip
4. 删除压缩包
5. MD5校验所有文件
6. 成功返回0，失败返回-1或-3
```

## 🚀 并行升级流程 ⭐ 

```
DoUpgrade() 被调用
    ↓
初始化升级协调器
    ↓
创建MCU升级线程 ──→ mcu_upgrade_thread()
    │                    ↓
    │              LordMcuUpgrade()
    │                    ↓
    │              最多重试3次
    │                    ↓
    │              5分钟超时保护
    │                    ↓
    │              标记完成 + 广播
    │
创建MPU升级线程 ──→ mpu_upgrade_thread()
    │                    ↓
    │              LordMpuUpgrade()
    │                    ↓
    │              单次执行
    │                    ↓
    │              10分钟超时保护
    │                    ↓
    │              标记完成 + 广播
    ↓
主线程监控（15分钟总超时）
    ↓
等待两个线程都完成
    ↓
检查升级结果
    ├─→ MCU成功 && MPU成功 → 返回0
    └─→ 任一失败 → 返回-1
    ↓
输出时间统计报告
```

## 📡 等待指令流程 ⭐ 新增

```
发送升级结果
    ↓
WaitForNextCommand(socket_fd, 120)
    ↓
使用select非阻塞等待（1秒轮询）
    ↓
120秒内收到指令？
    ├─→ 是
    │   ├─→ "reboot" / "reboot_begin"
    │   │   └─→ SrTboxSystemReboot() → 重启设备
    │   ├─→ "switch_partition" / "switch_partition_begin"
    │   │   └─→ OtaSwitchAOrBPartition2() → 切换分区
    │   └─→ 其他指令
    │       └─→ 忽略，继续等待
    └─→ 否（120秒超时）
        └─→ 关闭连接，返回监听状态
```

## 📈 性能对比 ⭐ 新增

### 串行升级
```
MCU (3分钟) → MPU (5分钟) = 8分钟
```

### 并行升级
```
MCU (3分钟) ↘
              → 5分钟（节省37.5%）
MPU (5分钟) ↗
```

## 🔒 线程安全 ⭐ 新增

```c
// 升级协调器
typedef struct {
    int mcuCompleted;      // MCU是否完成
    int mpuCompleted;      // MPU是否完成
    int mcuResult;         // MCU升级结果
    int mpuResult;         // MPU升级结果
    pthread_mutex_t statusMutex;           // 互斥锁
    pthread_cond_t allCompleteCond;        // 条件变量
} UpgradeCoordinator;

// 使用方式
pthread_mutex_lock(&g_coordinator.statusMutex);
// 访问共享数据
pthread_mutex_unlock(&g_coordinator.statusMutex);
pthread_cond_broadcast(&g_coordinator.allCompleteCond);
```

## 🚀 下一步扩展
- [ ] 分区切换 (OtaSwitchAOrBPartition)
- [ ] 重启控制 (SrTboxSystemReboot)
- [ ] 进度上报 (MQTT/CAN)
- [ ] 更多模块并行支持

---
*参考: otamodem2实现 + ethernet_upgrade的ota_unzip_file和多线程升级*
