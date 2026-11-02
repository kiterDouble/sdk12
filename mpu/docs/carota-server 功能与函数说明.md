文档1

carota_server_function_summary.md

# carota-server 功能与函数说明

> 代码路径：`app/carota-server` 编译产物：`carota_server`（安装到 `${CMAKE_INSTALL_PREFIX}/app/`） 文档基于当前工作区代码整理，行号对应当前版本。

## 1. 模块定位

`carota-server` 是运行在 MPU（TBOX 主控）上的 **OTA 本地服务端**：

- 基于 **CivetWeb** 在 `20003` 端口提供 HTTP 接口，供柯洛达（Carota）OTA Master / 九识云端适配层调用；
- 管理 **FOTA/DOTA 模式进入退出**、整车上下电与唤醒、CAN TP 锁、看门狗喂狗；
- 管理 **TBOX 自身及各 ADCU 模块（zoe/zoe-conf/…/uvm）的版本信息**（软件版本、硬件版本、Bootloader、SN、零件号、供应商、version_id）；
- 接收 ECU 升级请求、下发升级 URL、生成 SHA/版本参数，执行 TBOX 升级包并上报结果；
- 与 PM（电源管理）、MCU、Modem、MQTT/OTA 服务通过 SDK IPC 消息交互；
- 主/副模组之间做域名切换请求转发、休眠状态互通知。

运行环境路径约定：

| 路径                                             | 用途                                       |
| ------------------------------------------------ | ------------------------------------------ |
| `/mnt/emmc/upgrade/status`                       | UCU 当前状态（0 正常/1 FOTA/2 OBD/3 DOTA） |
| `/mnt/emmc/upgrade/ethernet/`                    | TBOX 升级包暂存目录                        |
| `/oemdata/parameters/MainModemUpgradeStatus`     | 主模组升级状态（0~4）                      |
| `/oemdata/parameters/kl15_status`                | 进入 OTA 前的 KL15 快照                    |
| `/oemdata/parameters/paramconfig.ini`            | ParamManager 持久化参数                    |
| `/oemdata/parameters/paramsDb`                   | SQLite 参数库（GetParam/UpdateParam）      |
| `/oemdata/tmp/tbox_upgrade_status.txt`           | 升级启动标记（`1`）                        |
| `/oemdata/tmp/tbox_upgrade_result.txt`           | 升级结果（`2` 成功）                       |
| `/oemdata/tmp                                    | flash/ucu_upgrade_result.txt`              |
| `/oemapp/upgrade/version.txt`、`version_uds.txt` | TBOX 版本串                                |
| `/oemapp/upgrade/ethernet_upgrade`               | TBOX 升级执行程序                          |

## 2. 文件清单

| 文件                                                         | 行数 | 职责                                                        |
| ------------------------------------------------------------ | ---- | ----------------------------------------------------------- |
| `src/main.c`                                                 | 3438 | 程序入口、HTTP 路由与全部 handler、IPC 回调、升级后状态自检 |
| `src/ucu_status.c`                                           | 320  | UCU/网关状态机、CAN TP 锁管理                               |
| `src/vehicle_status.c`                                       | 268  | 整车 CAN 信号缓存与查询接口                                 |
| `src/ucu_upgrade.c`                                          | 555  | TBOX 升级线程、升级包查找/执行、结果查询                    |
| `src/carotaFilehandle.c`                                     | 476  | 各类状态/版本/KL15 文件读写                                 |
| `src/carotaHttpClient.c`                                     | 634  | 本地 20005/20006、云端鉴权与域名转发 HTTP 客户端            |
| `src/carotsleephandle.c`                                     | 61   | 休眠状态缓存与 FOTA/DOTA 休眠联动线程                       |
| `src/carota_type.c`                                          | 144  | ParamManager 参数表（模块版本信息表）                       |
| `inc/carota_type.h`                                          | 152  | 参数 ID 枚举（`CAROTA_PARAM_ID_DEF`）                       |
| `inc/param_manager.h` / `param_types.h` / `param_config.h` / `param_adapter.h` / `param_version.h` | —    | 参数管理抽象层、类型、适配层、版本信息                      |
| `inc/iniparser.h`                                            | 134  | 简化版 INI 解析器接口                                       |
| `inc/ucu_status.h` / `vehicle_status.h` / `ucu_upgrade.h` / `carotaFilehandle.h` / `carotaHttpClient.h` / `carotsleephandle.h` | —    | 各模块对外接口                                              |
| `libs/libparammanager.so`                                    | —    | 参数管理库（预编译）                                        |
| `CMakeLists.txt`                                             | 103  | 构建脚本                                                    |

> 注：仓库中 `utils/app/carota-server` 是旧副本，与 `app/carota-server` 在 6 个文件中存在差异；本文档只描述 `app/carota-server`。

## 3. 构建与依赖（`CMakeLists.txt`）

- 目标：`add_executable(carota_server ...)`，安装到 `${CMAKE_INSTALL_PREFIX}/app/`。
- 源码包含：`cjson_com/cJSON*.c`、`mcuInterface/mcuinte.c`、`mcuinterbase.c` 以及本目录 7 个 `.c`。
- 头文件路径：`inc`、`cjson_com/inc`、`external/civetweb/include`、`server/service/pm/inc`、`mcuInterface/inc`。
- 链接：`curl`、`ssl/crypto`、`sqlite3`、`glib-2.0/gio/gobject`、`sr-hal/sr-ipc/sr-sdk`、`civetweb`、`libparammanager.so`、`pthread/rt/dl`。
- 通过 `PARAM_LIB_PATH` 可替换参数库位置；按 `MODEMTYPE`（`AG591` / `ag35cet`）分两套链接列表。

## 4. 启动流程（`main.c:3281`）

1. `LOG_INIT("carota")`、`InitSRSdk()`、`mg_init_library(0)`。
2. `ParamManagerInit("/oemdata/parameters/paramconfig.ini", NULL)`。
3. 注册参数表：`TBzoeStatus`、`TBcomputingKernelStatus`（main.c:3311-3312）。
4. `StartQueueMsgRcvThread("carotaServer")` 启动 IPC 收包线程，随后订阅消息（见 5.1.4）。
5. `UcuUpgradeinit()` 启动升级管理线程。
6. `mg_start` 启动 CivetWeb：端口 `20003`，`request_timeout_ms=10000`，日志 `error.log`，关闭 auth domain check。
7. 创建 `carotaTimerThread`（休眠联动线程）。
8. 注册全部 HTTP 路由（main.c:3354-3422）。
9. `VehicleStatusInit()` 订阅整车 CAN 数据。
10. `resetWdTimeout()`（60s 初始看门狗窗口）。
11. `g_main_loop_new/run`，并以 `g_timeout_add(2000, CheckAndHandlePowerOffUpgradeStatus, NULL)` 每 2s 检查一次“异常断电后升级状态恢复”。

## 5. 逐文件函数说明

### 5.1 `src/main.c`

#### 5.1.1 主要全局状态

| 变量                                                         | 说明                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| `g_mcuSyncKine_car`                                          | MCU 同步的 KL15/KL30/CAN 网络状态（`MCUSYNCKINE_tt`）        |
| `g_prepare_data`                                             | MCU 上报的柯洛达整车数据（`NEWMCUMPUDATA_t`：档位、车速、EPB、充电、电源模式、门状态、设防、VCU 升级标志等） |
| `g_hwVersion[32]`                                            | MCU 同步的硬件版本                                           |
| `g_modemSrvNotify`                                           | Modem 网络注册状态                                           |
| `g_pmStatus`                                                 | PM 电源状态                                                  |
| `g_modeChangeTimeout`                                        | 模式切换结果查询延时（2s）                                   |
| `OatFotaStatus`/`OatDotaStatus`                              | 环境切换（uat/prod）异步结果                                 |
| `dotamoderesult`                                             | DOTA 进入模式结果                                            |
| `sha_num`/`ota_file_path`/`package_version`                  | 由 `/ecuupgrade` 解析出的升级包信息                          |
| `carota_adcu_tv_no`                                          | ADCU 包序号（`tv_no`）                                       |
| `mcu_power_status`                                           | MCU 电源状态（等待 `9` 视为就绪）                            |
| `reboot_num`                                                 | 标记“最近一次 `/ecuupgrade` 是 TBOX 升级”，用于结果查询      |
| `temp_kl15`/`change_power`                                   | KL15 边沿检测、电源状态查询值                                |
| `g_is_waiting_for_b`/`g_pending_a_conn`/`g_b_notify_cond`/`g_b_response` | 预留的 A/B 双端等待机制（未实际使用）                        |
| `g_wdTimeout`/`g_wdrdsTimeout`/`g_wakeoutTimeout`            | 看门狗/唤醒超时时间戳（仅重置，无消费者）                    |

#### 5.1.2 URI 常量与接口注册

`main.c:104-209` 定义全部 URI 宏；`main.c:3354-3422` 完成注册。接口总表：

| URI                             | 方法 | Handler                         | 功能                                                         |
| ------------------------------- | ---- | ------------------------------- | ------------------------------------------------------------ |
| `/getinfo?dev=xxx`              | GET  | `GetInfoHandler`                | 查询模块版本信息（sv/hv/blv/sn/partnum/supplier/version_id） |
| `/getvin`                       | GET  | `GetVinHandler`                 | 返回 VIN 和车辆编码 VN                                       |
| `/prepare`                      | GET  | `PrepareHandler`                | 升级前置条件：档位、车速、手刹、充电、电压、VCU 可升级标志   |
| `/ecuupgrade?dev=xxx`           | POST | `EcuUpgradeHandler`             | 下发升级包 URL/SHA/版本，TBOX 则直接触发升级                 |
| `/ecuupgraderesult?dev=xxx`     | GET  | `EcuUpgradeResultHandler`       | 查询升级结果                                                 |
| `/resetwd`                      | GET  | `ResetWDHandler`                | 喂狗（60s）                                                  |
| `/rdsresetwd`                   | GET  | `ResetRDSWDHandler`             | RDS 喂狗（60s）                                              |
| `/modein`                       | GET  | `ModeInHandler`                 | 进入 FOTA 模式（写 UCU 状态=1）                              |
| `/modeinresult`                 | GET  | `ModeInResultHandler`           | 进入 FOTA 结果查询                                           |
| `/modeout`                      | GET  | `ModeOutHandler`                | 退出 FOTA，恢复 NORMAL，并恢复 KL15 电源状态                 |
| `/modeoutresult`                | GET  | `ModeOutResultHandler`          | 退出 FOTA 结果查询                                           |
| `/ready`                        | GET  | `ReadyHandler`                  | 环境就绪检查（VIN、VIN 非全 0、网络注册、年份 2025~2100）    |
| `/wake`                         | GET  | `WakeHandler`                   | 唤醒整车（含 MCU 上电、PM 事件）                             |
| `/wakeout`                      | GET  | `WakeoutHandler`                | 取消唤醒、清除休眠阻塞事件                                   |
| `/rtctime`                      | GET  | `RtctimeHandler`                | 返回当前时间戳 `t`                                           |
| `/alarm`                        | POST | `SETTIMEHandler`                | 解析预约时间（仅解析，未实际设置）                           |
| `/alarmcancel`                  | GET  | `SETTIMECANCELHandler`          | 清除预约/休眠事件                                            |
| `/tspmessage`                   | POST | `TspMessageHandler`             | 转发 TSP 消息到 `127.0.0.1:20005`，必要时先唤醒整车          |
| `/syncotafield`                 | POST | `SYNCTSPHandler`                | 将请求体通过 UDP 转发 `127.0.0.1:5010`，返回响应 JSON        |
| `/setotafield?dev=tbox`         | POST | `SetOtaFieldHandler`            | 转发设置 OTA 字段到 20005                                    |
| `/cantplock`                    | GET  | `CantpLockHandler`              | FOTA/DOTA 下锁 CAN TP                                        |
| `/cantpunlock`                  | GET  | `CantpUnlockHandler`            | 解锁 CAN TP                                                  |
| `/prepareinstall`               | POST | `PREPAREINSTALLHandler`         | 通知 MCU 准备 ADCU 安装（`Set_ecu_adcu(1/2)`）               |
| `/vehiclestatus`                | GET  | `CARSTATUSHandler`              | 电源模式、门状态、设防状态                                   |
| `/querypowerStatus`             | GET  | `POWERSTATUSHandle`             | 查询 KL15 上电变化标志                                       |
| `/resetPowerStatus`             | GET  | `RESETSTATUSHandle`             | 清零电源变化标志                                             |
| `/dotamodein`                   | GET  | `INDOTAMODETHandler`            | 进入 DOTA 模式（UCU 状态=3）                                 |
| `/dotamodeinresult`             | GET  | `QUERYRESULTOFDOTAMODETHandler` | 进入 DOTA 结果查询                                           |
| `/dotamodeout`                  | GET  | `DOTAMODEOUTHandler`            | 退出 DOTA，恢复 NORMAL，必要时整车下电                       |
| `/dotamodeoutresult`            | GET  | `DOTAMODEOUTRESULTHandler`      | 退出 DOTA 结果查询                                           |
| `/serverswitch`                 | POST | `SERVERSWITCHTHandler`          | 服务器环境切换，广播 `SYS_QUE_SERVER_SWITCH`                 |
| `/TBOXSetDomain`                | POST | `SetDomainHandler`              | FOTA/DOTA 环境（uat/prod）切换，并向对端模组转发             |
| `/getDomainParam`               | GET  | `GetDomainParamHandler`         | 返回 226/227 参数供副模组同步                                |
| `/notifyTBOXCouldSleep`         | POST | `notifyTboxSleepHandler`        | 依据 `platform`+`r` 设置/清除 PM 休眠事件                    |
| `/api/ota/adcu/info/{name}`     | GET  | `ADCU_SETMSG_QUEHandler`        | 查询指定 ADCU 的升级信息（URL/SHA/版本）                     |
| `/api/ota/adcu/info/{name}/set` | POST | `ADCU_SETMSG_QUEHandler`        | 批量写入 ADCU 版本信息参数                                   |
| `/download/*`                   | GET  | `FileDownloadHandler`           | 升级包文件下载                                               |
| `/testdebug`                    | GET  | `TESTDEBUG_URIHandler`          | 参数批量读写调试                                             |
| `/srUpgradeStatus`              | GET  | `SRUPGRADE_URLhandler`          | 读取 MQTT 升级状态并映射为 1/2/3/4                           |
| `/setotafield`                  | POST | `SetOtaFieldHandler`            | 同 `/setotafield?dev=tbox`                                   |

已定义但**未注册**的 URI 宏：`/fotagetchallenge`、`/fotasendkey`、`/hvup`、`/hvupresult`、`/hvdown`、`/hvdownresult`、`/hvoin`、`/hvoout`、`/hvoinresult`、`/hvooutresult`、`/remotecommand`、`/tspwake`、`/notifyFOTASleep`。

#### 5.1.3 通用/辅助函数

| 函数                                                         | 说明                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| `SendJSON(conn, json_obj)` (663)                             | `cJSON_PrintUnformatted` + `mg_send_http_ok` 发送 JSON       |
| `parse_uri(uri, ota_name, len, is_set)` (2581)               | 解析 `/api/ota/adcu/info/{name}[/set]`，返回 name 与是否 set 请求 |
| `log_message()` (2002)                                       | CivetWeb 日志回调，直接 `puts`                               |
| `GetVehMisModStatus()` (541)                                 | 返回 `g_prepare_data.AD_upgradeCheckStatus`（供 `ucu_upgrade` 判断是否可升级） |
| `resetWdTimeout()` / `resetRDSWdTimeout()` / `resetWakeoutTimeout()` | 重置 60s 超时                                                |
| `CheckAndHandlePowerOffUpgradeStatus()` (3170)               | 2s 周期定时器：读取 `MainModemUpgradeStatus`，若为“升级中”则调用 `ConcurrentOtaGetAllUpgradeStatus` 校验 MCU/MPU 分区状态，成功写 2、失败/超时写 3，并 `SrTboxSystemReboot` 重启；最多 300 次（约 10 分钟） |

#### 5.1.4 IPC 回调与订阅关系

| 回调                            | 订阅消息                                             | 作用                                                         |
| ------------------------------- | ---------------------------------------------------- | ------------------------------------------------------------ |
| `KL15INEStatusCallback` (3083)  | `SYS_QUE_TYPE_MCU_SYNC_KLINE_SGMW`                   | 解析 KL15(`[0]`)、KL30(`[1..2]`)、CAN 网络(`[7]`)；KL15 0↔1 时更新 `change_power` |
| `MCUPOWERStatusCallback` (3117) | `SYS_QUE_TYPE_MCU_SYNC_POWER_STATUS`                 | 取 `[2]` 作为 `mcu_power_status`                             |
| `hwversionCallback` (3138)      | `SYS_QUE_TYPE_MCU_VEHICLE_CTROL_VERSION_SYNC_TO_MPU` | 解析变长 TLV，取 `recvData[4]+6` 偏移后的字符串作为硬件版本  |
| `CarotaPmPowerState` (2068)     | `SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY`                 | 同步 PM 状态到 `carota_pmState`                              |
| `CarotaModemSrvInfo` (2008)     | `SYS_QUE_TYPE_MODEM_SIGNAL_NOFIFY`                   | 缓存 Modem 注册状态                                          |
| `OtaDotarecvSrvInfo` (2022)     | `0x002f0003`                                         | 接收 DOTA 环境切换结果                                       |
| `OtaFotarecvSrvInfo` (2036)     | `0x002f0001`                                         | 接收 FOTA 环境切换结果                                       |
| `VcuUpGradeInfo` (2050)         | `0x00011900`                                         | 缓存 MCU 整车/VCU 状态（`g_prepare_data`）                   |

主动发送的 IPC 消息：

| 消息                                       | 用途                                           |
| ------------------------------------------ | ---------------------------------------------- |
| `0x002f0002`                               | FOTA 环境切换命令（1=uat，2=prod）             |
| `0x002f0004`                               | DOTA 环境切换命令（1=uat，2=prod）             |
| `SYS_QUE_SERVER_SWITCH`(0x191046046)       | 服务器环境切换广播                             |
| `SYS_QUE_TYPE_SGMW_GET_OBD_STATUS`         | 通知 MCU 更新 UCU 状态（payload 为 GW 模式值） |
| `0x00024008` / `0x00024009` / `0x00024047` | 远程上电 / 下电 / 重启（`mcuinte.c`）          |
| `0x00033000` / `0x00033100`                | ADCU 安装准备 / DA 模式设置                    |

#### 5.1.5 主要 Handler 细节

**`GetInfoHandler` (681)** — 版本信息核心接口

- 校验 `dev` 必须在 `ALLOWED_DEVS[15]` 中，计算 `sql_rq_num = 105 + i*8`；`dev=="tbox"` 时为 `104 + i*8 = 208`。
- 非 tbox 分支：再用 `valid_names[13]` 求 `module_index`，`base_pid = module_index*8`，通过 `TBParameterAdapterStringGetByIDList` 批量取 6 个字符串（`+1..+6`），`TBParameterAdapterGetByIDList` 取 `+7` 的 `version_id`。
- tbox 分支：`GetParam` 读 208/15(hw)/16(blv)/3(sn)/19(partnum)/20(supplier)，软件版本由 `CarotaFileVersionGetVersion` 从 `version_uds.txt` 提取。
- 返回 `{dev_sv, dev_hv, dev_blv, dev_sn, dev_partnum, dev_supplier, version_id}`；进入/退出时分别 `PmSetSleepEvent("carota")` / `PmCleanSleepEvent("carota")` 阻止休眠。

**`EcuUpgradeHandler` (1115)** — 升级请求下发

- 先 `cp version.txt` 到 ethernet 目录，`reboot_num=1`。
- 解析 POST：`config`（URL 目录）、`version`、`summary`（SHA256）、`filename`、可选 `tv_no`。
- 拼接 `ota_file_path = http://192.168.1.201:20003/download{config}/{filename}`。
- ADCU 分支：`UpdateParam(sql+7, url)`、`UpdateParam(sql+0, version)`、`UpdateParam(sql+6, sha)`；`adcu` 额外写 `sql+8` 文件名。
- TBOX 分支（`sql_rq_num==208`）：`UpdateParam(215, url)`，写 `tbox_upgrade_status.txt=1`，调用 `UcuUpgradeStart(config)` 启动升级线程。

**`EcuUpgradeResultHandler` (1255)**

- `dev=="tbox"`：`ReadSMainModemUpgradeStatus()` → `-1` 失败、`1` upgrading、`2` success（若 `reboot_num!=0` 仍返回 upgrading）。
- 其他 dev：固定返回 success/upgrading（依据 `reboot_num`），不查真实结果。

**`ModeInHandler` / `ModeOutHandler`**

- ModeIn：若文件状态已是 DOTA 则拒绝；否则写 FOTA、保存 KL15、设置 `PM_SLEEP_EVENT_UPGRADE`、`Set_da_mode(1)`，并设定 2s 后查询窗口。
- ModeOut：读回 KL15；若当前是 FOTA，清 UCU 状态→NORMAL，并依据 KL15 快照执行 `ControlRemotePowerDown()` 或 `ControlrestartPowerUp()`；`Set_da_mode(3)`。

**`WakeHandler` / `WakeoutHandler`**

- Wake：KL15=0 时设置唤醒 TBOX/整车事件、`PmWakeupSetDevices("mcu",1)`，循环 `ControlRemotePowerUp()` 直到 KL15≠0 或 10 次超时（超时返回 `r=-1`）。
- Wakeout：清除 `PM_SLEEP_EVENT_WAKEUPCAR`、`PM_SLEEP_EVENT_UPGRADE`，清零唤醒超时。

**`notifyTboxSleepHandler` (275)**

- 解析 `platform`(fota/dota) 与 `r`；`r==0` → `PmSetSleepEvent(...)` 阻止休眠；`r==1` → `PmCleanSleepEvent(...)` 允许休眠。

**`SetDomainHandler` (352)**

- 解析 `target`(uat/prod)、`platform`(fota/dota)，向 `0x002f0002`/`0x002f0004` 广播命令 `1=uat, 2=prod`，最长等待 30s 结果。
- 之后若请求未带 `forwarded=1`，调用 `ForwardDomainSwitchRequest` 转发给对端模组，避免主副循环。

**`ADCU_SETMSG_QUEHandler` (2623)**

- `parse_uri` 区分查询/写入。
- 写入：按字段名映射到 `base_pid + offset`（`ota_name=0, name_sv=1, name_hv=2, name_blv=3, name_sn=4, name_partnum=5, name_supplier=6, version_id=7`），字符串批量 `TBParameterAdapterStringSetByIDList`，整型 `TBParameterAdapterSetByIDList`。
- 查询：`GetParam(sql_num+1/+6/+7)` 返回 `ota_version`、`summary`、`url`，附 `ota_name`、`status`（依据 `VCU_UpdateFlag`）、`version_id`。

**`FileDownloadHandler` (2945)**

- 以 `/download` 为前缀，拒绝含 `..` 的路径，`stat` 校验普通文件后以 `application/octet-stream` 分块 8KB 发送。

**`TspMessageHandler` (546)**

- 若 KL15=0 先唤醒 MCU/整车（等待 `mcu_power_status==9`、`ControlRemotePowerUp()`）；随后把请求体转发到 `127.0.0.1:20005/setotafield?dev=tbox`，包装为 `{subfun:"carota", message:"<响应>"}`。

**`SYNCTSPHandler` (1778)**

- UDP 发送请求体到 `127.0.0.1:5010`，9s 接收超时，响应按 JSON 解析后返回。

**其余 Handler 摘要**

| Handler                                   | 关键行为                                                     |
| ----------------------------------------- | ------------------------------------------------------------ |
| `GetVinHandler`                           | `GetParam(E_ParamId_VIN)` + `GetParam(102)` 返回 `{vin, vn}` |
| `PrepareHandler`                          | 返回 `gear/speed/hb/charge_sts/voltage/vcu_result`；电压由 `pwr_1/pwr_2` 计算，车速 `*0.1-100` 后除以 3.6 |
| `SRUPGRADE_URLhandler`                    | `ReadMqttUpgradeStatus()`，`-1` 转为 3，返回 `{UpgradeStatus}` |
| `TESTDEBUG_URIHandler`                    | 批量读取 zoe 8 个参数，写入两个测试值后再读，仅调试          |
| `ReadyHandler`                            | VIN 长度 17、非全 0、Modem 已注册、年份 2025~2100            |
| `RtctimeHandler`                          | 返回 `{t: time(NULL)}`                                       |
| `CantpLockHandler` / `CantpUnlockHandler` | 依据文件状态选 FOTA/DOTA 锁类型，返回 `{locked/unlocked, err, msg}` |
| `SETTIMEHandler`                          | 解析 `timestamp`、`eventduration`（当前仅解析，未落盘）      |
| `SETTIMECANCELHandler`                    | 清 `"Carota_OTA"` 与 `PM_SLEEP_EVENT_WAKEUPTBOX`             |
| `CARSTATUSHandler`                        | 返回 `powermode`（1/2 归一为 1）、`door_state`、`arming_state` |
| `ResetRDSWDHandler`                       | 重置 RDS 喂狗时间并直接 200                                  |
| `INDOTAMODETHandler`                      | FOTA 中拒绝；否则置 DOTA、保存 KL15、设置升级事件、`Set_da_mode(5)` |
| `QUERYRESULTOFDOTAMODETHandler`           | 2s 窗口后按文件状态返回 `r=1/-1/0`，`Set_da_mode(6)`         |
| `DOTAMODEOUTHandler`                      | 清 DOTA→NORMAL；KL15 快照为 0 时循环 `ControlRemotePowerDown()`；`Set_da_mode(7)` |
| `DOTAMODEOUTRESULTHandler`                | 2s 窗口后返回 `r`，清 `UPGRADE/WAKEUPCAR/WAKEUPTBOX`，`Set_da_mode(8)` |
| `PREPAREINSTALLHandler`                   | 解析 `type`（1→`Set_ecu_adcu(1)`，0→`Set_ecu_adcu(2)`）      |
| `SERVERSWITCHTHandler`                    | 解析 `env`，广播 `SYS_QUE_SERVER_SWITCH`                     |
| `GetDomainParamHandler`                   | 返回 `carDotaConfigMode`(226)/`carFotaConfigMode`(227)       |
| `POWERSTATUSHandle` / `RESETSTATUSHandle` | 读/清 `change_power`                                         |
| `SetOtaFieldHandler`                      | 转发请求体到 20005，返回目标响应或 500                       |
| `TESTDEBUG_URIHandler`                    | 仅调试用，无鉴权                                             |

### 5.2 `src/ucu_status.c`

| 函数                                                         | 说明                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| `UcuStatusMcuMsgProc` (44)                                   | MCU 消息回调：`data[0]` 低 4 位=UCU 状态、高 4 位=GW 状态；状态变化时先回调 `g_changedCb(old,new)` 再更新 `g_ucuStatus` |
| `checkLockCondition(type)` (92)                              | FOTA 锁要求当前为 NORMAL/FOTA；DOTA 锁要求 NORMAL/DOTA       |
| `UcuStatusSetStatus(status)` (116)                           | 校验范围与“当前必须 NORMAL”，转换为 GW 模式后通过 `McuSrvSendMesg(SYS_QUE_TYPE_SGMW_GET_OBD_STATUS)` 通知 MCU；**不更新本地 `g_ucuStatus`**（等 MCU 回读） |
| `UcuStatusClearStatus(status)` (160)                         | 仅当 `status==g_ucuStatus` 时下发 NORMAL                     |
| `UcuStatusGetStatus` / `GWStatusGetStatus` / `RpcGetVehicleStatus` | 加锁读取 UCU/GW/整车状态                                     |
| `UcuStatudCantpLock/Unlock/GetCantpLockType`                 | CAN TP 锁的加锁/解锁/查询（`g_canLockType`）                 |
| `UcuStatusChangedCheck` (278)                                | 状态变化联动：NORMAL→OBD 直接解锁；状态与锁类型不匹配时启动 5s 变更窗口 |
| `CanLockCheckTimeout` (300)                                  | 超时后把锁类型重置为 `CAN_LOCKED_NONE`                       |
| `UcuStatusInit(Cb)` (314)                                    | 复位状态、订阅 `SYS_QUE_TYPE_SGMW_GET_OBD_STATUS`、注册变化回调 |

> 注意：`main()` 中**没有**调用 `UcuStatusInit`，因此运行时 UCU 状态主要由文件 `/mnt/emmc/upgrade/status` 驱动；MCU 状态机与文件状态可能不一致。

### 5.3 `src/vehicle_status.c`

- 内部结构 `VehicleStatus` 缓存车速、SOC、电压、Ready 状态、充电状态、慢充 CC、档位、钥匙、手刹、设防、快充 CC2、`vcu_ota`。
- `VehicleStatusHandler`（170）：校验长度 `sizeof(Data_Carota_canVal_t)` 后整包 `memcpy`，映射字段：
  - 档位：0=N,1=D,2=R,3=P；新矩阵 10=P,11=S,12=D,13=N,14=R；
  - 钥匙：`KyPstn` 1=ACC、2=ON、其他=OFF；
  - `arming=AnThWaSt`、`hb=HandBrkSts`、`cc2Status=BMSCC2Sts`。
- 查询接口：`VehicleStatusGetSpeed/SOC/Voltage/Gear/Engine/Power/Charge/HB/Fota_GetFastCharge/ARMING/OTA`。
  - `VehicleStatusGetPower`：`vecStatRdy==1` → `"Ready"`，否则 `"NotReady"`；
  - `VehicleStatusGetHB`：手刹值取反（`1→0, 其他→1`）。
- `VehicleStatusInit`（264）：订阅 `Data_Carota_canVal_t_msgid`。

### 5.4 `src/ucu_upgrade.c`

| 函数                                                        | 说明                                                         |
| ----------------------------------------------------------- | ------------------------------------------------------------ |
| `GetSystemRetValue(cmd)` (46)                               | `system()` 封装，返回 `WEXITSTATUS` 或 `RET_ERR`             |
| `GetResultFromFile(path)` (66)                              | 读取文件首行，包含 `"success"` 视为成功                      |
| `WriteOkFile` / `DeleteOkFile` / `CheckOkFile` (95/101/107) | 写/删/查 `/oemdata/tmp                                       |
| `StartUpgradePkg(realpath, uflag, errlog)` (117)            | 打开目录查找前缀 `tbox` 的文件；`rename` 到 `/mnt/emmc/upgrade/ethernet/`；清理 `/oemdata/upgrade`、执行 `upgrade_link.sh`；建立 `tbox_target.tgz` 链接；执行 `/oemapp/upgrade/ethernet_upgrade <filename>`；失败时 `WriteSMainModemUpgradeStatus(3)` |
| `trimWhitespace` (204)                                      | 去除首尾空白                                                 |
| `updateVersionWithCmd()` (220)                              | 解析 `version.txt`（`_` 分隔，≥6 段），更新 SQLite `paramsDb` 的 14(软件版本)/19(零件号)/20(供应商)/16(固定 bootloader)。**当前未被调用** |
| `UcuUpgradeHandler` (310)                                   | 升级线程状态机：IDLE→sleep；DOING→写进度 0、可选 `CheckVehicleAllowUpgrade()` 云端许可、写状态 1、执行 `StartUpgradePkg`、成功写结果文件；FINISH→等待重启 |
| `UcuUpgradeStart(filepath)` (373)                           | 设置 `g_u8updateFileName` 并置 DOING                         |
| `ReadTboxUpgradeResult(errlog)` (395)                       | 读 `/oemdata/tmp/tbox_upgrade_result.txt`，严格转数字        |
| `UcuUpgradeGetResult(errlog)` (474)                         | 按内部状态返回：0 成功/1 进行中/-1 失败，并回填日志          |
| `UcuUpgradeinit()` (522)                                    | 若存在成功标记则置 FINISH+成功（90s 窗口），否则 IDLE；创建 detached 线程 |

状态常量：`IDLE=0`、`DOING=1`、`FINISH=2`；结果 `FAIL=-1`、`SUCCESS=0`、`DOING=1`。

### 5.5 `src/carotaFilehandle.c`

| 函数                                          | 说明                                                         |
| --------------------------------------------- | ------------------------------------------------------------ |
| `UcuStatusFileGetStatus(int*)` (20)           | 读 `/mnt/emmc/upgrade/status`（十进制）；文件不存在/空/非法→`UCU_STATUS_NORMAL` |
| `create_directory_if_not_exists(path)` (79)   | 逐级 `mkdir(0755)`，已存在忽略                               |
| `UcuStatusFileSetStatus(status)` (119)        | 确保目录存在，`O_TRUNC` 写入 `"%d\n"` 并 `fsync`             |
| `read_status()` (160)                         | 读 `/oemdata/tmp/tbox_upgrade_status.txt`：`"1"→1`、`"2"→0`、`"-1"→-1`、其他/失败→-2 |
| `ReadSMainModemUpgradeStatus()` (198)         | `cat` `/oemdata/parameters/MainModemUpgradeStatus`，0~4 合法；`1/4→1(升级中)`、`2→2(成功)`、其他→-1 |
| `ReadMqttUpgradeStatus()` (254)               | 同文件；`1→1`、`2→2`、`4→4`，其他→-1                         |
| `WriteSMainModemUpgradeStatus(status)` (315)  | 校验 0~4 后 `echo` 写回                                      |
| `CarotaFileVersionGetVersion(buf,size)` (338) | 读 `/oemapp/upgrade/version_uds.txt`，按 `_` 分割取第 3 段前 14 字符作为版本号；`size<14` 直接失败 |
| `ReadKl15StatusFromFile(uint8_t*)` (412)      | 读 `/oemdata/parameters/kl15_status`（`SCNu8`）              |
| `SaveKl15StatusToFile(v)` (439)               | 确保目录存在、删除旧文件后写入十进制值                       |

### 5.6 `src/carotaHttpClient.c`

| 函数                                                         | 说明                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| `curl_write_callback1` (14)                                  | 动态 `realloc` 追加响应数据（`CurlResponse{data,len}`）      |
| `ForwardRequestToPort20005` (32)                             | POST `127.0.0.1:20005/setotafield?dev=tbox`，JSON，超时 15s  |
| `NotifyFOTASleepMode(mode)` (89)                             | POST `127.0.0.1:20005/notifyFOTASleep`，body `{"mode":0      |
| `NotifyDOTASleepMode(mode)` (141)                            | POST `127.0.0.1:20006/notifyDOTASleep`，超时 35s             |
| `SendDOTAChangeDomainRequest()` (193)                        | GET `127.0.0.1:20006/notifyDOTASleep`，解析 `{"r":0}` 判定成功 |
| `SendAuthTokenRequest(serialNo, vehicleName, tokenBuf, len, vehicleId)` (283) | POST [`https://auth.zelostech.com.cn/vehicle/accessToken`](https://auth.zelostech.com.cn/vehicle/accessToken)，解析 `success/data.token/data.id` |
| `SendAllowUpgradeRequest(vehicleId, token)` (410)            | GET [`https://gateway.zelostech.com.cn/admin-server/vehicle/allowUpgrade/{vehicleId}`](https://gateway.zelostech.com.cn/admin-server/vehicle/allowUpgrade/{vehicleId})，Header `token: xxx`；`data==true→0`，否则 `1` |
| `CheckVehicleAllowUpgrade()` (516)                           | 取参数 102(vehicleName)、4(serialNo) → 申请 token → 查询是否允许升级；任一失败返回 1 |
| `ForwardDomainSwitchRequest(body)` (556)                     | POST `192.168.1.202:20003/TBOXSetDomain?forwarded=1`，主/副模组选择依据 `access("/dev/mmcblk0")`；HTTP 200 视为成功 |

### 5.7 `src/carotsleephandle.c`

- `CarotaGetPmState` / `CarotaSetPmState`：读写锁保护的 `carota_pmState`（初值 `PM_STATE_SLEEP`）。
- `carotaTimerThread`（33）：1s 轮询——
  - 状态 `PM_STATE_SLEEP` 且未通知过：`NotifyFOTASleepMode(0)`、`NotifyDOTASleepMode(0)`（唤醒对端 OTA 服务）；
  - 状态 `PM_STATE_NORMAL` 且已通知过：`NotifyFOTASleepMode(1)`、`NotifyDOTASleepMode(1)`，并 `SendDOTAChangeDomainRequest()`。

> 语义上“休眠时通知 0（唤醒）”与注释存在矛盾，属待确认逻辑。

### 5.8 `src/carota_type.c` 与 `carota_type.h`

- `carota_type.h`：`CAROTA_PARAM_ID_DEF` 枚举，13 个模块 ×8 个字段（`NAME/SW/HV/BLE/SN/PARTNUM/SUPPLIER/VERSIONID`），共 104 个 ID（0~103），另有 `PARAM_ID_SYS_STATUS_HANDLE=0x3000`、`PARAM_ID_CTRL_REBOOT_HANDLE=0x3001`、`PARAM_ID_CFG_DEVICE_ID_HANDLE=0x4001`。
- `carota_type.c` 定义三张 `ParamListTable`：
  - `TBStatus[16]`（仅 zoe/zoe-conf 的旧表，未注册）；
  - `TBzoeStatus[48]`（zoe、zoe-conf、zoe-dnn-model、zoe-map、computing-unit-mcu、lock-control-board 共 6 模块 ×8）；
  - `TBcomputingKernelStatus[56]`（computing-unit-kernel、camera、lidar、radar、gnss、sdc、uvm 共 7 模块 ×8）。
- INI 键名格式：`"<模块>:<字段>"`，字段名依次为 `ota_name`、`name_sv`、`name_hv`、`name_blv`、`name_sn`、`name_partnum`、`name_supplier`、`version_id`（int）。

### 5.9 参数管理头文件

- `param_types.h`：错误码 `PARAM_ERR_*`、类型 `PARAM_TYPE_INT/DOUBLE/STRING/ARRAY_*`、ID 区间（状态 0x0000~~0x2FFF、控制 0x3000~~0x3FFF、配置 0x4000~0x7FFF）、`ParamList`、`PARAM_VALUE_MAX_LEN=256`。
- `param_manager.h`：`ParamListTable`、`ParamTableInfo`、`ParamManagerInit/Release`、`ParamTableRegister`、`TBParameterGetByID/SetByID`、批量接口、`TBParameterSetFromString/GetAsString`、`ParamManagerSave/RestoreDefault`；便捷宏 `PARAM_TABLE_REGISTER`、`ParamGetInt/Double/String`、`PARAM_GET/SET_*`。控制参数读后自动清零，仅状态/配置参数持久化。
- `param_adapter.h`：兼容旧 TB 接口的 `TBParamListTable`/`TBParamList`、`TBParameterAdapter*` 批量接口，以及 `XPIntV`、`XSaveString`、`XGetString`、`XStrSaveString`、`XStrGetString` 等宏。
- `param_config.h`：示例参数 ID（`PARAM_ID_CTRL_REBOOT` 等）。
- `param_version.h`：版本 `1.0.0` 与特性标志查询 `ParamManagerGetVersionString/Version/HasFeature`。
- `iniparser.h`：简化 INI 字典（`IniEntry`、`dictionary`、加载/查询接口），无 `.c` 实现（实现在 `libparammanager.so`）。

## 6. 参数 ID 映射

### 6.1 模块索引 → SQLite 参数 ID（`GetParam`/`UpdateParam`）

`ALLOWED_DEVS` 顺序决定基址 `104 + i*8`：

| i    | dev                   | 基址 | version | sha  | url  | 备注                 |
| ---- | --------------------- | ---- | ------- | ---- | ---- | -------------------- |
| 0    | zoe                   | 104  | 104     | 110  | 111  |                      |
| 1    | zoe-conf              | 112  | 112     | 118  | 119  |                      |
| 2    | zoe-dnn-model         | 120  | 120     | 126  | 127  |                      |
| 3    | zoe-map               | 128  | 128     | 134  | 135  |                      |
| 4    | computing-unit-mcu    | 136  | 136     | 142  | 143  |                      |
| 5    | locking-control-board | 144  | 144     | 150  | 151  |                      |
| 6    | computing-unit-kernel | 152  | 152     | 158  | 159  |                      |
| 7    | camera                | 160  | 160     | 166  | 167  |                      |
| 8    | lidar                 | 168  | 168     | 174  | 175  |                      |
| 9    | radar                 | 176  | 176     | 182  | 183  |                      |
| 10   | gnss                  | 184  | 184     | 190  | 191  |                      |
| 11   | sdc                   | 192  | 192     | 198  | 199  |                      |
| 12   | uvm                   | 200  | 200     | 206  | 207  |                      |
| 13   | tbox                  | 208  | —       | —    | 215  | 升级直接触发         |
| 14   | adcu                  | 216  | 216     | 222  | 223  | 额外 `+8=224` 文件名 |

其他固定 ID：`3`(TBOX SN)、`4`(serialNo)、`14`(软件版本)、`15`(硬件版本)、`16`(Bootloader)、`19`(零件号)、`20`(供应商)、`102`(车辆编码 VN)、`226/227`(DOTA/FOTA 配置模式)、`E_ParamId_VIN`。

### 6.2 ParamManager（INI）ID

模块 `module_index` 的基址为 `module_index*8`，字段偏移 `+0..+7` 与上表字段顺序一致（`+0=ota_name`，`+1=name_sv`，…，`+7=version_id`）。两套参数系统（INI 与 SQLite）在 `GetInfoHandler`、`ADCU_SETMSG_QUEHandler` 中混用，需要保证同步。

## 7. 典型流程

### 7.1 FOTA（TBOX 自升级）

MCU/PMucu_upgrade线程carota_serverOTA MasterMCU/PMucu_upgrade线程carota_serverOTA MasterGET /prepare（查前置条件）GET /wake（唤醒整车）GET /modein（写状态=1, Set_da_mode(1)）GET /modeinresult（2s后确认）POST /ecuupgrade?dev=tbox（config/version/summary）UcuUpgradeStart(config)WriteSMainModemUpgradeStatus(1)查找 tbox* 包 → ethernet_upgrade → 写结果文件GET /ecuupgraderesult?dev=tboxGET /modeout（恢复 NORMAL，按快照上/下电）GET /modeoutresult（清 PM 事件）



### 7.2 ADCU 升级

1. `POST /ecuupgrade?dev=<name>`：把 `url/version/sha` 写入 SQLite 参数；
2. Master 调 `GET /api/ota/adcu/info/{name}` 取回 `url/summary/version_id/status`；
3. 通过 `GET /download...` 从本机下载升级包；
4. `POST /api/ota/adcu/info/{name}/set` 回写升级后的版本信息；
5. `GET /ecuupgraderesult?dev=<name>` 查询（依赖 `reboot_num`，非真实结果）。

### 7.3 DOTA 与休眠

- 进入：`/dotamodein` → 文件状态置 3、`Set_da_mode(5)`、保存 KL15、设置 `PM_SLEEP_EVENT_UPGRADE`。
- 退出：`/dotamodeout` → 清状态并恢复 NORMAL，若快照 KL15=0 则循环 `ControlRemotePowerDown()`。
- 结果接口在 2s 窗口后按文件状态返回 1/-1，并清除 `UPGRADE/WAKEUPCAR/WAKEUPTBOX` 事件。
- 休眠联动：PM 状态变化经 `CarotaPmPowerState` 进入 `carotaTimerThread`，联动 20005/20006 的 FOTA/DOTA 服务。

## 8. 关键状态机

| 状态/枚举                | 取值                                                         | 说明                |
| ------------------------ | ------------------------------------------------------------ | ------------------- |
| `UcuStatus_e`            | 0 NORMAL / 1 FOTA / 2 OBD / 3 DOTA                           | 文件与运行时共用    |
| `GwStatus_e`             | 同上                                                         | 高 4 位网关状态     |
| `CanLockedType_e`        | 0 NONE / 1 FOTA / 2 DOTA                                     | CAN TP 锁类型       |
| 升级内部状态             | 0 IDLE / 1 DOING / 2 FINISH                                  | `ucu_upgrade.c`     |
| `MainModemUpgradeStatus` | 0 正常 / 1 升级中 / 2 成功 / 3 失败 / 4 特殊                 | 文件持久化          |
| `Set_da_mode`            | 1 进 OTA / 2 进查询 / 3 退 OTA / 4 退查询 / 5 进诊断 / 6 诊断查询 / 7 退 DOTA / 8 诊断退出查询 | 发 MCU `0x00033100` |
| `Set_ecu_adcu`           | 1/2                                                          | 发 MCU `0x00033000` |

## 9. 风险与待确认项（代码审阅发现）

| 位置                       | 问题                                                         |
| -------------------------- | ------------------------------------------------------------ |
| `main.c:2191`              | `cJSON_PrintUnformatted(g_b_response)` 把 `char[4096]` 当 `cJSON*` 传入，属未定义行为，`/vehiclestatus` 可能崩溃 |
| `main.c:2200`              | A/B 等待机制中 `g_pending_a_conn` 从未赋值，逻辑为死代码     |
| 多处 handler               | 先 `SendJSON()`（内部 `mg_send_http_ok` 已发完整响应），随后又 `mg_printf` 发一次 HTTP 头，存在重复响应头 |
| `ucu_upgrade.c:400`        | `memset(errlog, 0, strlen(errlog))` 对调用方未初始化缓冲区取长度，UB |
| `ucu_upgrade.h:22`         | 声明 `StartUpgradePkg_ota2` 但无定义、无调用                 |
| `ucu_upgrade.c:220`        | `updateVersionWithCmd()` 定义但从未调用（版本库更新不会自动发生） |
| `carotaHttpClient.c:546`   | `CheckVehicleAllowUpgrade` 把 `vehicleName` 当作 `vehicleId` 传给 `SendAllowUpgradeRequest`，且忽略返回的 `vehicleId` |
| `carotaHttpClient.c:22-23` | 主/副模组转发 URL 完全相同，实际未区分                       |
| `main.c:719`、`2642`       | `locking-control-board` 与 `carota_type.c` 的 `lock-control-board` 不一致，导致参数查不到 |
| `main.c:1262`              | `EcuAllowedDevs` 存在拼写错误 `"ocking-control-board"`、`" computing-unit-kernel"`（前导空格） |
| `main.c:719`               | `/getinfo?dev=adcu` 通过白名单但不在 `valid_names` 中，直接 500 |
| `main.c:2945`              | `/download` 以绝对路径直接读文件，未限制根目录（仅过滤 `..`），存在任意文件读取风险 |
| `vehicle_status.c:83,98`   | 参数校验写成 `len < x && gear == NULL`（应为 `               |
| `ucu_status.c`             | `UcuStatusInit` 未被 `main` 调用；`UcuStatusSetStatus/ClearStatus` 不更新本地状态 |
| `carotsleephandle.c:44-54` | 系统 SLEEP 时发送 `mode=0`（唤醒），NORMAL 时发送 `mode=1`（休眠），与注释语义相反 |
| `main.c:571-590`、`1621`   | 在 HTTP 线程中长时间 `sleep`/忙等（最长 30s/10s），会占用 CivetWeb 工作线程 |
| `main.c:3170`              | 启动自检中 `UPGRADE_INIT` 分支被当作“已完成”却写 `3`（失败）并重启，语义可疑 |