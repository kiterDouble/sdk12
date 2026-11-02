//
// Created by Jaxsen on 2025/5/12.
//

#ifndef SR_TSP_FW_CA_CERT_HANDLER_H
#define SR_TSP_FW_CA_CERT_HANDLER_H
#include "srTspFWOs.h"
#ifdef __cplusplus
extern "C"
{
#endif
    int32_t SRTspGetCaPath(char *buffer, size_t len);
    int32_t SRTspGetClientRetPath(char *buffer, size_t len);
    int32_t SRTspGetClientKeyPath(char *buffer, size_t len);

    int32_t SRTspWriteCaCrt(const char *dirPath);
    int32_t SRTspWriteClientCrt(const char *dirPath);
    int32_t SRTspWriteClientKey(const char *dirPath);
#ifdef __cplusplus
}
#endif
#endif
