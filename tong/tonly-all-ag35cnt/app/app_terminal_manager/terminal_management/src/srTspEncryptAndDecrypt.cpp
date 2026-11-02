/*********************************************************
 * Copyright © 2025 SiRun (WuXi) . All rights reserved.
 *
 * Description: 终端管理平台数据加密/解密实现
 *              内部按 encrypt_type 分发到 DES-CBC / AES-128-CBC / SM4-CBC
 *********************************************************/
#include "srTspEncryptAndDecrypt.h"
#include "sr_log.h"

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/des.h>

#include <cstdlib>
#include <cstring>
#include <memory>

#define DES_BLOCK_SIZE 8

/* SM4-CBC 通过 sm4 软实现 */
extern "C"
{
#include "sm4.h"
}

/* ========================= 会话状态（文件级静态量） ========================= */

static int32_t g_encryptType = 0; /* DataEncryption: 0=NON, 1=DES, 2=AES, 3=SM4 */
static uint8_t g_iv[16] = {0};    /* CBC 初始向量 */
static uint8_t g_key[16] = {0};   /* 对称密钥 */
static bool g_sessionSet = false;

/* ========================= 公开 API ========================= */

void EncryptionSetSession(int32_t encryptType,
                          const uint8_t iv[16],
                          const uint8_t key[16])
{
    if (!iv || !key)
    {
        LOG_Error("[%s] iv or key is NULL", __func__);
        return;
    }

    g_encryptType = encryptType;
    (void)memcpy(g_iv, iv, sizeof(g_iv));
    (void)memcpy(g_key, key, sizeof(g_key));
    g_sessionSet = true;

    LOG_Info("[%s] type=%d", __func__, (int)encryptType);
    LOG_DebugHex(g_iv, sizeof(g_iv), "[EncryptionSetSession] iv");
    LOG_DebugHex(g_key, sizeof(g_key), "[EncryptionSetSession] key");
}

int32_t EncryptionGetType(void)
{
    return g_encryptType;
}

int EncryptionIsActive(void)
{
    return (g_encryptType != 0 && g_sessionSet) ? 1 : 0;
}

/* ========================= 内部辅助 ========================= */

namespace
{

    using EVP_CIPHER_CTX_ptr =
        std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>;
    using UNIQUE_UCHAR_ptr =
        std::unique_ptr<uint8_t[], decltype(&std::free)>;

    constexpr int SSL_OK = 1;

    // DES、AES、SM4 等分组密码的 CBC 模式 要求被加密的数据长度必须是“块大小”的整数倍（DES 为 8 字节，AES/SM4 为 16 字节）。
    // 但实际明文长度往往不是整倍数，如果不进行补齐，加密算法无法直接处理。

    /**
     * @brief 对 input 施加 PKCS#7 填充，写入固定长度 blockSize 倍数的 output
     */
    int Pkcs7Pad(const uint8_t *input, size_t inputLen,
                 uint8_t *output, size_t blockSize)
    {
        if (!input || !output || blockSize == 0U)
            return 0;

        // 1. 计算需要填充的字节数
        size_t padLen = blockSize - (inputLen % blockSize);
        // 2. 填充后的总长度
        size_t total = inputLen + padLen;
        // 3. 先把原始明文拷贝到输出缓冲区
        memcpy(output, input, inputLen);
        // 4. 在明文后面填充 padLen 个字节，每个字节的值都是 padLen (填充的字节数)
        memset(output + inputLen, (int)padLen, padLen);

        return (int)total;
    }

    /**
     * @brief 去除 PKCS#7 填充，返回实际明文长度；失败返回 0
     */
    int Pkcs7Unpad(const uint8_t *data, size_t dataLen)
    {
        if (!data || dataLen == 0U || dataLen > 4096U)
            return 0;

        // 1. 读取最后一个字节，其值为填充长度
        uint8_t padLen = data[dataLen - 1U];
        if (padLen == 0U || padLen > 16U || padLen > dataLen)
            return 0;
        // 2. 校验最后 padLen 个字节是否全部等于 padLen
        for (uint8_t i = 1U; i <= padLen; i++)
        {
            if (data[dataLen - i] != padLen)
                return 0;
        }
        // 3. 返回去除填充后的实际明文长度
        return (int)(dataLen - padLen);
    }

    /* ========================= DES-CBC ========================= */

    int DesCbcEncrypt(const uint8_t *key, const uint8_t *iv,
                      const uint8_t *plaintext, size_t plainLen,
                      uint8_t **output)
    {
        EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
        if (!ctx)
        {
            LOG_Error("DES-CBC: EVP_CIPHER_CTX_new failed");
            return 0;
        }

        /* DES 密钥 8 字节，IV 8 字节 */
        if (EVP_EncryptInit_ex(ctx.get(), EVP_des_cbc(), nullptr,
                               key, iv) != SSL_OK)
        {
            LOG_Error("DES-CBC: EncryptInit failed");
            return 0;
        }

        const int maxOut = (int)plainLen + DES_BLOCK_SIZE;
        UNIQUE_UCHAR_ptr encBuf(
            static_cast<uint8_t *>(std::calloc(maxOut, sizeof(uint8_t))),
            &std::free);
        if (!encBuf)
        {
            LOG_Error("DES-CBC: calloc failed");
            return 0;
        }

        int outLen = 0, tmpLen = 0;
        if (EVP_EncryptUpdate(ctx.get(), encBuf.get(), &tmpLen,
                              plaintext, (int)plainLen) != SSL_OK)
        {
            LOG_Error("DES-CBC: EncryptUpdate failed");
            return 0;
        }
        outLen += tmpLen;

        if (EVP_EncryptFinal_ex(ctx.get(), encBuf.get() + outLen, &tmpLen) != SSL_OK)
        {
            LOG_Error("DES-CBC: EncryptFinal failed");
            return 0;
        }
        outLen += tmpLen;

        *output = encBuf.release();
        return outLen;
    }

    int DesCbcDecrypt(const uint8_t *key, const uint8_t *iv,
                      const uint8_t *ciphertext, size_t cipherLen,
                      uint8_t **output)
    {
        EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
        if (!ctx)
        {
            LOG_Error("DES-CBC: EVP_CIPHER_CTX_new failed");
            return 0;
        }

        if (EVP_DecryptInit_ex(ctx.get(), EVP_des_cbc(), nullptr,
                               key, iv) != SSL_OK)
        {
            LOG_Error("DES-CBC: DecryptInit failed");
            return 0;
        }

        UNIQUE_UCHAR_ptr plainBuf(
            static_cast<uint8_t *>(std::calloc(cipherLen, sizeof(uint8_t))),
            &std::free);
        if (!plainBuf)
        {
            LOG_Error("DES-CBC: calloc failed");
            return 0;
        }

        int outLen = 0, tmpLen = 0;
        if (EVP_DecryptUpdate(ctx.get(), plainBuf.get(), &tmpLen,
                              ciphertext, (int)cipherLen) != SSL_OK)
        {
            LOG_Error("DES-CBC: DecryptUpdate failed");
            return 0;
        }
        outLen += tmpLen;

        if (EVP_DecryptFinal_ex(ctx.get(), plainBuf.get() + outLen, &tmpLen) != SSL_OK)
        {
            LOG_Error("DES-CBC: DecryptFinal failed (padding or key error)");
            return 0;
        }
        outLen += tmpLen;

        uint8_t *trimmed = static_cast<uint8_t *>(std::malloc(outLen));
        if (!trimmed)
            return 0;
        (void)memcpy(trimmed, plainBuf.get(), outLen);
        *output = trimmed;
        return outLen;
    }

    /* ========================= AES-128-CBC ========================= */

    int Aes128CbcEncrypt(const uint8_t *key, const uint8_t *iv,
                         const uint8_t *plaintext, size_t plainLen,
                         uint8_t **output)
    {
        EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
        if (!ctx)
        {
            LOG_Error("AES-CBC: EVP_CIPHER_CTX_new failed");
            return 0;
        }

        if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_128_cbc(), nullptr,
                               key, iv) != SSL_OK)
        {
            LOG_Error("AES-CBC: EncryptInit failed");
            return 0;
        }

        /* PKCS#7 填充后密文最长 = 明文 + 16 */
        const int maxOut = (int)plainLen + AES_BLOCK_SIZE;
        UNIQUE_UCHAR_ptr encBuf(
            static_cast<uint8_t *>(std::calloc(maxOut, sizeof(uint8_t))),
            &std::free);
        if (!encBuf)
        {
            LOG_Error("AES-CBC: calloc failed");
            return 0;
        }

        int outLen = 0, tmpLen = 0;
        if (EVP_EncryptUpdate(ctx.get(), encBuf.get(), &tmpLen,
                              plaintext, (int)plainLen) != SSL_OK)
        {
            LOG_Error("AES-CBC: EncryptUpdate failed");
            return 0;
        }
        outLen += tmpLen;

        if (EVP_EncryptFinal_ex(ctx.get(), encBuf.get() + outLen, &tmpLen) != SSL_OK)
        {
            LOG_Error("AES-CBC: EncryptFinal failed");
            return 0;
        }
        outLen += tmpLen;

        *output = encBuf.release();
        return outLen;
    }

    int Aes128CbcDecrypt(const uint8_t *key, const uint8_t *iv,
                         const uint8_t *ciphertext, size_t cipherLen,
                         uint8_t **output)
    {
        EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
        if (!ctx)
        {
            LOG_Error("AES-CBC: EVP_CIPHER_CTX_new failed");
            return 0;
        }

        if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_128_cbc(), nullptr,
                               key, iv) != SSL_OK)
        {
            LOG_Error("AES-CBC: DecryptInit failed");
            return 0;
        }

        UNIQUE_UCHAR_ptr plainBuf(
            static_cast<uint8_t *>(std::calloc(cipherLen, sizeof(uint8_t))),
            &std::free);
        if (!plainBuf)
        {
            LOG_Error("AES-CBC: calloc failed");
            return 0;
        }

        int outLen = 0, tmpLen = 0;
        if (EVP_DecryptUpdate(ctx.get(), plainBuf.get(), &tmpLen,
                              ciphertext, (int)cipherLen) != SSL_OK)
        {
            LOG_Error("AES-CBC: DecryptUpdate failed");
            return 0;
        }
        outLen += tmpLen;

        if (EVP_DecryptFinal_ex(ctx.get(), plainBuf.get() + outLen, &tmpLen) != SSL_OK)
        {
            LOG_Error("AES-CBC: DecryptFinal failed (padding or key error)");
            return 0;
        }
        outLen += tmpLen;

        /* EVP 已去除 PKCS#7 填充，直接返回 */
        uint8_t *trimmed = static_cast<uint8_t *>(std::malloc(outLen));
        if (!trimmed)
            return 0;
        (void)memcpy(trimmed, plainBuf.get(), outLen);
        *output = trimmed;
        return outLen;
    }

    /* ========================= SM4-CBC ========================= */

    int Sm4CbcEncrypt(const uint8_t *key, const uint8_t *iv,
                      const uint8_t *plaintext, size_t plainLen,
                      uint8_t **output)
    {
        /* SM4 CBC 需要 16 字节对齐输入 → 手动 PKCS#7 填充 */
        size_t blockSize = 16U;
        size_t paddedLen = ((plainLen / blockSize) + 1U) * blockSize;

        UNIQUE_UCHAR_ptr padded(
            static_cast<uint8_t *>(std::calloc(paddedLen, sizeof(uint8_t))),
            &std::free);
        if (!padded)
        {
            LOG_Error("SM4-CBC: calloc failed");
            return 0;
        }

        int total = Pkcs7Pad(plaintext, plainLen, padded.get(), blockSize);
        if (total <= 0 || (size_t)total != paddedLen)
        {
            LOG_Error("SM4-CBC: PKCS#7 pad failed");
            return 0;
        }

        /* sm4_crypt_cbc 原地加解密 (input == output) */
        uint8_t ivCopy[16];
        (void)memcpy(ivCopy, iv, 16);

        int rc = sm4_crypt_cbc(
            const_cast<uint8_t *>(key), /* key (非 const API) */
            SM4_ENCRYPT,
            total,
            ivCopy,
            padded.get(),
            padded.get());

        if (rc != 0)
        {
            LOG_Error("SM4-CBC: encrypt failed rc=%d", rc);
            return 0;
        }

        *output = padded.release();
        return total;
    }

    int Sm4CbcDecrypt(const uint8_t *key, const uint8_t *iv,
                      const uint8_t *ciphertext, size_t cipherLen,
                      uint8_t **output)
    {
        if (cipherLen % 16U != 0U)
        {
            LOG_Error("SM4-CBC: cipherLen %zu not block-aligned", cipherLen);
            return 0;
        }

        UNIQUE_UCHAR_ptr buf(
            static_cast<uint8_t *>(std::calloc(cipherLen, sizeof(uint8_t))),
            &std::free);
        if (!buf)
        {
            LOG_Error("SM4-CBC: calloc failed");
            return 0;
        }
        (void)memcpy(buf.get(), ciphertext, cipherLen);

        uint8_t ivCopy[16];
        (void)memcpy(ivCopy, iv, 16);

        int rc = sm4_crypt_cbc(
            const_cast<uint8_t *>(key),
            SM4_DECRYPT,
            (int)cipherLen,
            ivCopy,
            buf.get(),
            buf.get());

        if (rc != 0)
        {
            LOG_Error("SM4-CBC: decrypt failed rc=%d", rc);
            return 0;
        }

        int plainLen = Pkcs7Unpad(buf.get(), cipherLen);
        if (plainLen <= 0)
        {
            LOG_Error("SM4-CBC: PKCS#7 unpad failed");
            return 0;
        }

        uint8_t *plain = static_cast<uint8_t *>(std::malloc(plainLen));
        if (!plain)
            return 0;
        (void)memcpy(plain, buf.get(), plainLen);
        *output = plain;
        return plainLen;
    }

} // anonymous namespace

/* ========================= 公开加解密调度 ========================= */

int EncryptPayload(const uint8_t *plaintext, size_t plainLen,
                   uint8_t **output)
{
    if (!output || !plaintext || plainLen == 0U)
    {
        LOG_Error("EncryptPayload: invalid args");
        return 0;
    }
    if (!g_sessionSet || g_encryptType == 0)
    {
        LOG_Error("EncryptPayload: session not active");
        return 0;
    }

    switch (g_encryptType)
    {
    case 1: /* DES-CBC */
        return DesCbcEncrypt(g_key, g_iv, plaintext, plainLen, output);
    case 2: /* AES-128-CBC */
        return Aes128CbcEncrypt(g_key, g_iv, plaintext, plainLen, output);
    case 3: /* SM4-CBC */
        return Sm4CbcEncrypt(g_key, g_iv, plaintext, plainLen, output);
    default:
        LOG_Error("EncryptPayload: unsupported type %d", (int)g_encryptType);
        return 0;
    }
}

int DecryptPayload(const uint8_t *ciphertext, size_t cipherLen,
                   uint8_t **output)
{
    if (!output || !ciphertext || cipherLen == 0U)
    {
        LOG_Error("DecryptPayload: invalid args");
        return 0;
    }
    if (!g_sessionSet || g_encryptType == 0)
    {
        LOG_Debug("DecryptPayload: session not active, pass-through");
        return 0;
    }

    /* 最小密文长度 >= 该算法的一个块 */
    {
        size_t minLen = (g_encryptType == 1) ? 8U : 16U;
        if (cipherLen < minLen)
        {
            LOG_Error("DecryptPayload: cipherLen %zu too short (min %zu)", cipherLen, minLen);
            return 0;
        }
    }

    switch (g_encryptType)
    {
    case 1: /* DES-CBC */
        return DesCbcDecrypt(g_key, g_iv, ciphertext, cipherLen, output);
    case 2: /* AES-128-CBC */
        return Aes128CbcDecrypt(g_key, g_iv, ciphertext, cipherLen, output);
    case 3: /* SM4-CBC */
        return Sm4CbcDecrypt(g_key, g_iv, ciphertext, cipherLen, output);
    default:
        LOG_Error("DecryptPayload: unsupported type %d", (int)g_encryptType);
        return 0;
    }
}
