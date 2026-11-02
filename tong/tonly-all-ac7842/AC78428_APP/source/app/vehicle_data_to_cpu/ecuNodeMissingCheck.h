#ifndef _ECU_NODE_MISSING_CHECK_H
#define _ECU_NODE_MISSING_CHECK_H

#include <stdint.h>


void NodeMissingCheckInit(void);

void EcuNodeMissingCheckCanId(uint8_t canChannel,uint32_t canId);
uint8_t EcuNodeMissingStatus(uint8_t canChannel,uint32_t canId);
void NodeMissingCheckTimeOut(uint32_t cycleTime);

void ClearNodeMissingCheckConfig(void);
void NodeMissingCheckReset(void);
void DisableNodeMissingCheckByChannel(uint8_t channel);


#endif    //_ECU_NODE_MISSING_CHECK_H
