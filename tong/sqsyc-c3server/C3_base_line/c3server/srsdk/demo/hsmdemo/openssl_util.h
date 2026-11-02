
#ifndef OPENSSL_UTIL_H
#define OPENSSL_UTIL_H

#include <stdint.h>
#include <openssl/pkcs12.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/rand.h>
#include <glib.h>

typedef enum {
    RSA_PADDING_PKCS1_V15 = 1,  // 最常用：PKCS#1 v1.5
    RSA_PADDING_OAEP_SHA256,    // OAEP+SHA256（Java常用）
    RSA_PADDING_OAEP_SHA1,      // OAEP+SHA1（部分旧系统）
    RSA_PADDING_NO_PADDING      // 无填充（极少用）
} RSA_PADDING_TYPE;


/** 初始化 openssl 库 **/
void OpensslLibraryInit();

/** 释放 openssl 库 **/
void OpensslLibraryRelease();

/**
 * @brief 辅助函数：仅检查X509证书是否过期（只判断notAfter，不判断notBefore）
 * @param x509 已加载的X509证书对象
 * @param is_expired 传出：1=过期（当前时间>notAfter），0=未过期
 * @return 0成功，-1失败
 */
int OpensslCheckCertExpiredOnly(X509* x509, uint8_t* is_expired);

/** 获取TLS协议版本 **/
uint32_t OpensslGetTlsVersion(SSL *ssl);
/** 自定义ASN1_TIME转time_t */
int OpensslAsn1TimeToTime(int64_t* out_time, const ASN1_TIME* asn1_time, char* buf, size_t buf_len);

/** 打包 SHA256 DigestInfo  **/
int OpensslBuildSha256DigestInfo(const uint8_t *digest, uint8_t *digestInfo);

/** 从PEM格式内存缓冲区加载X509证书（必选），可选提取RSA公钥 */
int OpensslLoadX509RsaFromPemBuffer(const char *pemBuffer, int pemBufferLen, X509 **pClientCert, EVP_PKEY **certPubKey);

/** 从X509证书中提取SubjectCN */
int OpensslLoadCrtSubjectCN(X509* x509, char* SubjectCN, int SubjectCNMaxSize);

/** 加载秘钥文件 **/
int OpensslLoadRsaPrivateKey(const char *keyPath, EVP_PKEY** privKey);
/** 加载秘钥pemBuffer  **/
int OpensslLoadRsaPrivateKeyFromBuffer(const char *keyPemBuf, int keyPemSize, EVP_PKEY** privKey);

/**  加载PEM格式的RSA类型X509证书*/
int OpensslLoadRsaCertificate(const char *certPath, X509 **cert);

/** 从承载RSA私钥的EVP_PKEY导出公钥DER到指定缓冲区 **/
int OpensslExportRsaPubKeyDerFromPrivKey(EVP_PKEY* privkey, uint8_t* pubkeyDer, int* pubkeyDerBufferSize);

/** 使用 RSA 私钥（EVP_PKEY）进行 SHA256withRSA 签名 */
int OpensslRsaSignWithSHA256(EVP_PKEY* pkiKey, const uint8_t* data, int dataLen, uint8_t* signature, uint16_t* signatureLen);

/** 生成 RSA 秘钥 **/
int OpensslGenerateRsaPrivateKey(EVP_PKEY **pkey, int bits);
/**
 * @brief 生成 CSR
 * @return 0: 成功，-1: 参数错误，-2: 构建DN失败，-3: 创建CSR失败，-4: 签名失败
 */
int OpensslGenerateCsr(X509_REQ** csr, EVP_PKEY* pkey,
    const char* country,  const char* state, const char* city, const char* org, const char* unit,
    const char* vin, const char* tboxSn);

/**  检查证书加载状态、与私钥匹配性、是否过期 */
int OpensslCheckCrt(EVP_PKEY* pkiKey, const char* crtPath, uint8_t* isMatch, uint8_t* isExpired);

/** 私钥解密 */
int OpensslPrivateKeyDecryptEx(EVP_PKEY* pkiKey, RSA_PADDING_TYPE padding_type,
                               uint8_t* in, uint16_t inLen, uint8_t* outPlain, uint16_t* outPlainSize);

/** AES-ECB-PKCS5Padding 加密 */
int OpensslAesEcbPkcs5Encrypt(uint8_t* in, uint16_t inLen, uint8_t* aesKey, uint16_t keyLen, uint8_t* out, uint16_t* outLen);

/** AES-ECB-PKCS5Padding 解密 */
int OpensslAesEcbPkcs5Decrypt(uint8_t* in, uint16_t inLen, uint8_t* aesKey, uint16_t keyLen, uint8_t* out, uint16_t* outLen);


/** 从文件路径校验PEM证书和私钥是否匹配、仅判断证书是否过期（notAfter） */
int OpensslRsaCheckKeyAndCrtFile(const char* crtPemPath, const char* keyPemPath, const char* subjectCN);

/** 从内存缓冲区校验PEM证书和私钥是否匹配、仅判断证书是否过期（notAfter） */
int OpensslRsaCheckKeyAndCrtBuffer(const char* crtPemBuf, int crtPemSize, const char* keyPemBuf, int keyPemSize, const char* subjectCN);

/** 从X509证书SAN扩展提取域名/IP  */
int OpensslX509ExtractHostAndIPFromSAN(X509 *serverCrt, GList **dns_list, GList **ip_list);

/**  使用公钥进行 RSA 加密 */
int OpensslPublicKeyEncrypt(const char *pubKeyPath, RSA_PADDING_TYPE padding_type, const uint8_t *in, int inLen, uint8_t *out, int *outLen);

/** 使用 RSA 公钥进行 SHA256withRSA 验签 */
int OpensslRsaVerifyWithSHA256(EVP_PKEY* pubKey, const uint8_t *data, int dataLen, const uint8_t *signature, uint16_t signatureLen);

/** 从 PEM 文件加载 RSA 公钥为 EVP_PKEY */
int OpensslLoadRsaPublicKey(const char *pubKeyPath, EVP_PKEY **pubKey);

/** 保存 RSA 私钥到 PEM 文件 **/
int OpensslSaveRsaPrivateKey(EVP_PKEY *privKey, const char *filePath);

/** 保存 RSA 公钥到 PEM 文件 **/
int OpensslSaveRsaPublicKey(EVP_PKEY *privKey, const char *filePath);

/** 从私钥导出公钥 **/
int OpensslExtractPublicKey(EVP_PKEY *privKey, EVP_PKEY **pubKey);

/** 从 RSA 公钥中导出 X.509 SubjectPublicKeyInfo DER
 *  @param outBuf: 输出缓冲区
 *  @param outLen: 输入: 最大缓冲区大小; 输出: 实际打包长度
 *  @return 0 成功, -1 失败
 */
int OpensslExtracRsaPubKey(EVP_PKEY *pkey, uint8_t *outBuf, uint32_t *outLen);

/** 从 RSA 私钥中导出 PKCS#1 RSAPrivateKey DER
 *  @param outBuf: 输出缓冲区
 *  @param outLen: 输入: 最大缓冲区大小; 输出: 实际打包长度
 *  @return 0 成功, -1 失败
 */
int OpensslExtracRsaPriKey(EVP_PKEY *pkey, uint8_t *outBuf, uint32_t *outLen);

/** 从 RSA 私钥中导出完整密钥对 PKCS#8 PrivateKeyInfo DER
 *  @param outBuf: 输出缓冲区
 *  @param outLen: 输入: 最大缓冲区大小; 输出: 实际打包长度
 *  @return 0 成功, -1 失败
 */
int OpensslExtracRsaKeyPair(EVP_PKEY *pkey, uint8_t *outBuf, uint32_t *outLen);

/** 从 PEM 格式私钥缓冲区中导出 PKCS#8 PrivateKeyInfo DER
 *  @param keyPemBuf: PEM 私钥缓冲区
 *  @param keyPemSize: PEM 缓冲区长度
 *  @param outBuf: 输出缓冲区
 *  @param outLen: 输入: 最大缓冲区大小; 输出: 实际打包长度
 *  @return 0 成功, -1 失败
 */
int OpensslExtracRsaKeyPairFromBuffer(const char *keyPemBuf, int keyPemSize, uint8_t *outBuf, uint32_t *outLen);

int OpensslRsaVerify(EVP_PKEY *pkey, const uint8_t *plain, uint32_t plainLen,const uint8_t *signBuf, uint32_t signBufLen);
int OpensslRsaSign(EVP_PKEY *pkey, const uint8_t *plain, uint32_t plainLen, uint8_t *signBuf, uint32_t *signBufLen);
int OpensslRsaDecrypt(EVP_PKEY *pkey, int padding, const uint8_t *ciphertext, int ciphertextLen, uint8_t *plaintext, int *plaintextLen);
int OpensslRsaEncrypt(EVP_PKEY *pkey, int padding, const uint8_t *plaintext, int plaintextLen, uint8_t *ciphertext, int *ciphertextLen);

int OpensslGetRandom(uint8_t* buf, int len);

int OpensslX509NameToString(X509_NAME *name, char *buf, size_t buf_size);

#endif
