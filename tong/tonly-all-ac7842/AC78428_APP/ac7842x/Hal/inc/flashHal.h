/*************************************************
 Copyright © 2026 SiRun (AnHui) . All rights reserved.
 File Name:       flashHal.h
 Author:          
 Created Time:    2026年7月15日
 Description:     flash read and write , Upgrade interface
 Others:
*************************************************/
#ifndef _FLASH_HAL_H_
#define _FLASH_HAL_H_

/****************************** include ***************************************/
#include "stdint.h"

/****************************** Macro Definitions *****************************/
#define FLASH_HAL_STATUS_OK                              0   
#define FLASH_HAL_STATUS_ERR                            -1
#define FLASH_HAL_STATUS_FAIL                            1
#define FLASH_HAL_STATUS_CHECK_ERR                       2
#define FLASH_HAL_STATUS_NULL_ERR                        3

#define FLASH_SECTOR_SIZE                               0X02000
#define FLASH_PHRASE_SIZE                               (16U)

#define WORKFLASH_SECTOR_SIZE                           (2048UL) // DFLASH扇区2K
#define WORKFLASH_SECTOR_MASK                           (~0x07FFUL)

#define BLOCK_A_VERSION0_ADDRESS                        (0x02000000)
#define BLOCK_A_VERSION1_ADDRESS                        (0x02000010)
#define BLOCK_A_VERSION2_ADDRESS                        (0x02001FF0)

#define BLOCK_B_VERSION0_ADDRESS                        (0x02008000)
#define BLOCK_B_VERSION1_ADDRESS                        (0x02008010)
#define BLOCK_B_VERSION2_ADDRESS                        (0x02009FF0)

/* Region Memory Size */
#define FLASH_BOOT_TOTAL_SIZE                           0X00040000    // 256k 
#define FLASH_META_DATA_TOTAL_SIZE                      0X00010000    // 64k
#define FLASH_APP_SINGLE_SIZE                           0X000B0000    // 1024k - 320k = 704k
#define FLASH_BANK_SIZE                                 0x00100000    // 1024k
#define FLASH_ALL_END_ADDR                              0x001FFFFF
/* Region Start Address */
#define FLASH_BOOT_BASE_ADDRESS                         0X00000000
#define FLASH_BOOT_ABSTRACT_ADDRESS                     (FLASH_BOOT_BASE_ADDRESS + FLASH_BOOT_TOTAL_SIZE - FLASH_SECTOR_SIZE)
#define FLASH_META_DATA_BASE_ADDRESS                    (FLASH_BOOT_BASE_ADDRESS + FLASH_BOOT_TOTAL_SIZE)
#define FLASH_APP_BANKA_BASE_ADDRESS                    (FLASH_META_DATA_BASE_ADDRESS + FLASH_META_DATA_TOTAL_SIZE)
#define FLASH_APP_BANKA_INTEGRITY_ADDRESS               (FLASH_APP_BANKA_BASE_ADDRESS)
#define FLASH_APP_BANKA_ABSTRACT_ADDRESS                (FLASH_APP_BANKA_INTEGRITY_ADDRESS + FLASH_SECTOR_SIZE)
#define FLASH_APP_START_ADDRESS                         (FLASH_APP_BANKA_ABSTRACT_ADDRESS + FLASH_SECTOR_SIZE)

#define FLASH_BOOT_BANKB_BASE_ADDRESS                   (FLASH_BOOT_BASE_ADDRESS + FLASH_BANK_SIZE)
#define FLASH_BOOT_BANKB_ABSTRACT_ADDRESS               (FLASH_BOOT_ABSTRACT_ADDRESS + FLASH_BANK_SIZE)
#define FLASH_APP_BANKB_ABSTRACT_ADDRESS                (FLASH_APP_BANKA_ABSTRACT_ADDRESS + FLASH_BANK_SIZE)
/* Meta data detail address */
#define FLASH_META_DATA_IMAGEA_HEADER_FLAG_ADDRESS      (FLASH_META_DATA_BASE_ADDRESS)
#define FLASH_META_DATA_IMAGEB_HEADER_FLAG_ADDRESS      (FLASH_META_DATA_IMAGEA_HEADER_FLAG_ADDRESS + FLASH_BANK_SIZE)

/* Bootloader & App shared flag */
#define FLASH_BANK_APP_INTEGRITY_ENABLE                 0x55AA5AA5     // Bank App Integrity Enable
#define FLASH_BANK_APP_INTEGRITY_DISABLE                0x00000000     // Bank App Integrity Disable
#define FLASH_META_DATA_HEADER_END_FLAG                 0x5A5A5A5A     // META DATA FLAG

/* OTA升级标志定义 */
#define FLASH_UPGRADE_NONE       ((uint32_t)0x00000000) // 无升级
#define FLASH_UPGRADE_PENDING    ((uint32_t)0x11111111) // 等待升级
#define FLASH_UPGRADE_UPDATING   ((uint32_t)0x22222222) // 正在升级中
#define FLASH_UPGRADE_DONE       ((uint32_t)0x33333333) // 升级完成

#define FLASH_APP_BANKA_ID                              (1)    
#define FLASH_APP_BANKB_ID                              (2) 
#define FLASH_APP_DEFALT_BANK_ID                        (0)

/****************************** Type Definitions ******************************/
typedef struct
{
    uint32_t m_magicHeader;
    uint32_t m_currentVersion;
    uint32_t m_activePartition;
    uint32_t m_upgradeFlag;
    uint32_t m_integrityFlag;
    uint32_t m_release01;
    uint32_t m_release02;
    uint32_t m_magicTail;
} FlashHalMetaDataInfo_t;

/****************************** Function Declarations *************************/
/*************************************************
  Function:       FlashHalInit
  Description:    flash moudle init API
  Input:          无
  Output:         执行结果
  Return:         成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t FlashHalInit();

/*************************************************
  Function:       FlashHalDataBlockWrite
  Description:    flash block write . block size 2048 byte
  Input:          blockAddress ： block address
                  writeOffset ：offset address
                  pWriteData：The data to be written to flash
                  writeLength：data lenth
  Output:         执行结果
  Return:         成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t FlashHalDataBlockWrite(uint32_t blockAddress,uint32_t writeOffset,const uint8_t *pWriteData,uint32_t writeLength);

/*************************************************
  Function:       FlashHalSmallDataBlockWrite
  Description:    flash small block write
  Input:          /blockAddress ： block address
                  writeOffset ：offset address
                  pWriteData：The data to be written to flash
                  writeLength：data lenth
  Output:         执行结果
  Return:         成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:        
*************************************************/
int16_t FlashHalSmallDataBlockWrite(uint32_t blockAddress,uint32_t writeOffset,const uint8_t *pWriteData,uint32_t writeLength);

/*************************************************
  Function:       FlashHalDataBlockRead
  Description:    flash block  read
  Input:          blockAddress:  block address
                  readOffset:  offset address
                  pReadData:   The data from flash read
                  pReadLength:  read lenth
  Output:         执行结果
  Return:         成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t FlashHalDataBlockRead(uint32_t blockAddress,uint32_t readOffset,uint8_t *pReadData,uint32_t pReadLength);

/*************************************************
  Function:       FlashHalSmallDataBlockRead
  Description:    flash  small block  read
  Input:          blockAddress:  block address
                  readOffset:  offset address
                  pReadData:   The data from flash read
                  pReadLength:  read lenth
  Output:         执行结果
  Return:         成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t FlashHalSmallDataBlockRead(uint32_t blockAddress,uint32_t readOffset,uint8_t *pReadData,uint32_t pReadLength);

/*************************************************
  Function:       FlashHalWriteBootFlag
  Description:    在flash中写入boot启动标记
  Input:          flagValue：写入的标记
  Output:         
  Return:         
  Others:         
*************************************************/
void FlashHalWriteBootFlag(uint32_t flagValue);

/*************************************************
  Function:       FlashHalReadBootFlag
  Description:    读取flash中的boot启动标记
  Input:          
  Output:         flash中的boot启动标记
  Return:         
  Others:        
*************************************************/
uint32_t FlashHalReadBootFlag(void);

/*************************************************
  Function:       FlashHalOtaFlashWrite
  Description:     flash code 数据写入，用于升级
  Input:          writeAddress：写入地址
                  pData：写入数据
                  length：写入数据长度
  Output:         执行结果
  Return:         成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others: 
*************************************************/
int16_t FlashHalOtaFlashWrite(uint32_t writeAddress,const uint8_t *pData,uint32_t length);

/*************************************************
  Function:       FlashHalOtaFlashErase
  Description:    flash code区擦除
  Input:          BankFlag ：code区标记 
                  0：A分区  1：B分区
  Output:         执行结果
  Return:         成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:
*************************************************/
int16_t FlashHalOtaFlashErase(uint8_t BankFlag);

void FlashHalOtaIntegrityFlagUpdate(uint8_t flag);

/*************************************************
  Function:       FlashHalOtaSetActiveBank
  Description:    设置活动的code 分区
  Input:          BankFlag ：code 分区标记 
                  0：A分区  1：B分区
  Output:         执行结果
  Return:         成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:
*************************************************/
int16_t FlashHalOtaSetActiveBank(uint8_t BankFlag);

/*************************************************
  Function:       FlashHalOtaGetActiveBank
  Description:    获取当前活动code 分区
  Input:
  Output:         当前活动code 分区标记
  Return:         0：A分区  1：B分区
  Others: 
*************************************************/
uint8_t FlashHalOtaGetActiveBank(void);

/*************************************************
  Function:       FlashHalOtaSetBankSuccessFlag
  Description:    设置启动分区是否成功标记
  Input:          BankFlag：code分区标记 0：A分区  1：B分区
  Output:         返回成功标记
  Return:         /成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:
*************************************************/
int16_t FlashHalOtaSetBankSuccessFlag(uint8_t BankFlag,uint8_t successFlag);

void FlashHalTestMain(void);
/*************************************************
  Function:       FlashHalGetMetaDataInfo
  Description:    Reads meta data from flash and stores in global cache
  Input:          pMetaData: Pointer to store the read meta data
  Output:         pMetaData: Updated with the read meta data
  Return:         SUCC on success, ERR on failure
  Others:         Uses A -> B -> default fallback logic with validation
*************************************************/
uint32_t FlashHalGetMetaDataInfo(FlashHalMetaDataInfo_t *pMetaData);

/*************************************************
  Function:       FlashHalWriteMetaDataInfo
  Description:    Writes meta data structure to both Image A and Image B regions
  Input:          pMetaData: Pointer to meta data structure to be written
  Output:         None
  Return:         SUCC on success, ERR on failure
  Others:         Ensures meta data redundancy by writing to two separate regions
*************************************************/
uint32_t FlashHalWriteMetaDataInfo(const FlashHalMetaDataInfo_t *pMetaData);
int16_t FlashHalRead(uint32_t addr, uint8_t *data, uint32_t size);
uint8_t FlashHalOpenJtag(void);
uint8_t FlashHalCloseJtag(void);
#endif