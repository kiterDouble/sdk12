/**
 * @file      utils.c
 * @brief     related functions
 * @author    shaobo.wang
 * @version   v1
 * @date      2023-09-12 15:43:20
 * @copyright Copyright (c) V2XLINK
 */
 
#include <string.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include "utils.h"

/**
 * @brief 对输入数据进行 Base64 编码。
 *
 * @param[out] out 输出缓冲区，用于存储编码后的数据。
 * @param[in] in 输入数据的缓冲区。
 * @param[in] inLen 输入数据的长度。
 * @return 成功时返回编码后数据的长度，失败时返回 -1。
 */
int32_t Base64Encode(uint8_t *out, uint8_t *in, uint32_t inLen)
{
    if (!out || !in || inLen == 0)
        return -1;

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, in, inLen);
    BIO_flush(bio);

    BUF_MEM *bptr = NULL;
    BIO_get_mem_ptr(bio, &bptr);
    uint32_t size = bptr->length;
    memcpy(out, bptr->data, size);

    BIO_free_all(bio);

    return size;
}

/**
 * @brief 对输入数据进行 Base64 解码。
 *
 * @param[out] out 输出缓冲区，用于存储解码后的数据。
 * @param[in] in 输入数据的缓冲区。
 * @param[in] inLen 输入数据的长度。
 * @return 成功时返回解码后数据的长度，失败时返回 -1。
 */
int32_t Base64Decode(uint8_t *out, uint8_t *in, uint32_t inLen)
{
    if (!out || !in || inLen == 0)
        return -1;

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    BIO *bio = BIO_new_mem_buf(in, inLen);
    bio = BIO_push(b64, bio);

    uint32_t size = BIO_read(bio, out, inLen);

    BIO_free_all(bio);

    return size;
}

/**
 * @brief 对输入数据进行 MD5 哈希计算。
 *
 * @param[out] out 输出缓冲区，用于存储计算得到的 MD5 哈希值。
 * @param[in] in 输入数据的缓冲区。
 * @param[in] inLen 输入数据的长度。
 * @return 成功时返回 1，失败时返回 -1。
 */
int32_t Md5(uint8_t *out, uint8_t *in, uint32_t inLen)
{
    if (!out || !in || inLen == 0)
        return -1;

    MD5(in, inLen, out);

    return 1;
}

/**
 * @brief 对输入数据进行 SHA-256 哈希计算。
 *
 * @param[out] out 输出缓冲区，用于存储计算得到的 SHA-256 哈希值。
 * @param[in] in 输入数据的缓冲区。
 * @param[in] inLen 输入数据的长度。
 * @return 成功时返回 1，失败时返回 -1。
 */
int32_t Sha256(uint8_t *out, uint8_t *in, uint32_t inLen)
{
    if (!out || !in || inLen == 0)
        return -1;

    SHA256(in, inLen, out);

    return 1;
}
