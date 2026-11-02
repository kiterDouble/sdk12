/**
 * @file      utils.h
 * @brief     related functions
 * @author    shaobo.wang
 * @version   v1
 * @date      2023-09-12 15:43:20
 * @copyright Copyright (c) V2XLINK
 */
 
#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/**
 * @brief  base64 encode
 * @param  out : the result of base64 encode
 * @param  in : the input of base64 encode
 * @param  inLen : the length of input
 * @return -1 represents failure, the others are the length of the output
 */
int32_t Base64Encode(uint8_t *out, uint8_t *in, uint32_t inLen);

/**
 * @brief  base64 decode
 * @param  out : the result of base64 decode
 * @param  in : the input of base64 decode
 * @param  inLen : the length of input
 * @return -1 represents failure, the others are the length of the output
 */
int32_t Base64Decode(uint8_t *out, uint8_t *in, uint32_t inLen);

/**
 * @brief  base64 decode
 * @param  out : the result of md5
 * @param  in : the input of md5
 * @param  inLen : the length of input
 * @return -1 represents failure, 1 represents success
 */
int32_t Md5(uint8_t *out, uint8_t *in, uint32_t inLen);

/**
 * @brief  base64 decode
 * @param  out : the result of sha256
 * @param  in : the input of sha256
 * @param  inLen : the length of input
 * @return -1 represents failure, 1 represents success
 */
int32_t Sha256(uint8_t *out, uint8_t *in, uint32_t inLen);

#ifdef __cplusplus
}
#endif

#endif
