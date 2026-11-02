/*************************************************
 Copyright © 2026 SiRun (AnHui) . All rights reserved.
 File Name: secureBoot.h
 Author: HuShangrui
 Created Time: 2026年5月20日
 Description: 
 Others:
*************************************************/
#ifndef SECUREBOOT_H
#define SECUREBOOT_H
/****************************** include ***************************************/
#include <stdint.h>
#include "string.h"

/****************************** Macro Definitions ******************************/
/****************************** Type Definitions ******************************/
typedef enum {
    BOOT_OK = 0,                  // 执行成功
    BOOT_ERR_KMU_ENABLE,          // KUM 使能失败
    BOOT_ERR_KMU_CHECK,           // KUM 自检失败
    BOOT_ERR_BOOT_INTEGRITY,      // BOOT SHA 校验失败
    BOOT_ERR_BOOT_KEY,            // BOOT 密钥加载错误
    BOOT_ERR_BOOT_AUTH,           // BOOT 真实性校验失败
    BOOT_ERR_HEADER_INVALID,      // 头部魔术字(Magic)不匹配或结构异常
    BOOT_ERR_APP_INTEGRITY,       // APP SHA256 不匹配（固件损坏）
    BOOT_ERR_APP_KEY,             // APP 密钥加载错误
    BOOT_ERR_APP_AUTH,            // APP AES 解密后比对失败（固件非法）
    BOOT_ERR_APP_ADDRESS_INVALID, // 跳转地址非法
    BOOT_ERR_SYSTEM_FATAL         // 硬件层崩溃或异常
} BootStatus_t;

/****************************** Global Variables ******************************/
/****************************** Function Declarations *************************/
/****************************** Public Function Implementations ***************/
/*****************************************************************************
 * Function Name: SecBoot_Init
 * Description  : 初始化安全启动环境
 * Parameters   : None
 * Return Value : None
 * Note         : 
****************************************************************************/
void SecBoot_Init(void);

/*****************************************************************************
 * Function Name: SecBoot_deInit
 * Description  : 反初始化安全启动环境
 * Parameters   : None
 * Return Value : None
 * Note         : 通常在安全校验完成且准备跳转到应用固件(APP)之前调用，以确保系统状态干净，防止信息泄漏
****************************************************************************/
void SecBoot_deInit(void);

/*****************************************************************************
 * Function Name: SecureBoot_KmuStart
 * Description  : KUM启用
 * Parameters   : None
 * Return Value : BootStatus_t
 * Note         : 
****************************************************************************/
BootStatus_t SecureBoot_KmuStart(void);

/*****************************************************************************
 * Function Name: SecureBoot_CheckBackupBoot
 * Description  : 验证备份区BOOT是否有效
 * Parameters   : None
 * Return Value : BootStatus_t
 * Note         : 
****************************************************************************/
BootStatus_t SecureBoot_CheckBackupBoot(void);

/*****************************************************************************
 * Function Name: SecureBoot_VerifyApp
 * Description  : 校验APP的综合对外接口 (魔术字 -> 完整性 -> 真实性)
 * Parameters   : None
 * Return Value : BootStatus_t
 * Note         : 
****************************************************************************/
BootStatus_t SecureBoot_VerifyApp(void);

/*****************************************************************************
 * Function Name: SecureBoot_VerifyBoot
 * Description  : 校验Boot的综合对外接口 (魔术字 -> 完整性 -> 真实性)
 * Parameters   : None
 * Return Value : BootStatus_t
 * Note         : 
****************************************************************************/
BootStatus_t SecureBoot_VerifyBoot(void);



#endif // SECUREBOOT_H