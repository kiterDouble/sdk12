#include <sys/resource.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <execinfo.h>
#include <stdlib.h>
#include "sr_log.h"
#include "sirunMdlSafety.h"

/**
 * @brief  配置系统 coredump 相关参数
 *
 * @details
 *  该接口用于设置系统的 core 文件生成行为，包括：
 *  1) 解除 core 文件大小限制（setrlimit）
 *  2) 设置是否在 core 文件名中附带 PID
 *  3) 配置 core 文件名称格式（/proc/sys/kernel/core_pattern）
 *
 * @param[in]  pCoreID     是否启用 PID 作为 core 文件名后缀 ("1" 启用, "0" 禁用)
 * @param[in]  pCorePath   core 文件生成路径及名称格式，例如 "/data/core-%e-%p-%t"
 *
 * @return 0 表示成功，其他值表示失败
 */
int32_t SirunCoreDumpOpen(int8_t *pCoreID, int8_t *pCorePath)
{
    if (!pCoreID || !pCorePath) {
        LOG_Error("[MDL_SAFETY]Invalid parameter [%p,%p]\n",pCoreID,pCorePath);
        return -1;
    }
    int32_t ret     = -1;
    int32_t iFd1    = -1;
    int32_t iFd2    = -1;
    struct  rlimit  limit       = {0};
    struct  rlimit  limit_set   = {0};

    do {
        // 1. 配置 core ulimit
        if (getrlimit(RLIMIT_CORE, &limit)) {
            LOG_Error("[MDL_SAFETY]get resource limit fail\n");
            break;
        }

        limit_set.rlim_cur = limit_set.rlim_max = RLIM_INFINITY;
        if (setrlimit(RLIMIT_CORE, &limit_set)) {
            limit_set.rlim_cur = limit_set.rlim_max = limit.rlim_max;
            if (limit.rlim_max != RLIM_INFINITY) {
                LOG_Error("[MDL_SAFETY]CORE: cur=0x%lx, max=0x%lx\n", limit.rlim_cur, limit.rlim_max);
            }
            if (setrlimit(RLIMIT_CORE, &limit_set)) {
                LOG_Error("[MDL_SAFETY]set core ulimit fail\n");
                break;
            }
        }

        // 2.配置 core use pid
        if (pCoreID && strlen((char*)pCoreID) > 0) {
            iFd1 = open("/proc/sys/kernel/core_uses_pid", O_RDWR|O_NDELAY|O_TRUNC, 0644);
            if (iFd1 < 0) {
                LOG_Error("[MDL_SAFETY]open core_uses_pid fail\n");
                break;
            }
            if (strlen((char*)pCoreID) != (size_t)write(iFd1, pCoreID, strlen((char*)pCoreID))) {
                LOG_Error("[MDL_SAFETY]set core_uses_pid fail\n");
                break;
            }
        }

        // 3.配置 core pattern
        if (pCorePath && strlen((char*)pCorePath) > 0) {
            iFd2 = open("/proc/sys/kernel/core_pattern", O_RDWR|O_NDELAY|O_TRUNC, 0644);
            if (iFd2 < 0) {
                LOG_Error("[MDL_SAFETY]open core_pattern fail\n");
                break;
            }
            if (strlen((char*)pCorePath) != (size_t)write(iFd2, pCorePath, strlen((char*)pCorePath))) {
                LOG_Error("[MDL_SAFETY]set core_pattern fail\n");
                break;
            }
        }

        // 4.set core dump open success
        ret = 0;
        LOG_Info("[MDL_SAFETY]set core dump open success!\n");
    } while(0);

    if(iFd1 >= 0){
        close(iFd1);
    }
    if (iFd2 >= 0){
        close(iFd2);
    }

    return ret;
}

/**
 * @brief 信号异常回调函数，用于捕获崩溃信号并输出调用栈
 *
 * @param[in] signal_num   捕获到的信号编号
 *
 * @return 无返回值
 *
 */
static void SirunSignalCatchCallback(int32_t signal_num)
{
	int i = 0;
    void	*pbuf[100] = {0};
    char    cmd[64] = {0};
    int32_t	addr_num = backtrace(pbuf, 100);
    char	**pstr_symbols = backtrace_symbols(pbuf, addr_num);

	LOG_Info("[MDL_SAFETY]PID %d CATCH SIGNAL:%d \n", getpid(),signal_num);
	LOG_Info("[MDL_SAFETY]BACKTRACE: NUMBER OF ADDRESSES IS:%d \n", addr_num);
	
    if (pstr_symbols != NULL) {

        // 将崩溃栈信息打印至斯润log    
        for (i = 0; i < addr_num; i++) {
            LOG_Error("%03d %s", addr_num-i, pstr_symbols[i]);
        }
        // 将崩溃栈信息再打印至串口，防止日志接口截断过长的栈信息
        for (i = 0; i < addr_num; i++) {   
            printf("%03d %s\n", addr_num-i, pstr_symbols[i]);
        }
        printf("\n");
        // sprintf(cmd,"cat /proc/%d/maps",getpid());
        // LOG_Info("%s\n",cmd);
        // system(cmd);

        free(pstr_symbols);
        pstr_symbols = NULL;
    } else {
        LOG_Info("[MDL_SAFETY]BACKTRACE: CANNOT GET BACKTRACE SYMBOLS\n");
    }
}

/**
 * @brief 初始化异常信号捕获机制
 *
 * @param 无
 *
 * @return 0 – 成功，其他 – 失败
 *
 */
extern int32_t SirunSignalCatchInit(void)
{
    int32_t ret = 0;
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset( &sa.sa_mask );
    
    sa.sa_handler = SirunSignalCatchCallback;
    sa.sa_flags   = SA_SIGINFO | SA_RESETHAND;
    // sigaction( SIGSEGV, &sa, NULL );
    /*
    广汽默认监控以下信号，可在具体项目中修改。
    一、SIGSEGV：“段错误（Segmentation Fault）”	非法访问存储器，用于获取段错误时的函数栈信息
    典型原因：
    1、访问空指针或悬挂指针 (*NULL, *(0xdeadbeef));
    2、栈溢出（递归太深或栈变量过大）;
    3、写只读内存；
    4、野指针/越界访问（数组越界）。

    二、SIGFPE：“浮点或算术异常（Floating Point Exception）” 算术运算出错，如除数为 0 或溢出（不一定是浮点运算）
    典型原因：
    1、整数除以 0 (x/0 或 %0)
    2、浮点 NaN/Overflow 异常（硬件浮点触发）
    3、一些硬件 DSP 指令异常。

    三、SIGABRT：“异常终止（Abort）” 程序自己调用 abort() 或 assert() 触发
    1、assert() 失败；
    2、glibc 检测到严重错误（如 malloc/free 崩溃、std::terminate()）；
    3、程序手动调用 abort()。
    */
    int sigs[] = { SIGSEGV, SIGABRT, SIGFPE };
    for (size_t i = 0; i < sizeof(sigs)/sizeof(sigs[0]); ++i) {
        if (sigaction(sigs[i], &sa, NULL) != 0) {
            LOG_Error("[MDL_SAFETY]Fail to register signal %d\n", sigs[i]);
            ret = -1; /* 记录失败，继续注册其它信号 */
        }
    }
    if (ret == 0) {
        LOG_Info("[MDL_SAFETY]SirunSignalCatchInit success !!!\n");
    } else {
        LOG_Error("[MDL_SAFETY]SirunSignalCatchInit failed !!!\n");
    }
	return ret;
}

/**
 * @brief 设置存放coredump文件路径信息(调用后将会在指定路径生成coredump文件)
 *        注意！！！最好只在调试时候使用，避免进程挂死后，再被拉起，重复生成过多coredump文件，导致emmc空间被占满影响业务逻辑。
 *
 * @param[in] ppath: 文件路径
 *
 * @return 0 – 成功，其他 – 失败
 * 
 */
int32_t SirunSetCoreDumpInner(const int8_t *ppath)
{
    if (!ppath) {
        LOG_Error("[MDL_SAFETY]Invalid parameter [%p]\n",ppath);
        return -1;
    }
    char szCorePid[32] = {0};
    char szCorePath[128] = {"dsp_core_dum"};
    if (access((char*)ppath, 6) != 0) {
        LOG_Error("%s open error\n", ppath);
        return -1;
    }
    strcpy(szCorePid, "1");
    strcpy(szCorePath, (char*)ppath);
    strcat(szCorePath, "dspCore-%s-%e-%p-%t");

    if (0 != SirunCoreDumpOpen((int8_t*)szCorePid, (int8_t*)szCorePath)){
        LOG_Error("=============[MDL_SAFETY]gdb core dump open fail!=================\n");
        return -2;
    } else {
        LOG_Info("=============[MDL_SAFETY]gdb core dump open success!==============\n");
        return 0;
    }
}