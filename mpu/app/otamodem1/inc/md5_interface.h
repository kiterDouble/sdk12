#ifndef __MD5_INTERFACE_H__
#define __MD5_INTERFACE_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <openssl/md5.h>

int CalculateMd5(const char *filename, unsigned char output[MD5_DIGEST_LENGTH]);
int VerifyFileMd5(const char* filepath, const char* filename, int lineNum);
void HexStringToBytes(const char *hex_string, unsigned char *bytes, size_t length);

#ifdef __cplusplus
}
#endif
#endif
