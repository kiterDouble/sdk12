#ifndef _SYSTEM_INTERFACE_H_
#define _SYSTEM_INTERFACE_H_

// #ifdef __cplusplus
// extern "C"
// {
// #endif


gboolean GetCarNum(uint8_t *carNum, size_t len);
int DomainNameResolution();
int GetSystemRetValue(const char *cmd);
int CheckTimeSync();
int IsNetworkAvailable();
int GetSystemRetValue(const char *cmd);
// #ifdef __cplusplus
// }
// #endif

#endif