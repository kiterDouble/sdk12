#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "net_bewinner_sim_sw_task.h"
#include "err_code.h"
#include "sr_log.h"
#include "sr_param.h"
#include "modem_api.h"


/* 最大支持的运营商 SIM 卡槽数量（三网卡） */
#define BEWINNER_MAX_SIM_NUM    3
/* 运营商 ID 宏定义，按切换顺序：联通 -> 移动 -> 电信 */
#define BEWINNER_OPERATOR_UNICOM    1   /* 中国联通 */
#define BEWINNER_OPERATOR_MOBILE    2   /* 中国移动 */
#define BEWINNER_OPERATOR_TELECOM   3   /* 中国电信 */

/* ICCID 长度范围（兼容中国移动 19 位、中国联通 20 位等情况） */
#define BEWINNER_ICCID_LEN_MIN  17
#define BEWINNER_ICCID_LEN_MAX  20

/* AT 指令执行超时时间，单位毫秒 */
#define BEWINNER_AT_CMD_TIMEOUT_MS 1000

/* 读取多网卡 ID 与 ICCID 列表的 AT 指令（APDU：80 F2 EE CC 3A） */
#define BEWINNER_AT_READ_INFO      "AT+CSIM= 10,\"80F2EECC3A\""
/* 切换到联通（APDU：80 F2 EE 01 00） */
#define BEWINNER_AT_SWITCH_UNICOM  "AT+CSIM= 10,\"80F2EE0100\""
/* 切换到移动（APDU：80 F2 EE 02 00） */
#define BEWINNER_AT_SWITCH_MOBILE  "AT+CSIM= 10,\"80F2EE0200\""
/* 切换到电信（APDU：80 F2 EE 03 00） */
#define BEWINNER_AT_SWITCH_TELECOM "AT+CSIM= 10,\"80F2EE0300\""

/*
 * @brief 北纬 SIM 卡信息结构体
 *
 * 从 AT+CSIM= 10,"80F2EECC3A" 返回的 payload 中解析得到：
 * - simCount：识别到的 ICCID 数量（2 或 3）
 * - iccid[]：从 payload 中按顺序解析出的 ICCID 列表
 * - iccidParamSynced[]：对应 ICCID 是否已成功同步到参数服务，1 表示已同步
 */
typedef struct
{
    int  simCount;                      /* 检测到的 SIM 数量 */
    char iccid[BEWINNER_MAX_SIM_NUM][32]; /* ICCID 列表 */
    uint8_t iccidParamSynced[BEWINNER_MAX_SIM_NUM];
} BeWinnerSimInfo_t;

/*
 * @brief 异步切换请求队列
 * 外部调用者通过 g_async_queue_push 投递切换请求；
 * 工作线程阻塞在 g_async_queue_pop 上处理请求。
 * 使用 GLib 异步队列可避免手写 mutex/cond 的静态初始化兼容问题。
 */
static GAsyncQueue *gSwitchQueue = NULL;

/*
 * @brief 任务就绪标志
 * 工作线程在读到有效卡信息后将其置 1；
 * BeWinnerSimSwitch 在接收请求前会检查该标志。
 */
static gint gTaskReady = 0;

/*
 * @brief 运营商切换顺序表 按照 联通 -> 移动 -> 电信 的顺序循环切换。
 */
static const int gOperatorOrder[BEWINNER_MAX_SIM_NUM] = {
    BEWINNER_OPERATOR_UNICOM,
    BEWINNER_OPERATOR_MOBILE,
    BEWINNER_OPERATOR_TELECOM
};

/*
 * @brief 将运营商 ID 转换为可读字符串
 * @param order 运营商 ID
 * @return 运营商名称字符串；未知返回 "UNKNOWN"
 */
static const char *OperatorOrderToStr(int order)
{
    switch (order)
    {
        case BEWINNER_OPERATOR_UNICOM:
            return "UNICOM";
        case BEWINNER_OPERATOR_MOBILE:
            return "MOBILE";
        case BEWINNER_OPERATOR_TELECOM:
            return "TELECOM";
        default:
            break;
    }

    return "UNKNOWN";
}

/*
 * @brief 判断字符串是否全部由 'F' 组成（不区分大小写）
 * @param s 输入字符串
 * @return 全为 F 返回 1，否则返回 0
 */
static int BeWinnerAllFs(const char *s)
{
    if (s == NULL)
    {
        return 0;
    }

    for (; *s != '\0'; ++s)
    {
        if (*s != 'F' && *s != 'f')
        {
            return 0;
        }
    }
    return 1;
}

/*
 * @brief 判断字符串是否全部由十六进制数字组成
 * @param s 输入字符串
 * @return 全为十六进制数字返回 1，否则返回 0
 */
static int BeWinnerAllHex(const char *s)
{
    if (s == NULL)
    {
        return 0;
    }

    for (; *s != '\0'; ++s)
    {
        if (!isxdigit((unsigned char)*s))
        {
            return 0;
        }
    }
    return 1;
}

/*
 * @brief 去掉 ICCID 末尾的 E/F 等填充字符
 *
 * 北纬卡片在 ICCID 长度不足 20 位时，会在有效数字后填充 E/F 等字符补齐。
 * 本函数从字符串末尾开始删除非十进制数字字符（即 A-F 填充），
 * 保留有效的十进制 ICCID 数字。
 * @param s 输入/输出字符串，原地修改
 */
static void BeWinnerStripIccidPadding(char *s)
{
    size_t len = 0;

    if (s == NULL)
    {
        return;
    }

    len = strlen(s);
    while (len > 0 && !isdigit((unsigned char)s[len - 1]))
    {
        s[len - 1] = '\0';
        len--;
    }
}

/*
 * @brief 去掉响应 payload 首尾的状态码 9000 和填充字符 F
 *
 * 北纬卡返回 "9000" 表示 APDU 执行成功，前后还可能有若干填充 F。
 * 本函数在解析 ICCID 之前将其去除。
 * @param s 输入/输出字符串，原地修改
 */
static void BeWinnerTrim9000AndFs(char *s)
{
    /* 去掉末尾的 APDU 成功状态码 9000 */
    size_t len = 0;
    const char *start = NULL;

    if (s == NULL)
    {
        return;
    }

    len = strlen(s);

    if (len >= 4 && strcmp(s + len - 4, "9000") == 0)
    {
        len -= 4;
        s[len] = '\0';
    }

    /* 再去掉末尾填充字符 F */
    while (len > 0 && (s[len - 1] == 'F' || s[len - 1] == 'f'))
    {
        s[len - 1] = '\0';
        len--;
    }

    /* 去掉前导填充字符 F */
    start = s;
    while (*start == 'F' || *start == 'f')
    {
        start++;
    }

    if (start != s)
    {
        memmove(s, start, len - (size_t)(start - s) + 1);
    }
}

/*
 * @brief 解析 AT+CSIM 读卡信息响应的 payload
 *
 * 三网卡 payload 示例：
 *   89860623300005312493FFFFFFFF898608142623D0003451FFFFFFFF89860323472012115087FFFFFFFFB01000997...
 * 解析策略：
 *   1. 去掉首尾的 "9000" 和填充 F。
 *   2. 按分隔符 "FFFFFFFF" 切分。
 *   3. 对每个非空段先去掉末尾的 E/F 等 ICCID 填充字符。
 *   4. 长度在 BEWINNER_ICCID_LEN_MIN~BEWINNER_ICCID_LEN_MAX 之间且全为十六进制字符的段视为 ICCID。
 *   5. 其他非空段直接忽略。
 *
 * @param payloadRaw 从 +CSIM 响应中提取出的 payload 字符串
 * @param info 输出解析后的 SIM 卡信息
 * @return 至少解析出一个 ICCID 返回 RET_OK；否则返回 RET_ERR
 */
static int BeWinnerParseSimInfo(const char *payloadRaw, BeWinnerSimInfo_t *info)
{
    char payload[512] = {0};
    char token[128] = {0};
    const char *sep = "FFFFFFFF";
    const char *p = NULL;
    const char *next = NULL;
    size_t tlen = 0;
    size_t tlen2 = 0;
    int iccidCount = 0;

    if (payloadRaw == NULL || info == NULL)
    {
        return RET_ERR;
    }

    /* 清空输出结构体 */
    (void)memset(info, 0, sizeof(*info));

    /* 复制 payload 到本地缓冲区 */
    (void)strncpy(payload, payloadRaw, sizeof(payload) - 1);
    payload[sizeof(payload) - 1] = '\0';

    /* 去掉首尾状态码和填充 F */
    BeWinnerTrim9000AndFs(payload);

    /* 按 8 个 F 分隔符循环切分 */
    p = payload;
    while (1)
    {
        next = strstr(p, sep);
        if (next != NULL)
        {
            /* 当前分隔段长度为当前位置到下一个分隔符之间的距离 */
            tlen = (size_t)(next - p);
        }
        else
        {
            /* 最后一段直接取剩余长度 */
            tlen = strlen(p);
        }

        /* 防止 token 缓冲区溢出 */
        if (tlen >= sizeof(token))
        {
            tlen = sizeof(token) - 1;
        }
        (void)memcpy(token, p, tlen);
        token[tlen] = '\0';

        /* 非空且不是全 F 的段才有意义 */
        if (token[0] != '\0' && !BeWinnerAllFs(token))
        {
            /* 去掉 ICCID 末尾的 E/F 等填充字符 */
            BeWinnerStripIccidPadding(token);
            tlen2 = strlen(token);
            /* ICCID 长度在宏定义范围内且全为十六进制字符串 */
            if (tlen2 >= BEWINNER_ICCID_LEN_MIN && tlen2 <= BEWINNER_ICCID_LEN_MAX &&
                BeWinnerAllHex(token) && iccidCount < BEWINNER_MAX_SIM_NUM)
            {
                (void)strncpy(info->iccid[iccidCount], token, sizeof(info->iccid[iccidCount]) - 1);
                info->iccid[iccidCount][sizeof(info->iccid[iccidCount]) - 1] = '\0';
                iccidCount++;
            }
            /* 非 ICCID 段直接忽略 */
        }

        /* 没有更多分隔符则结束 */
        if (next == NULL)
        {
            break;
        }
        /* 跳到分隔符之后继续扫描 */
        p = next + strlen(sep);
    }

    /* 至少解析出一个 ICCID 才认为信息有效 */
    info->simCount = iccidCount;

    return (iccidCount > 0) ? RET_OK : RET_ERR;
}

/*
 * @brief 通过 ql_cmd 执行 AT 指令并获取 +CSIM: 响应行
 *
 * 使用 fork + execvp 启动 ql_cmd 子进程，通过管道读取输出。
 * 整个 AT 指令执行过程受 BEWINNER_AT_CMD_TIMEOUT_MS 超时保护，
 * 超时时会强制结束子进程，避免工作线程永久阻塞。
 *
 * @param atCmd 要执行的 AT 指令字符串
 * @param resp 输出缓冲区，用于保存 +CSIM: 响应行
 * @param respSize 输出缓冲区大小
 * @return RET_OK 找到 +CSIM: 响应；RET_ERR 失败或超时
 */
static int BeWinnerExecAtCmd(const char *atCmd, char *resp, int respSize)
{
    char *argv[4] = {0};
    int pipeFd[2] = {-1, -1};
    pid_t childPid = -1;
    char readBuf[512] = {0};
    char logBuf[512] = {0};
    int totalLen = 0;
    int found = 0;
    int idx = 0;
    struct pollfd pfd = {0};
    struct timespec startTime = {0};
    struct timespec curTime = {0};
    ssize_t readLen = 0;
    int ret = RET_ERR;
    int fdFlags = 0;
    int pollRet = 0;

    do
    {
        if (atCmd == NULL || resp == NULL || respSize <= 0)
        {
            LOG_Error("[%s] invalid parameter", __func__);
            break;
        }

        /* 打印下发的 AT 指令 */
        LOG_Info("[%s] tx AT: %s", __func__, atCmd);

        /* 构造 ql_cmd 参数列表：ql_cmd at <atCmd> */
        argv[0] = "ql_cmd";
        argv[1] = "at";
        argv[2] = (char *)atCmd;
        argv[3] = NULL;

        /* 创建管道 */
        if (pipe(pipeFd) != 0)
        {
            LOG_Error("[%s] pipe failed", __func__);
            break;
        }

        /* 创建子进程 */
        childPid = fork();
        if (childPid < 0)
        {
            LOG_Error("[%s] fork failed", __func__);
            break;
        }

        if (childPid == 0)
        {
            /* 子进程：创建新进程组，关闭读端，重定向 stdout/stderr 到写端 */
            /* setpgid 失败不退出，父进程会回退到按 PID 杀 */
            (void)setpgid(0, 0);
            (void)close(pipeFd[0]);
            (void)dup2(pipeFd[1], STDOUT_FILENO);
            (void)dup2(pipeFd[1], STDERR_FILENO);
            if (pipeFd[1] != STDOUT_FILENO && pipeFd[1] != STDERR_FILENO)
            {
                (void)close(pipeFd[1]);
            }

            /* 执行 ql_cmd */
            (void)execvp("ql_cmd", argv);
            _exit(127);
        }

        /* 父进程：关闭写端 */
        (void)close(pipeFd[1]);
        pipeFd[1] = -1;

        /* 设置读端为非阻塞 */
        fdFlags = fcntl(pipeFd[0], F_GETFL, 0);
        if (fdFlags >= 0)
        {
            (void)fcntl(pipeFd[0], F_SETFL, fdFlags | O_NONBLOCK);
        }

        /* 初始化 poll */
        pfd.fd = pipeFd[0];
        pfd.events = POLLIN;

        /* 记录开始时间 */
        if (clock_gettime(CLOCK_MONOTONIC, &startTime) != 0)
        {
            LOG_Error("[%s] clock_gettime failed", __func__);
            break;
        }

        while (1)
        {
            /* 计算剩余超时时间 */
            int elapsedMs = 0;
            int remainMs = 0;

            if (clock_gettime(CLOCK_MONOTONIC, &curTime) != 0)
            {
                LOG_Error("[%s] clock_gettime failed", __func__);
                break;
            }
            elapsedMs = (int)((curTime.tv_sec - startTime.tv_sec) * 1000 +
                              (curTime.tv_nsec - startTime.tv_nsec) / 1000000);
            remainMs = BEWINNER_AT_CMD_TIMEOUT_MS - elapsedMs;
            if (remainMs <= 0)
            {
                LOG_Error("[%s] AT command timeout", __func__);
                break;
            }

            /* 等待数据，带超时 */
            pollRet = poll(&pfd, 1, remainMs);
            if (pollRet < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                LOG_Error("[%s] poll failed", __func__);
                break;
            }
            if (pollRet == 0)
            {
                LOG_Error("[%s] AT command timeout", __func__);
                break;
            }

            /* 读取数据 */
            readLen = read(pipeFd[0], readBuf + totalLen, sizeof(readBuf) - totalLen - 1);
            if (readLen < 0)
            {
                if (errno == EAGAIN || errno == EINTR)
                {
                    continue;
                }
                LOG_Error("[%s] read failed", __func__);
                break;
            }
            if (readLen == 0)
            {
                /* EOF */
                break;
            }

            totalLen += (int)readLen;
            readBuf[totalLen] = '\0';

            /* 查找 +CSIM: 响应 */
            if (strstr(readBuf, "+CSIM:") != NULL)
            {
                found = 1;
                break;
            }

            /* 缓冲区已满 */
            if (totalLen >= (int)sizeof(readBuf) - 1)
            {
                LOG_Error("[%s] response too long", __func__);
                break;
            }
        }

        /* 如果子进程还在运行，强制结束整个进程组 */
        if (childPid > 0)
        {
            int status = 0;
            if (waitpid(childPid, &status, WNOHANG) == 0)
            {
                /* 优先按进程组杀；若 setpgid 失败则回退到按 PID 杀 */
                if (kill(-childPid, SIGKILL) != 0)
                {
                    (void)kill(childPid, SIGKILL);
                }
                (void)waitpid(childPid, &status, 0);
            }
            childPid = -1;
        }

        /* 打印原始 AT 响应数据；使用 logBuf 并把 \r\n 替换成空格，避免日志换行 */
        (void)memcpy(logBuf, readBuf, (size_t)totalLen);
        for (idx = 0; idx < totalLen; idx++)
        {
            if (logBuf[idx] == '\r' || logBuf[idx] == '\n')
            {
                logBuf[idx] = ' ';
            }
        }
        LOG_Info("[%s] rx raw(len=%d): %s", __func__, totalLen, logBuf);

        if (!found)
        {
            LOG_Error("[%s] no +CSIM response", __func__);
            break;
        }

        /* 从 readBuf 中提取 +CSIM: 所在行到 resp */
        {
            char *lineStart = strstr(readBuf, "+CSIM:");
            char *lineEnd = strchr(lineStart, '\n');
            int lineLen = 0;
            if (lineEnd != NULL)
            {
                lineLen = (int)(lineEnd - lineStart);
            }
            else
            {
                lineLen = (int)strlen(lineStart);
            }
            if (lineLen >= respSize)
            {
                lineLen = respSize - 1;
            }
            (void)memcpy(resp, lineStart, lineLen);
            resp[lineLen] = '\0';
        }

        ret = RET_OK;
    } while (0);

    /* 清理资源 */
    if (pipeFd[0] >= 0)
    {
        (void)close(pipeFd[0]);
    }
    if (pipeFd[1] >= 0)
    {
        (void)close(pipeFd[1]);
    }
    if (childPid > 0)
    {
        int status = 0;
        if (kill(-childPid, SIGKILL) != 0)
        {
            (void)kill(childPid, SIGKILL);
        }
        (void)waitpid(childPid, &status, 0);
    }

    return ret;
}

/*
 * @brief 从 AT 响应字符串中提取 +CSIM: 后面的引号内 payload
 *
 * 例如响应包含：+CSIM:120,"8986...9000"
 * 本函数返回两个双引号之间的十六进制字符串。
 *
 * @param resp 完整的 AT 响应字符串
 * @param payload 输出 payload 缓冲区
 * @param payloadSize payload 缓冲区大小
 * @return 成功返回 payload 指针；失败返回 NULL
 */
static const char *BeWinnerExtractCsimPayload(const char *resp, char *payload, int payloadSize)
{
    const char *csim = NULL;
    const char *q1 = NULL;
    const char *q2 = NULL;
    const char *result = NULL;
    int len = 0;

    do
    {
        if (resp == NULL || payload == NULL || payloadSize <= 0)
        {
            LOG_Error("[%s] invalid parameter", __func__);
            break;
        }

        /* 定位 +CSIM: 关键字 */
        csim = strstr(resp, "+CSIM:");
        if (csim == NULL)
        {
            LOG_Error("[%s] no +CSIM in response", __func__);
            break;
        }

        /* 第一个双引号 */
        q1 = strchr(csim, '"');
        if (q1 == NULL)
        {
            LOG_Error("[%s] no opening quote", __func__);
            break;
        }

        /* 第二个双引号 */
        q2 = strchr(q1 + 1, '"');
        if (q2 == NULL)
        {
            LOG_Error("[%s] no closing quote", __func__);
            break;
        }

        /* 两个引号之间的内容就是 payload */
        len = (int)(q2 - q1 - 1);
        if (len <= 0)
        {
            LOG_Error("[%s] empty or invalid payload", __func__);
            break;
        }
        if (len >= payloadSize)
        {
            len = payloadSize - 1;
        }

        (void)memcpy(payload, q1 + 1, len);
        payload[len] = '\0';
        result = payload;
    } while (0);

    return result;
}

/*
 * @brief 读取北纬多网卡信息并解析
 *
 * 发送 AT+CSIM= 10,"80F2EECC3A"，从 +CSIM 响应中提取 payload，
 * 再调用 BeWinnerParseSimInfo 得到 simCount / iccid[]。
 *
 * @param info 输出解析后的 SIM 卡信息
 * @return RET_OK 成功；RET_ERR 失败
 */
static int BeWinnerSimReadInfo(BeWinnerSimInfo_t *info)
{
    char resp[512] = {0};
    char payload[512] = {0};
    int ret = RET_ERR;

    do
    {
        if (info == NULL)
        {
            LOG_Error("[%s] invalid parameter", __func__);
            break;
        }

        /* 通过 ql_cmd 下发读卡信息 AT 指令 */
        if (BeWinnerExecAtCmd(BEWINNER_AT_READ_INFO, resp, sizeof(resp)) != RET_OK)
        {
            LOG_Error("[%s] exec read info cmd failed", __func__);
            break;
        }

        /* 提取 +CSIM: 后的 payload */
        if (BeWinnerExtractCsimPayload(resp, payload, sizeof(payload)) == NULL)
        {
            LOG_Error("[%s] extract payload failed, resp=%s", __func__, resp);
            break;
        }

        /* 解析 payload */
        if (BeWinnerParseSimInfo(payload, info) != RET_OK)
        {
            LOG_Error("[%s] parse sim info failed, payload=%s", __func__, payload);
            break;
        }

        /* 打印解析结果 */
        LOG_Info("[%s] simCount=%d, iccid[0]=%s, iccid[1]=%s, iccid[2]=%s",
                 __func__, info->simCount,
                 info->iccid[0], info->iccid[1], info->iccid[2]);

        ret = RET_OK;
    } while (0);

    return ret;
}

/*
 * @brief 执行实际的运营商切换
 *
 * 根据 operatorId 选择对应的 AT+CSIM APDU 指令，通过 ql_cmd 下发，
 * 并检查响应中是否包含成功标志 "9000" 或 "91xx"。
 *
 * @param operatorId 运营商 ID：1 联通 / 2 移动 / 3 电信
 * @return RET_OK 切换成功；RET_ERR 失败
 */
static int BeWinnerDoSwitch(int operatorId)
{
    const char *atCmd = NULL;
    char resp[256] = {0};
    char payload[64] = {0};
    const char *payloadStr = NULL;
    size_t payloadLen = 0;
    const char *sw = NULL;
    int ret = RET_ERR;

    do
    {
        /* 根据运营商选择 APDU 指令 */
        switch (operatorId)
        {
        case BEWINNER_OPERATOR_UNICOM:
            atCmd = BEWINNER_AT_SWITCH_UNICOM;
            break;
        case BEWINNER_OPERATOR_MOBILE:
            atCmd = BEWINNER_AT_SWITCH_MOBILE;
            break;
        case BEWINNER_OPERATOR_TELECOM:
            atCmd = BEWINNER_AT_SWITCH_TELECOM;
            break;
        default:
            LOG_Error("[%s] invalid operatorId=%s", __func__, OperatorOrderToStr(operatorId));
            break;
        }

        if (atCmd == NULL)
        {
            break;
        }

        /* 通过 ql_cmd 下发切换指令 */
        if (BeWinnerExecAtCmd(atCmd, resp, sizeof(resp)) != RET_OK)
        {
            LOG_Error("[%s] exec switch cmd failed, operatorId=%s", __func__, OperatorOrderToStr(operatorId));
            break;
        }

        /* 提取 +CSIM: 后的 payload，判断末尾状态字 */
        payloadStr = BeWinnerExtractCsimPayload(resp, payload, sizeof(payload));
        if (payloadStr == NULL)
        {
            LOG_Error("[%s] extract payload failed, resp=%s", __func__, resp);
            break;
        }

        payloadLen = strlen(payloadStr);
        if (payloadLen < 4)
        {
            LOG_Error("[%s] payload too short, payload=%s", __func__, payloadStr);
            break;
        }

        /* APDU 响应最后 4 位为状态字 */
        sw = payloadStr + payloadLen - 4;

        /* 判断切换是否成功：9000 或 91xx 均为成功响应 */
        if (strcmp(sw, "9000") == 0)
        {
            LOG_Info("[%s] switch to operator %s success", __func__, OperatorOrderToStr(operatorId));
            ret = RET_OK;
            break;
        }

        if (sw[0] == '9' && sw[1] == '1' &&
            isxdigit((unsigned char)sw[2]) && isxdigit((unsigned char)sw[3]))
        {
            LOG_Info("[%s] switch to operator %s success with SW %s", __func__, OperatorOrderToStr(operatorId), sw);
            ret = RET_OK;
            break;
        }

        LOG_Error("[%s] switch to operator %s failed, resp=%s", __func__, OperatorOrderToStr(operatorId), resp);
    } while (0);

    return ret;
}

/*
 * @brief 根据切换顺序计算下一个运营商 ID
 *
 * 以 currentOperatorId 为基准，返回切换顺序表中的下一个运营商。
 * 若 currentOperatorId 不在顺序表中，则返回第一个（联通）。
 *
 * @param currentOperatorId 当前运营商 ID
 * @return 下一个运营商 ID
 */
static int BeWinnerSimGetNextOperator(int currentOperatorId)
{
    int i = 0;
    int idx = 0;

    for (i = 0; i < BEWINNER_MAX_SIM_NUM; i++)
    {
        if (gOperatorOrder[i] == currentOperatorId)
        {
            idx = (i + 1) % BEWINNER_MAX_SIM_NUM;
            return gOperatorOrder[idx];
        }
    }

    /* 当前运营商不在顺序表中，默认返回第一个（联通） */
    return gOperatorOrder[0];
}

/*
 * @brief 获取当前运营商 ID
 *
 * 通过 ModemSrvGetOperator 获取 MCC+MNC 字符串；
 *
 * @return 当前运营商 ID
 */
static int BeWinnerGetCurrentOperator(void)
{
    uint8_t operatorStr[64] = {0};
    uint32_t operatorValue = 0;
    int operatorId = 0;

    /*  Modem 运营商 */
    if (ModemSrvGetOperator(operatorStr) == RET_OK)
    {
        LOG_Info("[%s] operator from modem: %s", __func__, operatorStr);
        operatorValue = (uint32_t)atoi((const char *)operatorStr);
        switch (operatorValue)
        {
        /* 中国联通 */
        case 46001:
        case 46006:
        case 46009:
            operatorId = BEWINNER_OPERATOR_UNICOM;
            break;
        /* 中国移动 */
        case 46000:
        case 46002:
        case 46004:
        case 46007:
        case 46008:
        case 46013:
            operatorId = BEWINNER_OPERATOR_MOBILE;
            break;
        /* 中国电信 */
        case 46003:
        case 46005:
        case 46011:
            operatorId = BEWINNER_OPERATOR_TELECOM;
            break;

        default:
            operatorId = 0;
            LOG_Warn("[%s] unknown operator value %u", __func__, operatorValue);
            break;
        }
    }

    return operatorId;
}

/*
 * @brief 北纬多网卡切换工作线程入口
 *
 * 线程生命周期：
 *   1. 读取并解析 SIM 卡信息；成功后置 gTaskReady=1。
 *   2. 阻塞在 g_async_queue_pop 上等待切换请求。
 *   3. 收到请求后执行切换
 * @param data 线程参数（当前未使用，保留以兼容 GLib 线程签名）
 * @return 线程退出时返回 NULL
 */
static void *BeWinnerSimSwitchTaskThreadFunc(void *data)
{
    BeWinnerSimInfo_t beWinnerSimInfo = {0};
    int simCardIccidLoaded = 0;
    int operatorIdLoadCount = 0;
    int currentOperatorId = 0;
    int operatorId = 0;
    int i = 0;
    gpointer req = NULL;
    (void)data;

    while (1)
    {
        /** 初始化 加载 sim卡 iccid **/
        if (simCardIccidLoaded == 0)
        {
            if (BeWinnerSimReadInfo(&beWinnerSimInfo) != RET_OK)
            {
                LOG_Warn("[%s] load SIM info failed, continue ...", __func__);
                sleep(5);
                continue;
            }
            else
            {
                /** 更新参数服务北纬SIM卡ICCID **/
                for (i = 0; i < beWinnerSimInfo.simCount; i ++)
                {
                    if(UpdateParam(E_ParamId_enumTonlyBeWinnerICCID0 + i,
                        (uint8_t *)beWinnerSimInfo.iccid[i],
                        (uint16_t)strlen(beWinnerSimInfo.iccid[i])) == 0)
                    {
                        beWinnerSimInfo.iccidParamSynced[i] = 1;
                        LOG_Warn("[%s] sync iccid[%d]=%s to param success", __func__, i, beWinnerSimInfo.iccid[i]);
                    }
                    else
                    {
                        LOG_Error("[%s] sync iccid[%d]=%s to param failed", __func__, i, beWinnerSimInfo.iccid[i]);
                    }
                }
                simCardIccidLoaded = 1;
            }
        }

        /** 初始化 获取当前运营商 **/
        if (currentOperatorId == 0)
        {
            currentOperatorId = BeWinnerGetCurrentOperator();
            if (currentOperatorId == 0)
            {
                /** 加载网络运营商失败 **/
                LOG_Warn("[%s] load net register failed, continue ...", __func__);
                operatorIdLoadCount++;
                if (operatorIdLoadCount >= 5)
                {
                    /** 初始化使用默认运营商 **/
                    currentOperatorId = BEWINNER_OPERATOR_UNICOM;
                }
                sleep(5);
                continue;
            }
        }

        /** 周期检查SIM ICCID是否已经同步到参数服务 **/
        for (i = 0; i < beWinnerSimInfo.simCount; i++)
        {
            if (beWinnerSimInfo.iccidParamSynced[i] == 0)
            {
                if(UpdateParam(E_ParamId_enumTonlyBeWinnerICCID0 + i,
                    (uint8_t *) beWinnerSimInfo.iccid[i],
                    (uint16_t) strlen(beWinnerSimInfo.iccid[i])) == 0)
                {
                    beWinnerSimInfo.iccidParamSynced[i] = 1;
                    LOG_Warn("[%s] sync iccid[%d]=%s to param success", __func__, i, beWinnerSimInfo.iccid[i]);
                }
                else
                {
                    LOG_Error("[%s] sync iccid[%d]=%s to param failed", __func__, i, beWinnerSimInfo.iccid[i]);
                }
            }
        }

        /** 配置全局初始化完成状态**/
        if (!g_atomic_int_get(&gTaskReady)) {
            g_atomic_int_set(&gTaskReady, 1);
        }
    
        /* 等待切换请求，5 秒超时；超时则继续循环，执行周期检查 */
        req = g_async_queue_timeout_pop(gSwitchQueue, 5000000);
        if (req == NULL)
        {
            continue;
        }

        /* 按顺序自动切换到下一个运营商 */
        operatorId = BeWinnerSimGetNextOperator(currentOperatorId);

        LOG_Info("[%s] switch to operator: %s", __func__, OperatorOrderToStr(operatorId));
        /* 执行切换 */
        if (BeWinnerDoSwitch(operatorId) == RET_OK)
        {
            /* 切换成功后更新当前运营商 */
            currentOperatorId = operatorId;
        }
    }

    return NULL;
}

/*
 * @brief 启动北纬多网卡切换后台任务
 * 创建异步请求队列和 GLib 工作线程，线程内部通过 ql_cmd 直接读取卡信息。
 * @return RET_OK 成功启动；RET_ERR 失败
 */
int BeWinnerSimTaskStart(void)
{
    GThread *thread = NULL;
    int ret = RET_ERR;

    /* 防止重复启动 */
    if (gSwitchQueue != NULL)
    {
        LOG_Warn("[%s] task already started", __func__);
        return RET_OK;
    }

    do {
        /* 创建异步请求队列 */
        gSwitchQueue = g_async_queue_new();
        if (gSwitchQueue == NULL)
        {
            LOG_Error("[%s] create async queue failed", __func__);
            break;
        }

        /* 初始状态为未就绪 */
        g_atomic_int_set(&gTaskReady, 0);

        /* 创建工作线程 */
        thread = g_thread_new("BeWinnerSimSwitchTask", BeWinnerSimSwitchTaskThreadFunc, NULL);
        if (thread == NULL)
        {
            LOG_Error("[%s] create thread failed", __func__);
            break;
        }

        /* 不保留线程句柄，让线程自行运行 */
        g_thread_unref(thread);
        LOG_Info("[%s] task started", __func__);
        ret = RET_OK;
    } while (0);

    /* 创建线程失败时回滚队列 */
    if (ret != RET_OK && gSwitchQueue != NULL)
    {
        g_async_queue_unref(gSwitchQueue);
        gSwitchQueue = NULL;
    }

    return ret;
}

/*
 * @brief 请求切换到下一个运营商
 *
 * 按照 联通 -> 移动 -> 电信 -> 联通 的顺序自动切换下一个运营商。
 * 本接口为异步接口：把切换请求投递到任务队列后立即返回，
 * 实际切换动作由后台线程通过 ql_cmd 执行。
 *
 * @return RET_OK 请求成功投递；RET_ERR 失败
 */
int BeWinnerSimSwitch(void)
{
    int ret = RET_ERR;

    do {
        /* 任务是否已启动 */
        if (gSwitchQueue == NULL)
        {
            LOG_Error("[%s] task not started", __func__);
            break;
        }

        /* 必须等读到卡信息后才能切换 */
        if (!g_atomic_int_get(&gTaskReady))
        {
            LOG_Error("[%s] task not ready", __func__);
            break;
        }

        /*  按顺序切换到下一个运营商 */
        g_async_queue_push(gSwitchQueue, GINT_TO_POINTER(1));
        LOG_Info("[%s] request switch to next operator", __func__);
        ret = RET_OK;
    } while (0);

    return ret;
}
