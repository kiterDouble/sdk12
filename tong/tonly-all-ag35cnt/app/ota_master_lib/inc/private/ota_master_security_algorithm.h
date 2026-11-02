#ifndef OTA_MASTER_SECURITY_ALGORITHM_H
#define OTA_MASTER_SECURITY_ALGORITHM_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

uint32_t UdsCalcKeyLv1(uint32_t seed, uint32_t ecuId);
uint32_t UdsCalcKeyLv2(uint32_t seed, uint32_t ecuId);
uint32_t UdsCalcKeyLv3(uint32_t seed, uint32_t ecuId);
uint32_t UdsCalcKeyVcu(uint32_t seed, uint8_t vcuType);
uint32_t UdsCalcKeyTcu(uint32_t seed);
uint32_t UdsCalcKeyTms(uint32_t seed);
uint32_t UdsCalcKeyMcu1(uint32_t seed);
uint32_t UdsCalcKeyMcu1Level1(uint32_t seed);
uint32_t UdsCalcCRC32(const uint8_t *data, uint32_t len);
uint32_t UdsCalcCRC32Vcu(const uint8_t *data, uint32_t len);
#ifdef __cplusplus
}
#endif

#endif /* OTA_MASTER_SECURITY_ALGORITHM_H */
