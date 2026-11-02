
#ifndef HTTPS__h
#define HTTPS__h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CERTVERIFICATIONLINK     "https://tls-gateway.zelostech.com.cn:8443"  

#define CERTIFICATEAPPLICATIONLINK    "https://tls-gateway.zelostech.com.cn:8083/api/v1/tls_info_create" 
#define HEADER                            "Content-Type: application/json"

typedef struct {

    const char *url;
    char *post_data;
    const char *header1;    
    const char *header2; 
    const char *header3;
    const char *file; 
    FILE * hardler;
    int response;
    int (*httpscallback)(void *ptr, size_t size, size_t nmemb, void *userdata);
} HTTPSClientParams;

int HttpsInit();
void HttpsDeinit();
int HttpsPostClient(const HTTPSClientParams *params);
int HttpsPostClientCertVer(const HTTPSClientParams *params) ;
#ifdef __cplusplus
}
#endif

#endif
