#ifndef _TIMER_INTERFACE_H_
#define _TIMER_INTERFACE_H_

#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>


uint64_t GetCurrentTime();
void DelayMS(unsigned int time) ;




#endif