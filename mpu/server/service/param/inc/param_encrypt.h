/**
 * @file param_encrypt.h
 * @author jiajiangze (jiangze.jia@sirun.net)
 * @brief 参数文件加密
 * @version 0.1
 * @date 2024-08-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef __PARAM_ENCRYPT_H__
#define __PARAM_ENCRYPT_H__
#ifdef __cplusplus
extern "C"
{
#endif
#include "stdint.h"

/**
 * @brief AES256CBC加密数据 openssl软件加密
 * 
 * @param plaintext 数据
 * @param plaintext_len 数据长度
 * @param ciphertext 加密后的数据
 * @return int32_t -1：失败 其他：加密后数据长度
 */
int32_t ParamAES256CBCEncrypt(unsigned char *plaintext, int32_t plaintext_len, unsigned char *ciphertext);

/**
 * @brief AES256CBC解密数据 openssl软件加密
 * 
 * @param ciphertext 待解密数据
 * @param ciphertext_len 待解密数据长度
 * @param plaintext 原始数据
 * @return int32_t -1：失败 其他：原始数据长度
 */
int32_t ParamAES256CBCDecrypt(unsigned char *ciphertext, int32_t ciphertext_len, unsigned char *plaintext);


#ifdef __cplusplus
}
#endif

#endif
