#include <stdlib.h>
#include <string.h>

#include "err_code.h"
#include "sr_log.h"
#include "sr_ipc.h"
#include "rpc_digital_key_api.h"


/** 查询数字钥匙状态 **/
int32_t RpcDigitalKeyQueryState(DigitalKeyState_t* state)
{
    int32_t ret = RET_ERR;
    uint8_t buffer[RPC_DIGITALKEY_MAX_BUFFER_LEN] = {0x0};
    uint16_t recvLen = sizeof(buffer);

    do{
        if (state == NULL) {
            break;
        }

        ret = RpcGetMsgSync(RPCSERVER_TYPE_DIGITALKEY, E_RPC_DIGITAL_KEY_SUBCMD_QUERY_STATE, 1000, 0, 0, (void *)buffer, &recvLen);
        if (ret != RET_OK) {
            LOG_Error("RpcGetMsgSync failed, ret=%d" , ret);
            break;
        }

        /* 校验枚举范围，避免上游越界取值 */
        if (recvLen != sizeof(DigitalKeyState_t)){
            LOG_Error("recvLen != sizeof(DigitalKeyState_t) recvLen=%d", recvLen);
            ret = RET_ERR;
            break;
        }

        (void)memcpy(state, &buffer, sizeof(DigitalKeyState_t));
    }while(0);

    return ret;
}

/** 更新数字钥匙蓝牙固件 **/
int32_t RpcDigitalKeyUpdateBleFirmware(const char* fwPath, int32_t timeoutSec, RpcDigitakKeyUpdateBleProgressCallBack cbFunc, void* userData)
{
    int32_t ret = RET_ERR;
    int32_t upgradeResult = 0;
    uint16_t recvLen = sizeof(upgradeResult);

    SubscriptionQueueMsg(SYS_QUE_TYPE_BLE_DIGITAL_KEY_OTA_PROGRESS_NOTIFY, (QueSubCallback) cbFunc, userData);

    do{
        if (fwPath == NULL || strlen(fwPath) == 0 || strlen(fwPath) >= RPC_DIGITALKEY_MAX_BUFFER_LEN) {
            break;
        }
    
        ret = RpcGetMsgSync(RPCSERVER_TYPE_DIGITALKEY, E_RPC_DIGITAL_KEY_SUBCMD_BLE_UPDATE, timeoutSec*1000,
                                        (void*)fwPath, (uint16_t)strlen(fwPath), (void *)&upgradeResult, &recvLen);
        if (ret != RET_OK) {
            LOG_Error("RpcGetMsgSync failed, ret=%d", ret);
            break;
        }

        /* 校验枚举范围，避免上游越界取值 */
        if (recvLen != sizeof(int32_t)){
            LOG_Error("recvLen != sizeof(int32_t) recvLen=%d", recvLen);
            ret = RET_ERR;
            break;
        }

        ret = upgradeResult;
    }while(0);

    UnSubscriptionQueueMsg(SYS_QUE_TYPE_BLE_DIGITAL_KEY_OTA_PROGRESS_NOTIFY);

    return ret;
}

/** 删除数字钥匙 **/
int32_t RpcDigitalKeyDelKey(const char* keyId)
{
    int32_t ret = RET_ERR;
    int32_t upgradeResult = 0;
    uint16_t recvLen = sizeof(upgradeResult);

    do{
        if (keyId == NULL || strlen(keyId) == 0) {
            break;
        }
    
        ret = RpcGetMsgSync(RPCSERVER_TYPE_DIGITALKEY, E_RPC_DIGITAL_KEY_SUBCMD_DEL_KEY, 1000, (uint8_t*)keyId, (uint16_t)strlen(keyId), (void *)&upgradeResult, &recvLen);
        if (ret != RET_OK) {
            LOG_Error("RpcGetMsgSync failed, ret=%d", ret);
            break;
        }

        /* 校验枚举范围，避免上游越界取值 */
        if (recvLen != sizeof(int32_t)){
            LOG_Error("recvLen != sizeof(int32_t) recvLen=%d", recvLen);
            ret = RET_ERR;
            break;
        }

        ret = upgradeResult;
    }while(0);

    return ret;
}
