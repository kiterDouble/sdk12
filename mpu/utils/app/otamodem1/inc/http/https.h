
#ifndef HTTPS__h
#define HTTPS__h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define  TOKENHEADER1              "Content-Type: application/json"
#define  TOKENHEADER2              "auth.zelostech.com.cn"

#define   ACCESSTOKEN                        "https://auth.zelostech.com.cn/app/accessToken"
#define   APPID                              "chassisTbox"
#define   APPKEY                             "nc4foB1pnR3jiAFPkzzqdZDHYBvJIpj1To7452cd4KWyb37DYG3"

#define   PollingVersionInformation          "http://gateway.zelostech.com.cn/zelos-service/chassisTbox/queryVersion/"
#define   REPORTVEHICLEINFORMATION            "http://gateway.zelostech.com.cn/zelos-service/chassisTbox/info/"
#define   DOWNLOADUPGRADEPACKAGEFILEPATH     "/oemdata/upgrade/download/Tbox.tar.gz"
#define   PATHUPGRADE                        "/oemdata/upgrade/"
#define   HEADER1                            "gateway.zelostech.com.cn"
#define   TBOXUPGRADEPACKAGEFILEPATH        "/oemdata/upgrade/download/"
#define   MD5DATA                           "/oemdata/upgrade/md5.txt"

#define BT1BINFILEPATH                     "/oemdata/upgrade/bt_1.bin"
#define BT2BINFILEPATH                     "/oemdata/upgrade/bt_2.bin"
#define MCU1BINFILEPATH                     "/oemdata/upgrade/mcu_1.bin"
#define MCU2BINFILEPATH                     "/oemdata/upgrade/mcu_2.bin"
#define MPUBINFILEPATHMPUBIN                "/oemdata/upgrade/update.zip"

#define   TOKENFILEPATH                      "/oemdata/token/token.txt"
#define   RMTOKENFILE                        "rm -rf /oemdata/token/token.txt"

#define MCU1BIN                              "mcu_1.bin"
#define MCU2BIN                              "mcu_2.bin"
#define MPUBIN                              "update.zip"
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
int HttpsGetClient( HTTPSClientParams *params) ;
int  HttpPostClient(const HTTPSClientParams *params) ;

#ifdef __cplusplus
}
#endif

#endif
