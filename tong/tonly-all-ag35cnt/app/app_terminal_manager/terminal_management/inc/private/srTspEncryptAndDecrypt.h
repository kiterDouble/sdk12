/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Description: 终端管理平台数据加密/解密接口
 *              支持 DES-CBC / AES-128-CBC / SM4-CBC
 *              由 ConnectAck 协商后激活
 *********************************************************/
#ifndef SRTSPENCRYPTANDDECRYPT_H
#define SRTSPENCRYPTANDDECRYPT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 保存加密配置
     *
     * @param encryptType 加密类型：
     *                      0 = NON (不加密)
     *                      1 = DES-CBC
     *                      2 = AES-128-CBC
     *                      3 = SM4-CBC
     * @param iv          IV 字符串（hex 编码或 raw，调用方负责转为 16 字节）
     * @param key         16 字节对称密钥
     *
     * @note 本函数通常只在登录阶段调用一次，后续只读，无需加锁
     */
    void EncryptionSetSession(int32_t encryptType, const uint8_t iv[16],
                              const uint8_t key[16]);

    /**
     * @brief 获取当前协商的加密类型
     * @return 0=NON, 1=DES, 2=AES, 3=SM4
     */
    int32_t EncryptionGetType(void);

    /**
     * @brief 当前会话是否启用加密
     * @return 0 不加密, 非0 已启用加密
     */
    int EncryptionIsActive(void);

    /**
     * @brief 使用会话参数加密明文
     *
     *        内部自动 PKCS#7 填充。
     *        encrypt_type = DES  → DES-CBC
     *        encrypt_type = AES  → AES-128-CBC
     *        encrypt_type = SM4  → SM4-CBC
     *
     * @param plaintext  明文字节
     * @param plainLen   明文长度（≥1 字节）
     * @param output     输出密文（由内部 calloc，调用方负责 free）
     *
     * @return 密文长度（字节），失败返回 0
     */
    int EncryptPayload(const uint8_t *plaintext, size_t plainLen,
                       uint8_t **output);

    /**
     * @brief 使用会话参数解密密文
     *
     *        内部自动去除 PKCS#7 填充。
     *
     * @param ciphertext 密文字节
     * @param cipherLen  密文长度（≥8 字节）
     * @param output     输出明文（由内部 calloc，调用方负责 free）
     *
     * @return 明文长度（字节），失败返回 0
     */
    int DecryptPayload(const uint8_t *ciphertext, size_t cipherLen,
                       uint8_t **output);

#ifdef __cplusplus
}
#endif

#endif // SRTSPENCRYPTANDDECRYPT_H
