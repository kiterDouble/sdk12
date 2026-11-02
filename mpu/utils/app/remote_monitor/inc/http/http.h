
#ifndef _HTTP_H_
#define _HTTP_H_


#include <iostream>
#include <queue>
#include <string>
#include <cstring>

typedef struct {

    const char *url;
    char *post_data;
    const char *header1;    
    int response;
    int (*httpscallback)(void *ptr, size_t size, size_t nmemb, void *userdata);
} HTTPSClientParams;

int HttpPostClient(const HTTPSClientParams *params) ;
//int HttpOtaPostClient(const char *url,size_t (*fun)(void *ptr, size_t size, size_t nmemb, void *userdata)) ;
void HttpInit();
void HttpDeinit();

#endif
