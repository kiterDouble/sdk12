
#include<string.h>
#include "sr_log.h"
#include "err_code.h"
#include "thread_interface.h"



int32_t CreateThread(pthread_t *threadId, threadFun fun, void *threadArg)
{
	pthread_attr_t attr;
	int32_t retValue = -1;

	if(NULL == threadId )
	{
		LOG_Error("%s argv threadId  is NULL", __FUNCTION__);
		return RET_ERR;
	}

	pthread_attr_init(&attr);

	//创建线程
	retValue = pthread_create(threadId, &attr, fun,(void *)threadArg);
	if(retValue < 0)
	{
		LOG_Error("pthread_create %s\n",strerror(errno));
		return RET_ERR;
	}

	LOG_Debug("mqtt  thread success");
	return RET_OK;
}