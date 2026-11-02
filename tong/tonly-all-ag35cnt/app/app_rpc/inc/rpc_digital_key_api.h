#ifndef RPC_DIGITAL_KEY_API_H
#define RPC_DIGITAL_KEY_API_H

#include <stdint.h>

#include "sr_ipc.h"

#define RPC_DIGITALKEY_MAX_BUFFER_LEN          (256U)
#define BLE_PAIRED_MAX_NUM 7

typedef enum{
    E_RPC_DIGITAL_KEY_SUBCMD_QUERY_STATE = 0x0,
    E_RPC_DIGITAL_KEY_SUBCMD_BLE_UPDATE,
    E_RPC_DIGITAL_KEY_SUBCMD_DEL_KEY,
}RpcDigitalKeySubcmd_e;


enum{
    E_OTA_RET_SUCCESS = 0x00,           /* OTA 升级成功 */
    E_OTA_RET_FAILED = 0x01,            /* OTA 文件无效 */
    E_OTA_RET_BUSY = 0x02,              /* OTA 任务忙 */
    E_OTA_RET_INVALID_FILE = 0x04,      /* OTA 文件无效 */
    E_OTA_RET_TIMEOUT = 0x05,           /* OTA 升级超时 */
};

typedef struct{
    struct{
        struct{
            uint32_t advertising:1;      //广播状态
            uint32_t connected:1;        //连接状态
            uint32_t bonded:1;           //绑定状态
            uint32_t pairing:1;          //配对中
            uint32_t termiating:1;       //断开中
            uint32_t disk:1;
            uint32_t pmState:1;          // 休眠唤醒状态(0: 休眠，1:工作)
            uint32_t reserved:24;
            uint16_t mtu;
        }state;
        struct{
            uint32_t shankhand:1;      //握手状态
            uint32_t calibration:1;    //标定模式
            uint32_t reserved:30;
        }mode;
        char fwVersion[32];
        uint8_t bleMac[6];
        uint8_t connectedMac[6];
        struct{
            uint8_t addrType;
            uint8_t mac[6];
        }pairedMac[BLE_PAIRED_MAX_NUM];
        uint8_t pairedMacNum;
    }bleState;
    uint8_t otaState;
}DigitalKeyState_t;

typedef struct{
    uint32_t progress;
    uint32_t total;
}DigitalKeyUpgradeProgress_t;

typedef void (*RpcDigitakKeyUpdateBleProgressCallBack)(const DigitalKeyUpgradeProgress_t *progressData, const uint16_t dataSize, void* userdata);

/** 查询数字钥匙状态 **/
int32_t RpcDigitalKeyQueryState(DigitalKeyState_t* state);

/** 更新数字钥匙蓝牙固件 **/
int32_t RpcDigitalKeyUpdateBleFirmware(const char* fwPath, int32_t timeoutSec, RpcDigitakKeyUpdateBleProgressCallBack cbFunc, void* userData);

/** 删除数字钥匙 **/
int32_t RpcDigitalKeyDelKey(const char* keyId);


#endif
