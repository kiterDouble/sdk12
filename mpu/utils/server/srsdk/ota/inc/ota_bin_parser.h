#ifndef __UPGRADE_BIN_H__
#define __UPGRADE_BIN_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name InitBinFileToList
 * @brief 初始化将Bin文件内容加载到列表中
 * 
 * @param filePathName 文件路径名
 * @return int32_t 返回初始化结果，RET_OK表示成功，RET_ERR表示错误。
 */
int32_t InitBinFileToList(const char *filePathName);


/**
 * @name GetUpgradeBinFileContenLen
 * @brief 获取升级Bin文件的内容长度
 * 
 * 本函数用于获取当前系统中Bin格式升级文件的内容长度。它首先检查全局Bin文件列表指针是否为NULL，
 * 如果是NULL，则记录错误日志并返回错误码；否则，返回Bin文件的内容长度。
 * 
 * @return int32_t 返回Bin文件的内容长度。如果g_BinFileList为NULL，则返回RET_ERR。
 */
int32_t GetUpgradeBinFileContenLen();

/**
 * @name GetUpgradeBinFileStartAddr
 * @brief 获取升级Bin文件的起始地址
 * 
 * @param startAddr 指向一个uint32_t类型的变量的指针，该函数会通过这个指针返回Bin文件的起始地址。
 * @return 若成功则返回RET_OK，若g_BinFileList或startAddr为空则返回RET_ERR。
 */
int32_t GetUpgradeBinFileStartAddr(uint32_t *startAddr);

/**
 * @name GetUpgradeBinFileCrc
 * @brief 获取升级Bin文件的CRC值
 * 
 * @param crc32Value 指向一个32位无符号整型变量的指针，用于接收Bin文件的CRC值。
 * @return 若成功，则返回RET_OK；若g_BinFileList或crc32Value为NULL，则返回RET_ERR。
 */
uint32_t GetUpgradeBinFileCrc(uint32_t *crc32Value);


/**
 * @name GetBinFileContenLenFormList
 * @brief 从列表中获取Bin文件内容的长度
 * 
 * @param fileConten 指向文件内容的指针
 * @param contenLen 文件内容的初始长度
 * @param readLen 需要读取的长度
 * @return 读取内容的长度，出错返回RET_ERR
 */
int32_t GetBinFileContenLenFormList(uint8_t *fileConten, int32_t contenLen, int32_t readLen);

/**
 * @name DestroyListAndFree
 * @brief 销毁列表并释放内存
 * 
 * 该函数用于销毁指定列表中的所有节点，并释放与这些节点相关联的内存。
 * 它是通过调用DeleteBinAllNodeFromListAndFree函数来实现的。
 * 
 * @param 无
 * @return int32_t 返回DeleteBinAllNodeFromListAndFree函数的执行结果。
 */
int32_t DestroyBinListAndFree();

#ifdef __cplusplus
}
#endif
#endif
