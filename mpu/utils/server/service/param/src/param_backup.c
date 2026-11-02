#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "sr_log.h"
#include "err_code.h"
#include "param_database.h"
#include "param_sync.h"

pthread_mutex_t paramfileLock=PTHREAD_MUTEX_INITIALIZER;

/** 判断STR参数内容是否全'0' **/
static bool param_strValueIsNotEmpty(const void *buf, size_t len) {
    if (buf == NULL || len == 0) {
        return false;
    }

    const uint8_t *p = (const uint8_t *)buf;
    for (size_t i = 0; i < len; i++) {
        if (p[i] != '0') {
            return true;
        }
    }

    return false;
}

/** 判断参数内容是否加载成功，内容是否有效 **/
bool param_loadAndCheckKeyParamValid(const char *dbfile)
{
    const uint32_t importParamList[] = {
        E_ParamId_SN,               /* Tbox Sn号 */
        E_ParamId_VIN,              /* 车辆VIN码 */
    };
    ParamInfo_t paramCell = {0};
    int paramValueLen = 0;
    bool valid = true;

    for (int i =0;i<sizeof(importParamList)/sizeof(importParamList[0]);i++)
    {
        if (ParamDatabaseLoadSingleCell(dbfile, importParamList[i], &paramCell) == false)
        {
            LOG_Error("Load param %d failed.", importParamList[i]);
            valid = false;
            break;
        }
        else
        {
            if (paramCell.type != PARAM_TYPE_STR)
            {
                LOG_Error("Param id[%d]:%s type[%d] error", paramCell.id, paramCell.name, paramCell.type);
                valid = false;
                break;
            }

            paramValueLen = strlen((const char *)paramCell.value);
            if ((paramValueLen < paramCell.min) || (paramValueLen > paramCell.max))
            {
                LOG_Error("Param id[%d]:%s valueLen[%d] is invaild", paramCell.id, paramCell.name, paramValueLen);
                valid = false;
            }

            if (!param_strValueIsNotEmpty(paramCell.value, paramValueLen))
            {
                LOG_Error("Param id[%d]:%s value[%s] is invaild", paramCell.id, paramCell.name, paramCell.value);
                valid = false;
            }
        }
    }

    return valid;
}

bool param_fileIsExist(const char *filePath)
{
    GFile *file = g_file_new_for_path(filePath);
    if (g_file_query_exists(file, NULL))
    {
        LOG_Debug("File %s exists.", filePath);
        g_object_unref(file);
        return true;
    }
    else
    {
        LOG_Debug("File %s does not exist.", filePath);
        g_object_unref(file);
        return false;
    }
}

void checkAndMkdir(const char *dirName)
{
    if (!g_file_test(dirName, G_FILE_TEST_IS_DIR))
    {
        if (g_mkdir_with_parents(dirName, 0755) == 0)
        {
            LOG_Debug("Directory created successfully or already exists: %s", dirName);
        }
        else
        {
            LOG_Debug("Error creating directory: %s", dirName);
        }
    }
    else
    {
        LOG_Debug("Directory already exists: %s", dirName);
    }
}

void *paramBackupCallBack(void *pArg)
{
    checkAndMkdir(PARAM_DICT_BACKUP_PATH);
    while (1)
    {
        sleep(BACKUP_CHECK_SECOND);
        pthread_mutex_lock(&paramfileLock);
        if (param_fileIsExist(PARAM_FILE_PATH) && ParamDatabaseHealthCheck(PARAM_FILE_PATH) && param_loadAndCheckKeyParamValid(PARAM_FILE_PATH)) // 正式文件存在并且内容合法的情况下 每30s备份一次
        {
            LOG_Warn("BackUp %s to %s.", PARAM_FILE_PATH, PARAM_FILE_BACKUP_PATH);
            ParamDatabaseBackUp(PARAM_FILE_BACKUP_PATH, PARAM_FILE_PATH);
        }
        else if (param_fileIsExist(PARAM_FILE_BACKUP_PATH) && ParamDatabaseHealthCheck(PARAM_FILE_BACKUP_PATH) && param_loadAndCheckKeyParamValid(PARAM_FILE_BACKUP_PATH)) // 正式文件不存在或者内容不合法，但是备份文件存在并且内容合法， 从备份文件恢复
        {
            LOG_Warn("Restore %s from %s.", PARAM_FILE_PATH, PARAM_FILE_BACKUP_PATH);
            ParamDatabaseBackUp(PARAM_FILE_PATH, PARAM_FILE_BACKUP_PATH);
        }
        else // 两个文件都不存在
        {
            LOG_Error("Parameter storage exception, both parameter files do not exist.");
        }
        pthread_mutex_unlock(&paramfileLock);
    }
    return NULL;
}