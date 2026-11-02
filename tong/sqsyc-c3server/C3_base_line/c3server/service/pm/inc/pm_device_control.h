#ifndef _PM_DEVICE_CONTROL_H_
#define _PM_DEVICE_CONTROL_H_

#include <stdint.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define POWER_ON        (1)
#define POWER_OFF       (0)
#define WAKE_UP         (1)
#define WAKE_DOWN       (0)
#define WAKE_MODE       (1)
#define POWER_MODE      (2)

#define CONFIG_FILE_PATH    "/oemapp/config/pm_config.json"
#define CONFIG_FILE_PATH_PRIMARY "/oemdata/config/pm_config.json"

// 模块配置模式
typedef struct {
    uint8_t isSupport;    // 是否支持当前的模式 0是不支持，1是支持
    uint8_t pinNumber;    // 管脚编号
    uint8_t active;  // 有效值 0是低有效，1是高有效
} PmConfigMode_t;

// 模块配置
typedef struct {
    char name[17];           // 模块名称，长度不能大于16
    PmConfigMode_t sleepMode; // 模块休眠模式
    PmConfigMode_t poweroffMode; // 模块关机模式
    uint8_t tboxSleep;       // Tbox休眠模式 (0-1), 0是设置模块进入休眠模式，1是设置模块进入关机模式
    uint8_t tboxDeepSleep;   // Tbox深度休眠模式 (0-1), 0是设置模块进入休眠模式，1是设置模块进入关机模式
} PmConfigModule_t;

/**
 * @brief  加载 PM 配置文件
 * @param  path  配置文件路径
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t LoadPmConfig(void);

/**
 * @brief  显示当前 PM 配置
 * @return void
 */
void ShowPmConfig(void);

/**
 * @brief  释放 PM 配置
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t FreePmConfig(void);

/**
 * @brief  执行 PM 工作
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t PmWorkAllModules(void);

/**
 * @brief  初始化 PMio
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t PmPowerIoInit(void);

/**
 * @brief  退出 PMio
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t PmPowerIoExit(void);

/**
 * @brief  控制 WiFi 模块的电源
 * @param  isPowerOn  POWER_ON 表示打开电源,POWER_OFF表示关闭电源
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t PmPowerControlWifiModule(int8_t isPowerOn);

/**
 * @brief  控制 Ethernet 模块的电源
 * @param  isPowerOn  POWER_ON 表示打开电源,POWER_OFF表示关闭电源
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t PmPowerControlEthModule(int8_t isPowerOn);

/**
 * @brief  控制代码转换模块的电源
 * @param  isPowerOn  POWER_ON表示打开电源,POWER_OFF表示关闭电源
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t PmPowerControlCodecModule(int8_t isPowerOn);

/**
 * @brief  控制 HSE 模块的电源
 * @param  isPowerOn  POWER_ON表示打开电源,POWER_OFF表示关闭电源
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t PmPowerControlHSEModule(int8_t isPowerOn);

/**
 * @brief  Wake up WiFi 模块
 * @param  isAwake  WAKE_UP 表示 wake up,WAKE_DOWN表示 sleep
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t PmSleepControlWifiModule(int8_t isAwake);

/**
 * @brief  Wake up Ethernet 模块
 * @param  isAwake  WAKE_UP 表示 wake up,WAKE_DOWN表示 sleep
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t PmSleepControlEthModule(int8_t isAwake);

/**
 * @brief  Wake up 代码转换模块
 * @param  isAwake  WAKE_UP表示 wake up,WAKE_DOWN表示 sleep
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t PmSleepControlCodecModule(int8_t isAwake);

/**
 * @brief  Wake up HSE 模块
 * @param  isAwake  WAKE_UP表示 wake up,WAKE_DOWN表示 sleep
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t PmSleepControlHSEModule(int8_t isAwake);

/**
 * @brief  设置 TBox 休眠
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t PmTboxSleepSetModules(void);

/**
 * @brief  设置 TBox 深度休眠
 * @return int8_t  -1表示失败，0表示成功
 */
int8_t PmTboxDeepSleepSetModules(void);

/**
 * @brief  获取 MCU wake  pin
 * @return uint32_t  wake  pin
 */
uint32_t GetMcuWakePin(void);

/**
 * @brief  获取 sms wake mode
 * @return uint32_t  wake mode
 */
uint32_t GetSmsWakeMode(void);

/**
 * @brief  获取 dial wake mode
 * @return uint32_t  wake mode
 */
uint32_t GetDialWakeMode(void);
/**
 * @brief 唤醒相关外设
 * @param  name  设备名称字符串
 * @param  mode  唤醒模式，WAKE_MODE：休眠唤醒，WAKE_UP：上电唤醒
 * @return uint32_t  wake mode
 */
int8_t PmWakeupDevices(const char * name,uint8_t mode);

#ifdef __cplusplus
}
#endif

#endif //_PM_DEVICE_CONTROL_H_

