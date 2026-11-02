/*************************************************
 Copyright © 2026 SiRun (AnHui) . All rights reserved.
 File Name: secureBoot.c
 Author: HuShangrui
 Created Time: 2026年7月14日
 Description: 
 Others:
*************************************************/
#include "secureBoot.h"
#include "flashHal.h"
#include "logHal.h"
#include "Cse_Hal.h"
#include "Fls_Hal.h"
#include "AC784xx_Fls_Reg.h"

/****************************** Macro Definitions ******************************/
#define SECURE_BOOT_KMU_OTP_ENABLE          0
#define SECUREBOOT_HEAR_MAGIC_HEAD          0x5A6A7A8AU
#define SECUREBOOT_HEAR_MAGIC_TAIL          0x8A7A6A5AU

#define SECUREBOOT_KMU_OTP_CODE             ((uint32_t)0xC0DEC0DEU)

#define SHA256_BYTE_LEN_32         (32U)
#define SAES256CMAC_BYTE_LEN_16    (16U)

/****************************** Type Definitions ******************************/
typedef struct {
    uint32_t magic_head;                           // 0x5a6a7a8a
    uint32_t start_addr;                           // 0x00054000
    uint32_t totalSize;                            // APP 实际大小
    uint8_t  sha256[SHA256_BYTE_LEN_32];           // 校验完整性的 SHA256 原文
    uint8_t  aes256cmac[SAES256CMAC_BYTE_LEN_16];  // 校验真实性的 AES CMAC 密文
    uint32_t magic_tail;                           // 0x8a7a6a5a
} __attribute__((packed)) SecureHeader_t;

// 2. 状态机流转状态枚举 (State)
typedef enum {
    STATE_CHECK_BOOT0_INTEGRITY = 0,
    STATE_CHECK_BOOT0_AUTH,
    STATE_CHECK_APP_HEADER,
    STATE_CHECK_APP_INTEGRITY,
    STATE_CHECK_APP_AUTH,
    STATE_JUMP_TO_APP,
    STATE_HALT_ERROR              // 统一的错误挂起/处理状态
} BootState_t;

/****************************** Global Variables ******************************/
BootState_t g_securebootState = STATE_CHECK_BOOT0_INTEGRITY;
SecureHeader_t g_appHeader;
SecureHeader_t g_bootHeader;

static uint8_t g_Aes128AppBuf[SAES256CMAC_BYTE_LEN_16];
static uint8_t g_Aes128BootBuf[SAES256CMAC_BYTE_LEN_16];

static const uint8_t HARDCODED_BOOT_KEY[16] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
};

/****************************** Function Declarations *************************/
static BootStatus_t SecureBoot_CheckMagicCode_App(void);
static BootStatus_t SecureBoot_CheckAppAuth(void);
static BootStatus_t SecureBoot_CheckMagicCode_Boot(void);
static BootStatus_t SecureBoot_CheckBootAuth(void);

/****************************** Public Function Implementations ***************/

/*****************************************************************************
 * Function Name: SecBoot_Init
 * Description  : 初始化安全启动环境
 * Parameters   : None
 * Return Value : None
 * Note         : 初始化底层 CSE 硬件
****************************************************************************/
void SecBoot_Init(void)
{
    Hal_StatusType stat;
    /* CSE的UID总共120bit, 数组的后15byte才是有效的, byte 0仅做填充 */
    uint8 uid[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x01};

    if (0x0F == Flash_Reg_GetDPartition())          /* CSE密钥空间为默认值0KB时, 才执行分区操作 */
    {
        Flash_CseType cseConfig =
        {
            .KeySize = FLASH_CSESIZE_512BYTES,
            .PartSize = FLASH_DLFASHSIZE_64KB,
            .Sfe = TRUE
        };
        Flash_Hal_EraseCseInfo();                           /* 擦除CSE Info Page */
        Flash_Hal_ProgramCseInfo(CSE_UID_BASE, 16, uid);    /* 写入自定义UID */
        stat = Flash_Hal_SetPartition(&cseConfig);           /* 注意: 设置CSE分区后无法执行全片擦除 */
        TBOX_PRINT("\r\nFlash_Hal_SetPartition, ret = 0x%02x", stat);
        Delay_ms(1000);

        NVIC_SystemReset();                         /* 重启系统, 使配置生效 */
    }
    
    /* 初始化 CSE 模块并使其就绪 */
    stat = CSE_Hal_Init();
    if (stat != STATUS_SUCCESS)
    {
        TBOX_PRINT("CSE Hardware Init Failed! Status: %d\n", stat);
    }
}

/*****************************************************************************
 * Function Name: SecBoot_deInit
 * Description  : 反初始化安全启动环境
 * Parameters   : None
 * Return Value : None
 * Note         : 通常在安全校验完成且准备跳转到应用固件(APP)之前调用，以确保系统状态干净，防止信息泄漏
****************************************************************************/
void SecBoot_deInit(void)
{
    // do nothing
}

/*****************************************************************************
 * Function Name: SecureBoot_CheckMagicCode_App
 * Description  : 校验APP魔术头与魔术尾，确保固件起始标志合法
 * Parameters   : None
 * Return Value : BootStatus_t
 * Note         : 
****************************************************************************/
static BootStatus_t SecureBoot_CheckMagicCode_App(void)
{
    SecureHeader_t *app_header = (SecureHeader_t *)FLASH_APP_BANKA_ABSTRACT_ADDRESS;

    memcpy(&g_appHeader, app_header, sizeof(SecureHeader_t));
    
    if ((g_appHeader.magic_head != SECUREBOOT_HEAR_MAGIC_HEAD) || (g_appHeader.magic_tail != SECUREBOOT_HEAR_MAGIC_TAIL))
    {
        TBOX_PRINT("Check App MagicCode ERR\n");
        return BOOT_ERR_HEADER_INVALID; // 魔术头校验失败
    }
    
    return BOOT_OK; // 校验成功
}

/*****************************************************************************
 * Function Name: SecureBoot_CheckAppAuth
 * Description  : 校验APP固件真实性 (基于硬件 CSE 的 AES-128 CMAC)
 * Parameters   : None
 * Return Value : BootStatus_t
 * Note         : 
****************************************************************************/
static BootStatus_t SecureBoot_CheckAppAuth(void)
{
    Hal_StatusType stat;
    Cse_InputOutputType cseInput;
    uint8_t Index = 0;

    /* 1. 检查固件参数合法性 (确保16字节对齐等基础要求) */
    if ((g_appHeader.start_addr % 16U != 0) || (g_appHeader.totalSize == 0U))
    {
        TBOX_PRINT("Auth Param Alignment ERR\n");
        return BOOT_ERR_APP_INTEGRITY;
    }

    stat = CSE_Hal_LoadPlainKey(HARDCODED_BOOT_KEY);
    if (stat != STATUS_SUCCESS)
    {
        TBOX_PRINT("Load Plain Key ERR, stat: %d\n", stat);
        return BOOT_ERR_APP_INTEGRITY;
    }

    /* 2. 配置硬件 CMAC 校验参数 */
    cseInput.InputPtr = (const uint8_t *)g_appHeader.start_addr;  
    cseInput.InputLength = g_appHeader.totalSize; 
    cseInput.OutputPtr = g_Aes128AppBuf;

    /* 3. 执行 Flash 地址模式的 MAC 校验 */
    stat = CSE_Hal_GenerateMAC(&cseInput, CSE_RAM_KEY, 100000U);

    /* 4. 判断计算过程是否发生硬件级错误及验证结果 */
    if (stat != STATUS_SUCCESS)
    {
        TBOX_PRINT("Check Boot CSE Hardware ERR, stat: %d\n", stat);
        return BOOT_ERR_BOOT_INTEGRITY;
    }

    /* 4. 逐字节比对算出的 AES256CMAC, 安全启动不建议使用memcmp */
    for (Index = 0U; Index < SAES256CMAC_BYTE_LEN_16; Index++)
    {
        if (g_Aes128AppBuf[Index] != g_appHeader.aes256cmac[Index])
        {
            uint32_t *p_calc = (uint32_t *)g_Aes128AppBuf;
            TBOX_PRINT("Calc CMAC: %08X %08X %08X %08X ERROR\n", 
                       p_calc[0], p_calc[1], p_calc[2], p_calc[3]);
            return BOOT_ERR_APP_INTEGRITY; // 比对失败：固件损坏或被篡改
        }
    }

    return BOOT_OK;
}

/*****************************************************************************
 * Function Name: SecureBoot_CheckMagicCode_Boot
 * Description  : 校验BOOT魔术头与魔术尾
 * Parameters   : None
 * Return Value : BootStatus_t
 * Note         : 
****************************************************************************/
static BootStatus_t SecureBoot_CheckMagicCode_Boot(void)
{
    SecureHeader_t *boot_header = (SecureHeader_t *)FLASH_BOOT_ABSTRACT_ADDRESS;

    memcpy(&g_bootHeader, boot_header, sizeof(SecureHeader_t));
    
    if ((g_bootHeader.magic_head != SECUREBOOT_HEAR_MAGIC_HEAD) || (g_bootHeader.magic_tail != SECUREBOOT_HEAR_MAGIC_TAIL))
    {
        TBOX_PRINT("Check Boot MagicCode ERR\n");
        return BOOT_ERR_HEADER_INVALID; // 魔术头校验失败
    }
    
    return BOOT_OK; // 校验成功
}

/*****************************************************************************
 * Function Name: SecureBoot_CheckBootAuth
 * Description  : 校验Boot固件真实性 (基于硬件 CSE 的 AES-128 CMAC)
 * Parameters   : None
 * Return Value : BootStatus_t
 * Note         : 
****************************************************************************/
static BootStatus_t SecureBoot_CheckBootAuth(void)
{
    Hal_StatusType stat;
    Cse_InputOutputType cseInput;
    uint8_t Index = 0;
    
    /* 1. 检查固件参数合法性 (确保16字节对齐等基础要求) */
    if ((g_bootHeader.start_addr % 16U != 0) || (g_bootHeader.totalSize == 0U))
    {
        TBOX_PRINT("Auth Param Alignment ERR\n");
        return BOOT_ERR_BOOT_INTEGRITY;
    }

    stat = CSE_Hal_LoadPlainKey(HARDCODED_BOOT_KEY);
    if (stat != STATUS_SUCCESS)
    {
        TBOX_PRINT("Load Plain Key ERR, stat: %d\n", stat);
        return BOOT_ERR_APP_INTEGRITY;
    }


    /* 2. 配置硬件 CMAC 校验参数 */
    cseInput.InputPtr = (const uint8_t *)g_bootHeader.start_addr;  
    cseInput.InputLength = g_bootHeader.totalSize; 
    cseInput.OutputPtr = g_Aes128BootBuf;

    /* 3. 执行 Flash 地址模式的 MAC 校验 */
    stat = CSE_Hal_GenerateMAC(&cseInput, CSE_RAM_KEY, 100000U);

    /* 4. 判断计算过程是否发生硬件级错误及验证结果 */
    if (stat != STATUS_SUCCESS)
    {
        TBOX_PRINT("Check Boot CSE Hardware ERR, stat: %d\n", stat);
        return BOOT_ERR_BOOT_INTEGRITY;
    }

    /* 4. 逐字节比对算出的 AES256CMAC, 安全启动不建议使用memcmp */
    for (Index = 0U; Index < SAES256CMAC_BYTE_LEN_16; Index++)
    {
        if (g_Aes128BootBuf[Index] != g_bootHeader.aes256cmac[Index])
        {
            uint32_t *p_calc = (uint32_t *)g_Aes128BootBuf;
            TBOX_PRINT("Calc CMAC: %08X %08X %08X %08X ERROR\n", 
                       p_calc[0], p_calc[1], p_calc[2], p_calc[3]);
            return BOOT_ERR_APP_INTEGRITY; // 比对失败：固件损坏或被篡改
        }
    }

    return BOOT_OK;
}

/*****************************************************************************
 * Function Name: SecureBoot_CheckBackupBoot
 * Description  : 验证备份区BOOT是否有效
 * Parameters   : None
 * Return Value : BootStatus_t
 * Note         : 
****************************************************************************/
BootStatus_t SecureBoot_CheckBackupBoot(void)
{
    Hal_StatusType stat;
    Cse_InputOutputType cseInput;
    static SecureHeader_t g_backupBootHeader;
    SecureHeader_t *boot_header = (SecureHeader_t *)FLASH_BOOT_BANKB_ABSTRACT_ADDRESS;
    uint8_t Index = 0;
    
    memcpy(&g_backupBootHeader, boot_header, sizeof(SecureHeader_t));
    
    /* 1. 合法性校验 */
    if ((g_backupBootHeader.magic_head != SECUREBOOT_HEAR_MAGIC_HEAD) || (g_backupBootHeader.magic_tail != SECUREBOOT_HEAR_MAGIC_TAIL))
    {
        TBOX_PRINT("Check Boot MagicCode ERR\n");
        return BOOT_ERR_HEADER_INVALID; // 魔术头校验失败
    }

    if ((g_backupBootHeader.totalSize == 0U) || (g_backupBootHeader.totalSize == 0xFFFFFFFFU))
    {
        TBOX_PRINT("Check BOOT SIZE ERR\n");
        return BOOT_ERR_BOOT_INTEGRITY;
    }

    stat = CSE_Hal_LoadPlainKey(HARDCODED_BOOT_KEY);
    if (stat != STATUS_SUCCESS)
    {
        TBOX_PRINT("Load Plain Key ERR, stat: %d\n", stat);
        return BOOT_ERR_APP_INTEGRITY;
    }

    /* 2. 配置硬件 CMAC 校验参数 */    
    cseInput.InputPtr = (const uint8_t *)g_backupBootHeader.start_addr;  
    cseInput.InputLength = g_backupBootHeader.totalSize; 
    cseInput.OutputPtr = g_Aes128BootBuf;

    /* 3. 执行 Flash 地址模式的 MAC 校验 */
    stat = CSE_Hal_GenerateMAC(&cseInput, CSE_RAM_KEY, 100000U);

    /* 4. 判断计算过程是否发生硬件级错误及验证结果 */
    if (stat != STATUS_SUCCESS)
    {
        TBOX_PRINT("Check Boot CSE Hardware ERR, stat: %d\n", stat);
        return BOOT_ERR_BOOT_INTEGRITY;
    }

    /* 4. 逐字节比对算出的 AES256CMAC, 安全启动不建议使用memcmp */
    for (Index = 0U; Index < SAES256CMAC_BYTE_LEN_16; Index++)
    {
        if (g_Aes128BootBuf[Index] != g_backupBootHeader.aes256cmac[Index])
        {
#if 0
            uint32_t *p_calc = (uint32_t *)g_Aes128BootBuf;
            TBOX_PRINT("Calc CMAC: %08X %08X %08X %08X ERROR\n", 
                       p_calc[0], p_calc[1], p_calc[2], p_calc[3]);
#endif
            return BOOT_ERR_APP_INTEGRITY; // 比对失败：固件损坏或被篡改
        }
    }

    return BOOT_OK; // 校验成功
}

/*****************************************************************************
 * Function Name: SecureBoot_VerifyApp
 * Description  : 校验APP的综合对外接口 (魔术字 -> 完整性 -> 真实性)
 * Parameters   : None
 * Return Value : BootStatus_t
 * Note         : 
****************************************************************************/
BootStatus_t SecureBoot_VerifyApp(void)
{
    BootStatus_t status = BOOT_OK;

    /* 1. 校验魔术头尾 */
    status = SecureBoot_CheckMagicCode_App();
    if (status != BOOT_OK)
    {
        return status;
    }

    /* 3. 校验APP完整性真实性 (AES-128 CMAC) */
    status = SecureBoot_CheckAppAuth();
    if (status != BOOT_OK)
    {
        return status;
    }

    return status;
}


/*****************************************************************************
 * Function Name: SecureBoot_KmuStart
 * Description  : KUM启用
 * Parameters   : None
 * Return Value : BootStatus_t
 * Note         : 
****************************************************************************/
BootStatus_t SecureBoot_KmuStart(void)
{
    return BOOT_OK;
}

/*****************************************************************************
 * Function Name: SecureBoot_VerifyBoot
 * Description  : 校验Boot的综合对外接口 (魔术字 -> 完整性 -> 真实性)
 * Parameters   : None
 * Return Value : BootStatus_t
 * Note         : 
****************************************************************************/
BootStatus_t SecureBoot_VerifyBoot(void)
{
    BootStatus_t status = BOOT_OK;

    /* 1. 校验魔术头尾 */
    status = SecureBoot_CheckMagicCode_Boot();
    if (status != BOOT_OK)
    {
        return status;
    }

    /* 3. 校验BOOT完整性真实性 (AES-128 CMAC) */
    status = SecureBoot_CheckBootAuth();
    if (status != BOOT_OK)
    {
        return status;
    }

    return status;
}