#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_upgrade.h"

/**
 * @name GetSystemRetValue
 * @brief 获取系统命令执行的返回值
 *        本函数通过调用system()函数执行指定的命令，并返回该命令的执行状态。
 * @param cmd 指向要执行的命令字符串的指针。
 * @return 如果执行成功，返回命令的退出状态；如果执行失败，返回RET_ERR。
 */
static int GetSystemRetValue(const uint8_t *cmd)
{
	pid_t status;
	
	if(NULL == cmd)
	{
		return RET_ERR;
	}
	//LOG_Debug("%s exe start", cmd);
	status = system((char *)cmd);
	//LOG_Debug("%s exe end:%d", cmd, status);
	if(-1 == status)
	{
        
		return RET_ERR;
	}
	
	if(!WIFEXITED(status))
	{
		return RET_ERR;
	}
	
	return WEXITSTATUS(status);
}

/**
 * @name HalModemUpgradeImpl
 * @brief 模组fota升级执行
 * @param filePath:输入函数, 升级文件路径
 * @return 成功返回2, 其他或小于0 执行升级脚本失败
 *         0:刷写成功 1:参数个数不正确 2:不满足升级条件 3:指定升级路径不成功 4:指定打印时间不成功 5:执行升级指令不成功 6:升级失败 7:升级状态错乱 
 */
int32_t HalModemUpgradeImpl(const uint8_t *filePath)
{
    int32_t ret = RET_ERR; 
    uint8_t cmd[1024] = {0x0,};

    if (!filePath || (strlen((char *)filePath) == 0)|| (strlen((char *)filePath) > 512))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    snprintf((char *)cmd, sizeof(cmd), "sh %s %s %s%s 2>&1 /dev/null", 
                              UPGRADE_SCRIPT_PATH_NAME, 
                                      (char *)filePath, 
                                      (char *)filePath, 
                                      UPGRADE_LOG_NAME);

    return GetSystemRetValue(cmd);
}

/**
 * @name HalModemGetUpgradeStatus
 * @brief 获取模组升级状态
 * @param filePath 指向待检查文件路径的指针。该文件路径不应为空，且长度不应超过512个字符。
 * @return 0:成功 1:升级中  2:需备份  3:升级失败  4:备份中  5:指令中缺少升级包路径
 */
int32_t HalModemGetUpgradeStatus(const uint8_t *filePath)
{
    int32_t ret = RET_ERR;  
    uint8_t cmd[1024] = {0x0,};

    if (!filePath || (strlen((char *)filePath) == 0)|| (strlen((char *)filePath) > 512))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    snprintf((char *)cmd, sizeof(cmd), "sh %s %s 2>&1 /dev/null", UPGRADE_STA_SCRIPT_PATH_NAME, (char *)filePath);

    return GetSystemRetValue(cmd);
}

/**
 * @name HalModemSwitch
 * @brief 切换Modem分区。
 * @param null
 * @return 返回操作的结果状态码。成功时返回RET_OK，失败时返回RET_ERR。
 */
int32_t HalModemSwitch(void)
{
    LOG_Debug("%s called.", __FUNCTION__);
    char cmd[] = "reboot";
    return GetSystemRetValue((const uint8_t*)cmd);
}