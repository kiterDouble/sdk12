#ifndef __ALGORITHM_H__
#define __ALGORITHM_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
uint16_t calcKey(uint16_t seed);
uint8_t CalcCRC8(uint8_t *data, uint32_t size);
uint16_t CalcCRC16(const uint8_t *data, uint32_t length);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif