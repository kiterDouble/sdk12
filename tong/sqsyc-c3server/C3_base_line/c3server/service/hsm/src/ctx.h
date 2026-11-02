
#ifndef HSM_CTX_H
#define HSM_CTX_H

#include <stdint.h>
#include <glib.h>

#include "hsm_api.h"
#include "pm_server_api.h"
#include "hsm_config.h"

#define HSM_INI_FILE_PATH           "/oemapp/config/hsm.ini"
#define HSM_INI_FILE_PATH_PRIMARY   "/oemdata/config/hsm.ini"
#define HSM_INIT_FAIL_COUNT_MAX     12

typedef enum
{
    E_HSM_COMM_METHOD_SPI = 1,
    E_HSM_COMM_METHOD_I2C,
} HsmCommMethod_e;

typedef struct
{
    HsmWorkState_e hsmWorkState; /* HSM工作状态：Normal/Fault/Unknown */
    uint8_t pmState;
    HsmApi_t* hsmApi;
    void* hsmUserData;           /* HSM后端私有上下文，由main分配/释放 */
    HsmConfig_t config;
    GMutex hsmApiMutex;          /* 保护 hsmApi/hsmUserData 的 init/release 与 RPC 并发访问 */
}Ctx_t;

#endif
