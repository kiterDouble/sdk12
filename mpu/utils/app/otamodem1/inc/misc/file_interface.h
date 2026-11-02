#ifndef _FILE_INTERFACE_H_
#define _FILE_INTERFACE_H_

#include <stdio.h>  



#define  UPGRADERESULTS         "/mnt/emmc/upgrade/upgraderesults.txt"
#define  DELETEUPGRADERESULTS   "rm -rf  /mnt/emmc/upgrade/upgraderesults.txt"
#define  OTAUPGRADETIMESTAMP                "/oemdata/log/otatimestamp.txt"
#define  FILE_MAX_SIZE (20) // 20MB

int OpenWriteFile(const char *filepath , char *data,size_t datalen);
int OpenReadFile(const char *filepath , char *data,int datalen);
int WriteFile(FILE *file , unsigned char *data,size_t datalen);
double GetFileSize(const char *filename);
int writeIntToFile(const char *filepath, int data);
int readIntFromFile(const char *filepath, int *data);
int writeIntWithTimestampToFile(const char *filepath, int flag) ;
int ReadSpecificLine(const char *filename, int lineNumber, char *lineContent, size_t maxLen);
void * OtaFileHander(void *arg);
#endif
