#ifndef __SR_OPENSSL_ENG_H__
#define __SR_OPENSSL_ENG_H__
#include <openssl/engine.h>
#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/ecdh.h>
#include <openssl/crypto.h>
#include <string.h>

#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "openssl/sha.h"
#include "openssl/x509v3.h"
#include "openssl/x509.h"

typedef volatile int CRYPTO_REF_COUNT;

/**
 * @brief 初始化自定义的openssl engine
 *
 */
void ENGINE_Init_sirun();

/**
 * @brief 注销自定义的openssl engine
 *
 */
void ENGINE_DeInit_sirun();

/**
 * @brief 生成csr文件
 *
 * @param version X509 REQ 携带的用户版本信息
 * @param subjectCN 证书所请求的域名 主题通用名称
 * @param subjectOU 组织单位
 * @param subjectO 组织
 * @param subjectL 城市
 * @param subjectST 省
 * @param subjectC 国家
 * @param subhectEMail 邮箱
 * @param rsa 存储rsa公钥的RSA结构指针
 * @param fileName 指定生成的csr文件保存到的位置 绝对路径
 * @return int 结果 1：成功 其他：失败
 */
int GenX509ReqFile(int version, const char *subjectCN, const char *subjectOU, const char *subjectO, const char *subjectL, const char *subjectST, const char *subjectC,
                   const char *subhectEMail, RSA *rsa, const char *fileName);

/**
 * @brief 保存公钥文件，将rsa参数中保存的公钥保存为pem类型的文件
 *
 * @param rsa 存储rsa公钥的RSA结构指针
 * @param fileName 指定生成的pem文件保存到的位置 绝对路径
 * @return int -1 失败 1 成功
 */
int SaveRSAPubKeyFile(RSA *rsa, char *fileName);

/**
 * @brief 读取公钥文件，将pem类型的文件中的公钥读取出来，保存到rsa结构中
 *
 * @param rsa 方向out 用于存储密钥的RSA 结构体指针
 * @param fileName pem文件的绝对路径
 * @return int 结果 -1 失败 1 成功
 */
int LoadRSAPubKey(RSA **rsa, char *fileName);

/**
 * @brief RSA签名
 *
 * @param m 待签名的原始数据指针
 * @param m_length 待签名的原始数据长度
 * @param sigret 签名输出
 * @param siglen 签名长度
 * @param rsa 存储rsa公钥的RSA结构指针
 * @return int 结果 1成功 0失败
 */
int RSA_signature(const unsigned char *m, unsigned int m_length,
                  unsigned char *sigret, unsigned int *siglen, RSA *rsa);

/**
 * @brief RSA验签 验证sigbuf是不是m的签名
 *
 * @param m 待验证的数据
 * @param m_length 待验证数据的长度
 * @param sigbuf 签名
 * @param siglen 签名长度
 * @param rsa 存储rsa公钥的RSA结构指针
 * @return int 结果 1 验签成功 其他 验签失败
 */
int RSA_verifySignature(const unsigned char *m, unsigned int m_length,
                        const unsigned char *sigbuf, unsigned int siglen, RSA *rsa);

/**
 * @brief 打印当前所有注册的engine的信息
 *
 */
void display_engine_list();

/**
 * @brief 出错时打印错误信息
 *
 */
void opensslHandleErrors(void);

#endif
