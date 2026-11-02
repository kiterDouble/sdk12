#ifndef  _UDS_TEST_SERVICE_H
#define  _UDS_TEST_SERVICE_H

#include <stdint.h>


void ServiceTestSetDiagnosticCan(uint8_t canChannel);
int16_t ServiceTestSecurityProcess(uint8_t *pUdsDataIn,uint16_t lenIn,uint8_t *pUdsDataOut,uint16_t *pLenOut);
int16_t ServiceTestProcess(uint8_t *pUdsDataIn,uint16_t lenIn,uint8_t *pUdsDataOut,uint16_t *pLenOut);
void ServiceTestCycleProcess(uint32_t cycleTime);

#endif