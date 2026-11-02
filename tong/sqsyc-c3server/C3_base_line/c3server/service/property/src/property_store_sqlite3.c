#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sqlite3.h>

#include "sr_log.h"
#include "property_store.h"
#include "property_util.h"

#define SQL_CMD_CREATE_TABLE  "CREATE TABLE IF NOT EXISTS %s (id INTEGER PRIMARY KEY AUTOINCREMENT,key TEXT NOT NULL UNIQUE,value TEXT NOT NULL);"
#define SQL_CMD_ADD_COLUMN    "INSERT INTO %s (key, value) VALUES (?, ?);"
#define SQL_CMD_UPDATE_COLUMN "UPDATE %s SET value = ? WHERE key = ?;"
#define SQL_CMD_QUERY_COLUMN_VALUE  "SELECT value FROM %s WHERE key = ?;"
#define SQL_CMD_IS_KEY_EXIST  "SELECT 1 FROM %s WHERE key = ?;"
#define SQL_CMD_QUERY_COLUMN  "SELECT key,value FROM %s;"
#define SQL_CMD_CLEAR_TABLE   "DELETE FROM %s;"


static int IsValidTable(const char* table)
{
    if (table == NULL)
    {
        return 0;
    }

    if ((strcmp(table, PROPERTY_TABLE_PERSIST) == 0) ||
        (strcmp(table, PROPERTY_TABLE_RO) == 0) ||
        (strcmp(table, PROPERTY_TABLE_RW) == 0))
    {
        return 1;
    }

    return 0;
}

static int Sqlite3CreateTable(sqlite3 *db, char* table){
    int ret = 0;
    char sql[128] = {0};
    sqlite3_stmt *stmt = NULL;

    if (!IsValidTable(table))
    {
        LOG_Error("[%s] invalid table: %s", __func__, table ? table : "(null)");
        return -1;
    }

    snprintf(sql, sizeof(sql), SQL_CMD_CREATE_TABLE, table);

    ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        LOG_Error("[%s] SQL prepare error: %s\n", __func__, sqlite3_errmsg(db));
        return ret;
    }

    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        LOG_Error("[%s] SQL step error: %s\n", __func__, sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return (ret == SQLITE_DONE) ? 0 : ret;
}

static int Sqlite3IsKeyExist(sqlite3 *db, char* table, char* key)
{
    int ret = 0;
    char sql[128] = {0};
    sqlite3_stmt *stmt = NULL;

    if (key == NULL || !IsValidTable(table))
    {
        return 0;
    }

    snprintf(sql, sizeof(sql), SQL_CMD_IS_KEY_EXIST, table);

    ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        LOG_Error("[%s] SQL prepare error: %s\n", __func__, sqlite3_errmsg(db));
        return 0;
    }

    ret = sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
    if (ret != SQLITE_OK)
    {
        LOG_Error("[%s] SQL bind error: %s\n", __func__, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    ret = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (ret == SQLITE_ROW) ? 1 : 0;
}

static int Sqlite3InsertColumn(sqlite3 *db, char* table, char* key, char* value)
{
    int ret = 0;
    char sql[288] = {0};
    sqlite3_stmt *stmt = NULL;

    if (key == NULL || value == NULL || !IsValidTable(table))
    {
        return -1;
    }

    snprintf(sql, sizeof(sql), SQL_CMD_ADD_COLUMN, table);

    ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        LOG_Error("[%s] SQL prepare error: %s\n", __func__, sqlite3_errmsg(db));
        return ret;
    }

    ret = sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
    if (ret != SQLITE_OK)
    {
        LOG_Error("[%s] SQL bind error[key]: %s\n", __func__, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ret;
    }

    ret = sqlite3_bind_text(stmt, 2, value, -1, SQLITE_STATIC);
    if (ret != SQLITE_OK)
    {
        LOG_Error("[%s] SQL bind error[value]: %s\n", __func__, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ret;
    }

    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        LOG_Error("[%s] SQL step error: %s\n", __func__, sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return (ret == SQLITE_DONE) ? 0 : ret;
}

static int Sqlite3UpdateColumn(sqlite3 *db, char* table, char* key, char* value)
{
    int ret = 0;
    char sql[288] = {0};
    sqlite3_stmt *stmt = NULL;

    if (key == NULL || value == NULL || !IsValidTable(table))
    {
        return -1;
    }

    snprintf(sql, sizeof(sql), SQL_CMD_UPDATE_COLUMN, table);

    ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        LOG_Error("[%s] SQL prepare error: %s\n", __func__, sqlite3_errmsg(db));
        return ret;
    }

    ret = sqlite3_bind_text(stmt, 1, value, -1, SQLITE_STATIC);
    if (ret != SQLITE_OK)
    {
        LOG_Error("[%s] SQL bind error[value]: %s\n", __func__, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ret;
    }

    ret = sqlite3_bind_text(stmt, 2, key, -1, SQLITE_STATIC);
    if (ret != SQLITE_OK)
    {
        LOG_Error("[%s] SQL bind error[key]: %s\n", __func__, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return ret;
    }

    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        LOG_Error("[%s] SQL step error: %s\n", __func__, sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return (ret == SQLITE_DONE) ? 0 : ret;
}

static int Sqlite3QueryTable(sqlite3 *db,  char* table, PropStoreQueryCallBack_t cbFunc, void* cbArg)
{
    int ret = 0;
    char sql[128] = {0};
    sqlite3_stmt *stmt = NULL;

    if (!IsValidTable(table))
    {
        LOG_Error("[%s] invalid table: %s", __func__, table ? table : "(null)");
        return -1;
    }

    snprintf(sql, sizeof(sql), SQL_CMD_QUERY_COLUMN, table);

    ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        LOG_Error("[%s] SQL prepare error: %s\n", __func__, sqlite3_errmsg(db));
        return ret;
    }

    while ((ret = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const unsigned char *key = sqlite3_column_text(stmt, 0);
        const unsigned char *value = sqlite3_column_text(stmt, 1);

        if ((key != NULL) && (value != NULL) && (cbFunc != NULL))
        {
            cbFunc((char*)key, (char*)value, cbArg);
        }
    }

    if (ret != SQLITE_DONE)
    {
        LOG_Error("[%s] SQL step error: %s\n", __func__, sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return (ret == SQLITE_DONE) ? 0 : ret;
}

static int Sqlite3ClearTable(sqlite3 *db, char* table)
{
    int ret = 0;
    char sql[128] = {0};
    sqlite3_stmt *stmt = NULL;

    if (!IsValidTable(table))
    {
        LOG_Error("[%s] invalid table: %s", __func__, table ? table : "(null)");
        return -1;
    }

    snprintf(sql, sizeof(sql), SQL_CMD_CLEAR_TABLE, table);

    ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        LOG_Error("[%s] SQL prepare error: %s\n", __func__, sqlite3_errmsg(db));
        return ret;
    }

    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE)
    {
        LOG_Error("[%s] SQL step error: %s\n", __func__, sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return (ret == SQLITE_DONE) ? 0 : ret;
}

/********************  sqlite3_property__store_interface ********************/

int Sqlite3PropertyStoreInit(void* dev, char* storePath)
{
    int ret = 0;
    sqlite3 **db = (sqlite3 **)dev;

    /** 检查并创建数据库所在文件夹 **/
    ret = CreateFileDirectory(storePath);
    if (ret != 0)
    {
        return ret;
    }

    ret = sqlite3_open(storePath, db);
    if (ret != SQLITE_OK)
    {
        LOG_Error("[%s] sqlite3_open error: %s", __func__, sqlite3_errmsg(*db));
        (void)sqlite3_close(*db);
        *db = NULL;
        return ret;
    }

    ret = Sqlite3CreateTable(*db, PROPERTY_TABLE_PERSIST);
    if(ret !=0)
    {
        (void)sqlite3_close(*db);
        *db = NULL;
        return ret;
    }

    ret = Sqlite3CreateTable(*db, PROPERTY_TABLE_RO);
    if(ret !=0)
    {
        (void)sqlite3_close(*db);
        *db = NULL;
        return ret;
    }

    ret = Sqlite3CreateTable(*db, PROPERTY_TABLE_RW);
    if(ret !=0)
    {
        (void)sqlite3_close(*db);
        *db = NULL;
        return ret;
    }

    return 0;
}

int Sqlite3PropertyStoreClear(void* dev, char* table)
{
    int ret = 0;
    sqlite3 *db = (sqlite3 *)dev;

    if (!IsValidTable(table))
    {
        LOG_Error("[%s] invalid table: %s", __func__, table ? table : "(null)");
        return -1;
    }

    ret = Sqlite3ClearTable(db, table);

    return ret;
}

int Sqlite3PropertyStoreSet(void* dev, char* table, char* key, char* value)
{
    int ret = 0;
    sqlite3 *db = (sqlite3 *)dev;

    if (!IsValidTable(table))
    {
        LOG_Error("[%s] invalid table: %s", __func__, table ? table : "(null)");
        return -1;
    }

    if (Sqlite3IsKeyExist(db, table, key))
    {
        ret = Sqlite3UpdateColumn(db, table, key, value);
    }
    else
    {
        ret = Sqlite3InsertColumn(db, table, key, value);
    }

    return ret;
}

int Sqlite3PropertyStoreQuery(void* dev, char* table, PropStoreQueryCallBack_t cbFunc, void* cbArg)
{
    int ret = 0;
    sqlite3 *db = (sqlite3 *)dev;

    if (!IsValidTable(table))
    {
        LOG_Error("[%s] invalid table: %s", __func__, table ? table : "(null)");
        return -1;
    }

    ret = Sqlite3QueryTable(db, table, cbFunc, cbArg);

    return ret;
}

int Sqlite3PropertyStoreDeinit(void** dev)
{
    sqlite3 *db = (sqlite3*) *dev;
    int ret = 0;

    if (db == NULL)
    {
        return 0;
    }

    ret = sqlite3_close(db);
    if (ret != SQLITE_OK)
    {
        LOG_Error("[%s] sqlite3_close error: %d", __func__, ret);
    }

    /* 无论关闭是否成功，句柄都不再可用，必须清空，避免重复释放或悬空指针 */
    *dev = NULL;
    return ret;
}

const PropStoreInterface_t g_sqlite3_property_store_interface = {
    .init = Sqlite3PropertyStoreInit,
    .clear = Sqlite3PropertyStoreClear,
    .set = Sqlite3PropertyStoreSet,
    .query = Sqlite3PropertyStoreQuery,
    .deinit = Sqlite3PropertyStoreDeinit,    
};
