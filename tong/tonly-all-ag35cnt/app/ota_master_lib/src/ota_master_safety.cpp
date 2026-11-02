#include <iostream>
#include <memory>
#include <algorithm>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/md5.h>
#include "ota_master_safety.h"
#include "ota_master_data.h"
#include "ota_master_log.h"

namespace {
static constexpr int SSL_OK = 1;
using EVP_CIPHER_CTX_ptr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>;
using UNIQUE_UCHAR_ptr = std::unique_ptr<uint8_t[], decltype(&std::free)>;
} // namespace

int AES128ECBEncrypt(const std::string &key, const std::string &input, uint8_t **output)
{
    if (output == nullptr) {
        OTA_LOG_E("Parameter: output is nullptr\n");
        return 0;
    }

    // 检查密钥长度
    if (key.size() != AES_BLOCK_SIZE) {
        OTA_LOG_E("Key must be 16 bytes\n");
        return 0;
    }

    // 使用智能指针自动管理上下文
    EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
    if (!ctx) {
        OTA_LOG_E("Failed to create EVP context\n");
        return 0;
    }

    // 初始化加密操作
    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_128_ecb(), nullptr, reinterpret_cast<const uint8_t *>(key.data()),
                           nullptr) != SSL_OK) {
        OTA_LOG_E("EncryptInit failed\n");
        return 0;
    }

    // 预分配输出缓冲区（使用智能指针自动管理内存）
    const int maxEncryptLen = input.size() + AES_BLOCK_SIZE;
    UNIQUE_UCHAR_ptr encryptBuf(static_cast<uint8_t *>(std::calloc(maxEncryptLen, sizeof(uint8_t))), &std::free);
    if (!encryptBuf) {
        OTA_LOG_E("Memory allocation failed\n");
        return 0;
    }

    int encryptLen = 0, tmpLen = 0;
    // 执行加密操作
    if (EVP_EncryptUpdate(ctx.get(), encryptBuf.get(), &tmpLen, reinterpret_cast<const uint8_t *>(input.data()),
                          input.size()) != SSL_OK) {
        OTA_LOG_E("EncryptUpdate failed\n");
        return 0;
    }
    encryptLen += tmpLen;

    // 处理最后的数据块
    if (EVP_EncryptFinal_ex(ctx.get(), encryptBuf.get() + encryptLen, &tmpLen) != SSL_OK) {
        OTA_LOG_E("EncryptFinal failed\n");
        return 0;
    }
    encryptLen += tmpLen;

    // 转移内存所有权到输出参数
    *output = encryptBuf.release();
    return encryptLen;
}

int AES128ECBDecrypt(const std::string &key, const std::string &input, uint8_t **output)
{
    if (output == nullptr) {
        OTA_LOG_E("Parameter: output is nullptr\n");
        return 0;
    }

    // 检查密钥长度
    if (key.size() != AES_BLOCK_SIZE) {
        OTA_LOG_E("Key must be 16 bytes\n");
        return 0;
    }

    // 使用智能指针自动管理上下文
    EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
    if (!ctx) {
        OTA_LOG_E("Failed to create EVP context\n");
        return 0;
    }

    // 初始化解密操作
    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_128_ecb(), nullptr, reinterpret_cast<const uint8_t *>(key.data()),
                           nullptr) != SSL_OK) {
        OTA_LOG_E("DecryptInit failed\n");
        return 0;
    }

    // 预分配输出缓冲区
    UNIQUE_UCHAR_ptr plainBuf(static_cast<uint8_t *>(std::calloc(input.size(), sizeof(uint8_t))), &std::free);
    if (!plainBuf) {
        OTA_LOG_E("Memory allocation failed\n");
        return 0;
    }

    int plainLen = 0, tmpLen = 0;
    // 执行解密操作
    if (EVP_DecryptUpdate(ctx.get(), plainBuf.get(), &tmpLen, reinterpret_cast<const uint8_t *>(input.c_str()),
                          input.size()) != SSL_OK) {
        OTA_LOG_E("DecryptUpdate failed\n");
        return 0;
    }
    plainLen += tmpLen;

    // 处理最后的数据块
    if (EVP_DecryptFinal_ex(ctx.get(), plainBuf.get() + plainLen, &tmpLen) != SSL_OK) {
        OTA_LOG_E("DecryptFinal failed\n");
        return 0;
    }
    plainLen += tmpLen;

    // 转移内存所有权到输出参数
    *output = plainBuf.release();
    return plainLen;
}

std::string CalculateMD5(const std::string &filePath)
{
    FILE *file = fopen(filePath.c_str(), "rb");
    if (file == nullptr) {
        OTA_LOG_E("Can not open file: %s\n", filePath.c_str());
        return std::string();
    }

    int bytesRead = 0;
    uint8_t data[1024] = {0};
    uint8_t charMd5[MD5_DIGEST_LENGTH] = {0};

#if OPENSSL_VERSION_NUMBER < OPENSSL_3_0_0L
    MD5_CTX mdContext;
    MD5_Init(&mdContext);
    while ((bytesRead = fread(data, 1, 1024, file)) != 0) {
        MD5_Update(&mdContext, data, bytesRead);
    }
    MD5_Final(charMd5, &mdContext);
#else
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx)
        return std::string();

    if (EVP_DigestInit_ex(ctx, EVP_md5(), NULL) == 1) {
        while ((bytesRead = fread(data, 1, 1024, file)) != 0) {
            EVP_DigestUpdate(ctx, data, bytesRead);
        }
        unsigned int len;
        EVP_DigestFinal_ex(ctx, charMd5, &len);
    }
    EVP_MD_CTX_free(ctx);
#endif

    fclose(file);

    char hexMd5[MD5_DIGEST_LENGTH * 2 + 1] = {0};
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        sprintf(&hexMd5[i * 2], "%02x", (unsigned int)charMd5[i]);
    }

    return std::string(hexMd5);
}

int CheckMD5(const std::string &filePath, const std::string &targetMD5)
{
    if (targetMD5.empty()) {
        OTA_LOG_E("Target MD5 is empty\n");
        return OTA_RET_ERR;
    }

    std::string sourceMD5 = CalculateMD5(filePath);
    if (sourceMD5.empty()) {
        OTA_LOG_E("Get source file:%s's MD5 return empty\n", filePath.c_str());
        return OTA_RET_ERR;
    }

    std::string lowerTargetMD5 = targetMD5;
    std::transform(sourceMD5.begin(), sourceMD5.end(), sourceMD5.begin(), ::tolower);
    std::transform(lowerTargetMD5.begin(), lowerTargetMD5.end(), lowerTargetMD5.begin(), ::tolower);
    if (sourceMD5 != lowerTargetMD5) {
        OTA_LOG_E("Source MD5[%s] isn't same with target MD5[%s]\n", sourceMD5.c_str(), lowerTargetMD5.c_str());
        return OTA_RET_ERR;
    }
    OTA_LOG_D("Source MD5[%s] is same with target MD5[%s]\n", sourceMD5.c_str(), lowerTargetMD5.c_str());
    return OTA_RET_OK;
}