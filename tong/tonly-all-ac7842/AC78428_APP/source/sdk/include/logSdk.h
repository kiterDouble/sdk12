#ifndef __LOG_SDK_H__
#define __LOG_SDK_H__

#include <stdint.h>

#define TBOX_PRINT(var...)        LogSdk_Print(var)

void LogSdk_Init(uint8_t mode);

void LogSdk_DeInit(void);

void LogSdk_Sleep(void);

void LogSdk_Wakeup(void);


#endif  /* __LOG_SDK_H__ */
