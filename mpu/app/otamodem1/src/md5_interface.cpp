#include <stdio.h>
#include<iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <openssl/md5.h>
#include "sr_log.h"
#include "otamodem1.h"
#include "md5_interface.h"
#include "system_interface.h"
#include "upgrade_interface.h"
#include "file_interface.h"


int CalculateMd5(const char *filename, unsigned char output[MD5_DIGEST_LENGTH]) 
{
    unsigned char buffer[1024];
    size_t bytes;
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Unable to open file \n");
        LOG_Error("Unable to open file");
        return -1;
    }

    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);
    // 读取文件并更新 MD5
    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) != 0) {
        MD5_Update(&md5_ctx, buffer, bytes);
    }

    MD5_Final(output, &md5_ctx);
    fclose(file);

    return 0;

}

int VerifyFileMd5(const char* filepath, const char* filename, int lineNum) {
    char line[65] = {0};
    unsigned char outbytes[MD5_DIGEST_LENGTH] = {0};
    unsigned char output[MD5_DIGEST_LENGTH] = {0};
    int ret;

    if(access(filepath, F_OK) != 0) {
        return 0; // 文件不存在，跳过校验
    }

    // 读取对应行的MD5值
    if (ReadSpecificLine(MD5DATA, lineNum, line, 64) != 0) {
        printf("%s文件第%d行读取错误\n", filename, lineNum);
        LOG_Error("%s文件第%d行读取错误\n", filename, lineNum);
        return -3;
    }

    LOG_Debug("%s文件第%d行: %s\n", filename, lineNum, line);

    // 计算文件实际MD5值
    ret = CalculateMd5(filepath, output);
    if(ret < 0) {
        printf("%s CalculateMd5 Error\n", filename);
        LOG_Error("%s CalculateMd5 Error\n", filename);
        return -3;
    }

    // 转换并比较MD5值
    HexStringToBytes(line, outbytes, strlen(line));
    if(memcmp(outbytes, output, MD5_DIGEST_LENGTH) != 0) {
        printf("%s升级包md5对比不一致...\n", filename);
        LOG_Error("%s升级包md5对比不一致...\n", filename);
        return -3;
    }

    LOG_Debug("%s CalculateMd5 memcmp ok\n", filename);
    return 0;
}

void HexStringToBytes(const char *hex_string, unsigned char *bytes, size_t length) {
    for (size_t i = 0; i < length; i++) {
        sscanf(hex_string + 2 * i, "%2hhx", &bytes[i]);
    }
}
