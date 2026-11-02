/*************************************************
 Copyright © 2026 SiRun (AnHui) . All rights reserved.
 File Name: flashHal.c
 Author: Hushangrui
 Created Time: 2026年4月25日
 Description: 
 Others: 
*************************************************/
/****************************** include ***************************************/
#include "flashHal.h"
#include "logHal.h"
#include "string.h"
#include "Fls_Hal.h"
#include "Core_Hal.h"
#include "peripheralHal.h"
#include "AC784xx_Fls_Reg.h"
/****************************** Macro Definitions ******************************/
#define PFLASH_BASE_ADDRESS                         (0x00000000U)
#define WORKFLASH_BASE_ADDRESS                      (0x01000000U) /* DFlash Base */
#define WORKFLASH_PARAMETER_ADDRESS_OFFSET          (0U)
#define WORKFLASH_SECTOR_SIZE                       (2048UL) // DFLASH扇区2K
#define WORKFLASH_SECTOR_MASK                       (0x07FFUL)   
#define WORKFLASH_SECTOR_STEP                       (0x0800UL)

/* 地址到偏移量转换宏，新版SDK接口要求传入Offset */
#define DFLASH_ADDR_TO_OFFSET(addr)                 ((uint32_t)(addr) - WORKFLASH_BASE_ADDRESS)
#define PFLASH_ADDR_TO_OFFSET(addr)                 ((uint32_t)(addr) - PFLASH_BASE_ADDRESS)

#define FLASH_META_DATA_IMAGEB_OFFSET               PFLASH_ADDR_TO_OFFSET(FLASH_META_DATA_IMAGEB_HEADER_FLAG_ADDRESS)

/****************************** Type Definitions ******************************/
typedef union
{
    uint8_t u8Data[WORKFLASH_SECTOR_SIZE];
    uint32_t u32Data[WORKFLASH_SECTOR_SIZE / sizeof(uint32_t)];
} WorkFlashBuffer_u;

/****************************** Global Variables ******************************/
static WorkFlashBuffer_u g_workFlashBuffer;
static uint8_t g_flashHalSectorBuf[FLASH_SECTOR_SIZE];

const static FlashHalMetaDataInfo_t g_flashHalMetaDataDefaultInfo = 
{
    .m_magicHeader         = FLASH_META_DATA_HEADER_END_FLAG,
    .m_currentVersion      = 0,
    .m_activePartition     = FLASH_APP_BANKA_ID,
    .m_upgradeFlag         = FLASH_UPGRADE_NONE,
    .m_integrityFlag       = FLASH_BANK_APP_INTEGRITY_ENABLE,
    .m_magicTail           = FLASH_META_DATA_HEADER_END_FLAG,
};

/****************************** Function Declarations *************************/
static uint8_t GetNewSectorNum(uint32_t sectorBaseOffset, uint8_t *newSectorFlag);
static uint8_t FlashHalIsMetaDataValid(const FlashHalMetaDataInfo_t *pMetaData);

/****************************** Internal Function Implementations ***************/
/*************************************************
  Function:       GetNewSectorNum
  Description:    Get active sector number for wear-leveling/ping-pong
  Input:          sectorBaseOffset - Base offset of the work flash block
                  newSectorFlag - Pointer to store new sector flag
  Return:         Selected active sector number (0 or 1)
*************************************************/
static uint8_t GetNewSectorNum(uint32_t sectorBaseOffset, uint8_t *newSectorFlag)
{
    uint8_t selectSectorNum;
    uint8_t sector0Flag;
    uint8_t sector1Flag;
    uint8_t newFlag;
    
    /* 读取两个镜像扇区末尾的标记字节 (位于 2 字节 偏移处) */
    DFlash_Hal_Read(sectorBaseOffset + (WORKFLASH_SECTOR_SIZE - 2), 1u, &sector0Flag);
    DFlash_Hal_Read(sectorBaseOffset + (WORKFLASH_SECTOR_SIZE - 2) + WORKFLASH_SECTOR_SIZE, 1u, &sector1Flag);
    
    if(sector0Flag == 0xFFu)
    {
        selectSectorNum = 0;
        newFlag = 0;
    }
    else if(sector1Flag == 0xFFu)
    {
        selectSectorNum = 1;
        newFlag = sector0Flag + 1u;        
    }
    else
    {
        if(sector0Flag < sector1Flag)
        {
            if(sector0Flag != 0u)
            {
                selectSectorNum = 0;
                newFlag = sector1Flag + 1u;
            }
            else
            {
              if(sector1Flag != 1u)
              {
                selectSectorNum = 1;
                newFlag = sector0Flag + 1u;                
              }
              else
              {
                selectSectorNum = 0;
                newFlag = sector1Flag + 1u;            
              }
            }
        }
        else
        {
          if(sector1Flag != 0u)
          {
                selectSectorNum = 1;
                newFlag = sector0Flag + 1u;
          }
          else
          {
            if(sector0Flag != 1u)
            {
              selectSectorNum = 0;
              newFlag = sector1Flag + 1u;
            }
            else
            {
                selectSectorNum = 1;
                newFlag = sector0Flag + 1u;       
            }
          }
        }
    }
    
    if(newFlag == 0xFFu)
    {
        newFlag = 0u;
    } 
    *newSectorFlag = newFlag;
    return selectSectorNum;
}


/*************************************************
  Function:       FlashHal_BootInit
  Description:    flash moudle init API
  Input:          无
  Output:         执行结果
  Return:         成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:         
*************************************************/
void FlashHal_BootInit(void)
{
    Flash_Config flashInitCfg = 
    {
        .Write_Timeout = 0,
        .Erase_Timeout = 0,
        .LVD_Enable    = TRUE
    };

    Flash_Hal_Init(&flashInitCfg);
}

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
int16_t FlashHalDataBlockWrite(uint32_t blockAddress, uint32_t writeOffset, const uint8_t *pWriteData, uint32_t writeLength)
{
    uint32_t sectorBaseOffset = 0U;
    uint8_t  newSectorFlag = 0U;
    uint8_t  sectorNum = 0U;
    uint32_t activeSectorOffset = 0U;
    uint32_t inactiveSectorOffset = 0U;
    Hal_StatusType st;

    if (pWriteData == NULL || writeLength == 0u || writeLength > WORKFLASH_SECTOR_SIZE)
    {
        return -1;
    }
    
    sectorBaseOffset = (blockAddress & WORKFLASH_SECTOR_MASK) + WORKFLASH_PARAMETER_ADDRESS_OFFSET;
    sectorNum = GetNewSectorNum(sectorBaseOffset, &newSectorFlag);
    
    activeSectorOffset = sectorBaseOffset + ((uint32_t)sectorNum * WORKFLASH_SECTOR_SIZE);
    inactiveSectorOffset = sectorBaseOffset + (((uint32_t)sectorNum ^ 1UL) * WORKFLASH_SECTOR_SIZE);

    if(writeOffset >= WORKFLASH_SECTOR_SIZE || (writeOffset + writeLength) > WORKFLASH_SECTOR_SIZE)
    {
        return -1;
    }
    
    /* 1. 读取 Inactive Sector 作为基础数据 */
    st = DFlash_Hal_Read(inactiveSectorOffset, WORKFLASH_SECTOR_SIZE, g_workFlashBuffer.u8Data);
    if (st != STATUS_SUCCESS)
    {
        return -1;
    }

    /* 2. 在 RAM 缓存中覆盖新数据和标记 */
    memcpy(&g_workFlashBuffer.u8Data[writeOffset], pWriteData, writeLength);
    g_workFlashBuffer.u8Data[(WORKFLASH_SECTOR_SIZE - 2)] = newSectorFlag;      

    /* 3. 擦除 Active Sector */
    __disable_irq();
    st = DFlash_Hal_PageErase(activeSectorOffset);
    __enable_irq();
    if (st != STATUS_SUCCESS)
    {
        return -1;
    }

    /* 4. 写入合并后的数据到 Active Sector */
    __disable_irq();
    st = DFlash_Hal_PageWrite(activeSectorOffset, WORKFLASH_SECTOR_SIZE, g_workFlashBuffer.u8Data);
    __enable_irq();
    if (st != STATUS_SUCCESS)
    {
        return -1;
    }

    return 0;
}

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
int16_t FlashHalDataBlockRead(uint32_t blockAddress, uint32_t readOffset, uint8_t *pReadData, uint32_t pReadLength)
{
    uint32_t sectorBaseOffset = 0U;
    uint8_t newSectorFlag;
    uint8_t sectorNum;
    uint32_t activeSectorOffset = 0U;
    Hal_StatusType st;
    
    if (pReadData == NULL || pReadLength == 0)
    {
        return -1;
    }

    sectorBaseOffset = (blockAddress & WORKFLASH_SECTOR_MASK) + WORKFLASH_PARAMETER_ADDRESS_OFFSET;
    sectorNum = GetNewSectorNum(sectorBaseOffset, &newSectorFlag);
    
    activeSectorOffset = sectorBaseOffset + ((uint32_t)sectorNum * WORKFLASH_SECTOR_SIZE);
    
    st = DFlash_Hal_Read(activeSectorOffset + readOffset, pReadLength, pReadData);
    
    return (st == STATUS_SUCCESS) ? 0 : -1;
}

/*************************************************
  Function:       FlashHalSmallDataBlockRead
  Description:    Flash small block read operation
  Input:          blockAddress - Base block address
                  readOffset - Offset within the block
                  pReadData - Buffer to store read data
                  pReadLength - Number of bytes to read
  Output:         none
  Return:         0 (FLASH_HAL_STATUS_OK) - Success
  Others:         Uses mutual exclusion to protect flash access
                  Returns immediately with success code
*************************************************/
int16_t FlashHalSmallDataBlockRead(uint32_t blockAddress, uint32_t readOffset, uint8_t *pReadData, uint32_t pReadLength)
{
    Hal_StatusType st;
    uint32_t targetAddress = blockAddress + readOffset;

    if (targetAddress >= WORKFLASH_BASE_ADDRESS)
    {
        st = DFlash_Hal_Read(DFLASH_ADDR_TO_OFFSET(targetAddress), pReadLength, pReadData);
    }
    else
    {
        st = PFlash_Hal_Read(PFLASH_ADDR_TO_OFFSET(targetAddress), pReadLength, pReadData);
    }
    
    return (st == STATUS_SUCCESS) ? 0 : -1;
}

/*************************************************
  Function:       FlashHalOtaGetActiveBank
  Description:    Get current active code partition
  Input:          none
  Output:         none
  Return:         1 - Bank A active
                  2 - Bank B active
                  0 - Invalid or undefined state
  Others:         Determined by build target definition
                  Uses __BUILD_TARGET__ macro to identify active bank
*************************************************/
uint8_t FlashHalOtaGetActiveBank(void)
{
    uint8_t bank = 0xFF;
#if __BUILD_TARGET__ == RELEASEAPP1
    bank = 1;
#elif __BUILD_TARGET__ == RELEASEAPP2  
    bank = 2;
#else
    bank = 0;
#endif
    return bank;
}

/*************************************************
  Function:       FlashHalWriteMetaDataInfo
  Description:    Writes meta data structure to both Image A and Image B regions
  Input:          pMetaData: Pointer to meta data structure to be written
  Output:         None
  Return:         SUCC on success, ERR on failure
  Others:         Ensures meta data redundancy by writing to two separate regions
*************************************************/
uint32_t FlashHalWriteMetaDataInfo(const FlashHalMetaDataInfo_t *pMetaData)
{
    Hal_StatusType st;
    uint8_t ret = 0;
    const uint32_t meta_data_size = sizeof(FlashHalMetaDataInfo_t);
    FlashHalMetaDataInfo_t readback_meta_data;

    if (pMetaData == NULL)
    {
        TBOX_PRINT("Line %d: NULL pointer for meta data write\r\n", __LINE__);
        return 1;
    }

    /* First write to Image A region */
    memset(g_flashHalSectorBuf, 0xFF, FLASH_SECTOR_SIZE); 
    memcpy(g_flashHalSectorBuf, pMetaData, meta_data_size);

    __disable_irq();
    st = PFlash_Hal_PageErase(FLASH_META_DATA_IMAGEA_HEADER_FLAG_ADDRESS);
    __enable_irq();
    
    if (st == STATUS_SUCCESS)
    {
        __disable_irq();
        st = PFlash_Hal_PageWrite(FLASH_META_DATA_IMAGEA_HEADER_FLAG_ADDRESS, FLASH_SECTOR_SIZE, g_flashHalSectorBuf);
        __enable_irq();      
        if (st != STATUS_SUCCESS)
        {
            TBOX_PRINT("Line %d: Image A metadata write failed %d\r\n", __LINE__, ret);
            ret = 1;
        }
    }
    else
    {
        TBOX_PRINT("Line %d: Image A sector erase failed %d\r\n", __LINE__, ret);
        ret = 1;
    }

    /* 回读校验 */
    if (ret == 0)
    {
        __disable_irq();
        FlashHalRead(FLASH_META_DATA_IMAGEA_HEADER_FLAG_ADDRESS, (uint8_t*)&readback_meta_data, meta_data_size);
        __enable_irq();
        
        if (memcmp(&readback_meta_data, pMetaData, meta_data_size) != 0)
        {
            TBOX_PRINT("Line %d: Image A metadata verify failed! Readback data mismatch.\r\n", __LINE__);
            ret = 1;
        }
    }
    
    return ret;
}

/*************************************************
  Function:       FlashHalWriteMetaDataInfo
  Description:    Writes meta data structure to both Image A and Image B regions
  Input:          pMetaData: Pointer to meta data structure to be written
  Output:         None
  Return:         FLASH_HAL_STATUS_OK on success, FLASH_HAL_STATUS_FAIL on failure
  Others:         Ensures meta data redundancy by writing to two separate regions
*************************************************/
uint32_t FlashHalWriteBankBMetaDataInfo(const FlashHalMetaDataInfo_t *pMetaData)
{
    Hal_StatusType st;
    uint32_t ret = FLASH_HAL_STATUS_OK;
    const uint32_t meta_data_size = sizeof(FlashHalMetaDataInfo_t);
    FlashHalMetaDataInfo_t readback_meta_data;

    if (pMetaData == NULL)
    {
        TBOX_PRINT("Line %d: NULL pointer for meta data write\r\n", __LINE__);
        return FLASH_HAL_STATUS_FAIL;
    }

    memset(g_flashHalSectorBuf, 0xFF, FLASH_SECTOR_SIZE); 
    memcpy(g_flashHalSectorBuf, pMetaData, meta_data_size);

    __disable_irq();
    st = PFlash_Hal_PageErase(FLASH_META_DATA_IMAGEB_HEADER_FLAG_ADDRESS);
    __enable_irq();
    
    if (st == FLASH_HAL_STATUS_OK)
    {
        __disable_irq();
        st = PFlash_Hal_PageWrite(FLASH_META_DATA_IMAGEB_HEADER_FLAG_ADDRESS, meta_data_size, g_flashHalSectorBuf);
        __enable_irq();  
        
        if (st != FLASH_HAL_STATUS_OK)
        {
            TBOX_PRINT("Line %d: Image B metadata write failed %d\r\n", __LINE__, ret);
        }
        else
        {
            memset(&readback_meta_data, 0, meta_data_size);
            __disable_irq();
            FlashHalRead(FLASH_META_DATA_IMAGEB_HEADER_FLAG_ADDRESS, (uint8_t*)&readback_meta_data, meta_data_size);
            __enable_irq();
            
            if (memcmp(&readback_meta_data, pMetaData, meta_data_size) != 0)
            {
                TBOX_PRINT("Line %d: Image B metadata verify failed! Readback data mismatch.\r\n", __LINE__);
                ret = FLASH_HAL_STATUS_FAIL;
            }
        }
    }
    else
    {
        TBOX_PRINT("Line %d: Image B sector erase failed %d\r\n", __LINE__, ret);
    }

    return ret;
}

/*************************************************
  Function:       FlashHalGetMetaDataInfo
  Description:    Reads meta data from flash and stores in global cache
  Input:          pMetaData: Pointer to store the read meta data
  Output:         pMetaData: Updated with the read meta data
  Return:         SUCC on success, ERR on failure
  Others:         Uses A -> B -> default fallback logic with validation
*************************************************/
uint32_t FlashHalGetMetaDataInfo(FlashHalMetaDataInfo_t *pMetaData)
{
    uint8_t result = FLASH_HAL_STATUS_FAIL;
    FlashHalMetaDataInfo_t tempMeta;

    if (pMetaData == NULL)
    {
        TBOX_PRINT("Line %d: NULL pointer for meta data info\r\n", __LINE__);
        return FLASH_HAL_STATUS_NULL_ERR;
    }

    memset(&tempMeta, 0, sizeof(FlashHalMetaDataInfo_t));
    PFlash_Hal_Read(FLASH_META_DATA_IMAGEA_HEADER_FLAG_ADDRESS, sizeof(FlashHalMetaDataInfo_t), (uint8_t*)&tempMeta);
    result = FlashHalIsMetaDataValid(&tempMeta);
    
    if (result == FLASH_HAL_STATUS_OK)
    {
        memcpy(pMetaData, &tempMeta, sizeof(FlashHalMetaDataInfo_t));
    }
    else
    {
        TBOX_PRINT("Meta data DefaultInfo\r\n");
        memcpy(pMetaData, &g_flashHalMetaDataDefaultInfo, sizeof(FlashHalMetaDataInfo_t));
        FlashHalWriteMetaDataInfo(&g_flashHalMetaDataDefaultInfo);
        result = FLASH_HAL_STATUS_OK;   
    }   
    
    return result;
}


/*************************************************
  Function:       FlashHalGetMetaDataInfo
  Description:    Reads meta data from flash and stores in global cache
  Input:          pMetaData: Pointer to store the read meta data
  Output:         pMetaData: Updated with the read meta data
  Return:         FLASH_HAL_STATUS_OK on success, FLASH_HAL_STATUS_FAIL on failure
  Others:         Uses A -> B -> default fallback logic with validation
*************************************************/
uint32_t FlashHalGetBankBMetaDataInfo(FlashHalMetaDataInfo_t *pMetaData)
{
    uint32_t result = FLASH_HAL_STATUS_FAIL;
    FlashHalMetaDataInfo_t tempMeta;

    if (pMetaData == NULL)
    {
        TBOX_PRINT("Line %d: NULL pointer for meta data info\r\n", __LINE__);
        return FLASH_HAL_STATUS_NULL_ERR;
    }

    memset(&tempMeta, 0, sizeof(FlashHalMetaDataInfo_t));
    FlashHalRead(FLASH_META_DATA_IMAGEB_HEADER_FLAG_ADDRESS, (uint8_t*)&tempMeta, sizeof(FlashHalMetaDataInfo_t));
    result = FlashHalIsMetaDataValid(&tempMeta);
    if (result == FLASH_HAL_STATUS_OK)
    {
        memcpy(pMetaData, &tempMeta, sizeof(FlashHalMetaDataInfo_t));
    }
    else
    {
        TBOX_PRINT("Meta data DefaultInfo\r\n");
        memcpy(pMetaData, &g_flashHalMetaDataDefaultInfo, sizeof(FlashHalMetaDataInfo_t));
        FlashHalWriteMetaDataInfo(&g_flashHalMetaDataDefaultInfo);
        result = FLASH_HAL_STATUS_OK;
    }

    return result;
}

/*************************************************
  Function:       FlashHalIsMetaDataValid
  Description:    Validates meta data structure using header and end flags
  Input:          pMetaData: Pointer to meta data structure to validate
  Output:         None
  Return:         SUCC if meta data is valid, ERR otherwise
  Others:         Checks header and end flags to determine validity
*************************************************/
static uint8_t FlashHalIsMetaDataValid(const FlashHalMetaDataInfo_t *pMetaData)
{
    uint32_t isValid = 1;
    if (pMetaData == NULL)
    {
        TBOX_PRINT("Line %d: NULL pointer for meta data validation\r\n", __LINE__);
        return 2;
    }

    if ((pMetaData->m_magicHeader == FLASH_META_DATA_HEADER_END_FLAG)
    && (pMetaData->m_magicTail == FLASH_META_DATA_HEADER_END_FLAG))
    {
        isValid = 0;
    }
    else
    {
        TBOX_PRINT("Line %d: Invalid meta data header or end flag\r\n", __LINE__);
    }

    return isValid;
}

/*************************************************
  Function:       FlashHalRead
  Description:    Flash read
  Input:          addr: Read address
                  data: Buffer to store read data
                  size: Data size to read
  Output:         Execution result
  Return:         Success: FLASH_HAL_STATUS_OK
                  Failure: FLASH_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t FlashHalRead(uint32_t addr, uint8_t *data, uint32_t size)
{
    Hal_StatusType st;
    if (addr >= WORKFLASH_BASE_ADDRESS)
    {
        st = DFlash_Hal_Read(DFLASH_ADDR_TO_OFFSET(addr), size, data);
    }
    else
    {
        st = PFlash_Hal_Read(PFLASH_ADDR_TO_OFFSET(addr), size, data);
    }
    return (st == STATUS_SUCCESS) ? 0 : -1;
}


/*************************************************
  Function:       FlashHalOtaFlashErase
  Description:    Flash code area erase
  Input:          None
  Output:         None
  Return:         Success: COMMON_SUCC
                  Failure: COMMON_ERR
  Others:         Erase entire region according to specified bank flag
*************************************************/
int16_t FlashHalOtaFlashErase(void)
{
    uint8_t ret = FLASH_HAL_STATUS_OK;
    uint32_t eraseAddress;
    uint32_t eraseLength;
    uint32_t currentAddress = 0u;
    uint32_t sectorCount;
    uint32_t i;
    int16_t result = FLASH_HAL_STATUS_OK;
    
    eraseAddress = FLASH_APP_BANKA_ABSTRACT_ADDRESS + FLASH_BANK_SIZE; // 擦除备份区
    eraseLength = FLASH_APP_SINGLE_SIZE - FLASH_SECTOR_SIZE;
    sectorCount = eraseLength / FLASH_SECTOR_SIZE;
    
    for (i = 0u; (i < sectorCount) && (result == FLASH_HAL_STATUS_OK); i++)
    {
        currentAddress = eraseAddress + (i * FLASH_SECTOR_SIZE);  

        __disable_irq();
        ret = (uint8_t)PFlash_Hal_PageErase(currentAddress);
        __enable_irq();
        PeripheralHalFeedWatchDog();
        
        if (ret != 0)
        {
            TBOX_PRINT("Line %d: Erase sector failed at address 0x%08X, error code %d\r\n", 
                        __LINE__, currentAddress, ret);
            result = FLASH_HAL_STATUS_ERR; 
        }
    }  
    return result;
}


/*************************************************
  Function:       FlashHalFlexWrite
  Description:    Write data to Flash, supports non-16-byte aligned addresses and arbitrary data lengths
  Input:          address: Write address
                  length: Data length to write
                  pWriteData: Pointer to data to be written
  Output:         None
  Return:         Success: COMMON_SUCC
                  Failure: COMMON_ERR
  Others:         This function can only write data within a single sector, cross-sector writing is not supported
*************************************************/
uint32_t FlashHalFlexWrite(uint32_t address, uint32_t length, uint8_t *pWriteData)
{
    uint8_t ret = 0;
    uint32_t sector_start = address & ~(FLASH_SECTOR_SIZE - 1); // Sector start address (8192-byte aligned)
    uint32_t sector_end = sector_start + FLASH_SECTOR_SIZE - 1;  // Sector end address
    uint32_t offset_in_sector = address - sector_start; // Offset of new data within the sector

    if ((address < FLASH_META_DATA_BASE_ADDRESS) || 
        (address + length > FLASH_ALL_END_ADDR) || 
        (pWriteData == NULL) || 
        (length == 0))
    {
        ret = 1;
        TBOX_PRINT("Line %d: flash write check error (invalid param) %d\r\n", __LINE__, ret);
    }
    else if (address + length - 1 > sector_end)
    {
        ret = 1;
        TBOX_PRINT("Line %d: write cross sector (not supported) %d\r\n", __LINE__, ret);
    }
    else
    {
        __disable_irq();
        FlashHalRead(sector_start, g_flashHalSectorBuf, FLASH_SECTOR_SIZE);
        __enable_irq();
        PeripheralHalFeedWatchDog();
        memcpy(&g_flashHalSectorBuf[offset_in_sector], pWriteData, length);
        __disable_irq();
        ret = (uint8_t)PFlash_Hal_PageErase(sector_start);
        __enable_irq();
        PeripheralHalFeedWatchDog(); 
        if (ret != 0)
        {
            TBOX_PRINT("Line %d: erase sector error %d\r\n", __LINE__, ret);
            __enable_irq();   
        }
        else
        {
            __disable_irq();
            ret = (uint8_t)PFlash_Hal_PageWrite(sector_start, FLASH_SECTOR_SIZE, g_flashHalSectorBuf);
            __enable_irq();
            PeripheralHalFeedWatchDog();
            if (ret != 0)
            {
                TBOX_PRINT("Line %d: program sector error %d\r\n", __LINE__, ret);
            }
        }    
    }
    return ret; 
}

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
int16_t FlashHalOtaFlashWrite(uint32_t writeAddress,volatile uint8_t *pData,uint32_t length)
{
    int16_t result = FLASH_HAL_STATUS_OK;
    uint8_t writeResult = 0u;
    uint8_t* writeData = (uint8_t*)pData;
    if (pData == NULL || length == 0)
    {
        TBOX_PRINT("Line %d: Invalid data pointer or zero length\r\n", __LINE__);
        result = FLASH_HAL_STATUS_ERR;
    }
    else
    {
        writeResult = FlashHalFlexWrite(writeAddress, length, writeData);
        if (writeResult != 0)
        {
            TBOX_PRINT("Line %d: FlashHalFlexWrite failed for packet data, result: %d\r\n", __LINE__, writeResult);
            result = FLASH_HAL_STATUS_ERR;
        }
    }  
    return result;
}


/*************************************************
  Function:       FlashHal_CopyBoot
  Description:    FlashHal_CopyBoot
  Input:          None
  Output:         None
  Return:         Success: COMMON_SUCC
                  Failure: COMMON_ERR
  Others:         
*************************************************/
int16_t FlashHal_CopyBoot(void)
{
    uint8_t ret = 0;
    int16_t result = FLASH_HAL_STATUS_OK;
    
    // 计算需要操作的扇区总数
    uint32_t sectorCount = FLASH_BOOT_TOTAL_SIZE / FLASH_SECTOR_SIZE;
    uint32_t i;
    
    uint32_t currentBackupAddr = 0u;
    uint32_t currentBootAddr = 0u;

    /* 1. 擦除BOOT备份区 */
    for (i = 0u; i < sectorCount; i++)
    {
        currentBackupAddr = FLASH_BOOT_BANKB_BASE_ADDRESS + (i * FLASH_SECTOR_SIZE);

        __disable_irq();
        ret = (uint8_t)PFlash_Hal_PageErase(currentBackupAddr);
        __enable_irq();

        if (ret != 0)
        {
            TBOX_PRINT("Line %d: Erase backup boot failed at 0x%08X, err %d\r\n", 
                       __LINE__, currentBackupAddr, ret);
            return FLASH_HAL_STATUS_ERR; // 擦除失败，直接返回
        }
    }

    /* 2. 将当前 BOOT 复制到备份区 */
    for (i = 0u; i < sectorCount; i++)
    {
        currentBootAddr = 0x0 + (i * FLASH_SECTOR_SIZE);
        currentBackupAddr = FLASH_BOOT_BANKB_BASE_ADDRESS + (i * FLASH_SECTOR_SIZE);
        memcpy(g_flashHalSectorBuf, (const void*)currentBootAddr, FLASH_SECTOR_SIZE);
        __disable_irq();
        ret = (uint8_t)PFlash_Hal_PageWrite(currentBackupAddr, FLASH_SECTOR_SIZE, g_flashHalSectorBuf);
        __enable_irq();
        PeripheralHalFeedWatchDog();

        if (ret != 0)
        {
            TBOX_PRINT("Line %d: Program backup boot failed at 0x%08X, err %d\r\n", 
                       __LINE__, currentBackupAddr, ret);
            return FLASH_HAL_STATUS_ERR; // 写入失败，直接返回
        }
    }

    return result;
}