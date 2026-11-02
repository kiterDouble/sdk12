#ifndef MD5_H
#define MD5_H

#include <stdint.h>
#include <stddef.h>

// MD5 哈希结果长度 16 字节
#define MD5_BLOCK_LEN    64
#define MD5_DIGEST_LEN   16

// MD5 上下文结构体
typedef struct {
    uint32_t state[4];
    uint8_t  buf[MD5_BLOCK_LEN];
    uint64_t len;
} MD5_CTX;

// 初始化 MD5 上下文
void md5_init(MD5_CTX *ctx);

// 追加数据
void md5_update(MD5_CTX *ctx, const uint8_t *data, size_t len);

// 完成计算，输出16字节二进制摘要
void md5_final(MD5_CTX *ctx, uint8_t digest[MD5_DIGEST_LEN]);

// 一键计算 MD5（简化接口）
void md5_calc(const uint8_t *data, size_t len, uint8_t digest[MD5_DIGEST_LEN]);

// 二进制摘要转32位小写字符串
void md5_bin2str(const uint8_t digest[MD5_DIGEST_LEN], char str[33]);

#endif

