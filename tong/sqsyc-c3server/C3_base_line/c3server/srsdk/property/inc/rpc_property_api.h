#ifndef RPC_PROPERTY_API_H
#define RPC_PROPERTY_API_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROPERTY_RPC_KEY_MAX_LEN    64
#define PROPERTY_RPC_VALUE_MAX_LEN  256
#define PROPERTY_RPC_QUERY_MAX_NUM  20

typedef enum
{
    E_PROPERTY_MSGTYPE_SET = 1,
    E_PROPERTY_MSGTYPE_GET,
    E_PROPERTY_MSGTYPE_QUERY,
} PropertySubcmd_e;

typedef struct
{
    char key[PROPERTY_RPC_KEY_MAX_LEN];
    char value[PROPERTY_RPC_VALUE_MAX_LEN];
} PropertySetReq_t;

typedef struct
{
    int32_t result;
} PropertySetResp_t;

typedef struct
{
    char key[PROPERTY_RPC_KEY_MAX_LEN];
    char defaultValue[PROPERTY_RPC_VALUE_MAX_LEN];
} PropertyGetReq_t;

typedef struct
{
    int32_t result;
    char value[PROPERTY_RPC_VALUE_MAX_LEN];
} PropertyGetResp_t;

typedef struct
{
    int32_t offset;
    int32_t length;
} PropertyQueryReq_t;

typedef struct
{
    char key[PROPERTY_RPC_KEY_MAX_LEN];
    char value[PROPERTY_RPC_VALUE_MAX_LEN];
} PropertyItem_t;

typedef struct
{
    int32_t result;
    int32_t number;
    int32_t size;
    PropertyItem_t items[PROPERTY_RPC_QUERY_MAX_NUM];
} PropertyQueryResp_t;

/**
 * @brief 设置属性值
 *
 * @param key 属性键
 * @param value 属性值
 * @return 0成功 其他失败
 */
int32_t RpcPropertySet(const char *key, const char *value);

/**
 * @brief 设置数值类型属性值
 *
 * @param key 属性键
 * @param value 属性值
 * @return 0成功 其他失败
 */
int32_t RpcPropertySetUint64(const char *key, uint64_t value);
int32_t RpcPropertySetInt64(const char *key, int64_t value);
int32_t RpcPropertySetUint32(const char *key, uint32_t value);
int32_t RpcPropertySetInt32(const char *key, int32_t value);
int32_t RpcPropertySetUint16(const char *key, uint16_t value);
int32_t RpcPropertySetInt16(const char *key, int16_t value);
int32_t RpcPropertySetUint8(const char *key, uint8_t value);
int32_t RpcPropertySetInt8(const char *key, int8_t value);
int32_t RpcPropertySetDouble(const char *key, double value);

/**
 * @brief 获取属性值
 *
 * @param key 属性键
 * @param defaultValue 默认值
 * @param value 输出缓冲区
 * @param valueBufSize 输出缓冲区大小
 * @return 0成功 其他失败
 */
int32_t RpcPropertyGet(const char *key, const char *defaultValue, char *value, uint32_t valueBufSize);

/**
 * @brief 获取数值类型属性值
 *
 * @param key 属性键
 * @param defaultValue 默认值
 * @return 读取到的属性值，失败时返回 defaultValue
 */
uint64_t RpcPropertyGetUint64(const char *key, uint64_t defaultValue);
int64_t RpcPropertyGetInt64(const char *key, int64_t defaultValue);
uint32_t RpcPropertyGetUint32(const char *key, uint32_t defaultValue);
int32_t RpcPropertyGetInt32(const char *key, int32_t defaultValue);
uint16_t RpcPropertyGetUint16(const char *key, uint16_t defaultValue);
int16_t RpcPropertyGetInt16(const char *key, int16_t defaultValue);
uint8_t RpcPropertyGetUint8(const char *key, uint8_t defaultValue);
int8_t RpcPropertyGetInt8(const char *key, int8_t defaultValue);
double RpcPropertyGetDouble(const char *key, double defaultValue);

/**
 * @brief 分页查询属性列表
 *
 * @param offset 偏移量
 * @param length 查询数量
 * @param items 输出缓冲区
 * @param itemBufNum items缓冲区最大条目数
 * @param number 实际返回条目数（输出）
 * @param size 总条目数（输出）
 * @return 0成功 其他失败
 */
int32_t RpcPropertyQuery(int32_t offset, int32_t length, PropertyItem_t *items, uint32_t itemBufNum, int32_t *number, int32_t *size);

#ifdef __cplusplus
}
#endif

#endif // RPC_PROPERTY_API_H
