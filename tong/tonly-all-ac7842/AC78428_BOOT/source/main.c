/*************************************************
 Copyright © 2026 SiRun (AnHui) . All rights reserved.
 File Name: main.c
 Author: 
 Created Time: 2026年7月14日
 Description: 
 Others:
*************************************************/
/****************************** include ***************************************/
/* ===========================================  Includes  =========================================== */
#include "stdint.h"
#include "Debugout_AC784xx.h"
#include "AC784xx_Spm_Reg.h"
#include "Spm_Hal.h"
#include "OsIf.h"
#include "OsIf_Time.h"
#include "clock_cfg.h"
#include "mpuHal.h"
#include "logHal.h"
#include "firmwareUpdateSdk.h"
#include "AC784xx_Fls_Reg.h"
#include "flashHal.h"
#include "secureBoot.h"
#include "peripheralHal.h"

/****************************** Macro Definitions ******************************/
/****************************** Type Definitions ******************************/
/****************************** Global Variables ******************************/
static uint32_t bootTime = 0u;
int16_t g_bootMpuHandle = -1;

/****************************** Function Declarations *************************/
static void BootReadyInit(void);
static void BootRunInit();

/****************************** Public Function Implementations ***************/
/*****************************************************************************
 * Function Name: BootReadyInit
 * Description  : Initialize modules required for bootloader readiness
 * Parameters   : None
 * Return Value : None
 * Note         : This function initializes GPIO, Flash, and Log modules needed for basic bootloader functionality
****************************************************************************/
static void BootReadyInit(void)
{
    MpuHal_BootGpioInit();
    LogHalInit(1);
    TBOX_PRINT("BOOTV001, bank%d\n", Flash_Reg_GetSwapStatus());
    FlashHal_BootInit();
    SecBoot_Init();
}

/*****************************************************************************
 * Function Name: BootRunInit
 * Description  : Initialize modules required for bootloader operation
 * Parameters   : None
 * Return Value : None
 * Note         : This function initializes peripherals, MPU and other modules needed for bootloader operation
****************************************************************************/
static void BootRunInit()
{
    PeripheralHalInit();
    MpuHal_BootInit();
    FirmwareUpdateSdkInit();
    g_bootMpuHandle = MpuHalOpen();
    if (g_bootMpuHandle < 0)
    {
        TBOX_PRINT("g_bootMpuHandle failed\r\n");
    }
}

/*****************************************************************************
 * Function Name: JumpAppRun
 * Description  : Jump to the user application at the specified memory address
 * Parameters   : appStartAddress - The starting address of the application firmware (vector table base address)
 * Return Value : None
 * Note         : This function usually disables interrupts, sets the Main Stack Pointer (MSP), 
 * and jumps to the application's reset handler. If successful, this function 
 * will not return.
****************************************************************************/
static void JumpAppRun(const uint32_t appStartAddress)
{
    bootloaderFun JumpToApp;
    
    /* 跳转前，外设去初始化 */
    TBOX_PRINT("JumpToApp\r\n");
    LogHalDeInit();
    SecBoot_deInit();
    PeripheralHalFeedWatchDog();

    SysTick->CTRL  = 0;
    SysTick->LOAD  = 0;
    SysTick->VAL   = 0UL;

    __disable_irq();

    /* 清除所有挂起中断 */
    for(int i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    JumpToApp = (bootloaderFun)(*(volatile uint32_t *)(appStartAddress + 4));

    SCB->VTOR = appStartAddress;
    __set_MSP(*(__IO uint32_t*)appStartAddress);
    __set_CONTROL(0);
    __ISB();  /* 指令同步屏障 */

    JumpToApp();
}
#if 0
void CSE_DeInit(void)
{
    uint8 g_blankMemorySlot[16] =   {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   /* 内存槽初始值, 所有密钥出厂(重置后)均为该值 */
    uint8 g_masterEcuKey[16]    =   {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

    Hal_StatusType ret;
    Cse_InputKeyInfoType keyInfo = {0};
    Cse_SheKeyInfoType sheKeyFormat;
    uint8 m1[16] = {0};
    uint8 m2[32] = {0};
    uint8 m3[16] = {0};
    uint8 m4Calculate[32] = {0};
    uint8 m5Calculate[16] = {0};
    uint8 m4Out[32] = {0};
    uint8 m5Out[16] = {0};

    sheKeyFormat.M1 = m1;
    sheKeyFormat.M2 = m2;
    sheKeyFormat.M3 = m3;
    sheKeyFormat.M4 = m4Calculate;
    sheKeyFormat.M5 = m5Calculate;

    if (0x0F != Flash_Reg_GetDPartition())          /* CSE密钥空间不为默认值0KB时, 才执行擦除分区操作 */
    {
        /* 若没有加载过MASTER_ECU_KEY, 需要先加载才能执行删除分区操作 */
        keyInfo.AuthKey = g_blankMemorySlot;        /* 授权密钥明文 */
        keyInfo.NewKey = g_masterEcuKey;            /* load新密钥明文 */
        keyInfo.Count = 0x01;                       /* 内存槽load计数值 */
        keyInfo.AuthKeyId = CSE_MASTER_ECU;         /* 授权密钥ID */
        keyInfo.NewKeyId = CSE_MASTER_ECU;          /* load新密钥ID */
        keyInfo.NewKeyAttr = KEY_ATTR_NONE;         /* load新密钥属性 */

        ret = CSE_Hal_CalcM1ToM5(&keyInfo, &sheKeyFormat);
        TBOX_PRINT("\r\nCSE_Hal_CalcM1ToM5, ret = 0x%02x", ret);

        sheKeyFormat.M4 = m4Out;
        sheKeyFormat.M5 = m5Out;
        ret = CSE_Hal_LoadKey(CSE_MASTER_ECU, &sheKeyFormat);
        TBOX_PRINT("\r\nCSE_Hal_LoadKey, ret = 0x%02x", ret);

        ret = CSE_Hal_SetSecureBootMode(0, CSE_BOOT_NODEFINE);
        TBOX_PRINT("\r\nSecure Boot Configuration Is Deleted %s", ret == STATUS_SUCCESS ? "Successful" : "Fail");

        ret = CSE_Hal_ResetKey(g_masterEcuKey);     /* 清除CSE硬件接口的密钥, 输入当前实际使用的MASTER_ECU_KEY */
        TBOX_PRINT("\r\nCSE_Hal_ResetKey, ret = 0x%02x", ret);

        CSE_Hal_Deinit();

        Flash_CseType cseConfig =
        {
            .KeySize = FLASH_CSESIZE_0BYTES,
            .Sfe = FALSE
        };
        ret = Flash_Hal_SetPartition(&cseConfig);   /* 擦除CSE分区, 设置密钥空间为0KB */
        TBOX_PRINT("\r\nFlash_Hal_SetPartition, ret = 0x%02x", ret);
    }
}

void SecureDebug_DeInit(void)
{
    TBOX_PRINT("\r\nGoing To Erase All Flash...");
    Delay_ms(1000);

    CSE_DeInit();
                                /* 注意: 若存在CSE功能, 全片擦除功能将不可用; 需要先清除CSE分区之后才可执行全片擦除 */
    Flash_Hal_AllErase();       /* 执行全片擦除, 清除所有Flash内容(包括Info区)并解除读保护, 执行完后需手动复位芯片 */
}
#endif

/*************************************************
   Function:        main
   Description:     Main entry point of the application
   Input:           None
   Output:          None
   Return:          int - Return code (never actually returns in normal execution)
   Others:          Initializes system hardware and global variables, then starts the task scheduler.
                    The while(1) loop should never be reached if the scheduler runs successfully.
  *************************************************/
int main(void)
{
    static uint8_t mpuStartFlag = 0u;
    static uint8_t bootErrorBuf[8] = {0u};
    static FlashHalMetaDataInfo_t g_bootflashHalMetaDataInfo;
    static uint8_t cycleCnt = 0u;
    static uint16_t bootCnt = 0u;
    BootStatus_t kmustatus = BOOT_OK;
    BootStatus_t bootstatus = BOOT_OK;
    BootStatus_t appstatus = BOOT_OK;
    uint32_t upgradeFlag = 0u;
    uint8_t otaflag = 0;

    Spm_Reg_SelectLVDThreshold(SPM_LOW);
    Ckgen_Hal_InitClk(&g_clockGenCfg);
    Ckgen_Hal_DistributeClk(&g_clkDistributeCfg);

    BootReadyInit();

    FlashHalGetMetaDataInfo(&g_bootflashHalMetaDataInfo);
    upgradeFlag = g_bootflashHalMetaDataInfo.m_upgradeFlag;
    
    switch (upgradeFlag)
    {
        case FLASH_UPGRADE_DONE:
            TBOX_PRINT("UPGRADE DONE\n");
            g_bootflashHalMetaDataInfo.m_upgradeFlag = FLASH_UPGRADE_NONE;
            g_bootflashHalMetaDataInfo.m_release01 = 0;
            FlashHalWriteMetaDataInfo(&g_bootflashHalMetaDataInfo);
        case FLASH_UPGRADE_NONE:
            /* 使能和校验KMU */
            kmustatus = SecureBoot_KmuStart();
            /* 校验BOOT完整性和真实性 */
            bootstatus = SecureBoot_VerifyBoot();
            /* 校验APP完整性和真实性 */
            appstatus = SecureBoot_VerifyApp();
            /* 验证完成，跳转APP */
            if ((kmustatus == BOOT_OK) && (appstatus == BOOT_OK) && (bootstatus == BOOT_OK))
            {
                JumpAppRun(FLASH_APP_START_ADDRESS);
            }
            else{
                /* 验证失败，停留BOOT，等待升级 */
                TBOX_PRINT("SECURE BOOT ERR, kmu = %d, boot = %d, app = %d\n", (uint8_t)kmustatus, (uint8_t)bootstatus, (uint8_t)appstatus);
                bootErrorBuf[0] = (uint8_t)kmustatus;
                bootErrorBuf[1] = (uint8_t)bootstatus;
                bootErrorBuf[2] = (uint8_t)appstatus;
                g_bootflashHalMetaDataInfo.m_release01 = 1;
                FlashHalWriteMetaDataInfo(&g_bootflashHalMetaDataInfo);
                otaflag = 2;
            }
            break;
        case FLASH_UPGRADE_PENDING:
            TBOX_PRINT("OTA START\n");
            g_bootflashHalMetaDataInfo.m_upgradeFlag = FLASH_UPGRADE_UPDATING;
            FlashHalWriteMetaDataInfo(&g_bootflashHalMetaDataInfo);
            otaflag = 1;
            break;
        case FLASH_UPGRADE_UPDATING:
            TBOX_PRINT("FLASH_UPGRADE_UPDATING\n");
            g_bootflashHalMetaDataInfo.m_release01 = 1;
            FlashHalWriteMetaDataInfo(&g_bootflashHalMetaDataInfo);
            otaflag = 3;
        default:
            break;
    }

    BootRunInit();
    while(1)
    {
        bootCnt++;
        MpuHal_ProcessRxData();
        FirmwareUpdateSdkCycleProcess();
        MpuHal_PrintDiagnosticReport();
        PeripheralHalFeedWatchDog();
        Delay_ms(1);
        if ((bootCnt == 1000) && (otaflag != 1))
        {
            if (otaflag == 2)
            {
                MpuHalDataPack_t TxPack = {0};
                TxPack.aid = 0x00u;
                TxPack.mid = 0x00u;
                TxPack.subcommand = 0x01u;
                TxPack.pDataBuffer = bootErrorBuf;
                TxPack.dataBufferSize = sizeof(bootErrorBuf);
                TxPack.dataLength = 3;
                MpuHal_Transmit(g_bootMpuHandle, &TxPack);
            }
            bootCnt = 0;
            TBOX_PRINT("BOOT cycle %d s\n", (++bootTime));
        }
        if ((otaflag != 1) && (g_bootflashHalMetaDataInfo.m_release01 == 1))
        {
            /* 触发回滚 */
            if (otaflag == 3)
            {
                TBOX_PRINT("Ota Fail Upgrade Rollback, Reset...\n");
                g_bootflashHalMetaDataInfo.m_upgradeFlag = FLASH_UPGRADE_NONE;
                g_bootflashHalMetaDataInfo.m_release01 = 0;
                FlashHalWriteMetaDataInfo(&g_bootflashHalMetaDataInfo);
                Delay_ms(500);
                PeripheralHalMcuHardReset();
                PeripheralHalMcuHardReset();
                PeripheralHalMcuHardReset();
                TBOX_PRINT("---Reset ERROR---\r\n");
            }
            /* 安全启动失败时，给 MPU 上电 */
            if (mpuStartFlag == 0)
            {
                MpuHalStart();
                mpuStartFlag = 1;
            }
            cycleCnt++;
            if (cycleCnt >= 5)
            {
                MpuHalCycleProcess(5);
                cycleCnt = 0u;
            }  
        }
    }
}

/* =============================================  EOF  ============================================== */
