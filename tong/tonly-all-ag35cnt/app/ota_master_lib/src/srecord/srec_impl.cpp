#include "srec_impl.h"
#include "ota_master_log.h"

static int8_t isValidSRecord(const char *line)
{
    return (line[0] == 'S');
}

static int32_t SrecImplGetValidRecordCounts(const char *fileName, int32_t *counts)
{
    FILE *file = fopen(fileName, "r"); // 打开文件
    if (file == NULL)
    {
        printf("%s open %s fail.\n", __FUNCTION__, fileName);
        return SRECORD_ERROR_FILE; // 文件打开失败
    }

    int32_t validRecords = 0;
    char record[SRECORD_RECORD_BUFF_SIZE]; // 存储单个记录的缓冲区

    while (fgets(record, SRECORD_RECORD_BUFF_SIZE, file))
    { // 逐行读取文件
        if (isValidSRecord(record))
        {
            validRecords++; // 记录有效，增加计数
        }
    }

    fclose(file); // 关闭文件

    *counts = validRecords; // 将有效记录数写入指针指向的位置
    return SRECORD_OK; // 返回成功标志
}

int32_t SrecImplParserFile(const char *fileName, SRecordSet_t *srecSet)
{
    SRecord srec;
    int32_t count = 0;
    int32_t items = 0;

    if (SrecImplGetValidRecordCounts(fileName, &count) < 0)
    {
        printf("%s get valid record count fail.\n", __FUNCTION__);
        return SRECORD_ERROR_FILE;
    }
    FILE *fp = fopen(fileName, "r"); // 打开文件
    if (fp == NULL)
    {
        printf("%s open %s fail.\n", __FUNCTION__, fileName);
        return SRECORD_ERROR_FILE; // 文件打开失败
    }

    srecSet->records = (SRecord*)calloc(1, sizeof(SRecord) * count);
    if (srecSet->records == NULL)
    {
        printf("%s calloc fail.\n", __FUNCTION__);
        fclose(fp);
        return SRECORD_ERROR_FILE;
    }
    while (Read_SRecord(&srec, fp) == SRECORD_OK)
    {
        memcpy(&srecSet->records[items], &srec, sizeof(SRecord));
        items++;
    }
    fclose(fp);    
    srecSet->recordCount = items;
    if(srecSet->recordCount != count)
    {
        printf("%s file parser fail.\n", __FUNCTION__);
        free(srecSet->records);
        return SRECORD_ERROR_FILE;
    }
    return SRECORD_OK;
}

int32_t SrecImplGetAddressRange(SRecordSet_t *srecSet, uint32_t *startAddr, uint32_t *endAddr)
{
    if (!srecSet || !startAddr || !endAddr) {
        return SRECORD_ERROR_FILE;
    }

    bool foundFirst = false;
    *startAddr = 0xFFFFFFFF;
    *endAddr = 0;

    OTA_LOG_D("Parsing address range from %d records", srecSet->recordCount);

    for (uint32_t i = 0; i < srecSet->recordCount; i++) {
        SRecord *record = &srecSet->records[i];
        
        if (record->type != SRECORD_TYPE_S1 && 
            record->type != SRECORD_TYPE_S2 && 
            record->type != SRECORD_TYPE_S3) {
            continue;
        }

        if (!foundFirst) {
            *startAddr = record->address;
            foundFirst = true;
        }

        if (record->address < *startAddr) {
            *startAddr = record->address;
        }

        uint32_t endAddrThis = record->address + record->dataLen - 1;
        if (endAddrThis > *endAddr) {
            *endAddr = endAddrThis;
        }
    }

    OTA_LOG_D("Found address range: 0x%08X - 0x%08X", *startAddr, *endAddr);

    return (*startAddr <= *endAddr) ? SRECORD_OK : SRECORD_ERROR_FILE;
}
int32_t SrecImplFree(SRecordSet_t *srecSet)
{
    if (!srecSet)
    {
        return SRECORD_ERROR_INVALID_ARGUMENTS;
    }
    
    if (srecSet->records != NULL)
    {
        free(srecSet->records);
        srecSet->records = NULL;
    }
    srecSet->recordCount = 0;
    return SRECORD_OK;
}
