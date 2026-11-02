#ifndef _BCALL_SERVER_API_H_
#define _BCALL_SERVER_API_H_

#include <stdint.h>

/*
* @brief include files here
*
*/
#ifdef __cplusplus
extern "C" {
#endif

typedef enum BcallStatus{
	BCALL_API_STATUS_INIT = 0,
     BCALL_API_STATUS_INCOMING,
     BCALL_API_STATUS_CALLING,
     BCALL_API_STATUS_ACTIVE,
     BCALL_API_STATUS_END,
     BCALL_API_STATUS_ABNORMAL_END,
     BCALL_API_STATUS_WAIT_PSAP,
     BCALL_API_STATUS_SNED_MSD,
     BCALL_API_STATUS_PUSH_MSD,
     BCALL_API_STATUS_UPDATE_MSD,
     BCALL_API_STATUS_MSD_SNED_SUCCESS,
     BCALL_API_STATUS_MSD_SNED_FAULT,
}EnuBcallStatus;

extern int32_t GetBcallStatusInfo(EnuBcallStatus *ecallStatus,int32_t timeOut);

#ifdef __cplusplus
}
#endif

#endif //_BCALL_SERVER_API_H_

