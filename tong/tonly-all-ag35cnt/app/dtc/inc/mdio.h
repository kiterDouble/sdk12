#ifndef MDIO_H
#define MDIO_H

#include "stdint.h"
#include "stdio.h"

#define MII_ADDR_C45        (0x8000)
#define PHY_ID_YT8010AN		(0x4f51eb02u)
#define PHY_IFNAME          "eth0"

typedef enum {
    CABLE_NORMAL = 0,
    CABLE_OPEN,
    CABLE_SHORT,
    CABLE_UNKNOWN
} cable_status_t;

typedef struct {
    cable_status_t status;
    uint16_t       fault_pos_raw; // 直接输出 length_ca 原始值
} cable_diag_result_t;

typedef struct {
    uint8_t ethIndex;       // Byte 0: 接口编号
    uint8_t linkState;      // Byte 1: 链路状态 0x00=up, 0x01=down
    uint8_t currentSQI;     // Byte 2: 当前SQI等级
    uint8_t worstSQI;       // Byte 3: 最差SQI
} EthStatusInfo_t;

typedef struct {
    uint8_t snrWarn;         // 1=信噪比警告
    uint16_t symErrCnt;      // 符号错误计数
    uint8_t initFail;        // 初始化失败
    uint8_t linkDown2Up;     // 链路恢复
    uint8_t linkUp2Down;     // 链路断开
    uint8_t smiFail;         // SMI 控制失败
    uint8_t sendClamp;       // 发送钳位
    uint8_t ethPowerOff;     // 电源管理关闭以太网
} Yt8010DiagStatus_t;

uint16_t mdio_read(char *devname, uint16_t mmd, uint16_t addr, uint8_t is_c45);

void mdio_write(char *devname, uint16_t mmd, uint16_t addr, uint16_t data, uint8_t is_c45);

cable_diag_result_t yt8010an_get_cable_status(char *ifname);

int GetEthTransceiverStatus(char *ifname, EthStatusInfo_t *out);

int GetEthernetCommFailure(char *ifname);

int GetEthernetLineFailure(char *ifname, uint8_t *detailBits);

int GetYt8010anDiagStatus(char *ifname, Yt8010DiagStatus_t *out);

#endif // MDIO_H