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
// #include <mcheck.h>
#include <ctype.h>
#include <glib.h>
#include <errno.h>
#include "sr_log.h"
#include "err_code.h"
#include "upgrade_list.h"
#include "ota_srec_parser.h"
#include "ota_crc32.h"
#include "hexTool.h"


#define SREC_FILE_ONE_LINE_MAX_SIZE     (256)
#define SREC_ADDR_LEN                   (4)
#define SREC_ONE_LINE_CHECK_LEN         (1)

typedef struct
{
    struct list_head list;
    int32_t len;
    uint8_t data[0];
} SrecList_t;

typedef struct
{
    SrecList_t sList;
    int32_t fileLen;
    uint32_t startAddr;
    uint32_t u32Crc;
} SrecPrivate_t;

static SrecPrivate_t *g_SrecFileList = NULL;

/**
 * @name HexStrToByte
 * @brief 将十六进制字符串转换为字节数据。
 * 
 * @param dest 指向用于存放转换结果的字节数组的指针。
 * @param pLen 指向整数的指针，用于返回转换后的字节数据长度。
 * @param source 指向十六进制字符串的指针。
 * @param sourceLen 十六进制字符串的长度。
 * @return 转换成功返回0，参数错误返回-1。
 */
static int32_t HexStrToByte(uint8_t *dest, int32_t *pLen, const uint8_t *source, int32_t sourceLen)
{
    if (!dest || !pLen || !source || !sourceLen)
    {
        return -1;
    }

    gint i = 0;
    guchar high4bit = 0;
    guchar low4bit = 0;
    gint count = 0;

    for (; i < sourceLen; i += 2)
    {
        if ((source[i] == '\r') || (source[i] == '\n'))
        {
            break;
        }
        high4bit = toupper(source[i]);
        low4bit = toupper(source[i + 1]);
        if (high4bit >= 'A')
        {
            high4bit = 0x0a + (high4bit - 'A');
        }
        else
        {
            high4bit = high4bit - '0';
        }
        if (low4bit >= 'A')
        {
            low4bit = 0x0a + (low4bit - 'A');
        }
        else
        {
            low4bit = low4bit - '0';
        }
        dest[count++] = ((high4bit << 4) & 0xf0) | (low4bit & 0x0f);
    }
    *pLen = count;
#if 0
    for (i = 0; i < count; i++)
    {
        printf("%02x ", dest[i]);
    }
    printf("\n");
#endif
    return 0;
}

/**
 * @name DestroyListAndFree
 * @brief 从链表中删除所有节点并释放内存
 * @param header 链表的头指针
 * @return 若成功执行返回RET_OK，若头指针为空返回RET_ERR
 */
int32_t DeleteAllNodeFromListAndFree(struct list_head *header)
{
    struct list_head *pos = NULL;
    struct list_head *pnext = NULL;
    SrecList_t *tmp = NULL;

    if(NULL == header)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    list_for_each_safe(pos, pnext, header)
    {
        tmp = list_entry(pos, SrecList_t, list);
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
 * 它是通过调用DeleteAllNodeFromListAndFree函数来实现的。
 * 
 * @param 无
 * @return int32_t 返回DeleteAllNodeFromListAndFree函数的执行结果。
 */
int32_t DestroyListAndFree()
{
    return DeleteAllNodeFromListAndFree(&g_SrecFileList->sList.list);
}


int32_t ReadSrecFileContenToList(FILE *fileFp, struct list_head *listHead)
{
    char fileContenBuff[SREC_FILE_ONE_LINE_MAX_SIZE] = {0};
    char hexFileContenBuff[SREC_FILE_ONE_LINE_MAX_SIZE / 2] = {0};
    int32_t hexLen = 0;
    char tmpBuff[32] = {0};
    char *endptr = NULL;
    int32_t lineContenLen = 0;
    SrecList_t *tmpListEntry = NULL;

    if(NULL == fileFp || NULL == listHead)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == g_SrecFileList)
    {
        LOG_Error("%s g_SrecFileList is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    g_SrecFileList->startAddr = 0x00;
    g_SrecFileList->fileLen = 0;
    g_SrecFileList->u32Crc = Crc32Init();

    while(1)
    {
        memset(fileContenBuff, 0x0, SREC_FILE_ONE_LINE_MAX_SIZE);
        if (fgets(fileContenBuff, SREC_FILE_ONE_LINE_MAX_SIZE, fileFp) == NULL)
        {
            LOG_Debug("fgets return NULL [%s]", strerror(errno));
            break;
        }

        if('S' == fileContenBuff[0] && '3' == fileContenBuff[1])
        {
            if(0x00 == g_SrecFileList->startAddr && 0 == g_SrecFileList->fileLen)
            {
                memset(tmpBuff, 0, 32);
                memcpy(tmpBuff, &fileContenBuff[4], 8);
                g_SrecFileList->startAddr = strtol(tmpBuff, &endptr, 16);
                LOG_Debug("g_SrecFileList->startAddr: %X", g_SrecFileList->startAddr);
            }
            memset(tmpBuff, 0, 32);
            memcpy(tmpBuff, &fileContenBuff[2], 2);
            lineContenLen = strtol(tmpBuff, &endptr, 16);
            // LOG_Debug("lineContenLen: %d", lineContenLen);
            if(lineContenLen > 255)
            {
                LOG_Debug("lineContenLen is error");
                g_SrecFileList->u32Crc = Crc32Final(g_SrecFileList->u32Crc);
                DeleteAllNodeFromListAndFree(listHead);
                return RET_ERR;
            }

            tmpListEntry = (SrecList_t *)calloc(1, sizeof(SrecList_t) + lineContenLen);
            if(NULL == tmpListEntry)
            {
                g_SrecFileList->u32Crc = Crc32Final(g_SrecFileList->u32Crc);
                DeleteAllNodeFromListAndFree(listHead);
                return RET_ERR;
            }

            HexStrToByte((uint8_t *)hexFileContenBuff, &hexLen, (uint8_t *)&fileContenBuff[2], strlen(&fileContenBuff[2]));

            g_SrecFileList->u32Crc = Crc32Update(g_SrecFileList->u32Crc, &hexFileContenBuff[5], 
                                    lineContenLen - SREC_ADDR_LEN - SREC_ONE_LINE_CHECK_LEN);
            memset(tmpListEntry->data, 0xff, lineContenLen);          //20 byte aligned
            memcpy(tmpListEntry->data, &hexFileContenBuff[1], lineContenLen - SREC_ONE_LINE_CHECK_LEN); //do not copy hexArray[0](len)��hexArray[len-1](checksum)
            //tmp->len = SREC_FIXED_DATALEN;
            //pPrivate->fileLen += SREC_FIXED_DATALEN;
            tmpListEntry->len = lineContenLen - SREC_ONE_LINE_CHECK_LEN;
            // LOG_DebugHex(tmpListEntry->data, tmpListEntry->len, "file conten");
            g_SrecFileList->fileLen += tmpListEntry->len;
            list_add_tail(&tmpListEntry->list, listHead);
        }
    }
    g_SrecFileList->u32Crc = Crc32Final(g_SrecFileList->u32Crc);
    return RET_OK;
}



/**
 * @name InitSrecFileToList
 * @brief 初始化将Srec文件内容加载到列表中
 * 
 * @param filePathName 文件路径名
 * @return int32_t 返回初始化结果，RET_OK表示成功，RET_ERR表示错误。
 */
int32_t InitSrecFileToList(const char *filePathName)
{
    int32_t retValue = RET_ERR;
    FILE *fileFp = NULL;

    if(NULL == filePathName)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    LOG_Debug("%s", filePathName);

    fileFp = fopen(filePathName, "rw");
    if(NULL == fileFp)
    {
        LOG_Error("%s , open %s  file error", __FUNCTION__, filePathName);
        return RET_ERR;
    }

    if(NULL != g_SrecFileList)
    {
        fclose(fileFp);
        LOG_Error("%s , list not free and return error", __FUNCTION__);
        return RET_ERR;
    }

    g_SrecFileList = (SrecPrivate_t *)calloc(1, sizeof(SrecPrivate_t));
    if (NULL == g_SrecFileList)
    {
        LOG_Debug("g_SrecFileList calloc error:[%s]", strerror(errno));
        fclose(fileFp);
        return RET_ERR;
    }

    INIT_LIST_HEAD(&g_SrecFileList->sList.list);

    retValue = ReadSrecFileContenToList(fileFp, &g_SrecFileList->sList.list);
    if(retValue < RET_OK)
    {
        LOG_Debug("ReadSrecFileContenToList  error");
        fclose(fileFp);
        return RET_ERR;
    }

    return RET_OK;
}


/**
 * @name GetUpgradeSrecFileContenLen
 * @brief 获取升级Srec文件的内容长度
 * 
 * 本函数用于获取当前系统中Srec格式升级文件的内容长度。它首先检查全局Srec文件列表指针是否为NULL，
 * 如果是NULL，则记录错误日志并返回错误码；否则，返回Srec文件的内容长度。
 * 
 * @return int32_t 返回Srec文件的内容长度。如果g_SrecFileList为NULL，则返回RET_ERR。
 */
int32_t GetUpgradeSrecFileContenLen()
{
    if(NULL == g_SrecFileList)
    {
        LOG_Error("%s g_SrecFileList is NULL", __FUNCTION__);
        return RET_ERR;
    }

    return g_SrecFileList->fileLen;
}

/**
 * @name GetUpgradeSrecFileStartAddr
 * @brief 获取升级Srec文件的起始地址
 * 
 * @param startAddr 指向一个uint32_t类型的变量的指针，该函数会通过这个指针返回Srec文件的起始地址。
 * @return 若成功则返回RET_OK，若g_SrecFileList或startAddr为空则返回RET_ERR。
 */
int32_t GetUpgradeSrecFileStartAddr(uint32_t *startAddr)
{
    if(NULL == g_SrecFileList)
    {
        LOG_Error("%s g_SrecFileList is NULL", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == startAddr)
    {
        LOG_Error("%s argv is invalid , startAddr is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    *startAddr = g_SrecFileList->startAddr;

    return RET_OK;
}


/**
 * @name GetUpgradeSrecFileCrc
 * @brief 获取升级Srec文件的CRC值
 * 
 * @param crc32Value 指向一个32位无符号整型变量的指针，用于接收Srec文件的CRC值。
 * @return 若成功，则返回RET_OK；若g_SrecFileList或crc32Value为NULL，则返回RET_ERR。
 */
uint32_t GetUpgradeSrecFileCrc(uint32_t *crc32Value)
{
    if(NULL == g_SrecFileList)
    {
        LOG_Error("%s g_SrecFileList is NULL", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == crc32Value)
    {
        LOG_Error("%s argv is invalid , crc32Value is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    *crc32Value = g_SrecFileList->u32Crc;
    return RET_OK;
}


/**
 * @name GetSrecFileContenLenFormList
 * @brief 从列表中获取Srec文件内容的长度
 * 
 * @param fileConten 指向文件内容的指针
 * @param contenLen 文件内容的初始长度
 * @param readLen 需要读取的长度
 * @return 读取内容的长度，出错返回RET_ERR
 */
int32_t GetSrecFileContenLenFormList(uint8_t *fileConten, int32_t contenLen, int32_t readLen)
{
    static struct list_head *pos = NULL;
    struct list_head *header = NULL;
    SrecList_t *tmp = NULL;
    int32_t readOffset = 0;
    static uint32_t addr = 0x00; 
    uint32_t tmpAddr = 0x00; 
    
    if(NULL == g_SrecFileList)
    {
        LOG_Error("%s g_SrecFileList is NULL", __FUNCTION__);
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

    header = &g_SrecFileList->sList.list;
    if(NULL == pos)
    {
	LOG_Error("%s tmpAddr :%08X", __FUNCTION__, g_SrecFileList->startAddr);
        pos = (header)->next;
    }
    

    while(1)
    {
        if(pos == (header))
        {
            LOG_Error("pos == (header) :%s", __FUNCTION__);
            break;
        }

        tmp = list_entry(pos, SrecList_t, list);
        if(NULL == tmp)
        {
            LOG_Error("list_entry :%s", __FUNCTION__);
            break;
        }

        if((readOffset + tmp->len - SREC_ADDR_LEN) > readLen)
        {
            LOG_Error("if((readOffset + tmp->len) > readLen) :%s", __FUNCTION__);
            break;
        }
        
        tmpAddr = 0x00;
        tmpAddr |= (tmp->data[0] << 24);
        tmpAddr |= (tmp->data[1] << 16);
        tmpAddr |= (tmp->data[2] << 8);
        tmpAddr |= tmp->data[3];

        if(0 == readOffset)
        {
            readOffset += tmp->len;
            memcpy(fileConten, tmp->data, tmp->len);
        }
        else
        {
            /*地址发生了跳变*/
            if(tmpAddr != addr)
            {
                LOG_Error("if((tmpAddr - tmp->len) != addr) :%s", __FUNCTION__);
                break;
            }
            
            memcpy(&fileConten[readOffset], &(tmp->data[SREC_ADDR_LEN]), tmp->len - SREC_ADDR_LEN);
            readOffset += tmp->len - SREC_ADDR_LEN;
        }
        addr = tmpAddr + tmp->len - SREC_ADDR_LEN;
        pos = pos->next;
	    tmp = NULL;
    }

    return readOffset;
}


