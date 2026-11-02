/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name:       crashDump.h
 Author:          
 Created Time:    
 Description:     MCU crash dump recorder - saves fault info to Flash
 Others:          
 *************************************************/
#ifndef _CRASH_DUMP_H_
#define _CRASH_DUMP_H_

/****************************** include ***************************************/
#include <stdint.h>
#include <stdbool.h>

/****************************** Macro Definitions *****************************/
#define CRASH_DUMP_MAGIC_NUMBER         0x43415348  /* "CRSH" in ASCII */
#define CRASH_DUMP_VERSION              0x0001

/* Storage location - using reserved work flash area */
#define CRASH_DUMP_FLASH_ADDR           0x0001C000  /* WORKFLASH_ADDRESS_RESERVED4 */
#define CRASH_DUMP_FLASH_SIZE           0x00004000  /* 16KB reserved area */

/* Fault types */
#define CRASH_FAULT_TYPE_HARDFAULT      1
#define CRASH_FAULT_TYPE_MEMMANAGE      2
#define CRASH_FAULT_TYPE_BUSFAULT       3
#define CRASH_FAULT_TYPE_USAGEFAULT     4
#define CRASH_FAULT_TYPE_NMI            5
#define CRASH_FAULT_TYPE_DEFAULT_IRQ    6

/* Maximum stack trace depth */
#define CRASH_DUMP_STACK_TRACE_DEPTH    8

/****************************** Type Definitions ******************************/
#pragma pack(push, 1)
typedef struct {
    uint32_t magicNumber;                   /* Magic number for validation */
    uint16_t version;                       /* Version of crash dump structure */
    uint16_t faultType;                     /* Type of fault occurred */
    
    /* Timestamps */
    uint32_t resetCount;                    /* MCU reset count from parameter storage */
    uint32_t reserved1;                     /* Reserved for future use */
    
    /* Cortex-M4 Fault Registers */
    uint32_t hfsr;                          /* Hard Fault Status Register */
    uint32_t cfsr;                          /* Configurable Fault Status Register */
    uint32_t mmfsr;                         /* MemManage Fault Status (CFSR[7:0]) */
    uint32_t bfsr;                          /* Bus Fault Status (CFSR[15:8]) */
    uint32_t ufsr;                          /* Usage Fault Status (CFSR[31:16]) */
    uint32_t dfsr;                          /* Debug Fault Status Register */
    uint32_t afsr;                          /* Auxiliary Fault Status Register */
    uint32_t bfar;                          /* Bus Fault Address Register */
    uint32_t mmfar;                         /* MemManage Fault Address Register */
    
    /* Stack Context */
    uint32_t msp;                           /* Main Stack Pointer */
    uint32_t psp;                           /* Process Stack Pointer */
    uint32_t pc;                            /* Program Counter */
    uint32_t lr;                            /* Link Register */
    uint32_t xpsr;                          /* Program Status Register */
    uint32_t r0_r3[4];                      /* Registers R0-R3 */
    uint32_t r12;                           /* Register R12 */
    
    /* Stack Trace */
    uint32_t stackTrace[CRASH_DUMP_STACK_TRACE_DEPTH];  /* Call stack backtrace */
    uint32_t stackTraceDepth;               /* Actual depth of stack trace */
    
    /* FreeRTOS Task Info */
    char taskName[16];                      /* Current task name */
    uint32_t taskStackSize;                 /* Task stack size */
    uint32_t taskStackHighWaterMark;        /* Minimum remaining stack space */
    
    /* Additional Info */
    uint32_t faultAddress;                  /* Faulting address (if applicable) */
    uint32_t faultInstruction;              /* Faulting instruction (PC) */
    
    /* CRC Checksum */
    uint32_t crc32;                         /* CRC32 checksum of entire structure */
} CrashDump_t;
#pragma pack(pop)

/****************************** Function Declarations *************************/

/*************************************************
  Function:       CrashDumpInit
  Description:    Initialize crash dump module
  Input:          None
  Output:         None
  Return:         0 on success, negative on error
  Others:         
 *************************************************/
int16_t CrashDumpInit(void);

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
int16_t CrashDumpSave(uint16_t faultType, void *pFaultRegs, uint32_t *pStackFrame, uint8_t usePSP);

/*************************************************
  Function:       CrashDumpRead
  Description:    Read crash dump from Flash
  Input:          pDump - pointer to structure to fill
  Output:         pDump filled with crash dump data
  Return:         0 on success, negative on error
  Others:         
 *************************************************/
int16_t CrashDumpRead(CrashDump_t *pDump);

/*************************************************
  Function:       CrashDumpClear
  Description:    Clear crash dump from Flash (invalidate magic number)
  Input:          None
  Output:         None
  Return:         0 on success, negative on error
  Others:         
 *************************************************/
int16_t CrashDumpClear(void);

/*************************************************
  Function:       CrashDumpIsValid
  Description:    Check if a valid crash dump exists in Flash
  Input:          None
  Output:         None
  Return:         true if valid crash dump exists, false otherwise
  Others:         
 *************************************************/
bool CrashDumpIsValid(void);

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
int16_t CrashDumpFormatLog(const CrashDump_t *pDump, char *pBuffer, uint16_t bufferSize);

/*************************************************
  Function:       CrashDumpUploadToMpu
  Description:    Upload crash dump to MPU for logging
  Input:          pDump - pointer to crash dump structure
  Output:         None
  Return:         0 on success, negative on error
  Others:         Uses LogHalUpLoadLog to send to MPU
 *************************************************/
int16_t CrashDumpUploadToMpu(const CrashDump_t *pDump);

/*************************************************
  Function:       CrashDumpCheckAndReport
  Description:    Check for existing crash dump on startup and report to MPU
  Input:          None
  Output:         None
  Return:         0 if no crash or reported successfully, negative on error
  Others:         Should be called early in system startup
 *************************************************/
int16_t CrashDumpCheckAndReport(void);

/*************************************************
  Function:       CrashDumpCalculateCrc32
  Description:    Calculate CRC32 checksum of crash dump
  Input:          pDump - pointer to crash dump structure
                  size - size to calculate (usually sizeof(CrashDump_t) - 4)
  Output:         None
  Return:         CRC32 value
  Others:         
 *************************************************/
uint32_t CrashDumpCalculateCrc32(const void *data, uint32_t size);

#endif /* _CRASH_DUMP_H_ */
