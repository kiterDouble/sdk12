
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


int32_t CreateDetachedThread(pthread_t *threadId, threadFun fun, void *threadArg)
{
    pthread_attr_t attr;
    int32_t retValue = -1;

    if (threadId == NULL || fun == NULL)
    {
        LOG_Error("%s argv invalid", __FUNCTION__);
        return -1;
    }

    /* 初始化属性 */
    retValue = pthread_attr_init(&attr);
    if (retValue != 0)
    {
        LOG_Error("pthread_attr_init fail: %s", strerror(retValue));
        return -1;
    }

    /* 设置线程分离态 */
    retValue = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (retValue != 0)
    {
        LOG_Error("pthread_attr_setdetachstate fail: %s", strerror(retValue));
        pthread_attr_destroy(&attr);
        return -1;
    }

    /* 创建线程 */
    retValue = pthread_create(threadId, &attr, fun, threadArg);
    if (retValue != 0)
    {
        LOG_Error("pthread_create fail: %s", strerror(retValue));
        pthread_attr_destroy(&attr);
        return -1;
    }

    /* 销毁属性对象 */
    pthread_attr_destroy(&attr);

    LOG_Debug("Create Detached Thread success, tid=%lu",
              (unsigned long)(*threadId));

    return 0;
}