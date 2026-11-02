#ifndef __UPGRADEMCU_H__
#define __UPGRADEMCU_H__


#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    McuStepGetVern = 0,
    McuStepGetSeed,
    McuStepUnlockFlash,
    McuStepPutFlash,
    McuStepEraseFlash,
    McuStepFlashing,
    McuStepFlashComplete,
    McuStepRestMcu
} McuProcessStep_E;

/**
 * @name UpgradeMcuProcess
 * @brief 升级MCU处理函数
 * 
 * @param filePath 文件路径，用于指定升级包的位置
 * @return int32_t 返回RET_OK表示成功，RET_ERR表示失败
 */
int32_t UpgradeMcuProcess(const char *filePath);

/**
 * @name GetMcuProcessStep
 * @brief 获取MCU升级阶段
 * 
 * @param 无
 * @return int32_t 返回RET_OK表示成功，RET_ERR表示失败
 */
McuProcessStep_E GetMcuProcessStep(void);

#ifdef __cplusplus
}
#endif
#endif
