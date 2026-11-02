/*************************************************************************
    > File Name: task_exc_handle.c
    > Author: zhangkh 
    > Created Time: 2018/2/1
    > Descripton: 
    > Version     time     person  contest 
    > 01                   zhangkh    
 ************************************************************************/
#include <ucontext.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <glib.h>
#include "err_code.h"
#include "task_exc_handle.h"
#include "sr_log.h"

/*define*/
#define TASK_SIGNAL_SUSPEND (SIGRTMIN + 2)
#define TASK_SIGNAL_RESUME (SIGRTMIN + 3)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* See feature_test_macros(7) */
#endif

/*
 * descripton: //used for doing sth.
*/

gint GetTracebackInfo(gint gintOsTaskId)
{
    gint *pgintTraceLevel[50];
    gint gintTraceLevelNum = 0;
    gint8 **pgint8CallInfo;
    guint32 i = 0;

    /* 如果是本任务异常，则直接解析本任务 */

    gintTraceLevelNum = backtrace((void **)pgintTraceLevel, 50);
    LOG_Debug("gintTraceLevelNum = %d", gintTraceLevelNum);

    if (gintTraceLevelNum > 0)
    {
        pgint8CallInfo = (gint8 **)backtrace_symbols((void **)pgintTraceLevel, gintTraceLevelNum);

        if (NULL != (gint8 **)pgint8CallInfo)
        {
            for (i = 0; i < (guint32)gintTraceLevelNum; i++)
            {
                LOG_Debug("%s", pgint8CallInfo[i]);
            }

            free((void *)pgint8CallInfo);
            return 0;
        }
    }
    return -1;
}

/*
 * descripton: //used for doing sth.
*/
void PthreadExcMonitor(gint gintosTaskId, gint gintExcVec, gint gintPcReg, gint gintDarReg)
{
    sigset_t SigSet;
    /* 信号*/
    gint gintSig = 0;

    LOG_Error("---------Pthread %d exception ---------", gintosTaskId);
    (void)GetTracebackInfo(gintosTaskId);
    exit(0);

    // (void)sigemptyset(&SigSet);
    // (void)sigaddset(&SigSet, TASK_SIGNAL_RESUME);
    // (void)sigwait(&SigSet, &gintSig);
}

/*
 * descripton: //used for doing sth.
*/
void ExcHookFunc(gint gintSigNo, siginfo_t *pSigInfo, void *pData)
{
    gint gintpc = 0;
    gint gintdar = 0;
    /* OS任务ID */
    gint gintOSTaskId;
    ucontext_t *pContext = (ucontext_t *)NULL;
    LOG_Debug("Get SIG:%d---PThread ID[%lu]", gintSigNo, pthread_self());
#if 0
    gintpc = (gint)((ucontext_t *)(gint)pData)->uc_mcontext.regs->nip;
    gintdar = (gint)((ucontext_t *)(gint)pData)->uc_mcontext.regs->dar;
#endif

    /* 获取OS任务ID(TId) */
    gintOSTaskId = (gint)syscall(SYS_gettid);
    PthreadExcMonitor(gintOSTaskId, gintSigNo, gintpc, gintdar);
}

/*
 * descripton: //used for doing sth.
*/
gint ExceptionHookInit(void)
{
    gint ret = 0;
    struct sigaction strSigAction;
    strSigAction.sa_sigaction = ExcHookFunc;
    strSigAction.sa_flags = SA_SIGINFO;
    ret = sigemptyset(&strSigAction.sa_mask);
    if (0 != ret)
    {
        return RET_ERR;
    }

    ret = sigaction(SIGSEGV, &strSigAction, NULL);
    if (0 != ret)
    {
        return RET_ERR;
    }

    ret = sigaction(SIGPIPE, &strSigAction, NULL);
    if (0 != ret)
    {
        return RET_ERR;
    }

    ret = sigaction(SIGILL, &strSigAction, NULL);
    if (0 != ret)
    {
        return RET_ERR;
    }

    ret = sigaction(SIGQUIT, &strSigAction, NULL);
    if (0 != ret)
    {
        return RET_ERR;
    }

    ret = sigaction(SIGFPE, &strSigAction, NULL);
    if (0 != ret)
    {
        return RET_ERR;
    }

    ret = sigaction(SIGBUS, &strSigAction, NULL);
    if (0 != ret)
    {
        return RET_ERR;
    }

    ret = sigaction(SIGIOT, &strSigAction, NULL);
    if (0 != ret)
    {
        return RET_ERR;
    }

    return RET_OK;
}
/*******************************end**************************************/
