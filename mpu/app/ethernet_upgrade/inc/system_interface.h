#ifndef __SYSTEM_INTERFACE_H__
#define __SYSTEM_INTERFACE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

int GetSysRetValue(const char *cmd);
int WriteFile(FILE *file , char *data,size_t datalen);
double GetFileSize(const char *filename);
int ReadSpecificLine(const char *filename, int lineNumber, char *lineContent, size_t maxLen);

void HexStringToBytes(const char *hex_string, unsigned char *bytes, size_t length);
int SwtMainModemUpgradeStatus(int status);
#ifdef __cplusplus
}
#endif
#endif