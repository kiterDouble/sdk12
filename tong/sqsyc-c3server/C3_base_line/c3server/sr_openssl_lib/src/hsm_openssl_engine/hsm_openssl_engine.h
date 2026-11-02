#ifndef HSM_OPENSSL_ENGINE_H
#define HSM_OPENSSL_ENGINE_H

#include <stdint.h>

#include <openssl/engine.h>


void OpensslEngineSwitchKeyId(uint16_t keyId);

int OpensslEngineInit(ENGINE** engine);

void OpensslEngineDeinit(ENGINE** engine);

int OpensslRsaGenerateTmpKey(EVP_PKEY **pkey, int bits);

#endif