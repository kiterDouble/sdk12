#ifndef _ETHERBET_UPGRADE_H_
#define _ETHERBET_UPGRADE_H_


#define PORT 8080
#define BUFFER_SIZE  20480
#define SERVER_IP                             "192.168.205.15"

#define   DOWNLOADUPGRADEPACKAGEFILEPATH     "/oemdata/upgrade/ethernet/"
#define   PATHUPGRADE                        "/oemdata/upgrade/"


#define MCU1BIN                              "/oemdata/upgrade/mcu_1.bin"
#define MCU2BIN                              "/oemdata/upgrade/mcu_2.bin"
#define MPUBIN                               "/oemdata/upgrade/update.zip"
#define CREATFILEETHERNET                    "touch /oemdata/upgrade/ethernet/ethernet.txt"
#define RMFILEETHERNET                       "rm -rf  /oemdata/upgrade/ethernet/ethernet.txt"


#define UPGRADESUCCESS                       "Upgrade Success"
#define UPGRADEFAIL                          "Upgrade Fail"
#define UPGRADETARFAIL                       "Upgrade Tar Fail"
#define UPGRADEOPENMD5FILEFAIL               "Upgrade Open MD5 File Fail"
#define UPGRADEMD5COMFAIL                     "Upgrade MD5 COM Fail"
#define UPGRADESTART                          "Upgrade start"


#define RPCSERVER_TYPE_OTAAPP  (0x70001002)
#define RPCAPPOTA_MSGTYPE_SUBCMD_ID1 (0)
void handle_signal(int sig);
#endif




