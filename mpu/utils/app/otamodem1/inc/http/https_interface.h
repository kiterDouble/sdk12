#ifndef HTTPS_INTERFACE__H
#define HTTPS_INTERFACE__H


#include <stdio.h>
#include <stddef.h>
#include "cjson_interface.h"

void * HTTPSThread(void *arg);
int TokenResponseHandler(void *response, size_t size, size_t nmemb, void *userdata) ;
void SetOtaFlag(int data);
int ReportVehicleInformation(ComponentData *data,UpgradeData *upgradedata,char *jsonstring,int flag);

#endif