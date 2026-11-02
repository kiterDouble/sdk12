/*************************************************
Copyright ? 2024 SiRun (Beijing) . All rights reserved.
  File Name:       canGateWay.h
  Author:          lei.wang
  Created Time:    2024
  Description:     
   Others:      // 其它说明
*************************************************/
#ifndef _CAN_GATE_WAY_H_
#define _CAN_GATE_WAY_H_

#include "stdint.h"

void CanGateWayCan1Rx(uint32_t canId,uint8_t *data,uint8_t dlc);
void CanGateWayCan2Rx(uint32_t canId,uint8_t *data,uint8_t dlc);
void CanGateWayCan3Rx(uint32_t canId,uint8_t *data,uint8_t dlc);
void CanGateWayCan4Rx(uint32_t canId,uint8_t *data,uint8_t dlc);
void CanGateWayCan5Rx(uint32_t canId,uint8_t *data,uint8_t dlc);
void CanGateWayCan6Rx(uint32_t canId,uint8_t *data,uint8_t dlc);

void CanGateWayCan1Tx(uint32_t canId,uint8_t *data,uint8_t dlc);
void CanGateWayCan2Tx(uint32_t canId,uint8_t *data,uint8_t dlc);
void CanGateWayCan3Tx(uint32_t canId,uint8_t *data,uint8_t dlc);
void CanGateWayCan4Tx(uint32_t canId,uint8_t *data,uint8_t dlc);
void CanGateWayCan5Tx(uint32_t canId,uint8_t *data,uint8_t dlc);
void CanGateWayCan6Tx(uint32_t canId,uint8_t *data,uint8_t dlc);


//int16_t Can1RxFilterCheckCanMessage(uint32_t canId,uint8_t *data,uint8_t dlc);
//int16_t Can2RxFilterCheckCanMessage(uint32_t canId,uint8_t *data,uint8_t dlc);
//int16_t Can3RxFilterCheckCanMessage(uint32_t canId,uint8_t *data,uint8_t dlc);
//int16_t Can4RxFilterCheckCanMessage(uint32_t canId,uint8_t *data,uint8_t dlc);
//int16_t Can5RxFilterCheckCanMessage(uint32_t canId,uint8_t *data,uint8_t dlc);
//int16_t Can6RxFilterCheckCanMessage(uint32_t canId,uint8_t *data,uint8_t dlc);

int16_t CanGateWaySetCanReportControl(uint8_t value);
uint8_t CanGateWayGetCanReportControl(void);
void CanGateWayCycleProcess(uint32_t time);







#endif
