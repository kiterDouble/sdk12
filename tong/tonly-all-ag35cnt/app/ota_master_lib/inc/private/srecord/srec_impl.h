#ifndef __SREC_IMPL_H__
#define __SREC_IMPL_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include "srecord.h"

typedef struct 
{
    SRecord *records;
    int recordCount;
}SRecordSet_t;

int32_t SrecImplParserFile(const char *fileName, SRecordSet_t *srecSet);
int32_t SrecImplGetAddressRange(SRecordSet_t *srecSet, uint32_t *startAddr, uint32_t *endAddr);
int32_t SrecImplFree(SRecordSet_t *srecSet);
#ifdef __cplusplus
}
#endif
#endif