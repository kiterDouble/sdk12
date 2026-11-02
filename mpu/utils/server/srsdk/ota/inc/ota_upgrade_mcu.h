#ifndef __UPGRADEMCU_H__
#define __UPGRADEMCU_H__


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @name UpgradeMcuProcess
 * @brief 升级MCU处理函数
 * 
 * @param filePath 文件路径，用于指定升级包的位置
 * @return int32_t 返回RET_OK表示成功，RET_ERR表示失败
 */
int32_t UpgradeMcuProcess(const char *filePath);

#ifdef __cplusplus
}
#endif
#endif
