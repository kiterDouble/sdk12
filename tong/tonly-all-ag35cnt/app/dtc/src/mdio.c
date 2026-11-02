#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/mii.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include "sr_log.h"
#include "mdio.h"

#define MII_BMCR        0x00
#define MII_BMSR        0x01
#define MII_PHYID1      0x02
#define MII_PHYID2      0x03

/* YT8010AN 扩展寄存器：通过 0x1E/0x1F 访问 */
#define YT_CSD1         0x9090 /* bit15:csd_ca_en, bit14:open, bit13:short, bit8:0:length */
#define YT_CSD2         0x9091 /* bit10:8 current SQI */
#define YT_CSD5         0x9094 /* bit10:8 worst SQI  */

/* BMCR 位 */
// #define BMCR_RESET       (1u<<15)
// #define BMCR_LOOPBACK    (1u<<14)
#define BMCR_SPEED100_APP    (1u<<13)
#define BMCR_AN_ENABLE   (1u<<12)
// #define BMCR_POWERDOWN   (1u<<11)
// #define BMCR_ISOLATE     (1u<<10)
// #define BMCR_RESTART_AN  (1u<<9)
#define BMCR_DUPLEX      (1u<<8)

#define print_error() \
    LOG_Debug("%s, line: %d\r\n", strerror(errno), __LINE__);

static int sockfd = -1;
static volatile sig_atomic_t g_stop = 0;

// ---- Clause 22 扩展寄存器读写封装（通过 0x1E/0x1F）----
static inline void ext_set_addr(char *dev, uint16_t ext_addr)
{
    // 写扩展地址到 0x1E(Address)
    mdio_write(dev, 0, 0x1E, ext_addr, 0 /*C22*/);
}
static inline uint16_t ext_read_data(char *dev)
{
    // 从 0x1F(Data) 读数据
    return mdio_read(dev, 0, 0x1F, 0 /*C22*/);
}
static inline void ext_write_data(char *dev, uint16_t val)
{
    // 向 0x1F(Data) 写数据
    mdio_write(dev, 0, 0x1F, val, 0 /*C22*/);
}
static inline uint16_t ext_read(char *dev, uint16_t ext_addr)
{
    ext_set_addr(dev, ext_addr);
    return ext_read_data(dev);
}
static inline void ext_write(char *dev, uint16_t ext_addr, uint16_t val)
{
    ext_set_addr(dev, ext_addr);
    ext_write_data(dev, val);
}

/* 读 Basic Mode Status Register (0x01) 判断 link */
static int GetLinkState(char *ifname)
{
    uint16_t val = mdio_read(ifname, 0, 0x01, 0);
    if (val & (1 << 2)) { // bit2 = Link Status
        return 0x00; // Link Up
    } else {
        return 0x01; // Link Down
    }
}

/* 读取扩展寄存器 0x9091 的 Current SQI 值 */
static uint16_t ExtRead(char *ifname, uint16_t extAddr)
{
    // Clause 22扩展访问：写地址0x1E，再读0x1F
    mdio_write(ifname, 0, 0x1E, extAddr, 0);
    return mdio_read(ifname, 0, 0x1F, 0);
}

static uint8_t GetCurrentSQI(char *ifname)
{
    uint16_t val = ExtRead(ifname, 0x9091); // Cable Status Diagnostics Reg2
    uint8_t sqi = (val >> 8) & 0x07;        // bit10:8 表示当前SQI等级(0~7)
    if (sqi > 7) sqi = 0xFF;
    return sqi;
}

/* 最差SQI在 0x9094 bit10:8（CSD Reg5） */
static uint8_t GetWorstSQI(char *ifname)
{
    uint16_t val = ExtRead(ifname, 0x9094);
    uint8_t sqi = (val >> 8) & 0x07;
    if (sqi > 7) sqi = 0xFF;
    return sqi;
}

static int CheckPhyIdentityOk(char *ifname)
{
    uint16_t id1 = mdio_read(ifname, 0, MII_PHYID1, 0);
    uint16_t id2 = mdio_read(ifname, 0, MII_PHYID2, 0);
    if (id1 == 0x0000 || id1 == 0xFFFF) {
        return 0;
    }
    if (id2 == 0x0000 || id2 == 0xFFFF) {
        return 0;
    }
    return 1;
}

static int RunCableDiagHasFail(char *ifname)
{
    /* 触发一次 CSD 检测（bit15 写1，完成后自清） */
    ext_write(ifname, YT_CSD1, (uint16_t)(1u << 15));
    for (int i = 0; i < 40; ++i) { /* 最长~200ms */
        uint16_t v = ext_read(ifname, YT_CSD1);
        if ((v & (1u << 15)) == 0) {
            int openFlag  = (v & (1u << 14)) ? 1 : 0;
            int shortFlag = (v & (1u << 13)) ? 1 : 0;
            return (openFlag || shortFlag) ? 1 : 0;
        }
        usleep(5000);
    }
    /* 超时，按失败处理 */
    return 1;
}

/* ———— 0x08: 以太网通讯错误（PHY回环+原始帧发送接收自检） ———— */
static void PhySetLoopback(char *ifname, int enable)
{
    uint16_t bmcr = mdio_read(ifname, 0, MII_BMCR, 0);
    /* 禁用自协商，强制 100M 全双工，按需开回环 */
    bmcr &= ~(BMCR_AN_ENABLE);
    bmcr |=  (BMCR_SPEED100_APP | BMCR_DUPLEX);
    if (enable) { bmcr |=  BMCR_LOOPBACK; }
    else        { bmcr &= ~BMCR_LOOPBACK; }
    mdio_write(ifname, 0, MII_BMCR, bmcr, 0);
    usleep(50 * 1000);
}

static int MacPhyLoopbackSelftest(char *ifname, int timeout_ms)
{
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0) {
        print_error();
        return -1;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) { print_error(); close(sock); return -2; }
    int ifindex = ifr.ifr_ifindex;

    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) { print_error(); close(sock); return -3; }
    unsigned char mac[6];
    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);

    struct sockaddr_ll saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sll_family  = AF_PACKET;
    saddr.sll_ifindex = ifindex;
    saddr.sll_halen   = ETH_ALEN;
    memcpy(saddr.sll_addr, mac, 6);

    /* 构造测试帧：Dst=Src=本机MAC, Ethertype=0x88B5, payload 46字节（最小帧） */
    unsigned char frame[64] = {0};
    memcpy(frame + 0, mac, 6);
    memcpy(frame + 6, mac, 6);
    frame[12] = 0x88; frame[13] = 0xB5;
    for (int i = 14; i < 60; ++i) {
        frame[i] = (unsigned char)i;
    }
    int send_len = 60; /* 驱动会补CRC */

    PhySetLoopback(ifname, 1);

    if (sendto(sock, frame, send_len, 0, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        print_error();
        PhySetLoopback(ifname, 0);
        close(sock);
        return -4;
    }

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);
    struct timeval tv;
    tv.tv_sec  = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int rv = select(sock + 1, &rfds, NULL, NULL, &tv);
    if (rv > 0 && FD_ISSET(sock, &rfds)) {
        unsigned char buf[1518];
        int n = (int)recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
        if (n >= 14 && buf[12] == 0x88 && buf[13] == 0xB5) {
            PhySetLoopback(ifname, 0);
            close(sock);
            return 0; /* 自检通过 */
        }
    }

    PhySetLoopback(ifname, 0);
    close(sock);
    return -5; /* 超时或未匹配 */
}

uint16_t mdio_read(char *devname, uint16_t mmd, uint16_t addr, uint8_t is_c45)
{
    struct mii_ioctl_data *mii = NULL;
    struct ifreq ifr;
    int ret = 0;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, devname, IFNAMSIZ - 1);

    if (sockfd < 0)
    {
        sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
        if (sockfd < 0)
        {
            print_error();
            return -1;
        }
    }

    // get phy address in smi bus
    ret = ioctl(sockfd, SIOCGMIIPHY, &ifr);
    if (ret < 0)
    {
        print_error();
        return -1;
    }

    mii = (struct mii_ioctl_data *)&ifr.ifr_data;

    if (is_c45)
        mii->phy_id = MII_ADDR_C45 | mii->phy_id << 5 | mmd;
    mii->reg_num = addr;
    ret = ioctl(sockfd, SIOCGMIIREG, &ifr);
    if (ret < 0)
    {
        print_error();
        return -2;
    }

    if (is_c45)
        printf("read  phy addr: 0x%02x | mmd: 0x%02x | reg: 0x%02x | value: 0x%04x\n",
               mii->phy_id, mmd, mii->reg_num, mii->val_out);
    else
        printf("read  phy addr: 0x%02x | reg: 0x%02x | value: 0x%04x\n",
               mii->phy_id, mii->reg_num, mii->val_out);

    return mii->val_out;
}

void mdio_write(char *devname, uint16_t mmd, uint16_t addr, uint16_t data, uint8_t is_c45)
{
    struct mii_ioctl_data *mii = NULL;
    struct ifreq ifr;
    int ret = 0;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, devname, IFNAMSIZ - 1);

    if (sockfd < 0)
    {
        sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
        if (sockfd < 0)
        {
            print_error();
            return;
        }
    }

    // get phy address in smi bus
    ret = ioctl(sockfd, SIOCGMIIPHY, &ifr);
    if (ret < 0)
    {
        print_error();
        return;
    }

    mii = (struct mii_ioctl_data *)&ifr.ifr_data;

    if (is_c45)
        mii->phy_id = MII_ADDR_C45 | mii->phy_id << 5 | mmd;
    mii->reg_num = addr;
    mii->val_in = data;
    ret = ioctl(sockfd, SIOCSMIIREG, &ifr);
    if (ret < 0)
    {
        print_error();
        return;
    }

    if (is_c45)
        printf("write phy addr: 0x%02x | mmd: 0x%02x | reg: 0x%02x | value: 0x%04x\n",
               mii->phy_id, mmd, mii->reg_num, mii->val_in);
    else
        printf("write phy addr: 0x%02x | reg: 0x%02x | value: 0x%04x\n",
               mii->phy_id, mii->reg_num, mii->val_in);

    return;
}

cable_diag_result_t yt8010an_get_cable_status(char *ifname)
{
    cable_diag_result_t r = {CABLE_UNKNOWN, 0};

    // 1) 读 Basic Status (0x01) 判断 link_status
    uint16_t bmsr = mdio_read(ifname, 0, 0x01, 0 /*C22*/);
    int link_up = (bmsr & (1 << 2)) != 0; // bit2 link_status（1=up）
    if (link_up)
    {
        r.status = CABLE_NORMAL;
        r.fault_pos_raw = 0;
        printf("Link is up -> Cable NORMAL (no CSD needed)\n");
        return r;
    }

    // 2) 触发线缆诊断：向 0x9090 的 bit15(csdl_ca_en) 写 1
    //    注意：该位为自清，检测完成后清零
    const uint16_t CSD_REG1 = 0x9090;
    ext_write(ifname, CSD_REG1, (uint16_t)(1u << 15));

    // 3) 轮询等待检测完成（bit15 自清）。最长等 ~200ms
    for (int i = 0; i < 40; ++i)
    {
        uint16_t v = ext_read(ifname, CSD_REG1);
        if ((v & (1u << 15)) == 0)
        {
            // 完成：解析结果
            int open_flag = (v & (1u << 14)) != 0;
            int short_flag = (v & (1u << 13)) != 0;
            uint16_t length_ca = (uint16_t)(v & 0x01FF);

            if (short_flag)
            {
                r.status = CABLE_SHORT;
                r.fault_pos_raw = length_ca;
                printf("CSD: SHORT detected, position(raw)=0x%03x\n", length_ca);
            }
            else if (open_flag)
            {
                r.status = CABLE_OPEN;
                r.fault_pos_raw = length_ca;
                printf("CSD: OPEN detected, position(raw)=0x%03x\n", length_ca);
            }
            else
            {
                // 未检测到开/短，可能是远端未接/其他异常
                r.status = CABLE_UNKNOWN;
                r.fault_pos_raw = length_ca;
                printf("CSD: No OPEN/SHORT flag, length(raw)=0x%03x -> UNKNOWN\n", length_ca);
            }
            return r;
        }
        usleep(5000); // 5ms 间隔
    }

    // 超时
    printf("CSD: timeout waiting for csd_ca_en to self-clear\n");
    r.status = CABLE_UNKNOWN;
    r.fault_pos_raw = 0;
    return r;
}

int GetEthTransceiverStatus(char *ifname, EthStatusInfo_t *out)
{
    if (ifname == NULL || out == NULL)
        return -1;

    memset(out, 0, sizeof(*out));

    /* Byte 0: 网卡索引 */
    unsigned int idx = if_nametoindex(ifname);
    out->ethIndex = (idx <= 255) ? idx : 255;

    /* Byte 1: Link状态 */
    out->linkState = GetLinkState(ifname);

    /* Byte 2: 当前信号质量值 */
    out->currentSQI = GetCurrentSQI(ifname);

    /* Byte 3: 最差信号质量值 */
    out->worstSQI = GetWorstSQI(ifname);

    return 0;
}

int GetEthernetCommFailure(char *ifname)
{
    /* 仅做回环自检；若失败则返回0x08 */
    if (MacPhyLoopbackSelftest(ifname, 500) != 0) {
        return 0x08;
    }
    return 0;
}

/* ———— 0x79: 以太网线路故障 ———— */
int GetEthernetLineFailure(char *ifname, uint8_t *detailBits)
{
    uint8_t bits = 0;

    /* 子原因1：系统初始化失败（接口不存在/PHY ID 异常） */
    if (if_nametoindex(ifname) == 0 || !CheckPhyIdentityOk(ifname)) {
        bits |= (1u << 0);
    }

    /* 子原因3：SMI访问失败（读BMSR返回0或0xFFFF 视为异常） */
    uint16_t bmsr = mdio_read(ifname, 0, MII_BMSR, 0);
    if (bmsr == 0x0000 || bmsr == 0xFFFF) {
        bits |= (1u << 2);
    }

    /* 子原因2：线路失败（仅在链路Down时执行 CSD 更有意义；这里不强制） */
    if (RunCableDiagHasFail(ifname)) {
        bits |= (1u << 1);
    }

    if (detailBits != NULL) {
        *detailBits = bits;
    }
    return (bits != 0) ? 0x79 : 0;
}

int GetYt8010anDiagStatus(char *ifname, Yt8010DiagStatus_t *out)
{
    if (!ifname || !out) return -1;
    memset(out, 0, sizeof(*out));

    /* 1. SNR warning (0x9091 bit15) */
    uint16_t v9091 = ExtRead(ifname, 0x9091);
    out->snrWarn = (v9091 & (1 << 15)) ? 1 : 0;

    /* 2. SYM_ERR_CNT (0x9051) */
    uint16_t v9051 = ExtRead(ifname, 0x9051);
    out->symErrCnt = v9051;

    /* 3. 初始化失败 (0x9000 bit1) */
    uint16_t v9000 = ExtRead(ifname, 0x9000);
    out->initFail = (v9000 & (1 << 1)) ? 1 : 0;

    /* 4~5. Link 变化 (0x01 bit2) */
    static int prevLink = -1;
    uint16_t bmsr = mdio_read(ifname, 0, 0x01, 0);
    int link = (bmsr & (1 << 2)) ? 1 : 0;
    if (prevLink == 1 && link == 0) out->linkUp2Down = 1;
    if (prevLink == 0 && link == 1) out->linkDown2Up = 1;
    prevLink = link;

    /* 6. SMI 控制失败 (0x9092 bit14) */
    uint16_t v9092 = ExtRead(ifname, 0x9092);
    out->smiFail = (v9092 & (1 << 14)) ? 1 : 0;

    /* 7. Sending Enable be clamp (0x9095 bit10) */
    uint16_t v9095 = ExtRead(ifname, 0x9095);
    out->sendClamp = (v9095 & (1 << 10)) ? 1 : 0;

    /* 8. Power Management Strategy (0x9075 bit5) */
    uint16_t v9075 = ExtRead(ifname, 0x9075);
    out->ethPowerOff = (v9075 & (1 << 5)) ? 1 : 0;

    return 0;
}
