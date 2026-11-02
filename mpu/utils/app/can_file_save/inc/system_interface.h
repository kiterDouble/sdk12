#ifndef __SYSTEM_INTERFACE_H__
#define __SYSTEM_INTERFACE_H__


#ifdef __cplusplus
extern "C" {
#endif

int GetSystemRetValue(const char *cmd);
int CheckTimeSync();
int GetEmmcSize();


#ifdef __cplusplus
}
#endif
#endif