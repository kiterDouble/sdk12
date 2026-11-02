#ifndef __MCU_DATA_H__
#define __MCU_DATA_H__
#include "mcu_ipc_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MCU_RECV_BUFFER_LEN (4096U)

typedef struct
{
    uint32_t speed;
    uint32_t param1;//uart flowCtrl or spi mode
    uint32_t param2;//uart dataBit or spi bits
    uint32_t param3;//uart stopBit or spi cpuPinn
    uint32_t param4;//uart parity or spi mcuPinn
    uint8_t device[64];
    uint8_t enable;
}McuSrvCfg_t;

typedef struct
{
    int32_t dataBusFd;
    int32_t upgradeStatus;
    McuSrvCfg_t cfg;
    int32_t (*BusOpen)(McuSrvCfg_t *cfg);
    int32_t (*busRead)(int32_t fd, uint8_t *data, int32_t dataLen);
    int32_t (*BusWrite)(int32_t fd, const uint8_t *data, int32_t dataLen);
    void (*BusClose)(int32_t fd);
    uint8_t recvBuf[MCU_RECV_BUFFER_LEN];
    uint32_t recvLen;
}McuSrvData_t;

typedef struct
{
    McuSrvData_t uartData;
    McuSrvData_t spiData;
}McuSrv_t;

#ifdef __cplusplus
}
#endif
#endif 