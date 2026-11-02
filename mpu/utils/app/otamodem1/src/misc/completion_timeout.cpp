#include "completion_timeout.h"
 
static completion_t comp;
static completion_t otacomp;
static pthread_mutex_t mutexlock = PTHREAD_MUTEX_INITIALIZER;

void PthreadMutexLock()
{
    pthread_mutex_lock(&mutexlock);
}

void PthreadMutexUnLock()
{
    pthread_mutex_unlock(&mutexlock);
}

void PthreadMutexDestroy()
{
    pthread_mutex_destroy(&mutexlock);
}
completion_t *GetComp()
{
    return &comp;
}

completion_t *GetOtaComp()
{
    return &otacomp;
}

void InitCompletion(completion_t *c) 
{
    pthread_mutex_init(&c->mutex, NULL);
    pthread_cond_init(&c->cond, NULL);
    c->completed = 0;
}

void CleanupCompletion(completion_t *c) 
{
    pthread_mutex_destroy(&c->mutex);
    pthread_cond_destroy(&c->cond);
}


int WaitForCompletionTimeout(completion_t *c, int timeout) 
{
    struct timespec ts;
    int ret = 0;

    pthread_mutex_lock(&c->mutex);
    if(timeout < 0)
    {
        while (!c->completed) 
        {
            pthread_cond_wait(&c->cond, &c->mutex);
        }

    }else{
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout;

        while (!c->completed) 
        {
            ret = pthread_cond_timedwait(&c->cond, &c->mutex, &ts);
            if (ret == ETIMEDOUT) {
                break;
            }
        }
    }

    int completed = c->completed;
    c->completed = 0;
    pthread_mutex_unlock(&c->mutex);
    return completed;
}


void Complete(completion_t *c) 
{
    pthread_mutex_lock(&c->mutex);
    c->completed = 1;
    pthread_cond_signal(&c->cond);
    pthread_mutex_unlock(&c->mutex);
}




