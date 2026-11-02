# otamodem1 - Modem A OTA升级模块

## 功能说明

otamodem1实现了完整的HTTP监听、升级包接收、解压校验和多线程并行升级功能。主要功能包括：

### 1. HTTP监听服务
- 监听端口：8080
- 协议：TCP Socket
- 支持阻塞和非阻塞模式

### 2. 升级包接收流程

#### 2.1 接收文件大小
支持两种协议格式：
- **旧协议**：仅接收8字节的double类型数据（单位：MB）
- **新协议**：接收8字节MB + 1字节type + 8字节bytes（单位：字节）

程序会自动检测协议类型并正确解析文件大小。

#### 2.2 检查磁盘空间
- 清理旧的升级文件
- 处理日志文件释放空间
- 验证可用空间是否满足要求

#### 2.3 接收升级包内容
- 使用20KB缓冲区接收数据
- 实时写入文件到 `/oemdata/upgrade/update.zip`
- 检测"end"标记判断传输结束
- 验证实际接收大小与期望大小是否匹配
- 防止误判：如果数据中包含"end"字符串但大小不匹配，继续接收

#### 2.4 解压升级包并校验MD5 ⭐ 
接收完成后自动执行以下操作：

**解压流程：**
1. 清理旧的升级文件（md5.txt, mcu_1.bin, mcu_2.bin, update.zip）
2. 执行 `tar -zxvf` 命令解压升级包
3. 最多重试3次，确保解压成功
4. 验证四个关键文件是否存在：
   - `md5.txt` - MD5校验文件
   - `mcu_1.bin` - MCU升级文件1
   - `mcu_2.bin` - MCU升级文件2
   - `update.zip` - MPU升级包

**MD5校验流程：**
1. 读取 `md5.txt` 文件中对应行的MD5值
2. 计算每个文件的实际MD5值
3. 对比预期MD5和实际MD5
4. 校验失败则返回错误状态

**关键文件说明：**
```
/oemdata/upgrade/
├── md5.txt          # MD5校验文件（每行一个文件的MD5值）
│   Line 1: MCU1 BIN的MD5
│   Line 2: MCU2 BIN的MD5
│   Line 3: Tbox (update.zip) 的MD5
├── mcu_1.bin        # MCU升级文件1
├── mcu_2.bin        # MCU升级文件2
└── update.zip       # MPU升级包
```

#### 2.5 多线程并行升级 ⭐ 
解压校验完成后，启动MCU和MPU的并行升级：

**升级架构：**
- **MCU升级线程**：独立线程执行MCU固件升级
- **MPU升级线程**：独立线程执行MPU系统升级
- **协调器机制**：使用互斥锁和条件变量同步两个线程
- **统一结果上报**：等待两个线程都完成后统一发送升级结果

**并行升级流程：**
1. 创建MCU升级线程（`mcu_upgrade_thread`）
2. 创建MPU升级线程（`mpu_upgrade_thread`）
3. 两个线程同时执行升级操作
4. 主线程监控升级状态（15分钟超时）
5. 等待两个线程都完成
6. 检查升级结果
7. 统一发送升级结果给客户端

**MCU升级特性：**
- 最多重试3次
- 每次重试间隔5秒
- 5分钟超时保护
- 状态监控：UPGRADE_MCU_SUCCESS / UPGRADE_MCU_FAILED

**MPU升级特性：**
- 单次执行（无重试）
- 10分钟超时保护
- 状态监控：UPGRADE_MPU_NEED_BACKUP / UPGRADE_MPU_FAILED
- 进度上报：65% → 100%

**时间统计：**
- 记录解压耗时
- 记录MCU升级耗时
- 记录MPU升级耗时
- 输出完整的时间报告

#### 2.6 等待下一步指令 ⭐ 新增
发送升级结果后，等待客户端发送下一步指令：

**支持的指令：**
1. **reboot / reboot_begin** - 重启设备
   - 调用 `SrTboxSystemReboot()` 执行重启
   - 重启类型为 REMOTE_CONTROL_REBOOT
   
2. **switch_partition / switch_partition_begin** - 切换分区
   - 调用 `OtaSwitchAOrBPartition2()` 执行分区切换
   - A/B分区互换

**等待机制：**
- 超时时间：**120秒**
- 使用select非阻塞等待，1秒轮询一次
- 收到有效指令后立即执行
- 收到未知指令则继续等待
- 超时后自动关闭连接

**执行流程：**
```
发送升级结果
    ↓
等待指令（最多120秒）
    ├─→ 收到"reboot" → 执行重启
    ├─→ 收到"switch_partition" → 切换分区
    ├─→ 收到其他指令 → 忽略并继续等待
    └─→ 120秒超时 → 关闭连接
```

#### 2.7 发送升级结果
- 成功：发送 "Upgrade Success"
- 失败：发送 "Upgrade Fail"
- **统一在并行升级完成后发送**
- **发送后等待下一步指令** ⭐ 新增

### 3. 状态管理
升级状态定义：
- 0: UPGRADE_STATE_IDLE - 空闲
- 1: UPGRADE_STATE_RUNNING - 升级进行中
- 2: UPGRADE_STATE_SUCCESS - 升级成功
- 3: UPGRADE_STATE_FAILED - 升级失败
- 4: UPGRADE_STATE_BACKUP - 备份状态

状态写入文件：`/oemdata/parameters/SlaveModemUpgradeStatus`

### 4. 文件结构

```
otamodem1/
├── inc/                          # 头文件目录
│   ├── otamodem1.h              # 主头文件，定义常量
│   ├── file_interface.h         # 文件操作接口
│   ├── log_interface.h          # 日志处理接口
│   ├── mtd_interface.h          # 存储空间管理接口
│   ├── system_interface.h       # 系统命令接口
│   ├── upgrade_interface.h      # 升级接口定义（含多线程）⭐ 增强
│   └── md5_interface.h          # MD5校验接口
├── src/                          # 源文件目录
│   ├── main.cpp                 # 主入口（保留）
│   ├── otamodem1.cpp            # 主逻辑实现
│   ├── file_interface.cpp       # 文件操作实现
│   ├── log_interface.cpp        # 日志处理实现
│   ├── mtd_interface.cpp        # 存储空间管理实现
│   ├── system_interface.cpp     # 系统命令实现
│   ├── upgrade_interface.cpp    # 升级功能实现（含多线程并行升级）⭐ 增强
│   └── md5_interface.cpp        # MD5校验实现
└── CMakeLists.txt               # 编译配置
```

### 5. 关键路径

- 升级目录：`/oemdata/upgrade/`
- 升级文件：`/oemdata/upgrade/update.zip`
- MD5文件：`/oemdata/upgrade/md5.txt`
- MCU文件1：`/oemdata/upgrade/mcu_1.bin`
- MCU文件2：`/oemdata/upgrade/mcu_2.bin`
- 进度文件：`/oemdata/parameters/SlaveModemUpProgress`
- 状态文件：`/oemdata/parameters/SlaveModemUpgradeStatus`

### 6. 编译和运行

```bash
# 编译
cd app/build
cmake ..
make modem_a

# 运行
./modem_a
```

### 7. 注意事项

1. 程序启动时会检查是否存在EMMC设备节点（`/dev/mmcblk0`），如果存在则认为是从模块，会阻塞进程
2. 接收文件时会进行大小验证，确保接收的数据完整
3. 支持断点续传的误判处理（数据中包含"end"字符串的情况）
4. 每次接收前会清理旧的升级文件和日志文件
5. 连接超时设置为10秒，避免长时间等待
6. **解压支持最多3次重试**，确保解压成功率
7. **MD5校验严格验证**，任何一个文件校验失败都会导致升级失败
8. 需要链接OpenSSL库（libcrypto, libssl）以支持MD5计算
9. **MCU和MPU并行升级**，提高升级效率 ⭐ 新增
10. **统一结果上报**，等待所有模块升级完成后统一发送结果 ⭐ 新增
11. **完整的超时保护**：MCU 5分钟，MPU 10分钟，总监控15分钟

### 8. 核心函数说明

#### ota_unzip_file()
```c
int ota_unzip_file(char *upgradefilepath);
```
- **功能**：解压升级包并校验文件完整性
- **参数**：upgradefilepath - 升级包路径
- **返回值**：成功返回0，失败返回-1
- **流程**：
  1. 清理旧文件
  2. 执行tar解压（最多3次重试）
  3. 验证关键文件存在性
  4. 删除压缩包
  5. MD5校验所有文件

#### DoUpgrade() ⭐ 新增
```c
int DoUpgrade();
```
- **功能**：执行MCU和MPU的并行升级
- **返回值**：成功返回0，失败返回-1
- **流程**：
  1. 初始化升级协调器
  2. 创建MCU升级线程
  3. 创建MPU升级线程
  4. 监控升级状态（15分钟超时）
  5. 等待两个线程完成
  6. 检查升级结果
  7. 输出时间统计报告
  8. 返回整体升级结果

#### LordMcuUpgrade() ⭐ 新增
```c
int LordMcuUpgrade();
```
- **功能**：执行MCU升级（带重试机制）
- **返回值**：成功返回0，失败返回-1
- **特性**：
  - 最多重试3次
  - 5秒重试间隔
  - 5分钟超时保护
  - 状态轮询（3秒间隔）

#### LordMpuUpgrade() ⭐ 新增
```c
int LordMpuUpgrade();
```
- **功能**：执行MPU升级
- **返回值**：成功返回0，失败返回-1
- **特性**：
  - 单次执行
  - 10分钟超时保护
  - 状态轮询（3秒间隔）
  - 进度上报

### 9. 多线程架构 ⭐ 新增

#### 升级协调器（UpgradeCoordinator）
```c
typedef struct {
    int mcuCompleted;      // MCU是否完成
    int mpuCompleted;      // MPU是否完成
    int mcuResult;         // MCU升级结果
    int mpuResult;         // MPU升级结果
    pthread_mutex_t statusMutex;           // 互斥锁
    pthread_cond_t allCompleteCond;        // 条件变量
} UpgradeCoordinator;
```

#### 时间统计（UpgradeTiming）
```c
typedef struct {
    struct timeval unzipStart, unzipEnd;
    double unzipDuration;     // 解压耗时
    
    struct timeval mcuStart, mcuEnd;
    double mcuDuration;       // MCU升级耗时
    
    struct timeval mpuStart, mpuEnd;
    double mpuDuration;       // MPU升级耗时
} UpgradeTiming;
```

#### 线程函数
- `mcu_upgrade_thread()` - MCU升级线程入口
- `mpu_upgrade_thread()` - MPU升级线程入口

### 10. 与otamodem2的区别

- otamodem1 (modem_a)：主模块，负责接收升级包、解压、MD5校验、**并行升级**
- otamodem2 (modem_b)：从模块，负责执行MPU升级和分区切换

当前实现完成了完整的升级流程：接收 → 解压 → 校验 → **并行升级** → 统一结果上报

### 11. 错误处理

| 错误场景 | 处理方式 |
|---------|---------|
| 解压失败 | 最多重试3次，每次重试前清理文件 |
| 文件缺失 | 立即返回错误，不进行MD5校验 |
| MD5不匹配 | 返回错误码-3，记录详细日志 |
| 磁盘空间不足 | 在接收前检查，拒绝接收 |
| 网络中断 | 检测到连接关闭，返回失败状态 |
| MCU升级失败 | 最多重试3次，失败后继续等待MPU完成 |
| MPU升级失败 | 记录错误，等待MCU完成后统一返回失败 |
| 升级超时 | MCU 5分钟，MPU 10分钟，总监控15分钟 |

### 12. 性能优化

- 解压过程记录时间统计
- MD5计算使用1KB缓冲区，平衡内存和速度
- 文件校验并行度：串行校验（保证顺序和可靠性）
- 清理操作使用sync确保数据落盘
- **MCU和MPU并行升级**，缩短总体升级时间 ⭐
- **非阻塞监控**，使用条件变量同步线程 ⭐
- **完整的时间统计**，便于性能分析 ⭐

### 13. 升级流程图 ⭐ 

```
接收文件完成
    ↓
解压升级包（最多3次重试）
    ↓
MD5校验（MCU1, MCU2, Tbox）
    ↓
【新增】创建MCU升级线程
    ↓
【新增】创建MPU升级线程
    ↓
【新增】并行执行升级
    ├─→ MCU升级（最多重试3次，5分钟超时）
    └─→ MPU升级（10分钟超时）
    ↓
【新增】等待两个线程完成（15分钟总超时）
    ↓
【新增】检查升级结果
    ├─→ 全部成功 → 发送"Upgrade Success"
    └─→ 任一失败 → 发送"Upgrade Fail"
    ↓
【新增】等待下一步指令（120秒超时）⭐
    ├─→ 收到"reboot" → 执行重启
    ├─→ 收到"switch_partition" → 切换分区
    ├─→ 收到其他指令 → 忽略并继续等待
    └─→ 120秒超时 → 关闭连接
    ↓
关闭连接，继续监听
```
