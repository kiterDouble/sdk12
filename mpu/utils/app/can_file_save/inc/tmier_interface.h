#ifndef __TMIER_INTERFACE_H__
#define __TMIER_INTERFACE_H__


#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

#define CANFILESAVETIME (2*24*3600)

long long  GetHeaderTime();
void DelayMS(unsigned int time) ;
void SetHeaderTime(long long ms_timestamp);
void WriteAscHeader(FILE *file,long long ms_timestamp) ;
double CalculateRelativeTime(long long start_time,long long current_time) ;
void GenerateNewFilename(long long ms_timestamp,char *buffer, size_t size) ;
long long GetCurrentTime() ;

#ifdef __cplusplus
}
#endif
#endif
