#ifndef _ECALL_SERVER_API_H_
#define _ECALL_SERVER_API_H_

#include <stdint.h>

/*
* @brief include files here
*
*/
#ifdef __cplusplus
extern "C" {
#endif

typedef enum EcallStatus{
	 ECALL_API_STATUS_INIT = 0,
     ECALL_API_STATUS_INCOMING,
     ECALL_API_STATUS_CALLING,
     ECALL_API_STATUS_ACTIVE,
     ECALL_API_STATUS_END,
     ECALL_API_STATUS_ABNORMAL_END,
     ECALL_API_STATUS_WAIT_PSAP,
     ECALL_API_STATUS_SNED_MSD,
     ECALL_API_STATUS_PUSH_MSD,
     ECALL_API_STATUS_UPDATE_MSD,
     ECALL_API_STATUS_MSD_SNED_SUCCESS,
     ECALL_API_STATUS_MSD_SNED_FAULT,
     MAX_ECALL_API_STATUS,
}EnuEcallStatus;

extern int32_t GetEcallStatusInfo(EnuEcallStatus *ecallStatus,int32_t timeOut);
extern const char *GetEcallStatusString(EnuEcallStatus status);

#ifdef __cplusplus
}
#endif

#endif //_ECALL_SERVER_API_H_

