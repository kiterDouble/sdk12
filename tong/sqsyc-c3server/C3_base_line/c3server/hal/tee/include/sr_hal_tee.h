#ifndef _SR_HAL_TEE_H
#define _SR_HAL_TEE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief TEE Keymaster 初始化/去初始化
 */
int SrHalTeeInit(void);
void SrHalTeeRelease(void);

/*
 * @brief 获取随机数
 */
int SrHalTeeRandom(uint32_t expectedLen, uint8_t *random);

/*
 * @brief RSA 密钥对生成
 * @param bits 密钥位数，如 1024/2048
 * @param pubDer 输出公钥 DER（X.509 SubjectPublicKeyInfo），函数内部分配内存
 * @param pubDerLen 输出公钥长度
 * @param keyBlob 输出 TEE key blob，函数内部分配内存
 * @param keyBlobLen 输出 blob 长度
 * @return 0 成功，其他失败
 */
int SrHalTeeRsaGenKey(uint32_t bits, uint8_t **pubDer, uint32_t *pubDerLen,
                      uint8_t **keyBlob, uint32_t *keyBlobLen);

/*
 * @brief 导入 PKCS#8 DER 私钥
 * @param pkcs8Der PKCS#8 DER 数据
 * @param derLen 数据长度
 * @param keyBlob 输出 TEE key blob，函数内部分配内存
 * @param keyBlobLen 输出 blob 长度
 * @return 0 成功，其他失败
 */
int SrHalTeeImportKey(const uint8_t *pkcs8Der, uint32_t derLen, uint8_t **keyBlob, uint32_t *keyBlobLen);

/*
 * @brief 销毁 TEE key blob
 */
int SrHalTeeDestroyKey(uint8_t *keyBlob, uint32_t keyBlobLen);

/*
 * @brief 从 keypair blob 中导出公钥（X.509 SubjectPublicKeyInfo DER）
 * @param keyBlob 密钥对 blob
 * @param keyBlobLen blob 长度
 * @param pubDer 输出公钥 DER，函数内部分配内存
 * @param pubDerLen 输出公钥长度
 * @return 0 成功，其他失败
 */
int SrHalTeeExportPubKey(const uint8_t *keyBlob, uint32_t keyBlobLen, uint8_t **pubDer, uint32_t *pubDerLen);

/*
 * @brief RSA 签名/验签/加密/解密
 * @param keyBlob 密钥 blob
 * @param keyBlobLen blob 长度
 * @param padding QL_KM_PAD_RSA_*
 * @param digest QL_KM_DIGEST_*
 * @return 0 成功，其他失败
 */
int SrHalTeeRsaSign(const uint8_t *keyBlob, uint32_t keyBlobLen,
                    uint8_t padding, uint8_t digest,
                    const uint8_t *data, uint32_t dataLen,
                    uint8_t *sig, uint32_t *sigLen);

int SrHalTeeRsaVerify(const uint8_t *keyBlob, uint32_t keyBlobLen,
                      uint8_t padding, uint8_t digest,
                      const uint8_t *data, uint32_t dataLen,
                      const uint8_t *sig, uint32_t sigLen);

int SrHalTeeRsaEncrypt(const uint8_t *keyBlob, uint32_t keyBlobLen,
                       uint8_t padding, uint8_t digest,
                       const uint8_t *in, uint32_t inLen,
                       uint8_t *out, uint32_t *outLen);

int SrHalTeeRsaDecrypt(const uint8_t *keyBlob, uint32_t keyBlobLen,
                       uint8_t padding, uint8_t digest,
                       const uint8_t *in, uint32_t inLen,
                       uint8_t *out, uint32_t *outLen);

/*
 * @brief TEE Secure Storage 初始化/去初始化
 */
int SrHalTeeSsInit(void);
void SrHalTeeSsDeinit(void);

/*
 * @brief Secure Storage 读写删
 * @param id 对象 ID（字符串）
 * @return 0 成功，其他失败
 */
int SrHalTeeSsWrite(const char *id, const uint8_t *data, uint32_t len);
int SrHalTeeSsRead(const char *id, uint8_t *data, uint32_t maxLen, uint32_t *outLen);
int SrHalTeeSsDelete(const char *id);

#ifdef __cplusplus
}
#endif

#endif /* _SR_HAL_TEE_H */
