#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <openssl/des.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/hmac.h>

#include "log_pack_compress_and_crypto.h"
#include "sr_log.h"

#define ARCHIVE_PAGE_SIZE (4*1024)
#define CRYPTO_BUFFER_SIZE 1024

typedef enum{
    E_COMPRESS_STAGE_START = 0x0,
    E_COMPRESS_STAGE_COMPRESSING,
    E_COMPRESS_STAGE_END,
    E_COMPRESS_STAGE_FAIL,
}CompressFileState_e;

typedef struct{
    uint8_t compressStage;
    const char* compressPassword;
    const char* outputFilePath;
    FILE* outPutFile;
    Crypto_interface_t* cryptoInterface; /* NULL 表示仅压缩不加密 */
    Crypto_Dev_t* cryptoDev;
    uint8_t cryptoOutBuf[CRYPTO_BUFFER_SIZE + 256 /*EVP_CIPHER_CTX_block_size(ctx) : 8*/];
    int32_t cryptoOutLen;
}CompressFileCallBackArg_t;

/**
 * 统一释放 archive、entry 和输入文件资源。
 * 不判断 archive_write_close 返回值，仅做资源释放。
 */
static void CompressFileCleanup(struct archive *archive_writer, struct archive_entry *archive_entry, FILE* inPutFile){
    if (archive_writer) {
        archive_write_close(archive_writer);
        archive_write_free(archive_writer);
    }

    if (archive_entry) {
        archive_entry_free(archive_entry);
    }

    if (inPutFile != NULL) {
        fclose(inPutFile);
    }
}

static ssize_t CompressDataCallback(struct archive *a, void *client_data, const void *buff, size_t length) {
    CompressFileCallBackArg_t *arg = (CompressFileCallBackArg_t *)client_data;
    int cryptoPageNum = 0;
    int cryptoPageDataLen = 0;
    int cryptoPageDataPos = 0;
    int index = 0;
    int ret = 0;
    size_t written = 0;

    (void)a;

    if (arg == NULL) {
        return -1;
    }

    switch(arg->compressStage){
        case E_COMPRESS_STAGE_START:
            {
                LOG_Warn("[%s] E_COMPRESS_STAGE_START", __func__);

                arg->outPutFile = fopen(arg->outputFilePath, "wb");
                if (arg->outPutFile == NULL) {
                    LOG_Error("fopen %s failed", arg->outputFilePath);
                    return -1;
                }

                /* cryptoInterface 为 NULL 时只压缩不加密，跳过加密头写入 */
                if (arg->cryptoInterface == NULL) {
                    break;
                }

                arg->cryptoOutLen = 0;
                ret = arg->cryptoInterface->crypto_start(arg->cryptoDev, arg->compressPassword, arg->cryptoOutBuf, &arg->cryptoOutLen);
                if (ret != 0) {
                    LOG_Error("crypto_start failed: %d", ret);
                    fclose(arg->outPutFile);
                    arg->outPutFile = NULL;
                    return -1;
                }

                written = fwrite(arg->cryptoOutBuf, 1, arg->cryptoOutLen, arg->outPutFile);
                if (written != (size_t)arg->cryptoOutLen) {
                    LOG_Error("fwrite crypto prefix failed");
                    fclose(arg->outPutFile);
                    arg->outPutFile = NULL;
                    return -1;
                }
            }
            break;

        case E_COMPRESS_STAGE_COMPRESSING:
            {
                if (arg->outPutFile == NULL) {
                    return -1;
                }

                if (arg->cryptoInterface == NULL) {
                    /* 不加密时直接写入原始压缩数据 */
                    written = fwrite(buff, 1, length, arg->outPutFile);
                    if (written != length) {
                        LOG_Error("fwrite compressed data failed");
                        return -1;
                    }
                } else {
                    /* 加密时按页加密后再写入 */
                    cryptoPageNum = (length / CRYPTO_BUFFER_SIZE) + ((length % CRYPTO_BUFFER_SIZE) > 0 ? 1 : 0);
                    for (index = 0; index < cryptoPageNum; index++) {
                        cryptoPageDataPos = index * CRYPTO_BUFFER_SIZE;
                        cryptoPageDataLen = (int)length - cryptoPageDataPos;
                        if (cryptoPageDataLen > CRYPTO_BUFFER_SIZE) {
                            cryptoPageDataLen = CRYPTO_BUFFER_SIZE;
                        }

                        arg->cryptoOutLen = 0;
                        ret = arg->cryptoInterface->crypto_encrypt(arg->cryptoDev, (uint8_t*)buff + cryptoPageDataPos, cryptoPageDataLen, arg->cryptoOutBuf, &arg->cryptoOutLen);
                        if (ret != 0) {
                            LOG_Error("crypto_encrypt failed: %d", ret);
                            return -1;
                        }

                        written = fwrite(arg->cryptoOutBuf, 1, arg->cryptoOutLen, arg->outPutFile);
                        if (written != (size_t)arg->cryptoOutLen) {
                            LOG_Error("fwrite crypto data failed");
                            return -1;
                        }
                    }
                }

                /* 统一节流：每处理完一批数据休眠 30ms，避免持续占用 CPU */
                if (length > 0) {
                    usleep(30 * 1000);
                }
            }
            break;

        case E_COMPRESS_STAGE_END:
            {
                LOG_Warn("[%s] E_COMPRESS_STAGE_END", __func__);
                if (arg->outPutFile != NULL) {
                    /* 加密时写尾块并释放 crypto 上下文 */
                    if (arg->cryptoInterface != NULL) {
                        arg->cryptoOutLen = 0;
                        ret = arg->cryptoInterface->crypto_finish(arg->cryptoDev, arg->cryptoOutBuf, &arg->cryptoOutLen);
                        if (ret != 0) {
                            LOG_Error("crypto_finish failed: %d", ret);
                            return -1;
                        }

                        written = fwrite(arg->cryptoOutBuf, 1, arg->cryptoOutLen, arg->outPutFile);
                        if (written != (size_t)arg->cryptoOutLen) {
                            LOG_Error("fwrite crypto tail failed");
                            return -1;
                        }

                        arg->cryptoInterface->crypto_free(arg->cryptoDev);
                    }

                    fclose(arg->outPutFile);
                    arg->outPutFile = NULL;
                }
            }
            break;

        case E_COMPRESS_STAGE_FAIL:
            {
                LOG_Error("[%s] E_COMPRESS_STAGE_FAIL", __func__);
                if (arg->outPutFile != NULL) {
                    /* 只有加密时才需要释放 crypto 上下文 */
                    if (arg->cryptoInterface != NULL) {
                        arg->cryptoInterface->crypto_free(arg->cryptoDev);
                    }
                    fclose(arg->outPutFile);
                    arg->outPutFile = NULL;
                }
            }
            break;

        default:
            break;
    }

    return (ssize_t)length;
}

int CompressAndEncryptFile(Crypto_interface_t* cryptoInterface,
        const char *inputFilePath, const char *compressFilePath,  const char *outputFilePath, const char* password) {

    struct stat fileStat;
    struct archive *archive_writer = NULL;
    struct archive_entry *archive_entry = NULL;
    char buffer[ARCHIVE_PAGE_SIZE] = {0};
    ssize_t bytes_read;
    Crypto_Dev_t cryptoDev = {0};
    FILE* inPutFile = NULL;
    int32_t result = 0;

    CompressFileCallBackArg_t compressFileUserData = {
        .compressStage = E_COMPRESS_STAGE_START,
        .compressPassword = password,
        .outputFilePath = outputFilePath,
        .outPutFile = NULL,
        .cryptoInterface = cryptoInterface,
        .cryptoDev = &cryptoDev,
        .cryptoOutLen = 0,
    };

    do {
        /* 1. 创建 libarchive writer，设置格式为 pax_restricted tar */
        archive_writer = archive_write_new();
        if (archive_writer == NULL) {
            LOG_Error("Failed to create archive writer.\n");
            result = -1;
            break;
        }

        /* 2. 设置压缩格式为 gzip */
        if (archive_write_set_format_pax_restricted(archive_writer) != ARCHIVE_OK) {
            LOG_Error("Failed to set archive format: %s\n", archive_error_string(archive_writer));
            result = -2;
            break;
        }

        if (archive_write_add_filter_gzip(archive_writer) != ARCHIVE_OK) {
            LOG_Error("Failed to add gzip filter: %s\n", archive_error_string(archive_writer));
            result = -3;
            break;
        }

        /* 3. 打开自定义写回调；所有压缩数据都会经过 CompressDataCallback 处理后写入文件 */
        if (archive_write_open(archive_writer, &compressFileUserData, NULL, CompressDataCallback, NULL) != ARCHIVE_OK) {
            LOG_Error("Failed to open archive with custom writer: %s\n", archive_error_string(archive_writer));
            result = -4;
            break;
        }

        /* 4. 获取源文件大小，用于写 archive header */
        if (stat(inputFilePath, &fileStat) != 0) {
            LOG_Error("Failed to get file status\n");
            result = -5;
            break;
        }

        /* 5. 创建 archive entry，设置包内文件名、大小等元信息 */
        archive_entry = archive_entry_new();
        if (archive_entry == NULL) {
            LOG_Error("Failed to create archive entry.\n");
            result = -6;
            break;
        }

        archive_entry_set_pathname(archive_entry, compressFilePath);
        archive_entry_set_size(archive_entry, fileStat.st_size);
        archive_entry_set_filetype(archive_entry, AE_IFREG);
        archive_entry_set_perm(archive_entry, 0644);

        if (archive_write_header(archive_writer, archive_entry) != ARCHIVE_OK) {
            LOG_Error("Failed to write archive header: %s\n", archive_error_string(archive_writer));
            result = -7;
            break;
        }

        /** 6. 打开输入文件并初始化加密上下文（写加密头） **/
        inPutFile = fopen(inputFilePath, "rb");
        if (inPutFile == NULL) {
            LOG_Error("Failed to open input file\n");
            result = -8;
            break;
        }

        LOG_Warn("[%s] E_COMPRESS_STAGE_START", __func__);
        compressFileUserData.compressStage = E_COMPRESS_STAGE_START;
        if (CompressDataCallback(NULL, &compressFileUserData, NULL, 0) != 0) {
            LOG_Error("crypto start failed");
            result = -10;
            break;
        }

        /** 7. 循环读取源文件，经 libarchive 压缩后通过回调加密写入 **/
        LOG_Warn("[%s] E_COMPRESS_STAGE_COMPRESSING", __func__);
        compressFileUserData.compressStage = E_COMPRESS_STAGE_COMPRESSING;

        while ((bytes_read = fread(buffer, 1, ARCHIVE_PAGE_SIZE, inPutFile)) > 0) {
            if (archive_write_data(archive_writer, buffer, bytes_read) != bytes_read) {
                LOG_Error("Failed to write data to archive: %s\n", archive_error_string(archive_writer));
                result = -9;
                break;
            }
        }
        if (result != 0) {
            break;
        }

        if (ferror(inPutFile)) {
            LOG_Error("fread input file failed");
            result = -9;
            break;
        }

        result = 0;
    } while (0);

    /* 8. 统一关闭 archive 和输入文件（不判断 archive_write_close 返回值） */
    CompressFileCleanup(archive_writer, archive_entry, inPutFile);
    archive_writer = NULL;
    archive_entry = NULL;
    inPutFile = NULL;

    if (result == 0) {
        /** 9. 写加密尾（PKCS#7 填充等），关闭输出文件 **/
        LOG_Warn("[%s] E_COMPRESS_STAGE_END", __func__);
        compressFileUserData.compressStage = E_COMPRESS_STAGE_END;
        if (CompressDataCallback(NULL, &compressFileUserData, NULL, 0) != 0) {
            LOG_Error("crypto finish failed");
            result = -11;
        }
    }

    if (result != 0) {
        /* 10. 任意步骤失败：释放 crypto 资源并删除可能已损坏的输出文件 */
        LOG_Error("[%s] E_COMPRESS_STAGE_FAIL", __func__);
        compressFileUserData.compressStage = E_COMPRESS_STAGE_FAIL;
        CompressDataCallback(NULL, &compressFileUserData, NULL, 0);
        remove(outputFilePath);
    }

    return result;
}
