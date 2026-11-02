#ifndef __MCU_LOCK_H__
#define __MCU_LOCK_H__
#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name setSendDataFlag
 * @brief 
 * @param 
 * @return 
 */
void setSendDataFlag(uint8_t flag);

/**
 * @name getSendDataFlag
 * @brief
 * @param 
 * @return 
 */
uint8_t getSendDataFlag();

#ifdef __cplusplus
}
#endif
#endif 