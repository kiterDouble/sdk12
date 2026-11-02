#ifndef _TIME_INTERFACE_H_
#define _TIME_INTERFACE_H_


typedef void (*TokenTimerCallback)(union sigval);

void TimerHandler(union sigval sv);
int TokenTimerInit(TokenTimerCallback callback);
void DelayMS(unsigned int time) ;
#endif