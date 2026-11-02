/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-09-04 10:31:31
 * @LastEditTime: 2019-09-04 16:47:13
 * @LastEditors: Please set LastEditors
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <glib.h>
#include <errno.h>
#include "sr_log.h"
#include "err_code.h"
#include "upgrade_list.h"
#include "ota_bin_parser.h"
#include "ota_crc32.h"
#include "hexTool.h"


#define BIN_FILE_ONE_LINE_MAX_SIZE     (512)
#define BIN_ADDR_LEN                   (4)
#define BIN_ONE_LINE_CHECK_LEN         (1)

typedef struct
{
    struct list_head list;
    int32_t len;
    uint8_t data[0];
} BinList_t;

typedef struct
{
    BinList_t sList;
    int32_t fileLen;
    uint32_t startAddr;
    uint32_t u32Crc;
} BinPrivate_t;

static BinPrivate_t *g_BinFileList = NULL;
static struct list_head *g_currentPos = NULL;  // 当前读取位置
static int32_t g_currentOffset = 0;            // 当前节点已读取偏移

/**
 * @name DestroyListAndFree
 * @brief 从链表中删除所有节点并释放内存
 * @param header 链表的头指针
 * @return 若成功执行返回RET_OK，若头指针为空返回RET_ERR
 */
int32_t DeleteBinAllNodeFromListAndFree(struct list_head *header)
{
    struct list_head *pos = NULL;
    struct list_head *pnext = NULL;
    BinList_t *tmp = NULL;

    if(NULL == header)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    list_for_each_safe(pos, pnext, header)
    {
        tmp = list_entry(pos, BinList_t, list);
        if(NULL != tmp)
        {
            list_del(pos);
            free(tmp);
        }
    }

    return RET_OK;
}

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
int32_t DestroyBinListAndFree()
{
    return DeleteBinAllNodeFromListAndFree(&g_BinFileList->sList.list);
}

int32_t ReadBinFileContenToList(FILE *fileFp, struct list_head *listHead)
{
    char fileContenBuff[BIN_FILE_ONE_LINE_MAX_SIZE] = {0};
    int32_t readlen = 0;
    BinList_t *tmpListEntry = NULL;

    if(NULL == fileFp || NULL == listHead)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == g_BinFileList)
    {
        LOG_Error("%s g_BinFileList is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    g_BinFileList->startAddr = 0x00;
    g_BinFileList->fileLen = 0;
    g_BinFileList->u32Crc = Crc32Init();

    while(1)
    {
        memset(fileContenBuff, 0x0, BIN_FILE_ONE_LINE_MAX_SIZE);
        if ((readlen = fread(fileContenBuff, 1, BIN_FILE_ONE_LINE_MAX_SIZE, fileFp)) != BIN_FILE_ONE_LINE_MAX_SIZE)
        {
            if (readlen < 0 || readlen == 0 ) {  
                if (feof(fileFp)) {  
                    LOG_Debug("End of file reached before reading %zu bytes.\n", readlen);  
                    break;
                }else if (ferror(fileFp)) {  
                    LOG_Error("Error reading file");  
                    break;
                }  
            }
        }

        LOG_Debug("read file data readlen =%zu",readlen);
        tmpListEntry = (BinList_t *)calloc(1, sizeof(BinList_t) + readlen);
        if(NULL == tmpListEntry)
        {
            g_BinFileList->u32Crc = Crc32Final(g_BinFileList->u32Crc);
            DeleteBinAllNodeFromListAndFree(listHead);
            return RET_ERR;
        }
        g_BinFileList->u32Crc = Crc32Update(g_BinFileList->u32Crc,fileContenBuff, readlen);
        memset(tmpListEntry->data, 0xff, readlen);         
        memcpy(tmpListEntry->data, fileContenBuff, readlen); 
        tmpListEntry->len = readlen;
        //LOG_DebugHex(tmpListEntry->data, tmpListEntry->len, "file conten");
        g_BinFileList->fileLen += tmpListEntry->len;
        list_add_tail(&tmpListEntry->list, listHead);

    }
    g_BinFileList->u32Crc = Crc32Final(g_BinFileList->u32Crc);
    LOG_Debug("g_BinFileList->u32Crc = %x",g_BinFileList->u32Crc);
    return RET_OK;
}


/**
 * @name InitBinFileToList
 * @brief 初始化将Bin文件内容加载到列表中
 * 
 * @param filePathName 文件路径名
 * @return int32_t 返回初始化结果，RET_OK表示成功，RET_ERR表示错误。
 */
int32_t InitBinFileToList(const char *filePathName)
{
    int32_t retValue = RET_ERR;
    FILE *fileFp = NULL;

    if(NULL == filePathName)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    LOG_Debug("%s", filePathName);

    fileFp = fopen(filePathName, "rb");
    if(NULL == fileFp)
    {
        LOG_Error("%s , open %s  file error", __FUNCTION__, filePathName);
        return RET_ERR;
    }

    if(NULL != g_BinFileList)
    {
        fclose(fileFp);
        LOG_Error("%s , list not free and return error", __FUNCTION__);
        return RET_ERR;
    }

    g_BinFileList = (BinPrivate_t *)calloc(1, sizeof(BinPrivate_t));
    if (NULL == g_BinFileList)
    {
        LOG_Debug("g_BinFileList calloc error:[%s]", strerror(errno));
        fclose(fileFp);
        return RET_ERR;
    }

    INIT_LIST_HEAD(&g_BinFileList->sList.list);

    retValue = ReadBinFileContenToList(fileFp, &g_BinFileList->sList.list);
    if(retValue < RET_OK)
    {
        LOG_Debug("ReadBinFileContenToList  error");
        fclose(fileFp);
        return RET_ERR;
    }

    return RET_OK;
}


/**
 * @name GetUpgradeBinFileContenLen
 * @brief 获取升级Bin文件的内容长度
 * 
 * 本函数用于获取当前系统中Bin格式升级文件的内容长度。它首先检查全局Bin文件列表指针是否为NULL，
 * 如果是NULL，则记录错误日志并返回错误码；否则，返回Bin文件的内容长度。
 * 
 * @return int32_t 返回Bin文件的内容长度。如果g_BinFileList为NULL，则返回RET_ERR。
 */
int32_t GetUpgradeBinFileContenLen()
{
    if(NULL == g_BinFileList)
    {
        LOG_Error("%s g_BinFileList is NULL", __FUNCTION__);
        return RET_ERR;
    }

    return g_BinFileList->fileLen;
}

/**
 * @name GetUpgradeBinFileStartAddr
 * @brief 获取升级Bin文件的起始地址
 * 
 * @param startAddr 指向一个uint32_t类型的变量的指针，该函数会通过这个指针返回Bin文件的起始地址。
 * @return 若成功则返回RET_OK，若g_BinFileList或startAddr为空则返回RET_ERR。
 */
int32_t GetUpgradeBinFileStartAddr(uint32_t *startAddr)
{
    if(NULL == g_BinFileList)
    {
        LOG_Error("%s g_BinFileList is NULL", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == startAddr)
    {
        LOG_Error("%s argv is invalid , startAddr is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    *startAddr = g_BinFileList->startAddr;

    return RET_OK;
}


/**
 * @name GetUpgradeBinFileCrc
 * @brief 获取升级Bin文件的CRC值
 * 
 * @param crc32Value 指向一个32位无符号整型变量的指针，用于接收Bin文件的CRC值。
 * @return 若成功，则返回RET_OK；若g_BinFileList或crc32Value为NULL，则返回RET_ERR。
 */
uint32_t GetUpgradeBinFileCrc(uint32_t *crc32Value)
{
    if(NULL == g_BinFileList)
    {
        LOG_Error("%s g_BinFileList is NULL", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == crc32Value)
    {
        LOG_Error("%s argv is invalid , crc32Value is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    *crc32Value = g_BinFileList->u32Crc;
    return RET_OK;
}


/**
 * @name GetBinFileContenLenFormList
 * @brief 从列表中获取Bin文件内容的长度
 * 
 * @param fileConten 指向文件内容的指针
 * @param contenLen 文件内容的初始长度
 * @param readLen 需要读取的长度
 * @return 读取内容的长度，出错返回RET_ERR
 */
int32_t GetBinFileContenLenFormList(uint8_t *fileConten, int32_t contenLen, int32_t readLen)
{
    struct list_head *header = NULL;
    BinList_t *tmp = NULL;
    int32_t readOffset = 0;

    if(NULL == g_BinFileList)
    {
        LOG_Error("%s g_BinFileList is NULL", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == fileConten)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    if(contenLen < readLen || readLen <= 0)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    header = &g_BinFileList->sList.list;
    
    // 第一次调用时初始化位置
    if(NULL == g_currentPos)
    {
        g_currentPos = (header)->next;
        g_currentOffset = 0;
    }
    
    while(1)
    {
        if(g_currentPos == (header))
        {
            LOG_Error("g_currentPos == (header) :%s", __FUNCTION__);
            break;
        }

        tmp = list_entry(g_currentPos, BinList_t, list);
        if(NULL == tmp)
        {
            LOG_Error("list_entry :%s", __FUNCTION__);
            break;
        }

        int32_t availableLen = tmp->len - g_currentOffset;
        int32_t copyLen = availableLen;
        
        if((readOffset + copyLen) > readLen)
        {
            copyLen = readLen - readOffset;
        }
        
        if(copyLen <= 0)
        {
            break;
        }
        
        memcpy(&fileConten[readOffset], &tmp->data[g_currentOffset], copyLen);
        readOffset += copyLen;
        g_currentOffset += copyLen;
        
        if(g_currentOffset >= tmp->len)
        {
            g_currentPos = g_currentPos->next;
            g_currentOffset = 0;
        }
        tmp = NULL;
        
        if(readOffset >= readLen)
        {
            break;
        }
    }
    LOG_Debug("GetBinFileContenLenFormList: readOffset=%d, g_currentOffset=%d", readOffset, g_currentOffset);
    return readOffset;
}

/**
 * @name ResetBinFileReadPosition
 * @brief 重置BIN文件读取位置
 *
 * 在升级开始时调用，确保从文件开头读取
 */
void ResetBinFileReadPosition(void)
{
    g_currentPos = NULL;
    g_currentOffset = 0;
    LOG_Debug("ResetBinFileReadPosition: read position reset");
}
