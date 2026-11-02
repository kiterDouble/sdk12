#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <thread>
#include "ethernet_interface.h"
#include <vector>
#include "file_interface.h"
#include "thread_interface.h"
#include "sr_log.h"
#include "can_file_save.h"
#include "system_interface.h"
#include "queue_msg.h"
#include "completion_timeout.h"
#include "sirunMdlSafety.h"
#include "kfifo.h"
#include "can_submodule.h"
static pthread_t canfile_threadId;
static pthread_t tarfile_threadId;

int ThreadInit()
{
    int ret;
    ret = CreateThread(&canfile_threadId, CanFileHandler, NULL);
    if (ret < 0)
    {
        LOG_Error("Create ReceiveFunction Thread Fail");
    }

    ret = CreateThread(&tarfile_threadId, TarFileHander, NULL);
    if (ret < 0)
    {
        LOG_Error("Create ReceiveFunction Thread Fail");
        return -1;
    }

    return 0;
}

void DeInit()
{
    pthread_join(canfile_threadId, NULL);
    pthread_join(tarfile_threadId, NULL);
}

int DetectModemType(void)
{
    int mode = 0;
    if (access("/dev/mmcblk0", F_OK) != 0)
    {
        mode = SLAVE_MODEM;
        LOG_Debug("Current modem: SLAVE_MODEM (%d)\n", mode);
    }
    else
    {
        mode = MAIN_MODEM;
        LOG_Debug("Current modem: MAIN_MODEM (%d)\n", mode);
    }

    return mode;
}

void MainModemMain(void)
{
    AppContext_t *ctx = GetAppContext();
    int ret = 0;
    MainModemInit(&(ctx->time_range.start_time));
    InitCompletion(GetComp());
    ret = TarGzCompress(NULL);
    if (ret < 0)
    {
        LOG_Error(" Tar Gz Compress Fail");
    }
    
    ret = ThreadInit();
    if (ret < 0)
    {
        LOG_Error("Thread Init Fail");
    }
    EthernetInit();
    return;
}

void SlaveModemMain(void)
{

    pthread_t serid;
    CreateDetachedThread(&serid, TcpServerThread, NULL);
    SafeQueue queue;
    std::vector<std::thread> threads;
    threads.emplace_back(handleServer, std::ref(queue));
    threads.emplace_back(SubmoduleHandleQueue, std::ref(queue));

    // 等待所有线程完成
    for (auto &t : threads)
    {
        t.join();
    }
    return;
}

int main(int argc, char *argv[])
{

    LOG_INIT("can_file_save");
    LOG_Debug("can_file_save start");
    int ret = 0;
    ret = AppContextInit();
    if (ret != 0)
    {
        LOG_Error("AppContextInit Fail");
        return ret;
    }
    AppContext_t *ctx = GetAppContext();
    ctx->modem_type = DetectModemType();

    if (ctx->modem_type == MAIN_MODEM)
    {
       
       // const char *tmp = "/mnt/canlog/";
       // SirunSetCoreDumpInner((int8_t *)tmp);
        SirunSignalCatchInit();
        MainModemMain();
    }
    else
    {
        SlaveModemMain();
    }

    AppContextDeinit();
    return 0;
}
