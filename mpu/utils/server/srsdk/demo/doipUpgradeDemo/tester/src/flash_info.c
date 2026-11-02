#include <string.h>
#include <stdlib.h>
#include "err_code.h"
#include "sr_log.h"
#include "cintelhex.h"
#include "flash_info.h"

int32_t GetFlashInfoFromHex(Flashinfo_t **flashinfo, const char *filepath)
{
    Flashinfo_t *info;
    int32_t ret = RET_ERR;

    if (!flashinfo || !filepath)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        goto err_exit;
    }

    ihex_recordset_t *recSet = ihex_rs_from_file(filepath);
    if (!recSet)
    {
        LOG_Error("%s ihex_rs_from_file fail.", __FUNCTION__);
        goto err_exit;        
    }
    uint32_t min = 0;
    uint32_t max = 0;
    uint32_t dataLen = 0;
    if (0 != ihex_rs_get_address_range(recSet, &min, &max))
    {
        LOG_Error("%s ihex_rs_get_address_range fail.", __FUNCTION__);
        goto err_exit0;
    }

    LOG_Debug("min=%x, max=%x, max-min=%x", min, max, (max-min));
    dataLen = max -min;
    uint8_t *buf;

    buf = (uint8_t*)calloc(1, dataLen);
    if (!buf)
    {
        LOG_Error("%s calloc buf fail.", __FUNCTION__);
        goto err_exit0;       
    }

    info = (Flashinfo_t*)calloc(1, sizeof(Flashinfo_t));
    if (!info)
    {
        LOG_Error("%s calloc info fail.", __FUNCTION__);
        goto err_exit1;
    }
    uint32_t i = 0;
    uint32_t nextaddr = 0;
    uint32_t curraddr = 0;
    uint32_t readn = 0;

    info->data = buf;
    info->dataLen = dataLen;
    do 
    {
        FlashSegmentInfo_t *seginfo;
        ihex_record_t *record;
        uint32_t offset;
        int32_t err;

        err = ihex_rs_iterate_data(recSet, &i, &record, &offset);
        if (err || !record)
        {
            break;
        }
        curraddr = offset + record->ihr_address;

        if (nextaddr != curraddr)
        {
            info->segNum++;
            info->segment[info->segNum - 1].memaddr = curraddr;
            info->segment[info->segNum - 1].memdata = &buf[readn];
            info->segment[info->segNum - 1].memsize = 0;
        }
        seginfo = &info->segment[info->segNum - 1];
        seginfo->memsize += record->ihr_length;
        nextaddr = curraddr + record->ihr_length;
        memcpy(&buf[readn], record->ihr_data, record->ihr_length);
        readn += record->ihr_length;        
    }while (i > 0);

    info->dataLen = readn;
    *flashinfo = info;
    ret = RET_OK;
    goto err_exit0;

err_exit1:
    free(buf);
err_exit0:
    ihex_rs_free(recSet);
err_exit:
    return ret;
}

int32_t FlashInfoFree(Flashinfo_t *flashinfo)
{
    if (!flashinfo)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;
    }

    if (flashinfo->data)
    {
        free(flashinfo->data);
    }
    free(flashinfo);
    return RET_OK;
}