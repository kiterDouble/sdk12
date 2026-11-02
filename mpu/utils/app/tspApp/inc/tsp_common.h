#ifndef  _TSP_COMMON_H__
#define  _TSP_COMMON_H__

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
#include "pm_server_api.h"
#include "common_functions.h"
#include "queue_msg.h"
#include "sr_ipc.h"
#include "sr_param.h"
#include "mqtt/MQTTClient.h"

//#define  SHA256_DIGEST_LENGTH  (256)

#define PM_TSP_APP_NAME "tspApp"
#define PM_TSPHEART_APP_NAME "tspAppheart"
#define IN  //input param
#define OUT //output param
#define INOUT//input & output param

unsigned char *StrToHex(unsigned char *pbDest, const unsigned char *pbSrc,int nLen);

unsigned char *HexToStr(unsigned char *pbDest, const unsigned char *pbSrc,int nLen);

int HexToStr2(unsigned char *pbDest, int destLen, const unsigned char *pbSrc,int nLen);

int GetSystemRetValue(const char *cmd);

int GetFileContent(const char *filePathName,char *content, int contentLen);

int GetFotaTmpFileContentAndEmptyFile(const char *filePath, char *content, int contentLen);

/**
 * @brief Get the Pm State object
 * 
 * @retval	RET_ERR     Error
 *          PM_STATE_SLEEP   Sleep
 *          PM_STATE_NORMAL   normal
 */
uint8_t GetPmState();

/**
 * @brief Set the Pm State object
 * 
 * @retval	RET_ERR     Error
 *          PM_STATE_SLEEP   Sleep
 *          PM_STATE_NORMAL   normal
 */
void SetPmState(uint8_t pmState);
#endif		
