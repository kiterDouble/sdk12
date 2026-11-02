#ifndef _THREAD_INTERFACE_H_
#define _THREAD_INTERFACE_H_


#include<thread>


typedef void *(threadFun)(void *);
int32_t CreateThread(pthread_t *threadId, threadFun fun, void *threadArg);



#endif