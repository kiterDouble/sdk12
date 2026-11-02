#ifndef __UPGRADEBT_H__
#define __UPGRADEBT_H__


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @name UpgradeBtProcess
 * @brief 升级BT处理函数
 * 
 * @param filePath 文件路径，用于指定升级包的位置
 * @return int32_t 返回RET_OK表示成功，RET_ERR表示失败
 */
int32_t UpgradeBtProcess(const char *filePath);

#ifdef __cplusplus
}
#endif
#endif
