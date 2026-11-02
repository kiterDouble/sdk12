/**
 * @file      netbyte.c
 * @brief     network byte conversion
 * @author    shaobo.wang
 * @version   v1
 * @date      2023-09-12 15:43:20
 * @copyright Copyright (c) V2XLINK
 */
 
#include "netbyte.h"

/**
 * @brief 将16位整数转换为网络字节序（大端序）的字节数组。
 *
 * 该函数将16位整数转换为网络字节序（大端序）的字节数组。
 *
 * @param[out] out 指向存储结果的字节数组的指针。
 * @param[in] in 输入的16位整数。
 */
void ShortToNetbyte(uint8_t *out, uint16_t in)
{
	if(!out)
		return;
	
	int16_t bytes = 0x1234;
	if(*(uint8_t *)&bytes == 0x34)
	{
		uint32_t outLen = sizeof(uint16_t);
		for(uint32_t i = 0; i < outLen; i++)
			*(out + i) = *((uint8_t *)&in + outLen - 1 - i);
	}
}

/**
 * @brief 将32位整数转换为网络字节序（大端序）的字节数组。
 *
 * 该函数将32位整数转换为网络字节序（大端序）的字节数组。
 *
 * @param[out] out 指向存储结果的字节数组的指针。
 * @param[in] in 输入的32位整数。
 */
void IntToNetbyte(uint8_t *out, uint32_t in)
{
	if(!out)
		return;
	
	int16_t bytes = 0x1234;
	if(*(uint8_t *)&bytes == 0x34)
	{
		uint32_t outLen = sizeof(uint32_t);
		for(uint32_t i = 0; i < outLen; i++)
			*(out + i) = *((uint8_t *)&in + outLen - 1 - i);
	}
}

/**
 * @brief 将64位整数转换为网络字节序（大端序）的字节数组。
 *
 * 该函数将64位整数转换为网络字节序（大端序）的字节数组。
 *
 * @param[out] out 指向存储结果的字节数组的指针。
 * @param[in] in 输入的64位整数。
 */
void LonglongToNetbyte(uint8_t *out, uint64_t in)
{
	if(!out)
		return;
	
	int16_t bytes = 0x1234;
	if(*(uint8_t *)&bytes == 0x34)
	{
		uint32_t outLen = sizeof(uint64_t);
		for(uint32_t i = 0; i < outLen; i++)
			*(out + i) = *((uint8_t *)&in + outLen - 1 - i);
	}
}

/**
 * @brief 将网络字节序（大端序）的字节数组转换为16位整数。
 *
 * 该函数将网络字节序（大端序）的字节数组转换为16位整数。
 *
 * @param[out] out 指向存储结果的16位整数的指针。
 * @param[in] in 指向存储输入字节数组的指针。
 * @param[in] inLen 输入字节数组的长度。
 */
void NetbyteToShort(uint16_t *out, uint8_t *in, uint32_t inLen)
{
	if(!out || !in || inLen != 2)
		return;
	
	int16_t bytes = 0x1234;
	if(*(uint8_t *)&bytes == 0x34)
	{
		for(uint32_t i = 0; i < inLen; i++)
			*((uint8_t *)out + i) = *(in + inLen - 1 - i);
	}
}

/**
 * @brief 将网络字节序（大端序）的字节数组转换为32位整数。
 *
 * 该函数将网络字节序（大端序）的字节数组转换为32位整数。
 *
 * @param[out] out 指向存储结果的32位整数的指针。
 * @param[in] in 指向存储输入字节数组的指针。
 * @param[in] inLen 输入字节数组的长度。
 */
void NetbyteToInt(uint32_t *out, uint8_t *in, uint32_t inLen)
{
	if(!out || !in || inLen != 4)
		return;
	
	int16_t bytes = 0x1234;
	if(*(uint8_t *)&bytes == 0x34)
	{
		for(uint32_t i = 0; i < inLen; i++)
			*((uint8_t *)out + i) = *(in + inLen - 1 - i);
	}
}

/**
 * @brief 将网络字节序（大端序）的字节数组转换为64位整数。
 *
 * 该函数将网络字节序（大端序）的字节数组转换为64位整数。
 *
 * @param[out] out 指向存储结果的64位整数的指针。
 * @param[in] in 指向存储输入字节数组的指针。
 * @param[in] inLen 输入字节数组的长度。
 */
void NetbyteToLonglong(uint64_t *out, uint8_t *in, uint32_t inLen)
{
	if(!out || !in || inLen != 8)
		return;
	
	int16_t bytes = 0x1234;
	if(*(uint8_t *)&bytes == 0x34)
	{
		for(uint32_t i = 0; i < inLen; i++)
			*((uint8_t *)out + i) = *(in + inLen - 1 - i);
	}
}