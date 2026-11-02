#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <fcntl.h>
#include <errno.h>
#include "cJSON.h"
#include "pm_device_control.h"
#include "sr_log.h"
#include "err_code.h"
#include "pm_mcu_control.h"

#define RES_OK RET_OK

static GList *moduleList = NULL;
static uint32_t g_mcuWakePin = 0;
static uint32_t g_smsWakeMode = 0;

uint32_t GetMcuWakePin(void)
{
    return g_mcuWakePin;
}

uint32_t GetSmsWakeMode(void)
{
    return g_smsWakeMode;
}

int8_t LoadPmConfig(const char *path) {
    // 读取配置文件
    FILE *file = fopen(path, "r");
    if (!file)
    {
        return RET_ERR;
    }

    // 读取文件内容
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *fileContent = (char *)malloc(fileSize + 1);
    fread(fileContent, 1, fileSize, file);
    fclose(file);

    
    LOG_Debug("JSON Content: %s", fileContent);
    // 反序列化配置文件
    cJSON *root = cJSON_Parse(fileContent);
    free(fileContent);
    fileContent = NULL;
    
    if (!root)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            LOG_Error("Error before: %s", error_ptr);
        }
        return RET_ERR;
    }

    cJSON *modules = cJSON_GetObjectItem(root, "modules");

    // 获取模块数量
    int moduleCount = cJSON_GetArraySize(modules);

    LOG_Debug("moduleCount : %d ", moduleCount);
    
    // 检查模块数组是否为一个数组
    if (!cJSON_IsArray(modules)) {
        LOG_Error("Error: 'modules' is not an array.");
        cJSON_Delete(modules);
        return RET_ERR;
    }

    for (int i = 0; i < moduleCount; ++i)
    {
        // 获取json对象
        cJSON *moduleJson = cJSON_GetArrayItem(modules, i);
        if (!moduleJson) 
        {
            LOG_Error("Error: Unable to get moduleJson at index %d.", i);
            continue;  
        }

        // 获取 "name" 字段
        cJSON *nameItem = cJSON_GetObjectItem(moduleJson, "name");
        if (nameItem != NULL && cJSON_IsString(nameItem))
        {
            LOG_Debug("name : %s ", nameItem->valuestring);

            // 分配PmConfigModule_t内存
            PmConfigModule_t *module = g_try_malloc0(sizeof(PmConfigModule_t));
            strncpy(module->name, nameItem->valuestring, 16);

            // 判断 support_mode 中是否包含 "sleep" 或 "poweroff"
            cJSON *supportModeJson = cJSON_GetObjectItem(moduleJson, "support_mode");
            cJSON *modeItem = NULL;
            
            module->sleepMode.isSupport = 0;  // 初始化为 0
            module->poweroffMode.isSupport = 0;  // 初始化为 0
            cJSON_ArrayForEach(modeItem, supportModeJson)
            {
                if (strcmp(modeItem->valuestring, "sleep") == 0)
                {
                    module->sleepMode.isSupport = 1;

                    // 获取 "pins" 字段
                    cJSON *pinsJson = cJSON_GetObjectItem(moduleJson, "pins");
                    if (pinsJson != NULL)
                    {
                        // 获取 "sleep_pin" 字段
                        cJSON *sleepPinJson = cJSON_GetObjectItem(pinsJson, "sleep_pin");
                        if (sleepPinJson != NULL)
                        {
                            // 获取 "pin_num" 和 "active" 字段的值
                            module->sleepMode.pinNumber = cJSON_GetObjectItem(sleepPinJson, "pin_num")->valueint;
                            module->sleepMode.active = cJSON_GetObjectItem(sleepPinJson, "active")->valueint;
                        }
                    }
                }
                if (strcmp(modeItem->valuestring, "poweroff") == 0)
                {
                    module->poweroffMode.isSupport = 1;

                    // 获取 "pins" 字段
                    cJSON *pinsJson = cJSON_GetObjectItem(moduleJson, "pins");
                    if (pinsJson != NULL)
                    {
                        // 获取 "power_pin" 字段
                        cJSON *powerPinJson = cJSON_GetObjectItem(pinsJson, "power_pin");
                        if (powerPinJson != NULL)
                        {
                            // 获取 "pin_num" 和 "active" 字段的值
                            module->poweroffMode.pinNumber = cJSON_GetObjectItem(powerPinJson, "pin_num")->valueint;
                            module->poweroffMode.active = cJSON_GetObjectItem(powerPinJson, "active")->valueint;
                        }
                    }
                }
            }

            // 给 tboxSleep 和 tboxDeepSleep 赋值
            const char *tboxSleepMode = cJSON_GetObjectItem(moduleJson, "tbox_sleep")->valuestring;
            if(NULL == tboxSleepMode)
            {
                LOG_Error("tboxSleepMode is NULL");
                continue;
            }
            module->tboxSleep = (strcmp(tboxSleepMode, "sleep") == 0) ? 0 : 1;

            const char *tboxDeepSleepMode = cJSON_GetObjectItem(moduleJson, "tbox_deep_sleep")->valuestring;
            if(NULL == tboxDeepSleepMode)
            {
                LOG_Error("tboxDeepSleepMode is NULL");
                continue;
            }
            module->tboxDeepSleep = (strcmp(tboxDeepSleepMode, "sleep") == 0) ? 0 : 1;

            // 添加到链表
            moduleList = g_list_append(moduleList, module);
        }
        else
        {
            LOG_Error("Error: Unable to get 'name' field.");
            continue;  
        }
    }

    g_mcuWakePin = cJSON_GetObjectItem(root, "mcu_wake")->valueint;

    g_smsWakeMode = cJSON_GetObjectItem(root, "sms_wake_mode")->valueint;
    
    cJSON_Delete(root);

    return RET_OK;
}

static int Ql_GPIO_SetLevel(uint8_t pinNumber, uint8_t pinLevel)
{
    char command[512] = {0};
    
    sprintf(command, "echo %d > /sys/class/gpio/gpio%d/value",pinLevel, pinNumber);
    int status = system(command);
    if (status == 0)
    {
        LOG_Debug("%s : exec cmd[ %s ] success.",__FUNCTION__,command);
        return RET_OK;
    } else {
        LOG_Debug("%s : exec cmd[ %s ] failed.",__FUNCTION__,command);
        return RET_ERR;
    }
    
}
static int Ql_GPIO_Init(uint8_t pinNumber,uint8_t pinDirection, uint8_t pinLevel, uint8_t pinPullsel)
{
    
    char command[512] = {0};

    sprintf(command, "echo %d > /sys/class/gpio/export", pinNumber);
    int status = system(command);
    if (status == 0)
    {
        LOG_Debug("%s : exec cmd[ %s ] success.",__FUNCTION__,command);
    } else {
        LOG_Error("%s : exec cmd[ %s ] failed.",__FUNCTION__,command);
    }
    
    memset(command,0,sizeof(command));
    
    sprintf(command, "echo %s > /sys/class/gpio/gpio%d/direction",(pinDirection == 0) ? "in" : "out", pinNumber);
    status = system(command);
    if (status == 0)
    {
        LOG_Debug("%s : exec cmd[ %s ] success.",__FUNCTION__,command);
        return RET_OK;
    } else {
        LOG_Error("%s : exec cmd[ %s ] failed.",__FUNCTION__,command);
        return RET_ERR;
    }
    
}
static int Ql_GPIO_Uninit(uint8_t pinNumber)
{
    //nothing to do
    return RET_OK;
}

int8_t FreePmConfig(void)
{
    // 判空操作
    if (moduleList == NULL)
    {
        return RET_OK;
    }

    // 释放链表节点
    g_list_free_full(moduleList, g_free);
    moduleList = NULL;

    return RET_OK;
}

void ShowPmConfig(void)
{
    
    if(NULL == moduleList)
    {
        LOG_Debug("moduleList is NULL");
        return;
    }
    // 打印配置信息
    GList *current = moduleList;
    while (current != NULL) 
    {
        PmConfigModule_t *module = (PmConfigModule_t *)current->data;

        LOG_Debug("Module Name: %s", module->name);
        LOG_Debug("Sleep Mode Support: %d", module->sleepMode.isSupport);
        LOG_Debug("Sleep Mode Pin Number: %d", module->sleepMode.pinNumber);
        LOG_Debug("Sleep Mode Active: %d", module->sleepMode.active);
        LOG_Debug("Poweroff Mode Support: %d", module->poweroffMode.isSupport);
        LOG_Debug("Poweroff Mode Pin Number: %d", module->poweroffMode.pinNumber);
        LOG_Debug("Poweroff Mode Active: %d", module->poweroffMode.active);
        LOG_Debug("Tbox Sleep Mode: %d", module->tboxSleep);
        LOG_Debug("Tbox Deep Sleep Mode: %d", module->tboxDeepSleep);

        current = g_list_next(current);
    }
}

static uint8_t getBitValue(uint32_t value,uint8_t bit)
{
    uint8_t BitValue = (value >> bit) & 1;
    return BitValue;
}

int8_t PmWorkAllModules(void) {
    if (moduleList == NULL)
    {
        LOG_Debug("moduleList is NULL ");
        return RET_ERR;
    }

    GList *current = moduleList;
    while (current != NULL)
    {
        PmConfigModule_t *module = (PmConfigModule_t *)current->data;

        // 处理 Poweroff Mode
        if (module->poweroffMode.isSupport == 1)
        {
            uint8_t pinNumber = module->poweroffMode.pinNumber;
            uint8_t active = getBitValue(module->poweroffMode.active,0);

            // 激活电源
            // 设置管脚
            int result = Ql_GPIO_SetLevel(pinNumber, active);
            if (result != RES_OK)
            {
                LOG_Error("Failed to set poweroff mode pin for module %s. Error code: %d ", module->name, result);
            }

            LOG_Debug("Set poweroff mode pin activate for module %s ", module->name);
        }
        else
        {
            LOG_Notice("module %s is not support poweroffMode .", module->name);
        }

        // 处理 Sleep Mode
        if (module->sleepMode.isSupport == 1)
        {
            uint8_t pinNumber = module->sleepMode.pinNumber;
            uint8_t active = getBitValue(module->sleepMode.active,0);
            //唤醒            
            // 设置管脚
            int result = Ql_GPIO_SetLevel(pinNumber, active);
            if (result != RES_OK)
            {
                LOG_Error("Failed to set sleep mode pin for module %s. Error code: %d ", module->name, result);
            }

            LOG_Debug("Set sleep mode pin for module %s ", module->name);
        }
        else
        {
            LOG_Notice("module %s is not support sleepMode. ", module->name);
        }

        current = g_list_next(current);
    }

    return RET_OK;
}

int8_t PmPowerIoInit(void)
{
    if (moduleList == NULL)
    {
        LOG_Debug("moduleList is NULL ");
        return RET_ERR;
    }

    GList *current = moduleList;
    while (current != NULL)
    {
        PmConfigModule_t *module = (PmConfigModule_t *)current->data;

        // 处理 Poweroff Mode
        if (module->poweroffMode.isSupport == 1)
        {
            uint8_t pinNumber = module->poweroffMode.pinNumber;
            uint8_t active = getBitValue(module->poweroffMode.active,0);

            // 激活电源
            // 设置管脚
            int result = Ql_GPIO_Init(pinNumber,1, active, 0);
            if (result != RES_OK)
            {
                LOG_Error("Failed to Ql_GPIO_Init poweroff mode pin for module %s. Error code: %d ", module->name, result);
            }

            LOG_Debug("Ql_GPIO_Init poweroff mode pin activate for module %s ", module->name);
        }
        else
        {
            LOG_Notice("module %s is not support poweroffMode .", module->name);
        }

        // 处理 Sleep Mode
        if (module->sleepMode.isSupport == 1)
        {
            uint8_t pinNumber = module->sleepMode.pinNumber;
            uint8_t active = getBitValue(module->sleepMode.active,0);

            // awake            
            // 设置管脚
            int result = Ql_GPIO_Init(pinNumber,1, active, 0);
            if (result != RES_OK)
            {
                LOG_Error("Failed to Ql_GPIO_Init sleep mode pin for module %s. Error code: %d ", module->name, result);
            }

            LOG_Debug("Ql_GPIO_Init sleep mode pin for module %s ", module->name);
        }
        else
        {
            LOG_Notice("module %s is not support sleepMode. ", module->name);
        }

        current = g_list_next(current);
    }

    //根据配置文件配置，初始化设备为工作状态
    PmWorkAllModules();

    return RET_OK;
}

int8_t PmPowerIoExit(void)
{
    if (moduleList == NULL) 
    {
        LOG_Debug("moduleList is NULL ");
        return RET_ERR;
    }

    GList *current = moduleList;
    while (current != NULL) {
        PmConfigModule_t *module = (PmConfigModule_t *)current->data;

        // 处理 Poweroff Mode
        if (module->poweroffMode.isSupport == 1)
        {
            uint8_t pinNumber = module->poweroffMode.pinNumber;

            // 去初始化
            int result = Ql_GPIO_Uninit(pinNumber);
            if (result != RES_OK)
            {
                LOG_Error("Failed to Ql_GPIO_Uninit poweroff mode pin for module %s. Error code: %d ", module->name, result);
            }

            LOG_Debug("Ql_GPIO_Uninit poweroff mode pin activate for module %s ", module->name);
        }
        else
        {
            LOG_Debug("module %s is not support poweroffMode .", module->name);
        }

        // 处理 Sleep Mode
        if (module->sleepMode.isSupport == 1)
        {
            uint8_t pinNumber = module->sleepMode.pinNumber;
            
            // 去初始化
            int result = Ql_GPIO_Uninit(pinNumber);
            if (result != RES_OK)
            {
                LOG_Error("Failed to Ql_GPIO_Uninit sleep mode pin for module %s. Error code: %d ", module->name, result);
            }

            LOG_Debug("Ql_GPIO_Uninit sleep mode pin for module %s ", module->name);
        }
        else
        {
            LOG_Debug("module %s is not support sleepMode. ", module->name);
        }

        current = g_list_next(current);
    }

    return RET_OK;
}

int8_t PmPowerControlAllModules(int8_t isPowerOn)
{
    if (moduleList == NULL)
    {
        LOG_Debug("moduleList is NULL");
        return RET_ERR;
    }

    GList *current = moduleList;
    while (current != NULL) {
        PmConfigModule_t *module = (PmConfigModule_t *)current->data;

        // 判断是否支持 Poweroff Mode
        if (module->poweroffMode.isSupport == 1)
        {
            uint8_t pinNumber = module->poweroffMode.pinNumber;
            uint8_t active = getBitValue(module->poweroffMode.active, 0);

            // 根据参数控制上电或下电
            if (isPowerOn)
            {
                // 上电
                int result = Ql_GPIO_SetLevel(pinNumber, active);
                if (result != RES_OK) 
                {
                    LOG_Error("Failed to power on module %s. Error code: %d", module->name, result);
                }
                LOG_Debug("Power on module %s", module->name);
            } 
            else
            {
                // 下电
                active ^= 1;  // 取反
                int result = Ql_GPIO_SetLevel(pinNumber, active);
                if (result != RES_OK)
                {
                    LOG_Error("Failed to power off module %s. Error code: %d", module->name, result);
                }
                LOG_Debug("Power off module %s", module->name);
            }
        } else {
            LOG_Debug("Module %s does not support poweroffMode.", module->name);
        }

        current = g_list_next(current);
    }

    return RET_OK;
}


int8_t PmSleepControlAllModules(int8_t isAwake)
{
    if (moduleList == NULL)
    {
        LOG_Debug("moduleList is NULL");
        return RET_ERR;
    }

    GList *current = moduleList;
    while (current != NULL) {
        PmConfigModule_t *module = (PmConfigModule_t *)current->data;

        // 判断是否支持 Sleep Mode
        if (module->sleepMode.isSupport == 1)
        {
            uint8_t pinNumber = module->sleepMode.pinNumber;
            uint8_t active = getBitValue(module->sleepMode.active, 0);

            // 根据参数控制休眠或唤醒
            if (isAwake)
            {
                // 唤醒
                int result = Ql_GPIO_SetLevel(pinNumber, active);
                if (result != RES_OK)
                {
                    LOG_Error("Failed to wake up module %s. Error code: %d", module->name, result);
                }
                LOG_Debug("Wake up module %s", module->name);
            } 
            else
            {
                // 休眠
                active ^= 1;  // 取反
                int result = Ql_GPIO_SetLevel(pinNumber, active);
                if (result != RES_OK)
                {
                    LOG_Error("Failed to enter sleep mode for module %s. Error code: %d", module->name, result);
                }
                LOG_Debug("Enter sleep mode for module %s", module->name);
            }
        } else {
            LOG_Debug("Module %s does not support sleepMode.", module->name);
        }

        current = g_list_next(current);
    }

    return RET_OK;
}

static PmConfigModule_t *findModuleByName(const char *moduleName)
{
    GList *current = moduleList;
    if(NULL == moduleName)        
    {
        LOG_Error("%s moduleName is NULL",__FUNCTION__);
        return NULL;
    }
    while (current != NULL) 
    {
        PmConfigModule_t *module = (PmConfigModule_t *)current->data;
        if (strcmp(module->name, moduleName) == 0) 
        {
            return module;
        }
        current = g_list_next(current);
    }
    return NULL;
}

static int8_t PmPowerControlSingleModule(const char *moduleName, int8_t isPowerOn)
{
    if (moduleList == NULL) 
    {
        LOG_Error("moduleList is NULL");
        return RET_ERR;
    }

    PmConfigModule_t *module = findModuleByName(moduleName);
    if (module == NULL) 
    {
        LOG_Error("Module %s not found in moduleList", moduleName);
        return RET_ERR;
    }

    // 判断是否支持 Poweroff Mode
    if (module->poweroffMode.isSupport == 1) 
    {
        uint8_t pinNumber = module->poweroffMode.pinNumber;
        uint8_t active = getBitValue(module->poweroffMode.active, 0);

        // 根据参数控制上电或下电
        if (isPowerOn) 
        {
            // 上电
            int result = Ql_GPIO_SetLevel(pinNumber, active);
            if (result != RES_OK) 
            {
                LOG_Debug("Failed to power on module %s. Error code: %d", module->name, result);
                return RET_ERR;
            }
            LOG_Debug("Power on module %s", module->name);
        } 
        else 
        {
            // 下电
            active ^= 1;  // 取反
            int result = Ql_GPIO_SetLevel(pinNumber, active);
            if (result != RES_OK) 
            {
                LOG_Debug("Failed to power off module %s. Error code: %d", module->name, result);
                return RET_ERR;
            }
            LOG_Debug("Power off module %s", module->name);
        }
    } 
    else 
    {
        LOG_Debug("Module %s does not support poweroffMode.", module->name);
    }

    return RET_OK;
}

static int8_t PmSleepControlSingleModule(const char *moduleName, int8_t isAwake)
{
    if (moduleList == NULL)
    {
        LOG_Debug("moduleList is NULL");
        return RET_ERR;
    }

    PmConfigModule_t *module = findModuleByName(moduleName);
    if (module == NULL) 
    {
        LOG_Debug("Module %s not found in moduleList", moduleName);
        return RET_ERR;
    }

    // 判断是否支持 Sleep Mode
    if (module->sleepMode.isSupport == 1) 
    {
        uint8_t pinNumber = module->sleepMode.pinNumber;
        uint8_t active = getBitValue(module->sleepMode.active, 0);

        // 根据参数控制休眠或唤醒
        if (isAwake) 
        {
            // 唤醒
            int result = Ql_GPIO_SetLevel(pinNumber, active);
            if (result != RES_OK)
            {
                LOG_Debug("Failed to wake up module %s. Error code: %d", module->name, result);
                return RET_ERR;
            }
            LOG_Debug("Wake up module %s", module->name);
        } else 
        {
            // 休眠
            active ^= 1;  // 取反
            int result = Ql_GPIO_SetLevel(pinNumber, active);
            if (result != RES_OK)
            {
                LOG_Debug("Failed to enter sleep mode for module %s. Error code: %d", module->name, result);
                return RET_ERR;
            }
            LOG_Debug("Enter sleep mode for module %s", module->name);
        }
    } 
    else
    {
        LOG_Debug("Module %s does not support sleepMode.", module->name);
    }

    return RET_OK;
}

int8_t PmPowerControlWifiModule(int8_t isPowerOn) 
{
    return PmPowerControlSingleModule("wifi", isPowerOn);
}

int8_t PmPowerControlEthModule(int8_t isPowerOn)
{
    return PmPowerControlSingleModule("eth", isPowerOn);
}

int8_t PmPowerControlCodecModule(int8_t isPowerOn)
{
    return PmPowerControlSingleModule("codec", isPowerOn);
}

int8_t PmPowerControlHSEModule(int8_t isPowerOn)
{
    return PmPowerControlSingleModule("HSE", isPowerOn);
}

int8_t PmSleepControlWifiModule(int8_t isAwake)
{
    return PmSleepControlSingleModule("wifi", isAwake);
}

int8_t PmSleepControlEthModule(int8_t isAwake)
{
    return PmSleepControlSingleModule("eth", isAwake);
}

int8_t PmSleepControlCodecModule(int8_t isAwake) 
{
    return PmSleepControlSingleModule("codec", isAwake);
}

int8_t PmSleepControlHSEModule(int8_t isAwake)
{
    return PmSleepControlSingleModule("HSE", isAwake);
}
int8_t PmTboxSleepSetModules(void)
{
    if (moduleList == NULL)
    {
        LOG_Debug("moduleList is NULL");
        return RET_ERR;
    }

    GList *current = moduleList;
    while (current != NULL)
    {
        PmConfigModule_t *module = (PmConfigModule_t *)current->data;

        // 根据tboxSleep的值判断进入休眠模式还是关机模式
        if (module->tboxSleep == 0)
        {
            // 休眠模式
            if (module->sleepMode.isSupport == 1)
            {
                uint8_t pinNumber = module->sleepMode.pinNumber;
                uint8_t active = getBitValue(module->sleepMode.active, 0) ^ 1;  // 取反

                int result = Ql_GPIO_SetLevel(pinNumber, active);
                if (result != RES_OK)
                {
                    LOG_Debug("Failed to set sleep mode pin for module %s during Tbox sleep. Error code: %d", module->name, result);
                    current = g_list_next(current);
                    continue;
                }
                LOG_Debug("Set sleep mode pin for module %s during Tbox sleep", module->name);
            }
            else
            {
                LOG_Debug("Module %s does not support sleepMode.", module->name);
            }
        }
        else if (module->tboxSleep == 1)
        {
            // 关机模式
            if (module->poweroffMode.isSupport == 1)
            {
                uint8_t poweroffPinNumber = module->poweroffMode.pinNumber;
                uint8_t poweroffActive = getBitValue(module->poweroffMode.active, 0) ^ 1;  // 取反

                int result = Ql_GPIO_SetLevel(poweroffPinNumber, poweroffActive);
                if (result != RES_OK)
                {
                    LOG_Debug("Failed to power off module %s during Tbox sleep. Error code: %d", module->name, result);
                    current = g_list_next(current);
                    continue;
                }
                LOG_Debug("Power off module %s during Tbox sleep", module->name);
            }
            else
            {
                LOG_Debug("Module %s does not support poweroffMode.", module->name);
            }
        }

        current = g_list_next(current);
    }

    return RET_OK;
}

int8_t PmTboxDeepSleepSetModules(void)
{
    if (moduleList == NULL)
    {
        LOG_Debug("moduleList is NULL");
        return RET_ERR;
    }

    GList *current = moduleList;
    while (current != NULL)
    {
        PmConfigModule_t *module = (PmConfigModule_t *)current->data;

        // 根据tboxDeepSleep的值判断进入休眠模式还是关机模式
        if (module->tboxDeepSleep == 0)
        {
            // 休眠模式
            if (module->sleepMode.isSupport == 1)
            {
                uint8_t pinNumber = module->sleepMode.pinNumber;
                uint8_t active = getBitValue(module->sleepMode.active, 0) ^ 1;  // 取反

                int result = Ql_GPIO_SetLevel(pinNumber, active);
                if (result != RES_OK)
                {
                    LOG_Debug("Failed to set sleep mode pin for module %s during Tbox deep sleep. Error code: %d", module->name, result);
                    current = g_list_next(current);
                    continue;
                }
                LOG_Debug("Set sleep mode pin for module %s during Tbox deep sleep", module->name);
            }
            else
            {
                LOG_Debug("Module %s does not support sleepMode.", module->name);
            }
        }
        else if (module->tboxDeepSleep == 1)
        {
            // 关机模式
            if (module->poweroffMode.isSupport == 1)
            {
                uint8_t poweroffPinNumber = module->poweroffMode.pinNumber;
                uint8_t poweroffActive = getBitValue(module->poweroffMode.active, 0) ^ 1;  // 取反

                int result = Ql_GPIO_SetLevel(poweroffPinNumber, poweroffActive);
                if (result != RES_OK)
                {
                    LOG_Debug("Failed to power off module %s during Tbox deep sleep. Error code: %d", module->name, result);
                    current = g_list_next(current);
                    continue;
                }
                LOG_Debug("Power off module %s during Tbox deep sleep", module->name);
            }
            else
            {
                LOG_Debug("Module %s does not support poweroffMode.", module->name);
            }
        }

        current = g_list_next(current);
    }

    return RET_OK;
}
int8_t PmWakeupDevices(const char * name,uint8_t mode)
{
    if(NULL == name)
    {
        LOG_Error("%s set error,device name is NULL",__FUNCTION__);
        return RET_ERR;
    }

    if(strcmp(name, "mcu") == 0)
    {
        PmSchedWakeUpMcu();
        LOG_Debug("%s Set %s wake up",__FUNCTION__,name);
        return RET_OK;
    }
    
    if(POWER_MODE == mode)
    {
        PmPowerControlSingleModule(name,POWER_ON);
        LOG_Debug("%s Set %s power on",__FUNCTION__,name);
    }
    else if(WAKE_MODE == mode)
    {
        PmSleepControlSingleModule(name,WAKE_UP);
        LOG_Debug("%s Set %s wake up",__FUNCTION__,name);
    }
    else
    {
        LOG_Debug("%s set error,mode %d is err.",__FUNCTION__,mode);
        return RET_ERR;
    }

    return RET_OK;
        
}


