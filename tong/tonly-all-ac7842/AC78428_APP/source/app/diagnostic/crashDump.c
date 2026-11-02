/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name:       crashDump.c
 Author:          
 Created Time:    
 Description:     MCU crash dump recorder implementation
 Others:          
 *************************************************/

/****************************** include ***************************************/
#include "crashDump.h"
#include "flashHal.h"
#include "logHal.h"
#include "ParameterStoreManageApp.h"
#include <string.h>
#include <stdio.h>

/****************************** Macro Definitions *****************************/
/* CRC32 polynomial: x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1 */
#define CRC32_POLYNOMIAL                0xEDB88320

/****************************** Type Definitions ******************************/

/****************************** Global Variables ******************************/
static uint32_t g_crc32Table[256];
static uint8_t g_crc32TableInitialized = 0;

/****************************** Private Function Implementations **************/

/*************************************************
  Function:       CrashDumpInitCrc32Table
  Description:    Initialize CRC32 lookup table
  Input:          None
  Output:         None
  Return:         None
  Others:         
 *************************************************/
static void CrashDumpInitCrc32Table(void)
{
    uint32_t i, j;
    uint32_t crc;
    
    if (g_crc32TableInitialized) {
        return;
    }
    
    for (i = 0; i < 256; i++) {
        crc = i;
        for (j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
            } else {
                crc = crc >> 1;
            }
        }
        g_crc32Table[i] = crc;
    }
    
    g_crc32TableInitialized = 1;
}

/****************************** Public Function Implementations ***************/

/*************************************************
  Function:       CrashDumpInit
  Description:    Initialize crash dump module
  Input:          None
  Output:         None
  Return:         0 on success, negative on error
  Others:         
 *************************************************/
int16_t CrashDumpInit(void)
{
    /* Initialize CRC32 table */
    CrashDumpInitCrc32Table();
    
    /* Ensure Flash HAL is initialized */
    if (FlashHalInit() != FLASH_HAL_STATUS_OK) {
        return -1;
    }
    
    return 0;
}

/*************************************************
  Function:       CrashDumpCalculateCrc32
  Description:    Calculate CRC32 checksum of crash dump
  Input:          data - pointer to data
                  size - size in bytes
  Output:         None
  Return:         CRC32 value
  Others:         
 *************************************************/
uint32_t CrashDumpCalculateCrc32(const void *data, uint32_t size)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t crc = 0xFFFFFFFF;
    uint32_t i;
    
    if (!g_crc32TableInitialized) {
        CrashDumpInitCrc32Table();
    }
    
    for (i = 0; i < size; i++) {
        crc = (crc >> 8) ^ g_crc32Table[(crc ^ bytes[i]) & 0xFF];
    }
    
    return crc ^ 0xFFFFFFFF;
}

/*************************************************
  Function:       CrashDumpSave
  Description:    Save crash dump to Flash (called from fault handlers)
  Input:          faultType - type of fault (CRASH_FAULT_TYPE_*)
                  pFaultRegs - pointer to fault registers structure
                  pStackFrame - pointer to exception stack frame
                  usePSP - 0=MSP, 1=PSP was active
  Output:         None
  Return:         0 on success, negative on error
  Others:         This function must be called with interrupts disabled
                  and should not use dynamic memory allocation
 *************************************************/
int16_t CrashDumpSave(uint16_t faultType, void *pFaultRegs, uint32_t *pStackFrame, uint8_t usePSP)
{
    CrashDump_t dump;
    int16_t ret;
    int i;
    uint32_t stackPointer;
    uint32_t resetCount = 0;
    uint32_t dataLength = 0;
    
    /* Clear structure */
    memset(&dump, 0, sizeof(CrashDump_t));
    
    /* Fill header */
    dump.magicNumber = CRASH_DUMP_MAGIC_NUMBER;
    dump.version = CRASH_DUMP_VERSION;
    dump.faultType = faultType;
    
    /* Try to get reset count (don't fail if this doesn't work) */
    WorkFlashVehicleInforRead(E_PARAMETER_INFO_MCU_RESET_COUNT, 
                              (uint8_t *)&resetCount, &dataLength);
    dump.resetCount = resetCount;
    
    /* Fill fault registers if provided */
    if (pFaultRegs != NULL) {
        /* pFaultRegs points to the FaultRegisters structure defined in FastIRQ.c */
        /* We need to extract the registers - assuming the same layout */
        uint32_t *pRegs = (uint32_t *)pFaultRegs;
        dump.hfsr = pRegs[0];   /* HFSR */
        dump.cfsr = pRegs[1];   /* CFSR */
        dump.mmfsr = pRegs[4];  /* MMSR */
        dump.bfsr = pRegs[3];   /* BFSR */
        dump.ufsr = pRegs[2];   /* UFSR */
        dump.dfsr = pRegs[5];   /* DFSR */
        dump.afsr = pRegs[6];   /* AFSR */
        dump.bfar = pRegs[7];   /* BFAR */
        dump.mmfar = pRegs[8];  /* MMFAR */
    }
    
    /* Fill stack information from exception stack frame */
    if (pStackFrame != NULL) {
        /* Cortex-M4 exception stack frame layout:
         * [0] = R0
         * [1] = R1
         * [2] = R2
         * [3] = R3
         * [4] = R12
         * [5] = LR
         * [6] = PC (ReturnAddress)
         * [7] = xPSR
         */
        dump.r0_r3[0] = pStackFrame[0];  /* R0 */
        dump.r0_r3[1] = pStackFrame[1];  /* R1 */
        dump.r0_r3[2] = pStackFrame[2];  /* R2 */
        dump.r0_r3[3] = pStackFrame[3];  /* R3 */
        dump.r12 = pStackFrame[4];       /* R12 */
        dump.lr = pStackFrame[5];        /* LR */
        dump.pc = pStackFrame[6];        /* PC */
        dump.xpsr = pStackFrame[7];      /* xPSR */
        dump.faultInstruction = pStackFrame[6];
        
        /* Get stack pointers */
        __asm volatile ("mrs %0, msp" : "=r" (dump.msp));
        __asm volatile ("mrs %0, psp" : "=r" (dump.psp));
        stackPointer = usePSP ? dump.psp : dump.msp;
        
        /* Perform stack backtrace */
        dump.stackTrace[0] = dump.lr;
        dump.stackTrace[1] = dump.pc;
        dump.stackTraceDepth = 2;
        
        /* Simple stack trace - walk up the stack looking for valid return addresses */
        for (i = 2; i < CRASH_DUMP_STACK_TRACE_DEPTH && dump.stackTraceDepth < CRASH_DUMP_STACK_TRACE_DEPTH; i++) {
            uint32_t addr;
            /* Move up stack (stack grows downward, so we add) */
            stackPointer += 4;
            addr = *((volatile uint32_t *)stackPointer);
            
            /* Check if it looks like a valid code address (in Flash range) */
            /* Z20K148 Flash range: 0x00000000 - 0x00200000 */
            if ((addr >= 0x00000000 && addr < 0x00200000) && (addr & 0x1) == 0) {
                dump.stackTrace[dump.stackTraceDepth] = addr;
                dump.stackTraceDepth++;
            }
        }
    }
    
    /* Get current task info if FreeRTOS is running */
    /* Note: In fault handler context, we cannot safely call FreeRTOS functions */
    /* The task name will be empty - it can be determined later from stack trace */
    strncpy(dump.taskName, "Unknown", sizeof(dump.taskName) - 1);
    
    /* Calculate CRC */
    dump.crc32 = CrashDumpCalculateCrc32(&dump, sizeof(CrashDump_t) - sizeof(uint32_t));
    
    /* Write to Flash - use small data block write */
    /* Note: In fault handler context, we assume Flash is already initialized */
    ret = FlashHalSmallDataBlockWrite(CRASH_DUMP_FLASH_ADDR, 0, 
                                      (uint8_t *)&dump, sizeof(CrashDump_t));
    
    return ret;
}

/*************************************************
  Function:       CrashDumpRead
  Description:    Read crash dump from Flash
  Input:          pDump - pointer to structure to fill
  Output:         pDump filled with crash dump data
  Return:         0 on success, negative on error
  Others:         
 *************************************************/
int16_t CrashDumpRead(CrashDump_t *pDump)
{
    int16_t ret;
    
    if (pDump == NULL) {
        return -1;
    }
    
    ret = FlashHalSmallDataBlockRead(CRASH_DUMP_FLASH_ADDR, 0,
                                     (uint8_t *)pDump, sizeof(CrashDump_t));
    if (ret != FLASH_HAL_STATUS_OK) {
        return -2;
    }
    
    return 0;
}

/*************************************************
  Function:       CrashDumpClear
  Description:    Clear crash dump from Flash
  Input:          None
  Output:         None
  Return:         0 on success, negative on error
  Others:         
 *************************************************/
int16_t CrashDumpClear(void)
{
    int16_t ret;
    uint32_t invalidMagic = 0xFFFFFFFF;
    
    /* Erase the first 16 bytes (magic number) to invalidate */
    ret = FlashHalSmallDataBlockWrite(CRASH_DUMP_FLASH_ADDR, 0,
                                      (uint8_t *)&invalidMagic, sizeof(uint32_t));
    
    return ret;
}

/*************************************************
  Function:       CrashDumpIsValid
  Description:    Check if a valid crash dump exists in Flash
  Input:          None
  Output:         None
  Return:         true if valid crash dump exists, false otherwise
  Others:         
 *************************************************/
bool CrashDumpIsValid(void)
{
    CrashDump_t dump;
    uint32_t crc;
    
    /* Read header */
    if (FlashHalSmallDataBlockRead(CRASH_DUMP_FLASH_ADDR, 0,
                                   (uint8_t *)&dump, sizeof(CrashDump_t)) != FLASH_HAL_STATUS_OK) {
        return false;
    }
    
    /* Check magic number */
    if (dump.magicNumber != CRASH_DUMP_MAGIC_NUMBER) {
        return false;
    }
    
    /* Verify version */
    if (dump.version != CRASH_DUMP_VERSION) {
        return false;
    }
    
    /* Verify CRC */
    crc = CrashDumpCalculateCrc32(&dump, sizeof(CrashDump_t) - sizeof(uint32_t));
    if (crc != dump.crc32) {
        return false;
    }
    
    return true;
}

/*************************************************
  Function:       CrashDumpFormatLog
  Description:    Format crash dump as human-readable log string
  Input:          pDump - pointer to crash dump structure
                  pBuffer - output buffer
                  bufferSize - size of output buffer
  Output:         pBuffer filled with formatted log
  Return:         Number of bytes written to buffer
  Others:         
 *************************************************/
int16_t CrashDumpFormatLog(const CrashDump_t *pDump, char *pBuffer, uint16_t bufferSize)
{
    int16_t len = 0;
    const char *faultTypeStr;
    int i;
    
    if (pDump == NULL || pBuffer == NULL || bufferSize == 0) {
        return 0;
    }
    
    /* Get fault type string */
    switch (pDump->faultType) {
        case CRASH_FAULT_TYPE_HARDFAULT:    faultTypeStr = "HardFault"; break;
        case CRASH_FAULT_TYPE_MEMMANAGE:    faultTypeStr = "MemManage"; break;
        case CRASH_FAULT_TYPE_BUSFAULT:     faultTypeStr = "BusFault"; break;
        case CRASH_FAULT_TYPE_USAGEFAULT:   faultTypeStr = "UsageFault"; break;
        case CRASH_FAULT_TYPE_NMI:          faultTypeStr = "NMI"; break;
        case CRASH_FAULT_TYPE_DEFAULT_IRQ:  faultTypeStr = "DefaultIRQ"; break;
        default:                            faultTypeStr = "Unknown"; break;
    }
    
    len += snprintf(pBuffer + len, bufferSize - len,
                    "\r\n========== MCU CRASH DUMP ==========\r\n");
    len += snprintf(pBuffer + len, bufferSize - len,
                    "Fault Type: %s\r\n", faultTypeStr);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "Reset Count: %lu\r\n", (unsigned long)pDump->resetCount);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "Task: %s\r\n", pDump->taskName);
    
    /* Fault Registers */
    len += snprintf(pBuffer + len, bufferSize - len,
                    "\r\n--- Fault Registers ---\r\n");
    len += snprintf(pBuffer + len, bufferSize - len,
                    "HFSR:  0x%08X\r\n", (unsigned int)pDump->hfsr);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "CFSR:  0x%08X\r\n", (unsigned int)pDump->cfsr);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "MMSR:  0x%02X\r\n", (unsigned int)pDump->mmfsr);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "BFSR:  0x%02X\r\n", (unsigned int)pDump->bfsr);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "UFSR:  0x%04X\r\n", (unsigned int)pDump->ufsr);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "DFSR:  0x%08X\r\n", (unsigned int)pDump->dfsr);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "AFSR:  0x%08X\r\n", (unsigned int)pDump->afsr);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "BFAR:  0x%08X\r\n", (unsigned int)pDump->bfar);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "MMFAR: 0x%08X\r\n", (unsigned int)pDump->mmfar);
    
    /* Stack Context */
    len += snprintf(pBuffer + len, bufferSize - len,
                    "\r\n--- Stack Context ---\r\n");
    len += snprintf(pBuffer + len, bufferSize - len,
                    "MSP: 0x%08X  PSP: 0x%08X\r\n", 
                    (unsigned int)pDump->msp, (unsigned int)pDump->psp);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "PC:  0x%08X  LR:  0x%08X\r\n",
                    (unsigned int)pDump->pc, (unsigned int)pDump->lr);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "xPSR: 0x%08X\r\n", (unsigned int)pDump->xpsr);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "R0: 0x%08X  R1: 0x%08X  R2: 0x%08X  R3: 0x%08X\r\n",
                    (unsigned int)pDump->r0_r3[0], (unsigned int)pDump->r0_r3[1],
                    (unsigned int)pDump->r0_r3[2], (unsigned int)pDump->r0_r3[3]);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "R12: 0x%08X\r\n", (unsigned int)pDump->r12);
    
    /* Stack Trace */
    len += snprintf(pBuffer + len, bufferSize - len,
                    "\r\n--- Stack Trace ---\r\n");
    for (i = 0; i < pDump->stackTraceDepth && i < CRASH_DUMP_STACK_TRACE_DEPTH; i++) {
        len += snprintf(pBuffer + len, bufferSize - len,
                        "[%d] 0x%08X\r\n", i, (unsigned int)pDump->stackTrace[i]);
    }
    
    /* Task Info */
    len += snprintf(pBuffer + len, bufferSize - len,
                    "\r\n--- Task Info ---\r\n");
    len += snprintf(pBuffer + len, bufferSize - len,
                    "Task: %s\r\n", pDump->taskName);
    len += snprintf(pBuffer + len, bufferSize - len,
                    "Stack Size: %lu bytes\r\n", (unsigned long)pDump->taskStackSize);
    
    len += snprintf(pBuffer + len, bufferSize - len,
                    "\r\n=====================================\r\n");
    
    return len;
}

/*************************************************
  Function:       CrashDumpUploadToMpu
  Description:    Upload crash dump to MPU for logging
  Input:          pDump - pointer to crash dump structure
  Output:         None
  Return:         0 on success, negative on error
  Others:         Uses LogHalUpLoadLog to send to MPU
 *************************************************/
int16_t CrashDumpUploadToMpu(const CrashDump_t *pDump)
{
    char logBuffer[512];
    int16_t len;
    
    if (pDump == NULL) {
        return -1;
    }
    
    /* Format the crash dump as log */
    len = CrashDumpFormatLog(pDump, logBuffer, sizeof(logBuffer));
    if (len <= 0) {
        return -2;
    }
    
    /* Send to MPU via LogHalUpLoadLog */
    /* Note: LogHalUpLoadLog handles splitting long messages */
    LogHalUpLoadLog("%s", logBuffer);
    
    return 0;
}

/*************************************************
  Function:       CrashDumpCheckAndReport
  Description:    Check for existing crash dump on startup and report to MPU
  Input:          None
  Output:         None
  Return:         0 if no crash or reported successfully, negative on error
  Others:         Should be called early in system startup
 *************************************************/
int16_t CrashDumpCheckAndReport(void)
{
    CrashDump_t dump;
    int16_t ret;
    const char *faultTypeStr;
    
    /* Check if valid crash dump exists */
    if (!CrashDumpIsValid()) {
        return 0;  /* No crash dump, nothing to do */
    }
    
    /* Read crash dump */
    ret = CrashDumpRead(&dump);
    if (ret != 0) {
        return -1;
    }
    
    /* Get fault type string */
    switch (dump.faultType) {
        case CRASH_FAULT_TYPE_HARDFAULT:    faultTypeStr = "HardFault"; break;
        case CRASH_FAULT_TYPE_MEMMANAGE:    faultTypeStr = "MemManage"; break;
        case CRASH_FAULT_TYPE_BUSFAULT:     faultTypeStr = "BusFault"; break;
        case CRASH_FAULT_TYPE_USAGEFAULT:   faultTypeStr = "UsageFault"; break;
        case CRASH_FAULT_TYPE_NMI:          faultTypeStr = "NMI"; break;
        case CRASH_FAULT_TYPE_DEFAULT_IRQ:  faultTypeStr = "DefaultIRQ"; break;
        default:                            faultTypeStr = "Unknown"; break;
    }
    
    /* Log locally first */
    TBOX_PRINT("\r\n*** PREVIOUS SYSTEM CRASH DETECTED ***\r\n");
    TBOX_PRINT("Fault Type: %s\r\n", faultTypeStr);
    TBOX_PRINT("Task: %s\r\n", dump.taskName);
    TBOX_PRINT("PC: 0x%08X  LR: 0x%08X\r\n", 
               (unsigned int)dump.pc, (unsigned int)dump.lr);
    TBOX_PRINT("Uploading crash dump to MPU...\r\n");
    
    /* Upload to MPU */
    ret = CrashDumpUploadToMpu(&dump);
    if (ret != 0) {
        TBOX_PRINT("Failed to upload crash dump to MPU\r\n");
        return -2;
    }
    
    TBOX_PRINT("Crash dump uploaded successfully\r\n");
    
    /* Clear crash dump after successful upload */
    CrashDumpClear();
    
    return 0;
}
