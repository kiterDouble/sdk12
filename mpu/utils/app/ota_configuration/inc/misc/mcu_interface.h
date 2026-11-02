#ifndef _MCU_INTERFACE_H_
#define _MCU_INTERFACE_H_

#include "pm_server_api.h"
#include "mcu_api.h"

#define KL15_IS_ON 		(1)
#define KL15_IS_OFF 	(0)

typedef struct 
{
   int datalen;
   unsigned char *candata;
}can_queue;

void CANIdDataFree(can_queue *data);
void CANIdCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);

void HandlePmSleepCmd(const uint8_t *data, const uint16_t len, void *userdata);
int GetPmState();
int GetKl15Status();
void HardLineSyncCallBack(const uint8_t* data, const uint16_t len, void* userdata);
#endif