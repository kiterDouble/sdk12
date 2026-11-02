/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name:       cryptoHal.h
 Author:          lei.wang
 Created Time:    2024
 Description:     AES （ecb） encryption and decryption
 Others:          none
*************************************************/
#ifndef __CRYPTO_HAL_H__
#define __CRYPTO_HAL_H__

/****************************** include ***************************************/
#include <stdint.h>

/****************************** Macro Definitions ******************************/
#define CRYPTO_HAL_STATUS_OK     0       //成功
#define CRYPTO_HAL_STATUS_ERR    -1      //失败

/****************************** Type Definitions ******************************/
typedef struct
{
    uint32_t Ek[44];
    uint32_t Dk[44];
}CryptoHalAes_t;

/****************************** Global Variables ******************************/
/****************************** Function Declarations *************************/
/*************************************************
  Function:       CryptoHalInit
  Description:    HSM接口模块初始化
  Input:          无
  Output:         无
  Return:         无
  Others:         
*************************************************/
void CryptoHalInit(void);

/*************************************************
  Function:       CryptoHalAesEcbInit
  Description:    Aes ecb 计算初始化
  Input:          pAesData ：Aes计算对象
  Output:         无
  Return:         成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:         
*************************************************/
void CryptoHalAesEcbInit(CryptoHalAes_t *pAesData);

/*************************************************
  Function:       CryptoHalAesEcbEncrypt
  Description:    Aes ecb 加密
  Input:          pKey ：Aes秘钥
                  lengthType：加密长度
                  pSrc：要加密的数据
                  srcSize:数据长度
  Output:         pDest：结果目标块地址
  Return:         成功：HSM_HAL_STATUS_OK
                  失败：HSM_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t CryptoHalAesEcbEncrypt(const uint8_t *pKey,uint8_t lengthType,const uint8_t *pSrc,uint16_t srcSize,uint8_t *pDest);

/*************************************************
  Function:       CryptoHalAesEcbDecrypt
  Description:    Aes ecb解密
  Input:          pKey ：Aes秘钥
                  lengthType：加密长度
                  pSrc：要解密的数据
                  srcSize:数据长度
  Output:         pDest：解密结果数据
  Return:         成功：HSM_HAL_STATUS_OK
                  失败：HSM_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t CryptoHalAesEcbDecrypt(const uint8_t *pKey,uint8_t lengthType,const uint8_t *pSrc,uint16_t srcSize,uint8_t *pDest);

/*************************************************
  Function:       CryptoHalAesCmacInit
  Description:    Cmac数据初始化
  Input:          pAesData：Aes计算对象
  Output:         执行结果
  Return:         无
  Others:         
*************************************************/
void CryptoHalAesCmacInit(CryptoHalAes_t *pAesData);

/*************************************************
  Function:       CryptoHalAesCmacEncrypt
  Description:    计算cmac值
  Input:          pKey ：Aes秘钥
                  lengthType：加密长度
                  pSrc：要加密的数据
                  srcSize：要加密数据长度
  Output:         pDest：加密结果数据
  Return:         成功：HSM_HAL_STATUS_OK
                  失败：HSM_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t CryptoHalAesCmacEncrypt(const uint8_t *pKey,uint8_t lengthType,uint8_t *pSrc,uint32_t srcSize,uint8_t *pDest);

/*************************************************
  Function:       CryptoHalAesGetRandomData
  Description:    获取随机数
  Input:          无
  Output:         pRandomData ：生成的随机数
  Return:         无
  Others:         
*************************************************/
void CryptoHalAesGetRandomData(uint32_t *pRandomData);


void CryptoHalTestMain(void);

#endif  /* __CRYPTO_HAL_H__ */