#ifndef _OTA_MODEM2_H_
#define _OTA_MODEM2_H_

#include <stdio.h>  

#define     PORT            8080
#define     BUFFER_SIZE     20480
#define     REMOVEUPGRADEFILE                  "rm -rf /oemdata/upgrade/download/Tbox.tar.gz"




#define     MCU1BIN                             "mcu_1.bin"
#define     MCU2BIN                             "mcu_2.bin"
#define     MPUBIN                              "update.zip"
#define     UPGRADESUCCESS                      "Upgrade Success"
#define     UPGRADEFAIL                         "Upgrade Fail"
#define     DATAERROR                           "Data Error"
#define     UPGRADETARFAIL                      "Upgrade Tar Fail"
#define     UPGRADEOPENMD5FILEFAIL              "Upgrade Open MD5 File Fail"
#define     UPGRADEMD5COMFAIL                   "Upgrade MD5 COM Fail"
#define     UPGRADESTART                        "Upgrade start"
#define     EMMCDEVICENODE                      "/dev/mmcblk0"


#endif