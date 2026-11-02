# OTA 升级期间休眠事件管理 - 设计文档

## 1. 概述

### 1.1 背景
在 OTA（Over-The-Air）升级过程中，系统需要保持唤醒状态以完成下载、刷写、验证等操作。如果系统在升级过程中意外进入休眠，可能导致升级失败、ECU 变砖等严重后果。

### 1.2 目的
本文档描述 OTA 升级期间休眠事件阻塞机制的设计和实现，确保升级过程中系统不会意外进入休眠状态，并在升级完成后正确释放休眠阻止事件。

### 1.3 适用范围
- 单 ECU 升级路径（通过 `tsp_ota.c` 模块）
- 多 ECU 批量升级路径（通过 `ota_master_state_manage.cpp` 状态机）

---

## 2. 架构设计

### 2.1 模块结构

```
┌─────────────────────────────────────────────────────────────────┐
│                        OTA Master                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────┐    ┌──────────────────────────────────┐   │
│  │   tsp_ota.c      │    │   ota_master_state_manage.cpp    │   │
│  │  (单ECU升级路径)  │    │   (多ECU批量升级路径)             │   │
│  └────────┬─────────┘    └───────────────┬──────────────────┘   │
│           │                              │                       │
│           │  OtaMasterPmSetSleepEvent()  │                       │
│           │  OtaMasterPmCleanSleepEventByType()                  │
│           │                              │                       │
│  ┌────────▼──────────────────────────────▼──────────────────┐   │
│  │              ota_master_pm_helper.c                       │   │
│  │           (电源管理辅助模块)                               │   │
│  └────────┬──────────────────────────────────────────────────┘   │
│           │                                                       │
│           │  PmSetSleepEvent()                                    │
│           │  PmCleanSleepEvent()                                  │
│  ┌────────▼──────────────────────────────────────────────────┐   │
│  │                    PM Server API                           │   │
│  │              (电源管理服务)                                  │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 核心组件

#### 2.2.1 电源管理辅助模块 (`ota_master_pm_helper.c/h`)

| 组件 | 说明 |
|------|------|
| `g_sleepEventSet[ECU_TYPE_MAX]` | 数组，跟踪每个 ECU 类型的休眠事件设置状态 |
| `g_activeEventCount` | 当前已激活的休眠事件数量 |
| `OtaMasterPmGetSleepEventName()` | 获取指定 ECU 类型对应的休眠事件名称 |
| `OtaMasterPmSetSleepEvent()` | 设置指定 ECU 类型的休眠阻止事件（幂等） |
| `OtaMasterPmCleanSleepEvent()` | 清除所有已激活的休眠阻止事件 |
| `OtaMasterPmCleanSleepEventByType()` | 清除指定 ECU 类型的休眠阻止事件 |
| `OtaMasterPmCleanAllSleepEvents()` | 强制清理所有升级休眠事件（异常恢复用） |
| `OtaMasterPmIsSleepBlocked()` | 检查是否有休眠事件正在阻止系统休眠 |
| `OtaMasterPmGetCurrentSleepEvent()` | 获取当前激活的休眠事件名称 |
| `OtaMasterPmGetEcuTypeString()` | 获取 ECU 类型的字符串描述 |

#### 2.2.2 ECU 类型定义

```c
typedef enum {
    ECU_TYPE_TBOX = 0,      /**< TBOX */
    ECU_TYPE_VCU,           /**< VCU (整车控制器) */
    ECU_TYPE_TMS,           /**< TMS (热管理系统) */
    ECU_TYPE_BMS,           /**< BMS (电池管理系统) */
    ECU_TYPE_MCU,           /**< MCU (微控制器) */
    ECU_TYPE_MPU,           /**< MPU (微处理器) */
    ECU_TYPE_OTHER          /**< 其他ECU */
} EcuType_e;
```

#### 2.2.3 休眠事件名称定义

| ECU 类型 | 休眠事件名称 | 宏定义 |
|----------|-------------|--------|
| TBOX | `"upgrade_tbox_event"` | `PM_SLEEP_EVENT_UPGRADE_TBOX` |
| VCU | `"upgrade_vcu_event"` | `PM_SLEEP_EVENT_UPGRADE_VCU` |
| TMS | `"upgrade_tms_event"` | `PM_SLEEP_EVENT_UPGRADE_TMS` |
| BMS | `"upgrade_bms_event"` | `PM_SLEEP_EVENT_UPGRADE_BMS` |
| MCU | `"upgrade_mcu_event"` | `PM_SLEEP_EVENT_UPGRADE_MCU` |
| MPU | `"upgrade_mpu_event"` | `PM_SLEEP_EVENT_UPGRADE_MPU` |
| OTHER | `"upgrade_other_event"` | `PM_SLEEP_EVENT_UPGRADE_OTHER` |

---

## 3. 休眠事件生命周期

### 3.1 单 ECU 升级路径 (`tsp_ota.c`)

```
┌─────────────────────────────────────────────────────────────────┐
│                    单 ECU 升级流程                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  tly_ota_dispatch()                                             │
│       │                                                         │
│       ├──► OtaMasterPmSetSleepEvent(ecuType)  ◄── 设置休眠事件   │
│       │                                                         │
│       ▼                                                         │
│  下载固件                                                        │
│       │                                                         │
│       ├──── 下载失败 ──► ota_download_finish(fail)               │
│       │                        │                                │
│       │                        ├──► OtaMasterPmCleanSleepEventByType() │
│       │                        │                                │
│       │                        ▼                                │
│       │                   清理资源，上报失败状态                    │
│       │                                                         │
│       ├──── 下载成功 ──► 启动刷写线程                             │
│       │                        │                                │
│       │                        ▼                                │
│       │                   ecu_brush_thread_func()               │
│       │                        │                                │
│       │                        ├──── 线程创建失败                │
│       │                        │     │                          │
│       │                        │     ├──► OtaMasterPmCleanSleepEventByType() │
│       │                        │     │                          │
│       │                        │     ▼                          │
│       │                        │   上报失败状态                    │
│       │                        │                                │
│       │                        ├──── 刷写完成                    │
│       │                              │                          │
│       │                              ▼                          │
│       │                         ota_upgrade_finish()            │
│       │                              │                          │
│       │                              ├──► OtaMasterPmCleanSleepEventByType() │
│       │                              │                          │
│       │                              ▼                          │
│       │                         上报升级结果                      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 3.2 多 ECU 批量升级路径 (`ota_master_state_manage.cpp`)

```
┌─────────────────────────────────────────────────────────────────┐
│                  多 ECU 批量升级流程                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  HandleOtaMasterStateUpgrading()                                │
│       │                                                         │
│       ├──► SetSleepEventForEcuTasks(highVoltEcuTasksInfo)       │
│       ├──► SetSleepEventForEcuTasks(lowVoltEcuTasksInfo)        │
│       │                                                         │
│       ▼                                                         │
│  执行升级任务（遍历所有 ECU）                                     │
│       │                                                         │
│       ├──── 下载失败 ──► HandleOtaMasterStateDownloadFailed()   │
│       │                        │                                │
│       │                        ├──► OtaMasterPmCleanAllSleepEvents() │
│       │                        │                                │
│       │                        ▼                                │
│       │                   上报下载失败结果                        │
│       │                                                         │
│       ├──── 升级失败 ──► HandleOtaMasterStateUpgradeFailed()    │
│       │                        │                                │
│       │                        ├──► OtaMasterPmCleanAllSleepEvents() │
│       │                        │                                │
│       │                        ▼                                │
│       │                   上报升级失败结果                        │
│       │                                                         │
│       ├──── 升级成功 ──► HandleOtaMasterStateUpgradeSuccess()   │
│       │                        │                                │
│       │                        ├──► OtaMasterPmCleanAllSleepEvents() │
│       │                        │                                │
│       │                        ▼                                │
│       │                   上报升级成功结果                        │
│       │                                                         │
│       ├──── 回滚 ─────► HandleOtaMasterStateRollback()          │
│       │                        │                                │
│       │                        ├──► SetSleepEventForEcuTasks()  │
│       │                        │   (防御性设置，幂等操作)          │
│       │                        │                                │
│       │                        ▼                                │
│       │                   执行回滚操作                           │
│       │                                                         │
│       ▼                                                         │
│  OtaStateManageDeinit()                                         │
│       │                                                         │
│       ├──► OtaMasterPmCleanAllSleepEvents()                     │
│       │                                                         │
└─────────────────────────────────────────────────────────────────┘
```

---

## 4. API 详细说明

### 4.1 设置休眠事件

```c
int8_t OtaMasterPmSetSleepEvent(EcuType_e ecuType);
```

**功能**：设置指定 ECU 类型的休眠阻止事件

**参数**：
- `ecuType`：ECU 类型（见 ECU 类型定义）

**返回值**：
- `RET_OK (0)`：成功
- `RET_ERR (-1)`：失败

**特性**：
- **幂等性**：如果该 ECU 类型的事件已经设置，直接返回 `RET_OK`，不重复调用 PM API
- **独立跟踪**：每个 ECU 类型独立维护自己的休眠事件，不会互相覆盖
- **计数管理**：内部维护 `g_activeEventCount` 计数器

### 4.2 清除指定类型的休眠事件

```c
int8_t OtaMasterPmCleanSleepEventByType(EcuType_e ecuType);
```

**功能**：清除指定 ECU 类型的休眠阻止事件

**参数**：
- `ecuType`：ECU 类型

**返回值**：
- `RET_OK (0)`：成功
- `RET_ERR (-1)`：失败

**特性**：
- 如果该 ECU 类型的事件未设置，直接返回 `RET_OK`

### 4.3 清除所有休眠事件

```c
int8_t OtaMasterPmCleanSleepEvent(void);
```

**功能**：遍历所有 ECU 类型，清除已设置的休眠事件

**返回值**：
- `RET_OK (0)`：成功
- `RET_ERR (-1)`：部分清除失败

### 4.4 强制清理所有休眠事件

```c
void OtaMasterPmCleanAllSleepEvents(void);
```

**功能**：无论内部状态如何，强制清除 PM 服务器中所有已知的升级休眠事件

**使用场景**：
- 进程启动时的异常恢复
- 确保不会有残留事件阻止系统休眠

### 4.5 检查休眠阻塞状态

```c
int OtaMasterPmIsSleepBlocked(void);
```

**功能**：检查是否有休眠事件正在阻止系统休眠

**返回值**：
- `1`：有事件阻止休眠
- `0`：没有事件阻止休眠

---

## 5. ECU 类型映射

### 5.1 单 ECU 升级路径映射 (`tsp_ota.c`)

| ECU ID | OTA 命令判断 | PM ECU 类型 |
|--------|-------------|-------------|
| 0 | `is_tbox_self_upgrade()` = true | `ECU_TYPE_TBOX` |
| 0 | `is_tbox_self_upgrade()` = false | `ECU_TYPE_OTHER` |
| `ECU_ID_VCU` | - | `ECU_TYPE_VCU` |
| `ECU_ID_TMS` | - | `ECU_TYPE_TMS` |
| `ECU_ID_TCU` | - | `ECU_TYPE_OTHER` |
| `ECU_ID_MCU1` | - | `ECU_TYPE_MCU` |

### 5.2 多 ECU 批量升级路径映射 (`ota_master_state_manage.cpp`)

| OTA ECU 类型枚举值 | PM ECU 类型 |
|-------------------|-------------|
| 0 (`E_OTA_ECU_TYPE_TBOX`) | `ECU_TYPE_TBOX` |
| 1 (`E_OTA_ECU_TYPE_BCM`) | `ECU_TYPE_OTHER` |
| 2 (`E_OTA_ECU_TYPE_BMS`) | `ECU_TYPE_BMS` |
| 3 (`E_OTA_ECU_TYPE_VCU`) | `ECU_TYPE_VCU` |
| 4 (`E_OTA_ECU_TYPE_OTHER`) | `ECU_TYPE_OTHER` |

---

## 6. 注意事项

### 6.1 两条升级路径的独立性

- **单 ECU 升级路径** (`tsp_ota.c`)：适用于通过 TSP 下发的单 ECU 升级指令
- **多 ECU 批量升级路径** (`ota_master_state_manage.cpp`)：适用于批量 ECU 升级任务

两条路径**不应同时执行**，否则可能导致休眠事件设置/清除混乱。

### 6.2 异常恢复

在以下情况下应调用 `OtaMasterPmCleanAllSleepEvents()` 进行强制清理：
- 进程异常退出后重新启动
- 状态机反初始化
- 升级过程中发生未预期的错误

### 6.3 日志记录

所有休眠事件操作都有对应的日志记录：
- 设置成功：`"OTA PM: Set sleep event for %s: %s (active count: %d)"`
- 设置失败：`"OTA PM: Failed to set sleep event for %s"`
- 清除成功：`"OTA PM: Cleaned sleep event for %s: %s (active count: %d)"`
- 清除失败：`"OTA PM: Failed to clean sleep event for %s: %s"`

---

## 7. 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| 1.0 | 2026-07-16 | 初始版本，实现 OTA 升级期间休眠事件阻塞机制 |

---

## 8. 相关文件

| 文件路径 | 说明 |
|----------|------|
| `app/ota_master_lib/src/ota_master_pm_helper.c` | 电源管理辅助模块实现 |
| `app/ota_master_lib/inc/private/ota_master_pm_helper.h` | 电源管理辅助模块头文件 |
| `app/ota_master_lib/src/ota_master_state_manage.cpp` | OTA 状态机（多 ECU 路径） |
| `app/app_tsp/src/tsp_ota.c` | TSP OTA 模块（单 ECU 路径） |
| `sqsyc-c3server/C3_base_line/c3server/srsdk/pm/inc/pm_server_api.h` | PM Server API 定义 |
