/**
 * @file      hexTool.c
 * @brief     convert string to hex
 * @author    shaobo.wang
 * @version   v1
 * @date      2023-09-12 15:43:20
 * @copyright Copyright (c) V2XLINK
 */
 
#include "hexTool.h"

/**
 * @brief 将十六进制字符串转换为字节数组。
 *
 * 该函数将十六进制字符串转换为字节数组。输入字符串中的每两个字符表示输出数组中的一个字节。
 *
 * @param[out] out 指向存储转换后数据的输出字节数组的指针。
 * @param[in] in 指向包含十六进制字符的输入字符串的指针。
 * @param[in] inLen 输入字符串的长度。
 */
void StringToHex(uint8_t *out, char *in, uint32_t inLen)
{
	if(!out || !in || inLen == 0 || inLen % 2 != 0)
		return;
	
    uint8_t high = 0, low = 0;
    for(uint32_t i = 0; i < inLen; i++)
    {
        if(*in >= '0' && *in <= '9')
            high = *in - '0';
        else if(*in >= 'A' && *in <= 'Z')
            high = *in - 'A' + 0x0A;
        else if(*in >= 'a' && *in <= 'z')
            high = *in - 'a' + 0x0A;
		
        in++;
		
        if(*in >= '0' && *in <= '9')
            low = *in - '0';
        else if(*in >= 'A' && *in <= 'Z')
            low = *in - 'A' + 0x0A;
        else if(*in >= 'a' && *in <= 'z')
            low = *in - 'a' + 0x0A;
		
        *out = high << 4 | low;
        out++;
        in++;
    }
}

/**
 * @brief 将字节数组转换为十六进制字符串。
 *
 * 该函数将字节数组转换为十六进制字符串。输入数组中的每个字节都由输出字符串中的两个字符表示。
 *
 * @param[out] out 指向存储转换后数据的输出字符串的指针。
 * @param[in] in 指向输入字节数组的指针。
 * @param[in] inLen 输入字节数组的长度。
 */
void HexToString(char *out, uint8_t *in, uint32_t inLen)
{
	if(!out || !in || inLen == 0)
		return;
	
    for(uint32_t i = 0; i < inLen; i++)
    {
        char high = *in >> 4 & 0x0F;
        char low = *in & 0x0F;
        if(high > 9)
            high = high + 'A' - 0x0A;
        else
            high = high + '0';
        if(low > 9)
            low = low + 'A' - 0x0A;
        else
            low = low + '0';
        *out = high;
        out++;
        *out = low;
        out++;
        in++;
    }
}