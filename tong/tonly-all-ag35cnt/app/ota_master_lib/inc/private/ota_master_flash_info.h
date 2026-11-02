#ifndef __OTA_MASTER_FLASH_INFO_H__
#define __OTA_MASTER_FLASH_INFO_H__
#include <stdint.h>

#define SUPPORT_MAX_SEGMENT (100) // 最大闪存段个数
#define FLASH_DOWNLOAD_BLOCK_SIZE (0x10000U) // 单次UDS下载块大小(64KB), 解析后按此切分数据段
typedef struct {
    uint32_t memaddr; // 闪存段在内存中的起始地址
    uint32_t memsize; // 闪存段的大小（byte）
    uint8_t *memdata; // 存储闪存段数据的指针
} FlashSegmentInfo_t;
typedef struct {
    uint32_t segNum;                                 //	闪存数据被划分为多少个连续的数据段
    FlashSegmentInfo_t segment[SUPPORT_MAX_SEGMENT]; // 存储各个闪存段信息数组
    uint32_t dataLen;                                // 闪存数据的总长度（byte）
    uint8_t *data;                                   // 存储闪存数据的内存地址
} Flashinfo_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief 从文件中读取并解析flash信息
 * @param flashinfo [out] 存储解析后的flash信息的指针
 * @param filepath [in] 要解析的文件路径
 * @return int 成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int GetFlashInfoFromFile(Flashinfo_t **flashinfo, const char *filepath);

/**
 * @brief 释放flash信息结构体占用的内存
 * @param flashinfo [in] 要释放的flash信息结构体指针
 * @return int 成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int FlashInfoFree(Flashinfo_t *flashinfo);

/**
 * @brief 合并多个Flash信息结构
 * @param dest [in,out] 目标Flash信息结构指针
 * @param src [in] 源Flash信息结构指针
 * @return int 成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
int MergeFlashInfo(Flashinfo_t *dest, const Flashinfo_t *src);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif