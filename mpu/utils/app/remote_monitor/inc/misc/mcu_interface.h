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

typedef struct
{
   uint8_t mcu_cmd_1;
   uint8_t mcu_cmd_2;
   uint8_t mcu_cmd_3;
   uint8_t mcu_cmd_4;
   uint8_t mcu_cmd_5;
   uint8_t mcu_cmd_6;
   uint8_t mcu_cmd_7;
   uint8_t mcu_cmd_8;
   uint8_t mcu_cmd_9;
   uint8_t mcu_cmd_10;

}MCUCMDSTATUS;

MCUCMDSTATUS mcu_cmd_status;


void CANIdDataFree(can_queue *data);
void CANIdCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);

void PmStatusCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);
int GetKl15Status();
void HardLineSyncCallBack(const uint8_t* data, const uint16_t len, void* userdata);

#endif