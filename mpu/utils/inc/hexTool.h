/**
 * @file      hexTool.h
 * @brief     convert string to hex
 * @author    shaobo.wang
 * @version   v1
 * @date      2023-09-12 15:43:20
 * @copyright Copyright (c) V2XLINK
 */
 
#ifndef _HEX_TOOL_H_
#define _HEX_TOOL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/**
 * @brief  convert string to hex
 * @param  out : the result of string
 * @param  in : the input of string
 * @param  inLen : the length of input
 * @return no return
 */
void StringToHex(uint8_t *out, char *in, uint32_t inLen);

/**
 * @brief  convert hex to string
 * @param  out : the result of hex
 * @param  in : the input of hex
 * @param  inLen : the length of input
 * @return no return
 */
void HexToString(char *out, uint8_t *in, uint32_t inLen);

#ifdef __cplusplus
}
#endif

#endif
