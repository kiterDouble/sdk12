#ifndef _COMPLETION_TIMEOUT_INTERFACE_H_
#define _COMPLETION_TIMEOUT_INTERFACE_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>


typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int completed;
} completion_t;


completion_t *GetComp();
void InitCompletion(completion_t *c); 
void CleanupCompletion(completion_t *c); 
int WaitForCompletionTimeout(completion_t *c, int timeout); 
void Complete(completion_t *c);
completion_t *GetOtaComp();

void PthreadMutexLock();
void PthreadMutexUnLock();
void PthreadMutexDestroy();

#endif
