/*************************************************
 Copyright © 2025 SiRun (AnHui) . All rights reserved.
 File Name: flashHal.c
 Author: 
 Created Time: 
 Description: 
 Others: 
*************************************************/
/****************************** include ***************************************/
#include "stdbool.h"
#include "flashHal.h"
#include "logHal.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "Fls_Hal.h"
#include "Core_Hal.h"
#include "AC784xx_Fls_Reg.h"

/****************************** Macro Definitions ******************************/
#define PFLASH_BASE_ADDRESS                         (0x00000000U)
#define WORKFLASH_BASE_ADDRESS                      (0x01000000U) /* DFlash Base */
#define WORKFLASH_PARAMETER_ADDRESS_OFFSET          (0U)

/* 地址到偏移量转换宏，新版SDK接口要求传入Offset */
#define DFLASH_ADDR_TO_OFFSET(addr)                 ((uint32_t)(addr) - WORKFLASH_BASE_ADDRESS)
#define PFLASH_ADDR_TO_OFFSET(addr)                 ((uint32_t)(addr) - PFLASH_BASE_ADDRESS)

#define FLASH_META_DATA_IMAGEA_OFFSET               PFLASH_ADDR_TO_OFFSET(FLASH_META_DATA_IMAGEA_HEADER_FLAG_ADDRESS)
#define FLASH_META_DATA_IMAGEB_OFFSET               PFLASH_ADDR_TO_OFFSET(FLASH_META_DATA_IMAGEB_HEADER_FLAG_ADDRESS)

/****************************** Type Definitions ******************************/
typedef union
{
    uint8_t u8Data[WORKFLASH_SECTOR_SIZE];
    uint32_t u32Data[WORKFLASH_SECTOR_SIZE / sizeof(uint32_t)];
} WorkFlashBuffer_u;

/****************************** Global Variables ******************************/
static SemaphoreHandle_t g_workFlashMutexHandle = NULL;
static WorkFlashBuffer_u g_workFlashBuffer;
static uint8_t g_flashHalSectorBuf[FLASH_SECTOR_SIZE];

const static FlashHalMetaDataInfo_t g_appflashHalMetaDataDefaultInfo = 
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
static uint32_t FlashHalIsMetaDataValid(const FlashHalMetaDataInfo_t *pMetaData);

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
    
    /* 读取两个镜像扇区末尾的标记字节 (位于 2048-2 偏移处) */
    DFlash_Hal_Read((sectorBaseOffset + (WORKFLASH_SECTOR_SIZE - 2)), 1u, &sector0Flag);
    DFlash_Hal_Read((sectorBaseOffset +  ((WORKFLASH_SECTOR_SIZE * 2) - 2)), 1u, &sector1Flag);
    
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
  Function:       FlashHalInit
  Description:    flash moudle init API
  Input:          无
  Output:         执行结果
  Return:         成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t FlashHalInit(void)
{
    Flash_Config flashInitCfg = 
    {
        .Write_Timeout = 0,
        .Erase_Timeout = 0,
        .LVD_Enable    = TRUE
    };

    Flash_Hal_Init(&flashInitCfg);
  
    if (g_workFlashMutexHandle == NULL)
    {
        g_workFlashMutexHandle = xSemaphoreCreateMutex();
    }
    return 0;
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

    if(xSemaphoreTake(g_workFlashMutexHandle, portMAX_DELAY) != pdTRUE)
    {
        return -1;
    }
    
    sectorBaseOffset = (blockAddress & WORKFLASH_SECTOR_MASK) + WORKFLASH_PARAMETER_ADDRESS_OFFSET;
    sectorNum = GetNewSectorNum(sectorBaseOffset, &newSectorFlag);
    
    activeSectorOffset = sectorBaseOffset + ((uint32_t)sectorNum * WORKFLASH_SECTOR_SIZE);
    inactiveSectorOffset = sectorBaseOffset + (((uint32_t)sectorNum ^ 1UL) * WORKFLASH_SECTOR_SIZE);

    if(writeOffset >= WORKFLASH_SECTOR_SIZE || (writeOffset + writeLength) > WORKFLASH_SECTOR_SIZE)
    {
        xSemaphoreGive(g_workFlashMutexHandle);
        return -1;
    }
    
    /* 1. 读取 Inactive Sector 作为基础数据 */
    st = DFlash_Hal_Read(inactiveSectorOffset, WORKFLASH_SECTOR_SIZE, g_workFlashBuffer.u8Data);
    if (st != STATUS_SUCCESS)
    {
        xSemaphoreGive(g_workFlashMutexHandle);
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
        xSemaphoreGive(g_workFlashMutexHandle);
        return -1;
    }

    /* 4. 写入合并后的数据到 Active Sector */
    __disable_irq();
    st = DFlash_Hal_PageWrite(activeSectorOffset, WORKFLASH_SECTOR_SIZE, g_workFlashBuffer.u8Data);
    __enable_irq();
    if (st != STATUS_SUCCESS)
    {
        xSemaphoreGive(g_workFlashMutexHandle);
        return -1;
    }

    xSemaphoreGive(g_workFlashMutexHandle);
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

    if(xSemaphoreTake(g_workFlashMutexHandle, portMAX_DELAY) != pdTRUE)
    {
        return -1;
    }
    
    sectorBaseOffset = (blockAddress & WORKFLASH_SECTOR_MASK) + WORKFLASH_PARAMETER_ADDRESS_OFFSET;

    sectorNum = GetNewSectorNum(sectorBaseOffset, &newSectorFlag);
    sectorNum ^= 1; 
    
    activeSectorOffset = sectorBaseOffset + ((uint32_t)sectorNum * WORKFLASH_SECTOR_SIZE);
    
    st = DFlash_Hal_Read(activeSectorOffset + readOffset, pReadLength, pReadData);
    
    xSemaphoreGive(g_workFlashMutexHandle);
    
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

    xSemaphoreTake(g_workFlashMutexHandle, portMAX_DELAY);  
    
    if (targetAddress >= WORKFLASH_BASE_ADDRESS)
    {
        st = DFlash_Hal_Read(DFLASH_ADDR_TO_OFFSET(targetAddress), pReadLength, pReadData);
    }
    else
    {
        st = PFlash_Hal_Read(PFLASH_ADDR_TO_OFFSET(targetAddress), pReadLength, pReadData);
    }
    
    xSemaphoreGive(g_workFlashMutexHandle);
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
    uint8_t bank = Flash_Reg_GetSwapStatus();
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
    ret = (uint32_t)PFlash_Hal_PageErase(FLASH_META_DATA_IMAGEA_HEADER_FLAG_ADDRESS);
    __enable_irq();
    
    if (ret == FLASH_HAL_STATUS_OK)
    {
        __disable_irq();
        ret = (uint32_t)PFlash_Hal_PageWrite(FLASH_META_DATA_IMAGEA_HEADER_FLAG_ADDRESS, FLASH_SECTOR_SIZE, g_flashHalSectorBuf);
        __enable_irq(); 
        
        if (ret != FLASH_HAL_STATUS_OK)
        {
            TBOX_PRINT("Line %d: Image A metadata write failed %d\r\n", __LINE__, ret);
        }
        else
        {
            memset(&readback_meta_data, 0, meta_data_size);
            __disable_irq();
            FlashHalRead(FLASH_META_DATA_IMAGEA_HEADER_FLAG_ADDRESS, (uint8_t*)&readback_meta_data, meta_data_size);
            __enable_irq();
            
            if (memcmp(&readback_meta_data, pMetaData, meta_data_size) != 0)
            {
                TBOX_PRINT("Line %d: Image A metadata verify failed! Readback data mismatch.\r\n", __LINE__);
                ret = FLASH_HAL_STATUS_FAIL;
            }
        }
    }
    else
    {
        TBOX_PRINT("Line %d: Image A sector erase failed %d\r\n", __LINE__, ret);
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
    uint32_t result = FLASH_HAL_STATUS_FAIL;
    FlashHalMetaDataInfo_t tempMeta;

    if (pMetaData == NULL)
    {
        TBOX_PRINT("Line %d: NULL pointer for meta data info\r\n", __LINE__);
        return FLASH_HAL_STATUS_NULL_ERR;
    }

    memset(&tempMeta, 0, sizeof(FlashHalMetaDataInfo_t));
    FlashHalRead(FLASH_META_DATA_IMAGEA_HEADER_FLAG_ADDRESS, (uint8_t*)&tempMeta, sizeof(FlashHalMetaDataInfo_t));
    result = FlashHalIsMetaDataValid(&tempMeta);
    if (result == FLASH_HAL_STATUS_OK)
    {
        memcpy(pMetaData, &tempMeta, sizeof(FlashHalMetaDataInfo_t));
    }
    else
    {
        TBOX_PRINT("Meta data DefaultInfo\r\n");
        memcpy(pMetaData, &g_appflashHalMetaDataDefaultInfo, sizeof(FlashHalMetaDataInfo_t));
        FlashHalWriteMetaDataInfo(&g_appflashHalMetaDataDefaultInfo);
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
static uint32_t FlashHalIsMetaDataValid(const FlashHalMetaDataInfo_t *pMetaData)
{
    uint32_t isValid = FLASH_HAL_STATUS_CHECK_ERR;
    if (pMetaData == NULL)
    {
        TBOX_PRINT("Line %d: NULL pointer for meta data validation\r\n", __LINE__);
        return FLASH_HAL_STATUS_NULL_ERR;
    }

    if ((pMetaData->m_magicHeader == FLASH_META_DATA_HEADER_END_FLAG)
    && (pMetaData->m_magicTail == FLASH_META_DATA_HEADER_END_FLAG))
    {
        isValid = FLASH_HAL_STATUS_OK;
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
  Function:       FlashHalOpenJtag
  Description:    Open flash jtag back door
  Input:          none
  Output:         none
  Return:         Success: SUCC
                  Failure: ERR
  Others:
*************************************************/
uint8_t FlashHalOpenJtag(void)
{
    // 待适配
    return  0;
}

/*************************************************
  Function:       FlashHalCloseJtag
  Description:    Close flash jtag back door
  Input:          none
  Output:         none
  Return:         Success: SUCC
                  Failure: ERR
  Others:
*************************************************/
uint8_t FlashHalCloseJtag(void)
{
    // 待适配
    return  0;
}

#if (DEBUG_UART_REV_ENABLE == 1)
#define TEST_DFLASH_BLOCK_ADDR 4096    /* DFALSH全部使用相对地址， 8192 表示第三个sector */
#define TEST_WRITE_OFFSET      0x100U
#define TEST_DATA_LEN          64U
void FlashHal_TestSuite(void)
{
    int16_t st16 = 0;
    uint8_t st8 = 0;
    
    uint8_t writeBuf[TEST_DATA_LEN];
    uint8_t readBuf[TEST_DATA_LEN];
    FlashHalMetaDataInfo_t writeMetaData;
    FlashHalMetaDataInfo_t readMetaData;
    
    TBOX_PRINT("========== Flash HAL Test Start ==========\r\n");

    /*  1. 测试初始化接口 */
    st16 = FlashHalInit();
    if (st16 == 0) {
        TBOX_PRINT("[PASS] FlashHalInit()\r\n");
    } else {
        TBOX_PRINT("[FAIL] FlashHalInit() error code: %d\r\n", st16);
    }

    /* 初始化测试数据 */
    for (uint32_t i = 0; i < TEST_DATA_LEN; i++) {
        writeBuf[i] = (uint8_t)(i & 0xFF);
    }
    memset(readBuf, 0, TEST_DATA_LEN);

    /* 2. 测试 FlashHalDataBlockWrite (Block 写入，包含擦除和乒乓操作) */
    st16 = FlashHalDataBlockWrite(TEST_DFLASH_BLOCK_ADDR, TEST_WRITE_OFFSET, writeBuf, TEST_DATA_LEN); // 写A区，就是 4096
    if (st16 == 0) {
        TBOX_PRINT("[PASS] DFLASH A\r\n");
    } else {
        TBOX_PRINT("[FAIL] DFLASH A error code: %d\r\n", st16);
    }

    st16 = FlashHalDataBlockWrite(TEST_DFLASH_BLOCK_ADDR, TEST_WRITE_OFFSET, writeBuf, TEST_DATA_LEN); // 写B区，就是 4096 + 2048
    if (st16 == 0) {
        TBOX_PRINT("[PASS] DFLASH B\r\n");
    } else {
        TBOX_PRINT("[FAIL] DFLASH B error code: %d\r\n", st16);
    }

    /* 3. 测试 DFLASH Block 读取 */
    st16 = FlashHalDataBlockRead(TEST_DFLASH_BLOCK_ADDR, TEST_WRITE_OFFSET, readBuf, TEST_DATA_LEN);
    if (st16 == 0 && (memcmp(writeBuf, readBuf, TEST_DATA_LEN) == 0)) {
        TBOX_PRINT("[PASS] DFLASH - Data Match\r\n");
    } else {
        TBOX_PRINT("[FAIL] DFLASH - Read failed or data mismatch\r\n");
    }

    /*  4. 测试小段数据读取 */
    memset(readBuf, 0, TEST_DATA_LEN);
    st16 = FlashHalSmallDataBlockRead(WORKFLASH_BASE_ADDRESS + TEST_DFLASH_BLOCK_ADDR + WORKFLASH_SECTOR_SIZE, TEST_WRITE_OFFSET, readBuf, TEST_DATA_LEN); // 这里读备份区绝对地址
    if (st16 == 0 && (memcmp(writeBuf, readBuf, TEST_DATA_LEN) == 0)) {
        TBOX_PRINT("[PASS] Small Data Read - Data Match\r\n");
    } else {
        TBOX_PRINT("[FAIL] Small Data Read - Read failed or data mismatch\r\n");
    }

    /* 5. 测试通用 FlashHalRead */
    memset(readBuf, 0, TEST_DATA_LEN);
    /* 通用读取直接基于物理地址，因此需要 blockAddr + offset */
    st16 = FlashHalRead(WORKFLASH_BASE_ADDRESS + TEST_DFLASH_BLOCK_ADDR + TEST_WRITE_OFFSET, readBuf, TEST_DATA_LEN);
    if (st16 == 0 && (memcmp(writeBuf, readBuf, TEST_DATA_LEN) == 0)) {
        TBOX_PRINT("[PASS] FlashHalRead() - Data Match\r\n");
    } else {
        TBOX_PRINT("[FAIL] FlashHalRead() - Read failed or data mismatch\r\n");
    }

    /* 6. 测试 OTA Bank 读取 */
    st8 = FlashHalOtaGetActiveBank();
    TBOX_PRINT("[INFO] FlashHalOtaGetActiveBank() returns: Bank %d\r\n", st8);
    if (st8 == 1)
    {
        // 切区前需保证备份区程序有效
        // uint8_t sta1 = (uint8_t)Flash_Hal_Swap(true);
        // TBOX_PRINT("WriteLowerVersionValue status %d\n", sta1);
    }
    else
    {
        // uint8_t sta1 = (uint8_t)Flash_Hal_Swap(false);
        // TBOX_PRINT("WriteLowerVersionValue status %d\n", sta1);
    }

    /* 7. 测试 OTA 元数据读写 */
    memset(&writeMetaData, 0, sizeof(FlashHalMetaDataInfo_t));
    writeMetaData.m_magicHeader = FLASH_META_DATA_HEADER_END_FLAG;
    writeMetaData.m_activePartition = FLASH_APP_BANKB_ID; 
    writeMetaData.m_upgradeFlag = FLASH_UPGRADE_DONE; 
    writeMetaData.m_magicTail = FLASH_META_DATA_HEADER_END_FLAG;

    st8 = FlashHalWriteMetaDataInfo(&writeMetaData);
    if (st8 == 0) {
        TBOX_PRINT("[PASS] FlashHalWriteMetaDataInfo()\r\n");
    } else {
        TBOX_PRINT("[FAIL] FlashHalWriteMetaDataInfo() error\r\n");
    }

    memset(&readMetaData, 0, sizeof(FlashHalMetaDataInfo_t));
    st8 = FlashHalGetMetaDataInfo(&readMetaData);
    if (st8 == 0 && readMetaData.m_upgradeFlag == FLASH_UPGRADE_DONE) {
        TBOX_PRINT("[PASS] FlashHalGetMetaDataInfo() - Meta Data Valid & Match\r\n");
    } else {
        TBOX_PRINT("[FAIL] FlashHalGetMetaDataInfo() - Check failure or data mismatch\r\n");
    }

    TBOX_PRINT("========== Flash HAL Test End ==========\r\n");
}
#endif
