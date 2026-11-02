#ifndef _AES128_H
#define _AES128_H

#include <stdint.h>

/*typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned int uint32_t;
typedef int int32_t;*/


#ifdef  __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t MACkey[16];
    uint8_t k1[16];
    uint8_t k2[16];
}Aes128Cmac_t;
    
int Aes128EcbDecrypt(const uint8_t *key, uint32_t keyLen, const uint8_t *ct, uint8_t *pt, uint32_t len);
int Aes128EcbEncrypt(const uint8_t *key, uint32_t keyLen, const uint8_t *pt, uint8_t *ct, uint32_t len);


void Aes128CmacLoadMacKey(Aes128Cmac_t *pData,uint8_t *key);
void Aes128CmacGenerateMAC(Aes128Cmac_t *pData,int len, uint8_t *add, uint8_t *macvalue);
uint8_t Aes128CmacVerifyMAC(Aes128Cmac_t *pData,int len, uint8_t *add, uint8_t *macvalue1);













#ifdef  __cplusplus
}
#endif


#endif

