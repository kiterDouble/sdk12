#ifndef  _CYCLE_COMMON_H__
#define  _CYCLE_COMMON_H__


#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <queue>
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <glib.h>
#include "sr_log.h"
#include "err_code.h"
#include "common_functions.h"
#include "sr_ipc.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define PM_MQTT_APP_NAME                "mqtt_proto"

#define IN  //input param
#define OUT //output param
#define INOUT//input & output param

unsigned char *StrToHex(unsigned char *pbDest, const unsigned char *pbSrc,int nLen);

unsigned char *HexToStr(unsigned char *pbDest, const unsigned char *pbSrc,int nLen);

int HexToStr2(unsigned char *pbDest, int destLen, const unsigned char *pbSrc,int nLen);

int GetSystemRetValue(const char *cmd);

int GetFileContent(const char *filePathName,char *content, int contentLen);

int GetFotaTmpFileContentAndEmptyFile(const char *filePath, char *content, int contentLen);

int GetMd5sumValue(char *md5SumValue, int md5SumLen, char *md5Data, int md5DataLen);

gint NumToBCDCode(char *numData, int numDataLen, char *bcdData, int bcdDataLen);

int GetBJTime(char *data, int dataLen);

/**
 * @brief Get the Pm State object
 * 
 * @retval	RET_ERR     Error
 *          SGMW_PM_STATE_INIT     INIT
 *          SGMW_PM_STATE_SLEEP    Sleep
 *          SGMW_PM_STATE_NORMAL   normal
 */
uint8_t GetPmState();

/**
 * @brief Set the Pm State object
 * 
 * @retval	RET_ERR     Error
 *          SGMW_PM_STATE_INIT     INIT
 *          SGMW_PM_STATE_SLEEP    Sleep
 *          SGMW_PM_STATE_NORMAL   normal
 */
void SetPmState(uint8_t pmState);

#ifdef __cplusplus
}
#endif
#endif		
