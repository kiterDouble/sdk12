/*************************************************
 Copyright © 2026 SiRun (AnHui) . All rights reserved.
 File Name: firmwareUpdateSdk.c
 Author: Hushangrui
 Created Time: 2026年5月11日
 Description: 
 Others: 
*************************************************/
/****************************** include ***************************************/
#include "stdbool.h"
#include "firmwareUpdateSdk.h"
#include "stdlib.h"
#include "crc8_16_32.h"
#include "mpuHal.h"
#include "flashHal.h"
#include "peripheralHal.h"
#include "logHal.h"
#include "firmwareUpdateSdk.h"
#include "string.h"
#include "secureBoot.h"
#include "Fls_Hal.h"
#include "AC784xx_Fls_Reg.h"

/****************************** Macro Definitions ******************************/
/****************************** Type Definitions ******************************/
/****************************** Global Variables ******************************/
static FlashState_e g_flashState = E_FlashState_Idle;  
static uint8_t g_versionNumber[20] = {'v','0','0','1','.','0','0','2',};                              //软件版本号长度
static Crc32Objec_t g_crc32Object;
static uint32_t g_crcData = 0xFFFFFFFF;
static FlashHalMetaDataInfo_t s_MetaDataInfo;
uint8_t g_otaResetFlag = 0u;

int16_t g_mpuHandle = -1;
static uint8_t g_dataBuffer[2200] = {0};
static MpuHalDataPack_t  g_dataPack;
static uint8_t g_recvDataBuffer[4096] = {0};

/* 超时重发相关变量 */
static uint8_t g_lastTxDataBuffer[20] = {0}; // 缓存上一帧的发送数据
static MpuHalDataPack_t g_lastTxPack = {0};  // 缓存上一帧的包结构
static uint16_t g_ackWaitTimeoutCnt = 0;     // 1s超时计数器
static uint8_t g_isWaitingForNextFrame = 0;  // 是否处于等待下一帧的状态标志

/****************************** Function Declarations *************************/
static uint8_t FirmwareUpdateSdkEraseFlash(uint8_t *dataPack);
static uint8_t FirmwareUpdateSdkLoadCode(uint8_t *dataPack, uint16_t dataBufferSize);
static uint8_t FirmwareUpdateSdkCodeCheck(uint8_t *dataPack);

/****************************** Public Function Implementations ***************/
/*************************************************
* Function:       FirmwareUpdateSdkEraseFlash
* Description:    Erase flash area according to specified address in data packet
* Input:          dataPack: Data packet containing erase command and address information
*                             - dataPack[10-13]: Verification code (should be 0x01,0x02,0x03,0x04)
*                             - dataPack[14-17]: Erase start address
* Output:         None
* Return:         0: Success
*                 1: Flash erase failed
*                 2: Data verification failed
* Others:         Uses global variables: flashAppFlag, g_flashState, g_crcData
*************************************************/
static uint8_t FirmwareUpdateSdkEraseFlash(uint8_t *dataPack)
{
    uint8_t ret = FLASH_HAL_STATUS_OK;
    uint32_t address = 0;
    
    TBOX_PRINT("--05 Erase flash code %d,%d,%d,%d--\r\n", dataPack[2], dataPack[3], dataPack[4], dataPack[5]);
    if (dataPack[2] == 0x01 && dataPack[3] == 0x02 && dataPack[4] == 0x03 && dataPack[5] == 0x04)
    {
        address = ((dataPack[6] << 24) + (dataPack[7] << 16)+ (dataPack[8] << 8) + dataPack[9]);

        if (address != FLASH_APP_BANKA_ABSTRACT_ADDRESS)
        {
            TBOX_PRINT("05 address error,  start address is 0x%x\r\n", address);
            return 3;
        }

        if (FLASH_HAL_STATUS_OK == FlashHalOtaFlashErase())
        {
            g_flashState = E_FlashState_FlashErase;
            g_crcData = Crc32Init(&g_crc32Object,0x04C11DB7);
        }
        else
        {
            TBOX_PRINT("Erase flash error\r\n");
            g_flashState = E_FlashState_Idle;
            ret = 1;
        }
    }
    else
    {
        g_flashState = E_FlashState_Idle;
        ret = 2;
    }
    return ret;
}

/*************************************************
 Function: FirmwareUpdateSdkLoadCode
 Description: 加载代码到指定地址
 Input: dataPack 数据打包结构体指针
        dataBufferSize 数据缓冲区大小
 Output: None
 Return: E_OK-成功, E_NOT_OK-失败
 Others:
*************************************************/
static uint8_t FirmwareUpdateSdkLoadCode(uint8_t *dataPack, uint16_t dataBufferSize)
{
    uint8_t ret = 0U;
    uint32_t len = 0U;
    uint32_t addr = 0U;

    if (dataPack != NULL) 
    {
        if (g_flashState == E_FlashState_FlashErase)
        {
            addr = ((uint32_t)dataPack[2] << 24) | ((uint32_t)dataPack[3] << 16) | ((uint32_t)dataPack[4] << 8)  | (uint32_t)dataPack[5];
            addr += FLASH_BANK_SIZE;
            
            len = dataBufferSize - 10;
            
            TBOX_PRINT("--06 addr=%08x len=%d cnt=%02x %02x--\r\n", addr, len, dataPack[0], dataPack[1]);

            if (len <= 0 || (len > (FLASH_APP_SINGLE_SIZE - FLASH_SECTOR_SIZE)))
            {
                TBOX_PRINT("06 invalid data length: %d\r\n", len);
                return 0x0A;
            }
            if ((addr < FLASH_APP_BANKB_ABSTRACT_ADDRESS) || (addr > FLASH_ALL_END_ADDR))
            {
                TBOX_PRINT("06 address out of range: %08x\r\n", addr);
                return 0x0B;
            }
            if ((addr + len) > FLASH_ALL_END_ADDR)
            {
                TBOX_PRINT("06 address+length exceeds flash boundary: %08x\r\n", (addr + len));
                return 0x0C;
            }

            if (FLASH_HAL_STATUS_OK != FlashHalOtaFlashWrite(addr, &dataPack[10], len))
            {
                TBOX_PRINT("06 FlashHalOtaFlashWrite operation failed\r\n");
                ret = 0x03; 
            }
            else
            {
                g_crcData = Crc32(&g_crc32Object, g_crcData, (uint8_t *)addr, len);
            }

        }
        else
        {
            TBOX_PRINT("--06 Invalid flash state: %d (expected: %d)--\r\n", 
                      g_flashState, E_FlashState_FlashErase);
            ret = 0x06; 
        }
    }
    else
    {
        TBOX_PRINT("FirmwareUpdateSdkLoadCode: dataPack is NULL\r\n");
    }
    return ret;
}

/*************************************************
  Function:       FirmwareUpdateSdkCodeCheck
  Description:    Verify downloaded firmware code integrity and update metadata
  Input:          dataPack: Data packet containing checksum and completion information
                  - dataPack[10-13]: Expected CRC32 checksum (big-endian)
  Output:         None
  Return:         Success: 0
                  Error: 0x01 (CRC32 checksum mismatch)
  Others:         Uses global variables g_crcData, flashAppFlag, s_MetaDataInfo
                  Updates flash metadata and integrity flags upon successful verification
*************************************************/
static uint8_t FirmwareUpdateSdkCodeCheck(uint8_t *dataPack)
{
    uint8_t ret = 0U;
    uint8_t newVersionData[8] = {0};
    uint32_t bankversion = 0;
    
    TBOX_PRINT("--07 Download Complete--\r\n");

    g_crcData ^= 0xFFFFFFFF; 
    uint32_t checksum = (dataPack[2] << 24) + (dataPack[3] << 16) + (dataPack[4] << 8) + dataPack[5];
    if(checksum != g_crcData)
    {
        TBOX_PRINT("checksum = %x, g_crcData = %x\r\n", checksum, g_crcData);
        return 0x01; 
    }

    memset(newVersionData, 0, 8);
    FlashHalGetMetaDataInfo(&s_MetaDataInfo);
    s_MetaDataInfo.m_upgradeFlag = FLASH_UPGRADE_NONE;
    s_MetaDataInfo.m_integrityFlag = FLASH_BANK_APP_INTEGRITY_ENABLE;
    bankversion = s_MetaDataInfo.m_currentVersion;
    newVersionData[0] = (uint8_t)(bankversion & 0xFF);
    newVersionData[1] = (uint8_t)((bankversion >> 8) & 0xFF);
    newVersionData[2] = (uint8_t)((bankversion >> 16) & 0xFF);
    newVersionData[3] = (uint8_t)((bankversion >> 24) & 0xFF);
    PeripheralHalFeedWatchDog();

    uint32_t bank = Flash_Reg_GetSwapStatus();
    if (bank == 1)
    {
        s_MetaDataInfo.m_activePartition = FLASH_APP_BANKB_ID;
        FlashHalWriteMetaDataInfo(&s_MetaDataInfo);
        /* 当前区是低位区，切到高位区 */
        uint8_t sta1 = (uint8_t)Flash_Hal_Swap(true);
        if (sta1 != 0)
        {
            TBOX_PRINT("WriteLowerVersionValue error %d\n", sta1);
            ret = 1;
        }
    }
    else
    {
        s_MetaDataInfo.m_activePartition = FLASH_APP_BANKA_ID;
        FlashHalWriteMetaDataInfo(&s_MetaDataInfo);
        /* 当前区是高位区，切到低位区 */
        uint8_t sta1 = (uint8_t)Flash_Hal_Swap(false);
        if (sta1 != 0)
        {
            TBOX_PRINT("WriteLowerVersionValue error %d\n", sta1);
            ret = 1;
        }
    }
    PeripheralHalFeedWatchDog();
    s_MetaDataInfo.m_currentVersion = bankversion + 1;
    s_MetaDataInfo.m_upgradeFlag = FLASH_UPGRADE_DONE;
    FlashHalWriteBankBMetaDataInfo(&s_MetaDataInfo);

    return ret;
}


/*************************************************
 Function: FirmwareUpdateSdkInit
 Description: 初始化固件升级SDK
 Input: void
 Output: None
 Return: int16_t 0 成功 -1 失败
 Others:
*************************************************/
int16_t FirmwareUpdateSdkInit(void)
{
    MpuHalFilter_t filter;
    
    g_dataPack.pDataBuffer = g_dataBuffer;
    g_dataPack.dataBufferSize = sizeof(g_dataBuffer);

    filter.aid = 0x03;
    filter.midMin = 0x01;
    filter.midMax = 0xFF;

    g_mpuHandle = MpuHalOpen();
    if (g_mpuHandle < 0)
    {
        TBOX_PRINT("MpuHalOpen failed\r\n");
        return -1;
    }

    MpuHalSetRxFilter(g_mpuHandle, &filter);
    MpuHalSetRxBuffer(g_mpuHandle, g_recvDataBuffer, sizeof(g_recvDataBuffer));
    return 0;
}

/*************************************************
  Function:       FirmwareUpdateSdkCycleProcess
  Description:    Firmware update module cycle process interface
  Input:          None
  Output:         None
  Return:         None
  Others:         
*************************************************/
void FirmwareUpdateSdkCycleProcess(void)
{
    int16_t ret = MPU_HAL_STATUS_ERR;
    uint8_t i = 0U;
    uint8_t curAid = 0U;
    uint8_t dataTxLen = 0U;
    static uint8_t dataTxAraay[20] = {0};
    static uint32_t otaResetCnt = 0;
    FirmwareUpdateSdkCmd_e s_UpdateMid = E_FirmwareUpdateSdkCmd_Default;
    uint16_t newCmdCounter = 0;
    static uint16_t lastCmdCounter = 0xff11U;

    if (g_otaResetFlag == 1)
    {
        otaResetCnt++;
        if (otaResetCnt > 15000) // 15s超时复位
        {
            TBOX_PRINT("Boot Ota Rev TimeOut Reset\r\n");
            Delay_ms(500);
            PeripheralHalMcuHardReset();
            PeripheralHalMcuHardReset();
            PeripheralHalMcuHardReset();
            TBOX_PRINT("---Reset ERROR---\r\n");
        }
    }
    ret = MpuHal_Receive(g_mpuHandle, &g_dataPack, 0);
    if (ret == MPU_HAL_STATUS_OK)
    {
        g_isWaitingForNextFrame = 0;
        g_ackWaitTimeoutCnt = 0;

        curAid = g_dataPack.aid;   
        if (curAid == PROTOCOL_AID_FWUPD)
        {
            s_UpdateMid = (FirmwareUpdateSdkCmd_e)g_dataPack.mid;
            memset(dataTxAraay, 0, sizeof(dataTxAraay));
            /* 2字节counter */
            dataTxAraay[0] = g_dataPack.pDataBuffer[0];
            dataTxAraay[1] = g_dataPack.pDataBuffer[1];
            newCmdCounter = (uint16_t)(dataTxAraay[0] << 8) | dataTxAraay[1];
            /* 刷新 BOOT 下复位保底计数器 */
            g_otaResetFlag = 1;
            otaResetCnt = 0;

            switch(s_UpdateMid)
            {
                case E_FirmwareUpdateSdkCmd_GetMcuVersion:
                    TBOX_PRINT("---01 Read Version---\r\n");
                    dataTxAraay[2] = 0x00; 
                    /* 11字节版本号 */
                    for(i = 0; i < 11U; i++) {
                        dataTxAraay[3 + i] = g_versionNumber[i]; 
                    }
                    dataTxLen = 14U;
                    break;

                case E_FirmwareUpdateSdkCmd_GetMcuSeed:
                    TBOX_PRINT("---02 Get Seed---\r\n");
                    dataTxAraay[2] = 0x00; // status
                    /* 4字节种子 */
                    dataTxAraay[3] = 0x01; 
                    dataTxAraay[4] = 0x02;
                    dataTxAraay[5] = 0x03; 
                    dataTxAraay[6] = 0x04;
                    dataTxLen = 7U;
                    break;

                case E_FirmwareUpdateSdkCmd_UnlockMcuFlash:
                    TBOX_PRINT("---03 Unlock Flash---\r\n");
                    if((g_dataPack.pDataBuffer[2] == 0x01) && (g_dataPack.pDataBuffer[3] == 0x02) && (g_dataPack.pDataBuffer[4] == 0x03) && (g_dataPack.pDataBuffer[5] == 0x04))
                    { 
                        dataTxAraay[2] = 0x00; 
                    } 
                    else 
                    { 
                        dataTxAraay[2] = 0x01; 
                    }
                    dataTxLen = 3U;
                    break;

                case E_FirmwareUpdateSdkCmd_EnterDownloadMode:
                    TBOX_PRINT("---04 Enter Download Mode---\r\n");
                    if ((g_dataPack.pDataBuffer[2] == 0x01) && (g_dataPack.pDataBuffer[3] == 0x02) && (g_dataPack.pDataBuffer[4] == 0x03) && (g_dataPack.pDataBuffer[5] == 0x04))
                    {
                        if (newCmdCounter != lastCmdCounter)
                        {
                            uint8_t ret = FLASH_HAL_STATUS_OK;
                            uint8_t cnt = 3;
                            /* 检查备份区BOOT是否有效 */
                            while (cnt --)
                            {
                                BootStatus_t bootStatus = SecureBoot_CheckBackupBoot();
                                PeripheralHalFeedWatchDog();
                                if (bootStatus == BOOT_OK)
                                {
                                    TBOX_PRINT("Check BackUp OK\n");
                                    ret = FLASH_HAL_STATUS_OK;
                                    break;
                                }
                                ret = FlashHal_CopyBoot();
                                TBOX_PRINT("Copy Boot ret %d\n", ret);
                                PeripheralHalFeedWatchDog();
                            }

                            if (ret != FLASH_HAL_STATUS_OK)
                            {
                                dataTxAraay[2] = 0x02;
                                g_flashState = E_FlashState_Idle;
                                TBOX_PRINT("backup boot copy error\n");
                            }
                            else
                            {
                                dataTxAraay[2] = 0x00;
                                g_flashState = E_FlashState_FlashIn; // 进入下载状态
                            }
                        }
                        else
                        {
                            TBOX_PRINT("Same cmd counter\r\n");
                            dataTxAraay[2] = 0x00; 
                        }
                    } 
                    else 
                    { 
                        dataTxAraay[2] = 0x01; 
                    }
                    dataTxLen = 3U;
                    break;

                case E_FirmwareUpdateSdkCmd_EraseMcuMemory:
                    /* 传入数据 1 2 3 4 地址 长度 */
                    if (newCmdCounter != lastCmdCounter)
                    {
                        dataTxAraay[2] = FirmwareUpdateSdkEraseFlash(g_dataPack.pDataBuffer);
                    }
                    else
                    {
                        TBOX_PRINT("Same cmd counter\r\n");
                        dataTxAraay[2] = 0x00; 
                    }
                    
                    dataTxLen = 3U;
                    break;

                case E_FirmwareUpdateSdkCmd_DownloadMcuMemory:
                    if (newCmdCounter != lastCmdCounter)
                    {
                        dataTxAraay[2] = FirmwareUpdateSdkLoadCode(g_dataPack.pDataBuffer, g_dataPack.dataLength);
                    }
                    else
                    {
                        TBOX_PRINT("Same cmd counter\r\n");
                        dataTxAraay[2] = 0x00; 
                    }         
                    dataTxLen = 3U;
                    break;
                        
                case E_FirmwareUpdateSdkCmd_VirifyMcuMemory:
                    dataTxAraay[2] = FirmwareUpdateSdkCodeCheck(g_dataPack.pDataBuffer);
                    dataTxLen = 3U;
                    break;
                        
                case E_FirmwareUpdateSdkCmd_SoftwareResetMcu:
                    TBOX_PRINT("---08 Reset---\r\n");
                    dataTxAraay[2] = 0x00; 
                    dataTxLen = 3U;
                    break;

                case E_FirmwareUpdateSdkCmd_GetMcuBankId:
                    TBOX_PRINT("---09 Get Bank Id ");
                    dataTxAraay[2] = 0x00; 
                    dataTxAraay[3] = (uint8_t)Flash_Reg_GetSwapStatus();
                    dataTxLen = 4U;
                    TBOX_PRINT("is %d---\r\n", dataTxAraay[3]);
                    break;
                    
                default:
                    dataTxAraay[2] = 0xFF; dataTxLen = 3U;
                    break;
                }
      
                MpuHalDataPack_t TxPack = {0};
                TxPack.aid = curAid;
                TxPack.mid =(uint8_t)s_UpdateMid;
                TxPack.subcommand = g_dataPack.subcommand;
                TxPack.pDataBuffer = dataTxAraay;
                TxPack.dataBufferSize = sizeof(dataTxAraay);
                TxPack.dataLength = dataTxLen;
                MpuHal_Transmit(g_mpuHandle, &TxPack);
                lastCmdCounter = newCmdCounter;



                if (s_UpdateMid == E_FirmwareUpdateSdkCmd_SoftwareResetMcu)
                {
                    Delay_ms(500);
                    PeripheralHalMcuHardReset();
                    PeripheralHalMcuHardReset();
                    PeripheralHalMcuHardReset();
                    TBOX_PRINT("---Reset ERROR---\r\n");
                }
                else
                {
                    memcpy(g_lastTxDataBuffer, dataTxAraay, dataTxLen);
                    g_lastTxPack.aid = TxPack.aid;
                    g_lastTxPack.mid = TxPack.mid;
                    g_lastTxPack.subcommand = TxPack.subcommand;
                    g_lastTxPack.pDataBuffer = g_lastTxDataBuffer;
                    g_lastTxPack.dataBufferSize = sizeof(g_lastTxDataBuffer);
                    g_lastTxPack.dataLength = TxPack.dataLength;
                    g_isWaitingForNextFrame = 2; // 开启等待
                    g_ackWaitTimeoutCnt = 0;     // 清零计时器
                }
        }
    }
    else
    {
        if (g_isWaitingForNextFrame > 0)
        {
            g_ackWaitTimeoutCnt++;
            if (g_ackWaitTimeoutCnt >= 500) // 500ms重发ack
            {
                /* 如果500ms后依然没收到，重发2次 */
                g_ackWaitTimeoutCnt = 0;
                g_isWaitingForNextFrame--;
                // MpuHal_Transmit(g_mpuHandle, &g_lastTxPack); // 重新发送缓存的上一帧，由于重发需要 MPU 同步修改，当前不做重发
                TBOX_PRINT("Timeout 500ms Retransmit ACK mid:%x\r\n", g_lastTxPack.mid);
            }
        }
    }
}

