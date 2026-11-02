// #ifndef  _UDS_SECURITY_CALCULATE_H
// #define  _UDS_SECURITY_CALCULATE_H

// #include <stdint.h>

// typedef enum
// {
//     E_SECURITY_CALCULATE_LEVEL1,
//     E_SECURITY_CALCULATE_LEVEL2,
//     E_SECURITY_CALCULATE_LEVEL3,//FBL
//     E_SECURITY_CALCULATE_LEVEL4,
//     E_SECURITY_CALCULATE_LEVEL5,
//     E_SECURITY_CALCULATE_LEVEL9,
// }SecurityCalculateLevel_e;

// void UdsGetSeed(uint8_t *pSeed,uint8_t *pSeedLength);

// void UdsSeedtoKey(uint8_t *seed,uint8_t seedLen,uint8_t *key,uint8_t *pKeyLen,SecurityCalculateLevel_e securityLevel);

// #endif
#ifndef _UDS_SECURITY_CALCULATE_H
#define _UDS_SECURITY_CALCULATE_H

#include <stdint.h>

void UdsGetSeed(uint8_t *pSeed, uint8_t *pSeedLength);

int Seed2Key(const unsigned char *iSeedArray,     /* Array for the seed [in] */
             const unsigned short iSeedArraySize, /* Length of the array for the seed [in] */
             const unsigned int iSecurityLevel,   /* Security level [in] */
             const char *iVariant,                /* Name of the active variant [in] */
             unsigned char *ioKeyArray,           /* Array for the key [in, out] */
             unsigned int iKeyArraySize,          /* Maximum length of the array for the key [in] */
             unsigned int *oSize);                /* Length of the key [out] */

#endif