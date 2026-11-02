#ifndef _HTTP_THREAD_H_
#define _HTTP_THREAD_H_


#include<thread>


int ThreadInit();
void DeInit();
void SetAppOtaStatus(unsigned char data);
unsigned char GetAppOtaStatus();


#endif