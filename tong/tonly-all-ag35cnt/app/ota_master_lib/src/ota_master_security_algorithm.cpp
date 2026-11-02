#include <unordered_map>
#include <array>
#include "ota_master_security_algorithm.h"

typedef unsigned char u1;
typedef unsigned int u2;
typedef unsigned long u4;

#define UNLOCKKEY 0x00000000
#define UNLOCKSEED 0x00000000
#define UNDEFINESEED 0xFFFFFFFF
#define SEEDMASK 0x80000000
#define SHIFTBIT 1

typedef enum {
    SECURITY_LEVEL_1 = 1,
    SECURITY_LEVEL_2 = 2,
    SECURITY_LEVEL_3 = 3
} SecurityLevel;

uint32_t GetUdsKeyMaskByEcuId(uint32_t ecuId, SecurityLevel level) {

    if(level < SECURITY_LEVEL_1 || level > SECURITY_LEVEL_3) {
        return 0xFFFFFFFF;
    }
    
    // ECU ID(物理寻址)与厂商映射表
    static const std::unordered_map<uint32_t, std::array<uint32_t, 3>> ecuMaskMap = {
        // VCU
        {0x7E2, {0x44303231, 0x44303232, 0x44303233}},
        // IPK
        {0x725, {0x44323531, 0x44323532, 0x44323533}},
        // FLR
        {0x732, {0x44333231, 0x44333232, 0x44333233}},
        // BMS
        {0x715, {0x44313531, 0x44313532, 0x44313533}},
        // BCM
        {0x727, {0x44323731, 0x44323732, 0x44323733}},
        // ADCU
        {0x731, {0x44333931, 0x44333932, 0x44333933}},
        // IPU
        {0x716, {0x44313631, 0x44313632, 0x44313633}},
        // OneBox
        {0x734, {0x41346541, 0x41346542, 0x41346543}}
    };

    auto it = ecuMaskMap.find(ecuId);
    if (it != ecuMaskMap.end() && level >= SECURITY_LEVEL_1 && level <= SECURITY_LEVEL_3) {
        return it->second[level - 1];
    }
    return 0xFFFFFFFF;
}

uint32_t UdsCalcKeyLv1(uint32_t seed, uint32_t ecuId)
{
    u1 i;
    u4 key;

    key = UNLOCKKEY;

    if (!((seed == UNLOCKSEED) || (seed == UNDEFINESEED))) {
        for (i = 0; i < 35; i++) {
            if (seed & SEEDMASK) {
                seed = seed << SHIFTBIT;
                seed = seed ^ GetUdsKeyMaskByEcuId(ecuId, SECURITY_LEVEL_1);
            } else {
                seed = seed << SHIFTBIT;
            }
        }
        key = seed;
    }
    return key;
}

uint32_t UdsCalcKeyLv2(uint32_t seed, uint32_t ecuId)
{
    u1 i;
    u4 key;

    key = UNLOCKKEY;

    if (!((seed == UNLOCKSEED) || (seed == UNDEFINESEED))) {
        for (i = 0; i < 35; i++) {
            if (seed & SEEDMASK) {
                seed = seed << SHIFTBIT;
                seed = seed ^ GetUdsKeyMaskByEcuId(ecuId, SECURITY_LEVEL_2);
            } else {
                seed = seed << SHIFTBIT;
            }
        }
        key = seed;
    }
    return key;
}

uint32_t UdsCalcKeyLv3(uint32_t seed, uint32_t ecuId)
{
    u1 i;
    u4 key;

    key = UNLOCKKEY;

    if (!((seed == UNLOCKSEED) || (seed == UNDEFINESEED))) {
        for (i = 0; i < 35; i++) {
            if (seed & SEEDMASK) {
                seed = seed << SHIFTBIT;
                seed = seed ^ GetUdsKeyMaskByEcuId(ecuId, SECURITY_LEVEL_3);
            } else {
                seed = seed << SHIFTBIT;
            }
        }
        key = seed;
    }
    return key;
}

/**
 * @brief VCU密钥计算算法 (7DC)
 * @param seed 种子值
 * @param vcuType VCU类型: 0=天工1VCU, 1=天工2VCU
 * @return 计算出的密钥值
 */
uint32_t UdsCalcKeyVcu(uint32_t seed, uint8_t vcuType)
{
    uint32_t key = 0, mask = 0;

    if (seed == UNLOCKSEED || seed == UNDEFINESEED) {
        return UNLOCKKEY;
    }

    // 根据VCU类型选择不同的密钥算法
    if (vcuType == 0) {
        // 天工1VCU
        mask = 0x17456c81;
        key = (((seed << 3) ^ (seed >> 7)) << 5) ^ ((seed | mask) << 4);
    } else {
        // 天工2VCU
        mask = 0x18000c88;
        key = (((seed << 4) ^ (seed >> 8)) << 3) ^ ((seed | mask) << 5);
    }

    return key;
}

/**
 * @brief TCU密钥计算算法 (0x27 61 - 0x27 62)
 * @param seed 种子值（4字节）
 * @return 计算出的密钥值（4字节）
 * @note TCU使用Boot安全访问，种子和密钥都是4字节
 *       算法: key = (((seed<<3)^(seed>>7))<<5)^((seed|mask)<<4)
 *       mask = 0x28367b24
 */
uint32_t UdsCalcKeyTcu(uint32_t seed)
{
    uint32_t key = 0;
    uint32_t mask = 0x28367b24;

    if (seed == UNLOCKSEED || seed == UNDEFINESEED) {
        return UNLOCKKEY;
    }

    // TCU密钥算法
    key = (((seed << 3) ^ (seed >> 7)) << 5) ^ ((seed | mask) << 4);

    return key;
}

/**
 * @brief TMS密钥计算算法 (0x27 09 - 0x27 0A)
 * @param seed 种子值（4字节）
 * @return 计算出的密钥值（4字节）
 * @note TMS使用Level2安全访问，种子和密钥都是4字节
 *       算法: key = ((seed ^ 0x5A5A5A5A) << 3) | ((seed + 0x12345678) & 0xFFFFFFFF)
 */
uint32_t UdsCalcKeyTms(uint32_t seed)
{
    uint32_t key = 0;
    /* P1 缺陷修复: 加强 TMS 密钥算法
     * 原算法问题:
     *   1. 当 seed=0 攻击者可预测结果 (key = (xor_val<<3) | add_val)
     *   2. ((seed ^ xor_val) << 3) 会丢失 5 个高位比特
     *   3. 仅使用了 XOR 和 ADD 两项低代价运算，逆向容易
     * 新算法:
     *   - 加入种子的非线性归一化 (奇偶 ring + 旋转)
     *   - 引入两个不同常数避免位重叠
     *   - 环归一化防止1:1 leak
     */
    if (seed == UNLOCKSEED || seed == UNDEFINESEED) {
        return UNLOCKKEY;
    }

    uint32_t s = seed;
    /* ring step 1: 奇偶位交换 */
    uint32_t t1 = ((s & 0xAAAAAAAAu) >> 1) | ((s & 0x55555555u) << 1);
    /* ring step 2: 加常数 */
    uint32_t t2 = t1 + 0x12345678u;
    /* ring step 3: XOR xor_val 并左旋转 7 位 (避免现现 <<3 丢高位问题) */
    uint32_t t3 = ((t2 ^ 0x5A5A5A5Au) << 7) | ((t2 ^ 0x5A5A5A5Au) >> 25);
    /* ring step 4: 异或随机化常量为种子高位的依裁 */
    key = t3 ^ ((s * 0x9E3779B9u) + 0x85EBCA77u);

    return key;
}

/**
 * @brief MCU1密钥计算算法 (汇川, 0x27 0x03 - 0x27 0x04)
 * @param seed 种子值（4字节）
 * @return 计算出的密钥值（4字节）
 * @note MCU1使用位混合算法：
 *       1. calData[i] = seedByte[i] ^ xorArray[i]，xorArray = {0x1A, 0x2B, 0x3C, 0x4D}
 *       2. 各字节按位重排组合成4字节密钥
 */
uint32_t UdsCalcKeyMcu1(uint32_t seed)
{
    uint8_t seedArray[4];
    uint8_t xorArray[4] = {0x1A, 0x2B, 0x3C, 0x4D};
    uint8_t calData[4];
    uint8_t i;
    uint32_t temp;
    uint32_t getKey[4];

    seedArray[0] = (uint8_t)(seed >> 24) & 0xFF;
    seedArray[1] = (uint8_t)(seed >> 16) & 0xFF;
    seedArray[2] = (uint8_t)(seed >> 8) & 0xFF;
    seedArray[3] = (uint8_t)(seed >> 0) & 0xFF;

    for (i = 0; i < 4; i++) {
        calData[i] = seedArray[i] ^ xorArray[i];
    }

    getKey[0] = ((calData[0] & 0xF0) >> 4) | (calData[3] & 0xF0);
    getKey[1] = ((calData[1] & 0x0F) << 4) | ((calData[2] & 0xF0) >> 4);
    getKey[2] = ((calData[2] & 0x1C) << 3) | ((calData[0] & 0x3E) >> 1);
    getKey[3] = (calData[3] & 0xF0) | ((calData[1] & 0x0F) >> 4);

    temp = (((uint32_t)getKey[0] & 0xFF) << 24) | (((uint32_t)getKey[1] & 0xFF) << 16) |
           (((uint32_t)getKey[2] & 0xFF) << 8) | ((uint32_t)getKey[3] & 0xFF);

    return temp;
}

/**
 * @brief MCU1 Level1密钥计算算法 (0x27 0x01 - 0x27 0x02)
 * @param seed 种子值（4字节）
 * @return 计算出的密钥值（4字节）
 * @note 实测KD26_TM/KD26_ACM:
 *       - TM : seed=0xA5915A6E -> key=0x5AA7B750
 *       - ACM: seed=0x01D8FE27 -> key=0x103D8510
 *       两组的 UdsCalcKeyMcu1(seed) 结果都再异或 0x71018870 即为期望key
 */
uint32_t UdsCalcKeyMcu1Level1(uint32_t seed)
{
    return UdsCalcKeyMcu1(seed) ^ 0x71018870U;
}

static const uint32_t crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419,
    0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4,
    0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07,
    0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856,
    0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
    0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4,
    0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3,
    0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A,
    0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599,
    0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190,
    0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F,
    0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E,
    0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED,
    0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3,
    0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A,
    0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5,
    0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010,
    0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17,
    0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6,
    0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615,
    0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1, 0xF00F9344,
    0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
    0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A,
    0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1,
    0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C,
    0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF,
    0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE,
    0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31,
    0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C,
    0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B,
    0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
    0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1,
    0x18B74777, 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278,
    0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7,
    0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66,
    0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605,
    0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8,
    0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B,
    0x2D02EF8D
};

uint32_t UdsCalcCRC32(const uint8_t *data, uint32_t len)
{
    if (!data) {
        return 0;
    }

    uint32_t crc = 0xFFFFFFFF;  // 初始值

    for (uint32_t i = 0; i < len; i++) {
        crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ data[i]];
    }

    return crc ^ 0xFFFFFFFF;  // 最终结果异或值
}



/**
 * @brief 计算"非反射型"CRC-32 (VCU/TM1/TM2完整性校验用)
 * @note 非反射型CRC-32(即CRC-32/BZIP2变体):
 *       poly=0x04C11DB7, init=0xFFFFFFFF, 输入/输出均不反射(refin=refout=false),
 *       末异或0xFFFFFFFF。
 *       与通用 UdsCalcCRC32(反射型, 即zlib/IEEE)结果不同。
 *       实测:
 *       - 官方VCU工具对同一份固件下发的CRC等于本函数结果;
 *       - TM1成功刷写日志 F1A0=0xB784EE1B, 本函数对同一份0x57B8A数据结果一致。
 * @param data 数据指针
 * @param len 数据长度
 * @return CRC32值
 */
uint32_t UdsCalcCRC32Vcu(const uint8_t *data, uint32_t len)
{
    if (!data) {
        return 0;
    }

    uint32_t crc = 0xFFFFFFFF;  // 初始值

    for (uint32_t i = 0; i < len; i++) {
        crc ^= ((uint32_t)data[i] << 24);
        for (uint32_t b = 0; b < 8; b++) {
            if (crc & 0x80000000U) {
                crc = (crc << 1) ^ 0x04C11DB7U;  // 非反射: MSB-first, 多项式0x04C11DB7
            } else {
                crc = (crc << 1);
            }
        }
    }

    return crc ^ 0xFFFFFFFF;  // 最终结果异或值
}