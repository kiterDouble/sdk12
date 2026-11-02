#ifndef __SYSTEM_INTERFACE_H__
#define __SYSTEM_INTERFACE_H__

#include "kfifo.h"
#include "can_file_save.h"
#include "file_interface.h"

#ifdef __cplusplus
extern "C" {
#endif


/* 模组类型定义 */
#define MAIN_MODEM  1
#define SLAVE_MODEM 2

/* 备份完成标志定义 */
#define BACKUP_NOT_FINISH   0
#define BACKUP_FINISH       1

/* 数据采集状态定义 */
#define COLLECT_STATUS_STOP     0
#define COLLECT_STATUS_RUNNING   1


typedef struct AppContext
{
    /* CAN数据FIFO缓冲区 */
    kfifo_t can_fifo;
    
    /* 时间范围信息 */
    TimeRange_t time_range;
    
    /* 文件写入上下文 */
    FileWriterCtx_t main_file_ctx;   /* 主模组文件写入上下文 */
    FileWriterCtx_t backup_file_ctx; /* 备份文件写入上下文 */
    
    /* 状态标志 */
    volatile int backup_finish_flag; /* 备份完成标志 */
    volatile int collect_status;     /* 数据采集状态 */
    /* 模组类型 */
    uint8_t modem_type;              /* MAIN_MODEM(1) 或 SLAVE_MODEM(2) */
    
} AppContext_t;


AppContext_t* GetAppContext(void);
int AppContextInit(void);
void AppContextDeinit(void);

/* backup_finish_flag 的 setter/getter 函数 */
void SetBackupFinishFlag(int value);
int GetBackupFinishFlag(void);

/* collect_status 的 setter/getter 函数 */
void SetCollectStatus(int value);
int GetCollectStatus(void);

int GetSystemRetValue(const char *cmd);
int CheckTimeSync();
int GetEmmcSize();


#ifdef __cplusplus
}
#endif
#endif