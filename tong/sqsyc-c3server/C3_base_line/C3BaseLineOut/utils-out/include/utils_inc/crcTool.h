/**
 * @file      crcTool.h
 * @brief     crc related functions
 * @author    shaobo.wang
 * @version   v1
 * @date      2023-09-12 15:43:20
 * @copyright Copyright (c) V2XLINK
 */
 
#ifndef _CRC_TOOL_H_
#define _CRC_TOOL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/**
 * @brief the type of crc8
 */
typedef enum
{
	Crc8Def = 1, /* default */
	Crc8Itu  /* Crc8Itu */
} Crc8TypeE;

/**
 * @brief the type of crc16
 */
typedef enum
{
	Crc16Ccitt = 1, /* default */
	Crc16Ibm,       /* Crc16Ibm */
	Crc16Modbus,    /* Crc16Modbus */
	Crc16Usb        /* Crc16Usb */
} Crc16TypeE;

/**
 * @brief the type of crc32
 */
typedef enum
{
	Crc32Def = 1, /* default */
	Crc32Mpeg2 /* Crc32Mpeg2 */
} Crc32TypeE;

/**
 * @brief  Crc8
 * @param  in : the input of crc8
 * @param  inLen : the length of input
 * @param  Crc8Type : the type of crc8
 * @return the result of crc8
 */
uint8_t Crc8(uint8_t *in, uint32_t inLen, Crc8TypeE Crc8Type);

/**
 * @brief  Crc16
 * @param  in : the input of crc16
 * @param  inLen : the length of input
 * @param  Crc16Type : the type of crc16
 * @return the result of crc16
 */
uint16_t Crc16(uint8_t *in, uint32_t inLen, Crc16TypeE Crc16Type);

/**
 * @brief  Crc32
 * @param  in : the input of crc32
 * @param  inLen : the length of input
 * @param  Crc32Type : the type of crc32
 * @return the result of crc32
 */
uint32_t Crc32(uint8_t *in, uint32_t inLen, Crc32TypeE Crc32Type);

#ifdef __cplusplus
}
#endif

#endif
