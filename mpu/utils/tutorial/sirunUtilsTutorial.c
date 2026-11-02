/**
 * @file      sirun_utils_tutorial.c
 * @brief     sirun_utils tutorial
 * @author    shaobo.wang
 * @version   v1
 * @date      2023-09-12 15:43:20
 * @copyright Copyright (c) V2XLINK
 */
 
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "crcTool.h"
#include "hexTool.h"
#include "netbyte.h"
#include "utils.h"

static void hexPrintf(uint8_t *in, uint32_t inLen)
{
	if(!in || inLen == 0)
		return;
	
	for (uint32_t i = 0; i < inLen; i++)
		printf("%02X", in[i]);
	printf("\n");
}

int main()
{
	char test[] = "1234567890";
	uint8_t out[16] = "";
	StringToHex(out, test, strlen(test));
	hexPrintf(out, strlen(test) / 2);
	
	char out2[16] = "";
	HexToString(out2, out, strlen(test) / 2);
	printf("%s\n", out2);
	
	uint8_t instr[] = "Hello, World";
	uint8_t outstr1[1024] = "";
	memset(outstr1, 0x00, sizeof(outstr1));
	Base64Encode(outstr1, instr, strlen((char *)instr));
	printf("Base64Encode: %s\n", outstr1);
	
	uint8_t outstr2[1024] = "";
	memset(outstr2, 0x00, sizeof(outstr2));
	Base64Decode(outstr2, outstr1, strlen((char *)outstr1));
	printf("Base64Decode: %s\n",outstr2);
	
	uint8_t ret[16] = "";
	memset(ret, 0x00, sizeof(ret));
	Md5(ret, instr,strlen((char *)instr));
	hexPrintf(ret, 16);
	
	uint8_t ret2[32] = "";
	memset(ret2, 0x00, sizeof(ret2));
	Sha256(ret2, instr,strlen((char *)instr));
	hexPrintf(ret2, 32);
	
    // 计算CRC8
	uint8_t crcResult3 = Crc8(instr, strlen((char *)instr), Crc8Def);
	printf("Crc8: %X\n", crcResult3);
	
	crcResult3 = Crc8(instr, strlen((char *)instr), Crc8Itu);
	printf("Crc8Itu: %X\n", crcResult3);
	
    // 计算CRC16
    uint16_t crcResult2 = Crc16(instr, strlen((char *)instr), Crc16Ccitt);
	printf("Crc16Ccitt: %X\n", crcResult2);
	
    crcResult2 = Crc16(instr, strlen((char *)instr), Crc16Ibm);
	printf("Crc16Ibm: %X\n", crcResult2);
	
    crcResult2 = Crc16(instr, strlen((char *)instr), Crc16Modbus);
	printf("Crc16Modbus: %X\n", crcResult2);
	
    crcResult2 = Crc16(instr, strlen((char *)instr), Crc16Usb);
	printf("Crc16Usb: %X\n", crcResult2);
	
	// 计算CRC32
    uint32_t crcResult = Crc32(instr, strlen((char *)instr), Crc32Def);
	printf("Crc32Def: %X\n", crcResult);
	
    crcResult = Crc32(instr, strlen((char *)instr), Crc32Mpeg2);
	printf("Crc32Mpeg2: %X\n", crcResult);
	
	return 0;
}
