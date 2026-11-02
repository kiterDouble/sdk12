#ifndef _PKI_SDK_H_
#define _PKI_SDK_H_

#include <stdio.h>
#include <openssl/ssl.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief   证书检查函数
 * @return 成功返回0/失败返回-1
 */
int CertificateInspection();
/**
 * @brief 引擎初始化函数
 * @return void
 */
void SiRunPKIEngineInit();
/**
 * @brief 获取引擎的EVP_PKEY结构(使用结束记得EVP_PKEY_free(s_pubKey))
 * @return 成功返回0/失败返回-1
 */
EVP_PKEY *SiRunPKIEvpPkeyStruct();
/**
 * @brief  该函数等同于SSL_CTX_use_PrivateKey_file这个函数
 * @return 成功返回0/失败返回-1
 */
int SiRunPKISSLCTXUsePrivateKey(SSL_CTX *ctx);

/**
 * @brief 注销引擎函数
 */
void SiRunPKIEngineDeInit();


/**
 * @brief RSA公钥加密
 * @param plaintext 明文数据
 * @param pt_len 明文长度
 * @param ciphertext 密文输出缓冲区
 * @param pt_len 密文长度
 * @param padding 填充模式  暂时仅支持RSA_PKCS1_PADDING
 * @return 成功返回0/失败返回-1
 */
int SiRunPKIRSAPublicEnc(const unsigned char *plaintext, int pt_len,unsigned char *ciphertext,int *ct_len,int padding);


/**
 * @brief RSA私钥解密
 * @param ciphertext 密文数据
 * @param pt_len 明文长度
 * @param plaintext 明文数据
 * @param pt_len 明文长度
 * @param padding 填充模式 暂时仅支持RSA_PKCS1_PADDING
 * @return 成功返回0/失败返回-1
 */

int SiRunPKIRSAPrivateDec(const unsigned char *ciphertext,int ct_len, unsigned char *plaintext,int *pt_len,int padding);


#ifdef __cplusplus
}
#endif

#endif