# ota\_master SDK 开发指导说明

版本历史

|  **版本**  |  **日期**  |  **修改说明**  |
| --- | --- | --- |
|  1.0  |   2025-04-07   |  初版  |
| 1.1 | 2025-07-16 | 1、新增TBox回滚通知回调函数<br />2、StOtaTaskPolicyEcuInfo新增供应商ID信息字段<br />3、StOtaTaskPolicyInfo新增整车版本号字段<br />4、StEcuVersionInfo新增供应商ID、零件好信息字段<br />5、示例代码优化 |

---

# 1. 概述

## 1.1 功能说明

本文档用于指导开发者在C/C++程序中如何使用libota\_master.so对车身零部件固件进行升级；

*   **安全校验：**集成AES128 ECB加解密和MD5校验机制；
    
*   **断点续传：**支持固件下载中断后恢复；
    
*   **升级模式：**支持标准升级、静默升级；
    
*   **可配置项**：支持配置前置条件、可升级ECU信息配置；

## 1.2 目标读者

第三方开发者、系统集成人员

## 1.3 软件依赖

|  **编号**  |  **依赖**  |  **版本要求**  |  **用途**  |
| --- | --- | --- | --- |
|  1  |  jsoncpp  |  v1.y.z  |  json文件解析  |
|  2  |  openssl   |  v1.1及以上  |  AES128加解密  |
|  3  |  curl  |  v7.7及以上  |  文件下载、上传  |
|  4  |  zlib  |  v1.2.12及以上  |  解压缩文件  |

## 1.4 参考文档

|  **编号**  |  **文档名**  |  **文档说明**  |  **版本**  |
| --- | --- | --- | --- |
|  1  |  CV01项目\_OTA开发需求规范V1.5-20241230.docx  |  功能需求规范  |  v1.5  |
|  2  |  菱电TSP平台与TBOX的OTA交互协议\_V3.5\_20250226\_待评审7-多拉.docx  |  平台与T-Box OTA交互协议  |  v3.5  |
|  3  |  QDLE J02.012-2024 BootloaderOnCAN刷写需求规范.pdf  |  CAN刷写规范  |   |

## 1.5 术语定义

|  **英文缩写**  |  **英文愿意**  |  **中文含义**  |
| --- | --- | --- |
|  T-Box  |  Telematics BOX  |  车载终端  |
|  OTA平台  |  OTA Telematics Service Provider  |  OTA远程监控平台  |
|  APP  |  T-Box OTA Master APP  |  T-Box终端接入libota\_master SDK的APP  |
|  SDK  |  libota\_master  |  OTA  Master SDK  |
|  OBD  |  On Board Diagnostics  |  车载诊断系统  |
|  ECU  |  Electronic Control Unit  |  电子控制单元  |

# 2 系统概述

## 2.1 系统框图

<img src="https://alidocs.oss-cn-zhangjiakou.aliyuncs.com/a/RODKALagxHmn7q2J/5aeaeb6a7ce247c38148db0053d0ad8b2439.png" alt="image" style="zoom: 33%;" />

## 2.2 标准升级时序概述

<img src="https://alidocs.oss-cn-zhangjiakou.aliyuncs.com/a/RODKALagxHmn7q2J/7db5358f32564b0eafbb505fc70768102439.png" alt="image" style="zoom:33%;" />

# 3 配置文件管理

## 3.1 文件基础属性

*   文件类型：JSON结构化配置文件（支持版本化管理）
    
*   存储位置：由应用层决定
    
*   访问权限：644（root只读，应用层可读不可写）
    
*   更新机制：通过TBox终端固件烧录工具强制写入或者OTA升级方式写入
    
*   文件命名：配置文件名固定，不可变更，具体命名见3.2、3.3章节
    

## 3.2 升级前置条件配置信息

配置文件名：ota\_preconditions\_config.json；

[警告]**文件名不可更改；**

### 3.2.1 功能定义

定义进入刷写模式前必须满足的复合条件检测策略，采用多级与/或逻辑组合：

|  **属性名**  |  **子属性**  |  **类型**  |  **说明**  |  **备注**  |
| --- | --- | --- | --- | --- |
|  battery\_voltage  |  enabled  |  bool  |  是否启用该前置条件  |  FOTA升级时要求当前蓄电池电压值  |
|  |  description  |  string  |  条件说明  ||
|  |  min\_voltage  |  float  |  蓄电池电压最低要求  ||
|  gear\_position  |  enabled  |  bool  |  是否启用该前置条件  |  FOTA升级时要求当前档位为P挡  |
|  |  description  |  string  |  条件说明  ||
|  |  required\_gear  |  int  |  要求档位  ||
|  vehicle\_speed  |  enabled  |  bool  |  是否启用该前置条件  |  FOTA升级时要求车速为0km/h  |
|  |  description  |  string  |  条件说明  ||
|  |  required\_speed  |  int  |  要求车速  ||
|  vcu_hv_not_ready  |  enabled  |  bool  |  是否启用该前置条件  |  "要求动力系统就绪状态为未就绪 (Not Ready);"  |
|  |  description  |  string  |  条件说明  ||
|  bms\_real\_soc  |  enabled  |  bool  |  是否启用该前置条件  |  FOTA升级时要求动力电池大于阈值  |
|  |  description  |  string  |  条件说明  ||
|  |  min\_soc  |  int  |  SOC最低要求  ||
|  required\_vehicle\_no\_fault  |  enabled  |  bool  |  是否启用该前置条件  |  FOTA升级时要求整车故障状态为无故障  |
|  |  description  |  string  |  条件说明  ||
|  required\_plug\_off  |  enabled  |  bool  |  是否启用该前置条件  |  FOTA升级时要求充电枪的状态为“未连接”  |
|  |  description  |  string  |  条件说明  ||
|  |  required\_plug\_sts  |  int  |  要求的充电枪状态  ||
|  required\_no\_remote\_ctrl\_executing  |  enabled  |  bool  |  是否启用该前置条件  |  FOTA升级时要求无远程指令正在执行  |
|  |  description  |  string  |  条件说明  ||
|  no_obd_diagnostic_dev_conn  |  enabled  |  bool  |  是否启用该前置条件  |  FOTA升级时要求OBD诊断口应无诊断设备连接运行  |
|  |  description  |  string  |  条件说明  ||
|  bcm\_anti\_theft\_auth\_passed  |  enabled  |  bool  |  是否启用该前置条件  |  需通过BCM防盗认证  |
|  |  description  |  string  |  条件说明  ||

### 3.2.2 文件内容

```shell
{
    "battery_voltage": {
        "enabled": true,
        "description": "蓄电池电压要求: U >= 12.0V",
        "min_voltage": 12.0
    },
 
    "gear_position": {
        "enabled": true,
        "description": "要求当前档位为P挡,value = 0",
        "required_gear": 0
    },
 
    "vehicle_speed": {
        "enabled": true,
        "description": "车速要求为 0 km/h",
        "required_speed": 0
    },
 
    "vcu_hv_not_ready": {
        "enabled": true,
        "description": "要求动力系统就绪状态为未就绪 (Not Ready);"
    },
 
    "bms_real_soc": {
        "enabled": true,
        "description": "动力电池 SOC 大于 25%",
        "min_soc": 25
    },
 
    "required_vehicle_no_fault": {
        "enabled": false,
        "description": "整车故障要求为无故障 (VCU_VehicleFault_Sts = 0x0)"
    },
 
    "required_plug_off": {
        "enabled": true,
        "description": "充电枪连接状态要求为未连接"
    },
 
    "required_no_remote_ctrl_executing": {
        "enabled": true,
        "description": "无远程指令正在执行"
    },
 
    "no_obd_diagnostic_dev_conn": {
        "enabled": true,
        "description": "OBD诊断口应无诊断设备连接运行"
    },
 
    "bcm_anti_theft_auth_passed": {
        "enabled": true,
        "description": "需通过BCM防盗认证"
    }
}
```

## 3.3 可OTA的ECU配置

配置文件名：support\_ota\_ecu\_config.json；

[警告]**文件名不可更改；**

### 3.3.1 功能定义

用于配置车辆中支持通过T-Box进行OTA升级的ECU关键信息，包括零部件名，升级的CAN通道，升级时的电源模式（高压、低压），物理寻址ID，回滚方式等;

|  **属性名**  |  **类型**  |  **约束**  |  **说明**  |
| --- | --- | --- | --- |
|  name  |  string  |  不能为空  |  ECU零部件名称  |
|  ecu\_id  |  string  |  不能为空  |  ECU ID  |
|  ecu\_type  |  int  |  不能为空  |  ECU类型：   0：T-Box；   1：BCM； 2：BMS   3：VCU；   4：其他ECU  |
|  can\_port  |  int  |  不能为空  |  CAN端口  |
|  physical\_id  |  string  |  不能为空  |  物理寻址ID；  |
|  function\_id  |  string  |  不能为空  |  功能寻址ID；  |
|  response\_id  |  string  |  不能为空  |  应答ID；  |
|  power\_mode  |  int  |  0或1  |  0：低压升级 1：高压升级  |
| rollback | / | / | 回滚方式配置字段 |
| supplier_id | string | 不能为空 | 供应商ID，根据供应商ID配置回滚方式 |
| succ_roll_back_mode | int | 0或1 | 组合升级过程该件升级成功，后续件升级失败，该件回滚方式：<br/>0：由Tbox刷写回滚<br />1：命令通知自行回滚 |
| failed_roll_back_mode | int | 0或1 | 该件升级过程失败，回滚方式：<br/>0：由Tbox刷写回滚<br/>1：ECU失败自行回滚 |
| rollback_timeout | int | 不为0 | ECU回滚的超时时间 |

### 3.3.1 文件内容

```shell
{
    "fota_ecu": [
        {
            "name": "TBOX",
            "ecu_id": "0x703",
            "ecu_type": 0,
            "can_port": 0,
            "physical_id": "0x0703",
            "function_id": "0x07DF",
            "response_id": "0x0783",
            "power_mode": 1,
            "rollback": [
                {
                    "supplier_id": "DL03005",
                    "succ_roll_back_mode" : 0,
                    "failed_roll_back_mode" : 2,
                    "rollback_timeout": 900
                }
            ]
        },

        {
            "name": "BCM",
            "ecu_id": "0x727",
            "ecu_type": 1,
            "can_port": 1,
            "physical_id": "0x0727",
            "function_id": "0x07DF",
            "response_id": "0x07A7",
            "power_mode": 0,
            "rollback": [
                {
                    "supplier_id": "DL03024",
                    "succ_roll_back_mode" : 0,
                    "failed_roll_back_mode" : 0,
                    "rollback_timeout": 300
                }
            ]
        },

        {
            "name": "BMS",
            "ecu_id": "0x715",
            "ecu_type": 2,
            "can_port": 2,
            "physical_id": "0x0715",
            "function_id": "0x07DF",
            "response_id": "0x0795",
            "power_mode": 0,
            "rollback": [
                {
                    "supplier_id": "DL01002",
                    "succ_roll_back_mode" : 1,
                    "failed_roll_back_mode" : 2,
                    "rollback_timeout": 300
                },
 
                {
                    "supplier_id": "DL01003",
                    "succ_roll_back_mode" : 0,
                    "failed_roll_back_mode" : 2,
                    "rollback_timeout": 300
                },
 
                {
                    "supplier_id": "DL01004",
                    "succ_roll_back_mode" : 0,
                    "failed_roll_back_mode" : 0,
                    "rollback_timeout": 300
                }
            ]
        },

        {
            "name": "VCU",
            "ecu_id": "0x7E2",
            "ecu_type": 3,
            "can_port": 2,
            "physical_id": "0x07E2",
            "function_id": "0x07DF",
            "response_id": "0x07EA",
            "power_mode": 0,
            "rollback": [
                {
                    "supplier_id": "DL30002",
                    "succ_roll_back_mode" : 1,
                    "failed_roll_back_mode" : 2,
                    "rollback_timeout": 300
                }
            ]
        },

        {
            "name": "FLR",
            "ecu_id": "0x732",
            "ecu_type": 4,
            "can_port": 2,
            "physical_id": "0x0732",
            "function_id": "0x07DF",
            "response_id": "0x07B2",
            "power_mode": 1,
            "rollback": [
                {
                    "supplier_id": "DL03022",
                    "succ_roll_back_mode" : 0,
                    "failed_roll_back_mode" : 0,
                    "rollback_timeout": 300
                }
            ]
        },

        {
            "name": "IPK",
            "ecu_id": "0x725",
            "ecu_type": 4,
            "can_port": 3,
            "physical_id": "0x0725",
            "function_id": "0x07DF",
            "response_id": "0x07A5",
            "power_mode": 1,
            "rollback": [
                {
                    "supplier_id": "DL03004",
                    "succ_roll_back_mode" : 0,
                    "failed_roll_back_mode" : 2,
                    "rollback_timeout": 1800
                }
            ]
        },

        {
            "name": "IPU",
            "ecu_id": "0x716",
            "ecu_type": 4,
            "can_port": 2,
            "physical_id": "0x0716",
            "function_id": "0x07DF",
            "response_id": "0x0796",
            "power_mode": 0,
            "rollback_timeout": 300,
            "rollback": [
                {
                    "supplier_id": "DL01001",
                    "succ_roll_back_mode" : 0,
                    "failed_roll_back_mode" : 2,
                    "rollback_timeout": 300
                }
            ]
        },

        {
            "name": "ADCU",
            "ecu_id": "0x731",
            "ecu_type": 4,
            "can_port": 3,
            "physical_id": "0x0731",
            "function_id": "0x07DF",
            "response_id": "0x07B1",
            "power_mode": 1,
            "rollback": [
                {
                    "supplier_id": "DL03022",
                    "succ_roll_back_mode" : 0,
                    "failed_roll_back_mode" : 2,
                    "rollback_timeout": 1200
                }
            ]
        }
    ]
}
```

# 4 API接口说明

## 4.1 头文件

libota\_master API头文件为：ota\_master\_api.h和ota\_master\_log.h；

## 4.2 函数概览

|  **编号**  |  **函数**  |  **说明**  |
| --- | --- | --- |
|  1  |  OtaMasterInit  |  SDK初始化  |
|  2  |  OtaMasterDeinit  |  SDK去初始化  |
|  3  |  OtaMasterRegisterReportCallback  |  结果上报回调接口注册  |
|  4  |  OtaMasterUnregisterReportCallback  |  结果上报回调函数去注册  |
|  5  |  OtaMasterRegisterQueryCallback  |  信息查询回调函数注册  |
|  6  |  OtaMasterUnregisterQueryCallback  |  信息查询回调函数去注册  |
|  7  |  OtaMasterRegisterRequestCallback  |  请求类型回调函数注册  |
|  8  |  OtaMasterUnregisterRequestCallback  |  请求类型回调函数去注册  |
|  9  |  OtaMasterGetOtaStatus  |  获取OTA状态  |
|  10  |  OtaMasterStart  |  启动OTA Master  |
|  11  |  OtaMasterStop  |  停止OTA Master  |
|  12  |  OtaMasterStartOtaTask  |  开始OTA任务  |
|  13  |  OtaMasterClearOtaTask  |  清空OTA任务  |
|  14  |  OtaMasterConfirmStartUpgrading  |  确认开始执行升级  |
|  15  |  OtaMasterAES128ECBEncrypt  |  AES128 ECB加密  |
|  16  |  OtaMasterAES128ECBDecrypt  |  AES128 ECB解密  |
|  17  |  OtaMasterNoticeMcuUdsDataAck  |  通知MasterSDK MCU收到MPU发送的UDS数据后的ACK数据  |
|  18  |  OtaMasterCheckMD5  |  MD5校验  |
|  19  |  OtaMasterNoticeEcuUdsData  |  APP通知SDK：UDS诊断数据  |
|  20  |  OtaMasterNoticeEcuVersionAsyncQueryResult  |  APP通知SDK：ECU版本异步查询结果  |
|  21  |  OtaMasterNoticeUpgradePreconditionsAsyncQueryResult  |  APP通知SDK：升级前置条件异步查询结果  |
|  22  |  OtaMasterNoticeTboxRollbackResult  |  APP通知SDK：TBox的版本回滚结果  |
|  23  |  OtaMasterNoticeTboxSelfUpgradeResult  |  APP通知SDK：TBox的版本自升级结果  |
|  24  |  OtaMasterNoticeReportDownloadResultHasACK  |  APP通知SDK：OTA平台接收到下载结果并应答  |
|  25  |  OtaMasterNoticeReportUpgradeResultHasACK  |  APP通知SDK：OTA平台接收到升级结果并应答  |
|  26  |  OtaMasterNoticeUpDownHighVoltageResult  |  APP通知SDK：上下高压请求结果  |
|  27  |  OtaMasterNoticeFotaModeOnOffSwResult  |  APP通知SDK：FOTA模式切换结果（禁止电源模式切换结果）  |
|  28  |  OtaMasterLogInit  |  Log初始化  |
|  29  |  OtaMasterLogFinish  |  结束OTA Master日志  |
|  30  |  OtaMasterLogReset  |  备份OTA Master日志文件并重置当前日志  |
|  31  |  OtaMasterUploadLog  |  上传OTA Master日志；  |
|  32  |  OtaMasterLogPrint  |  日志输出  |

## 4.3 函数详解

### 4.3.1 OtaMasterInit

该函数用于初始化SDK

*   **函数原型**
    

```c
int OtaMasterInit(const char *configFilePath, const char *downloadFolder, const char *unzipFolder)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  configFilePath  |  const char \*  |  IN  |  配置文件路径  |
|  downloadFolder  |  const char \*  |  IN  |  升级文件下载存储目录  |
|  unzipFolder  |  const char \*  |  IN  |  升级文件解压目录  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  初始化成功  |
|  OTA\_RET\_ERR  |  初始化失败  |

### 4.3.2 OtaMasterDeinit

该函数用于去初始化SDK

**函数原型**

```c
void OtaMasterDeinit(void)
```

### 4.3.3 OtaMasterRegisterReportCallback

该函数用于注册结果上报回调接口

*   **函数原型**
    

```c
int OtaMasterRegisterReportCallback(const StOtaMasterReportCallback *callback)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  callback  |  const StOtaMasterReportCallback \*  |  IN  |  结果上报回调函数集合；详见**：****5.9章节**  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  注册成功  |
|  OTA\_RET\_ERR  |  注册失败  |

### 4.3.4 OtaMasterUnregisterReportCallback

该函数用于去注册结果上报回调函数，对应**章节：4.3.3 OtaMasterRegisterReportCallback**

*   **函数原型**
    

```c
void OtaMasterUnregisterReportCallback(void)
```

### 4.3.5 OtaMasterRegisterQueryCallback

该函数用于注册信息查询回调函数

*   **函数原型**

```c
int OtaMasterRegisterQueryCallback(const StOtaMasterQueryCallback *callback)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  callback  |  const StOtaMasterQueryCallback \*  |  IN  |  信息查询回调函数集合；详见：**5.10章节**  |

*   **返回值说明**

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  注册成功  |
|  OTA\_RET\_ERR  |  注册失败  |

### 4.3.6 OtaMasterUnregisterQueryCallback

该函数用于去注册信息查询回调函数，对应**章节：****4.3.5 OtaMasterRegisterQueryCallback**

*   **函数原型**
    

```c
void OtaMasterUnregisterQueryCallback(void)
```

### 4.3.7 OtaMasterRegisterRequestCallback

该函数用于注册请求类型回调函数

*   **函数原型**
    

```c
int OtaMasterRegisterRequestCallback(const StOtaMasterRequestCallback *callback)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  callback  |  const StOtaMasterRequestCallback \*  |  IN  |  请求类型回调函数集合；详见**：5.11章节**  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  注册成功  |
|  OTA\_RET\_ERR  |  注册失败  |

### 4.3.8 OtaMasterUnregisterRequestCallback

该函数用于去注册请求类型回调函数，对应**：****4.3.7 OtaMasterRegisterRequestCallback****章节**

*   **函数原型**
    

```c
void OtaMasterUnregisterRequestCallback(void)
```

### 4.3.9 OtaMasterGetOtaStatus

该函数用于获取当前OTA状态

*   **函数原型**
    

```c
OtaMasterStatus OtaMasterGetOtaStatus(void)
```

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OtaMasterStatus  |  OTA Master状态，详见**：4.3.9.1章节**  |

#### 4.3.9.1 OtaMasterStatus

*   **定义**
    

```c
typedef enum {
    OTA_NOT_READY = 0,   // OTA未准备，初始化中
    OTA_IN_PROGRESS = 1, // OTA中
    OTA_READY = 2,       // OTA准备好，可OTA
} OtaMasterStatus;
```

*   **枚举说明**
    

|  **枚举定义**  |  **值**  |  **说明**  |
| --- | --- | --- |
|  OTA\_NOT\_READY  |  0  |  OTA未准备  |
|  OTA\_IN\_PROGRESS  |  1  |  OTA中  |
|  OTA\_READY  |  2  |  OTA准备好，可OTA  |

### 4.3.10 OtaMasterStart

该函数用于启动OTA Master功能，主要启动SDK内部状态机，完成任务自检；若有遗留任务（OTA过程TBox断电、OTA App崩溃、TBox自升级完成自重启完成分区切换）未完成，SDK会自动恢复任务并继续执行；建议在KL15 ON后调用改接口用于启动OTA Master功能；

*   **函数原型**

```c
int OtaMasterStart(void)
```

*   **返回值说明**

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  OTA Master启动成功  |
|  OTA\_RET\_ERR  |  OTA Master启动失败  |

### 4.3.11 OtaMasterStop

该函数用于停止OTA Master功能；

若SDK处于非刷写状态中则可正常停止；若SDK正在下载升级件过程，会中断下载并正常结束SDK状态机；下一次KL15 ON时再调用OtaMasterStart后会恢复之前中断的下载动作，且支持断点续传；

[警告]若TBox正在执行自升级或者正在刷写其他ECU，则无法正常停止OTA Master；OTA App应阻止TBox进入休眠并等待升级/回滚动作执行完成后方可再次调用该接口；

建议KL15 OFF后调用此接口关闭OTA Master功能；

*   **函数原型**

```c
int OtaMasterStop(void)
```

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  OTA Master停止成功  |
|  OTA\_RET\_ERR  |  OTA Master停止失败  |

### 4.3.12 OtaMasterStartOtaTask

该函数用于启动OTA任务；OTA Master APP在接收到OTA平台下发的OTA任务并将任务信息转换为SDK要求的策略信息结构，通过调用此接口启动OTA任务；

*   **函数原型**
    

```c
int OtaMasterStartOtaTask(const StOtaTaskPolicyInfo *policyInfo)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  policyInfo  |  const StOtaTaskPolicyInfo \*  |  IN  |  OTA任务策略信息；详见**：5.5章节**  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  OTA 任务启动成功  |
|  OTA\_RET\_ERR  |  OTA 任务启动失败  |

### 4.3.13 OtaMasterClearOtaTask

该函数用于清除OTA任务；

标准升级模式下，SDK下载完升级件后回调通知OTA App上报下载成功结果到OTA 平台，OTA 平台在接收到下载成功结果后通过APP或小程序提醒用户有升级任务待升级；若客户点击取消升级或者超时未操作，OTA平台下发清除/取消任务指令到TBox OTA APP，OTA  App通过调用该接口通知SDK清除任务；

OTA Master清除完下载的升级件、重置升级状态与任务配置信息后，通过**：5.9.4章节回调函数**回调通知OTA APP上报清除结果到OTA平台；

*   **函数原型**
    

```c
int OtaMasterClearOtaTask(void)
```

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  OTA 任务清除成功  |
|  OTA\_RET\_ERR  |  OTA 任务清除失败  |

### 4.3.14 OtaMasterConfirmStartUpgrading

该函数用于确认开始执行升级；标准升级模式下，OTA升级文件下载成功并成功上报下载结果到平台，平台下发确认升级指令后通过此接口通知SDK开始执行刷写；

*   **函数原型**
    

```c
int OtaMasterConfirmStartUpgrading(const char *taskId)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  taskId  |  const char \*  |  IN  |  任务ID  |

### 4.3.15 OtaMasterAES128ECBEncrypt

该函数用于AES128 ECB加密

*   **函数原型**
    

```c
int OtaMasterAES128ECBEncrypt(const uint8_t *key, int keyLen, const uint8_t *input, int inputLen, uint8_t **output)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  key  |  const uint8\_t \*  |  IN  |  加密KEY  |
|  keyLen  |  int  |  IN  |  加密KEY长度  |
|  input  |  const uint8\_t \*  |  IN  |  加密源数据  |
|  inputLen  |  int  |  IN  |  加密源数据长度  |
|  output  |  uint8\_t \*\*  |  OUT  |  加密后输出数据；内存由API内部分配，加密成功后，内存由调用者自行通过free方式释放；  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  0  |  加密失败  |
|  \>0  |  加密成功  |

### 4.3.16 OtaMasterAES128ECBDecrypt

该函数用于AES128 ECB解密

*   **函数原型**
    

```c
int OtaMasterAES128ECBDecrypt(const uint8_t *key, int keyLen, const uint8_t *input, int inputLen, uint8_t **output)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  key  |  const uint8\_t \*  |  IN  |  加密KEY  |
|  keyLen  |  int  |  IN  |  加密KEY长度  |
|  input  |  const uint8\_t \*  |  IN  |  加密源数据  |
|  inputLen  |  int  |  IN  |  加密源数据长度  |
|  output  |  uint8\_t \*\*  |  OUT  |  解密后输出数据；内存由API内部分配，解密成功后，内存由调用者自行通过free方式释放；  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  0  |  解密失败  |
|  \>0  |  解密成功  |

### 4.3.17 OtaMasterNoticeMcuUdsDataAck

该函数用于通知libota\_master MCU收到MPU发送的UDS数据后的ACK数据

*   **函数原型**
    

```c
/**
 * MCU回复ACK数据格式
 * 
 * +-----------------------------------------------------------------------------------------------------------------+
 * | 字节索引 | 字段名称             | 值范围/类型         | 描述                                                         |
 * |----------|--------------------|-------------------|-------------------------------------------------------------|
 * | [0]      | 协议类型标识         | 0x00              | 不使用。                                                     |
 * | [1]      | ACK类型标识         | 0x00               | 不使用。                                                     |
 * | [2]      | 计数器索引           | 0x00~0xFF (uint8) | 必须与SendUdsDataCallbackFunc中的data[2]一致，用于匹配请求与响应。|
 * +-----------------------------------------------------------------------------------------------------------------+
 * 
 * @note 数据总长度固定为3字节。
 * @example MCU ACK示例：
 *   buffer = {0x00, 0x00, 0x5A}; // 确认接收counterIdx=0x5A的报文
 */
void OtaMasterNoticeMcuUdsDataAck(const uint8_t *data, uint16_t dataLen)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  data  |  const uint8\_t \*  |  IN  |  数据地址  |
|  dataLen  |  uint16\_t  |  IN  |  数据长度  |

### 4.3.18 OtaMasterCheckMD5

该函数用于文件MD5校验

*   **函数原型**
    

```c
int OtaMasterCheckMD5(const char *filePath, const char *targetMD5)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  filePath  |  const char \*  |  IN  |  文件路径  |
|  targetMD5  |  const char \*  |  IN  |  目标MD5  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  MD5校验成功  |
|  OTA\_RET\_ERR  |  MD5校验失败  |

### 4.3.19 OtaMasterNoticeEcuUdsData

该函数用于通知libota\_master UDS数据；libota\_master在刷写其他ECU件时，APP将接收到的MCU透传来的UDS数据传给libota\_master；

*   **函数原型**
    

```c
/**
 * ECU响应报文数据格式
 * 
 * +--------------------------------------------------------------------------------------------------------------------+
 * | 字节索引 | 字段名称             | 值范围/类型        | 描述                                                             |
 * |----------|--------------------|-------------------|----------------------------------------------------------------|
 * | [0]      | 协议类型标识         | 0x00              | 标识为ECU响应协议。                                               |
 * | [1]      | 发送模式标志         | 0x00 或 0x01       | 0x00: 首次响应；0x01: 重复响应（如超时重传）。                       |
 * | [2]      | 计数器索引           | 0x00~0xFF (uint8) | 必须与SendUdsDataCallbackFunc中的data[2]一致，用于匹配请求与响应。   |
 * | [3]-[6]  | 响应CAN ID          | uint8_t[4]        | ECU的响应ID。                                                   |
 * | [7]-N    | 报文载荷             | uint8_t[]         | ECU返回的实际数据。                                              |
 * +--------------------------------------------------------------------------------------------------------------------+
 * 
 * @example ECU响应示例：
 *   buffer = {0x00, 0x00, 0x5A, 0x00,0x00,0x07,0xA7,0x05,0xAA,0xBB,0xCC};
 */
void OtaMasterNoticeEcuUdsData(const uint8_t *data, uint16_t dataLen)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  data  |  const uint8\_t   |  IN  |  数据地址  |
|  dataLen  |  uint16\_t  |  IN  |  数据长度  |

### 4.3.20 OtaMasterNoticeEcuVersionAsyncQueryResult

该函数用于通知libota\_master ECU版本异步查询结果；

*   **函数原型**
    

```c
void OtaMasterNoticeEcuVersionAsyncQueryResult(const StEcuVersionInfo *ecuVersions, uint8_t ecuCnt)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  ecuVersions  |  const StEcuVersionInfo \*  |  IN  |  ECU版本信息；详见：**5.7章节**  |
|  ecuCnt  |  uint8\_t  |  IN  |  ECU数  |

### 4.3.21 OtaMasterNoticeUpgradePreconditionsAsyncQueryResult

该函数用于通知libota\_master 升级前置条件异步查询结果；

*   **函数原型**
    

```c
void OtaMasterNoticeUpgradePreconditionsAsyncQueryResult(const StUpgradePreconditions *preconditons)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  preconditons  |  const StUpgradePreconditions \*  |  IN  |  升级前置条件查询结果；详见：**5.8章节**  |

### 4.3.22 OtaMasterNoticeTboxRollbackResult

该函数用于通知libota\_master TBox版本回滚结果

*   **函数原型**
    

```c
void OtaMasterNoticeTboxRollbackResult(int result, OtaMasterTboxRebootState rebootState)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  result  |  int  |  IN  |  回滚结果：OTA\_RET\_OK：回滚成功；OTA\_RET\_ERR：回滚失败  |
| rebootState | OtaMasterTboxRebootState | IN | 若TBox回滚成功，告知Master SDK是否需要重启TBox |

### 4.3.23 OtaMasterNoticeTboxSelfUpgradeResult

该函数用于通知libota\_master TBox自升级结果；

*   **函数原型**

```c
void OtaMasterNoticeTboxSelfUpgradeResult(int result, OtaMasterTboxRebootState rebootState)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  result  |  int  |  IN  |  升级结果：OTA\_RET\_OK：升级成功；OTA\_RET\_ERR：升级失败  |
|  rebootState  |  OtaMasterTboxRebootState  |  IN  |  若TBox升级成功，告知Master SDK是否需要重启TBox；详见：**4.3.23.1章节**  |

#### 4.3.23.1 OtaMasterTboxRebootState

*   **定义**

```c
typedef enum {
    OTA_TBOX_NO_REBOOT = 0,   // TBox不重启
    OTA_TBOX_NEED_REBOOT = 1, // TBox需要重启
} OtaMasterTboxRebootState;
```

*   **枚举说明**
    

|  **枚举定义**  |  **值**  |  **说明**  |
| --- | --- | --- |
|  OTA\_TBOX\_NO\_REBOOT  |  0  |  TBox不重启  |
|  OTA\_TBOX\_NEED\_REBOOT  |  1  |  TBox需要重启  |

### 4.3.24 OtaMasterNoticeReportDownloadResultHasACK

该函数用于通知libota\_master 云端对上报的下载结果有应答

*   **函数原型**
    

```c
void OtaMasterNoticeReportDownloadResultHasACK(const char *taskId)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  taskId  |  const char \*  |  IN  |  任务ID  |

### 4.3.25 OtaMasterNoticeReportUpgradeResultHasACK

该函数用于通知libota\_master 云端对上报的升级结果有应答

*   **函数原型**
    

```c
void OtaMasterNoticeReportUpgradeResultHasACK(const char *taskId)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  taskId  |  const char \*  |  IN  |  任务ID  |

### 4.3.26 OtaMasterNoticeUpDownHighVoltageResult

该函数用于通知libota\_master 上下高压请求结果

*   **函数原型**
    

```c
void OtaMasterNoticeUpDownHighVoltageResult(int result, OtaMasterUpgradePowerMode powerMode)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  result  |  int  |  IN  |  请求结果：OTA\_RET\_OK：请求成功；OTA\_RET\_ERR：请求失败  |
|  powerMode  |  OtaMasterUpgradePowerMode  |  IN  |  上下高压请求模式；详见：**4.3.26.1章节**  |

#### 4.3.26.1 OtaMasterUpgradePowerMode

*   **定义**
    

```c
typedef enum {
    OTA_LOW_VOLTAGE_UPGRADE = 0,  // 低压升级
    OTA_HIGH_VOLTAGE_UPGRADE = 1, // 高压升级
    STOP_HIGH_VOLTAGE_POWER_DOWN = 2, // 下高压后置零
} OtaMasterUpgradePowerMode;
```

*   **枚举说明**
    

|  **枚举定义**  |  **值**  |  **说明**  |
| --- | --- | --- |
|  OTA\_LOW\_VOLTAGE\_UPGRADE  |  0  |  低压升级  |
|  OTA\_HIGH\_VOLTAGE\_UPGRADE  |  1  |  高压升级  |

### 4.3.27 OtaMasterNoticeFotaModeOnOffSwResult

该函数用于通知libota\_master FOTA模式切换（禁止电源模式切换）结果

*   **函数原型**
    

```c
void OtaMasterNoticeFotaModeOnOffSwResult(int result, CanSignalOtaMode mode)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  result  |  int  |  IN  |  请求结果：OTA\_RET\_OK：请求成功；OTA\_RET\_ERR：请求失败  |
|  mode  |  CanSignalOtaMode  |  IN  |  OTA Mode ON/OFF；详见：**4.3.27.1****章节**  |

#### 4.3.27.1 CanSignalOtaMode

*   **定义**
    

```c
typedef enum {
    CAN_SIG_OTA_MODE_OFF = 0,  // OTA模式关闭
    CAN_SIG_OTA_MODE_ON = 1,   // OTA模式开启
} CanSignalOtaMode;
```

*   **枚举说明**
    

|  **枚举定义**  |  **值**  |  **说明**  |
| --- | --- | --- |
|  CAN\_SIG\_OTA\_MODE\_OFF  |  0  |  OTA模式关闭  |
|  CAN\_SIG\_OTA\_MODE\_ON  |  1  |  OTA模式开启  |

### 4.3.28 OtaMasterLogInit

该函数用于OTA Master Log初始化；OTA日志所有功能必须在成功调用该初始化接口后方可正常工作；

*   **函数原型**
    

```c
OtaLogErrorCode OtaMasterLogInit(const char *tag, const char *logDir)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  tag  |  const char \*  |  IN  |  日志标签  |
|  logDir  |  const char \*  |  IN  |  日志文件存储目录  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OtaLogErrorCode  |  OTA\_LOG\_OK初始化成功，其他失败；详见：**6.4章节**  |

### 4.3.29 OtaMasterLogFinish

该函数用于结束OTA Master日志输出；该函数调用后将不再输出OTA日志到本地文件，且无法使用日志备份、日志文件上传功能；

*   **函数原型**
    

```c
void OtaMasterLogFinish(void)
```

### 4.3.30 OtaMasterLogReset

该函数用于备份OTA Master日志文件并重置当前日志；该函数需要在'OtaMasterLogInit'调用后方可调用，该函数调用后会将当前OTA日志进行备份，并重置当前日志文件从头开始记录；

[警告]正常情况SDK内部会根据是否上传OTA日志表述在任务完成或失败后自行判断是否上传日志；

*   **函数原型**
    

```c
OtaLogErrorCode OtaMasterLogReset(void)
```

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_LOG\_OK  |  重置成功  |
|  其他  |  重置失败  |

### 4.3.31 OtaMasterUploadLog

该函数用于上传OTA Master日志到云端；APP不需要使用，SDK内部会在成功上报下载失败结果、升级结果到OTA平台且收到平台应答后自行打包上传OTA日志到云端；

*   **函数原型**
    

```c
void OtaMasterUploadLog(const char *taskId, const char *uploadUrl)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  taskId  |  const char \*  |  IN  |  OTA任务ID  |
|  uploadUrl  |  const char \*  |  IN  |  日志上传URL  |

### 4.3.32 OtaMasterLogPrint

该函数用于上传OTA Master日志到云端；APP使用SDK封装的日志宏进行日志输出，无需使用此接口，**详见章节：**

*   **函数原型**
    

```c
void OtaMasterLogPrint(OtaMasterLogLevel level, const char *file, int line, const char *format, ...)
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  level  |  OtaMasterLogLevel  |  IN  |  日志输出等级  |
|  file  |  const char \*  |  IN  |  日志输出所在文件  |
|  line  |  int  |  IN  |  日志所在文件行数  |
|  format  |  const char \*  |  IN  |  日志输出格式  |
|  ...  |  NA  |  IN  |  日志参数  |

# 5 数据结构说明

## 5.1 StOtaTaskPolicyFileInfo

OTA任务策略升级文件下载信息；

*   **定义**
    

```c
typedef struct {
    uint32_t fileSize;       // 文件大小
    const char *downloadUrl; // 下载地址
    const char *md5;         // 下载文件MD5
} StOtaTaskPolicyFileInfo;
```

*   **参数说明**
    

|  **类型**  |  **参数**  |  **描述**  |
| --- | --- | --- |
|  uint32\_t  |  fileSize  |  文件大小  |
|  const char \*  |  downloadUrl  |  下载地址  |
|  const char \*  |  md5  |  文件MD5  |

## 5.2 StOtaTaskPolicyFlashProtocol

OTA任务策略刷写协议信息；

*   **定义**
    

```c
typedef struct {
    uint8_t canPort;           // CAN端口
    uint32_t physicalId;       // 物理寻址ID
    uint32_t functionId;       // 功能寻址ID
    uint32_t responseId;       // 响应ID
    uint32_t secAccessLv1Mask; // 安全访问级别1掩码
    uint32_t secAccessLv2Mask; // 安全访问级别2掩码
} StOtaTaskPolicyFlashProtocol;
```

*   **参数说明**
    

|  **类型**  |  **参数**  |  **描述**  |
| --- | --- | --- |
|  uint8\_t  |  canPort  |  CAN端口，云端不下发则默认为0xFF  |
|  uint32\_t  |  physicalId  |  物理寻址ID，云端不下发则默认为0xFFFFFFFF  |
|  uint32\_t  |  functionId  |  功能寻址ID，云端不下发则默认为0xFFFFFFFF  |
|  uint32\_t  |  responseId  |  响应ID，云端不下发则默认为0xFFFFFFFF  |
|  uint32\_t  |  secAccessLv1Mask  |  安全访问级别1掩码（预留）  |
|  uint32\_t  |  secAccessLv2Mask  |  安全访问级别2掩码（预留）  |

## 5.3 StOtaTaskPolicyVersionInfo

OTA任务策略软件版本信息；

*   **定义**
    

```c
typedef struct {
    const char *partNum;  // 零部件号
    const char *HwVer;    // 硬件版本
    const char *oemSwVer; // OEM软件号
    const char *swVer;    // 软件版本
} StOtaTaskPolicyVersionInfo;
```

*   **参数说明**
    

|  **类型**  |  **参数**  |  **描述**  |
| --- | --- | --- |
|  const char \*  |  partNum  |  零部件号  |
|  const char \*  |  hwVer  |  硬件版本  |
|  const char \*  |  oemSwVer  |  OEM软件号  |
|  const char \*  |  swVer  |  软件版本  |

## 5.4 StOtaTaskPolicyEcuInfo

OTA任务策略升级ECU件信息

*   **定义**
    

```c
typedef struct {
    uint16_t ecuId;                          // 升级ECU ID
    uint8_t upgradeOrder;                    // 升级顺序
    const char *supplierId;                  // 供应商ID
    StOtaTaskPolicyFileInfo upgradeFileInfo; // 主文件信息
    StOtaTaskPolicyFileInfo backupFileInfo;  // 备份文件信息
    StOtaTaskPolicyFlashProtocol flashProto; // 刷写协议
    StOtaTaskPolicyVersionInfo supportVer;   // 支持版本
    StOtaTaskPolicyVersionInfo checkVer;     // 校验版本
} StOtaTaskPolicyEcuInfo;
```

*   **参数说明**
    

|  **类型**  |  **参数**  |  **描述**  |
| --- | --- | --- |
|  uint16\_t  |  ecuId  |  升级ECU ID  |
|  uint8\_t  |  upgradeOrder  |  升级顺序  |
| const char * | supplierId | 供应商ID |
|  StOtaTaskPolicyFileInfo  |  upgradeFileInfo  |  主文件信息  |
|  StOtaTaskPolicyFileInfo  |  backupFileInfo  |  备份文件信息  |
|  StOtaTaskPolicyFlashProtocol  |  flashProto  |  刷写协议  |
|  StOtaTaskPolicyVersionInfo  |  supportVer  |  支持版本  |
|  StOtaTaskPolicyVersionInfo  |  checkVer  |  校验版本  |

## 5.5 StOtaTaskPolicyInfo

OTA任务策略信息；

OTA任务策略信息包含OTA任务ID、任务名、任务信息、日志上传地址、任务开始时间、任务持续时间、是否上传OTA日志、升级模式、升级件总数、升级件信息等;

taskStartTime - OTA任务开始时间，为UTC时间戳，单位秒；当taskDuration不为0时，该时间由应用层从接收到OTA任务开始指令开始计时；若taskDuration为0时，该值默认传0，由master sdk在刷写开始时自行获取；

 taskDuration - OTA全任务持续时间，包含升级包下载、解压、前置条件判断、刷写、等待用户确认等时间；当该值为0时，默认全任务升级时长无限大，只考虑upgradeDuration；

upgradeDuration - 表示升级文件刷写时长，当taskDuration为0时，该值有效，仅进行刷写计时；若upgradeDuration也为0时，表示刷写时长无限大；

*   **定义**
    

```c
typedef struct {
    const char *taskId;                  // OTA任务ID
    const char *taskName;                // OTA任务名
    const char *taskInfo;                // OTA任务信息
    const char *logUploadAddr;           // OTA日志上传地址
    const char *vehicleOtaVer;           // 整车OTA版本
    uint32_t taskStartTime;              // OTA任务开始时间（UTC时间戳，单位秒）
    uint8_t taskDuration;                // OTA全任务持续时间（单位：分）
    uint8_t upgradeDuration;             // 升级文件刷写时长（单位：分）
    uint8_t isUploadLog;                 // 是否上传OTA日志 0：不上传 1：上传
    uint8_t upgradeMode;                 // 升级模式（0：静默升级，1：标准升级）
    uint8_t taskCount;                   // 升级件总数
    StOtaTaskPolicyEcuInfo *taskEcuInfo; // 升级件信息
} StOtaTaskPolicyInfo;
```

*   **参数说明**
    

|  **类型**  |  **参数**  |  **描述**  |
| --- | --- | --- |
|  const char \*  |  taskId  |  OTA任务ID  |
|  const char \*  |  taskName  |  OTA任务名  |
|  const char \*  |  taskInfo  |  OTA任务信息  |
|  const char *  |  logUploadAddr  |  OTA日志上传地址F  |
| const char * | vehicleOtaVer | 整车OTA版本 |
|  uint32\_t  |  taskStartTime  |  OTA任务开始时间（UTC时间戳，单位秒）  |
|  uint8\_t  |  taskDuration  |  OTA全任务持续时间（单位：分）  |
|  uint8\_t  |  upgradeDuration  |  升级文件刷写时长（单位：分）  |
|  uint8\_t  |  isUploadLog  |  是否上传OTA日志 0：不上传 1：上传  |
|  uint8\_t  |  upgradeMode  |  升级模式（0：静默升级，1：标准升级）  |
|  uint8\_t  |  taskCount  |  升级件总数  |
|  StOtaTaskPolicyEcuInfo \*  |  taskEcuInfo  |  升级件信息  |

## 5.6 StQueryEcuVerInfo

ECU版本查询信息；

*   **定义**
    

```c
typedef struct QueryEcuVerInfo_st {
    uint16_t ecuId;      // ECU ID
    uint8_t canPort;     // CAN端口 1：CAN BD; 2：CAN PT; 3：CAN FD;
    uint32_t physicalId; // 物理寻址ID
    uint32_t functionId; // 功能寻址ID
    uint32_t responseId; // 应答ID
} StQueryEcuVerInfo;
```

*   **参数说明**
    

|  **类型**  |  **参数**  |  **描述**  |
| --- | --- | --- |
|  uint16\_t  |  ecuId  |  ECU ID  |
|  uint8\_t  |  canPort  |  CAN端口 1：CAN BD; 2：CAN PT; 3：CAN FD;  |
|  uint32\_t  |  physicalId  |  物理寻址ID  |
|  uint32\_t  |  functionId  |  功能寻址ID  |
|  uint32\_t  |  responseId  |  应答ID  |

## 5.7 StEcuVersionInfo

ECU软硬件版本信息

*   **定义**
    

```c
typedef struct EcuVersionInfo_st {
    uint16_t ecuId;                         // ECU ID
    const char *swVersion;                  // 软件版本号
    const char *hwVersion;                  // 硬件版本号
    const char *supplierId;                 // 供应商ID
    const char *partNum;                    // 零部件号
} StEcuVersionInfo;
```

*   **参数说明**
    

|  **类型**  |  **参数**  |  **描述**  |
| --- | --- | --- |
|  uint16\_t  |  ecuId  |  ECU ID  |
|  const char \*  |  swVersion  |  软件版本号  |
|  const char \*  |  hwVersion  |  硬件版本号  |
| const char * | supplierId | 供应商ID |
| const char * | partNum | 零部件号 |

## 5.8 StUpgradePreconditions

升级前置条件；

*   **定义**
    

```c
typedef struct UpgradePreconditions_st {
    float batteryVoltage;                   // 蓄电池电压
    uint8_t gearPosition;                   // 当前档位; 0：P档，1：R档，2：N档，3：D档
    uint8_t vcuHvStatus;                    // 动力系统就绪状态; 0x0: Not ready，0x1: Ready
    uint16_t vehicleSpeed;                  // 车速
    uint8_t bmsRealSoc;                     // 动力电池 SOC
    uint8_t hasVehicleFault;                // 整车是否故障 0：无故障，1：有故障
    uint8_t chargePlugState;                // 充电枪连接状态 0：未连接，1：已连接
    uint8_t hasRemoteCtrlExecuting;         // 有无远程指令正在执行 0：无远控指令正在执行 1：有远控指令正在执行
    uint8_t isObdDiagDevConn;               // OBD诊断设备是否连接 0：未连接 1：已连接
    uint8_t isBcmAntiTheftAuthPassed;       // 二级防盗认证（BCM&TBOX）是否通过 0：未通过 1：已通过
} StUpgradePreconditions;
```

*   **参数说明**
    

|  **类型**  |  **参数**  |  **描述**  |
| --- | --- | --- |
|  float  |  batteryVoltage  |  蓄电池电压  |
|  uint8\_t  |  gearPosition  |  当前档位; 0：P档，1：R档，2：N档，3：D档  |
|  uint8\_t  |  vcuHvStatus  |  动力系统就绪状态; 0x0: Not ready，0x1: Ready  |
|  uint16\_t  |  vehicleSpeed  |  车速  |
|  uint8\_t  |  bmsRealSoc  |  动力电池 SOC  |
|  uint8\_t  |  hasVehicleFault  |  整车是否故障 0：无故障，1：有故障  |
|  uint8\_t  |  chargePlugState  |  充电枪连接状态 0：未连接，1：已连接  |
|  uint8\_t  |  hasRemoteCtrlExecuting  |  有无远程指令正在执行 0：无远控指令正在执行 1：有远控指令正在执行  |
|  uint8\_t  |  isObdDiagDevConn  |  OBD诊断设备是否连接 0：未连接 1：已连接  |
|  uint8\_t  |  isBcmAntiTheftAuthPassed  |  二级防盗认证（BCM&TBOX）是否通过 0：未通过 1：已通过  |

## 5.9 StOtaMasterReportCallback

OTA Master结果上报回调函数集合；

*   **定义**
    

```c
typedef struct {
    ReportStartDownloadCallbackFunc startDownloadCb;                       // 上报下载开始
    ReportDownloadResultCallbackFunc downloadResultCb;                     // 上报下载结果
    ReportUpgradeResultCallbackFunc upgradeResultCb;                       // 上报升级结果
    ReportOtaTaskClearResultCallbackFunc otaTaskClearResultCb;             // 上报OTA任务清空结果
    ReportPreconditionsCheckResultCallbackFunc preconditionsCheckResultCb; // 上报前置条件检测结果
    ReportStartUpgradeCallbackFunc startUpgradeCb;                         // 上报OTA刷写开始
    ReportStartUpgradeCallbackFunc startRollbackCb;                        // 上报回滚其他ECU
} StOtaMasterReportCallback;
```

*   **参数说明**
    

|  **类型**  |  **参数**  |  **描述**  |
| --- | --- | --- |
|  ReportStartDownloadCallbackFunc  |  startDownloadCb  |  上报下载开始，详见：**5.9.1章节**  |
|  ReportDownloadResultCallbackFunc  |  downloadResultCb  |  上报下载结果回调函数，详见：**5.9.2章节**  |
|  ReportUpgradeResultCallbackFunc  |  upgradeResultCb  |  上报升级结果回调函数，详见：**5.9.3章节**  |
|  ReportOtaTaskClearResultCallbackFunc  |  otaTaskClearResultCb  |  上报OTA任务清空结果回调函数，详见：**5.9.4章节**  |
|  ReportPreconditionsCheckResultCallbackFunc  |  preconditionsCheckResultCb  |  上报前置条件检测结果回调函数，详见：**5.9.5章节**  |
|  ReportStartUpgradeCallbackFunc  |  startUpgradeCb  |  上报OTA刷写开始回调函数，详见：**5.9.6章节**  |
| ReportStartUpgradeCallbackFunc | startRollbackCb | 上报回滚其他ECU，详见：**5.9.7章节** |

### 5.9.1 ReportStartDownloadCallbackFunc

该回调函数用于回调上报OTA升级件下载开始；

*   **函数原型**
    

```c
typedef int (*ReportStartDownloadCallbackFunc)(const char *taskId);
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  taskId  |  const char \*  |  IN  |  OTA任务ID  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  上报成功  |
|  OTA\_RET\_ERR  |  上报失败  |

### 5.9.2 ReportDownloadResultCallbackFunc

该回调函数用于回调上报升级包下载结果；

*   **函数原型**
    

```c
typedef int (*ReportDownloadResultCallbackFunc)(const char *taskId, int result, OtaUpgradeErrorCode errorCode);
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  taskId  |  const char \*  |  IN  |  OTA任务ID  |
|  reslut  |  int  |  IN  |  下载结果 OTA\_RET\_OK：成功 其他：失败  |
|  errorCode  |  OtaUpgradeErrorCode  |  IN  |  失败时的错误码  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  上报成功  |
|  OTA\_RET\_ERR  |  上报失败  |

### 5.9.3 ReportUpgradeResultCallbackFunc

该回调函数用于回调上报升级结果；

*   **函数原型**
    

```c
typedef int (*ReportUpgradeResultCallbackFunc)(const char *taskId, OtaUpgradeResult result, OtaUpgradeErrorCode errorCode);
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  taskId  |  const char \*  |  IN  |  OTA任务ID  |
|  reslut  |  int  |  IN  |  升级结果； OTA\_RET\_OK：成功 其他：失败  |
|  errorCode  |  OtaUpgradeErrorCode  |  IN  |  失败时的错误码  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  上报成功  |
|  OTA\_RET\_ERR  |  上报失败  |

### 5.9.4 ReportOtaTaskClearResultCallbackFunc

该回调函数用于回调上报OTA任务清除结果；

*   **函数原型**
    

```c
typedef int (*ReportOtaTaskClearResultCallbackFunc)(const char *taskId, int result);
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  taskId  |  const char \*  |  IN  |  OTA任务ID  |
|  reslut  |  int  |  IN  |  OTA任务清除结果； OTA\_RET\_OK：成功 其他：失败  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  上报成功  |
|  OTA\_RET\_ERR  |  上报失败  |

### 5.9.5 ReportPreconditionsCheckResultCallbackFunc

该回调函数用于回调上报升级前置条件检测结果回调；

*   **函数原型**
    

```c
typedef int (*ReportPreconditionsCheckResultCallbackFunc)(const char *taskId, int result, OtaPreconditionsErrorCode errCode);
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  taskId  |  const char \*  |  IN  |  OTA任务ID  |
|  reslut  |  int  |  IN  |  升级前置条件检测结果； OTA\_RET\_OK：成功 其他：失败  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  上报成功  |
|  OTA\_RET\_ERR  |  上报失败  |

### 5.9.6 ReportStartUpgradeCallbackFunc

该回调函数用于回调上报OTA刷写开始；

*   **函数原型**

```c
typedef int (*ReportStartUpgradeCallbackFunc)(const char *taskId);
```

*   **参数说明**

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  taskId  |  const char \*  |  IN  |  OTA任务ID  |

*   **返回值说明**

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  上报成功  |
|  OTA\_RET\_ERR  |  上报失败  |

### 5.9.7 ReportStartRollbackCallbackFunc

该回调函数用于回调上报开始版本回退；

- **函数原型**

```c
typedef int (*ReportStartRollbackCallbackFunc)(const char *taskId);
```

- **参数说明**

| **参数名** | **类型**      | **I/O** | **说明**  |
| ---------- | ------------- | ------- | --------- |
| taskId     | const char \* | IN      | OTA任务ID |

- **返回值说明**

| **返回值**    | **含义** |
| ------------- | -------- |
| OTA\_RET\_OK  | 上报成功 |
| OTA\_RET\_ERR | 上报失败 |

## 5.10 StOtaMasterQueryCallback

OTA Master查询函数集合

*   **定义**
    

```c
typedef struct {
    QueryTaskECUVersionSyncCallbackFunc queryTaskECUVerSyncCb;              // 同步查询ECU版本信息
    QueryTaskECUVersionAsyncCallbackFunc queryTaskECUVerAsyncCb;            // 异步查询ECU版本信息
    QueryUpgradePreconditionsSyncCallbackFunc queryUpgradePrecondsSyncCb;   // 同步查询前置条件
    QueryUpgradePreconditionsAsyncCallbackFunc queryUpgradePrecondsAsyncCb; // 异步查询前置条件
} StOtaMasterQueryCallback;
```

*   **参数**
    

|  **类型**  |  **参数**  |  **描述**  |
| --- | --- | --- |
|  QueryTaskECUVersionSyncCallbackFunc  |  queryTaskECUVerSyncCb  |  同步查询ECU版本信息回调函数，详见：**5.10.1章节**  |
|  QueryTaskECUVersionAsyncCallbackFunc  |  queryTaskECUVerAsyncCb  |  异步查询ECU版本信息回调函数，详见：**5.10.2章节**  |
|  QueryUpgradePreconditionsSyncCallbackFunc  |  queryUpgradePrecondsSyncCb  |  同步查询前置条件回调函数，详见：**5.10.3章节**  |
|  QueryUpgradePreconditionsAsyncCallbackFunc  |  queryUpgradePrecondsAsyncCb  |  异步查询前置条件回调函数，详见：**5.10.4章节**  |

### 5.10.1 QueryTaskECUVersionSyncCallbackFunc

该回调函数用于同步查询ECU版本信息；

当注册该同步查询回调函数时，SDK在需要查询ECU版本信息时会调用此回调函数，APP在查询到ECU版本信息后更新到ecuVersion中，并返回查询结果；

*   **函数原型**
    

```c
typedef int (*QueryTaskECUVersionSyncCallbackFunc)(const struct QueryEcuVerInfo_st *queryEcusInfo, uint8_t ecuCnt, struct EcuVersionInfo_st *ecuVersion);
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  queryEcusInfo  |  const struct QueryEcuVerInfo\_st \*  |  IN  |  查询的ECU信息集合；详见：**5.6****章节**  |
|  ecuCnt  |  uint8\_t  |  IN  |  查询的ECU数；  |
|  ecuVersion  |  struct EcuVersionInfo\_st \*  |  OUT  |  ECU版本信息；详见：**5.7****章节**  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  查询成功  |
|  OTA\_RET\_ERR  |  查询失败  |

### 5.10.2 QueryTaskECUVersionAsyncCallbackFunc

该函数用于回调异步查询ECU版本信息；

当未注册 \`QueryTaskECUVersionSyncCallbackFunc\` 函数，而仅注册本函数时，SDK在需要查询ECU版本信息时会调用此回调函数。 注册方不能阻塞该函数，且异步方式获取目标ECU的版本信息后通过\`OtaMasterNoticeEcuVersionAsyncQueryResult\` 告知 SDK查询结果。

*   **函数原型**
    

```c
typedef void (*QueryTaskECUVersionAsyncCallbackFunc)(const struct QueryEcuVerInfo_st *queryEcusInfo, uint8_t ecuCnt);
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  queryEcusInfo  |  const struct QueryEcuVerInfo\_st \*  |  IN  |  查询的ECU信息集合  |
|  ecuCnt  |  uint8\_t  |  IN  |  查询的ECU数，表示 \`ecuIds\` 数组中的元素个数。  |

### 5.10.3 QueryUpgradePreconditionsSyncCallbackFunc

该函数用于回调同步查询前置条件；

当注册该同步查询回调函数时，SDK在准备升级前会调用此回调函数同步查询升级前置条件是否满足，注册方同步查询所有要求的前置条件具体信息后更新到preconditons中，并返回查询结果；

*   **函数原型**
    

```c
typedef int (*QueryUpgradePreconditionsSyncCallbackFunc)(struct UpgradePreconditions_st *preconditions);
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  preconditons  |  struct UpgradePreconditions\_st \*  |  OUT  |  查询的前置条件结果；若有不要求的前置条件，可给默认值0；  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  查询成功  |
|  OTA\_RET\_ERR  |  查询失败  |

### 5.10.4 QueryUpgradePreconditionsAsyncCallbackFunc

该函数用于回调异步查询前置条件；

当未注册 \`QueryUpgradePreconditionsSyncCallbackFunc\`函数，而仅注册本函数时，SDK在准备升级前会调用此回调函数异步查询升级前置条件是否满足。 注册方不能阻塞该函数，异步方式查询所有要求的前置条件具体信息后通过\`OtaMasterNoticeUpgradePreconditionsAsyncQueryResult\` 告知 SDK 查询结果。

*   **函数原型**
    

```c
typedef void (*QueryUpgradePreconditionsAsyncCallbackFunc)(void);
```

## 5.11 StOtaMasterRequestCallback

OTA Master请求/发送回调函数集合；

*   **定义**
    

```c
typedef struct {
    SendUdsDataCallbackFunc sendUdsDataCb;                     // 发送UDS协议数据
    SendUdsDataAckCallbackFunc sendUdsDataAckCb;               // UDS协议数据ACK
    RequestUpDownHighVoltageCallbackFunc reqUpDownHighVoltCb;  // 上下高压请求
    RequestFotaModeOnOffSwCallbackFunc reqFotaModeOnOff;       // FOTA Mode ON/OFF请求
    RequestTboxSelfUpgradeCallbackFunc reqTboxSelfUpgradeCb;   // TBox自升级触发请求
    RequestTboxSelfRollbackCallbackFunc reqTboxSelfRollbackCb; // TBox自回滚触发请求
    RequestTboxRebootCallbackFunc reqTboxRebootCb;             // TBox重启请求
} StOtaMasterRequestCallback;
```

*   **参数**
    

|  **类型**  |  **参数**  |  **描述**  |
| --- | --- | --- |
|  SendUdsDataCallbackFunc  |  sendUdsDataCb  |  发送UDS协议数据回调函数，详见：**5.11.1章节**  |
|  SendUdsDataAckCallbackFunc  |  sendUdsDataAckCb  |  UDS协议数据ACK回调函数，详见：**5.11.2章节**  |
|  RequestUpDownHighVoltageCallbackFunc  |  reqUpDownHighVoltCb  |  上下高压请求回调函数，详见：**5.11.3章节**  |
|  RequestFotaModeOnOffSwCallbackFunc  |  reqFotaModeOnOff  |  FOTA Mode ON/OFF请求回调函数，详见：**5.11.4章节**  |
|  RequestTboxSelfUpgradeCallbackFunc  |  reqTboxSelfUpgradeCb  |  TBox自升级触发请求回调函数，详见：**5.11.5章节**  |
|  RequestTboxSelfRollbackCallbackFunc  |  reqTboxSelfRollbackCb  |  TBox自回滚触发请求回调函数，详见：**5.11.6章节**  |
|  RequestTboxRebootCallbackFunc  |  reqTboxRebootCb  |  TBox重启请求回调函数，详见：**5.11.7章节**  |

### 5.11.1 SendUdsDataCallbackFunc

该回调函数用于回调通过APP发送UDS协议数据到MCU；

*   **函数原型**
    

```c
/**
 * 协议结构（按字节索引顺序）：
 * 
  * +-------------------------------------------------------------------------------------------------------+
 * | 字节索引 | 字段名称             | 值范围/类型        | 描述                                                |
 * |----------|----------------------|-------------------|-------------------------------------------------|
 * | [0]      | 协议起始标识         | 固定值 0x00        | 标识数据包的开始，固定为0。                           |
 * | [1]      | 发送模式标志         | 0x00 或 0x01       | 0x00: 首次发送（新会话）；0x01: 重复发送（重传或续传）   |
 * | [2]      | 计数器索引           | 0x00~0xFF (uint8) | 发送计数器，每次发送自增1，溢出后自动回绕为0。           |
 * | [3]-[6]  | 目标CAN ID          | uint32_t          | 目标设备CAN标识符（物理寻址或功能寻址），以大端格式存储。   |
 * | [7]-[10] | 响应CAN ID          | uint32_t          | ECU回复的响应CAN标识符，存储格式同目标CAN ID。          |
 * | [11]     | CAN通道号           | 0x00~0xFF         | 指定CAN总线通道（如0x00:CAN0, 0x01:CAN1）。            |
 * | [12]-N   | 报文载荷            | uint8_t[]         | 实际传输的CAN报文数据，长度可变。                       |
 * +--------------------------------------------------------------------------------------------------------+
 * 
 * @note 使用说明：
 * 1. **字节序**：多字节字段（如CAN ID）按小端格式存储（低字节在前，高字节在后）。
 * 2. **计数器回绕**：计数器索引达到0xFF后，下一次发送应回绕为0x00。
 * 3. **报文长度**：总长度 = 12字节头部 + 报文载荷长度（需确保缓冲区足够大）。
 * 4. **重传机制**：发送模式标志为0x01时，接收方应忽略计数器索引的变化，视为同一数据包的重传。
 * 
 * @example 示例数据：
 *   uint8_t buffer[] = {
 *       0x00,               // [0] 起始标识
 *       0x00,               // [1] 首次发送
 *       0x5A,               // [2] 计数器索引=90
 *       0x00, 0x00, 0x07, 0x27, // [3]-[6] 目标CAN ID=0x00000727
 *       0x00, 0x00, 0x07, 0xA7, // [7]-[10] 响应CAN ID=0x000007A7
 *       0x00,               // [11] CAN通道0
 *       0xAA, 0xBB, 0xCC    // [12]-[14] 报文载荷（3字节）
 *   };
 */
typedef int (*SendUdsDataCallbackFunc)(const uint8_t *data, uint16_t dataLen);
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  data  |  const uint8\_t \*  |  IN  |  数据起始地址；  |
|  dataLen  |  uint16\_t  |  IN  |  数据长度  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  发送成功  |
|  OTA\_RET\_ERR  |  发送失败  |

### 5.11.2 SendUdsDataAckCallbackFunc

该回调函数用于回调通过APP发送接收UDS数据后的应答数据；

*   **函数原型**
    

```c
/**
 * MPU回复ACK数据格式 (类型4)
 * 
 * +----------------------------------------------------------------------------------------------+
 * | 字节索引 | 字段名称             | 值范围/类型        | 描述                                       |
 * |----------|-------------------|-------------------|-------------------------------------------|
 * | [0]      | 协议类型标识        | 0x00              | 标识为ACK回复协议。                          |
 * | [1]      | ACK类型标识        | 0x00              | 标识为MPU的ACK。                            |
 * | [2]      | 计数器索引          | 0x00~0xFF (uint8) | 必须与SendUdsDataCallbackFunc中的data[2]:  |
 * |          |                   |                   | counterIdx一致，表示对应请求已处理完成。       |
 * +----------------------------------------------------------------------------------------------+
 * 
 * @note 数据总长度固定为3字节。
 * @example MPU ACK示例：
 *   buffer = {0x00, 0x00, 0x5A}; // 确认处理完成counterIdx=0x5A的报文
 */
typedef int (*SendUdsDataAckCallbackFunc)(const uint8_t *data, uint16_t dataLen);
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  data  |  const uint8\_t \*  |  IN  |  数据起始地址  |
|  dataLen  |  uint16\_t  |  IN  |  数据长度  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  发送成功  |
|  OTA\_RET\_ERR  |  发送失败  |

### 5.11.3 RequestUpDownHighVoltageCallbackFunc

该回调函数用于回调通过APP发送上、下高压请求到CAN总线；

1.  OTA\_HIGH\_VOLTAGE\_UPGRADE：上高压请求；
    
    1.   \* b. VCU检测到低压上电后，执行上高压，若TBOX检测到高压状态VCU\_HV\_Sts =0x2: HV ON，则高压上电成功，高压上电通过；
        
    2.  VCU检测到低压上电后，执行上高压，若TBOX检测到高压状态VCU\_HV\_Sts =0x2: HV ON，则高压上电成功，高压上电通过；
    
2.  OTA\_LOW\_VOLTAGE\_UPGRADE：下高压请求
    
    1.  TBOX发送下高压请求TBOX\_HV\_Req=01:ON，VCU执行高压下电后返回VCU\_HV\_Sts=0x1: Standby，则高压下电请求通过;
        
    2.  VCU执行高压下电后返回VCU\_HV\_Sts！=0x1: Standby，表示下高压失败；
        

上下高压结果通过\`OtaMasterNoticeUpDownHighVoltageResult\`异步通知SDK

*   **函数原型**
    

```c
typedef int (*RequestUpDownHighVoltageCallbackFunc)(OtaMasterUpgradePowerMode powerMode);
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  powerMode  |  OtaMasterUpgradePowerMode  |  IN  |  见函数说明  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  请求发送成功  |
|  OTA\_RET\_ERR  |  请求发送失败  |

### 5.11.4 RequestFotaModeOnOffSwCallbackFunc

该回调函数用于回调通过APP发送FOTA Mode ON/OFF切换请求；

模式切换结果通过\`OtaMasterNoticeFotaModeOnOffResult\`异步通知SDK；

*   **函数原型**
    

```c
typedef int (*RequestFotaModeOnOffSwCallbackFunc)(CanSignalOtaMode isModeOn);
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  isModeOn  |  CanSignalOtaMode  |  IN  |  升级模式状态；详见：**4.3.27.1章节**  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  请求发送成功  |
|  OTA\_RET\_ERR  |  请求发送失败  |

### 5.11.5 RequestTboxSelfUpgradeCallbackFunc

该回调函数用于回调请求TBox执行自升级；

*   **函数原型**
    

```c
typedef int (*RequestTboxSelfUpgradeCallbackFunc)(const char *upgradePath);
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  upgradePath  |  const char \*  |  IN  |  TBox升级包解压后的绝对路径  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  请求成功  |
|  OTA\_RET\_ERR  |  请求失败  |

### 5.11.6 RequestTboxSelfRollbackCallbackFunc

该回调函数用于请求TBox进行升级回滚；

当OTA任务有多个升级件要升级时，TBox已经升级成功，但是其他件升级失败，SDK通过该回调函数通知TBox进行版本回滚；

*   **函数原型**
    

```c
typedef int (*RequestTboxSelfRollbackCallbackFunc)(const char *backupPath);
```

*   **参数说明**
    

|  **参数名**  |  **类型**  |  **I/O**  |  **说明**  |
| --- | --- | --- | --- |
|  backupPath  |  const char \*  |  IN  |  TBox备份包解压后的绝对路径  |

*   **返回值说明**
    

|  **返回值**  |  **含义**  |
| --- | --- |
|  OTA\_RET\_OK  |  请求成功  |
|  OTA\_RET\_ERR  |  请求失败  |

### 5.11.7 RequestTboxRebootCallbackFunc

该回调函数用于请求请求TBox进行重启切换分区；

当TBox升级完成后且需要通过重启才能完成分区切换时，SDK通过此回调函数通知APP可以执行重启后APP才可以进行重启；

*   **函数原型**
    

```c
typedef void (*RequestTboxRebootCallbackFunc)(void);
```

# 6 错误码说明

## 6.1 升级结果状态码

表示OTA 任务的最终升级结果；

*   **定义**
    

```c
typedef enum {
    OTA_UPGRADE_SUCC = 0x01,           // 升级成功
    OTA_UPGRADE_FAILED = 0x02,         // 升级失败
    OTA_UPGRADE_ROLLBACK_SUCC = 0x03,  // 升级失败, 回滚成功
    OTA_UPGRADE_ROLLBACK_FAILED = 0x04 // 升级失败, 回滚失败
} OtaUpgradeResult;
```

*   **枚举说明**
    

|  枚举值  |  值  |  说明  |
| --- | --- | --- |
|  OTA\_UPGRADE\_SUCC  |  0x01  |  升级成功  |
|  OTA\_UPGRADE\_FAILED  |  0x02  |  升级失败  |
|  OTA\_UPGRADE\_ROLLBACK\_SUCC  |  0x03  |  升级失败, 回滚成功  |
|  OTA\_UPGRADE\_ROLLBACK\_FAILED  |  0x04  |  升级失败, 回滚失败  |

## 6.2 升级结果错误码

*   **定义**
    

```c
typedef enum {
    OTA_OK = 0x00,                                  // 无错误
    OTA_ERR_UPGRADE_FILE_DOWNLOAD_FAILED = 0x01,    // 升级文件下载失败
    OTA_ERR_UPGRADE_FILE_MD5_CHECK_FAILED = 0x02,   // 升级文件MD5校验失败
    OTA_ERR_UPGRADE_FILE_DECOMPRESS_FAILED = 0x03,  // 升级文件解压失败
    OTA_ERR_PROGRAM_FILE_CHECK_FAILED = 0x04,       // 程序文件效验不通过
    OTA_ERR_PROGRAM_FILE_TOO_LARGE = 0x05,          // 程序文件过大无法存储
    OTA_ERR_UPGRADING_MPU_NO_RESPONSE = 0x06,       // 刷写MPU过程无应答
    OTA_ERR_UPGRADING_MPU_NEGATIVE_RESPONSE = 0x07, // 刷写MPU过程否定应答
    OTA_ERR_VER_VERIFY_FAILE_AFTER_UPGRADE = 0x08,  // 刷写目标升级后版本校验失败
    OTA_ERR_NOT_SATISFIED_UPGRADE_CONDITION = 0x09, // 不满足刷写条件
    OTA_ERR_VEHICLE_LOW_VOLTAGE = 0x0A,             // 车辆低电压
    OTA_ERR_UPGRADE_IN_PROGRESS = 0x0B,             // 刷写正在执行
    OTA_ERR_NEW_TASK = 0x0C,                        // 平台下发新任务，当前任务失败
    OTA_ERR_POLICY_FILE_DOWNLOAD_FAILED = 0x0D,     // 升级策略文件下载失败
    OTA_ERR_POLICY_FILE_CHECK_FAILED = 0x0E,        // 升级策略文件校验不通过
    OTA_ERR_INTERRUPTED_BY_ECU = 0x0F,              // 被ECU升级中断
    OTA_ERR_CURRENT_VERSION_NOT_SUPPORT = 0x10,     // 当前版本不是支持版本
    OTA_ERR_OFFLINE_UPGRADE_OCCUR = 0x11,           // 发生线下刷写
    OTA_ERR_UPGRADE_FILE_NOT_EXIST = 0x12,          // 升级包不存在
    OTA_ERR_BACKUP_FILE_NOT_EXIST = 0x13,           // 备份包不存在
    OTA_ERR_PROHIBIT_POWER_SWITCH_FAILED = 0x14,    // BCM进入OTA模式失败（禁止电源切换）
    OTA_ERR_APPLY_HIGH_VOLTAGE_FAILED = 0x15,       // 上高压失败
    OTA_ERR_BRUSH_VERSION_INCONSISTENT = 0x16,      // 实际刷写版本与目标刷写版本不一致
    OTA_ERR_BACKUP_FILE_DOWNLOAD_FAILED = 0x17,     // 备份文件下载失败
    OTA_ERR_BACKUP_FILE_MD5_CHECK_FAILED = 0x18,    // 备份文件MD5校验失败
    OTA_ERR_BACKUP_FILE_DECOMPRESS_FAILED = 0x19,   // 备份文件解压失败
    OTA_ERR_HIGH_VOLTAGE_DOWN_FAILED = 0x1A,        // 高压下电失败
    OTA_ERR_ANTI_THEFT_AUTH_FAILED = 0x1B,          // 防盗认证不通过
    OTA_ERR_UPGRADE_TARGET_ECU_FAILED = 0x1C,       // 升级目标ECU失败
    OTA_ERR_PART_NUM_OR_HW_VER_NOT_MATCH = 0x1D,    // 零件号或硬件版本不匹配
    OTA_ERR_UPGRADE_POLICY_FILE_FORMAT_ERR = 0x1E,  // 升级策略文件格式错误
    OTA_ERR_BIN_FILE_NUM_NOT_MATCH = 0x1F,          // bin文件数量不匹配
    OTA_ERR_BIN_FILE_MD5_CHECK_FAILED = 0x20,       // bin文件MD5校验失败
    OTA_ERR_MPU_UPGRADE_FAILED = 0x21,              // bin文件解压失败
    OTA_ERR_TASK_TIMEOUT = 0x22                     // 任务超时
} OtaUpgradeErrorCode;
```

*   **枚举说明**
    

|  枚举值  |  值  |  说明  |
| --- | --- | --- |
|  OTA\_OK  |  0x00  |  无错误  |
|  OTA\_ERR\_UPGRADE\_FILE\_DOWNLOAD\_FAILED  |  0x01  |  升级文件下载失败  |
|  OTA\_ERR\_UPGRADE\_FILE\_MD5\_CHECK\_FAILED  |  0x02  |  升级文件MD5校验失败  |
|  OTA\_ERR\_UPGRADE\_FILE\_DECOMPRESS\_FAILED  |  0x03  |  升级文件解压失败  |
|  OTA\_ERR\_PROGRAM\_FILE\_CHECK\_FAILED  |  0x04  |  程序文件效验不通过  |
|  OTA\_ERR\_PROGRAM\_FILE\_TOO\_LARGE  |  0x05  |  程序文件过大无法存储  |
|  OTA\_ERR\_UPGRADING\_MPU\_NO\_RESPONSE  |  0x06  |  刷写MPU过程无应答  |
|  OTA\_ERR\_UPGRADING\_MPU\_NEGATIVE\_RESPONSE  |  0x07  |  刷写MPU过程否定应答  |
|  OTA\_ERR\_VER\_VERIFY\_FAILE\_AFTER\_UPGRADE  |  0x08  |  刷写目标升级后版本校验失败  |
|  OTA\_ERR\_NOT\_SATISFIED\_UPGRADE\_CONDITION  |  0x09  |  不满足刷写条件  |
|  OTA\_ERR\_VEHICLE\_LOW\_VOLTAGE  |  0x0A  |  车辆低电压  |
|  OTA\_ERR\_UPGRADE\_IN\_PROGRESS  |  0x0B  |  刷写正在执行  |
|  OTA\_ERR\_NEW\_TASK  |  0x0C  |  平台下发新任务，当前任务失败  |
|  OTA\_ERR\_POLICY\_FILE\_DOWNLOAD\_FAILED  |  0x0D  |  升级策略文件下载失败  |
|  OTA\_ERR\_POLICY\_FILE\_CHECK\_FAILED  |  0x0E  |  升级策略文件校验不通过  |
|  OTA\_ERR\_INTERRUPTED\_BY\_ECU  |  0x0F  |  被ECU升级中断  |
|  OTA\_ERR\_CURRENT\_VERSION\_NOT\_SUPPORT  |  0x10  |  当前版本不是支持版本  |
|  OTA\_ERR\_OFFLINE\_UPGRADE\_OCCUR  |  0x11  |  发生线下刷写  |
|  OTA\_ERR\_UPGRADE\_FILE\_NOT\_EXIST  |  0x12  |  升级包不存在  |
|  OTA\_ERR\_BACKUP\_FILE\_NOT\_EXIST  |  0x13  |  备份包不存在  |
|  OTA\_ERR\_PROHIBIT\_POWER\_SWITCH\_FAILED  |  0x14  |  BCM进入OTA模式失败（禁止电源切换）  |
|  OTA\_ERR\_APPLY\_HIGH\_VOLTAGE\_FAILED  |  0x15  |  上高压失败  |
|  OTA\_ERR\_BRUSH\_VERSION\_INCONSISTENT  |  0x16  |  实际刷写版本与目标刷写版本不一致  |
|  OTA\_ERR\_BACKUP\_FILE\_DOWNLOAD\_FAILED  |  0x17  |  备份文件下载失败  |
|  OTA\_ERR\_BACKUP\_FILE\_MD5\_CHECK\_FAILED  |  0x18  |  备份文件MD5校验失败  |
|  OTA\_ERR\_BACKUP\_FILE\_DECOMPRESS\_FAILED  |  0x19  |  备份文件解压失败  |
|  OTA\_ERR\_HIGH\_VOLTAGE\_DOWN\_FAILED  |  0x1A  |  高压下电失败  |
|  OTA\_ERR\_ANTI\_THEFT\_AUTH\_FAILED  |  0x1B  |  防盗认证不通过  |
|  OTA\_ERR\_UPGRADE\_TARGET\_ECU\_FAILED  |  0x1C  |  升级目标ECU失败  |
|  OTA\_ERR\_PART\_NUM\_OR\_HW\_VER\_NOT\_MATCH  |  0x1D  |  零件号或硬件版本不匹配  |
|  OTA\_ERR\_UPGRADE\_POLICY\_FILE\_FORMAT\_ERR  |  0x1E  |  升级策略文件格式错误  |
|  OTA\_ERR\_BIN\_FILE\_NUM\_NOT\_MATCH  |  0x1F  |  bin文件数量不匹配  |
|  OTA\_ERR\_BIN\_FILE\_MD5\_CHECK\_FAILED  |  0x20  |  bin文件MD5校验失败  |
|  OTA\_ERR\_MPU\_UPGRADE\_FAILED  |  0x21  |  bin文件解压失败  |
|  OTA\_ERR\_TASK\_TIMEOUT  |  0x22  |  任务超时  |

## 6.3 升级前置条件检测错误码

*   **定义**
    

```c
typedef enum {
    PRECONDS_ERR_NONE = 0x00,                                      // 无错误
    PRECONDS_ERR_VEHICLE_IN_READY = 0x01,                          // 车辆处于READY状态
    PRECONDS_ERR_VEHICLE_IN_MOTION = 0x02,                         // 车辆非静止状态
    PRECONDS_ERR_POWER_BATTERY_QUANTITY_INSUFFICIENT = 0x03,       // 车辆动力电池电量不足
    PRECONDS_ERR_LOW_VOLTAGE_BATTERY_QUANTITY_INSUFFICIENT = 0x04, // 车辆低压蓄电池电量不足
    PRECONDS_ERR_NOT_P_OR_N_GEAR = 0x05,                           // 车辆挡位不是P/N挡
    PRECONDS_ERR_VEHICLE_MALFUNCTION = 0x06,                       // 整车有故障
    PRECONDS_ERR_CHARGE_PLUG_CONN = 0x07,                          // 充电枪连接
    PRECONDS_ERR_REMOTE_CTRL_EXECUTING = 0x08,                     // 远程指令正在执行
    PRECONDS_ERR_DIAGNOSTIC_DEV_CONN = 0x09,                       // 有诊断设备连接
    PRECONDS_ERR_UNKNOWN_REASON = 0xFE                             // 未知原因
} OtaPreconditionsErrorCode;
```

*   **枚举说明**
    

|  枚举值  |  值  |  说明  |
| --- | --- | --- |
|  PRECONDS\_ERR\_NONE  |  0x00  |  无错误  |
|  PRECONDS\_ERR\_VEHICLE\_IN\_READY  |  0x01  |  车辆处于READY状态  |
|  PRECONDS\_ERR\_VEHICLE\_IN\_MOTION  |  0x02  |  车辆非静止状态  |
|  PRECONDS\_ERR\_POWER\_BATTERY\_QUANTITY\_INSUFFICIENT  |  0x03  |  车辆动力电池电量不足  |
|  PRECONDS\_ERR\_LOW\_VOLTAGE\_BATTERY\_QUANTITY\_INSUFFICIENT  |  0x04  |  车辆低压蓄电池电量不足  |
|  PRECONDS\_ERR\_NOT\_P\_OR\_N\_GEAR  |  0x05  |  车辆挡位不是P/N挡  |
|  PRECONDS\_ERR\_VEHICLE\_MALFUNCTION  |  0x06  |  整车有故障  |
|  PRECONDS\_ERR\_CHARGE\_PLUG\_CONN  |  0x07  |  充电枪连接  |
|  PRECONDS\_ERR\_REMOTE\_CTRL\_EXECUTING  |  0x08  |  远程指令正在执行  |
|  PRECONDS\_ERR\_DIAGNOSTIC\_DEV\_CONN  |  0x09  |  有诊断设备连接  |
|  PRECONDS\_ERR\_UNKNOWN\_REASON  |  0xFE  |  未知原因  |

## 6.4 日志相关错误码

*   **定义**
    

```c
typedef enum {
    OTA_LOG_OK = 0x0,                      // 无错误
    OTA_LOG_NOT_INITIALIZES = 0x01,        // OTA Log未初始化
    OTA_LOG_ALREADY_INITIALIZED = 0x02,    // OTA Log已经初始化
    OTA_LOG_FILE_CREATION_FAILED = 0x03,   // OTA日志文件创建失败
    OTA_LOG_FILE_BACKUP_FAILED = 0x04,     // OTA日志文件备份失败
    OTA_LOG_NULL_POINTER_PARAMETER = 0x05, // 空指针参数
    OTA_LOG_INVALID_PARAMETER = 0x06       // 无效参数
} OtaLogErrorCode;
```

*   **枚举说明**
    

|  枚举值  |  值  |  说明  |
| --- | --- | --- |
|  OTA\_LOG\_OK\_ERR\_NONE  |  0x00  |  无错误  |
|  OTA\_LOG\_NOT\_INITIALIZES  |  0x01  |  OTA Log未初始化  |
|  OTA\_LOG\_ALREADY\_INITIALIZED  |  0x02  |  OTA Log已经初始化  |
|  OTA\_LOG\_FILE\_CREATION\_FAILED  |  0x03  |  OTA日志文件创建失败  |
|  OTA\_LOG\_FILE\_BACKUP\_FAILED  |  0x04  |  OTA日志文件备份失败  |
|  OTA\_LOG\_NULL\_POINTER\_PARAMETER  |  0x05  |  空指针参数  |
|  OTA\_LOG\_INVALID\_PARAMETER  |  0x06  |  无效参数  |

# 7 示例

## 7.1 初始化

[警告]注意：初始化动作在OTA APP进程生命周期内，只要做一次即可；重复初始化仅会生效一次；

### 7.1.1 示例代码

```c
#include <stdint.h>
#include <stdio.h>
#include "ota_master_api.h"
#include "ota_master_log.h"

static int ReportDownloadResultCallback(const char *taskId, int reslut, OtaUpgradeErrorCode errorCode)
{
    OTA_LOG_D("ReportDownloadResultCallback taskId:%s reslut: %d, errorCode: %d", taskId, reslut, errorCode);
    return OTA_RET_OK;
}

static int ReportUpgradeResultCallback(const char *taskId, OtaUpgradeResult reslut, OtaUpgradeErrorCode errorCode)
{
    OTA_LOG_D("ReportUpgradeResultCallback taskId:%s reslut: %d, errorCode: %d", taskId, reslut, errorCode);
    return OTA_RET_OK;
}

static int ReportOtaTaskClearResultCallback(const char *taskId, int reslut)
{
    OTA_LOG_D("ReportOtaTaskClearResultCallback taskId:%s reslut: %d", taskId, reslut);
    return OTA_RET_OK;
}

static int ReportPreconditionsCheckResultCallback(const char *taskId, int result, OtaPreconditionsErrorCode errCode)
{
    OTA_LOG_D("ReportPreconditionsCheckResultCallback taskId:%s result: %d, errCode: %d", taskId, result, errCode);
    return OTA_RET_OK;
}

static int ReportStartUpgradeCallback(const char *taskId)
{
    OTA_LOG_D("ReportStartUpgradeCallback taskId: %s", taskId);
    return OTA_RET_OK;
}

static int SendUdsDataCallback(const uint8_t *data, uint16_t dataLen)
{
    OTA_LOG_D("SendUdsDataCallback dataLen: %d", dataLen);
    return OTA_RET_OK;
}

static int SendUdsDataAckCallback(const uint8_t *data, uint16_t dataLen)
{
    OTA_LOG_D("SendUdsDataAckCallback dataLen: %d", dataLen);
    return OTA_RET_OK;
}

static int QueryTaskECUVersionSyncCallback(const StQueryEcuVerInfo *queryEcusInfo, uint8_t ecuCnt, StEcuVersionInfo *ecuVersion)
{
    OTA_LOG_D("QueryTaskECUVersionSyncCallback ecuCnt: %d", ecuCnt);
    return OTA_RET_OK;
}

static int QueryUpgradePreconditionsSyncCallback(StUpgradePreconditions *preconditions)
{
    OTA_LOG_D("QueryUpgradePreconditionsSyncCallback");
    return OTA_RET_OK;
}

static int RequestLowOrHighVoltageCallback(OtaMasterUpgradePowerMode powerMode)
{
    OTA_LOG_D("RequestLowOrHighVoltageCallback powerMode: %d", powerMode);
    return OTA_RET_OK;
}

static int RequestFotaModeOnOffCallback(CanSignalOtaMode isModeOn)
{
    OTA_LOG_D("RequestFotaModeOnOffCallback isModeOn: %d", isModeOn);
    return OTA_RET_OK;
}

static int RequestTboxSelfUpgradeCallback(const char *upgradePath)
{
    OTA_LOG_D("RequestTboxSelfUpgradeCallback upgradePath: %s", upgradePath);
    return OTA_RET_OK;
}

static int RequestTboxSelfRollbackCallback(const char *backupPath)
{
    OTA_LOG_D("RequestTboxSelfRollbackCallback backupPath: %s", backupPath);
    return OTA_RET_OK;
}

static int RequestEcuVerRollbackCallback(uint16_t ecuId, uint32_t physicalId, uint32_t responseId, uint8_t canPort)
{
    OTA_LOG_D("RequestEcuVerRollbackCallback ecuId: %d, physicalId: %d, responseId: %d, canPort: %d", ecuId, physicalId,
              responseId, canPort);
    return OTA_RET_OK;
}

static void RequestTboxRebootCallback(void)
{
    OTA_LOG_D("RequestTboxRebootCallback");
}

static void TestOtaMasterRegisterReportCallback(void)
{
    static StOtaMasterReportCallback otaMasterReportCb = {.downloadResultCb = ReportDownloadResultCallback,
                                                   .upgradeResultCb = ReportUpgradeResultCallback,
                                                   .otaTaskClearResultCb = ReportOtaTaskClearResultCallback,
                                                   .preconditionsCheckResultCb = ReportPreconditionsCheckResultCallback,
                                                   .startUpgradeCb = ReportStartUpgradeCallback};
    OtaMasterRegisterReportCallback(&otaMasterReportCb);
}

static void TestOtaMasterRegisterQueryCallback(void)
{
    static StOtaMasterQueryCallback otaMasterQueryCb = {.queryTaskECUVerSyncCb = QueryTaskECUVersionSyncCallback,
                                                 .queryTaskECUVerAsyncCb = nullptr,
                                                 .queryUpgradePrecondsSyncCb = QueryUpgradePreconditionsSyncCallback,
                                                 .queryUpgradePrecondsAsyncCb = nullptr};
    OtaMasterRegisterQueryCallback(&otaMasterQueryCb);
}

static void TestOtaMasterRegisterRequestCallback(void)
{
    static StOtaMasterRequestCallback otaMasterRequestCallback = {.sendUdsDataCb = SendUdsDataCallback,
                                                           .sendUdsDataAckCb = SendUdsDataAckCallback,
                                                           .reqUpDownHighVoltCb = RequestLowOrHighVoltageCallback,
                                                           .reqFotaModeOnOff = RequestFotaModeOnOffCallback,
                                                           .reqTboxSelfUpgradeCb = RequestTboxSelfUpgradeCallback,
                                                           .reqTboxSelfRollbackCb = RequestTboxSelfRollbackCallback,
                                                           .reqEcuVerRollbackCb = RequestEcuVerRollbackCallback,
                                                           .reqTboxRebootCb = RequestTboxRebootCallback};
    OtaMasterRegisterRequestCallback(&otaMasterRequestCallback);
}

static const char *OTA_MASTER_LOG_SAVE_FOLDER = "/sdcard/ota_master/";
int main(int argc, char **argc)
{
    // SDK日志初始化
    OtaLogErrorCode logError = OtaMasterLogInit("ota_master_demo", OTA_MASTER_LOG_SAVE_FOLDER);
    if (logError != OTA_LOG_OK) {
        printf("OtaMasterLogInit failed, error: %d\n", logError);
        return;
    }
    OTA_LOG_D("OtaMasterLogInit success");

    // SDK初始化
    int error = OtaMasterInit(OTA_MASTER_CONFIG_FOLDER, OTA_MASTER_DOWNLOAD_FOLDER, OTA_MASTER_UNZIP_FOLDER);
    if (error != OTA_RET_OK) {
        OTA_LOG_E("OtaMasterInit failed");
        return;
    }
    OTA_LOG_D("OtaMasterInit success");

    // 注册回调函数
    TestOtaMasterRegisterReportCallback();
    TestOtaMasterRegisterQueryCallback();
    TestOtaMasterRegisterRequestCallback();

    return 0; 
}
```

## 7.2 OTA任务触发

本示例代码以KL15 ON作为OTA Master启动标志，具体以项目要求为准；

### 7.2.1 时序图

![image](https://alidocs.oss-cn-zhangjiakou.aliyuncs.com/a/RODKALagxHmn7q2J/4d229b2212ca4710918b303d158545af2439.png)

### 7.2.2 示例代码

```c
// KL15状态变化回调
void OnPowerStatusChanged(uint8_t kl15Sts)
{
    static uint8_t isOtaMasterStart = OTA_MASTER_STOP;
    if (kl15_sts == KL15_ON && isOtaMasterStart != OTA_MASTER_START) {
        // 启动OTA Master
        if (OtaMasterStart() != OTA_RET_OK) {
            OTA_LOG_E("OtaMasterStart failed");
            return;
        }
        isOtaMasterStart = OTA_MASTER_START;
    }
}

// 提取OTA平台下发的OTA任务策略信息
int ExtractOtaTaskPolicyInfo(const uint8_t *data, uint16_t dataLen, StOtaTaskPolicyInfo *otaTaskPolicyInfo)
{
    //具体已以OTA平台协议为准
    otaTaskPolicyInfo->taskId = "1234-abcd-aaa";
    otaTaskPolicyInfo->taskInfo = "test";
    ...
    return OTA_RET_OK;
}

// 接收到来自OTA平台的任务触发消息（这里仅作为示例代码，不代表真实OTA平台任务触发方式）
void OnOtaMessagesReceived(const uint8_t *data, uint16_t dataLen)
{
    // 解析data并确认是OTA消息
    ...

    // 检测当前是否有正在进行的OTA任务
    if (OtaMasterGetOtaStatus() == OTA_IN_PROGRESS) {
        OTA_LOG_E("There is already an ongoing OTA task");
        //TODO 上报OTA任务触发失败，因为有正在进行的OTA任务；
        return;
    }

    StOtaTaskPolicyInfo otaTaskPolicyInfo;
    //TODO 提取OTA任务信息并转换为OTA Master SDK要求的任务策略信息；
    if (ExtractOtaTaskPolicyInfo(data, dataLen, &otaTaskPolicyInfo) != RET_OK) {
        OTA_LOG_E("Ectract OTA task policy info failed");
        //TODO 上报OTA任务触发失败，因为OTA任务信息提取失败；
        return;
    }

    // 任务触发
    if (OtaMasterStartOtaTask(&otaTaskPolicyInfo) != OTA_RET_OK) {
        //失败原因可能是SDK未初始化、任务ID为空或任务信息为空、有正在进行的任务；
        OTA_LOG_E("Start OTA task failed");
    }
}
```

## 7.3 OTA任务暂停及恢复

### 7.3.1 示例代码

```c
// KL15状态变化回调
void OnPowerStatusChanged(uint8_t kl15Sts)
{
    static uint8_t isOtaMasterStart = OTA_MASTER_STOP;
    if (kl15_sts == KL15_OFF && isOtaMasterStart != OTA_MASTER_STOP) {
        // OTA任务暂停
        if (OtaMasterStop() != OTA_RET_OK) {
            // 若OtaMasterStop执行失败，表示OTA Master SDK正在进行ECU刷写或回滚；
            // 等待升级结果回调触发后，再次调用此接口即可正常结束OTA Master；
            // 若SDK正在执行下载动作，会正常中断下载并保留文件，再下次调用OtaMasterStart后会恢复继续下载；（PS：需文件下载服务器支持断点续传）
            OTA_LOG_E("OtaMasterStart failed");
            return;
        }
        isOtaMasterStart = OTA_MASTER_STOP;
        //TODO 删除休眠事件，允许TBox进入休眠
    }
}
```

## 7.4 TBox自升级&回滚说明

### 7.4.1 示例代码

```c
static void *TBoxSelfUpgradeThread(const char *upgradeFileDir)
{
    //TODO TBox自升级
    DoTboxSelfUpgrade(upgradeFileDir);

    //升级完成或失败后通知SDK升级结果
    //result 升级结果：OTA_RET_OK：升级成功；OTA_RET_ERR：升级失败
    //若TBox升级成功，告知SDK是否需要重启TBox
    OtaMasterNoticeTboxSelfUpgradeResult(result, rebootState);
}

static int TboxSelfUpgradeCallback(const char *upgradePath)
{
    OTA_LOG_D("RequestTboxSelfUpgradeCallback upgradePath: %s", upgradePath);
    // TODO 异步执行TBox自升级
    pthread_t upgradeThread;
    CreateThread(&upgradeThread, TBoxSelfUpgradeThread, upgradePath);

    // 返回TBox自升级触发结果；成功返回OTA_RET_OK，失败返回OTA_RET_ERR
    return OTA_RET_OK;
}

static void *TBoxSelfRollbackThread(const char *upgradeFileDir)
{
    //TODO TBox自回滚
    DoTboxSelfRollback(upgradeFileDir);

    //回滚完成或失败后通知SDK回滚结果
    //OTA_RET_OK：回滚成功；OTA_RET_ERR：回滚失败
    //若TBox版本回退成功，告知SDK是否需要重启TBox
    OtaMasterNoticeTboxRollbackResult(result， rebootState);
}

static int TboxSelfRollbackCallback(const char *backupPath)
{
    OTA_LOG_D("RequestTboxSelfUpgradeCallback upgradePath: %s", upgradePath);
    // TODO 异步执行TBox回滚
    pthread_t upgradeThread;
    CreateThread(&upgradeThread, TBoxSelRollbackThread, upgradePath);

    // 返回TBox回滚触发结果；成功返回OTA_RET_OK，失败返回OTA_RET_ERR
    return OTA_RET_OK;
}
```

## 7.5 UDS刷写其他ECU会话过程

libota_master SDK通过UDS CAN诊断协议实现其他ECU升级刷写；UDS诊断刷写协议应视具体项目为准、此示例时序图仅做展示用；

### 7.5.1 时序图

![image](https://alidocs.oss-cn-zhangjiakou.aliyuncs.com/a/RODKALagxHmn7q2J/3932fd138fb046d1bc927e0460dd0f202439.png)
