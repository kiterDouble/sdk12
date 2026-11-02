#ifndef _OTAMODULE_INTERFACE_H_
#define _OTAMODULE_INTERFACE_H_


#define SERVERIP                             "192.168.205.15"
#define UPGRADESUCCESS                       "Upgrade Success"
#define UPGRADEFAIL                          "Upgrade Fail"
#define UPGRADESTART                          "Upgrade start"
#define PORT 8080
#define BUFFER_SIZE 1024 

void * OtaModule2(void *arg);

#endif