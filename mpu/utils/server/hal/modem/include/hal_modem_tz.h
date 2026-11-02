/*
 * hal_modem_tz.h
 *
 *  Created on: 2026-2-3
 *  TrustZone related header for modem HAL
 */

#ifndef __HAL_MODEM_TZ_H__
#define __HAL_MODEM_TZ_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include "openssl/rsa.h"
#include "sr_log.h"

// NandFlash分区名称
#define SS_PARTITION "secdata"

// 用于存储此数据的进程信息
#define SS_PROCESS   "tzload"
#define SS_RSA_PUBLIC_KEY_FILE "modem_rsa_pubkey.pem"

// 文件名称定义（用于在TrustZone中标记公钥）
#define RSA_KEY_PAIR_1 "modem_rsa_keypair_1.pem"
#define RSA_KEY_PAIR_2 "modem_rsa_keypair_2.pem"




#define SRTEE_COND_CHK(cond,err,loc) \
do {\
	if ( cond ) {\
		LOG_Error("[TZ]func:%s [%s] ret = %d\n",__FUNCTION__,#cond,err);\
		ret = err;\
		goto loc; \
	}\
}while(0)

// 修复的对齐宏（安全版本）
#define __4KB__             4096

// 基本对齐宏（注意括号）
#define ALIGN_BASIC(x, a)   (((x) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_4K_BASIC(x)   ALIGN_BASIC(x, __4KB__)

// 安全对齐宏（带类型转换）
#define ALIGN(x, a)         (((uintptr_t)(x) + ((uintptr_t)(a) - 1)) & \
                            ~((uintptr_t)((a) - 1)))
#define ALIGN_4K(x)         ALIGN(x, __4KB__)

/**
* @name HalModemTzWrite
* @brief 将数据写入TrustZone安全存储区
* @param[in] data 要写入的数据指针
* @param[in] dataLen 要写入的数据长度
* @param[in] fileName 存储数据的文件名
* @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
*/
int32_t HalModemTzWrite(const uint8_t *data, uint32_t dataLen,const char *fileName);

/**
* @name HalModemTzRead
* @brief 从TrustZone安全存储区读取数据
* @param[in] filename 要读取的文件名
* @param[in] outbuf 用于存储读取数据的缓冲区指针
* @param[in out] outlen 输入为缓冲区大小，输出为实际读取数据长度
* @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
*/
int32_t HalModemTzRead(const char *filename, uint8_t *outbuf, uint32_t *outlen);

/**
* @name HalModemTzDelete
* @brief 删除TrustZone安全存储区的数据
* @param[in] filename 要删除的文件名
* @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
*/
int32_t HalModemTzDelete(const char *filename);

/**
* @name HalModemTzQuery
* @brief 查询TrustZone安全存储区的数据状态
* @param[in] filename 要查询的文件名
* @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
*/
int32_t HalModemTzQuery(const char *filename);


/**
 * @brief 导入RSA密钥对到TrustZone
 * @param[in] rsa RSA结构体指针
 * @param[in] keypair_filename 密钥对文件名
 * @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
 */
int32_t HalModemTzImportKey(RSA *rsa, const char *keypair_filename);

/**
 * @brief 删除TrustZone中的RSA密钥对
 * @param[in] keypair_filename 密钥对文件名
 * @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
 */
int32_t HalModemTzRSADelKey(const char *keypair_filename);

/**
 * @brief 向Tee申请生成RSA密钥对，密钥对存储在Tee中（privateKey），并提取出公钥信息进行返回，同时将公钥保存在Tee（publicKey）。
 * 
 * @param bits RSA 密钥的​​模数长度（1024、2048、3072、4096）
 * @param e RSA 的​​公钥指数（public exponent）​​，通常是一个​​小奇数​​
 * @param keypair_filename 密钥对文件名
 * @return RSA 公钥信息
 */
RSA *HalModemTzRSAKeygen(int32_t bits, BIGNUM *e, const char *keypair_filename);


/**
 * @brief 使用Tee中存储的私钥进行签名。
 * 
 * @param rsabits RSA 密钥的​​模数长度（1024、2048、3072、4096）
 * @param der 待签名​​数据
 * @param derlen 待签名​​数据长度
 * @param sigret 签名后数据
 * @param siglen in 签名前缓冲区长度，out 签名后数据实际长度​​
 * @param keypair_filename 密钥对文件名
 * @return 签名后数据长度
 */
int32_t HalModemTzRSASign(int32_t rsabits, unsigned char *der, unsigned int derlen, unsigned char *sigret, unsigned int *siglen, const char *keypair_filename);

/**
 * @brief 获取Tee中的RSA公钥信息。
 * 
 * @return RSA公钥
 */
RSA *HalModemTzRSAGetPubKey(const char *keypair_filename);


/**
 * @brief 使用Tee中存储的私钥进行加密。实际为无填充签名
 * 
 * @param rsabits RSA 加密的​​模数长度（1024、2048、3072、4096）
 * @param der 待加密​​数据
 * @param derlen 待加密​​数据长度
 * @param sigret 加密后数据
 * @param padding in padding
 * @param keypair_filename 密钥对文件名
 * @return 加密后数据长度
 */
int32_t HalModemTzRSAPrivEnc(int32_t rsabits, unsigned char *der, unsigned int derlen, unsigned char *sigret, unsigned int padding, const char *keypair_filename);


/**
 * @brief 使用Tee中存储的私钥进行解密。
 * 
 * @param flen 加密数据长度 
 * @param rsabits 密钥的​​模数长度（1024、2048、3072、4096）
 * @param from 待解密​​数据
 * @param to 解密后数据
 * @param keypair_filename 密钥对文件名
 * @return 解密数据长度
 */
int32_t  HalModemTzRSAPrivDec(int32_t flen, int32_t rsabits, const unsigned char *from, unsigned char *to, const char *keypair_filename);


/**
 * @brief 获取Tee中的RSA公钥信息。
 * @param keypair_filename 密钥对文件名
 * @return 是否成功打开文件,0成功，其他：通用错误
 */
int32_t HalModemTzGetPrivateKeyState(const char *keypair_filename);

#ifdef __cplusplus
}
#endif
#endif