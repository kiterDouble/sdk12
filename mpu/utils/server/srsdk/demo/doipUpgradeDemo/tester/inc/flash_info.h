#ifndef __FLASH_INFO_H__
#define __FLASH_INFO_H__
#include <stdint.h>

#define SUPPORT_MAX_SEGMENT (20)
typedef struct
{
    uint32_t memaddr;
    uint32_t memsize;
    uint8_t *memdata;
}FlashSegmentInfo_t;
typedef struct 
{
    uint32_t segNum;
    FlashSegmentInfo_t segment[SUPPORT_MAX_SEGMENT];
    uint32_t dataLen;
    uint8_t *data;
}Flashinfo_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
int32_t GetFlashInfoFromHex(Flashinfo_t **flashinfo, const char *filepath);
int32_t FlashInfoFree(Flashinfo_t *flashinfo);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif