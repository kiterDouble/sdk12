#ifndef _CRC8_16_32_H
#define _CRC8_16_32_H

#include <stdint.h>

typedef struct
{
    uint32_t table[256];
}Crc32Objec_t;

uint16_t CcittFalseCrc16(uint16_t crc,const uint8_t data[], uint16_t len);
uint16_t CcittCrc16(uint16_t crcIn,const uint8_t data[], uint16_t len);

uint32_t Crc32Init(Crc32Objec_t*pObject,uint32_t poly);
uint32_t Crc32(Crc32Objec_t* pObject,uint32_t crc, void *input, int len);
uint32_t Crc32FinalResult(Crc32Objec_t* pObject,uint32_t crc);











#endif    //_CRC8_16_32_H
