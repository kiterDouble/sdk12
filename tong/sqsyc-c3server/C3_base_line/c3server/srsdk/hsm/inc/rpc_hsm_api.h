#ifndef RPC_HSM_API_H
#define RPC_HSM_API_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 证书和秘钥安装不同区域 **/
#define KEYID_DIGITALKEY_PKI_TEMP   0x01  /** 数字钥匙 临时通讯秘钥**/
#define KEYID_DIGITALKEY_PKI_CA     0x02  /** 数字钥匙 PKI_CA证书公钥 **/
#define KEYID_DIGITALKEY_PKI_KEY    0x03  /** 数字钥匙 PKI KEY **/

#define CRTID_DIGITALKEY_PKI    0x01  /** 数字钥匙PKI CRT **/


#define CONFIG_PATH_MAX_LEN       256
#define RANDOM_DATA_MAX_LEN       2048
#define RANDOM_DATA_MIN_LEN       4
#define PUB_KEY_MAX_LEN           512
#define QUERY_HSM_MAX_LEN         512
#define RSA_OUTPUT_MAX_LEN        256
#define RSA_LARGE_OUTPUT_MAX_LEN  1024
#define SIGNATURE_MAX_LEN         256
#define DATA_MAX_LEN              4096

#define EXPORT_KEY_MAX_LEN        1024
#define IMPORT_KEY_MAX_LEN        2048

#define EXPORT_CRT_MAX_LEN        4032
#define IMPORT_CRT_MAX_LEN        4032

#define CRT_SLOT_COUNT            32         // 证书槽数量
#define KEY_SLOT_COUNT            256         // 密钥槽数量
#define KEY_INFO_SIZE             2            // 每条密钥信息大小（字节）
#define QUERY_KEY_DATA_LEN        (KEY_SLOT_COUNT * KEY_INFO_SIZE)  // 512字节
#define KEY_NOT_EXIST             0xFFFF       // 密钥不存在标识

typedef enum
{
    E_HSM_MSGTYPE_RANDOM = 1,
    E_HSM_MSGTYPE_GEN_KEY,
    E_HSM_MSGTYPE_RSA_ENCRYPT,
    E_HSM_MSGTYPE_RSA_DECRYPT,
    E_HSM_MSGTYPE_SIGN,
    E_HSM_MSGTYPE_VERIFY,

    E_HSM_MSGTYPE_QUERY_KEY,
    E_HSM_MSGTYPE_DELETE_KEY,
    E_HSM_MSGTYPE_EXPORT_KEY,
    E_HSM_MSGTYPE_IMPORT_KEY,

    E_HSM_MSGTYPE_QUERY_CRT,
    E_HSM_MSGTYPE_DELETE_CRT,
    E_HSM_MSGTYPE_EXPORT_CRT,
    E_HSM_MSGTYPE_IMPORT_CRT,
    E_HSM_MSGTYPE_QUERY_KEY_EXISTS,
    E_HSM_MSGTYPE_QUERY_CRT_EXISTS,
    E_HSM_MSGTYPE_GET_WORK_STATE,
} HsmSubcmd_e;

typedef enum
{
    E_HSM_WORK_STATE_NORMAL  = 0x00,
    E_HSM_WORK_STATE_FAULT   = 0x01,
    E_HSM_WORK_STATE_UNKNOWN = 0x02,
} HsmWorkState_e;

typedef enum{
    E_ALG_NONE = 0x0,
    E_ALG_RSA1024 = 0x1,
    E_ALG_RSA2048 = 0x2,
}RsaKeyAlg_e;

typedef enum{
    E_RSA_SIGN_PADDING_NONE = 0x0,
    E_RSA_SIGN_PADDING_OAEP = 0x1,
    E_RSA_SIGN_PADDING_PSS = 0x2,
    E_RSA_SIGN_PADDING_PKCS1 = 0x3,
}RsaSignPaddingAlg_e;

typedef enum{
    E_DIGIST_NONE = 0x0,
    E_DIGIST_SHA1 = 0x1,
    E_DIGIST_SHA224 = 0x2,
    E_DIGIST_SHA256 = 0x3,
    E_DIGIST_SHA384 = 0x4,
    E_DIGIST_SHA512 = 0x5,
    E_DIGIST_SM3 = 0x6,
}DigistAlg_e;

typedef enum
{
	E_KEY_TYPE_PRI            = 0x00,    
	E_KEY_TYPE_PUB            = 0x01,
    E_KEY_TYPE_PRI_PUB_PAIR   = 0x02,
}KeyType_e;


/**
 * @brief 密钥算法枚举
 */
typedef enum {
    E_HSM_KEY_ALG_RSA = 0,   /** RSA 算法 **/
    E_HSM_KEY_ALG_SM2,       /** SM2 国密算法 **/
    E_HSM_KEY_ALG_ECC,       /** ECC 椭圆曲线算法 **/
    E_HSM_KEY_ALG_AES,       /** AES 对称算法 **/
    E_HSM_KEY_ALG_SM4,       /** SM4 国密对称算法 **/
    E_HSM_KEY_ALG_3DES,      /** 3DES 对称算法 **/
    E_HSM_KEY_ALG_MAX,
} HsmKeyAlg_e;

/**
 * @brief 密钥类型枚举
 */
typedef enum {
    E_HSM_KEY_TYPE_PUBLIC = 0,   /** 公钥 **/
    E_HSM_KEY_TYPE_PRIVATE,      /** 私钥 **/
    E_HSM_KEY_TYPE_KEY_PAIR,     /** 密钥对 **/
    E_HSM_KEY_TYPE_SECRET,       /** 对称密钥/秘密密钥 **/
    E_HSM_KEY_TYPE_MAX,
} HsmKeyType_e;


/** 通用响应结果 **/
typedef struct
{
    int32_t result;
} CommonResp_t;

/** 获取HSM工作状态响应 **/
typedef struct
{
    int32_t result;
    uint8_t state;  // E_HSM_WORK_STATE_*
} HsmWorkStateResp_t;


/** 生成密钥对 请求 和 响应**/
typedef struct
{
    uint8_t alg;
    uint16_t keyId;
} GenKeyReq_t;

typedef struct
{
    int32_t result;
    uint32_t pubKeyLen;
    uint8_t  pubKey[PUB_KEY_MAX_LEN];
} GenKeyResp_t;


/** RSA加密 请求 和 响应**/
typedef struct
{
    uint8_t alg;
    uint16_t keyId;
    uint8_t padding;
    uint32_t dataLen;
    uint8_t plaintext[DATA_MAX_LEN];
} RsaEncryptReq_t;

typedef struct
{
    int32_t result;
    int32_t encLen;
    uint8_t ciphertext[RSA_LARGE_OUTPUT_MAX_LEN];
} RsaEncryptResp_t;

/** RSA解密请求 和 响应**/
typedef struct
{
    uint8_t alg;
    uint16_t keyId;
    uint8_t padding;
    uint32_t dataLen;
    uint8_t ciphertext[DATA_MAX_LEN];
} RsaDecryptReq_t;

typedef struct
{
    int32_t result;
    int32_t decLen;
    uint8_t plaintext[RSA_LARGE_OUTPUT_MAX_LEN];
} RsaDecryptResp_t;


/** RSA签名 请求 和 响应**/
typedef struct
{
    uint8_t alg;
    uint16_t keyId;
    uint8_t digist;
    uint32_t dataLen;
    uint8_t data[DATA_MAX_LEN];
} SignReq_t;

typedef struct
{
    int32_t result;
    uint32_t signLen;
    uint8_t signature[SIGNATURE_MAX_LEN];
} SignResp_t;

/** 随机数 **/
typedef struct
{
    int32_t result;
    uint32_t randomLen;
    uint8_t randomData[RANDOM_DATA_MAX_LEN];
} RandomResp_t;



/** 导出密钥 响应 **/
typedef struct
{
    int32_t result;
    uint32_t outBufLen;
    uint8_t outBuf[EXPORT_KEY_MAX_LEN];
} ExportKeyResp_t;

typedef struct
{
    // uint8_t uKeyAlg;
    // uint16_t uKeyId;
    uint8_t exKeyAlg;
    uint16_t exKeyId;
    uint8_t exKeyType;
    // bool ifCipher;
    // bool ifTransKey;
} ExportKeyReq_t;

/** 导入密钥请求 **/
typedef struct
{
    uint8_t inKeyAlg;
    uint16_t inKeyId;
    uint8_t inKeyType;
    uint32_t inKeyValLen;
    uint8_t inKeyVal[IMPORT_KEY_MAX_LEN];
} ImportKeyReq_t;

/** RSA验签请求 **/
typedef struct
{
    uint8_t alg;
    uint16_t keyId;
    uint8_t digist;
    uint32_t dataLen;
    uint8_t data[DATA_MAX_LEN];
    uint32_t signLen;
    uint8_t signature[SIGNATURE_MAX_LEN];
} VerifyReq_t;

/**
 * @brief 密钥信息结构体
 * @note 算法与密钥类型解耦，不再暴露芯片私有 algId 编码
 */
typedef struct {
    uint8_t       keyId;      /** 密钥ID (0-255) **/
    HsmKeyAlg_e   keyAlg;     /** 密钥算法 **/
    HsmKeyType_e  keyType;    /** 公钥/私钥/密钥对 **/
    uint32_t      keyBits;    /** 密钥位数（RSA模长/椭圆曲线位长等） **/
} KeyInfo_t;

/**
 * @brief 密钥查询结果结构体
 */
typedef struct {
    uint8_t  validKeyCount;        // 有效密钥数量
    KeyInfo_t keyList[KEY_SLOT_COUNT]; // 有效密钥列表
} QueryKeyResult_t;

/** 查询秘钥信息 **/
typedef struct
{
    int32_t result;
    uint8_t validKeyCount;
    KeyInfo_t keyList[KEY_SLOT_COUNT];
} QueryKeyResp_t;

typedef struct {
    uint8_t  validCrtCount;        // 有效证书数量
    uint16_t crtList[CRT_SLOT_COUNT]; // 有效证书列表
} QueryCrtResult_t;

/** 查询证书 响应 **/
typedef struct
{
    int32_t result;
    uint8_t validCrtCount;
    uint16_t crtList[CRT_SLOT_COUNT];
} QueryCrtResp_t;

/** 导出证书 请求 和 响应 **/
typedef struct
{
    uint16_t crtId;
} ExportCrtReq_t;

typedef struct
{
    int32_t result;
    uint32_t outBufLen;
    uint8_t outBuf[EXPORT_CRT_MAX_LEN];
} ExportCrtResp_t;

/** 导入证书请求 **/
typedef struct
{
    uint16_t crtId;
    uint32_t crtValLen;
    uint8_t crtVal[EXPORT_CRT_MAX_LEN];
} ImportCrtReq_t;

/**
 * @brief 生成随机数
 * @param expectedLen 期望的随机数长度
 * @param random 输出缓冲区
 * @return 0成功 -2 RPC接口错误 其他失败
 */
int32_t RpcHsmRandom(uint32_t expectedLen, uint8_t *random);

/**
 * @brief 生成RSA密钥对
 * @param alg 密钥算法
 * @param keyId 密钥ID
 * @param pubKeyDer 公钥输出缓冲区（X.509 SubjectPublicKeyInfo DER格式）
 * @param pubKeyDerBufSize 输入时为缓冲区大小，输出时为实际公钥长度
 * @return 0成功 -2 RPC接口错误 其他失败
 */
int32_t RpcHsmGenKey(RsaKeyAlg_e alg, uint16_t keyId, 
                                uint8_t* pubKeyDer, uint32_t *pubKeyDerBufSize);

/**
 * @brief RSA公钥加密
 * @param alg 加密算法
 * @param keyId 密钥ID
 * @param padding 填充方式
 * @param plaintext 明文数据
 * @param plainLen 明文长度
 * @param ciphertext 密文输出缓冲区
 * @param cipherLen 密文长度输出
 * @return 0成功 -2 RPC接口错误 其他失败
 */
int32_t RpcHsmEncrypt(RsaKeyAlg_e alg, uint16_t keyId, uint8_t padding, const uint8_t *plaintext, uint32_t plainLen,
                                 uint8_t *ciphertext, uint32_t *cipherLen);

/**
 * @brief RSA私钥解密
 * @param alg 加密算法
 * @param keyId 密钥ID
 * @param padding 填充方式
 * @param ciphertext 密文数据
 * @param cipherLen 密文长度
 * @param plaintext 明文输出缓冲区
 * @param plainLen 明文长度输出
 * @return 0成功 -2 RPC接口错误 其他失败
 */
int32_t RpcHsmDecrypt(RsaKeyAlg_e alg, uint16_t keyId, uint8_t padding, const uint8_t *ciphertext, uint32_t cipherLen,
                                 uint8_t *plaintext, uint32_t *plainLen);

/**
 * @brief RSA签名
 * @param alg 密钥算法
 * @param keyId 密钥ID
 * @param digist 摘要算法
 * @param data 待签名数据
 * @param dataLen 数据长度
 * @param signature 签名输出缓冲区
 * @param signLen 输入时为缓冲区大小，输出时为实际签名长度
 * @return 0成功 -2 RPC接口错误 其他失败
 */
int32_t RpcHsmSign(RsaKeyAlg_e alg, uint16_t keyId, DigistAlg_e digist, const uint8_t *data, uint32_t dataLen,
                              uint8_t *signature, uint32_t *signLen);

/**
 * @brief RSA验签
 * @param alg 密钥算法
 * @param keyId 密钥ID
 * @param digist 摘要算法
 * @param data 待验签数据
 * @param dataLen 数据长度
 * @param signature 签名数据
 * @param signLen 签名长度
 * @return 0成功 1验签失败 -2 RPC接口错误 其他失败
 */
int32_t RpcHsmVerify(RsaKeyAlg_e alg, uint16_t keyId, DigistAlg_e digist, const uint8_t *data, uint32_t dataLen,
                                uint8_t *signature, uint32_t signLen);


/**
 * @brief 查询密钥并解析结果
 * @param keyResult 密钥查询结果输出
 * @return 0成功 -2 RPC接口错误 其他失败
 */
int32_t RpcHsmQueryKey(QueryKeyResult_t *keyResult);

/**
 * @brief 查询密钥是否存在
 * @param keyId 密钥ID
 * @return 1存在 0不存在 -2 RPC接口错误
 */
int RpcHsmQueryKeyExists(uint16_t keyId);

/**
 * @brief 密钥删除
 * @param keyId 密钥ID
 * @return 0成功 -2 RPC接口错误 其他失败
 */
 int32_t RpcHsmDeleteKey(uint16_t keyId);

/**
 * @brief 导出密钥
 * @param exKeyAlg 导出密钥算法
 * @param exKeyId 导出密钥ID
 * @param exKeyType 导出密钥类型
 *                - E_KEY_TYPE_PUB (0x01): 输出 X.509 SubjectPublicKeyInfo DER
 *                - E_KEY_TYPE_PRI (0x00): 输出 PKCS#8 PrivateKeyInfo DER
 *                - E_KEY_TYPE_PRI_PUB_PAIR (0x02): 输出 PKCS#8 DER
 * @param outBuf 密钥数据输出缓冲区
 * @param outBufLen 输入时为缓冲区大小，输出时为实际数据长度
 * @return 0成功 -2 RPC接口错误 其他失败
 */
int32_t RpcHsmExportKey(RsaKeyAlg_e exKeyAlg, uint16_t exKeyId, KeyType_e exKeyType, uint8_t *outBuf, uint32_t *outBufLen);

/**
 * @brief 导入密钥（公钥 / 私钥 / 密钥对）
 * @param inKeyAlg 导入密钥算法（E_ALG_RSA1024 / E_ALG_RSA2048）
 * @param inKeyId 导入密钥ID
 * @param inKeyType 导入密钥类型
 *                - E_KEY_TYPE_PUB  (0x01): X.509 SubjectPublicKeyInfo DER
 *                - E_KEY_TYPE_PRI  (0x00): PKCS#8 PrivateKeyInfo DER
 *                - E_KEY_TYPE_PRI_PUB_PAIR (0x02): PKCS#8 DER
 * @param inKeyVal 导入密钥值（X.509 DER / PKCS#8 DER）
 * @param inKeyValLen 导入密钥值长度
 * @return 0成功 -2 RPC接口错误 其他失败
 */
int32_t RpcHsmImportKey(RsaKeyAlg_e inKeyAlg, uint16_t inKeyId, KeyType_e inKeyType, const uint8_t *inKeyVal, uint32_t inKeyValLen);



/**
 * @brief 查询证书并解析结果
 * @param crtResult 证书查询结果输出
 * @return 0成功 -2 RPC接口错误 其他失败
 */
int32_t RpcHsmQueryCrt(QueryCrtResult_t *crtResult);

/**
 * @brief 查询证书是否存在
 * @param crtId 证书ID
 * @return 1存在 0不存在  -2 RPC接口错误
 */
int RpcHsmQueryCrtExists(uint16_t crtId);

/**
 * @brief 删除证书
 * @param crtId 证书ID
 * @return 0成功 -2 RPC接口错误 其他失败
 */
int32_t RpcHsmDeleteCrt(uint16_t crtId);

/**
 * @brief 导出证书
 * @param exCrtId 导出证书ID
 * @param outBuf 证书数据输出缓冲区
 * @param outBufLen 输入时为缓冲区大小，输出时为实际证书长度
 * @return 0成功 -2 RPC接口错误 其他失败
 */
int32_t RpcHsmExportCrt(uint16_t exCrtId, uint8_t *outBuf, uint32_t *outBufLen);

/**
 * @brief 导入证书
 * @param inCrtId 导入证书ID
 * @param inCrtVal 证书数据
 * @param inCrtValLen 证书数据长度
 * @return 0成功 -2 RPC接口错误 其他失败
 */
int32_t RpcHsmImportCrt(uint16_t inCrtId, const uint8_t *inCrtVal, uint32_t inCrtValLen);

/**
 * @brief 获取HSM工作状态
 * @param state 输出状态值，E_HSM_WORK_STATE_NORMAL/FAULT/UNKNOWN
 * @return 0成功，-2 RPC接口错误，其他失败
 */
int32_t RpcHsmGetWorkState(uint8_t *state);


#ifdef __cplusplus
}
#endif

#endif // RPC_HSM_API_H
