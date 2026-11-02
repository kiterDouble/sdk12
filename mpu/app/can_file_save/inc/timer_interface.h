#ifndef __TMIER_INTERFACE_H__
#define __TMIER_INTERFACE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>



/* MCU时间戳转换参数
 * timestamp 单位: 微秒(us)
 * 转换公式: timestamp = mcutime * 10us * 10 = mcutime * 100us
 * 帧间隔: 10us (每帧间隔10微秒)
 */
#define MCU_TIME_TO_US(mcutime) ((uint64_t)(mcutime) * 100ULL)   /* MCU时间戳转微秒 */
#define FRAME_INTERVAL_US       10ULL                             /* 帧间隔(微秒) */


void DelayMS(unsigned int time) ;
void SetHeaderTime(long long ms_timestamp);
void WriteAscHeader(FILE *file,long long ms_timestamp) ;
double CalculateRelativeTime(long long start_time,long long current_time) ;
void GenerateNewFilename(int64_t timestamp,char *buffer, size_t size) ;
void FormatTimestamp(uint64_t timestamp_us, char *buffer, size_t size) ;
long long GetCurrentTime() ;
uint64_t GetMonoUs();
#ifdef __cplusplus
}
#endif
#endif
