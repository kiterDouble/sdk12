
#include "sr_log.h"
#include "thread_interface.h"


int32_t CreateThread(pthread_t *threadId, threadFun fun, void *threadArg)
{
	pthread_attr_t attr;
	int32_t retValue = -1;

	if(NULL == threadId )
	{
		LOG_Error("%s argv threadId  is NULL", __FUNCTION__);
		return -1;
	}

	pthread_attr_init(&attr);
	retValue = pthread_create(threadId, &attr, fun,(void *)threadArg);
	if(retValue < 0)
	{
		LOG_Error("pthread_create %s",strerror(errno));
		return -1;
	}

	LOG_Debug("Create Thread  success");
	return 0;
}


