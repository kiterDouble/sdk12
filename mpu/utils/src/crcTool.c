/**
 * @file      crcTool.c
 * @brief     CRC related functions
 * @author    shaobo.wang
 * @version   v1
 * @date      2023-09-12 15:43:20
 * @copyright Copyright (c) V2XLINK
 */
#include "crcTool.h"
/**
 * @brief 将输入的 16 位数值的位反转，并将结果存储在另一个 16 位变量中。
 *
 * 该函数接受一个 16 位输入，反转其位，并将结果存储在另一个 16 位变量中。
 *
 * @param[out] out 指向 uint16_t 变量的指针，用于存储反转后的值。
 * @param[in] in 指向 uint16_t 变量的指针，包含输入值。
 * 
 * @return 成功时返回 1，失败时返回 -1（如果 out 或 in 指针为 NULL）。
 */
static int32_t InvertShort(uint16_t *out, uint16_t *in)
{
	if(!out || !in)
		return -1;
	
	uint16_t temp = 0;
	for(uint32_t i = 0; i < 16; i++)
	{
		if(*in & (1 << i))
			temp |= 1 << (15 - i);
	}
	*out = temp;
	
	return 1;
}

/**
 * @brief 将输入的 32 位数值的位反转，并将结果存储在另一个 32 位变量中。
 *
 * 该函数接受一个 32 位输入，反转其位，并将结果存储在另一个 32 位变量中。
 *
 * @param[out] out 指向 uint32_t 变量的指针，用于存储反转后的值。
 * @param[in] in 指向 uint32_t 变量的指针，包含输入值。
 * 
 * @return 成功时返回 1，失败时返回 -1（如果 out 或 in 指针为 NULL）。
 */
static int32_t InvertInt(uint32_t *out, uint32_t *in)
{
	if(!out || !in)
		return -1;
	
	uint32_t temp = 0;
	for(uint32_t i = 0; i < 32; i++)
	{
		if(*in & (1 << i))
			temp |= 1 << (31 - i);
	}
	*out = temp;
	
	return 1;
}

/**
 * @brief 使用指定的 CRC 类型计算输入字节数组的 CRC-8 校验码。
 *
 * @param[in] in 指向输入字节数组的指针。
 * @param[in] inLen 输入字节数组的长度。
 * @param[in] Crc8Type 指定 CRC-8 的类型。
 * 
 * @return 计算得到的 CRC-8 校验码。
 */
static uint8_t Crc8NoInvert(uint8_t *in, uint32_t inLen, Crc8TypeE Crc8Type)
{
	uint8_t crcIn = 0x00;
	uint8_t crcPoly = 0x07;
	uint8_t crcXor = 0x00;
	
	if(Crc8Type == Crc8Itu)
		crcXor = 0x55;
	
    while(inLen--)
    {
		crcIn ^= *in++;
        for(uint32_t i = 0; i < 8; i++)
        {
            if(crcIn & 0x80)
                crcIn = (crcIn << 1) ^ crcPoly;
            else
                crcIn = (crcIn << 1);
        }
    }
	
	return crcIn ^ crcXor;
}

/**
 * @brief 计算输入字节数组的 CRC-8 校验码。
 *
 * @param[in] in 指向输入字节数组的指针。
 * @param[in] inLen 输入字节数组的长度。
 * @param[in] Crc8Type 指定 CRC-8 的类型。
 * 
 * @return 计算得到的 CRC-8 校验码。
 */
uint8_t Crc8(uint8_t *in, uint32_t inLen, Crc8TypeE Crc8Type)
{
	uint8_t ret = 0;
	if(Crc8Type == Crc8Def || Crc8Type == Crc8Itu)
		ret = Crc8NoInvert(in, inLen, Crc8Type);
	
	return ret;
}

/**
 * @brief 使用指定的 CRC 类型计算输入字节数组的 CRC-16 校验码，并反转结果。
 *
 * @param[in] in 指向输入字节数组的指针。
 * @param[in] inLen 输入字节数组的长度。
 * @param[in] Crc16Type 指定 CRC-16 的类型。
 * 
 * @return 计算得到的 CRC-16 校验码。
 */
static uint16_t Crc16Invert(uint8_t *in, uint32_t inLen, Crc16TypeE Crc16Type)
{
	uint16_t crcIn = 0x0000;
	uint16_t crcPoly = 0x1021;
	uint16_t crcXor = 0x0000;
	
	if(Crc16Type == Crc16Modbus || Crc16Type == Crc16Usb)
		crcIn = 0xFFFF;
	if(Crc16Type == Crc16Ibm || Crc16Type == Crc16Modbus || Crc16Type == Crc16Usb)
		crcPoly = 0x8005;
	if(Crc16Type == Crc16Usb)
		crcXor = 0xFFFF;
	
	InvertShort(&crcPoly, &crcPoly);
	
    while(inLen--)
    {
        crcIn ^= *in++;
        for(uint32_t i = 0; i < 8; i++)
        {
            if(crcIn & 0x0001)
                crcIn = (crcIn >> 1) ^ crcPoly;
            else
                crcIn = (crcIn >> 1);
        }
    }
	
    return crcIn ^ crcXor;
}

/**
 * @brief 计算输入字节数组的 CRC-16 校验码。
 *
 * @param[in] in 指向输入字节数组的指针。
 * @param[in] inLen 输入字节数组的长度。
 * @param[in] Crc16Type 指定 CRC-16 的类型。
 * 
 * @return 计算得到的 CRC-16 校验码。
 */
uint16_t Crc16(uint8_t *in, uint32_t inLen, Crc16TypeE Crc16Type)
{
	uint16_t ret = 0;
	if(Crc16Type == Crc16Ccitt || Crc16Type == Crc16Ibm || Crc16Type == Crc16Modbus || Crc16Type == Crc16Usb)
		ret = Crc16Invert(in, inLen, Crc16Type);
	
	return ret;
}

/**
 * @brief 使用指定的 CRC 类型计算输入字节数组的 CRC-32 校验码，并反转结果。
 *
 * @param[in] in 指向输入字节数组的指针。
 * @param[in] inLen 输入字节数组的长度。
 * @param[in] Crc32Type 指定 CRC-32 的类型。
 * 
 * @return 计算得到的 CRC-32 校验码。
 */
static uint32_t Crc32Invert(uint8_t *in, uint32_t inLen, Crc32TypeE Crc32Type)
{
	uint32_t crcIn = 0xFFFFFFFF;
	uint32_t crcPoly = 0x04C11DB7;
	uint32_t crcXor = 0xFFFFFFFF;
	
	InvertInt(&crcPoly, &crcPoly);
	
    while(inLen--)
    {
        crcIn ^= *in++;
        for(uint32_t i = 0; i < 8; i++)
        {
            if(crcIn & 0x00000001)
                crcIn = (crcIn >> 1) ^ crcPoly;
            else
                crcIn = (crcIn >> 1);
        }
    }
	
    return crcIn ^ crcXor;
}

/**
 * @brief 使用指定的 CRC 类型计算输入字节数组的 CRC-32 校验码。
 *
 * @param[in] in 指向输入字节数组的指针。
 * @param[in] inLen 输入字节数组的长度。
 * @param[in] Crc32Type 指定 CRC-32 的类型。
 * 
 * @return 计算得到的 CRC-32 校验码。
 */
static uint32_t Crc32NoInvert(uint8_t *in, uint32_t inLen, Crc32TypeE Crc32Type)
{
	uint32_t crcIn = 0xFFFFFFFF;
	uint32_t crcPoly = 0x04C11DB7;
	uint32_t crcXor = 0x00000000;
	
    while(inLen--)
    {
		crcIn ^= *in++ << 24;
        for(uint32_t i = 0; i < 8; i++)
        {
            if(crcIn & 0x80000000)
                crcIn = (crcIn << 1) ^ crcPoly;
            else
                crcIn = (crcIn << 1);
        }
    }
	
	return crcIn ^ crcXor;
}

/**
 * @brief 计算输入字节数组的 CRC-32 校验码。
 *
 * @param[in] in 指向输入字节数组的指针。
 * @param[in] inLen 输入字节数组的长度。
 * @param[in] Crc32Type 指定 CRC-32 的类型。
 * 
 * @return 计算得到的 CRC-32 校验码。
 */
uint32_t Crc32(uint8_t *in, uint32_t inLen, Crc32TypeE Crc32Type)
{
	uint32_t ret = 0;
	if(Crc32Type == Crc32Def)
		ret = Crc32Invert(in, inLen, Crc32Type);
	else if(Crc32Type == Crc32Mpeg2)
		ret = Crc32NoInvert(in, inLen, Crc32Type);
	
	return ret;
}
