#ifndef __UPGRADE_INTERFACE_H__
#define __UPGRADE_INTERFACE_H__


#ifdef __cplusplus
extern "C" {
#endif

#define     DOWNLOADUPGRADEPACKAGEFILEPATH     "/oemdata/upgrade/download/Tbox.tar.gz"
#define     PATHUPGRADE                        "/oemdata/upgrade/"
#define     TBOXUPGRADEPACKAGEFILEPATH         "/oemdata/upgrade/download/"
#define     MD5DATA                            "/oemdata/upgrade/md5.txt"


#define     MCU1BINFILEPATH                    "/oemdata/upgrade/mcu_1.bin"
#define     MCU2BINFILEPATH                    "/oemdata/upgrade/mcu_2.bin"
#define     MPUBINFILEPATHMPUBIN               "/oemdata/upgrade/update.zip"

#define     BT1BINFILEPATH                     "/oemdata/upgrade/bt_1.bin"
#define     BT2BINFILEPATH                     "/oemdata/upgrade/bt_2.bin"

int DoUpgrade();

#ifdef __cplusplus
}
#endif
#endif