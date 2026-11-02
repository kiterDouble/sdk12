#include "mqtt_qos.h"
#include "mqtt_connect.h"

#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include <regex.h>

#include "ql_type.h"
#include "ql_data_throughput.h"
#include "ql_data_throughput_common.h"

#include "err_code.h"
#include "sr_log.h"
#include "mqtt_get_modem_info.h"

// ====================== 1. 核心配置（对齐原Shell脚本）======================
#define ETH_NAME            "rmnet_data0"
#define PING_TARGET         "mqtt.zelostech.com.cn"
#define QOS_STATE_FILE      "/oemdata/qos/qos_enable.state"

#define MAX_PATH_LEN            128
#define TX_BYTES_PATH           "/sys/class/net/%s/statistics/tx_bytes"
#define SAMPLING_INTERVAL       1  // 采样间隔（秒），建议1-5秒

// 带宽调整常量定义
#define BW_512K                5120    // 512Kbit
#define BW_1M                  10240   // 1Mbit

// 带宽基础配置
// #define TOTAL_BANDWIDTH 716800   // 总带宽70M
#define TOTAL_BANDWIDTH 1029120   // 总带宽70M
// #define TC_100_INIT 512         // 1:100初始带宽（512Kbit不可削减）
// #define TC_200_INIT 512         // 1:200初始带宽（512Kbit）
#define TC_100_INIT 2560         // 1:100初始带宽（512Kbit不可削减）
#define TC_200_INIT 2560         // 1:200初始带宽（512Kbit）
#define TC_300_INIT 307200       // 1:300初始带宽（30Mbit）
#define TC_400_INIT 204800       // 1:400初始带宽（20Mbit）
#define TC_500_INIT 5120         // 1:500初始带宽（512Kbit）
#define TC_600_INIT (TOTAL_BANDWIDTH - TC_100_INIT - TC_200_INIT - \
                     TC_300_INIT - TC_400_INIT - TC_500_INIT) // 剩余保底带宽


// Ping检测配置
#define PING_INTERVAL 0.1f      // Ping周期100ms（浮点型）
#define PING_TIMEOUT 0.5f       // Ping超时500ms
#define DELAY_THRESHOLD 100     // 延迟阈值100ms
#define STABLE_RECOVERY_COUNT 3 // 连续稳定恢复次数
#define RECOVER_AMOUNT 10       // 每次恢复带宽10M

// 吞吐量上报间隔
#define THROUGHPUT_INTERVAL     100     // 100ms
// 延迟上报间隔
#define LATENCY_INTERVAL        100     // 100ms

// 吞吐量阈值（512K）
#define THR_MIN_THRESHOLD    512    // 512Kbps阈值


// ====================== 吞吐量阈值配置 ======================
#define THROUGHPUT_LOW        10240    // 低吞吐量阈值（10Mbps，kbps）
#define THROUGHPUT_MID        30720    // 中吞吐量阈值（30Mbps，kbps）
#define THROUGHPUT_HIGH       51200    // 高吞吐量阈值（50Mbps，kbps）
#define THROUGHPUT_OVERLOAD   61440    // 过载阈值（60Mbps，kbps）

// 吞吐量稳定计数阈值（连续N次满足条件才触发调整，防抖动）
#define THROUGHPUT_STABLE_COUNT     5       // 连续5次检测达标触发调整
#define BW_ADJUST_STEP              5120       // 每次带宽调整步长（5M）
#define BW_RECOVER_STEP             8192       // 带宽恢复步长（8M，快恢复）

// 吞吐量接近总带宽的判定阈值（百分比）
#define THR_CLOSE_TO_BW_THRESH      0.9f       // 吞吐量达到总带宽90%判定为"接近"
// 队列扩容步长（每个队列每次扩容512K）
#define QUEUE_EXPAND_STEP           512        // 512Kbit/次
// 各队列带宽上限（初始值即为上限）
#define TC_100_MAX      5120
#define TC_200_MAX      5120
// Keep enough root bandwidth for both high-priority classes at their ceilings.
#define TC_MIN_BW       (TC_100_MAX + TC_200_MAX)
#define TC_300_MAX      TC_300_INIT
#define TC_400_MAX      TC_400_INIT
#define TC_500_MAX      TC_500_INIT
#define TC_600_MAX      TC_600_INIT

// 吞吐量调整策略（[吞吐量下限, 吞吐量上限, 目标总带宽占比]）
// 目标总带宽占比：低优先级队列总带宽 / 全局总带宽（TOTAL_BANDWIDTH）
typedef struct {
    uint32_t thr_lower;    // 吞吐量下限(kbps)
    uint32_t thr_upper;    // 吞吐量上限(kbps)
    float    bw_ratio;     // 低优先级带宽占比（0~1）
} ThrAdjustStrategy;

// 策略表：吞吐量越高，低优先级带宽占比越低
ThrAdjustStrategy thr_strategies[] = {
    {0,                     THROUGHPUT_LOW,         0.8},       // 低吞吐量：低优占80%总带宽
    {THROUGHPUT_LOW,        THROUGHPUT_MID,         0.6},       // 中低吞吐量：低优占60%
    {THROUGHPUT_MID,        THROUGHPUT_HIGH,        0.4},       // 中高吞吐量：低优占40%
    {THROUGHPUT_HIGH,       THROUGHPUT_OVERLOAD,    0.2},       // 高吞吐量：低优占20%
    {THROUGHPUT_OVERLOAD,   UINT32_MAX,             0.1}        // 过载：低优仅占10%
};
#define THR_STRATEGY_COUNT (sizeof(thr_strategies) / sizeof(ThrAdjustStrategy))







// ====================== 丢包率阈值配置 ======================
#define LOSS_RATE_HIGH_PRIO_THRESH  1.0f   // 高优队列丢包率阈值(%)：超过则紧急压缩低优
#define LOSS_RATE_LOW_PRIO_THRESH   5.0f   // 低优队列丢包率阈值(%)：超过则适度恢复
#define LOSS_RATE_SAMPLE_CNT        10     // 丢包率采样次数（防抖动）
#define LOSS_ADJUST_STEP            10240     // 丢包驱动的带宽调整步长(10M)
// 队列丢包判定阈值（超过该值则扩容）
#define QUEUE_LOSS_TRIGGER_THRESH   0.5f    // 队列丢包率超过0.5%则触发扩容

// 队列丢包率统计结构体
typedef struct {
    float loss_rate;          // 当前丢包率(%)
    int sample_count;         // 采样计数
    float loss_sum;           // 丢包率累加和（用于求平均）
} QueueLossStat_t;

// 各队列丢包率统计（1:100/1:200/1:300/1:400/1:500/1:600）
QueueLossStat_t g_queue_loss[6] = {0};
#define QUEUE_100_IDX 0
#define QUEUE_200_IDX 1
#define QUEUE_300_IDX 2
#define QUEUE_400_IDX 3
#define QUEUE_500_IDX 4
#define QUEUE_600_IDX 5
#define QUEUE_COUNT (sizeof(g_queue_loss) / sizeof(g_queue_loss[0]))







// 弱网带宽调整策略（[下限, 上限, 减幅]）
typedef struct {
    int lower;
    int upper;
    int reduce;
} AdjustStrategy;

AdjustStrategy adjust_strategies[] = {
    {100, 150, 10},
    {150, 200, 10},
    {200, 300, 30},
    {300, 400, 30},
    {400, 500, 30},
    {500, 1000, 50}
};

#define STRATEGY_COUNT (sizeof(adjust_strategies) / sizeof(AdjustStrategy))

// ====================== 2. 全局共享数据（线程安全）======================
// 当前带宽变量
typedef struct {
    uint32_t bw_100;        // 1:100带宽（kbit）
    uint32_t bw_200;        // 1:200带宽（kbit）
    uint32_t bw_300;        // 1:300带宽（kbit）
    uint32_t bw_400;        // 1:400带宽（kbit）
    uint32_t bw_500;        // 1:500带宽（kbit）
    uint32_t bw_600;        // 1:600带宽（kbit）
    uint32_t total;
} BandwidthInfo;

// 线程共享数据
typedef struct {
    int ping_delay;                 // 最新Ping延迟（ms）
    int stable_counter;             // 连续稳定计数器
    int is_restricted;              // 是否处于带宽限制状态
    BandwidthInfo bw_info;          // 当前带宽信息
    pthread_mutex_t mutex;          // 互斥锁
    int run_flag;                   // 线程运行标志（0=退出）
    uint32_t up_rate;               // 上行吞吐量（kbps）
    uint32_t up_latency;            // 上行延迟（ms）
    uint32_t tx_rate;               // 网卡发送速率（kbps）
    uint32_t stable_latency_counter;// 延迟连续稳定计数器
    uint32_t stable_thr_counter;    // 吞吐量连续稳定计数器
    uint32_t last_thr;              // 上一次吞吐量值
    int loss_driven_adjust;         // 丢包驱动的调整标记
} QoSCtx_t;

QoSCtx_t g_qosCtx = {
    .ping_delay = 0,
    .stable_counter = 0,
    .is_restricted = 0,
    .bw_info = {TC_100_INIT, TC_200_INIT, TC_300_INIT, TC_400_INIT, TC_500_INIT, TC_600_INIT, TOTAL_BANDWIDTH},
    .run_flag = 1,
    .up_rate = TC_100_INIT + TC_200_INIT + TC_300_INIT + TC_400_INIT + TC_500_INIT + TC_600_INIT,
    .up_latency = 0,
    .tx_rate = 0,
    .stable_latency_counter = 0,
    .stable_thr_counter = 0,
    .last_thr = 0,
    .loss_driven_adjust = 0
};


static float get_queue_loss_rate(int classid);
static void update_all_queue_loss(void);

/**
 * @brief 读取tx_bytes累计值（64位无符号整数）
 * @param tx_bytes 输出参数，存储读取到的累计发送字节数
 * @return 0成功，-1失败
 */
int read_tx_bytes(uint64_t *tx_bytes)
{
    char path[MAX_PATH_LEN] = {0};
    snprintf(path, sizeof(path), TX_BYTES_PATH, ETH_NAME);

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror("fopen tx_bytes file failed");
        return -1;
    }

    // 读取64位无符号整数（tx_bytes是累计值，可能很大）
    if (fscanf(fp, "%lu", tx_bytes) != 1) {
        perror("fscanf tx_bytes failed");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

/**
 * @brief 计算实时上行带宽（返回Kbit/s，用于TC配置）
 * @return 带宽值（Kbit/s），失败返回-1.0
 */
double calculate_real_time_bandwidth(void)
{
    uint64_t tx1, tx2;
    time_t t1, t2;
    double delta_t, delta_tx, bandwidth_kbit;

    // 第一次读取tx_bytes和时间
    if (read_tx_bytes(&tx1) != 0) {
        return -1.0;
    }
    t1 = time(NULL);

    // 等待采样间隔
    sleep(SAMPLING_INTERVAL);

    // 第二次读取tx_bytes和时间
    if (read_tx_bytes(&tx2) != 0) {
        return -1.0;
    }
    t2 = time(NULL);

    // 计算实际时间间隔（避免sleep误差）
    delta_t = difftime(t2, t1);
    if (delta_t <= 0) {
        delta_t = 1.0; // 防止除以0
    }

    // 处理tx_bytes回绕（64位无符号整数最大值）
    const uint64_t UINT64_MAX_VAL = 0xFFFFFFFFFFFFFFFF;
    if (tx2 >= tx1) {
        delta_tx = (double)(tx2 - tx1);
    } else {
        // 回绕时的差值计算：(最大值 - tx1) + tx2 + 1
        delta_tx = (double)(UINT64_MAX_VAL - tx1 + tx2 + 1);
    }

    // 计算带宽：字节数 -> 比特数 -> bit/s -> Kbit/s
    double bit_per_sec = (delta_tx * 8) / delta_t;
    bandwidth_kbit = bit_per_sec / 1000.0; // 转换为Kbit/s

    return bandwidth_kbit;
}

/**
 * @brief 转换带宽单位（bit/s -> Kbit/s/Mbit/s）
 * @param bit_per_sec 原始带宽（bit/s）
 * @param out_str 输出缓冲，存储带单位的带宽字符串
 * @param out_len 输出缓冲长度
 */
void convert_bandwidth(double bit_per_sec, char *out_str, int out_len)
{
    double kbit_per_sec = bit_per_sec / 1000.0;
    double mbit_per_sec = kbit_per_sec / 1000.0;

    if (mbit_per_sec >= 1.0) {
        snprintf(out_str, out_len, "%.2f Mbit/s", mbit_per_sec);
    } else if (kbit_per_sec >= 1.0) {
        snprintf(out_str, out_len, "%.2f Kbit/s", kbit_per_sec);
    } else {
        snprintf(out_str, out_len, "%.0f bit/s", bit_per_sec);
    }
}

/**
 * @brief 校验吞吐量值是否合法（过滤突发/异常值）
 * 
 * @param current 
 * @param last 
 * @return int 
 */
static int is_thr_valid(uint32_t current, uint32_t last)
{
    if (current == 0 || last == 0)
        return 1; // 首次检测直接合法

    // 波动超过±20%判定为异常
    uint32_t diff = abs((int)current - (int)last);

    return (diff * 100 / last) <= 20;
}

/**
 * @brief 根据吞吐量获取低优先级带宽目标占比
 * 
 * @param thr 
 * @return float 
 */
static float get_thr_bw_ratio(uint32_t thr)
{
    for (int i = 0; i < THR_STRATEGY_COUNT; i++) {
        ThrAdjustStrategy *s = &thr_strategies[i];
        if (thr >= s->thr_lower && thr < s->thr_upper) {
            return s->bw_ratio;
        }
    }
    return 0.1; // 默认过载状态，占比10%
}



/**
 * @brief 检查网络接口是否存在
 */
static int check_interface(void)
{
    struct ifreq ifr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        LOG_Error("❌ 创建socket失败: %s", strerror(errno));
        return 0;
    }
    strncpy(ifr.ifr_name, ETH_NAME, IFNAMSIZ);
    int ret = ioctl(sock, SIOCGIFFLAGS, &ifr) >= 0 ? 1 : 0;
    close(sock);
    if (!ret) {
        LOG_Error("❌ 网络接口%s不存在, 等待恢复...", ETH_NAME);
    }
    return ret;
}

/**
 * @brief 检查网络连通性
 */
static int check_network(void)
{
    if (!check_interface())
        return 0;

    // 执行ping检测（1个包，间隔100ms，超时500s）
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "ping -c 1 -i %.1f -W %.1f %s >/dev/null 2>&1", PING_INTERVAL, PING_TIMEOUT, PING_TARGET);
    int ret = system(cmd) == 0 ? 1 : 0;
    if (ret) {
        LOG_Debug("网络OK(接口：%s, 检测IP: %s)", ETH_NAME, PING_TARGET);
    } else {
        LOG_Error("❌ 网络不可达（接口：%s, 检测IP: %s), 等待恢复...", ETH_NAME, PING_TARGET);
    }
    return ret;
}

/**
 * @brief 获取Ping延迟（ms，整数）
 */
static int get_ping_delay(void)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd),
             "ping -c 1 -i %.1f -W %.1f %s 2>&1 | awk -F'time=| ms' '/time=/{print $2}'",
             PING_INTERVAL, PING_TIMEOUT, PING_TARGET);

    FILE *fp = popen(cmd, "r");
    if (!fp)
        return 9999;

    char buf[32] = {0};
    fgets(buf, sizeof(buf), fp);
    pclose(fp);

    // 处理空值/超时
    if (strlen(buf) == 0 || strcmp(buf, " ") == 0) {
        return 9999;
    }
    // 转整数（截断小数）
    return atoi(buf);
}

/**
 * @brief 根据延迟获取减幅
 */
int get_reduce_amount(int delay)
{
    for (int i = 0; i < STRATEGY_COUNT; i++) {
        AdjustStrategy *s = &adjust_strategies[i];
        if (delay >= s->lower && delay < s->upper) {
            return s->reduce;
        }
    }
    return 50; // 超过1000ms减50M
}

/**
 * @brief 计算总带宽
 */
static int calc_total_bw(BandwidthInfo *bw)
{
    return bw->bw_100 + bw->bw_200 + bw->bw_300 + bw->bw_400 + bw->bw_500 + bw->bw_600;
}

/**
 * @brief 更新TC类带宽
 */
static void update_tc_class(int classid, int rate, int ceil, int prio)
{
    char cmd[256];

    if (ceil <= 0 || ceil < rate) {
        ceil = rate > 0 ? rate : 1;
        LOG_Warn("1:%d ceil is invalid, clamp to %dK", classid, ceil);
    }

    // if (rate > ceil) {
    //     rate = ceil;
    // }

    if (rate > 0) {
        if (classid == 1) {
            snprintf(cmd, sizeof(cmd),
                 "tc class change dev %s parent 1:1 classid 1:%d htb rate %dkbit ceil %dkbit prio %d",
                 ETH_NAME, classid, rate, ceil, prio);
        } else if (classid == 100 || classid == 200 ||
                   classid == 300 || classid == 400 || classid == 500 || classid == 600) {
            snprintf(cmd, sizeof(cmd),
                 "tc class change dev %s parent 1:1 classid 1:%d htb rate %dkbit ceil %dkbit prio %d",
                 ETH_NAME, classid, rate, ceil, prio);
        }
        LOG_Debug("✅ 更新1:%d带宽: %dK", classid, rate);
    } else {
        if (classid == 1 || classid == 100 || classid == 200 ||
            classid == 300 || classid == 400 || classid == 500 || classid == 600) {
            snprintf(cmd, sizeof(cmd),
                    "tc class change dev %s parent 1:1 classid 1:%d htb rate 1kbit ceil %dkbit prio %d",
                    ETH_NAME, classid, ceil, prio);
            LOG_Debug("✅ 更新1:%d带宽: 无保底(rate 1K, 可借用至 ceil %dK)", classid, ceil);
        } else {
            snprintf(cmd, sizeof(cmd),
                    "tc class change dev %s parent 1:1 classid 1:%d htb rate 2048kbit ceil 5120kbit prio %d",
                    ETH_NAME, classid, prio);
            LOG_Debug("✅ 更新1:%d带宽: 最低保障(rate 1bps/ceil 256kbit)", classid);
        }
    }
    SrGetSystemRetValue(cmd);
    LOG_Debug("执行命令: %s", cmd);
}

/**
 * @brief 初始化HTB规则
 */
static int init_htb(void)
{
    int i = 0;
    const char *high_prio_ips[] = {"192.168.1.0/24", "192.168.201.0/24", NULL};
    const char *hex_ips[] = {"0xc0a80100", "0xc0a8c900", NULL};

    LOG_Debug("开始初始化TBOX QoS规则(总带宽：%dM)...", TOTAL_BANDWIDTH);

    // 清理旧规则
    if (check_interface()) {
        char cmd[128];
        snprintf(cmd, sizeof(cmd), "tc qdisc del dev %s root >/dev/null 2>&1", ETH_NAME);
        SrGetSystemRetValue(cmd);
        LOG_Debug("执行命令: %s", cmd);
        LOG_Debug("清理旧规则成功（接口：%s)", ETH_NAME);
    } else {
        LOG_Debug("⚠️ 接口不存在，跳过旧规则清理");
        return -1;
    }

    // 创建HTB根队列
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "tc qdisc add dev %s root handle 1: htb default 600", ETH_NAME);
    LOG_Debug("执行命令: %s", cmd);
    if (SrGetSystemRetValue(cmd) != 0)
        return -1;

    snprintf(cmd, sizeof(cmd),
             "tc class add dev %s parent 1: classid 1:1 htb rate %dkbit ceil %dkbit prio 0",
             ETH_NAME, TOTAL_BANDWIDTH, TOTAL_BANDWIDTH);
    LOG_Debug("执行命令: %s", cmd);
    if (SrGetSystemRetValue(cmd) != 0)
        return -1;

    // 创建TC类（1:100 ~ 1:500）
    snprintf(cmd, sizeof(cmd),
             "tc class add dev %s parent 1:1 classid 1:100 htb rate %dkbit ceil %dkbit prio 1",
             ETH_NAME, TC_100_INIT, TOTAL_BANDWIDTH);
    LOG_Debug("执行命令: %s", cmd);
    if (SrGetSystemRetValue(cmd) != 0)
        return -1;
    LOG_Debug("创建1:100 队列：%dM(prio:1)", TC_100_INIT);

    snprintf(cmd, sizeof(cmd),
             "tc class add dev %s parent 1:1 classid 1:200 htb rate %dkbit ceil %dkbit prio 2",
             ETH_NAME, TC_200_INIT, TOTAL_BANDWIDTH);
    LOG_Debug("执行命令: %s", cmd);
    if (SrGetSystemRetValue(cmd) != 0)
        return -1;
    LOG_Debug("创建1:200 队列：%dM(prio:2)", TC_200_INIT);
    
    snprintf(cmd, sizeof(cmd),
             "tc class add dev %s parent 1:1 classid 1:300 htb rate %dkbit ceil %dkbit prio 3",
             ETH_NAME, TC_300_INIT, TC_300_MAX);
    LOG_Debug("执行命令: %s", cmd);
    if (SrGetSystemRetValue(cmd) != 0)
        return -1;
    LOG_Debug("创建1:300 队列：%dM(prio:3)", TC_300_INIT);
    
    snprintf(cmd, sizeof(cmd),
             "tc class add dev %s parent 1:1 classid 1:400 htb rate %dkbit ceil %dkbit prio 4",
             ETH_NAME, TC_400_INIT, TC_400_MAX);
    LOG_Debug("执行命令: %s", cmd);
    if (SrGetSystemRetValue(cmd) != 0)
        return -1;
    LOG_Debug("创建1:400 队列：%dM(prio:4)", TC_400_INIT);

    snprintf(cmd, sizeof(cmd),
             "tc class add dev %s parent 1:1 classid 1:500 htb rate %dkbit ceil %dkbit prio 5",
             ETH_NAME, TC_500_INIT, TC_500_MAX);
    LOG_Debug("执行命令: %s", cmd);
    if (SrGetSystemRetValue(cmd) != 0)
        return -1;
    LOG_Debug("创建1:500 队列：%dM(prio:5)", TC_500_INIT);

    snprintf(cmd, sizeof(cmd),
             "tc class add dev %s parent 1:1 classid 1:600 htb rate %dkbit ceil %dkbit prio 6",
             ETH_NAME, TC_600_INIT, TC_600_MAX);
    LOG_Debug("执行命令: %s", cmd);
    if (SrGetSystemRetValue(cmd) != 0)
        return -1;
    LOG_Debug("创建1:600 队列：%dM(prio:6)", TC_600_INIT);


    // ========== 流量过滤规则（对齐原Shell脚本）==========
    // 1. 内网IP → 1:100（示例，可扩展）
    for (i = 0; high_prio_ips[i]; i++) {
        snprintf(cmd, sizeof(cmd),
                 "tc filter add dev %s protocol ip parent 1: u32 match u32 %s 0xffffff00 at 16 flowid 1:100",
                 ETH_NAME, hex_ips[i]);
        SrGetSystemRetValue(cmd);
        LOG_Debug("添加规则: 内网IP %s → 1:100", high_prio_ips[i]);
    }

    // ARP → 1:100（极简匹配）
    snprintf(cmd, sizeof(cmd),
                 "tc filter add dev %s protocol arp parent 1: u32 match u8 0 0 flowid 1:100",
                 ETH_NAME);
    SrGetSystemRetValue(cmd);
    LOG_Debug("添加规则: ARP → 1:100");

    // 2. ICMP → 1:100
    snprintf(cmd, sizeof(cmd),
             "tc filter add dev %s protocol ip parent 1: u32 match u8 1 0xff at 9 flowid 1:100",
             ETH_NAME);
    SrGetSystemRetValue(cmd);
    LOG_Debug("添加规则: ICMP协议 → 1:100");

    // 3. 高优TCP端口 → 1:100（示例端口）
    int high_tcp_ports[] = {23456, 7999, 25555, -1};
    for (int i = 0; high_tcp_ports[i] != -1; i++) {
        char hex_port[16];
        snprintf(hex_port, sizeof(hex_port), "0x0000%04x", high_tcp_ports[i]);
        snprintf(cmd, sizeof(cmd),
                 "tc filter add dev %s protocol ip parent 1: u32 match ip protocol 6 0xff match u32 %s 0x0000ffff at 20 flowid 1:100",
                 ETH_NAME, hex_port);
        SrGetSystemRetValue(cmd);
        LOG_Debug("添加规则: TCP %d → 1:100", high_tcp_ports[i]);
    }

    // 4. 高优UDP端口 → 1:100
    int high_udp_ports[] = {53, 123, -1};
    for (int i = 0; high_udp_ports[i] != -1; i++) {
        char hex_port[16];
        snprintf(hex_port, sizeof(hex_port), "0x0000%04x", high_udp_ports[i]);
        snprintf(cmd, sizeof(cmd),
                 "tc filter add dev %s protocol ip parent 1: u32 match ip protocol 17 0xff match u32 %s 0x0000ffff at 20 flowid 1:100",
                 ETH_NAME, hex_port);
        SrGetSystemRetValue(cmd);
        printf("#### udp cmd: %s", cmd);
        LOG_Debug("添加规则: UDP %d → 1:100", high_udp_ports[i]);
    }

    // 5. 中优TCP端口 → 1:200
    int mid_tcp_ports[] = {1993, 6993, 8993, -1};     // 目的端口
    for (int i = 0; mid_tcp_ports[i] != -1; i++) {
        char hex_port[16];
        snprintf(hex_port, sizeof(hex_port), "0x0000%04x", mid_tcp_ports[i]);
        snprintf(cmd, sizeof(cmd),
                 "tc filter add dev %s protocol ip parent 1: u32 match ip protocol 6 0xff match u32 %s 0x0000ffff at 20 flowid 1:200",
                 ETH_NAME, hex_port);
        SrGetSystemRetValue(cmd);
        printf("#### tcp cmd: %s\n", cmd);
        LOG_Debug("添加规则: TCP %d → 1:200", mid_tcp_ports[i]);
    }

    // 6. 中优UDP端口 → 1:200
    int mid_udp_ports[] = {9090, -1};
    for (int i = 0; mid_udp_ports[i] != -1; i++) {
        char hex_port[16];
        snprintf(hex_port, sizeof(hex_port), "0x0000%04x", mid_udp_ports[i]);
        snprintf(cmd, sizeof(cmd),
                 "tc filter add dev %s protocol ip parent 1: u32 match ip protocol 17 0xff match u32 %s 0x0000ffff at 20 flowid 1:200",
                 ETH_NAME, hex_port);
        SrGetSystemRetValue(cmd);
        LOG_Debug("添加规则: UDP %d → 1:200", mid_udp_ports[i]);
    }

    // 7. 低优TCP端口 → 1:300
    int low_tcp_ports[] = {20000, 20001, 1770, 1780, 1790, 1800, 1810, 1820, -1};
    for (int i = 0; low_tcp_ports[i] != -1; i++) {
        char hex_port[16];
        snprintf(hex_port, sizeof(hex_port), "0x0000%04x", low_tcp_ports[i]);
        snprintf(cmd, sizeof(cmd),
                 "tc filter add dev %s protocol ip parent 1: u32 match ip protocol 6 0xff match u32 %s 0x0000ffff at 20 flowid 1:300",
                 ETH_NAME, hex_port);
        SrGetSystemRetValue(cmd);
        LOG_Debug("添加规则: TCP %d → 1:300", low_tcp_ports[i]);
    }

    // 8. 低优UDP端口 DSCP≠0 + 指定UDP端口 → 1:300（先匹配DSCP，再匹配端口）
    int dscp_none_zero_ports[] = {8000, 8001, 8002, 8003, 8004, 8005, 8200, 8201, 8202, 8203, 8204, 8205, -1};
    for (int i = 0; dscp_none_zero_ports[i] != -1; i++) {
        char hex_port[16];
        snprintf(hex_port, sizeof(hex_port), "0x0000%04x", dscp_none_zero_ports[i]);
        snprintf(cmd, sizeof(cmd),
                 "tc filter add dev %s protocol ip parent 1: u32 match ip protocol 17 0xff match u32 0x00040000 0x00fc0000 at 0 match u32 %s 0x0000ffff at 20 flowid 1:300",
                 ETH_NAME, hex_port);
        SrGetSystemRetValue(cmd);
        LOG_Debug("添加规则: UDP(DSCP!=0) %d → 1:300", dscp_none_zero_ports[i]);
    }

    // 9. DSCP=0 + 指定UDP端口 → 1:400
    int dscp_zero_ports[] = {8000, 8001, 8002, 8003, 8004, 8005, 8200, 8201, 8202, 8203, 8204, 8205, -1};
    for (int i = 0; dscp_zero_ports[i] != -1; i++) {
        char hex_port[16];
        snprintf(hex_port, sizeof(hex_port), "0x0000%04x", dscp_zero_ports[i]);
        snprintf(cmd, sizeof(cmd),
                 "tc filter add dev %s protocol ip parent 1: u32 match ip protocol 17 0xff match u32 0x00000000 0x00fc0000 at 0 match u32 %s 0x0000ffff at 20 flowid 1:400",
                 ETH_NAME, hex_port);
        SrGetSystemRetValue(cmd);
        LOG_Debug("添加规则: UDP(DSCP==0) %d → 1:400", dscp_zero_ports[i]);
    }

    // 10. TCP 9788 → 1:500（0x263c）
    snprintf(cmd, sizeof(cmd),
                 "tc filter add dev %s protocol ip parent 1: u32 match ip protocol 6 0xff match u32 0x0000263c 0x0000ffff at 20 flowid 1:500",
                 ETH_NAME);
    SrGetSystemRetValue(cmd);
    LOG_Debug("添加规则: TCP 9788 → 1:500");

    // Ordinary TCP/UDP traffic is deliberately handled by "htb default 600".
    // Do not add protocol catch-all filters: a low pref value could override
    // the specific port/IP/DSCP rules above.

    LOG_Debug("✅ TBOX QoS规则初始化完成: ");
    LOG_Debug("  总带宽：%dK", TOTAL_BANDWIDTH);
    LOG_Debug("  1:100(优先级1): %dK", TC_100_INIT);
    LOG_Debug("  1:200(优先级2): %dK", TC_200_INIT);
    LOG_Debug("  1:300(优先级3): %dK", TC_300_INIT);
    LOG_Debug("  1:400(优先级4): %dK", TC_400_INIT);
    LOG_Debug("  1:500(优先级5): %dK", TC_500_INIT);
    LOG_Debug("  1:600(优先级6): %dK", TC_600_INIT);

    return 0;
}

/**
 * @brief 调整低优先级带宽
 */
void adjust_low_prio_bw(int target_total_bw)
{
    // pthread_mutex_lock(&g_qosCtx.mutex);
    BandwidthInfo *bw = &g_qosCtx.bw_info;

    // 边界检查：目标带宽不能小于1:100，不能超过总带宽
    if (target_total_bw < TC_MIN_BW) {
        target_total_bw = TC_MIN_BW;
        LOG_Debug("⚠️ 目标带宽已达最小限制(1:100需求带宽:%dK)", TC_MIN_BW);
    }
    if (target_total_bw > TOTAL_BANDWIDTH) {
        target_total_bw = TOTAL_BANDWIDTH;
    }

    // 计算带宽差
    int current_total = calc_total_bw(bw);
    int bw_diff = target_total_bw - current_total;
    if (bw_diff == 0) {
        // pthread_mutex_unlock(&g_qosCtx.mutex);
        return;
    }

    LOG_Debug("开始调整带宽...");
    LOG_Debug("调整前 - 1:100=%dK | 1:200=%dK | 1:300=%dK | 1:400=%dK | 1:500=%dK | 1:600=%dK",
            bw->bw_100, bw->bw_200, bw->bw_300, bw->bw_400, bw->bw_500, bw->bw_600);

    // 削减带宽：6→5→4→3
    if (bw_diff < 0) {
        int reduce_needed = -bw_diff;
        // 减6级
        if (reduce_needed > 0 && bw->bw_600 > 0) {
            int reduce = bw->bw_600 < reduce_needed ? bw->bw_600 : reduce_needed;
            bw->bw_600 -= reduce;
            reduce_needed -= reduce;
        }
        // 减5级
        if (reduce_needed > 0 && bw->bw_500 > 0) {
            int reduce = bw->bw_500 < reduce_needed ? bw->bw_500 : reduce_needed;
            bw->bw_500 -= reduce;
            reduce_needed -= reduce;
        }
        // 减4级
        if (reduce_needed > 0 && bw->bw_400 > 0) {
            int reduce = bw->bw_400 < reduce_needed ? bw->bw_400 : reduce_needed;
            bw->bw_400 -= reduce;
            reduce_needed -= reduce;
        }
        // 减3级
        if (reduce_needed > 0 && bw->bw_300 > 0) {
            int reduce = bw->bw_300 < reduce_needed ? bw->bw_300 : reduce_needed;
            bw->bw_300 -= reduce;
            reduce_needed -= reduce;
        }
        // // 减2级
        // if (reduce_needed > 0 && bw->bw_200 > 0) {
        //     int reduce = bw->bw_200 < reduce_needed ? bw->bw_200 : reduce_needed;
        //     bw->bw_200 -= reduce;
        //     reduce_needed -= reduce;
        // }
        // // 减1级
        // if (reduce_needed > 0 && bw->bw_100 > 0) {
        //     int reduce = bw->bw_100 < reduce_needed ? bw->bw_100 : reduce_needed;
        //     bw->bw_100 -= reduce;
        //     reduce_needed -= reduce;
        // }
    }
    // 恢复带宽：3→4→5→6级
    else {
        int increase_needed = bw_diff;
        // // 恢复1级
        // if (increase_needed > 0 && bw->bw_100 < TC_100_INIT) {
        //     int increase = (TC_100_INIT - bw->bw_100) < increase_needed ? (TC_100_INIT - bw->bw_100) : increase_needed;
        //     bw->bw_100 += increase;
        //     increase_needed -= increase;
        // }
        // // 恢复2级
        // if (increase_needed > 0 && bw->bw_200 < TC_200_INIT) {
        //     int increase = (TC_200_INIT - bw->bw_200) < increase_needed ? (TC_200_INIT - bw->bw_200) : increase_needed;
        //     bw->bw_200 += increase;
        //     increase_needed -= increase;
        // }
        // 恢复3级
        if (increase_needed > 0 && bw->bw_300 < TC_300_MAX) {
            int increase = (TC_300_MAX - bw->bw_300) < increase_needed ? (TC_300_MAX - bw->bw_300) : increase_needed;
            bw->bw_300 += increase;
            increase_needed -= increase;
        }
        // 恢复4级
        if (increase_needed > 0 && bw->bw_400 < TC_400_MAX) {
            int increase = (TC_400_MAX - bw->bw_400) < increase_needed ? (TC_400_MAX - bw->bw_400) : increase_needed;
            bw->bw_400 += increase;
            increase_needed -= increase;
        }
        // 恢复5级
        if (increase_needed > 0 && bw->bw_500 < TC_500_MAX) {
            int increase = (TC_500_MAX - bw->bw_500) < increase_needed ? (TC_500_MAX - bw->bw_500) : increase_needed;
            bw->bw_500 += increase;
            increase_needed -= increase;
        }
        // 恢复6级
        if (increase_needed > 0 && bw->bw_600 < TC_600_MAX) {
            int increase = (TC_600_MAX - bw->bw_600) < increase_needed ? (TC_600_MAX - bw->bw_600) : increase_needed;
            bw->bw_600 += increase;
            increase_needed -= increase;
        }
    }

    int totalBw = bw->bw_100 + bw->bw_200 + bw->bw_300 + bw->bw_400 + bw->bw_500 + bw->bw_600;

    // 更新TC规则
    // update_tc_class(1, calc_total_bw(bw), totalBw, 0); // 根类1:1
    update_tc_class(1, totalBw, totalBw, 0); // 根类1:1
    update_tc_class(100, bw->bw_100, totalBw, 1);      // 1:100
    update_tc_class(200, bw->bw_200, totalBw, 2);      // 1:200
    update_tc_class(300, bw->bw_300, totalBw, 3);      // 1:300
    update_tc_class(400, bw->bw_400, totalBw, 4);      // 1:400
    update_tc_class(500, bw->bw_500, totalBw, 5);      // 1:500
    update_tc_class(600, bw->bw_600, totalBw, 6);      // 1:600

    // 更新总带宽
    bw->total = calc_total_bw(bw);
    LOG_Debug("📶 带宽调整完成：总带宽=%dK | 1:100=%dK | 1:200=%dK | 1:300=%dK | 1:400=%dK | 1:500=%dK | 1:600=%dK",
            bw->total, bw->bw_100, bw->bw_200, bw->bw_300, bw->bw_400, bw->bw_500, bw->bw_600);

    // pthread_mutex_unlock(&g_qosCtx.mutex);
}

/**
 * @brief 新增：单个队列带宽扩容（带上限控制）
 * @param queue_idx 队列索引（QUEUE_100_IDX ~ QUEUE_600_IDX）
 * @return 1=扩容成功，0=已达上限，-1=参数错误
 */
static int expand_queue_bw(int queue_idx)
{
    if (queue_idx < 0 || queue_idx > QUEUE_600_IDX) {
        LOG_Error("❌ 队列索引错误：%d", queue_idx);
        return -1;
    }

    pthread_mutex_lock(&g_qosCtx.mutex);
    BandwidthInfo *bw = &g_qosCtx.bw_info;
    uint32_t *queue_bw = NULL;
    uint32_t queue_max = 0;
    int classid = 0;

    // 绑定队列索引与带宽/上限/TC类ID
    switch(queue_idx) {
        case QUEUE_100_IDX:
            queue_bw = &bw->bw_100;
            queue_max = TC_100_MAX;
            classid = 100;
            break;
        case QUEUE_200_IDX:
            queue_bw = &bw->bw_200;
            queue_max = TC_200_MAX;
            classid = 200;
            break;
        case QUEUE_300_IDX:
            queue_bw = &bw->bw_300;
            queue_max = TC_300_MAX;
            classid = 300;
            break;
        case QUEUE_400_IDX:
            queue_bw = &bw->bw_400;
            queue_max = TC_400_MAX;
            classid = 400;
            break;
        case QUEUE_500_IDX:
            queue_bw = &bw->bw_500;
            queue_max = TC_500_MAX;
            classid = 500;
            break;
        case QUEUE_600_IDX:
            queue_bw = &bw->bw_600;
            queue_max = TC_600_MAX;
            classid = 600;
            break;
        default:
            pthread_mutex_unlock(&g_qosCtx.mutex);
            return -1;
    }

    // 检查是否已达上限
    if (*queue_bw >= queue_max) {
        LOG_Debug("⚠️ 队列1:%d已达带宽上限(%dK)，无需扩容", classid, queue_max);
        pthread_mutex_unlock(&g_qosCtx.mutex);
        return 0;
    }

    // 计算扩容后带宽（不超过上限）
    uint32_t new_bw = *queue_bw + QUEUE_EXPAND_STEP;
    if (new_bw > queue_max) {
        new_bw = queue_max;
    }

    // 更新队列带宽
    *queue_bw = new_bw;
    // int totalBw = calc_total_bw(bw);
    int totalBw = g_qosCtx.tx_rate;
    update_tc_class(classid, new_bw, totalBw, queue_idx + 1); // prio=索引+1

    LOG_Debug("✅ 队列1:%d扩容完成: %dK → %dK (上限:%dK)", 
            classid, *queue_bw - QUEUE_EXPAND_STEP, new_bw, queue_max);

    // 更新总带宽
    bw->total = totalBw;
    pthread_mutex_unlock(&g_qosCtx.mutex);

    return 1;
}



/**
 * @brief 新增：吞吐量接近总带宽时的丢包检测与队列扩容
 */
static void check_thr_near_bw_and_expand(void)
{
    pthread_mutex_lock(&g_qosCtx.mutex);
    uint32_t current_thr = g_qosCtx.up_rate;
    uint32_t total_bw = g_qosCtx.tx_rate;
    pthread_mutex_unlock(&g_qosCtx.mutex);

    // 判定是否"接近"总带宽：吞吐量 ≥ 总带宽*90% 或 吞吐量=总带宽
    int is_near = (current_thr >= (uint32_t)(total_bw * THR_CLOSE_TO_BW_THRESH)) || (current_thr == total_bw);
    if (!is_near) {
        return;
    }

    LOG_Debug("📌 吞吐量(%dK)接近总带宽(%dK)，开始检测各队列丢包率...", current_thr, total_bw);

    // 逐个检查5个队列丢包率，超过阈值则扩容
    update_all_queue_loss(); // 刷新最新丢包率
    for (int i = 0; i < (int)QUEUE_COUNT; i++) {
        float loss_rate = g_queue_loss[i].loss_rate;
        if (loss_rate < 0) {
            LOG_Warn("⚠️ 队列%d丢包率读取失败, 跳过", i+1);
            continue;
        }

        if (loss_rate > QUEUE_LOSS_TRIGGER_THRESH) {
            LOG_Debug("⚠️ 队列%d丢包率(%.2f%%)超过阈值(%.2f%%)，触发扩容", 
                    i+1, loss_rate, QUEUE_LOSS_TRIGGER_THRESH);
            expand_queue_bw(i);
        } else {
            LOG_Debug("✅ 队列%d丢包率(%.2f%%)正常，无需扩容", i+1, loss_rate);
        }
    }
}

/**
 * @brief 清理TC规则（优雅退出）
 */
void cleanup(void)
{
    LOG_Debug("========== TBOX QoS策略退出==========");
    if (check_interface()) {
        char cmd[128];
        snprintf(cmd, sizeof(cmd), "tc qdisc del dev %s root >/dev/null 2>&1", ETH_NAME);
        SrGetSystemRetValue(cmd);
        LOG_Debug("✅ 清理%s上的TC规则完成", ETH_NAME);
    }
    exit(0);
}


/**
 * @brief 读取指定队列的丢包率(%)
 * @param classid 队列ID（100/200/300/400/500）
 * @return 丢包率(%)，失败返回-1
 */
static float get_queue_loss_rate(int classid)
{
    char cmd[256];
    char buf[1024] = {0};
    FILE *fp = NULL;
    float loss_rate = -1.0f;
    int pkt_total = 0, pkt_loss = 0;

    // 执行tc命令获取队列统计（HTB类统计）
    snprintf(cmd, sizeof(cmd), "tc -s class show dev %s classid 1:%d", ETH_NAME, classid);
    fp = popen(cmd, "r");
    if (!fp) {
        LOG_Error("执行tc命令失败: %s", cmd);
        return -1.0f;
    }

    // 解析输出（示例："Sent 1000 bytes 100 pkt (dropped 10, overlimits 0)"）
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strstr(buf, "dropped")) {
            sscanf(buf, "%*s %*d %*s %d %*s (dropped %d", &pkt_total, &pkt_loss);
            if (pkt_total > 0) {
                loss_rate = (float)pkt_loss / pkt_total * 100; // 转百分比
            }
            break;
        }
    }
    pclose(fp);
    return loss_rate;
}

/**
 * @brief 统计所有队列丢包率（滑动平均）
 * 
 */
static void update_all_queue_loss(void)
{
    int classids[] = {100, 200, 300, 400, 500, 600};

    for (int i = 0; i < (int)QUEUE_COUNT; i++) {
        float loss = get_queue_loss_rate(classids[i]);
        if (loss < 0)
            continue;

        // 滑动平均：累加后求平均，避免单次波动
        QueueLossStat_t *stat = &g_queue_loss[i];
        stat->loss_sum += loss;
        stat->sample_count++;
        if (stat->sample_count >= LOSS_RATE_SAMPLE_CNT) {
            stat->loss_rate = stat->loss_sum / stat->sample_count;
            // 重置采样
            stat->loss_sum = 0;
            stat->sample_count = 0;
        }
    }
}


/**
 * @brief 基于丢包率的带宽调整决策
 * @return 目标调整步长（正数=恢复，负数=压缩，0=无调整）
 */
static int get_loss_driven_step(void)
{
    update_all_queue_loss();
    int adjust_step = 0;

    // 逻辑1：高优队列（1:100/1:200）丢包率超标 → 紧急压缩低优带宽
    float high_prio_loss = (g_queue_loss[QUEUE_100_IDX].loss_rate + g_queue_loss[QUEUE_200_IDX].loss_rate) / 2;
    if (high_prio_loss > LOSS_RATE_HIGH_PRIO_THRESH) {
        adjust_step = -LOSS_ADJUST_STEP; // 压缩LOSS_ADJUST_STEP 10M
        LOG_Debug("⚠️ 高优队列丢包率超标(%.2f%% > %.2f%%)，紧急压缩低优带宽%dK",
                high_prio_loss, LOSS_RATE_HIGH_PRIO_THRESH, LOSS_ADJUST_STEP);
        g_qosCtx.loss_driven_adjust = 1;
    }
    // 逻辑2：低优队列整体丢包率过高 → 适度恢复带宽
    else {
        float low_prio_loss = (g_queue_loss[QUEUE_300_IDX].loss_rate + 
                               g_queue_loss[QUEUE_400_IDX].loss_rate + 
                               g_queue_loss[QUEUE_500_IDX].loss_rate +
                               g_queue_loss[QUEUE_600_IDX].loss_rate) / 4;
        if (low_prio_loss > LOSS_RATE_LOW_PRIO_THRESH && g_qosCtx.is_restricted) {
            adjust_step = LOSS_ADJUST_STEP / 2; // 恢复一半步长，避免过度
            LOG_Debug("✅ 低优队列丢包率过高(%.2f%% > %.2f%%)，适度恢复带宽%dK",
                    low_prio_loss, LOSS_RATE_LOW_PRIO_THRESH, adjust_step);
            g_qosCtx.loss_driven_adjust = 1;
        } else {
            g_qosCtx.loss_driven_adjust = 0; // 无丢包驱动调整
        }
    }
    return adjust_step;
}


/**
 * @brief 动态调整1:1根队列总带宽（核心新增逻辑）
 */
static void adjust_all_bandwidth(void)
{
    char cmd[256];

    pthread_mutex_lock(&g_qosCtx.mutex);

    // 1. 获取实时带宽和当前吞吐量（统一单位：Kbit/s）
    double real_time_bw_kbit = calculate_real_time_bandwidth();
    uint32_t current_rate = g_qosCtx.up_rate;
    if (real_time_bw_kbit < 0) {
        LOG_Error("❌ 读取实时带宽失败，跳过根队列调整");
        pthread_mutex_unlock(&g_qosCtx.mutex);
        return;
    }
    uint32_t real_time_bw = (uint32_t)real_time_bw_kbit;

    // 2. 获取1:100+1:200带宽和（作为最小值限制）
    uint32_t bw_100_200_sum = g_qosCtx.bw_info.bw_100 + g_qosCtx.bw_info.bw_200;
    // 当前1:1根队列总带宽
    uint32_t current_root_bw = g_qosCtx.bw_info.total;

    // 3. 根据条件调整队列带宽
    uint32_t new_root_bw = current_root_bw;

    LOG_Debug("real_time_bw[TxRate]: %d, current_rate[吞吐量]: %d\n", real_time_bw, current_rate);
    if (real_time_bw > current_rate) {
        // 条件1：实时带宽 > 吞吐量 → 新带宽 = 吞吐量 + 512K
        new_root_bw = current_rate + BW_512K;
        // 保证新带宽 > 1:100+1:200带宽和
        if (new_root_bw <= bw_100_200_sum) {
            // new_root_bw = bw_100_200_sum + BW_512K;
            // new_root_bw = bw_100_200_sum;
            new_root_bw = bw_100_200_sum + 100;
        }
        LOG_Debug("📈 实时带宽(%.0fK) > 吞吐量(%dK)，根队列带宽调整为：%dK(≥1:100+1:200=%dK)",
                 real_time_bw_kbit, current_rate, new_root_bw, bw_100_200_sum);
        
        // 更新1:1
        snprintf(cmd, sizeof(cmd),
                 "tc class change dev %s parent 1: classid 1:1 htb rate %dkbit ceil %dkbit prio 0",
                 ETH_NAME, new_root_bw, TOTAL_BANDWIDTH);
        SrGetSystemRetValue(cmd);
        LOG_Debug("✅ 执行根队列更新命令: %s", cmd);

        uint32_t new_ceil_bw = new_root_bw / 4;

        // 调整1:300 1:400 1:500 1:600
        snprintf(cmd, sizeof(cmd), "tc class change dev %s parent 1:1 classid 1:300 htb rate 1bps ceil %dkbit prio 3", ETH_NAME, new_ceil_bw);
        SrGetSystemRetValue(cmd);

        snprintf(cmd, sizeof(cmd), "tc class change dev %s parent 1:1 classid 1:400 htb rate 1bps ceil %dkbit prio 4", ETH_NAME, new_ceil_bw);
        SrGetSystemRetValue(cmd);

        snprintf(cmd, sizeof(cmd), "tc class change dev %s parent 1:1 classid 1:500 htb rate 1bps ceil %dkbit prio 5", ETH_NAME, new_ceil_bw);
        SrGetSystemRetValue(cmd);

        snprintf(cmd, sizeof(cmd), "tc class change dev %s parent 1:1 classid 1:600 htb rate 1bps ceil %dkbit prio 6", ETH_NAME, new_ceil_bw);
        SrGetSystemRetValue(cmd);
    } else if (real_time_bw == current_rate) {
        // 条件2：实时带宽 = 吞吐量 → 新带宽 = 当前带宽 + 1M
        new_root_bw = current_root_bw + BW_1M;
        LOG_Debug("📈 实时带宽(%.0fK) = 吞吐量(%dK), 根队列带宽增加1M: %dK → %dK",
                 real_time_bw_kbit, current_rate, current_root_bw, new_root_bw);
    } else {
        // 其他情况不调整
        pthread_mutex_unlock(&g_qosCtx.mutex);
        return;
    }

    // 4. 边界校验：不超过全局总带宽上限
    if (new_root_bw > TOTAL_BANDWIDTH) {
        new_root_bw = TOTAL_BANDWIDTH;
        LOG_Warn("⚠️ 根队列带宽已达全局上限(%dK)，无法继续调整", TOTAL_BANDWIDTH);
    }

#if 0
    // 5. 更新根队列带宽（1:1）
    if (new_root_bw != current_root_bw) {
        // 更新共享变量
        g_qosCtx.bw_info.total = new_root_bw;
        // 调用TC命令更新根队列
        char cmd[256];
        snprintf(cmd, sizeof(cmd),
                 "tc class change dev %s parent 1: classid 1:1 htb rate %dkbit ceil %dkbit prio 0",
                 ETH_NAME, new_root_bw, TOTAL_BANDWIDTH);
        SrGetSystemRetValue(cmd);
        LOG_Debug("✅ 执行根队列更新命令: %s", cmd);
    }
#endif

    pthread_mutex_unlock(&g_qosCtx.mutex);
}

/**
 * @brief 基于上行延迟计算带宽调整步长（核心逻辑）
 * @return 带宽调整步长（Kbit/s）：正数=增加，负数=减小，0=无调整
 */
static int get_latency_bw_step(void)
{
    int latency = g_qosCtx.up_latency;
    int step = 0; // 调整步长（单位：Kbit/s，1M=1024K，10M=10240K）

    // 严格按延迟区间定义调整步长
    if (latency < 10) {
        step = 10 * 10240; // <10ms：+10M
        LOG_Debug("📶 延迟<%dms, 带宽+10M(步长：%dK)", 10, step);
    } 
    // else if (latency >= 10 && latency < 15) {
    //     step = 7 * 1024;  // 10~15ms：+7M
    //     LOG_Debug("📶 延迟%d~%dms, 带宽+7M(步长：%dK)", 10, 15, step);
    // } else if (latency >= 15 && latency < 20) {
    //     step = 5 * 1024;  // 10~30ms：+5M
    //     LOG_Debug("📶 延迟%d~%dms, 带宽+1M(步长：%dK)", 15, 20, step);
    // } else if (latency >= 20 && latency < 25) {
    //     step = 3 * 1024;  // 10~30ms：+3M
    //     LOG_Debug("📶 延迟%d~%dms, 带宽+1M(步长：%dK)", 20, 25, step);
    // } else if (latency >= 25 && latency < 30) {
    //     step = 1 * 1024;  // 10~30ms：+1M
    //     LOG_Debug("📶 延迟%d~%dms, 带宽+1M(步长：%dK)", 25, 30, step);
    // } 
    else if (latency >= 10 && latency < 25) {
        if (GetMqttSendStatus() != 1) {
            step = 0;
            LOG_Debug("📶 延迟%d~%dms, MQTT断连, 带宽不调整", 10, 25);
        } else {
            step = 2 * 10240;  // 10~25ms：+3M
            LOG_Debug("📶 延迟%d~%dms, 带宽+3M(步长：%dK)", 10, 25, step);
        }
    } else if (latency >= 25 && latency < 35) {
        step = -2 * 10240; // 30~50ms：-3M
        LOG_Debug("📶 延迟%d~%dms, 带宽-1M(步长：%dK)", 25, 30, step);
    } else if (latency >= 35 && latency < 50) {
        step = -4 * 10240; // 30~50ms：-5M
        LOG_Debug("📶 延迟%d~%dms, 带宽-1M(步长：%dK)", 30, 50, step);
    } else if (latency >= 50 && latency < 80) {
        step = -5 * 10240;  // 50~80ms：-5M
        LOG_Debug("📶 延迟%d~%dms, 带宽-5M(步长: %dK)", 50, 80, step);
    } else if (latency >= 80 && latency < 110) {
        step = -10 * 10240;// 80~110ms：-10M
        LOG_Debug("📶 延迟%d~%dms, 带宽-10M(步长：%dK)", 80, 110, step);
    } else if (latency >= 110 && latency < 140) {
        step = -15 * 10240;// 110~140ms：-15M
        LOG_Debug("📶 延迟%d~%dms, 带宽-15M(步长：%dK)", 110, 140, step);
    } else if (latency >= 140 && latency < 170) {
        step = -20 * 10240;// 140~170ms：-20M
        LOG_Debug("📶 延迟%d~%dms, 带宽-20M(步长：%dK)", 140, 170, step);
    } else if (latency >= 170 && latency < 200) {
        step = -25 * 10240;// 170~200ms：-25M
        LOG_Debug("📶 延迟%d~%dms, 带宽-30M(步长：%dK)", 170, 200, step);
    } else if (latency >= 200 && latency < 300) {
        step = -35 * 10240;// 200~300ms：-35M
        LOG_Debug("📶 延迟%d~%dms, 带宽-30M(步长：%dK)", 200, 300, step);
    } else if (latency >= 300 && latency < 400) {
        step = -45 * 10240;// 200~300ms：-45M
        LOG_Debug("📶 延迟%d~%dms, 带宽-30M(步长：%dK)", 300, 400, step);
    } else if (latency >= 400 && latency < 500) {
        step = -55 * 10240;// 400~500ms：-55M
        LOG_Debug("📶 延迟%d~%dms, 带宽-50M(步长：%dK)", 400, 500, step);
    } else if (latency >= 500 && latency <= 1000) {
        step = -70 * 10240;// 500~1000ms：-70M
        LOG_Debug("📶 延迟%d~%dms, 带宽-70M(步长：%dK)", 500, 1000, step);
    } else {
        // step = 0; // 未命中区间，无调整
        // LOG_Debug("📶 延迟%dms, 无带宽调整", latency);
        step = -70 * 10240;// 500~1000ms：-70M
        LOG_Debug("📶 延迟%dms, 带宽-70M(步长：%dK)", latency, step);
    }

    return step;
}

/**
 * @brief 基于上行延迟的QoS带宽调整入口函数
 */
void adjust_bw_by_latency(void)
{
    pthread_mutex_lock(&g_qosCtx.mutex);

    // 1. 基础校验：延迟非法则退出
    if (g_qosCtx.up_latency < 0) {
        LOG_Error("❌ 上行延迟非法（%dms), 跳过调整", g_qosCtx.up_latency);
        pthread_mutex_unlock(&g_qosCtx.mutex);
        return;
    }

    // 2. 获取延迟对应的调整步长
    int adjust_step = get_latency_bw_step();
    if (adjust_step == 0) {
        pthread_mutex_unlock(&g_qosCtx.mutex);
        return;
    }

    // 3. 计算目标总带宽（带边界保护）
    BandwidthInfo *bw = &g_qosCtx.bw_info;
    int32_t current_total = calc_total_bw(bw);
    int32_t target_total = current_total + adjust_step;

    // 边界限制：最小不低于高优队列总和（1:100+1:200），最大不超过全局总带宽
    int32_t min_total = TC_MIN_BW; // 保障1:100/1:200并发达到各自上限
    int32_t max_total = TOTAL_BANDWIDTH;         // 全局总带宽上限


    if (target_total < min_total) {
        LOG_Warn("⚠️ 目标带宽(%dK)低于高优队列最小值(%dK)，限制为最小值", target_total, min_total);
        target_total = min_total;
    }
    if (target_total > max_total) {
        LOG_Warn("⚠️ 目标带宽(%dK)超过全局上限(%dK)，限制为上限", target_total, max_total);
        target_total = max_total;
    }

    // 4. 执行带宽调整（复用现有调整逻辑）
    if (target_total != current_total) {
        adjust_low_prio_bw(target_total);
    } else {
        LOG_Debug("📶 目标带宽与当前一致（%dK), 无需调整", current_total);
    }

    pthread_mutex_unlock(&g_qosCtx.mutex);
}

/**
 * @brief 延迟QoS调整线程（示例：循环检测+调整）
 * @param arg 线程参数（无）
 * @return 线程返回值
 */
void *QoS_LatencyAdjust(void *arg)
{
    LOG_Debug("✅ 延迟驱动QoS线程启动");
    
    while (g_qosCtx.run_flag) {
        // // 1. 检查网络可用性
        // if (!check_network()) {
        //     sleep(1);
        //     continue;
        // }

        // if (GetMqttSendStatus() != 1) {
        //     usleep(50000);
        //     continue;
        // }

        if (GetQoSFlag() != 1) {
            usleep(50000);
            continue;
        }

        // 3. 执行延迟驱动的带宽调整
        adjust_bw_by_latency();

        LOG_Debug("!!!!!!!!!!!!!!");

        // 4. 调整间隔（可配置，示例1秒）
        sleep(1);
    }

    LOG_Debug("❌ 延迟驱动QoS线程退出");
    pthread_exit(NULL);
}

// ====================== 4. 线程函数 ======================
// /**
//  * @brief Ping检测线程：周期性检测延迟，写入共享变量
//  */
// void *QoS_PingDetectThread(void *arg)
// {
//     static int count = 0;
//     LOG_Debug("Ping检测线程启动(周期：%.1fs, 阈值：%dms)...", PING_INTERVAL, DELAY_THRESHOLD);

//     while (g_qosCtx.run_flag) {
//         if (GetQoSFlag() != 1) {
//             usleep(50000);
//             continue;
//         }

//         int delay = get_ping_delay();

//         // 线程安全更新延迟值
//         pthread_mutex_lock(&g_qosCtx.mutex);
//         g_qosCtx.ping_delay = delay;
//         pthread_mutex_unlock(&g_qosCtx.mutex);

//         count++;
//         if (count == 10) {
//             LOG_Debug("📊 实时状态: ping延迟=%dms | 总带宽=%dM | 稳定计数=%d/%d | 限制状态=%d",
//                 delay, g_qosCtx.bw_info.total, g_qosCtx.stable_counter,
//                 STABLE_RECOVERY_COUNT, g_qosCtx.is_restricted);
//             count = 0;
//         }

//         usleep((useconds_t)(PING_INTERVAL * 1000000)); // 100ms周期
//     }

//     LOG_Debug("Ping检测线程退出");
//     pthread_exit(NULL);
// }



/**
 * @brief 吞吐量检测线程：周期性读取吞吐量，更新共享变量
 * 
 * @param arg 
 * @return void* 
 */
void *QoS_ThroughputDetectThread(void *arg)
{
    int rsrp, sinr;

    LOG_Debug("吞吐量检测线程启动(上报周期：%dms)...", THROUGHPUT_INTERVAL);

    while (g_qosCtx.run_flag) {
        if (GetQoSFlag() != 1) {
            usleep(50000);
            continue;
        }

        pthread_mutex_lock(&g_qosCtx.mutex);
        // 核心修改：对rate进行阈值判断
        uint32_t current_thr = g_qosCtx.up_rate;
        
        // 当吞吐量小于1024K时，设置为1024；大于1024K时保留原始值
        if (current_thr < TC_MIN_BW) {
            LOG_Debug("📌 吞吐量低于1024K(%dkbps), 修正为1024kbps", current_thr);
            g_qosCtx.up_rate = TC_MIN_BW;
            current_thr = TC_MIN_BW; // 同步修正当前使用的值
        } else {
            g_qosCtx.up_rate = current_thr; // 大于等于1024K时保留原值
        }
        
        uint32_t last_thr = g_qosCtx.last_thr;
        
        // 校验吞吐量合法性，更新稳定计数器
        if (is_thr_valid(current_thr, last_thr)) {
            g_qosCtx.stable_thr_counter++;
        } else {
            g_qosCtx.stable_thr_counter = 0; // 波动过大，重置计数
        }
        
        g_qosCtx.last_thr = current_thr; // 更新上一次值
        pthread_mutex_unlock(&g_qosCtx.mutex);

        // 检测吞吐量是否接近总带宽，若是则触发队列丢包检查与扩容
        // check_thr_near_bw_and_expand();

        // 每10次打印状态（约5秒）
        static int count = 0;
        count++;
        if (count == 10) {
            update_all_queue_loss(); // 刷新丢包率

            LOG_Debug("📊 实时状态: 吞吐量=%dkbps | 低优带宽占比=%.0f%% | 稳定计数=%d/%d | 限制状态=%d",
                current_thr, get_thr_bw_ratio(current_thr)*100,
                g_qosCtx.stable_thr_counter, THROUGHPUT_STABLE_COUNT,
                g_qosCtx.is_restricted);
            
            LOG_Debug("📊 丢包率状态: 1:100=%.2f%% | 1:200=%.2f%% | 1:300=%.2f%% | 1:400=%.2f%% | 1:500=%.2f%% | 1:600=%.2f%%",
                g_queue_loss[QUEUE_100_IDX].loss_rate,
                g_queue_loss[QUEUE_200_IDX].loss_rate,
                g_queue_loss[QUEUE_300_IDX].loss_rate,
                g_queue_loss[QUEUE_400_IDX].loss_rate,
                g_queue_loss[QUEUE_500_IDX].loss_rate,
                g_queue_loss[QUEUE_600_IDX].loss_rate);
            
            count = 0;
        }

        // 调用根队列带宽动态调整逻辑
        adjust_all_bandwidth();

        usleep((useconds_t)(THROUGHPUT_INTERVAL * 1000)); // 与吞吐量上报周期同步
    }

    LOG_Debug("吞吐量检测线程退出");
    pthread_exit(NULL);
}

// /**
//  * @brief 带宽调整线程：根据Ping结果动态调整带宽
//  */
// void *QoS_BwAdjustThread(void *arg)
// {
//     LOG_Debug("带宽调整线程启动...");
//     LOG_Debug("带宽调整策略：");
//     for (int i = 0; i < STRATEGY_COUNT; i++) {
//         AdjustStrategy *s = &adjust_strategies[i];
//         LOG_Debug("  %d~%dms → 减%dM", s->lower, s->upper, s->reduce);
//     }

//     while (g_qosCtx.run_flag) {
//         if (GetQoSFlag() != 1) {
//             usleep(50000);
//             continue;
//         }

//         pthread_mutex_lock(&g_qosCtx.mutex);
//         int delay = g_qosCtx.ping_delay;
//         int stable_counter = g_qosCtx.stable_counter;
//         int is_restricted = g_qosCtx.is_restricted;
//         pthread_mutex_unlock(&g_qosCtx.mutex);

//         // 逻辑1：延迟超标 → 削减带宽
//         if (delay > DELAY_THRESHOLD) {
//             // 重置稳定计数器，标记限制状态
//             pthread_mutex_lock(&g_qosCtx.mutex);
//             g_qosCtx.stable_counter = 0;
//             g_qosCtx.is_restricted = 1;
//             pthread_mutex_unlock(&g_qosCtx.mutex);

//             int reduce = get_reduce_amount(delay);
//             LOG_Debug("⚠️ 延迟超标（%dms > %dms), 触发限制策略：减幅=%dM",
//                     delay, DELAY_THRESHOLD, reduce);

//             // 计算目标总带宽并调整
//             int target_total = g_qosCtx.bw_info.total - reduce;
//             adjust_low_prio_bw(target_total);
//         }
//         // 逻辑2：延迟稳定 → 累计计数，达标则恢复带宽
//         else {
//             pthread_mutex_lock(&g_qosCtx.mutex);
//             g_qosCtx.stable_counter++;
//             stable_counter = g_qosCtx.stable_counter;
//             pthread_mutex_unlock(&g_qosCtx.mutex);

//             if (stable_counter >= STABLE_RECOVERY_COUNT && is_restricted) {
//                 // 重置计数器，恢复带宽
//                 pthread_mutex_lock(&g_qosCtx.mutex);
//                 g_qosCtx.stable_counter = 0;
//                 pthread_mutex_unlock(&g_qosCtx.mutex);

//                 LOG_Debug("✅ 延迟稳定（连续%d次≤%dms), 触发恢复策略：增幅=%dM",
//                         STABLE_RECOVERY_COUNT, DELAY_THRESHOLD, RECOVER_AMOUNT);

//                 // 计算目标总带宽并调整
//                 int target_total = g_qosCtx.bw_info.total + RECOVER_AMOUNT;
//                 adjust_low_prio_bw(target_total);

//                 // 检查是否恢复到最大值，取消限制状态
//                 pthread_mutex_lock(&g_qosCtx.mutex);
//                 if (g_qosCtx.bw_info.total == TOTAL_BANDWIDTH) {
//                     g_qosCtx.is_restricted = 0;
//                     LOG_Debug("🎉 带宽已恢复到最大值（%dM), 退出限制状态", TOTAL_BANDWIDTH);
//                 }
//                 pthread_mutex_unlock(&g_qosCtx.mutex);
//             }
//         }

//         usleep((useconds_t)(PING_INTERVAL * 1000000)); // 与Ping线程同周期
//     }

//     LOG_Debug("带宽调整线程退出");
//     pthread_exit(NULL);
// }


/**
 * @brief 带宽调整线程：吞吐量+丢包率双维度驱动
 * 
 * @param arg 
 * @return void* 
 */
void *QoS_BwAdjustThread(void *arg)
{
    LOG_Debug("吞吐量+丢包率 双维度带宽调整线程启动...");
    LOG_Debug("带宽调整策略：");
    for (int i = 0; i < THR_STRATEGY_COUNT; i++) {
        ThrAdjustStrategy *s = &thr_strategies[i];
        LOG_Debug("  %dkbps~%dkbps → 低优带宽占比=%.0f%%", s->thr_lower, s->thr_upper, s->bw_ratio*100);
    }
    LOG_Debug("丢包率阈值：高优=%.2f%% | 低优=%.2f%%", LOSS_RATE_HIGH_PRIO_THRESH, LOSS_RATE_LOW_PRIO_THRESH);

    while (g_qosCtx.run_flag) {
        if (GetQoSFlag() != 1) {
            usleep(50000);
            continue;
        }

#if 0
        pthread_mutex_lock(&g_qosCtx.mutex);
        uint32_t current_thr = g_qosCtx.up_rate;
        uint32_t stable_count = g_qosCtx.stable_thr_counter;
        pthread_mutex_unlock(&g_qosCtx.mutex);

        // 步骤1：优先处理丢包驱动的调整（高优丢包紧急压缩）
        int loss_step = get_loss_driven_step();
        if (loss_step != 0) {
            int target_total = g_qosCtx.bw_info.total + loss_step;
            // 边界检查
            if (target_total < TC_MIN_BW)
                target_total = TC_MIN_BW;
            if (target_total > TOTAL_BANDWIDTH)
                target_total = TOTAL_BANDWIDTH;
            
            LOG_Debug("📌 丢包驱动调整：当前总带宽=%dK → 目标=%dK (步长=%dK)",
                    g_qosCtx.bw_info.total, target_total, loss_step);
            adjust_low_prio_bw(target_total);
            usleep((useconds_t)(THROUGHPUT_INTERVAL * 1000));
            continue; // 丢包驱动调整优先，跳过本次吞吐量逻辑
        }

        // 步骤2：吞吐量驱动的常规调整
        if (stable_count >= THROUGHPUT_STABLE_COUNT && !g_qosCtx.loss_driven_adjust) {
            float target_ratio = get_thr_bw_ratio(current_thr);
            int low_prio_target = TOTAL_BANDWIDTH * target_ratio;
            adjust_low_prio_bw(low_prio_target);
        }
#endif
        usleep((useconds_t)(THROUGHPUT_INTERVAL * 1000)); // 与吞吐量检测周期同步
    }

    LOG_Debug("带宽调整线程退出");
    pthread_exit(NULL);
}

/**
 * @brief 获取上行发送速率
 * 
 * @param arg 
 * @return void* 
 */
void *QoS_GetRate(void *arg)
{
    while (1) {
        // 1. 计算父类1:1的目标rate
        double bw_kbit = calculate_real_time_bandwidth();
        if (bw_kbit < 0) {
            fprintf(stderr, "获取实时带宽失败\n");
            return NULL;
        }

        // 转换为易读格式输出
        char bw_str[64] = {0};
        convert_bandwidth(bw_kbit * 1000, bw_str, sizeof(bw_str));
        printf("rmnet_data0 实时上行带宽：%s\n", bw_str);
        // printf("用于TC配置的带宽值(Mbit/s): %.2f\n", bw_mbit);

        pthread_mutex_lock(&g_qosCtx.mutex);
        g_qosCtx.tx_rate = bw_kbit;
        pthread_mutex_unlock(&g_qosCtx.mutex);
    }
}


static void ql_data_throughput_callback(QL_DATA_THROUGHPUT_DIRECTION_E direction, ql_data_throughput_info_t *p_msg)
{
    if (direction == QL_DATA_THROUGHPUT_UPLINK) {
        //printf("isValid: %d\n", p_msg->rate_valid);
        if (p_msg->rate_valid) {
            // printf("up_rate: %dkbps\n", p_msg->rate);
            g_qosCtx.up_rate = p_msg->rate;
        }
    }
}

static void ql_data_latency_callback(ql_data_latency_info_t *p_msg)
{
    printf("ul_report_status_valid: %d, ul_link_latency_est_valid: %d\n", p_msg->ul_report_status_valid, p_msg->ul_link_latency_est_valid);

    if (p_msg->ul_report_status_valid == 1 && p_msg->ul_link_latency_est_valid == 1) {
        printf("@@@@ul_link_latency_est: %d\n", p_msg->ul_link_latency_est / 1000);
        g_qosCtx.up_latency = p_msg->ul_link_latency_est / 1000;
    }
}

/**
 * @brief QoS功能初始化
 * 
 * @return int 
 */
int QoS_Init(void)
{
    int ret = -1;
    ql_data_throughput_info_t info = {0};

    // 初始化QoS目录
    SrGetSystemRetValue("mkdir -p /oemdata/qos");

    // 初始化数据吞吐量服务
    ret = ql_data_throughput_init();
    if (ret != QL_ERR_OK) {
        printf("ql_ms_data_throughput_init fail\n");
        return -1;
    }

    // 设置数据吞吐量回调
    ret = ql_data_throughput_ind_cb_set(ql_data_throughput_callback);
    if (ret != QL_ERR_OK) {
        printf("ql_ms_data_throughput_ind_cb_set fail\n");
        return -1;
    }

    // 使能数据吞吐量上报
    ret = ql_data_throughput_report_enable(QL_DATA_THROUGHPUT_UPLINK, THROUGHPUT_INTERVAL);
    if (ret != QL_ERR_OK) {
        printf("ql_data_throughput_report_enable fail\n");
        return -1;
    }

    // 设置延迟回调
    ret = ql_data_latency_ind_cb_set(ql_data_latency_callback);
    if (ret != QL_ERR_OK) {
        printf("ql_data_latency_ind_cb_set fail\n");
        return -1;
    }

    // 使能延迟上报
    ret = ql_data_latency_report_config(1, LATENCY_INTERVAL);
    if (ret != QL_ERR_OK) {
        printf("ql_data_latency_report_config fail\n");
        return -1;
    }

    // 初始化互斥锁
    if (pthread_mutex_init(&g_qosCtx.mutex, NULL) != 0) {
        LOG_Error("❌ 互斥锁初始化失败：%s", strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * @brief QoS功能反初始化
 * 
 */
void QoS_DeInit(void)
{
    // 反初始化数据吞吐量服务
    ql_data_throughput_deinit();

    // 清理资源
    pthread_mutex_destroy(&g_qosCtx.mutex);
}

/**
 * @brief 打开QoS功能
 * 
 * @return int 
 */
int QoS_Enable(void)
{
    int init_retry = 3;
    int init_count = 0;
    char cmd[128];
    int ret = -1;

    // 使能数据吞吐量上报
    ret = ql_data_throughput_report_enable(QL_DATA_THROUGHPUT_UPLINK, THROUGHPUT_INTERVAL);
    if (ret != QL_ERR_OK) {
        printf("ql_data_throughput_report_enable fail\n");
        return -1;
    }
    LOG_Debug("ql_data_throughput_report_enable success");

    ret = ql_data_latency_report_config(1, LATENCY_INTERVAL);
    if (ret != QL_ERR_OK) {
        printf("ql_data_latency_report_config fail\n");
        return -1;
    }
    LOG_Debug("ql_data_latency_report_config 1 success");

    if (check_interface() != 1) {
        LOG_Error("check_interface fail");
        return -1;
    }

    // 设置txqueuelen=5
    snprintf(cmd, sizeof(cmd), "ip link set dev %s txqueuelen 5", ETH_NAME);
    SrGetSystemRetValue(cmd);
    LOG_Debug("✅ 设置txqueuelen=5成功");

    // 初始化HTB规则（重试机制）
    while (init_htb() != 0 && init_count < init_retry) {
        init_count++;
        LOG_Error("⚠️ HTB规则初始化失败, %d/%d重试中...", init_count, init_retry);
        return -1;
    }
    if (init_count >= init_retry) {
        LOG_Error("❌ HTB规则初始化失败(重试%d次), 退出", init_retry);
        return -1;
    }

    LOG_Debug("TBOX QoS已打开");

    return 0;
}

/**
 * @brief 关闭QoS功能
 * 
 * @return int 
 */
int QoS_Disable(void)
{
    char cmd[128];
    int ret = -1;

    ret = ql_data_throughput_report_disable(QL_DATA_THROUGHPUT_UPLINK);
    if (ret != QL_ERR_OK) {
        printf("ql_data_throughput_report_disable fail\n");
        return -1;
    }

    LOG_Debug("ql_data_throughput_report_disable UPLINK success");

    ret = ql_data_throughput_report_disable(QL_DATA_THROUGHPUT_DOWNLINK);
    if (ret != QL_ERR_OK) {
        printf("ql_data_throughput_report_disable fail\n");
        return -1;
    }

    LOG_Debug("ql_data_throughput_report_disable DOWNLINK success");

    ret = ql_data_latency_report_config(0, LATENCY_INTERVAL);
    if (ret != QL_ERR_OK) {
        printf("ql_data_latency_report_config fail\n");
        return -1;
    }
    LOG_Debug("ql_data_latency_report_config 0 success");

    if (check_interface() != 1) {
        LOG_Error("check_interface fail");
        return -1;
    }

    // 设置txqueuelen=1000
    snprintf(cmd, sizeof(cmd), "ip link set dev %s txqueuelen 1000", ETH_NAME);
    SrGetSystemRetValue(cmd);
    LOG_Debug("✅ 恢复txqueuelen=1000成功");

    // 清理规则
    snprintf(cmd, sizeof(cmd), "tc qdisc del dev %s root >/dev/null 2>&1", ETH_NAME);
    SrGetSystemRetValue(cmd);
    LOG_Debug("✅ 清理规则成功");
    LOG_Debug("TBOX QoS已关闭");

    return 0;
}

/**
 * @brief 从MCU同步QoS开关状态，如果已打开，需要使能QoS
 * 
 * @return int 
 */
int QoS_CheckSwitch(void)
{
    SimCardInfoToMpu_t simCardInfoFromMcu = {0};
    uint8_t qosSwitch = 0;

    int32_t getInfoResult = GetSimCardInfoFromMcu(&simCardInfoFromMcu);
    if (getInfoResult != RET_OK) {
        LOG_Error("Failed to get sim card info from MCU");
        return -1;
    }

    qosSwitch = (simCardInfoFromMcu.funcSwitch & 0x08) >> 3; // bit3
    
    return qosSwitch;
}
