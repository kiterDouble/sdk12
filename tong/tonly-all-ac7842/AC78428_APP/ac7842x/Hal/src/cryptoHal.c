/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name: cryptoHal.c
 Author: 
 Created Time: 
 Description: 
 Others: 
*************************************************/

/****************************** include ***************************************/
#include "cryptoHal.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdlib.h>
#include <time.h>
#include "Ckgen_Hal.h"
#include "logHal.h"
/* Note: Z20K148MS aes_drv.h removed - AC7842x AES hardware differs from Z20K148MS.
   Hardware AES path (AES_SDK_V2) is disabled. Software AES path available via Software_algorithms macro. */

/****************************** Macro Definitions ******************************/
#define Software_algorithms    0
#define BLOCKSIZE              16  //AES-128分组长度为16字节
#define FOURBITRAND            0x0

#if (Software_algorithms)

// uint8_t y[4] -> uint32_t x
#define LOAD32H(x, y) \
  do { (x) = ((uint32_t)((y)[0] & 0xff)<<24) | ((uint32_t)((y)[1] & 0xff)<<16) | \
             ((uint32_t)((y)[2] & 0xff)<<8)  | ((uint32_t)((y)[3] & 0xff));} while(0)

// uint32_t x -> uint8_t y[4]
#define STORE32H(x, y) \
  do { (y)[0] = (uint8_t)(((x)>>24) & 0xff); (y)[1] = (uint8_t)(((x)>>16) & 0xff);   \
       (y)[2] = (uint8_t)(((x)>>8) & 0xff); (y)[3] = (uint8_t)((x) & 0xff); } while(0)

// 从uint32_t x中提取从低位开始的第n个字节
#define BYTE(x, n) (((x) >> (8 * (n))) & 0xff)

/* used for keyExpansion */
// 字节替换然后循环左移1位
#define MIX(x) (((S[BYTE(x, 2)] << 24) & 0xff000000) ^ ((S[BYTE(x, 1)] << 16) & 0xff0000) ^ \
                ((S[BYTE(x, 0)] << 8) & 0xff00) ^ (S[BYTE(x, 3)] & 0xff))

// uint32_t x循环左移n位
#define ROF32(x, n)  (((x) << (n)) | ((x) >> (32-(n))))
// uint32_t x循环右移n位
#define ROR32(x, n)  (((x) >> (n)) | ((x) << (32-(n))))

#endif

/****************************** Type Definitions ******************************/
#if (!Software_algorithms)
typedef union
{
    uint32_t u32Bit[4];
    uint8_t  u8Bit[16];
} Unn4Word16Byte_t;
#endif

/****************************** Global Variables ******************************/
#if (Software_algorithms)
/* for 128-bit blocks, Rijndael never uses more than 10 rcon values */
// AES-128轮常量
static const uint32_t rcon[10] = {
        0x01000000UL, 0x02000000UL, 0x04000000UL, 0x08000000UL, 0x10000000UL,
        0x20000000UL, 0x40000000UL, 0x80000000UL, 0x1B000000UL, 0x36000000UL
};

// S盒
const unsigned char S[256] = {
        0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
        0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
        0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
        0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
        0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
        0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
        0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
        0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
        0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
        0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
        0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
        0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
        0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
        0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
        0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
        0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

//逆S盒
const unsigned char inv_S[256] = {
        0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
        0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
        0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
        0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
        0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
        0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
        0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
        0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
        0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
        0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
        0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
        0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
        0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
        0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
        0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
        0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};
#endif
/****************************** Function Declarations *************************/
/****************************** Public Function Implementations ***************/


#if (Software_algorithms)

/* copy in[16] to state[4][4] */
static int HsmHalLoadStateArray(uint8_t (*state)[4], const uint8_t *in) {
    int i,j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            state[j][i] = *in++;
        }
    }
    return 0;
}

/* copy state[4][4] to out[16] */
static int HsmHalStoreStateArray(uint8_t (*state)[4], uint8_t *out) {
    int i,j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            *out++ = state[j][i];
        }
    }
    return 0;
}
//秘钥扩展
static int HsmHalKeyExpansion(const uint8_t *key, uint32_t keyLen, CryptoHalAes_t *aesKey) {
    uint32_t * w ;
    uint32_t * v ;
    int i,j;
    if (NULL == key || NULL == aesKey){
        //printf("keyExpansion param is NULL\n");
        return -1;
    }

    if (keyLen != 16){
        //printf("keyExpansion keyLen = %d, Not support.\n", keyLen);
        return -1;
    }

    w = aesKey->Ek;  //加密秘钥
    v = aesKey->Dk;  //解密秘钥
    /* keyLen is 16 Bytes, generate uint32_t W[44]. */

    /* W[0-3] */
    for (i = 0; i < 4; ++i) {
        LOAD32H(w[i], key + 4*i);
    }

    /* W[4-43] */
    for (i = 0; i < 10; ++i) {
        w[4] = w[0] ^ MIX(w[3]) ^ rcon[i];
        w[5] = w[1] ^ w[4];
        w[6] = w[2] ^ w[5];
        w[7] = w[3] ^ w[6];
        w += 4;
    }

    w = aesKey->Ek + 44 - 4;
    //解密秘钥矩阵为加密秘钥矩阵的倒序，方便使用，把ek的11个矩阵倒序排列分配给dk作为解密秘钥
    //即dk[0-3]=ek[41-44], dk[4-7]=ek[37-40]... dk[41-44]=ek[0-3]
    for (j = 0; j < 11; ++j) {

        for (i = 0; i < 4; ++i) {
            v[i] = w[i];
        }
        w -= 4;
        v += 4;
    }

    return 0;
}

// 轮秘钥加
static int HsmHalAddRoundKey(uint8_t (*state)[4], const uint32_t *key) {
    uint8_t k[4][4];
    int i,j;
    /* i: row, j: col */
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            k[i][j] = (uint8_t) BYTE(key[j], 3 - i);  /* 把 uint32 key[4] 先转换为矩阵 uint8 k[4][4] */
            state[i][j] ^= k[i][j];
        }
    }

    return 0;
}

//字节替换
static int HsmHalSubBytes(uint8_t (*state)[4]) {
    /* i: row, j: col */
    int i,j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            state[i][j] = S[state[i][j]]; //直接使用原始字节作为S盒数据下标
        }
    }

    return 0;
}

//逆字节替换
static int HsmHalInvSubBytes(uint8_t (*state)[4]) {
    /* i: row, j: col */
    int i,j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            state[i][j] = inv_S[state[i][j]];
        }
    }
    return 0;
}

//行移位
static int HsmHalShiftRows(uint8_t (*state)[4]) {
    uint32_t block[4] = {0};

    /* i: row */
    int i;
    for (i = 0; i < 4; ++i) {
    //便于行循环移位，先把一行4字节拼成uint_32结构，移位后再转成独立的4个字节uint8_t
        LOAD32H(block[i], state[i]);
        block[i] = ROF32(block[i], 8*i);
        STORE32H(block[i], state[i]);
    }

    return 0;
}

//逆行移位
static int HsmHalInvShiftRows(uint8_t (*state)[4]) {
    uint32_t block[4] = {0};

    /* i: row */
    int i;
    for (i = 0; i < 4; ++i) {
        LOAD32H(block[i], state[i]);
        block[i] = ROR32(block[i], 8*i);
        STORE32H(block[i], state[i]);
    }

    return 0;
}

/* Galois Field (256) Multiplication of two Bytes */
// 两字节的伽罗华域乘法运算
static uint8_t HsmHalGMul(uint8_t u, uint8_t v) {
    uint8_t p = 0;
    int i;
    int flag;

    for (i = 0; i < 8; ++i) {
        if (u & 0x01) {    //
            p ^= v;
        }

        flag = (v & 0x80);
        v <<= 1;
        if (flag) {
            v ^= 0x1B; /* x^8 + x^4 + x^3 + x + 1 */
        }

        u >>= 1;
    }

    return p;
}

// 列混合
static int HsmHalMixColumns(uint8_t (*state)[4]) {
    uint8_t tmp[4][4];
    int i,j;
    uint8_t M[4][4] = {{0x02, 0x03, 0x01, 0x01},
           {0x01, 0x02, 0x03, 0x01},
           {0x01, 0x01, 0x02, 0x03},
           {0x03, 0x01, 0x01, 0x02}};

    /* copy state[4][4] to tmp[4][4] */
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j){
            tmp[i][j] = state[i][j];
        }
    }

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {  //伽罗华域加法和乘法
            state[i][j] = HsmHalGMul(M[i][0], tmp[0][j]) ^ HsmHalGMul(M[i][1], tmp[1][j])
                        ^ HsmHalGMul(M[i][2], tmp[2][j]) ^ HsmHalGMul(M[i][3], tmp[3][j]);
        }
    }

    return 0;
}

// 逆列混合
static int HsmHalInvMixColumns(uint8_t (*state)[4]) {
    int i,j;
    uint8_t tmp[4][4];
    uint8_t M[4][4] = {{0x0E, 0x0B, 0x0D, 0x09},
                       {0x09, 0x0E, 0x0B, 0x0D},
                       {0x0D, 0x09, 0x0E, 0x0B},
                       {0x0B, 0x0D, 0x09, 0x0E}};  //使用列混合矩阵的逆矩阵

    /* copy state[4][4] to tmp[4][4] */
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j){
            tmp[i][j] = state[i][j];
        }
    }

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            state[i][j] = HsmHalGMul(M[i][0], tmp[0][j]) ^ HsmHalGMul(M[i][1], tmp[1][j])
                          ^ HsmHalGMul(M[i][2], tmp[2][j]) ^ HsmHalGMul(M[i][3], tmp[3][j]);
        }
    }

    return 0;
}


/*************************************************
  Function:       CryptoHalInit
  Description:    HSM接口模块初始化
  Input:          无
  Output:         无
  Return:         无
  Others:         
*************************************************/
void CryptoHalInit(void)
{
    return ;
}

/*************************************************
  Function:       CryptoHalAesEcbInit
  Description:    Aes ecb 计算初始化
  Input:          pAesData ：Aes计算对象
  Output:         无
  Return:         成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:         
*************************************************/
void CryptoHalAesEcbInit(CryptoHalAes_t *pAesData)
{

}

/*************************************************
  Function:       CryptoHalAesEcbEncrypt
  Description:    Aes ecb 加密
  Input:          pKey ：Aes秘钥
                  lengthType：加密长度
                  pSrc：要加密的数据
                  srcSize:数据长度
  Output:         pDest：结果目标块地址
  Return:         成功：HSM_HAL_STATUS_OK
                  失败：HSM_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t CryptoHalAesEcbEncrypt(const uint8_t *pKey,uint8_t lengthType,const uint8_t *pSrc,uint16_t srcSize,uint8_t *pDest)
{
    CryptoHalAes_t aesKey;
    uint8_t *pos = pDest;
    const uint32_t *rk = aesKey.Ek;  //解密秘钥指针
    uint8_t actualKey[16] = {0};
    uint8_t state[4][4] = {0};
    int i,j;
    if (NULL == pKey || NULL == pSrc || NULL == pDest){
        return -1;
    }

    if (lengthType == 0) // AES128
    {
        if (srcSize % BLOCKSIZE)
        {
            return -1;
        }

        //memcpy(actualKey, pKey, 16);
        for(i = 0;i<16;i++)
        {
            actualKey[i] = pKey[i];
        }
        HsmHalKeyExpansion(actualKey, 16, &aesKey);  // 秘钥扩展

        // 使用ECB模式循环加密多个分组长度的数据
        for (i = 0; i < srcSize; i += BLOCKSIZE) {
            HsmHalLoadStateArray(state, pSrc);
            HsmHalAddRoundKey(state, rk);

            for ( j = 1; j < 10; ++j) {
                rk += 4;
                HsmHalSubBytes(state);   // 字节替换
                HsmHalShiftRows(state);  // 行移位
                HsmHalMixColumns(state); // 列混合
                HsmHalAddRoundKey(state, rk); // 轮秘钥加
            }

            HsmHalSubBytes(state);    // 字节替换
            HsmHalShiftRows(state);  // 行移位
            // 此处不进行列混合
            HsmHalAddRoundKey(state, rk+4); // 轮秘钥加

            // 把4x4状态矩阵转换为uint8_t一维数组输出保存
            HsmHalStoreStateArray(state, pos);

            pos += BLOCKSIZE;  // 加密数据内存指针移动到下一个分组
            pSrc += BLOCKSIZE;   // 明文数据指针移动到下一个分组
            rk = aesKey.Ek;    // 恢复rk指针到秘钥初始位置
        }
    }
    else if(lengthType == 1)   //AES 192
    {

    }
    else if(lengthType == 2)   //AES 256
    {
    
    }
    else
    {
        return -1;
    }
    return 0;
}

/*************************************************
  Function:       CryptoHalAesEcbDecrypt
  Description:    Aes ecb解密
  Input:          pKey ：Aes秘钥
                  lengthType：加密长度
                  pSrc：要解密的数据
                  srcSize:数据长度
  Output:         pDest：解密结果数据
  Return:         成功：HSM_HAL_STATUS_OK
                  失败：HSM_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t CryptoHalAesEcbDecrypt(const uint8_t *pKey,uint8_t lengthType,const uint8_t *pSrc,uint16_t srcSize,uint8_t *pDest)
{
    CryptoHalAes_t aesKey;
    uint8_t *pos = pDest;
    const uint32_t *rk = aesKey.Dk;  //解密秘钥指针
    //uint8_t out[BLOCKSIZE] = {0};
    uint8_t actualKey[16] = {0};
    uint8_t state[4][4] = {0};
    int i,j;

    if (NULL == pKey || NULL == pSrc || NULL == pDest){
        return -1;
    }

    if (lengthType == 0) // AES128
    {
        if (srcSize % BLOCKSIZE)
        {
            return -1;
        }
        //memcpy(actualKey, pKey, 16);
        for(i = 0;i<16;i++)
        {
            actualKey[i] = pKey[i];
        }
        HsmHalKeyExpansion(actualKey, 16, &aesKey);  //秘钥扩展，同加密

        for (i = 0; i < srcSize; i += BLOCKSIZE) {
            // 把16字节的密文转换为4x4状态矩阵来进行处理
            HsmHalLoadStateArray(state, pSrc);
            // 轮秘钥加，同加密
            HsmHalAddRoundKey(state, rk);

            for (j = 1; j < 10; ++j) {
                rk += 4;
                HsmHalInvShiftRows(state);    // 逆行移位
                HsmHalInvSubBytes(state);     // 逆字节替换，这两步顺序可以颠倒
                HsmHalAddRoundKey(state, rk); // 轮秘钥加，同加密
                HsmHalInvMixColumns(state);   // 逆列混合
            }

            HsmHalInvSubBytes(state);   // 逆字节替换
            HsmHalInvShiftRows(state);  // 逆行移位
            // 此处没有逆列混合
            HsmHalAddRoundKey(state, rk+4);  // 轮秘钥加，同加密

            HsmHalStoreStateArray(state, pos);  // 保存明文数据
            pos += BLOCKSIZE;  // 输出数据内存指针移位分组长度
            pSrc += BLOCKSIZE;   // 输入数据内存指针移位分组长度
            rk = aesKey.Dk;    // 恢复rk指针到秘钥初始位置
        }
    }
    else if(lengthType == 1)   //AES 192
    {

    }
    else if(lengthType == 2)   //AES 256
    {
    
    }
    else
    {
        return -1;
    }
    return 0;
}

/*************************************************
  Function:       CryptoHalAesCmacInit
  Description:    Cmac数据初始化
  Input:          pAesData：Aes计算对象
  Output:         执行结果
  Return:         无
  Others:         
*************************************************/
void CryptoHalAesCmacInit(CryptoHalAes_t *pAesData)
{

}

/**************************************************/

void CryptoHalArrayXor(uint16_t len,uint8_t *a1,uint8_t *a2,uint8_t *des)
{
    uint16_t i = 0;
    for(i = 0;i< len;i++)
    {
        des[i] = a1[i] ^ a2[i];
    }

    return ;
}


static void HsmHalLeftShiftOneBit(uint8_t *input,uint8_t *output)
{
    int i;
    uint8_t overflow = 0;
    for ( i=15; i>=0; i-- )
    {
        output[i] = input[i] << 1;
        output[i] |= overflow;
        overflow =(input[i] & 0x80)?1:0;
    }
    return;
}
static void HsmHalGenerateSubkey(uint8_t *key, uint8_t *K1, uint8_t *K2)
{
    uint8_t Rb[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x87};
    uint8_t L[16];
    uint8_t Z[16];

    uint8_t tmp[16];
    int i;
    for ( i=0; i<16; i++)
    {
        Z[i] = 0;
    }
    CryptoHalAesEcbEncrypt(key,0,Z,16,L);
    if((L[0] & 0x80)==0)
    {
        HsmHalLeftShiftOneBit(L,K1);
    }
    else
    {
        HsmHalLeftShiftOneBit(L,tmp);
        CryptoHalArrayXor(16,tmp,Rb,K1);
    }
    if((K1[0]& 0x80)== 0)
    {
        HsmHalLeftShiftOneBit(K1,K2);
    }
    else
    {
        HsmHalLeftShiftOneBit(K1,tmp);
        CryptoHalArrayXor(16,tmp,Rb,K2);
    }
    return;

}
static void padding ( uint8_t *lastb,uint8_t *pad, uint32_t length )
{
    uint8_t i = 0;
    for (i=0; i<16; i++ )
    {
        if(i < length)
        {
            pad[i] = lastb[i];
        }
        else if(i == length )
        {
            pad[i] = 0x80;
        }
        else
        {
            pad[i] = 0x00;
        }
    }
}

/*************************************************
  Function:       CryptoHalAesCmacEncrypt
  Description:    计算cmac值
  Input:          pKey ：Aes秘钥
                  lengthType：加密长度
                  pSrc：要加密的数据
                  srcSize：要加密数据长度
  Output:         pDest：加密结果数据
  Return:         成功：HSM_HAL_STATUS_OK
                  失败：HSM_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t CryptoHalAesCmacEncrypt(uint8_t *pKey,uint8_t lengthType,uint8_t *pSrc,uint32_t srcSize,uint8_t *pDest)
{
    uint8_t X[16],Y[16], M_last[16], padded[16];
    uint8_t K1[16], K2[16];
    int n, i, flag;

    HsmHalGenerateSubkey(pKey,K1,K2);
    n = (srcSize+15)/ 16;
    if(n== 0)
    {
        n = 1;
        flag = 0;
    }
    else
    {
        if((srcSize%16)== 0)
        {
            flag = 1;
        }
        else
        {
            flag = 0;
        }
    }
    if( flag )
    {
        CryptoHalArrayXor(16,&pSrc[16*(n-1)],K1,M_last);
    }
    else
    {
        padding(&pSrc[16*(n-1)],padded,srcSize%16);
        CryptoHalArrayXor(16,padded,K2,M_last);
    }
    for ( i = 0;i < 16; i++ )
    {
        X[i] = 0;
    }
    for ( i=0; i<n-1; i++ )
    {
        CryptoHalArrayXor(16,X,&pSrc[16*i],Y);
        CryptoHalAesEcbEncrypt(pKey,0,Y,16,X);
    }
    CryptoHalArrayXor(16,X,M_last,Y);

    CryptoHalAesEcbEncrypt(pKey,0,Y,16,X);
    for ( i=0;i < 16; i++ )
    {
        pDest[i] = X[i];
    }
    return 0;
}

/*************************************************
  Function:       CryptoHalAesGetRandomData
  Description:    获取随机数
  Input:          无
  Output:         pRandomData ：生成的随机数
  Return:         无
  Others:         
*************************************************/
void CryptoHalAesGetRandomData(uint32_t *pRandomData)
{
    uint32_t randData;
    uint8_t i = 0;
    static uint32_t j = 0;
    uint8_t *data = (uint8_t *)pRandomData;
    randData = (time(NULL));//RTOS_HalGetTickCount();
    srand(randData + j++);
    
    for (i = 0; i < 4; i++)
    {
        data[i] = rand();
    }
    return ;
}


#else
/*************************************************
  Function:       CryptoHalInit
  Description:    HSM接口模块初始化
  Input:          无
  Output:         无
  Return:         无
  Others:         
*************************************************/
// static cy_stc_crypto_context_t        g_cryptoCtx;

void CryptoHalInit(void)
{
    /* AES bus clock is always enabled on AC7842x platform (via HSM peripheral) */
}

/*************************************************
  Function:       CryptoHalAesEcbInit
  Description:    Aes ecb 计算初始化
  Input:          pAesData ：Aes计算对象
  Output:         无
  Return:         成功：FLASH_HAL_STATUS_OK
                  失败：FLASH_HAL_STATUS_ERR
  Others:         
*************************************************/
void CryptoHalAesEcbInit(CryptoHalAes_t *pAesData)
{

}

/*************************************************
  Function:       CryptoHalAesEcbEncrypt
  Description:    Aes ecb 加密
  Input:          pKey ：Aes秘钥
                  lengthType：加密长度
                  pSrc：要加密的数据
                  srcSize:数据长度
  Output:         pDest：结果目标块地址
  Return:         成功：HSM_HAL_STATUS_OK
                  失败：HSM_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t CryptoHalAesEcbEncrypt(const uint8_t *pKey,uint8_t lengthType,const uint8_t *pSrc,uint16_t srcSize,uint8_t *pDest)
{
#ifdef AES_SDK_V2
    uint32_t key[4] = {0};
    uint32_t inData[4] = {0};
    AES_CryptConfig_t cryptConfig = {
        .cryptAlg = AES_ALG_AES,      /*!< cryptographic algorithm type */
        .cryptOptMode = AES_ECB_INIT,    /*!< operation mode */
        .cryptOpt = AES_ENCRYPTION,       /*!< cryptographic operation type */
        .blockSize = 0x80
    };
    AES_StatusError_t err = AES_NO_ERR;
    uint32_t outData[4] = {0};
    //CryptoHalAes_t aesKey;
    uint8_t *pos = pDest;
    int i;

    if (NULL == pKey || NULL == pSrc || NULL == pDest){
        return -1;
    }

    if (lengthType == 0) // AES128
    {
        if (srcSize % BLOCKSIZE)
        {
            return -1;
        }

#if 0
        memcpy(key, pKey, 16);   
        // 传入key：key[16]={0x0f,0x15,0x71,0xc9,0x47,0xd9,0xe8,0x59,0x0c,0xb7,0xad,0xd6,0xaf,0x7f,0x67,0x98};
        // key[0] = 0xc971150f
        // 实际key: 0x98,0x67,0x7f,0xaf,0xd6,0xad,0xb7,0x0c,0x59,0xe8,0xd9,0x47,0xc9,0x71,0x15,0x0f
#else
        key[3] = (pKey[0] << 24) + (pKey[1] << 16) + (pKey[2] << 8) + pKey[3];
        key[2] = (pKey[4] << 24) + (pKey[5] << 16) + (pKey[6] << 8) + pKey[7];
        key[1] = (pKey[8] << 24) + (pKey[9] << 16) + (pKey[10] << 8) + pKey[11];
        key[0] = (pKey[12] << 24) + (pKey[13] << 16) + (pKey[14] << 8) + pKey[15];
#endif
        AES_SetKey(key);
        // 使用ECB模式循环加密多个分组长度的数据
        for (i = 0; i < srcSize; i += BLOCKSIZE) {
            /* Encrypt 128bit x 4 plain text */

            if(i != 0)
            {
                cryptConfig.cryptOptMode = AES_ECB_UPDATE;
            }
#if 0
            memcpy(inData, pSrc, 16);   
            // 传入明文：input[16] = {0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10};
            // inData[0] = 0x67452301
            // 实际明文：0x10,0x32,0x54,0x76,0x98,0xba,0xdc,0xfe,0xef,0xcd,0xab,0x89,0x67,0x45,0x23,0x01
#else
            inData[3] = (pSrc[0] << 24) + (pSrc[1] << 16) + (pSrc[2] << 8) + pSrc[3];
            inData[2] = (pSrc[4] << 24) + (pSrc[5] << 16) + (pSrc[6] << 8) + pSrc[7];
            inData[1] = (pSrc[8] << 24) + (pSrc[9] << 16) + (pSrc[10] << 8) + pSrc[11];
            inData[0] = (pSrc[12] << 24) + (pSrc[13] << 16) + (pSrc[14] << 8) + pSrc[15];
#endif
            AES_SetInputData(inData);
            AES_CryptConfig(&cryptConfig);
            while(AES_StartOpt(FOURBITRAND) != 0);
            while(AES_GetOptStatus(&err) == 0);
            AES_GetOutputData(outData);
#if 0
            memcpy(pos, outData, 16);   
            // outData[0] = 0x681F2EC7
            // 实际加密结果 904044C59D45CAF58707AAE3681F2EC7
            // 返回打印 c7 2e 1f 68 e3 aa 07 87 f5 ca 45 9d c5 44 40 90
            pos += BLOCKSIZE;  // 加密数据内存指针移动到下一个分组
#else
            for(uint8_t i = 0; i < 4; i++)
            {
                *pos++ = (outData[3 - i] >> 24) & 0xFF;
                *pos++ = (outData[3 - i] >> 16) & 0xFF;
                *pos++ = (outData[3 - i] >> 8) & 0xFF;
                *pos++ = outData[3 - i] & 0xFF;
            }
#endif
            pSrc += BLOCKSIZE;   // 明文数据指针移动到下一个分组
        }
    }
    else
    {
        return -1;
    }
#endif
    return 0;
}

/*************************************************
  Function:       CryptoHalAesEcbDecrypt
  Description:    Aes ecb解密
  Input:          pKey ：Aes秘钥
                  lengthType：加密长度
                  pSrc：要解密的数据
                  srcSize:数据长度
  Output:         pDest：解密结果数据
  Return:         成功：HSM_HAL_STATUS_OK
                  失败：HSM_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t CryptoHalAesEcbDecrypt(const uint8_t *pKey,uint8_t lengthType,const uint8_t *pSrc,uint16_t srcSize,uint8_t *pDest)
{
#ifdef AES_SDK_V2
    uint32_t key[4] = {0};
    uint32_t inData[4] = {0};
    AES_CryptConfig_t cryptConfig = {
        .cryptAlg = AES_ALG_AES,      /*!< cryptographic algorithm type */
        .cryptOptMode = AES_ECB_INIT,    /*!< operation mode */
        .cryptOpt = AES_DECRYPTION,       /*!< cryptographic operation type */
        .blockSize = 0x80
    };
    AES_StatusError_t err = AES_NO_ERR;
    uint32_t outData[4] = {0};
    //CryptoHalAes_t aesKey;
    uint8_t *pos = pDest;
    int i;

    if (NULL == pKey || NULL == pSrc || NULL == pDest){
        return -1;
    }

    if (lengthType == 0) // AES128
    {
        if (srcSize % BLOCKSIZE)
        {
            return -1;
        }

        key[3] = (pKey[0] << 24) + (pKey[1] << 16) + (pKey[2] << 8) + pKey[3];
        key[2] = (pKey[4] << 24) + (pKey[5] << 16) + (pKey[6] << 8) + pKey[7];
        key[1] = (pKey[8] << 24) + (pKey[9] << 16) + (pKey[10] << 8) + pKey[11];
        key[0] = (pKey[12] << 24) + (pKey[13] << 16) + (pKey[14] << 8) + pKey[15];
        AES_SetKey(key);
        // 使用ECB模式循环解密多个分组长度的数据
        for (i = 0; i < srcSize; i += BLOCKSIZE) {
            /* Encrypt 128bit x 4 plain text */

            if(i != 0)
            {
                cryptConfig.cryptOptMode = AES_ECB_UPDATE;
            }
            inData[3] = (pSrc[0] << 24) + (pSrc[1] << 16) + (pSrc[2] << 8) + pSrc[3];
            inData[2] = (pSrc[4] << 24) + (pSrc[5] << 16) + (pSrc[6] << 8) + pSrc[7];
            inData[1] = (pSrc[8] << 24) + (pSrc[9] << 16) + (pSrc[10] << 8) + pSrc[11];
            inData[0] = (pSrc[12] << 24) + (pSrc[13] << 16) + (pSrc[14] << 8) + pSrc[15];
            AES_SetInputData(inData);
            AES_CryptConfig(&cryptConfig);
            while(AES_StartOpt(FOURBITRAND) != 0);
            while(AES_GetOptStatus(&err) == 0);
            AES_GetOutputData(outData);
            for(uint8_t i = 0; i < 4; i++)
            {
                *pos++ = (outData[3 - i] >> 24) & 0xFF;
                *pos++ = (outData[3 - i] >> 16) & 0xFF;
                *pos++ = (outData[3 - i] >> 8) & 0xFF;
                *pos++ = outData[3 - i] & 0xFF;
            }
            pSrc += BLOCKSIZE;   // 明文数据指针移动到下一个分组
        }
    }
    else
    {
        return -1;
    }
#endif
    return 0;
}

/*************************************************
  Function:       CryptoHalAesCmacInit
  Description:    Cmac数据初始化
  Input:          pAesData：Aes计算对象
  Output:         执行结果
  Return:         无
  Others:         
*************************************************/
void CryptoHalAesCmacInit(CryptoHalAes_t *pAesData)
{

}

/*************************************************
  Function:       CryptoHalAesCmacEncrypt
  Description:    计算cmac值
  Input:          pKey ：Aes秘钥
                  lengthType：加密长度
                  pSrc：要加密的数据
                  srcSize：要加密数据长度
  Output:         pDest：加密结果数据
  Return:         成功：HSM_HAL_STATUS_OK
                  失败：HSM_HAL_STATUS_ERR
  Others:         
*************************************************/
int16_t CryptoHalAesCmacEncrypt(const uint8_t *pKey,uint8_t lengthType,uint8_t *pSrc,uint32_t srcSize,uint8_t *pDest)
{
#ifdef AES_SDK_V2
    uint32_t key[4] = {0};
    uint32_t inData[4] = {0};
    AES_CryptConfig_t cryptConfig = {
        .cryptAlg = AES_ALG_AES,      /*!< cryptographic algorithm type */
        .cryptOptMode = AES_CMAC_INTI,    /*!< operation mode */
        .cryptOpt = AES_ENCRYPTION,       /*!< cryptographic operation type */
        .blockSize = 0x80
    };
    AES_StatusError_t err = AES_NO_ERR;
    uint32_t outData[4] = {0};
    //CryptoHalAes_t aesKey;
    uint8_t *pos = pDest;
    int i;

    if (NULL == pKey || NULL == pSrc || NULL == pDest){
        return -1;
    }

    if (lengthType == 0) // AES128
    {
        if (srcSize % BLOCKSIZE)
        {
            return -1;
        }

        // 1. Write Key vector to AES_KEY0 – AES_KEY3
        key[3] = (pKey[0] << 24) + (pKey[1] << 16) + (pKey[2] << 8) + pKey[3];
        key[2] = (pKey[4] << 24) + (pKey[5] << 16) + (pKey[6] << 8) + pKey[7];
        key[1] = (pKey[8] << 24) + (pKey[9] << 16) + (pKey[10] << 8) + pKey[11];
        key[0] = (pKey[12] << 24) + (pKey[13] << 16) + (pKey[14] << 8) + pKey[15];
        AES_SetKey(key);
        // 2. Configure AES_CONFIG
        AES_CryptConfig(&cryptConfig);
        // 3. Reseed and start operation by writing AES_CONTROL
        while(AES_StartOpt(FOURBITRAND) != 0);
        // 4. Poll AES_STATUS until the operation is completed
        while(AES_GetOptStatus(&err) == 0);

        // 循环加密多个分组长度的数据
        for (i = 0; i < srcSize; i += BLOCKSIZE) {
            // 5. Write data block i to AES_DATA_I0 - AES_DATA_I3
            inData[3] = (pSrc[0] << 24) + (pSrc[1] << 16) + (pSrc[2] << 8) + pSrc[3];
            inData[2] = (pSrc[4] << 24) + (pSrc[5] << 16) + (pSrc[6] << 8) + pSrc[7];
            inData[1] = (pSrc[8] << 24) + (pSrc[9] << 16) + (pSrc[10] << 8) + pSrc[11];
            inData[0] = (pSrc[12] << 24) + (pSrc[13] << 16) + (pSrc[14] << 8) + pSrc[15];
            AES_SetInputData(inData);
            // 6. Configure AES_CONFIG
            cryptConfig.cryptOptMode = AES_CMAC_UPDATE;
            AES_CryptConfig(&cryptConfig);
            // 7. Reseed and start operation by writing AES_CONTROL
            while(AES_StartOpt(FOURBITRAND) != 0);
            // 8. Poll AES_STATUS until the operation is completed
            while(AES_GetOptStatus(&err) == 0);
            // 9. Repeat step 5 through step 8 until the last data block remains
            pSrc += BLOCKSIZE;   // 明文数据指针移动到下一个分组
        }
        // 10. Configure AES_CONFIG
        cryptConfig.cryptOptMode = AES_CMAC_FINISH_K2;
        AES_CryptConfig(&cryptConfig);
        // 11. Reseed and start operation by writing AES_CONTROL
        while(AES_StartOpt(FOURBITRAND) != 0);
        // 12. Poll AES_STATUS until the operation is completed
        while(AES_GetOptStatus(&err) == 0);
        // 13. Read mac from AES_DATA_O0 - AES_DATA_O3
        AES_GetOutputData(outData);
        for(uint8_t i = 0; i < 4; i++)
        {
            *pos++ = (outData[3 - i] >> 24) & 0xFF;
            *pos++ = (outData[3 - i] >> 16) & 0xFF;
            *pos++ = (outData[3 - i] >> 8) & 0xFF;
            *pos++ = outData[3 - i] & 0xFF;
        }
    }
    else
    {
        return -1;
    }
#endif
    return 0;
}

/*************************************************
  Function:       CryptoHalAesGetRandomData
  Description:    获取随机数
  Input:          无
  Output:         pRandomData ：生成的随机数
  Return:         无
  Others:         
*************************************************/
void CryptoHalAesGetRandomData(uint32_t *pRandomData)
{
    uint32_t randData;
    uint8_t i = 0;
    static uint32_t j = 0;
    uint8_t *data = (uint8_t *)pRandomData;
    randData = (time(NULL));    //xTaskGetTickCount();
    srand(randData + j++);
    
    for (i = 0; i < 4; i++)
    {
        data[i] = rand() % 256;
    }
}


#endif

#if  1
//test main
void CryptoHalTestMain(void)
{
    uint8_t input[16] = {0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10};
    const uint8_t key[16]={0x0f,0x15,0x71,0xc9,0x47,0xd9,0xe8,0x59,0x0c,0xb7,0xad,0xd6,0xaf,0x7f,0x67,0x98};
    uint8_t result[16] = {0xff, 0x0b ,0x84 ,0x4a ,0x08 ,0x53 ,0xbf ,0x7c ,0x69 ,0x34 ,0xab ,0x43 ,0x64 ,0x14 ,0x8f ,0xb9};
    uint8_t output1[16] = {0};
    uint8_t output2[16] = {0};
    uint8_t output3[16] = {0};
    uint16_t i = 0;
    static uint8_t count = 0;

    if(count++ < 10)
    {
        return;
    }
    count = 0;

    CryptoHalAesEcbEncrypt(key,0,input,16,output1);
    TBOX_PRINT("EcbEncrypt: ");
    for(i = 0;i <16;i++)
    {
        TBOX_PRINT("%02x ",output1[i]);
    }
    TBOX_PRINT("\r\n");

    CryptoHalAesEcbDecrypt(key,0,result,16,output2);
    __disable_irq();
    TBOX_PRINT("EcbDecrypt: ");
    for(i = 0;i <16;i++)
    {
        TBOX_PRINT("%02x ",output2[i]);
    }
    TBOX_PRINT("\r\n");
    __enable_irq();

    uint8_t key3[16] = {0x3d, 0x2e, 0x6d, 0xe2, 0xa1, 0x25, 0x17, 0xba,0xc5, 0xb3, 0x1b, 0xbd, 0x0e, 0x7e, 0x3b, 0x54};
    uint8_t input3[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
    //uint8_t result3[16] = {0xa5 ,0x1e ,0xb8 ,0x07 ,0xbc ,0x44 ,0x7f ,0x92 ,0xfa ,0x33 ,0xb5 ,0x69 ,0x9a ,0xcb ,0x4c ,0xdf};
    CryptoHalAesCmacEncrypt(key3,0,input3,16,output3);
    
    __disable_irq();
    TBOX_PRINT("CmacEncrypt: ");
    for(i = 0;i <16;i++)
    {
        TBOX_PRINT("%02x ",output3[i]);
    }
    TBOX_PRINT("\r\n");
    __enable_irq();
}
#endif
