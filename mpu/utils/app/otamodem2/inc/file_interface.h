#ifndef __FILE_INTERFACE_H__
#define __FILE_INTERFACE_H__


#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
int WriteFile(FILE *file ,unsigned char *data,size_t datalen);
int ReadSpecificLine(const char *filename, int lineNumber, char *lineContent, size_t maxLen);
void RemoveUpgradeFile();
int SetsloveModemUpgradeStatus(int status);
#ifdef __cplusplus
}
#endif
#endif


