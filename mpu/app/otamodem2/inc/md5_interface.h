#ifndef __MD5_INTERFACE_H__
#define __MD5_INTERFACE_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <openssl/md5.h>
int CalculateMd5(const char *filename, unsigned char output[MD5_DIGEST_LENGTH]) ;

#ifdef __cplusplus
}
#endif
#endif