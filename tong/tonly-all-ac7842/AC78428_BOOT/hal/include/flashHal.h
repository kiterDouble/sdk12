/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name:       flashHal.h
 Author:          lei.wang
 Created Time:
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
#define FLASH_WRITE_MINIMAM_SIZE                        0X00000010
#define FLASH_PHRASE_SIZE                               (16U)

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
#define FLASH_BANK_APP_INTEGRITY_ENABLE                 0x55AA5AA5     //Bank App Integrity Enable
#define FLASH_BANK_APP_INTEGRITY_DISABLE                0x00000000     //Bank App Integrity Disable
#define FLASH_META_DATA_HEADER_END_FLAG                 0x5A5A5A5A     //META DATA FLAG
#define FLASH_META_DATA_DEFAULT_VALUE                   0XFFFFFFFF     //Reserve Data 

/* OTA升级标志定义 */
#define FLASH_UPGRADE_NONE       ((uint32_t)0x00000000) // 无升级
#define FLASH_UPGRADE_PENDING    ((uint32_t)0x11111111) // 等待升级
#define FLASH_UPGRADE_UPDATING   ((uint32_t)0x22222222) // 正在升级中
#define FLASH_UPGRADE_DONE       ((uint32_t)0x33333333) // 升级完成

#define FLASH_APP_BANKA_ID                              (1)    
#define FLASH_APP_BANKB_ID                              (2) 
#define FLASH_APP_DEFALT_BANK_ID                        (0)

/****************************** Type Definitions ******************************/
typedef void (*bootloaderFun)(void);

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
  Function:       FlashHal_BootInit
  Description:    flash moudle init API
  Input:          无
  Output:         执行结果
  Return:         成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:         
*************************************************/
void FlashHal_BootInit();

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
  Function:       FlashHalOtaFlashWrite
  Description:    OTA flash write
  Input:          writeAddress: Write address
                  pData: Data to write
                  length: Data length to write
  Output:         Execution result
  Return:         Success: FLASH_HAL_STATUS_OK
                  Failure: FLASH_HAL_STATUS_ERR
  Others:         This function is called after receiving data packets, the length is not large
*************************************************/
int16_t FlashHalOtaFlashWrite(uint32_t writeAddress,volatile uint8_t *pData,uint32_t length);

/*************************************************
  Function:       FlashHalOtaFlashErase
  Description:    Flash code area erase
  Input:          None
  Output:         None
  Return:         Success: SUCC
                  Failure: ERR
  Others:         Erase entire region according to specified bank flag
*************************************************/
int16_t FlashHalOtaFlashErase(void);

/*************************************************
  Function:       FlashHalOtaSetBankSuccessFlag
  Description:    设置启动分区是否成功标记
  Input:          BankFlag：code分区标记 0：A分区  1：B分区
  Output:         返回成功标记
  Return:         /成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:
*************************************************/

/*************************************************
  Function:       FlashHalGetMetaDataInfo
  Description:    Reads meta data from flash and stores in global cache
  Input:          pMetaData: Pointer to store the read meta data
  Output:         pMetaData: Updated with the read meta data
  Return:         FLASH_HAL_STATUS_OK on success, FLASH_HAL_STATUS_FAIL on failure
  Others:         Uses A -> B -> default fallback logic with validation
*************************************************/
uint32_t FlashHalGetMetaDataInfo(FlashHalMetaDataInfo_t *pMetaData);

/*************************************************
  Function:       FlashHalWriteMetaDataInfo
  Description:    Writes meta data structure to both Image A and Image B regions
  Input:          pMetaData: Pointer to meta data structure to be written
  Output:         None
  Return:         FLASH_HAL_STATUS_OK on success, FLASH_HAL_STATUS_FAIL on failure
  Others:         Ensures meta data redundancy by writing to two separate regions
*************************************************/
uint32_t FlashHalWriteMetaDataInfo(const FlashHalMetaDataInfo_t *pMetaData);

/*************************************************
  Function:       FlashHalGetMetaDataInfo
  Description:    Reads meta data from flash and stores in global cache
  Input:          pMetaData: Pointer to store the read meta data
  Output:         pMetaData: Updated with the read meta data
  Return:         FLASH_HAL_STATUS_OK on success, FLASH_HAL_STATUS_FAIL on failure
  Others:         Uses A -> B -> default fallback logic with validation
*************************************************/
uint32_t FlashHalGetBankBMetaDataInfo(FlashHalMetaDataInfo_t *pMetaData);

/*************************************************
  Function:       FlashHalWriteMetaDataInfo
  Description:    Writes meta data structure to both Image A and Image B regions
  Input:          pMetaData: Pointer to meta data structure to be written
  Output:         None
  Return:         FLASH_HAL_STATUS_OK on success, FLASH_HAL_STATUS_FAIL on failure
  Others:         Ensures meta data redundancy by writing to two separate regions
*************************************************/
uint32_t FlashHalWriteBankBMetaDataInfo(const FlashHalMetaDataInfo_t *pMetaData);

/*************************************************
  Function:       FlashHalRead
  Description:    Flash read
  Input:          addr: Read address
                  data: Buffer to store read data
                  size: Data size to read
  Output:         Execution result
  Return:         void
  Others:         
*************************************************/
int16_t FlashHalRead(uint32_t addr, uint8_t *data, uint32_t size);

/*************************************************
  Function:       FlashHal_CopyBoot
  Description:    FlashHal_CopyBoot
  Input:          None
  Output:         None
  Return:         Success: COMMON_SUCC
                  Failure: COMMON_ERR
  Others:         
*************************************************/
int16_t FlashHal_CopyBoot(void);

#endif