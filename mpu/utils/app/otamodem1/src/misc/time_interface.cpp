#include <time.h>      
#include <signal.h>    
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sr_log.h"
#include "https.h"
#include "cjson_interface.h"
#include "time_interface.h"
#include "https_interface.h"
#include "system_interface.h"
#include "completion_timeout.h"
static timer_t timer_id;


void TimerHandler(union sigval sv) 
{
    HTTPSClientParams params = {0};
    LOG_Debug("TimerHandler Start ...");
//    SetOtaFlag(1);
    PthreadMutexLock();
	params.header1 = TOKENHEADER1;
	params.header2 = TOKENHEADER2;
	params.httpscallback = TokenResponseHandler;
    params.url = ACCESSTOKEN;
	params.post_data = CreateJsonData(APPID, APPKEY); 
    LOG_Debug("token data = %s",params.post_data);

    for(int i =0 ; i < 3;i++)
    {
        if (HttpsPostClient(&params) == 0) 
        {
            LOG_Debug("TimerHandler HTTPS POST request succeeded.\n");
            break;
        } else {

            LOG_Error("TimerHandler HTTPS POST request failed.\n");
        }
    }
	
	free(params.post_data);
//    SetOtaFlag(0);
    PthreadMutexUnLock();
}

int TokenTimerInit(TokenTimerCallback callback)
{
    struct sigevent sev = {0};
    struct itimerspec its = {0};
    LOG_Debug("TokenTimerInit start\n");
    
    
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = callback;
    sev.sigev_value.sival_ptr = NULL;

    if (timer_create(CLOCK_REALTIME_ALARM, &sev, &timer_id) == -1) 
    {
        LOG_Error("timer_create CLOCK_REALTIME_ALARM fail \n");
        return -1;
    }

    its.it_value.tv_sec = 2;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 20 * 3600;
    its.it_interval.tv_nsec = 0;

    if (timer_settime(timer_id, 0, &its, NULL) == -1) {
        LOG_Error("timer_settime fail \n");
        timer_delete(timer_id);
        timer_id = 0;
        return -1;
        
    }

    return 0;
}



void DelayMS(unsigned int time)
{
    struct timespec req = {
        .tv_sec  = 0,          // 秒
        .tv_nsec = time * 1000000 // 纳秒 (10ms = 10,000,000ns)
    };
    struct timespec rem;

    while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
        req = rem; // 被信号中断后继续剩余延时
    }
}