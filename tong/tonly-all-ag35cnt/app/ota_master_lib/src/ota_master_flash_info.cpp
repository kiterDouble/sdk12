#include "cintelhex.h"
#include "srec_impl.h"
#include "ota_master_flash_info.h"
#include "ota_master_log.h"
#include "ota_master_error_code.h"
#include <string>
#include <errno.h>
#include <cstring>

#define BUFFER_SIZE 1024 // 缓冲区大小

enum FileType {
    FileTypeUnknown = 0, // 未知类型
    FileTypeHex,         // HEX类型
    FileTypeSRecord      // S-Record类型
};

/**
 * @brief 检测文件类型(HEX或S-Record)
 * @param filePath [in] 文件路径
 * @param detectedType [out] 检测到的文件类型
 * @return int 成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
static int detectFileType(const char *filePath, FileType *detectedType)
{
    if (!filePath || !detectedType) {
        OTA_LOG_E("detectFileType: invalid arguments");
        return OTA_RET_ERR;
    }

    char buffer[BUFFER_SIZE] = {0};
    FILE *file = fopen(filePath, "r");
    if (!file) {
        OTA_LOG_E("detectFileType: Failed to open file: %s, errno: %d (%s)", filePath, errno, strerror(errno));
        return OTA_RET_ERR;
    }

    int ret = OTA_RET_ERR;
    if (fgets(buffer, sizeof(buffer), file)) {
        // 跳过可能的BOM头 (UTF-8 BOM: EF BB BF)
        int startIdx = 0;
        if ((unsigned char)buffer[0] == 0xEF && (unsigned char)buffer[1] == 0xBB && (unsigned char)buffer[2] == 0xBF) {
            startIdx = 3;
            OTA_LOG_D("detectFileType: Detected UTF-8 BOM, skipping 3 bytes");
        }
        
        if (buffer[startIdx] == ':') {
            *detectedType = FileTypeHex;
            ret = OTA_RET_OK;
            OTA_LOG_D("detectFileType: Detected HEX file: %s", filePath);
        } else if (buffer[startIdx] == 'S') {
            *detectedType = FileTypeSRecord;
            ret = OTA_RET_OK;
            OTA_LOG_D("detectFileType: Detected S-Record file: %s", filePath);
        } else {
            *detectedType = FileTypeUnknown;
            OTA_LOG_E("detectFileType: Unknown file type, first char: 0x%02X ('%c'), file: %s",
                      (unsigned char)buffer[startIdx],
                      (buffer[startIdx] >= 32 && buffer[startIdx] <= 126) ? buffer[startIdx] : '?',
                      filePath);
        }
    } else {
        OTA_LOG_E("detectFileType: File is empty or read failed: %s", filePath);
    }

    fclose(file);
    return ret;
}

/**
 * @brief 创建并初始化flash信息结构体
 * @param dataLen [in] 需要分配的数据缓冲区大小
 * @return Flashinfo_t* 成功返回结构体指针，失败返回NULL
 */
static Flashinfo_t *createFlashInfo(uint32_t dataLen)
{
    uint8_t *buf = (uint8_t *)calloc(1, dataLen);
    if (!buf) {
        OTA_LOG_E("Memory allocation failed for buffer");
        return NULL;
    }

    Flashinfo_t *info = (Flashinfo_t *)calloc(1, sizeof(Flashinfo_t));
    if (!info) {
        OTA_LOG_E("Memory allocation failed for flash info");
        free(buf);
        return NULL;
    }

    info->data = buf;
    info->dataLen = dataLen;
    return info;
}

/**
 * @brief 从HEX文件中解析flash信息
 * @param flashinfo [out] 存储解析后的flash信息的指针
 * @param filepath [in] HEX文件路径
 * @return int 成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
static int GetFlashInfoFromHex(Flashinfo_t **flashinfo, const char *filepath)
{
    if (!flashinfo || !filepath) {
        OTA_LOG_E("Invalid parameters");
        return OTA_RET_ERR;
    }

    ihex_recordset_t *recSet = ihex_rs_from_file(filepath);
    if (!recSet) {
        OTA_LOG_E("Failed to read hex file");
        return OTA_RET_ERR;
    }

    uint32_t min = 0, max = 0;
    if (ihex_rs_get_address_range(recSet, &min, &max) != 0) {
        OTA_LOG_E("Failed to get address range");
        ihex_rs_free(recSet);
        return OTA_RET_ERR;
    }

    // Use actual data size instead of address range to avoid excessive memory allocation
    // when address range is large but actual data is small
    uint32_t dataLen = ihex_rs_get_size(recSet);
    OTA_LOG_D("Hex file address range: 0x%08X - 0x%08X, actual data size: %u bytes", min, max, dataLen);
    Flashinfo_t *info = createFlashInfo(dataLen);
    if (!info) {
        ihex_rs_free(recSet);
        return OTA_RET_ERR;
    }

    uint32_t i = 0;
    uint32_t nextaddr = 0;
    uint32_t curraddr = 0;
    uint32_t readn = 0;

    do {
        FlashSegmentInfo_t *seginfo;
        ihex_record_t *record;
        uint32_t offset;
        int32_t err;

        err = ihex_rs_iterate_data(recSet, &i, &record, &offset);
        if (err || !record) {
            break;
        }
        curraddr = offset + record->ihr_address;

        if (readn + record->ihr_length > dataLen) {
            OTA_LOG_E("Buffer overflow detected! readn=%u, length=%u, dataLen=%u", readn, record->ihr_length, dataLen);
            FlashInfoFree(info);
            ihex_rs_free(recSet);
            return OTA_RET_ERR;
        }

        if ((nextaddr != curraddr) || curraddr == 0x0) {
            if (info->segNum >= SUPPORT_MAX_SEGMENT) {
                OTA_LOG_E("segNum=%d, Segment count exceeds maximum: %d", info->segNum, SUPPORT_MAX_SEGMENT);
                FlashInfoFree(info);
                ihex_rs_free(recSet);
                return OTA_RET_ERR;
            }
            info->segNum++;
            info->segment[info->segNum - 1].memaddr = curraddr;
            info->segment[info->segNum - 1].memdata = &info->data[readn];
            info->segment[info->segNum - 1].memsize = 0;
        }

        seginfo = &info->segment[info->segNum - 1];
        seginfo->memsize += record->ihr_length;
        nextaddr = curraddr + record->ihr_length;
        memcpy(&info->data[readn], record->ihr_data, record->ihr_length);
        readn += record->ihr_length;
    } while (i > 0);

    OTA_LOG_D("Final read size: %u bytes, segments: %u", readn, info->segNum);
    info->dataLen = readn;
    *flashinfo = info;
    ihex_rs_free(recSet);
    return OTA_RET_OK;
}

/**
 * @brief 从S-Record文件中解析flash信息
 * @param flashinfo [out] 存储解析后的flash信息的指针
 * @param filepath [in] S-Record文件路径
 * @return int 成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 */
static int GetFlashInfoFromSrec(Flashinfo_t **flashinfo, const char *filepath)
{
    if (!flashinfo || !filepath) {
        OTA_LOG_E("Invalid parameters");
        return OTA_RET_ERR;
    }

    SRecordSet_t srecSet;
    if (SrecImplParserFile(filepath, &srecSet) != SRECORD_OK) {
        OTA_LOG_E("Failed to parse S-Record file");
        return OTA_RET_ERR;
    }

    uint32_t startAddr = 0, endAddr = 0;
    if (SrecImplGetAddressRange(&srecSet, &startAddr, &endAddr) != SRECORD_OK) {
        OTA_LOG_E("Failed to get address range");
        SrecImplFree(&srecSet);
        return OTA_RET_ERR;
    }

    uint32_t dataLen = ((endAddr - startAddr + 1024) / 1024) * 1024;
    Flashinfo_t *info = createFlashInfo(dataLen);
    if (!info) {
        SrecImplFree(&srecSet);
        return OTA_RET_ERR;
    }

    uint32_t i = 0;
    uint32_t nextaddr = 0;
    uint32_t curraddr = 0;
    uint32_t readn = 0;

    for (; i < srecSet.recordCount; i++) {
        if ((SRECORD_TYPE_S0 == srecSet.records[i].type) || (SRECORD_TYPE_S4 == srecSet.records[i].type) ||
            (SRECORD_TYPE_S5 == srecSet.records[i].type) || (SRECORD_TYPE_S6 == srecSet.records[i].type)) {
            continue;
        }

        if ((SRECORD_TYPE_S7 == srecSet.records[i].type) || (SRECORD_TYPE_S8 == srecSet.records[i].type) ||
            (SRECORD_TYPE_S9 == srecSet.records[i].type)) {
            break;
        }
        FlashSegmentInfo_t *seginfo;

        curraddr = srecSet.records[i].address;

        if (readn + srecSet.records[i].dataLen > dataLen) {
            OTA_LOG_E("Buffer overflow detected! readn=%u, length=%u, dataLen=%u", readn, srecSet.records[i].dataLen, dataLen);
            FlashInfoFree(info);
            SrecImplFree(&srecSet);
            return OTA_RET_ERR;
        }

        if ((nextaddr != curraddr) || (curraddr == 0x0)) {
            if (info->segNum >= SUPPORT_MAX_SEGMENT) {
                OTA_LOG_E("segNum=%d, Segment count exceeds maximum: %d", info->segNum, SUPPORT_MAX_SEGMENT);
                FlashInfoFree(info);
                SrecImplFree(&srecSet);
                return OTA_RET_ERR;
            }
            info->segNum++;
            info->segment[info->segNum - 1].memaddr = curraddr;
            info->segment[info->segNum - 1].memdata = &info->data[readn];
            info->segment[info->segNum - 1].memsize = 0;
        }
        seginfo = &info->segment[info->segNum - 1];
        memcpy(&info->data[readn], srecSet.records[i].data, srecSet.records[i].dataLen);
        seginfo->memsize += srecSet.records[i].dataLen;
        nextaddr = curraddr + srecSet.records[i].dataLen;
        readn += srecSet.records[i].dataLen;
    }

    info->dataLen = readn;
    *flashinfo = info;
    SrecImplFree(&srecSet);
    return OTA_RET_OK;
}

/**
 * @brief 将解析出的数据段按UDS单次下载块(64KB)切分, 保留原始段边界
 * @param info [in,out] Flash信息结构指针
 * @return int 成功返回OTA_RET_OK，失败返回OTA_RET_ERR
 * @note 每个地址连续段独立, 段内按64KB切块, 首块补齐到64KB边界,
 *       中间整块, 末尾零头独立; 不做跨段合并, 不填充
 */
static int SplitFlashInfoByDownloadBlock(Flashinfo_t *info)
{
    if (!info || info->segNum == 0) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    const uint32_t blockSize = FLASH_DOWNLOAD_BLOCK_SIZE;
    const uint32_t blockMask = blockSize - 1;

    // 按起始地址升序排序
    for (uint32_t i = 1; i < info->segNum; i++) {
        FlashSegmentInfo_t key = info->segment[i];
        int32_t j = (int32_t)i - 1;
        while (j >= 0 && info->segment[j].memaddr > key.memaddr) {
            info->segment[j + 1] = info->segment[j];
            j--;
        }
        info->segment[j + 1] = key;
    }

    // 统计切块数量
    uint32_t newSegNum = 0;
    for (uint32_t i = 0; i < info->segNum; i++) {
        uint32_t addr = info->segment[i].memaddr;
        uint32_t remain = info->segment[i].memsize;
        while (remain > 0) {
            uint32_t offset = addr & blockMask;
            uint32_t chunk;
            if (offset == 0) {
                chunk = (remain < blockSize) ? remain : blockSize;
            } else {
                uint32_t toBoundary = blockSize - offset;
                chunk = (remain < toBoundary) ? remain : toBoundary;
            }
            newSegNum++;
            addr += chunk;
            remain -= chunk;
        }
    }

    if (newSegNum > SUPPORT_MAX_SEGMENT) {
        OTA_LOG_E("%s split segment count exceeds maximum: %d\n", __FUNCTION__, SUPPORT_MAX_SEGMENT);
        return OTA_RET_ERR;
    }

    // 构建切分后的段
    FlashSegmentInfo_t newSegment[SUPPORT_MAX_SEGMENT];
    uint32_t dataOff = 0;
    uint32_t k = 0;
    for (uint32_t i = 0; i < info->segNum; i++) {
        uint32_t addr = info->segment[i].memaddr;
        uint32_t remain = info->segment[i].memsize;
        while (remain > 0) {
            uint32_t offset = addr & blockMask;
            uint32_t chunk;
            if (offset == 0) {
                chunk = (remain < blockSize) ? remain : blockSize;
            } else {
                uint32_t toBoundary = blockSize - offset;
                chunk = (remain < toBoundary) ? remain : toBoundary;
            }
            newSegment[k].memaddr = addr;
            newSegment[k].memsize = chunk;
            newSegment[k].memdata = info->data + dataOff;
            dataOff += chunk;
            addr += chunk;
            remain -= chunk;
            k++;
        }
    }
    
    memcpy(info->segment, newSegment, sizeof(FlashSegmentInfo_t) * newSegNum);
    info->segNum = newSegNum;

    OTA_LOG_D("%s split segments=%u\n", __FUNCTION__, info->segNum);
    return OTA_RET_OK;
}

int GetFlashInfoFromFile(Flashinfo_t **flashinfo, const char *filepath)
{
    FileType detectedType;

    if (!flashinfo || !filepath) {
        OTA_LOG_E("%s: invalid arguments\n", __FUNCTION__);
        return OTA_RET_ERR;
    }
    if (0 != detectFileType(filepath, &detectedType)) {
        OTA_LOG_E("%s: detect file type failed\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    OTA_LOG_D("%s: detected file type %d for file %s\n", __FUNCTION__, detectedType, filepath);
    int ret = OTA_RET_ERR;
    switch (detectedType) {
        case FileTypeSRecord:
            OTA_LOG_D("----%s,%d.\n", __FUNCTION__, __LINE__);
            ret = GetFlashInfoFromSrec(flashinfo, filepath);
            break;
        case FileTypeHex:
            OTA_LOG_D("----%s,%d.\n", __FUNCTION__, __LINE__);
            ret = GetFlashInfoFromHex(flashinfo, filepath);
            break;
        default:
            OTA_LOG_D("%s: unsupported file type\n", __FUNCTION__);
            return OTA_RET_ERR;
    }

    if (ret != OTA_RET_OK) {
        return ret;
    }

    // 按UDS单次下载块(64KB)切分数据段，保留原始段边界
    return SplitFlashInfoByDownloadBlock(*flashinfo);
}

int FlashInfoFree(Flashinfo_t *flashinfo)
{
    if (!flashinfo) {
        OTA_LOG_E("%s param invalid.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    if (flashinfo->data) {
        free(flashinfo->data);
    }
    free(flashinfo);
    return OTA_RET_OK;
}

int MergeFlashInfo(Flashinfo_t *dest, const Flashinfo_t *src)
{
    if (!dest || !src) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    OTA_LOG_D("Before merge: dest segments=%d, data length=%d", dest->segNum, dest->dataLen);
    OTA_LOG_D("Source info: segments=%d, data length=%d", src->segNum, src->dataLen);

    // 段数量检查
    if ((dest->segNum + src->segNum) > SUPPORT_MAX_SEGMENT) {
        OTA_LOG_E("%s segment number exceeds maximum limit %d.\n", __FUNCTION__, SUPPORT_MAX_SEGMENT);
        return OTA_RET_ERR;
    }

    // 分配新内存用于合并数据
    uint32_t newDataLen = dest->dataLen + src->dataLen;
    uint8_t *newData = (uint8_t *)malloc(newDataLen);
    if (!newData) {
        OTA_LOG_E("%s memory allocation failed.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    // 复制数据
    memcpy(newData, dest->data, dest->dataLen);
    memcpy(newData + dest->dataLen, src->data, src->dataLen);

    // 保存旧的数据指针用于计算偏移量
    uint8_t *oldData = dest->data;

    // 更新dest的数据指针和长度
    dest->data = newData;
    dest->dataLen = newDataLen;

    // 更新原有段的memdata指针
    for (uint32_t i = 0; i < dest->segNum; i++) {
        uint32_t offset = dest->segment[i].memdata - oldData;
        dest->segment[i].memdata = &newData[offset];
        OTA_LOG_D("Updated segment %d: addr=0x%08x, size=%d", i, dest->segment[i].memaddr, dest->segment[i].memsize);
    }

    // 复制并更新新段的信息
    for (uint32_t i = 0; i < src->segNum; i++) {
        uint32_t newIndex = dest->segNum + i;
        dest->segment[newIndex].memaddr = src->segment[i].memaddr;
        dest->segment[newIndex].memsize = src->segment[i].memsize;
        uint32_t offset = src->segment[i].memdata - src->data;
        dest->segment[newIndex].memdata = &newData[dest->dataLen - newDataLen + offset];
        OTA_LOG_D("Added new segment %d: addr=0x%08x, size=%d", newIndex, dest->segment[newIndex].memaddr, dest->segment[newIndex].memsize);
    }

    dest->segNum += src->segNum;
    free(oldData);

    OTA_LOG_D("Merge completed: total segments=%d, total data length=%d", dest->segNum, dest->dataLen);
    return OTA_RET_OK;
}