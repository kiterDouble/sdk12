/**
 * @file      netbyte.h
 * @brief     network byte conversion
 * @author    shaobo.wang
 * @version   v1
 * @date      2023-09-12 15:43:20
 * @copyright Copyright (c) V2XLINK
 */
 
#ifndef _NETBYTE_H_
#define _NETBYTE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/**
 * @brief  convert uint16_t to network byte
 * @param  out : the network byte of uint16_t
 * @param  in : uint16_t
 * @return no return
 */
void ShortToNetbyte(uint8_t *out, uint16_t in);

/**
 * @brief  convert uint32_t to network byte
 * @param  out : the network byte of uint32_t
 * @param  in : uint32_t
 * @return no return
 */
void IntToNetbyte(uint8_t *out, uint32_t in);

/**
 * @brief  convert uint64_t to network byte
 * @param  out : the network byte of uint64_t
 * @param  in : uint64_t
 * @return no return
 */
void LonglongToNetbyte(uint8_t *out, uint64_t in);

/**
 * @brief  convert network byte to uint16_t
 * @param  out : uint16_t
 * @param  in : the network byte of uint16_t
 * @param  inLen : the length of input
 * @return no return
 */
void NetbyteToShort(uint16_t *out, uint8_t *in, uint32_t inLen);

/**
 * @brief  convert network byte to uint32_t
 * @param  out : uint32_t
 * @param  in : the network byte of uint32_t
 * @param  inLen : the length of input
 * @return no return
 */
void NetbyteToInt(uint32_t *out, uint8_t *in, uint32_t inLen);

/**
 * @brief  convert network byte to uint64_t
 * @param  out : uint64_t
 * @param  in : the network byte of uint64_t
 * @param  inLen : the length of input
 * @return no return
 */
void NetbyteToLonglong(uint64_t *out, uint8_t *in, uint32_t inLen);

#ifdef __cplusplus
}
#endif

#endif
