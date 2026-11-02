
#ifndef __SR_HAL_HSM__
#define __SR_HAL_HSM__

#include <openssl/engine.h>
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "fmse_drv.h"
#include "fmse_cmd.h"
#include "sr_log.h"

int32_t HalHsmInit(void);
void HalHsmDeinit(void);

int32_t HalHsmRand(unsigned char *buf, int num);
int32_t HalHsmSHA256Update(EVP_MD_CTX *ctx, const void *data, size_t count);
int32_t HalHsmRSAPubEnc(uint16_t key_Fid,int32_t flen, int32_t rsabits, const unsigned char *rand_buf, const unsigned char *from, unsigned char *to);

int32_t HalHsmRSASign(uint16_t key_Fid,int32_t rsabits, unsigned char *der, unsigned int derlen,
                                    unsigned char *sigret, unsigned int *siglen);
int32_t HalHsmRSAVerify(uint16_t key_Fid,int32_t rsabits, uint16_t *rlen, uint8_t* rbuf,
                                    const unsigned char *sigbuf, unsigned int siglen);
int32_t HalHsmRSAKeygen(int32_t bits, BIGNUM *e, BIGNUM **n, BIGNUM **d);
int32_t HalHsmImportRSAPrivate(const char *rsa,uint16_t pub_Fid,uint16_t pri_Fid);
int32_t HalHsmRSAGetPubKey(uint16_t key_Fid,int32_t bits, BIGNUM **n);
int32_t HalHsmRSAPrivEnc(uint16_t key_Fid,int32_t flen, int32_t rsabits, const unsigned char *from, unsigned char *to);
int32_t HalHsmRSAPrivDec(uint16_t key_Fid,int32_t flen, int32_t rsabits, const unsigned char *from, unsigned char *to);
int32_t HalHsmEnterSleepMode();
#endif