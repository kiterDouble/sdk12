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
   uint8_t mcu_cmd1;
   uint8_t mcu_cmd2;
   uint8_t mcu_cmd3;
   uint8_t mcu_cmd4;
   uint8_t mcu_cmd5;
   uint8_t mcu_cmd6;
   uint8_t mcu_cmd7;
   uint8_t mcu_cmd8;
   uint8_t mcu_cmd9;
   uint8_t mcu_cmd10;

}MCUCMDSTATUS;

extern MCUCMDSTATUS mcu_cmd_status;
void CANIdDataFree(can_queue *data);
void CANIdCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata);

void HandlePmSleepCmd(const uint8_t *data, const uint16_t len, void *userdata);
int GetPmState();
int GetKl15Status();
void HardLineSyncCallBack(const uint8_t* data, const uint16_t len, void* userdata);
void get_cmd_num(const uint8_t* data, const uint16_t len, void* userdata);
int GetMqttControlEnable(void);
#endif