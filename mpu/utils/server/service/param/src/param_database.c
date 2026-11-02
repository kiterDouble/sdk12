#include <glib.h>
#include <glib/gprintf.h>
#include <pthread.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sr_log.h"
#include "err_code.h"
#include "param_database.h"
#include "param_sync.h"
#include "param_backup.h"
#include "param_encrypt.h"
#include "stdint.h"
#include "sr_param.h"

#define TABLENAME "paramstbl"
#define TABLENAME_BAK "paramstbl_bak"
#define true 1
#define false 0
#define INT_TO_STR(x) #x
#define PARAM_INT_TO_STR(x) INT_TO_STR(x)

static ParamInfo_t g_paramArray[E_ParamId_UserEnd + 1];
pthread_rwlock_t g_paramArryRWLock = PTHREAD_RWLOCK_INITIALIZER;
extern pthread_mutex_t paramfileLock;

static const char *title[1][8] = {
    {"id", "name", "value", "type", "min", "max", "paramSources", "rpcWriteEnable"},
};
#define STATEMENT "id integer primary key, name text, value text, type integer, min integer, max integer, paramSources integer, rpcWriteEnable integer"

const char *element[][8] = {
    {"0", "E_ParamId_ICCID", "00000000000000000000", "0", "20", "20", "0", "1"},
    {"1", "E_ParamId_IMEI", "000000000000000", "0", "1", "15", "0", "1"},
    {"2", "E_ParamId_IMSI", "000000000000000", "0", "1", "15", "0", "1"},
    {"3", "E_ParamId_SN", "000000000000000000000000000", "0", "1", "27", "2", "1"},
    {"4", "E_ParamId_VIN", "00000000000000000", "0", "17", "17", "2", "1"},
    {"5", "E_ParamId_TSPAddr", "tbox.tspaddr.com", "0", "4", "64", "2", "1"},
    {"6", "E_ParamId_TSPPort", "8888", "1", "0", "65535", "2", "1"},
    {"7", "E_ParamId_GB32960Addr", "tbox.gb32960addr.com", "0", "4", "64", "2", "1"},
    {"8", "E_ParamId_GB329060Port", "8888", "1", "0", "65535", "2", "1"},
    {"9", "E_ParamId_HJ1239Addr", "tbox.hj1239addr.com", "0", "4", "64", "2", "1"},
    {"10", "E_ParamId_HJ1239Port", "8888", "1", "0", "65535", "2", "1"},
    {"11", "E_ParamId_ECallNumber", "00000000000000000", "0", "3", "17", "2", "1"},
    {"12", "E_ParamId_BCallNumber", "00000000000000000", "0", "3", "17", "2", "1"},
    {"13", "E_ParamId_ICallNumber", "00000000000000000", "0", "3", "17", "2", "1"},
    {"14", "E_ParamId_SW_Version", "127005010010_025", "0", "0", "64", "2", "1"},
    {"15", "E_ParamId_HW_Version", "xxx", "0", "0", "64", "2", "1"},
    {"16", "E_ParamId_CustomSW_Version", "xxx", "0", "0", "64", "2", "1"},
    {"17", "E_ParamId_CarVersion", "xxx", "0", "0", "64", "2", "1"},
    {"18", "E_ParamId_ManufactureData", "2023/12/25", "0", "10", "10", "2", "1"},
    {"19", "E_ParamId_ParatNumber", "xxx", "0", "0", "64", "2", "1"},
    {"20", "E_ParamId_SupIdentifier", "xxx", "0", "0", "64", "2", "1"},
    {"21", "E_ParamId_ecallConfigEnable", "1", "1", "0", "1", "2", "1"},
    {"22", "E_ParamId_ecallConfigVoiceConf", "0", "1", "0", "1", "2", "1"},
    {"23", "E_ParamId_ecallConfigMode", "0", "1", "0", "1", "2", "1"},
    {"24", "E_ParamId_ecallConfigProcessinfo", "1", "1", "0", "1", "2", "1"},
    {"25", "E_ParamId_ecallConfigStartTimer", "5", "1", "0", "65535", "2", "1"},
    {"26", "E_ParamId_ecallConfigHackTimer", "5", "1", "0", "65535", "2", "1"},
    {"27", "E_ParamId_ecallConfigMsdTransmission", "20", "1", "0", "65535", "2", "1"},
    {"28", "E_ParamId_ecallConfigMoFailRedial", "1", "1", "0", "1", "2", "1"},
    {"29", "E_ParamId_ecallConfigDropRedial", "1", "1", "0", "1", "2", "1"},
    {"30", "E_ParamId_ecallConfigCleardownTimer", "3600", "1", "0", "65535", "2", "1"},
    {"31", "E_ParamId_ecallConfigInitiationTimer", "2", "1", "0", "65535", "2", "1"},
    {"32", "E_ParamId_ecallConfigNadRegistrationTimer", "3600", "1", "0", "65535", "2", "1"},
    {"33", "E_ParamId_ecallConfigNadUnregistrationTimer", "43200", "1", "0", "65535", "2", "1"},
    {"34", "E_ParamId_ecallConfigSystem", "0", "1", "0", "1", "2", "1"},
    {"35", "E_ParamId_ecallConfigEraGlonassRedialTimer", "3", "1", "0", "65535", "2", "1"},
    {"36", "E_ParamId_ecallConfigEraGlonassAutoAnswerTimer", "2", "1", "0", "65535", "2", "1"},
    {"37", "E_ParamId_ecallConfigRedialDurationTimer", "120", "1", "0", "65535", "2", "1"},
    {"38", "E_ParamId_ecallConfigTestMode", "0", "1", "0", "1", "2", "1"},
    {"39", "E_ParamId_ecallConfigTestNum", "112", "0", "3", "20", "2", "1"},
    {"40", "E_ParamId_netAllowDial", "0", "1", "0", "1", "2", "1"},
    {"41", "E_ParamId_paramEncryptStatus", "0", "1", "0", "2", "2", "1"},
    {"42", "E_ParamId_ecallConfigCallAutoAnswerTime", "20", "1", "0", "65535", "3", "1"},
    {"43", "E_ParamId_ecallConfigPosTestRegistrationTime", "120", "1", "0", "65535", "3", "1"},
    {"44", "E_ParamId_ecallConfigTestModeEndDistance", "300", "1", "0", "65535", "3", "1"},
    {"45", "E_ParamId_ecallConfigIntMemTransmitInterval", "60", "1", "0", "65535", "3", "1"},
    {"46", "E_ParamId_ecallConfigIntMemTransmitAttempts", "10", "1", "0", "65535", "3", "1"},
    {"47", "E_ParamId_ecallConfigCCFT", "60", "1", "0", "65535", "3", "1"},
    {"48", "E_ParamId_ecallConfigMSDMaxTransmissionTime", "20", "1", "0", "65535", "3", "1"},
    {"49", "E_ParamId_ecallConfigNADDeregistrationTime", "3600", "1", "0", "65535", "3", "1"},
    {"50", "E_ParamId_ecallConfigEcallNoAutomaticTriggering", "1", "1", "0", "1", "3", "1"},
    {"51", "E_ParamId_ecallConfigEcallDialDiration", "5", "1", "0", "65535", "3", "1"},
    {"52", "E_ParamId_ecallConfigEcallAutoDialAttempts", "10", "1", "0", "65535", "3", "1"},
    {"53", "E_ParamId_ecallConfigEcallManualDialAttempts", "10", "1", "0", "10", "3", "1"},
    {"54", "E_ParamId_ecallConfigEcallManualCanCancel", "1", "1", "0", "1", "3", "1"},
    {"55", "E_ParamId_ecallConfigEcallSMSFallbackNumber", "112", "0", "0", "20", "3", "1"},
    {"56", "E_ParamId_ecallConfigTestRegistrationPeriod", "5", "1", "0", "65535", "3", "1"},
    {"57", "E_ParamId_ecallConfigGNSSDataRate", "1", "1", "0", "65535", "3", "1"},
    {"58", "E_ParamId_ecallConfigVehicleType", "M1", "0", "0", "16", "3", "1"},
    {"59", "E_ParamId_ecallConfigVehiclePropulsionStorageType", "1", "1", "1", "7", "3", "1"}};

static int32_t GetRowData(void* param, int32_t column, char **columnValue, char **columName)
{
    int32_t id = 0;
    id = atoi(columnValue[0]);
    g_paramArray[id].id = id;
    strcpy((char *)g_paramArray[id].name, (char *)columnValue[1]);
    strcpy((char *)g_paramArray[id].value, (char *)columnValue[2]);
    g_paramArray[id].type = atoi(columnValue[3]);
    g_paramArray[id].min = atoi(columnValue[4]);
    g_paramArray[id].max = atoi(columnValue[5]);
    g_paramArray[id].paramSources = atoi(columnValue[6]);
    g_paramArray[id].rpcWriteEnable = atoi(columnValue[7]) == 0 ? false : true;
    LOG_Debug("id:[%d] name:[%s] value:[%s] type:[%d] min:[%d] max:[%d]", id, g_paramArray[id].name, g_paramArray[id].value, g_paramArray[id].type,
              g_paramArray[id].min, g_paramArray[id].max);
    return SQLITE_OK;
}

//将表格数据读出到g_paramArray
static bool LoadParamToMem()
{
    sqlite3 *database;
    // sqlite3_stmt *stmt;
    int32_t rc;
    // uint8_t *error;
    // bool runing = true;
    char sql[256] = {0x00};
    char *errmsg;
    sprintf(sql, "select %s,%s,%s,%s,%s,%s,%s,%s from %s order by id",
            title[0][0], title[0][1], title[0][2], title[0][3],
            title[0][4], title[0][5], title[0][6], title[0][7],
            TABLENAME);

    if ((rc = sqlite3_open(PARAM_FILE_PATH, &database)) != SQLITE_OK)
    {
        LOG_Error("* ERROR (%d) - sqlite3_open() failed in %s.", rc, __func__);
        return false;
    }

    pthread_rwlock_wrlock(&g_paramArryRWLock);
    memset(&g_paramArray, 0x00, sizeof(g_paramArray));
    rc = sqlite3_exec(database, sql, GetRowData, NULL, &errmsg);
    if (rc != SQLITE_OK)
    {
        LOG_Error("Load param err:%s", errmsg);
        sqlite3_free(errmsg);
    }
    pthread_rwlock_unlock(&g_paramArryRWLock);
    sqlite3_close(database);
    return true;
}

static bool LoadUnsetParamToMem(char* dbfile, int32_t start_id, int32_t len)
{
    sqlite3 *database;
    int32_t rc;
    char sql[256] = {0x00};
    char *errmsg;
    sprintf(sql, "select %s,%s,%s,%s,%s,%s,%s,%s from %s where id between %d and %d order by id",
        title[0][0], title[0][1], title[0][2], title[0][3],
        title[0][4], title[0][5], title[0][6], title[0][7],
        TABLENAME, start_id, start_id + len);

    if ((rc = sqlite3_open(dbfile, &database)) != SQLITE_OK)
    {
        LOG_Error("* ERROR (%d) - sqlite3_open() failed in %s.", rc, __func__);
        return false;
    }

    pthread_rwlock_wrlock(&g_paramArryRWLock);
    rc = sqlite3_exec(database, sql, GetRowData, NULL, &errmsg);
    if (rc != SQLITE_OK)
    {
        LOG_Error("Load param err:%s", errmsg);
        sqlite3_free(errmsg);
    }
    pthread_rwlock_unlock(&g_paramArryRWLock);
    sqlite3_close(database);
    return true;
}

//判断当前数据库中的基础参数和默认基础参数是否相同，不同说明数据库中的比较新，返回true
static bool ParamIsChanged()
{
    bool result = false;
    pthread_rwlock_rdlock(&g_paramArryRWLock);
    for (int i = 0; i < E_ParamId_enumParamEnd; i++)
    {
        if ((i != atoi(element[i][0])) ||
            (strcmp((const char *)g_paramArray[i].name, (const char *)element[i][1]) != 0))
        {
            /**id */
            result = true;
            LOG_Warn("Param is changed ,will be rebuild and reload");
            break;
        }
    }
    pthread_rwlock_unlock(&g_paramArryRWLock);
    return result;
}

//获取sql返回的行数
int32_t GetCountNumBySql(const char *dbfile,const char *sql)
{
    sqlite3 *CountDb;
    sqlite3_stmt *stmt;
    bool Runing = true;
    int32_t ret = 0;
    int32_t Count = 0;

    ret = sqlite3_open(dbfile, &CountDb);
    if (ret != SQLITE_OK)
    {

        sqlite3_close(CountDb);
        return -1;
    }

    while (1)
    {
        ret = sqlite3_prepare(CountDb, sql, -1, &stmt, NULL);
        if (ret == SQLITE_BUSY)
        {
            //usleep(10*ONE_MS);
            continue;
        }
        else if (ret == SQLITE_OK)
        {
            break;
        }
        else
        {

            sqlite3_finalize(stmt);
            sqlite3_close(CountDb);
            return -1;
        }
    }

    while (Runing)
    {
        ret = sqlite3_step(stmt);
        switch (ret)
        {
        case SQLITE_BUSY:
            sqlite3_reset(stmt);
            Runing = true;
            break;
        case SQLITE_DONE:
            Count = 0;

            Runing = false;
            break;
        case SQLITE_ROW:

            Count = sqlite3_column_int(stmt, 0);
            Runing = false;
            break;
        default:

            Runing = false;
            break;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(CountDb);
    return Count;
}

//执行sql但并不关心任何返回
int32_t ExecNoReturnSql(const char *sql)
{
    sqlite3 *database;
    int32_t rc;
    char *error;
    /* Open the sqlite db */
    LOG_Debug("SQL Exec [%s]", sql);
    //如果 new_database.db 这个文件不存在，SQLite 将会创建一个空的数据库文件，并将其命名为 new_database.db
    if ((rc = sqlite3_open(PARAM_FILE_PATH, &database)) != SQLITE_OK)
    {
        LOG_Error("* ERROR (%d) - sqlite3_open() failed in %s.", rc, __func__);
        return rc;
    }
    /* Create a bak table */
    if ((rc = sqlite3_exec(database, (const char *)sql, NULL, NULL, &error)) != SQLITE_OK)
    {
        sqlite3_close(database);
        LOG_Error("* ERROR (%d) - sqlite3_exec() failed in %s.", rc, __func__);
        return rc;
    }
    if ((rc = sqlite3_close(database)) != SQLITE_OK)
    {
        LOG_Error("* ERROR (%d) - sqlite3_close() failed in %s.", rc, __func__);
        return rc;
    }
    LOG_Debug("SQL Exec [%s] over", sql);
    return SQLITE_OK;
}

int32_t DropTable(const char *tableName)
{
    char droptableSql[256] = {0x00};
    // sqlite3 *database;
    // int32_t rc;
    // uint8_t *error;
    sprintf(droptableSql, "drop table if exists %s", tableName);
    return ExecNoReturnSql(droptableSql);
}

int32_t CreateBakTableFrom(const char *tableName)
{
    char createBakTable[256] = {0x00};
    sprintf(createBakTable, "create table if not exists %s as select * from %s", TABLENAME_BAK, tableName);
    return ExecNoReturnSql(createBakTable);
}

int32_t DeleateAllElementFromTaleb(const char *tableName)
{
    char deleteAllElement[256] = {0x00};
    sprintf(deleteAllElement, "delete from  %s", tableName);
    return ExecNoReturnSql(deleteAllElement);
}

int32_t DeleteUserParamWithId(const uint32_t id)
{
    if (id < E_ParamId_UserStart || id > E_ParamId_UserEnd)
        return RET_ERR;
    char deleteAllElement[256] = {0x00};
    sprintf(deleteAllElement, "delete from %s where id = %u", TABLENAME, id);
    pthread_mutex_lock(&paramfileLock);
    if(SQLITE_OK == ExecNoReturnSql(deleteAllElement))
    {
        pthread_mutex_unlock(&paramfileLock);
        pthread_rwlock_wrlock(&g_paramArryRWLock);
        memset(&g_paramArray[id], 0x00, sizeof(ParamInfo_t));
        pthread_rwlock_unlock(&g_paramArryRWLock);
        return RET_OK;
    }
    pthread_mutex_unlock(&paramfileLock);
    return RET_ERR;
}

//将默认数据插入新建的空表
static void InsertDefaultData()
{
#if 0
    char sql[512 * E_ParamId_enumParamEnd] = {0x00};
    char value[512] = {0x00};
    sprintf(sql, "insert into %s(%s,%s,%s,%s,%s,%s,%s,%s) values", TABLENAME,
            title[0][0], title[0][1], title[0][2], title[0][3],
            title[0][4], title[0][5], title[0][6], title[0][7]);
    for (int32_t i = 0; i < E_ParamId_enumParamEnd; i++)
    {
        memset(value, 0x00, sizeof(value));
        sprintf(value, "(%s,\"%s\",\"%s\",%s,%s,%s,%s,%s)", element[i][0], element[i][1], element[i][2], element[i][3], element[i][4], element[i][5], element[i][6], element[i][7]);
        if (i != E_ParamId_enumParamEnd - 1)
        {
            strcat(value, ",");
        }
        strcat(sql, value);
    }
    ExecNoReturnSql(sql);
#endif
    LOG_Warn("InsertDefaultData Restore %s from %s.", PARAM_FILE_PATH, PARAM_FILE_OEMAPP_PATH);
    ParamDatabaseBackUp(PARAM_FILE_PATH, PARAM_FILE_OEMAPP_PATH);
}

//从备份表中选择数据，将其更新到目标表中
//只会针对备份表中已存在于目标表中的行进行操作
int32_t UpdateFromBakDb(const char *tableName, const char *bakName)
{
    char update[1024] = {0x00};
    sprintf(update, "update %s set value=(select value from %s where %s.name=%s.name),\
                    type=(select type from %s where %s.name=%s.name),\
                    min=(select min from %s where %s.name=%s.name),\
                    max=(select max from %s where %s.name=%s.name),\
                    paramSources=(select paramSources from %s where %s.name=%s.name),\
                    rpcWriteEnable=(select rpcWriteEnable from %s where %s.name=%s.name) \
                    where %s.name in (select name from %s);",
            tableName, bakName, tableName, bakName,
            bakName, tableName, bakName,
            bakName, tableName, bakName,
            bakName, tableName, bakName,
            bakName, tableName, bakName,
            bakName, tableName, bakName,
            tableName, bakName);
    return ExecNoReturnSql(update);
}

//从备份表中将所有用户数据拷贝出来给目标表
int32_t UpdateUserParamsFromBakDb(const char *tableName, const char *bakName)
{
    char update[512] = {0x00};
    sprintf(update, "INSERT INTO %s (%s, %s, %s, %s, %s, %s, %s, %s) \
            SELECT %s, %s, %s, %s, %s, %s, %s, %s \
            FROM %s \
            WHERE id > %u AND id < %u;",
            tableName,
            title[0][0], title[0][1], title[0][2], title[0][3],
            title[0][4], title[0][5], title[0][6], title[0][7],
            title[0][0], title[0][1], title[0][2], title[0][3],
            title[0][4], title[0][5], title[0][6], title[0][7],
            bakName,
            (uint32_t)E_ParamId_UserStart - 1,
            (uint32_t)E_ParamId_UserEnd + 1);
    return ExecNoReturnSql(update);
}

void FreeSpace(void)
{
    ExecNoReturnSql("VACUUM");
}

static bool CheckParamIsVaild(DeviceParamEnum paramName, void* paramValue)
{
    bool result = true;
    if(g_paramArray[paramName].rpcWriteEnable == false)
    {
        LOG_Error("Parameter %d is not allowed to be modified", (uint32_t)paramName);
        return false;
    }
    guint limit = 0;
    pthread_rwlock_rdlock(&g_paramArryRWLock);
    switch (g_paramArray[paramName].type)
    {
    case PARAM_TYPE_INT:
        limit = *(guint *)paramValue;
        if ((limit < g_paramArray[paramName].min) || (limit > g_paramArray[paramName].max))
        {
            LOG_Error("Param id[%d],value[%d] is invaild,min or max", paramName, limit);
            result = false;
        }
        else if (limit == atoi((char *)g_paramArray[paramName].value))
        {
            LOG_Error("Param id[%d],value[%d] is invaild,equal", paramName, limit);
            result = true;
        }
        break;
    case PARAM_TYPE_STR:
        limit = strlen((const char *)paramValue);
        if ((limit < g_paramArray[paramName].min) || (limit > g_paramArray[paramName].max))
        {
            LOG_Error("Param id[%d],valueLen[%d] is invaild,min[%d] or max[%d]", paramName, limit, g_paramArray[paramName].min, g_paramArray[paramName].max);
            result = false;
        }
        else if (strcmp((char *)g_paramArray[paramName].value, (const char *)paramValue) == 0)
        {
            LOG_Error("Param id[%d],valueLen[%d] is invaild,equal", paramName, limit);
            result = true;
        }
        break;
    default:
        LOG_Error("Unknown param type:%d", g_paramArray[paramName].type);
        break;
    }
    pthread_rwlock_unlock(&g_paramArryRWLock);
    return result;
}

static void UpdateParamToDb(DeviceParamEnum paramName, void* paramValue)
{
    char updateSql[512] = {0x00};
    sprintf(updateSql, "update %s set value='%s' where id=%d", TABLENAME, (char *)paramValue, paramName);
    ExecNoReturnSql(updateSql);
}

static void UpdateToDbSingleParam(DeviceParamEnum paramName, ParamInfo_t param)
{
    char sql[512] = {0x00};
    char value[512] = {0x00};
    sprintf(sql, "insert into %s(%s,%s,%s,%s,%s,%s,%s,%s) values", TABLENAME,
            title[0][0], title[0][1], title[0][2], title[0][3],
            title[0][4], title[0][5], title[0][6], title[0][7]);

    memset(value, 0x00, sizeof(value));
    sprintf(value, "(%d,\"%s\",\"%s\",%d,%d,%d,%d,%d)",
            param.id, param.name, param.value, 
            param.type, param.min, param.max, 
            param.paramSources, param.rpcWriteEnable);

    strcat(sql, value);
    ExecNoReturnSql(sql);
}

/**
 * @brief 检查：
 * 1.oemdata中是否存在参数数据库，如果存在的话退出
 * 2.如果oemdata中不存在参数数据库，然后检查oemapp中是否存在，存在的话将oemapp中的数据库拷贝到oemdata
 * 3.如果oemapp和oemdata都不存在参数数据库，则退出，后面会用代码中的默认参数创建参数数据库
 * 
 */
void checkParamFile()
{
    if(param_fileIsExist(PARAM_FILE_PATH) && ParamDatabaseHealthCheck(PARAM_FILE_PATH))
    {
        return;
    }
    if(param_fileIsExist(PARAM_FILE_BACKUP_PATH) && ParamDatabaseHealthCheck(PARAM_FILE_BACKUP_PATH) && param_loadAndCheckKeyParamValid(PARAM_FILE_BACKUP_PATH))
    {
        ParamDatabaseBackUp(PARAM_FILE_PATH,PARAM_FILE_BACKUP_PATH);
        LOG_Warn("Restore %s from %s.", PARAM_FILE_PATH, PARAM_FILE_BACKUP_PATH);
    }
    else if(param_fileIsExist(PARAM_FILE_OEMAPP_PATH))
    {
        ParamDatabaseBackUp(PARAM_FILE_PATH,PARAM_FILE_OEMAPP_PATH);
        LOG_Warn("Restore %s from %s.", PARAM_FILE_PATH, PARAM_FILE_OEMAPP_PATH);
    }
    else
    {
        return;
    }
}

int32_t updateUnsetParamsFromOemapp(int32_t dataCount, int32_t rowCountDiff)
{
    //如果数据库中缺少rowCountDiff个数据，就从oemapp里面的数据库中读出dataCount ~ dataCount+rowCountDiff里面的value到oemdata里面
    //1.从oemapp拷贝数据库作为临时查询数据库，因为oemapp只读
    if(!param_fileIsExist(PARAM_FILE_OEMAPP_PATH))
    {
        return RET_ERR;
    }
    char dest_path[256];
    snprintf(dest_path, sizeof(dest_path), "%s_oemapp", PARAM_FILE_PATH);
    ParamDatabaseBackUp(dest_path, PARAM_FILE_OEMAPP_PATH);
    //2.获取oemapp数据库中基础参数数量
    char paramsSql[256] = {0x00};
    sprintf(paramsSql, "select count(*) from %s where id < %d", TABLENAME, E_ParamId_BaseEnd);
    int32_t oemappParamCount = GetCountNumBySql(dest_path, paramsSql);
    //3.计算差几个
    int32_t countDiff = oemappParamCount - dataCount;
    if(countDiff <= 0)
    {
        return RET_ERR;
    }
    //4.获取多的几个id数据到内存
    if(countDiff > rowCountDiff)
    {
        //特殊情况：oemapp db参数数量>代码中默认表数量>oemdata db参数数量，限制最终数量到代码中默认表数量
        countDiff = rowCountDiff;
    }
    LoadUnsetParamToMem(dest_path, dataCount, countDiff);
    //5.更新到数据库
    for (int i = 0; i < countDiff; i++)
    {
        UpdateParamToDb(dataCount + i, g_paramArray[dataCount + i].value);
    }
    return RET_OK;
}

//读数据库到g_paramArray
int32_t ParamsDbInit(void)
{
    bool useDefault = false;
    int32_t dataCount = 0;
    int32_t rowCountDiff = 0;
    char paramsSql[256] = {0x00};

    for (int i = 0; i < E_ParamId_UserEnd + 1; i++)
    {
        g_paramArray[i].paramSources = PARAM_SOURCES_DB;
    }

    //不存在/oemdata/parameters目录的话就创建
    checkAndMkdir(PARAM_DICT_PATH);

    //根据oemapp和oemdata中的参数文件进行处理
    checkParamFile();

    //1.初始化paramstbl表格
    sprintf(paramsSql, "CREATE TABLE IF NOT EXISTS %s(%s)", TABLENAME, STATEMENT);
    ExecNoReturnSql(paramsSql);
    // pthread_rwlock_init(&g_paramArryRWLock, NULL);

    //2.获取表格中基础参数的参数数量
    sprintf(paramsSql, "select count(*) from %s where id < %d", TABLENAME, E_ParamId_BaseEnd);
    dataCount = GetCountNumBySql(PARAM_FILE_PATH ,paramsSql);
    //默认表和数据库中参数数量差值
    rowCountDiff = sizeof(element) / sizeof(element[0]) - dataCount;
    LOG_Debug("dataCount: %d rowCountDiff: %d", dataCount, rowCountDiff);
    if (0 == dataCount)
    {
        //当表格为空时 将默认数据插入表格
        InsertDefaultData();
        useDefault = true;
    }
    LOG_Debug("start load param to mem");
    //3.将表格数据读出到g_paramArray
    if (!LoadParamToMem())
    {
        LOG_Error("Load param into mem failed");
    }
    //4.如果程序开始前数据库有数据 将原有数据中有效的（符合默认表中定义的name的）数据筛选出来
    //通过默认数据为模板进行格式化，然后重新更新paramstbl表，重新读取表格
    //通过这一步保证不合定义的参数不被处理（比如参数被人为写错ID的情况）
    if ((!useDefault) && (ParamIsChanged()))
    {
        LOG_Warn("param changed");
        /**param is changed*/
        /*check paramstbl data*/
        //删除paramstbl_bak表
        DropTable(TABLENAME_BAK);
        //根据paramstbl表创建bak备份表
        if (CreateBakTableFrom(TABLENAME) != SQLITE_OK)
        {
            return RET_ERR;
        }
        //删除paramstbl表中所有数据
        DeleateAllElementFromTaleb(TABLENAME);
        //插入默认数据
        InsertDefaultData();
        //从备份表中筛选有效数据修改到paramstbl
        UpdateFromBakDb(TABLENAME, TABLENAME_BAK);
        //从备份表中将所有用户数据拷贝出来给目标表
        UpdateUserParamsFromBakDb(TABLENAME, TABLENAME_BAK);
        //删除paramstbl_bak备份表
        DropTable(TABLENAME_BAK);
        //优化 SQLite 数据库文件的大小，通过执行 VACUUM 命令来释放未使用的空间
        FreeSpace();
        //将表格数据读出到g_paramArray
        LoadParamToMem();
    }
    if(rowCountDiff > 0 && dataCount != 0)
    {
        updateUnsetParamsFromOemapp(dataCount, rowCountDiff);
    }

    return RET_OK;
}

void hexToString(const uint8_t *data, size_t len, char *output)
{
    const char hexDigits[] = "0123456789ABCDEF";

    for (size_t i = 0; i < len; ++i)
    {
        output[2 * i] = hexDigits[(data[i] >> 4) & 0x0F];
        output[2 * i + 1] = hexDigits[data[i] & 0x0F];
    }

    output[2 * len] = '\0'; // Null-terminate the string
}

static uint8_t hexCharToNibble(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return 0; // Invalid hex character, handle as needed
}

void stringToHEX(const char *hexString, uint8_t *output, size_t *len)
{
    size_t strLen = strlen(hexString);
    if (strLen % 2 != 0)
    {
        printf("Invalid hex string length.\n");
        *len = 0;
        return;
    }

    *len = strLen / 2;
    for (size_t i = 0; i < *len; ++i)
    {
        uint8_t highNibble = hexCharToNibble(hexString[2 * i]);
        uint8_t lowNibble = hexCharToNibble(hexString[2 * i + 1]);
        output[i] = (highNibble << 4) | lowNibble;
    }
}

int32_t ParamsEncryptHexToStr(unsigned char *plaintextStr, unsigned char *outStr)
{
    int32_t ciphertext_len;
    unsigned char ciphertext[256] = {0};
    ciphertext_len = ParamAES256CBCEncrypt(plaintextStr, strlen((const char *)plaintextStr), ciphertext);
    if (RET_ERR == ciphertext_len)
    {
        return RET_ERR;
    }
    char output[2 * ciphertext_len + 1];
    hexToString(ciphertext, ciphertext_len, output);
    LOG_Debug("encrypt out str:%s", output);
    strcpy((char *)outStr, (const char *)output);
    return RET_OK;
}

int32_t ParamsDecryptToText(unsigned char *inHexStr, unsigned char *text)
{
    size_t len;
    uint8_t hexData[256] = {0}; // Buffer for output
    stringToHEX((const char *)inHexStr, hexData, &len);
    int decryptedtext_len;
    decryptedtext_len = ParamAES256CBCDecrypt(hexData, len, text);
    if (RET_ERR == decryptedtext_len)
    {
        return RET_ERR;
    }
    return decryptedtext_len;
}

void ParamsEncryptInit()
{
    //1.首先判断E_ParamId_paramEncryptStatus 0：不需要加密关键数据 1：需要加密关键数据且暂时未加密 2：已加密关键数据
    int8_t encryptStatus = atoi((const char *)g_paramArray[E_ParamId_paramEncryptStatus].value);
    if(1 == encryptStatus)
    {
        //需要加密关键数据，且关键数据还没有被加密
        //vin
        uint8_t val[256] = {0};
        if(RET_ERR == ParamsEncryptHexToStr(g_paramArray[E_ParamId_VIN].value,val))
        {
            return;
        }
        memset(g_paramArray[E_ParamId_VIN].value, 0x00, sizeof(g_paramArray[E_ParamId_VIN].value));
        strcpy((char *)g_paramArray[E_ParamId_VIN].value, (const char *)val);
        UpdateParamToDb(E_ParamId_VIN, val);
        
        //然后更新E_ParamId_paramEncryptStatus 变更为已加密
        int32_t paramValue = 2;
        memset(g_paramArray[E_ParamId_paramEncryptStatus].value, 0x00, sizeof(g_paramArray[E_ParamId_paramEncryptStatus].value));
        sprintf((char *)g_paramArray[E_ParamId_paramEncryptStatus].value, "%d", paramValue);
        UpdateParamToDb(E_ParamId_paramEncryptStatus, g_paramArray[E_ParamId_paramEncryptStatus].value);
    }
}

bool GetParams(ParamInfo_t *para)
{
    uint32_t arg_paramIndex = para->id;
    pthread_rwlock_rdlock(&g_paramArryRWLock);
    ParamInfo_t emptyParam;
    memset(&emptyParam, 0, sizeof(ParamInfo_t));
    if(memcmp(&(g_paramArray[arg_paramIndex]), &emptyParam, sizeof(ParamInfo_t)) == 0)
    {
        para->id = E_ParamId_InvalidId;
    }
    else
    {
        *para = g_paramArray[arg_paramIndex];
        int8_t encryptStatus = atoi((const char *)g_paramArray[E_ParamId_paramEncryptStatus].value);
        if(E_ParamId_VIN == (DeviceParamEnum)arg_paramIndex && 2 == encryptStatus)
        {
            //解密
            uint8_t output[256] = {0}; // Buffer for output
            if (RET_ERR == ParamsDecryptToText(g_paramArray[arg_paramIndex].value, output))
            {
                pthread_rwlock_unlock(&g_paramArryRWLock);
                return false;
            }
            LOG_Debug("Param AES256CBC Decrypt: vin:%s", output);
            memset(para->value,0x00,sizeof(para->value));
            strcpy((char *)(para->value), (const char *)output);
        }
    }
    pthread_rwlock_unlock(&g_paramArryRWLock);
    return TRUE;
}

/*get params*/
int32_t GetParamsValue(DeviceParamEnum paramName, void* paramValue)
{
    if (paramName > E_ParamId_UserEnd)
    {
        LOG_Warn("Requset Param Id over Max:%d", paramName);
        return RET_ERR;
    }
    if (paramValue == NULL)
    {
        LOG_Warn("The paramValue is NULL Point ,Please Check");
        return RET_ERR;
    }
    pthread_rwlock_rdlock(&g_paramArryRWLock);
    int8_t encryptStatus = atoi((const char *)g_paramArray[E_ParamId_paramEncryptStatus].value);
    if (E_ParamId_VIN == paramName && 2 == encryptStatus)
    {
        // 解密
        uint8_t output[256] = {0}; // Buffer for output
        if (RET_ERR == ParamsDecryptToText(g_paramArray[paramName].value, output))
        {
            pthread_rwlock_unlock(&g_paramArryRWLock);
            return RET_ERR;
        }
        LOG_Debug("Param AES256CBC Decrypt: vin:%s", output);
        strcpy((char *)paramValue, (const char *)output);
        pthread_rwlock_unlock(&g_paramArryRWLock);
        return RET_OK;
    }
    switch (g_paramArray[paramName].type)
    {
    case PARAM_TYPE_INT:
        *(int32_t *)paramValue = atoi((const char *)g_paramArray[paramName].value);
        break;
    case PARAM_TYPE_STR:
        strcpy((char *)paramValue, (char *)g_paramArray[paramName].value);
        break;
    default:
        LOG_Warn("Unknown param type:%d", g_paramArray[paramName].type);
        break;
    }
    pthread_rwlock_unlock(&g_paramArryRWLock);
    return RET_OK;
}
/*set params*/
int32_t SetParamsValue(DeviceParamEnum paramName, void* paramValue, bool syncToMcu)
{
    if (paramName > E_ParamId_UserEnd)
    {
        LOG_Error("Set Param Id over Max:%d", paramName);
        return RET_ERR;
    }
    if (paramValue == NULL)
    {
        LOG_Error("Set Param:The paramValue is NULL Point ,Please Check");
        return RET_ERR;
    }
    LOG_Debug("Set Param[%d] start", paramName);
    if (false == CheckParamIsVaild(paramName, paramValue))
    {
        LOG_Error("CheckParamIsVaild");
        return RET_ERR;
    }
    LOG_Debug("CheckParam is OK");
    char paramDbValue[256] = {0x00};
    // unsigned char paramFlag = 0;
    pthread_rwlock_wrlock(&g_paramArryRWLock);
    memset(g_paramArray[paramName].value, 0x00, sizeof(g_paramArray[paramName].value));
    switch (g_paramArray[paramName].type)
    {
        case PARAM_TYPE_INT:
        {
            sprintf((char *)g_paramArray[paramName].value, "%d", *(int32_t *)paramValue);
            break;
        }
        case PARAM_TYPE_STR:
        {
            int8_t encryptStatus = atoi((const char *)g_paramArray[E_ParamId_paramEncryptStatus].value);
            if (paramName == E_ParamId_VIN && encryptStatus == 2)
            {
                uint8_t val[256] = {0};
                if (RET_ERR == ParamsEncryptHexToStr(paramValue, val))
                {
                    LOG_Error("Param AES256CBC Decrypt ERROR!");
                    strcpy((char *)g_paramArray[paramName].value, (const char *)paramValue);
                    break;
                }
                LOG_Debug("vin encrypt:%s", val);
                strcpy((char *)g_paramArray[paramName].value, (const char *)val);
            }
            else
            {
                strcpy((char *)g_paramArray[paramName].value, (const char *)paramValue);
            }
            break;
        }
        default:
        {
            LOG_Warn("Unknown param type:%d", g_paramArray[paramName].type);
            break;
        }
    }
    strcpy(paramDbValue, (const char *)g_paramArray[paramName].value);
    pthread_rwlock_unlock(&g_paramArryRWLock);

    pthread_mutex_lock(&paramfileLock);
    if (param_fileIsExist(PARAM_FILE_PATH) && ParamDatabaseHealthCheck(PARAM_FILE_PATH) )
    {
        UpdateParamToDb(paramName, paramDbValue);
    }
    else if (param_fileIsExist(PARAM_FILE_BACKUP_PATH) && ParamDatabaseHealthCheck(PARAM_FILE_BACKUP_PATH) && param_loadAndCheckKeyParamValid(PARAM_FILE_BACKUP_PATH))
    {
        LOG_Warn("Restore %s from %s.", PARAM_FILE_PATH, PARAM_FILE_BACKUP_PATH);
        ParamDatabaseBackUp(PARAM_FILE_PATH, PARAM_FILE_BACKUP_PATH);
        UpdateParamToDb(paramName, paramDbValue);
    }
    else // 两个文件都不存在
    {
        LOG_Error("Parameter storage exception, both parameter files do not exist. Update Param To Db failed!!");
    }
    pthread_mutex_unlock(&paramfileLock);

    if(syncToMcu)
    {
        SyncParamToMcu(paramName, paramValue); //同步参数
    }

    if (paramName < E_ParamId_BaseEnd)
    {
        ParamInfo_t paramUpdate = {0};
        paramUpdate.id = paramName;
        GetParams(&paramUpdate);
        SendMsgToAllQueue(SYS_QUE_TYPE_PARAM_UPDATE, (void *)&paramUpdate, sizeof(ParamInfo_t));
    }

    return RET_OK;
}

int32_t SetUserParams(ParamInfo_t param)
{
    DeviceParamEnum paramName = (DeviceParamEnum)param.id;
    if (paramName > E_ParamId_UserEnd || paramName < E_ParamId_UserStart)
    {
        LOG_Error("Set Param Id over Max:%d", paramName);
        return RET_ERR;
    }
    LOG_Debug("Set Param[%d] start", paramName);
    pthread_rwlock_wrlock(&g_paramArryRWLock);
    if (g_paramArray[paramName].value[0] == '\0')
    {
        g_paramArray[paramName] = param;
    }
    else
    {
        LOG_Error("Set Param[%d] false, duplicate param", paramName);
        pthread_rwlock_unlock(&g_paramArryRWLock);
        return RET_ERR;
    }
    pthread_rwlock_unlock(&g_paramArryRWLock);
    pthread_mutex_lock(&paramfileLock);
    UpdateToDbSingleParam(paramName, param);
    pthread_mutex_unlock(&paramfileLock);
    return RET_OK;
}


int32_t GetParamsValueType(DeviceParamEnum paramIndex, uint8_t *type)
{
    if (paramIndex > E_ParamId_UserEnd || paramIndex < E_ParamId_ICCID)
    {
        LOG_Error("Set Param Id over Max:%d", paramIndex);
        return RET_ERR;
    }
    ParamInfo_t emptyParam;
    memset(&emptyParam, 0, sizeof(ParamInfo_t));
    if (memcmp(&(g_paramArray[paramIndex]), &emptyParam, sizeof(ParamInfo_t)) == 0)
    {
        LOG_Error("param does not exist:id:%d", paramIndex);
        return RET_ERR;
    }
    if (g_paramArray[paramIndex].type == PARAM_TYPE_INT)
    {
        *type = PARAM_TYPE_INT; 
    }
    else
    {   
        *type = PARAM_TYPE_STR;
    }
    return RET_OK;
}

uint8_t GetParamsSources(DeviceParamEnum paramIndex)
{
    return g_paramArray[paramIndex].paramSources;
}

/* 数据库健康检查 回调函数：接收 PRAGMA integrity_check 的返回结果 */
static int ParamDatabaseIntegrityCheckCallback(void *data, int argc, char **argv, char **azColName)
{
    bool *isHealthy = (bool *)data;

    /* PRAGMA integrity_check 的返回规则：
     - 正常时返回单行单列的 "ok"
     - 异常时返回具体的错误描述（如 "database disk image is malformed"）*/
    if (argc > 0 && argv[0] != NULL) {
        if (strcmp(argv[0], "ok") != 0) {
            LOG_Error("Database integrity error: %s", argv[0]);
            *isHealthy = false;
        }
    }
    return 0;
}

/* 数据库健康检查函数*/
bool ParamDatabaseHealthCheck(const char* dbPath)
{
    sqlite3 *db = NULL;
    char *errmsg = NULL;
    int rc;
    bool isHealthy = true; // 默认假设数据库健康

    // 1. 尝试打开数据库
    rc = sqlite3_open(dbPath, &db);
    if (rc != SQLITE_OK) {
        LOG_Error("Failed to open database: %s, error: %s", dbPath, sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    // 2. 执行完整性检查（传入回调函数接收结果）
    rc = sqlite3_exec(db, "PRAGMA integrity_check", ParamDatabaseIntegrityCheckCallback, &isHealthy, &errmsg);
    if (rc != SQLITE_OK) {
        LOG_Error("Failed to execute integrity check: %s", errmsg);
        sqlite3_free(errmsg);
        sqlite3_close(db);
        return false;
    }

    // 3. 根据回调函数的结果判断数据库是否健康
    if (!isHealthy) {
        LOG_Error("Database integrity check failed: %s", dbPath);
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    return true;
}

/* 加载单个参数 */
bool ParamDatabaseLoadSingleCell(char* dbfile, int32_t targetId, ParamInfo_t *paramCell)
{
    sqlite3 *database = NULL;
    sqlite3_stmt *stmt = NULL;
    int32_t rc;
    char sql[256] = {0};
    bool ret = false;

    do {
        if (dbfile == NULL || paramCell == NULL)
        {
            LOG_Error("Invalid input params: dbfile=%p, paramCell=%p", dbfile, paramCell);
            break;
        }

        // 2. 初始化输出结构体（避免脏数据）
        memset(paramCell, 0, sizeof(ParamInfo_t));

        // 3. 拼接SQL（使用占位符，避免注入）
        snprintf(sql, sizeof(sql), "select %s,%s,%s,%s,%s,%s,%s,%s from %s where id = ?",
                title[0][0], title[0][1], title[0][2], title[0][3],
                title[0][4], title[0][5], title[0][6], title[0][7],
                TABLENAME);

        // 4. 打开数据库
        rc = sqlite3_open(dbfile, &database);
        if (rc != SQLITE_OK)
        {
            LOG_Error("Open db failed: %s, id=%d, err=%s", dbfile, targetId, sqlite3_errmsg(database));
            break;
        }

        // 5. 准备预处理语句
        rc = sqlite3_prepare_v2(database, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            LOG_Error("Prepare SQL failed: %s, id=%d, err=%s", sql, targetId, sqlite3_errmsg(database));
            break;
        }

        // 6. 绑定ID参数（第1个占位符?）
        sqlite3_bind_int(stmt, 1, targetId);

        // 7. 执行查询并读取结果
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) // 查到数据行
        {
            paramCell->id = (uint32_t)sqlite3_column_int(stmt, 0);
            const char *nameStr = (const char*)sqlite3_column_text(stmt, 1);
            if (nameStr != NULL)
            {
                strncpy((char*)paramCell->name, nameStr, sizeof(paramCell->name)-1);
            }
            const char *valueStr = (const char*)sqlite3_column_text(stmt, 2);
            if (valueStr != NULL)
            {
                strncpy((char*)paramCell->value, valueStr, sizeof(paramCell->value)-1);
            }

            paramCell->type = (uint8_t)sqlite3_column_int(stmt, 3);
            paramCell->min = (int32_t)sqlite3_column_int(stmt, 4);
            paramCell->max = (int32_t)sqlite3_column_int(stmt, 5);
            paramCell->paramSources = (uint8_t)sqlite3_column_int(stmt, 6);
            paramCell->rpcWriteEnable = (sqlite3_column_int(stmt, 7) != 0) ? true : false;

            LOG_Info("Load param success: id=%d, name=%s, type=%d", targetId, paramCell->name, paramCell->type);
            ret = true;
        }
        else if (rc == SQLITE_DONE) // 无匹配数据
        {
            LOG_Error("No param found for id=%d in table %s", targetId, TABLENAME);
            ret = false;
        }
        else // 查询出错
        {
            LOG_Error("Step SQL failed: id=%d, err=%s", targetId, sqlite3_errmsg(database));
            ret = false;
        }

    } while (0);

    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }
    if (database != NULL)
    {
        sqlite3_close(database);
        database = NULL;
    }

    return ret;
}


bool ParamDatabaseBackUp(const char *dstDbPath,const char *srcDbPath)
{
    sqlite3 *srcDb = NULL;
    sqlite3 *dstDb = NULL;
    sqlite3_backup *pBackup = NULL;
    bool backupSuccess = false;
    int rc;

    do {
        if (srcDbPath == NULL || dstDbPath == NULL) {
            LOG_Error("Invalid input: srcDbPath=%p, dstDbPath=%p", srcDbPath, dstDbPath);
            break;
        }

        if (access(dstDbPath, F_OK) == 0) { // 检查目标文件是否存在
            if (remove(dstDbPath) != 0) {
                LOG_Error("Failed to delete target file: %s (permission/locked?)", dstDbPath);
                break; // 删除失败
            }
            LOG_Info("Old backup file %s deleted", dstDbPath);
        }

        // ========== 第三步：打开源数据库（只读模式） ==========
        rc = sqlite3_open_v2(srcDbPath, &srcDb,  SQLITE_OPEN_READONLY | SQLITE_OPEN_URI, NULL);
        if (rc != SQLITE_OK) {
            LOG_Error("Open source db failed: %s, err: %s", srcDbPath, sqlite3_errmsg(srcDb));
            break;
        }

        // ========== 第四步：打开目标数据库（不存在则自动创建） ==========
        rc = sqlite3_open_v2(dstDbPath, &dstDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL);
        if (rc != SQLITE_OK) {
            LOG_Error("Open dest db failed: %s, err: %s", dstDbPath, sqlite3_errmsg(dstDb));
            break;
        }

        // ========== 第五步：初始化备份对象 ==========
        pBackup = sqlite3_backup_init(dstDb, "main", srcDb, "main");
        if (pBackup == NULL) {
            LOG_Error("Backup init failed: err: %s", sqlite3_errmsg(dstDb));
            break;
        }

        rc = sqlite3_backup_step(pBackup, -1);
        if (rc != SQLITE_DONE) {
            LOG_Error("Backup step failed: err: %s, remaining pages: %d", sqlite3_errmsg(dstDb), sqlite3_backup_remaining(pBackup));
            break;
        }

        // ========== 备份成功 ==========
        LOG_Info("Backup success! %s -> %s", srcDbPath, dstDbPath);
        backupSuccess = true;

    } while (0);

    if (pBackup != NULL) {
        sqlite3_backup_finish(pBackup);
        pBackup = NULL;
    }
    if (srcDb != NULL) {
        sqlite3_close(srcDb);
        srcDb = NULL;
    }
    if (dstDb != NULL) {
        sqlite3_close(dstDb);
        dstDb = NULL;
    }

    return backupSuccess;
}
