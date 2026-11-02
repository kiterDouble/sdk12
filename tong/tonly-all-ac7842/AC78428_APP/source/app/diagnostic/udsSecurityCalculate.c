// #include "FreeRTOS.h"
// #include "task.h"
// #include <string.h>
// #include "stdlib.h"
// #include "udsSecurityCalculate.h"

// /*********configure seed length**************/

// static const uint8_t SeedLength = 4;

// #define NC_DEFAULT_SEED  0xa548fd85 /* default seed used if result from random generator is 0 */

// #define NC_UDS_KEYMASK_LV1   0x2a7c9a38
// #define NC_UDS_KEYMASK_LV2   0x391adc35
// #define NC_UDS_KEYMASK_LV3   0x271ad321

// const unsigned int ncUdsKeymul[32] = {

// 0x7678,0x9130,0xd753,0x750f,0x72cb,0x55f7,0x13da,0x786b,

// 0x372a,0x4932,0x0e7c,0x3687,0x3261,0xa82c,0x8935,0xd00c,

// 0x1995,0x4311,0xb854,0x0d8d,0x9863,0x1a21,0xf753,0xd6d3,

// 0xb15d,0x7f3d,0x6821,0x791c,0x26c5,0x2e37,0x0e69,0x64a0 };
// #if 0
// /*******************************/
// static unsigned long croleft(unsigned long c, unsigned int b)
// {
// 	unsigned long left=c<<b;
// 	unsigned long right=c>>(32-b);
// 	unsigned long croleftvalue=left|right;
// 	return croleftvalue;
// }
// /*******************************/

// /*******************************/

// static unsigned short int croshortright(unsigned short int c, unsigned short int b)
// {
// 	unsigned short int right=c>>b;
// 	unsigned short int left=c<<(16-b);
// 	unsigned short int crorightvalue=left|right;
// 	return crorightvalue;
// }
// /*******************************/

// /*******************************/
// static unsigned long mulu32_lv1(unsigned long val1,unsigned long val2)
// {
//   unsigned long x,y,z,p;
//   x = (val1&NC_UDS_KEYMASK_LV1)|((~val1)&val2);
//   y = ((croleft(val1,1))&(croleft(val2,14)))|((croleft(NC_UDS_KEYMASK_LV1,21))&(~(croleft(val1,30))));
//   z = (croleft(val1,17))^(croleft(val2,4))^(croleft(NC_UDS_KEYMASK_LV1,11));
//   p = x^y^z;
//   return p;
// }
// /*******************************/

// /*******************************/
// /*
// static unsigned long uds_calc_key_lv1(unsigned long seed)
// {
//   unsigned long temp;
//   unsigned short int  index;
//   unsigned short int  mult1;
//   unsigned short int  mult2;

//   if(seed == 0)
//   {
// 	seed = NC_DEFAULT_SEED;
//   }
//   else
//   {}
//   for (index=0x5D39, temp=0x80000000; temp; temp>>=1)
//   {
// 	if (temp & seed)
// 	{
// 		index = croshortright(index, 1);
// 		if (temp & NC_UDS_KEYMASK_LV1)
// 		{
// 			index ^= 0x74c9;
// 		}
// 	}
//   }
//   mult1 = (ncUdsKeymul[(index>>2) & ((1<<5)-1)]^index);
//   mult2 = (ncUdsKeymul[(index>>8) & ((1<<5)-1)]^index);
//   temp = (((unsigned long)mult1)<<16)|((unsigned long)mult2);
//   temp = mulu32_lv1(seed,temp);
//   return temp;
// }
// */
// /*******************************/

// /*******************************/
// static unsigned long mulu32_lv2(unsigned long val1,unsigned long val2)
// {
//   unsigned long x,y,z,p;
//   x = (val1&NC_UDS_KEYMASK_LV2)|((~val1)&val2);
//   y = ((croleft(val1,1))&(croleft(val2,14)))|((croleft(NC_UDS_KEYMASK_LV2,21))&(~(croleft(val1,30))));
//   z = (croleft(val1,17))^(croleft(val2,4))^(croleft(NC_UDS_KEYMASK_LV2,11));
//   p = x^y^z;
//   return p;
// }
// /*******************************/

// /*******************************/
// /*
// static unsigned long uds_calc_key_lv2(unsigned long seed)
// {
//   unsigned long temp;
//   unsigned short int  index;
//   unsigned short int  mult1;
//   unsigned short int  mult2;

//   if(seed == 0)
//   {
// 	seed = NC_DEFAULT_SEED;
//   }
//   else
//   {}
//   for (index=0x5D39, temp=0x80000000; temp; temp>>=1)
//   {
// 	if (temp & seed)
// 	{
// 		index = croshortright(index, 1);
// 		if (temp & NC_UDS_KEYMASK_LV2)
// 		{
// 			index ^= 0x74c9;
// 		}
// 	}
//   }
//   mult1 = (ncUdsKeymul[(index>>2) & ((1<<5)-1)]^index);
//   mult2 = (ncUdsKeymul[(index>>8) & ((1<<5)-1)]^index);
//   temp = (((unsigned long)mult1)<<16)|((unsigned long)mult2);
//   temp = mulu32_lv2(seed,temp);
//   return temp;
// }
// */
// static unsigned long mulu32_lv3(unsigned long val1,unsigned long val2)
// {
//   unsigned long x,y,z,p;
//   x = (val1&NC_UDS_KEYMASK_LV3)|((~val1)&val2);
//   y = ((croleft(val1,1))&(croleft(val2,14)))|((croleft(NC_UDS_KEYMASK_LV3,21))&(~(croleft(val1,30))));
//   z = (croleft(val1,17))^(croleft(val2,4))^(croleft(NC_UDS_KEYMASK_LV3,11));
//   p = x^y^z;
//   return p;
// }
// /*
// static unsigned long uds_calc_key_lv3(unsigned long seed)
// {
//   unsigned long temp;
//   unsigned short int  index;
//   unsigned short int  mult1;
//   unsigned short int  mult2;

//   if(seed == 0)
//   {
// 	seed = NC_DEFAULT_SEED;
//   }
//   else
//   {

//   }
//   for (index=0x5D39, temp=0x80000000; temp; temp>>=1)
//   {
// 	if (temp & seed)
// 	{
// 		index = croshortright(index, 1);
// 		if (temp & NC_UDS_KEYMASK_LV3)
// 		{
// 			index ^= 0x74c9;
// 		}
// 	}
//   }
//   mult1 = (ncUdsKeymul[(index>>2) & ((1<<5)-1)]^index);
//   mult2 = (ncUdsKeymul[(index>>8) & ((1<<5)-1)]^index);
//   temp = (((unsigned long)mult1)<<16)|((unsigned long)mult2);
//   temp = mulu32_lv3(seed,temp);
//   return temp;
// }
// */
// /*******************************/
// #endif
// void UdsSeedtoKey(uint8_t *seed,uint8_t seedLen,uint8_t *key,uint8_t *pKeyLen,SecurityCalculateLevel_e securityLevel)
// {
//   /*
//   uint32_t u32Key;
//   uint32_t u32Seed;

//   u32Key = 0x00;
//   u32Seed = (seed[0]<<24)+(seed[1]<<16)+(seed[2]<<8)+(seed[3]<<0);

//   if(securityLevel==E_SECURITY_CALCULATE_LEVEL1)
//   {
//     u32Key = uds_calc_key_lv1(u32Seed);
//   }
//   else if(securityLevel==E_SECURITY_CALCULATE_LEVEL2)//FBL
//   {
//     u32Key = uds_calc_key_lv2(u32Seed);
//   }
//   else if(securityLevel==E_SECURITY_CALCULATE_LEVEL3)//FBL
//   {
//     u32Key = uds_calc_key_lv3(u32Seed);
//   }
//   else if(securityLevel==E_SECURITY_CALCULATE_LEVEL2)
//   {

//   }
//   key[0] = (uint8_t)(u32Key>>24);
//   key[1] = (uint8_t)(u32Key>>16);
//   key[2] = (uint8_t)(u32Key>>8);
//   key[3] = (uint8_t)(u32Key>>0);
//   *pKeyLen = 4;
// */
//       uint32_t i;
//   uint32_t u32Key;
//   uint32_t u32Seed;

//   u32Key = 0x00;
//   u32Seed = (seed[0]<<24)+(seed[1]<<16)+(seed[2]<<8)+(seed[3]<<0);

//  if(u32Seed != 0)
//   {
//   	for(i = 0;i<35;i++)
//   	{
//   		if(u32Seed&0x80000000)
//   		{
//   			u32Seed = u32Seed << 1;
//             if(securityLevel == E_SECURITY_CALCULATE_LEVEL1)//APP
//   			{
//   				u32Seed = u32Seed ^ 0x54424F58;
//   			}
//   			else if(securityLevel == E_SECURITY_CALCULATE_LEVEL2)//BOOT
//   			{
//   				u32Seed = u32Seed ^ 0x54424F35;
//   			}
//   		}
//   		else
//   		{
//   			u32Seed = u32Seed << 1;
//   		}
//   	}
//   	u32Key = u32Seed;
//   }
//   key[0] = (uint8_t)(u32Key>>24);
//   key[1] = (uint8_t)(u32Key>>16);
//   key[2] = (uint8_t)(u32Key>>8);
//   key[3] = (uint8_t)(u32Key>>0);
//   *pKeyLen = 4;
// }

// void UdsGetSeed(uint8_t *pSeed,uint8_t *pSeedLength)
// {
// #if(1)
//   uint32_t randData;
//   uint16_t i;

//   randData = xTaskGetTickCount();
//   srand(randData);
//   for (i = 0; i < SeedLength; i++)
//   {
//         pSeed[i] = rand();
//   }
// #else//debug test
//   pSeed[0] = 0x12;
//   pSeed[1] = 0x34;
//   pSeed[2] = 0x56;
//   pSeed[3] = 0x78;
// #endif
//   *pSeedLength = SeedLength;
// }

#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdlib.h>
#include "udsSecurityCalculate.h"

#if 1 //SQ 
// =================================================================================================
// --- BEGIN: AES-128 Implementation ---
// =================================================================================================
static unsigned char sBox[] = {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, 0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, 0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, 0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, 0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};
static unsigned char Sbox[256];
static unsigned char w[11][4][4];
static void KeyExpansion(const unsigned char *key, unsigned char w[][4][4]);
static unsigned char FFmul(unsigned char a, unsigned char b);
static void SubBytes(unsigned char state[][4]);
static void ShiftRows(unsigned char state[][4]);
static void MixColumns(unsigned char state[][4]);
static void AddRoundKey(unsigned char state[][4], unsigned char k[][4]);
static void AES_Init(const unsigned char *key);
static unsigned char *AES_Cipher(unsigned char *input);

static void KeyExpansion(const unsigned char *key, unsigned char w[][4][4])
{
  int i, j, r, c;
  unsigned char rc[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};
  for (r = 0; r < 4; r++)
  {
    for (c = 0; c < 4; c++)
    {
      w[0][r][c] = key[r + c * 4];
    }
  }
  for (i = 1; i <= 10; i++)
  {
    for (j = 0; j < 4; j++)
    {
      unsigned char t[4];
      for (r = 0; r < 4; r++)
      {
        t[r] = j ? w[i][r][j - 1] : w[i - 1][r][3];
      }
      if (j == 0)
      {
        unsigned char temp = t[0];
        for (r = 0; r < 3; r++)
        {
          t[r] = Sbox[t[(r + 1) % 4]];
        }
        t[3] = Sbox[temp];
        t[0] ^= rc[i - 1];
      }
      for (r = 0; r < 4; r++)
      {
        w[i][r][j] = w[i - 1][r][j] ^ t[r];
      }
    }
  }
}

static unsigned char FFmul(unsigned char a, unsigned char b)
{
  unsigned char bw[4];
  unsigned char res = 0;
  int i;
  bw[0] = b;
  for (i = 1; i < 4; i++)
  {
    bw[i] = bw[i - 1] << 1;
    if (bw[i - 1] & 0x80)
    {
      bw[i] ^= 0x1b;
    }
  }
  for (i = 0; i < 4; i++)
  {
    if ((a >> i) & 0x01)
    {
      res ^= bw[i];
    }
  }
  return res;
}

static void SubBytes(unsigned char state[][4])
{
  int r, c;
  for (r = 0; r < 4; r++)
  {
    for (c = 0; c < 4; c++)
    {
      state[r][c] = Sbox[state[r][c]];
    }
  }
}

static void ShiftRows(unsigned char state[][4])
{
  unsigned char t[4];
  int r, c;
  for (r = 1; r < 4; r++)
  {
    for (c = 0; c < 4; c++)
    {
      t[c] = state[r][(c + r) % 4];
    }
    for (c = 0; c < 4; c++)
    {
      state[r][c] = t[c];
    }
  }
}

static void MixColumns(unsigned char state[][4])
{
  unsigned char t[4];
  int r, c;
  for (c = 0; c < 4; c++)
  {
    for (r = 0; r < 4; r++)
    {
      t[r] = state[r][c];
    }
    for (r = 0; r < 4; r++)
    {
      state[r][c] = FFmul(0x02, t[r]) ^ FFmul(0x03, t[(r + 1) % 4]) ^ FFmul(0x01, t[(r + 2) % 4]) ^ FFmul(0x01, t[(r + 3) % 4]);
    }
  }
}

static void AddRoundKey(unsigned char state[][4], unsigned char k[][4])
{
  int r, c;
  for (c = 0; c < 4; c++)
  {
    for (r = 0; r < 4; r++)
    {
      state[r][c] ^= k[r][c];
    }
  }
}

static void AES_Init(const unsigned char *key)
{
  memcpy(Sbox, sBox, 256);
  KeyExpansion(key, w);
}

static unsigned char *AES_Cipher(unsigned char *input)
{
  unsigned char state[4][4];
  int i, r, c;
  for (r = 0; r < 4; r++)
  {
    for (c = 0; c < 4; c++)
    {
      state[r][c] = input[c * 4 + r];
    }
  }
  AddRoundKey(state, w[0]);
  for (i = 1; i <= 10; i++)
  {
    SubBytes(state);
    ShiftRows(state);
    if (i != 10)
      MixColumns(state);
    AddRoundKey(state, w[i]);
  }
  for (r = 0; r < 4; r++)
  {
    for (c = 0; c < 4; c++)
    {
      input[c * 4 + r] = state[r][c];
    }
  }
  return input;
}
// =================================================================================================
// --- END: AES-128 Implementation ---
// =================================================================================================

// --------------------------安全等级私钥 --------------------------
// Level1 PrivKey: 47 4E 27 4A 30 A5 9F D5 AE 79 8E 67 23 0D 1A 93
static const unsigned char PrivKey_Level1[16] = {0x47,0x4E,0x27,0x4A,0x30,0xA5,0x9F,0xD5,0xAE,0x79,0x8E,0x67,0x23,0x0D,0x1A,0x93};
// Level2 PrivKey: F3 F7 BF D2 40 BB 15 77 72 15 E2 8B D7 86 21 C5
static const unsigned char PrivKey_Level2[16] = {0xF3,0xF7,0xBF,0xD2,0x40,0xBB,0x15,0x77,0x72,0x15,0xE2,0x8B,0xD7,0x86,0x21,0xC5};
// Level3 PrivKey: E7 56 78 42 20 BC 26 A5 D8 66 BE C5 E6 83 EA AE
static const unsigned char PrivKey_Level3[16] = {0xE7,0x56,0x78,0x42,0x20,0xBC,0x26,0xA5,0xD8,0x66,0xBE,0xC5,0xE6,0x83,0xEA,0xAE};

// -----------------------Seed长度为16字节 --------------------------
static const uint8_t SeedLength = 16;

// -------------------------- 公共API函数（修改适配） --------------------------
void UdsGetSeed(uint8_t *pSeed, uint8_t *pSeedLength)
{
  uint32_t randData = 0;
  uint16_t i = 0;
  
  if (pSeed == NULL || pSeedLength == NULL) {
    return;
  }
  
  // 使用FreeRTOS TickCount作为随机种子
  randData = xTaskGetTickCount();
  srand((unsigned int)randData);
  
  // 生成16字节随机种子
  for (i = 0; i < SeedLength; i++)
  {
    pSeed[i] = (unsigned char)(rand() & 0xFF);
  }
  
  *pSeedLength = SeedLength;
}

int Seed2Key(const unsigned char *iSeedArray, const unsigned short iSeedArraySize, const unsigned int iSecurityLevel,
             const char *iVariant, unsigned char *ioKeyArray, unsigned int iKeyArraySize, unsigned int *oSize)
{
  unsigned char aes_result[16] = {0};  // AES加密结果  
  unsigned char dat = (unsigned char)iSecurityLevel;
  const unsigned char *privKey = NULL;
  // uint16_t i = 0;
  
  if (iSeedArray == NULL || ioKeyArray == NULL || oSize == NULL) {
    return 0;
  }
  
  // Seed必须16字节，Key缓冲区至少16字节
  if (iSeedArraySize != 16 || iKeyArraySize < 16) {
    return 0;
  }
  
  // -------------------------- 安全等级映射 --------------------------
  // 0x03映射为0x02
  if (iSecurityLevel == 0x03) {
    dat = 0x02;
  } else {
    dat = (unsigned char)iSecurityLevel;
  }
  
  // -------------------------- 选择对应等级私钥 --------------------------
  switch (dat) {
    case 0x01:  // Level1 (27 01/02)
      privKey = PrivKey_Level1;
      break;
    case 0x02:  // Level2 (27 03/04)
      privKey = PrivKey_Level2;
      break;
    case 0x09:  // Level3 (27 09/0A)
      privKey = PrivKey_Level3;
      break;
    default:
      // 不支持的安全等级
      return 0;
  }
  
  // -------------------------- AES128 ECB加密核心逻辑 --------------------------
  // 初始化AES（使用对应等级的私钥）
  AES_Init(privKey);
  
  // 复制16字节Seed到临时缓冲区
  memcpy(aes_result, iSeedArray, 16);
  
  // AES128 ECB加密：Seed + 私钥 -> Key
  AES_Cipher(aes_result);
  *oSize = 16;
  memcpy(ioKeyArray, aes_result, *oSize);
  return 1;
}
#else
// =================================================================================================
// --- BEGIN: AES-128 Implementation (来自客户文件) ---
// =================================================================================================
static unsigned char sBox[] = {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, 0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, 0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, 0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, 0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};
static unsigned char Sbox[256];
static unsigned char w[11][4][4];
static void KeyExpansion(const unsigned char *key, unsigned char w[][4][4]);
static unsigned char FFmul(unsigned char a, unsigned char b);
static void SubBytes(unsigned char state[][4]);
static void ShiftRows(unsigned char state[][4]);
static void MixColumns(unsigned char state[][4]);
static void AddRoundKey(unsigned char state[][4], unsigned char k[][4]);
static void AES_Init(const unsigned char *key);
static unsigned char *AES_Cipher(unsigned char *input);
static void KeyExpansion(const unsigned char *key, unsigned char w[][4][4])
{
  int i, j, r, c;
  unsigned char rc[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};
  for (r = 0; r < 4; r++)
  {
    for (c = 0; c < 4; c++)
    {
      w[0][r][c] = key[r + c * 4];
    }
  }
  for (i = 1; i <= 10; i++)
  {
    for (j = 0; j < 4; j++)
    {
      unsigned char t[4];
      for (r = 0; r < 4; r++)
      {
        t[r] = j ? w[i][r][j - 1] : w[i - 1][r][3];
      }
      if (j == 0)
      {
        unsigned char temp = t[0];
        for (r = 0; r < 3; r++)
        {
          t[r] = Sbox[t[(r + 1) % 4]];
        }
        t[3] = Sbox[temp];
        t[0] ^= rc[i - 1];
      }
      for (r = 0; r < 4; r++)
      {
        w[i][r][j] = w[i - 1][r][j] ^ t[r];
      }
    }
  }
}
static unsigned char FFmul(unsigned char a, unsigned char b)
{
  unsigned char bw[4];
  unsigned char res = 0;
  int i;
  bw[0] = b;
  for (i = 1; i < 4; i++)
  {
    bw[i] = bw[i - 1] << 1;
    if (bw[i - 1] & 0x80)
    {
      bw[i] ^= 0x1b;
    }
  }
  for (i = 0; i < 4; i++)
  {
    if ((a >> i) & 0x01)
    {
      res ^= bw[i];
    }
  }
  return res;
}
static void SubBytes(unsigned char state[][4])
{
  int r, c;
  for (r = 0; r < 4; r++)
  {
    for (c = 0; c < 4; c++)
    {
      state[r][c] = Sbox[state[r][c]];
    }
  }
}
static void ShiftRows(unsigned char state[][4])
{
  unsigned char t[4];
  int r, c;
  for (r = 1; r < 4; r++)
  {
    for (c = 0; c < 4; c++)
    {
      t[c] = state[r][(c + r) % 4];
    }
    for (c = 0; c < 4; c++)
    {
      state[r][c] = t[c];
    }
  }
}
static void MixColumns(unsigned char state[][4])
{
  unsigned char t[4];
  int r, c;
  for (c = 0; c < 4; c++)
  {
    for (r = 0; r < 4; r++)
    {
      t[r] = state[r][c];
    }
    for (r = 0; r < 4; r++)
    {
      state[r][c] = FFmul(0x02, t[r]) ^ FFmul(0x03, t[(r + 1) % 4]) ^ FFmul(0x01, t[(r + 2) % 4]) ^ FFmul(0x01, t[(r + 3) % 4]);
    }
  }
}
static void AddRoundKey(unsigned char state[][4], unsigned char k[][4])
{
  int r, c;
  for (c = 0; c < 4; c++)
  {
    for (r = 0; r < 4; r++)
    {
      state[r][c] ^= k[r][c];
    }
  }
}
static void AES_Init(const unsigned char *key)
{
  memcpy(Sbox, sBox, 256);
  KeyExpansion(key, w);
}
static unsigned char *AES_Cipher(unsigned char *input)
{
  unsigned char state[4][4];
  int i, r, c;
  for (r = 0; r < 4; r++)
  {
    for (c = 0; c < 4; c++)
    {
      state[r][c] = input[c * 4 + r];
    }
  }
  AddRoundKey(state, w[0]);
  for (i = 1; i <= 10; i++)
  {
    SubBytes(state);
    ShiftRows(state);
    if (i != 10)
      MixColumns(state);
    AddRoundKey(state, w[i]);
  }
  for (r = 0; r < 4; r++)
  {
    for (c = 0; c < 4; c++)
    {
      input[c * 4 + r] = state[r][c];
    }
  }
  return input;
}
// =================================================================================================
// --- END: AES-128 Implementation ---
// =================================================================================================

// ECU Variant - 算法的核心密钥
static const unsigned char Var[16] = {0x53, 0xE5, 0x18, 0xB6, 0x39, 0x39, 0x04, 0xE7, 0x97, 0x81, 0x9C, 0x7A, 0x94, 0x1F, 0x9D, 0xDC};
static const uint8_t SeedLength = 4;

// =================================================================================================
// --- Public API Functions ---
// =================================================================================================

void UdsGetSeed(uint8_t *pSeed, uint8_t *pSeedLength)
{
  uint32_t randData;
  uint16_t i;
  randData = xTaskGetTickCount();
  srand(randData);
  for (i = 0; i < SeedLength; i++)
  {
    pSeed[i] = rand();
  }
  *pSeedLength = SeedLength;
}

int Seed2Key(const unsigned char *iSeedArray, const unsigned short iSeedArraySize, const unsigned int iSecurityLevel,
             const char *iVariant, unsigned char *ioKeyArray, unsigned int iKeyArraySize, unsigned int *oSize)
{
  unsigned char temp[16] = {0};
  // unsigned int Size = 0;
  unsigned char dat = iSecurityLevel;
  unsigned char data[16];
  unsigned char i;

  // for canoe used
  if (iSecurityLevel == 0x03)
  {
    dat = 0x02;
  }

  // GAC security level:01-27 01/02;02-27 03/04;11-27 11/12
  if (iSecurityLevel != 0x01 && iSecurityLevel != 0x02 && iSecurityLevel != 0x03 && iSecurityLevel != 0x21 && iSecurityLevel != 0x11)
    return 0;

  AES_Init(Var);
  for (i = 0; i < 16; i++)
  {
    temp[i] = dat;
  }
  for (i = 0; i < 4; i++)
  {
    temp[i] = iSeedArray[i];
  }
  AES_Cipher(temp);
  for (i = 0; i < 8; i++)
  {
    data[i] = temp[i] ^ temp[15 - i];
  }
  for (i = 0; i < 4; i++)
  {
    data[i] = data[i] ^ data[3 - i];
  }
  for (i = 0; i < 4; i++)
  {
    ioKeyArray[i] = data[i];
  }

  *oSize = 4;
  return 1;
}
#endif