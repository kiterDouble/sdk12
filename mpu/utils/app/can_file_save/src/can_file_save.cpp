
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <thread>
#include <dirent.h>
#include "ethernet_interface.h"
#include "file_interface.h"
#include "thread_interface.h"
#include "sr_log.h"
#include "can_file_save.h"
#include "system_interface.h"
#include "queue_msg.h"
#include "completion_timeout.h"

static pthread_t canfile_threadId;
static pthread_t tarfile_threadId;

int ThreadInit()
{
    int ret;
    ret = CreateThread(&canfile_threadId, CanFileHander, NULL);
    if(ret < 0)
    {
        LOG_Error("Create ReceiveFunction Thread Fail");
    }

    ret = CreateThread(&tarfile_threadId, TarFileHander, NULL);
    if(ret < 0)
    {
        LOG_Error("Create ReceiveFunction Thread Fail");
        return -1;
    }
    return 0;
}

void DeInit()
{
    pthread_join(canfile_threadId, NULL);
   
}





int main(int argc, char *argv[])
{
    int ret = 0;
    
    LOG_INIT("can_file_save");
   
    ret = CheckTimeSync();
    if(ret < 0)
    {
        LOG_Error("CheckTimeSync fail");
    }

    InitCompletion(GetComp());
    ret = TarGzCompress(NULL);
    if(ret < 0)
    {
        LOG_Error(" Tar Gz Compress Fail");
    }
   
    ret = ThreadInit();
    if(ret < 0)
    {
        LOG_Error("Thread Init Fail");
    }

    EthernetInit();
    
}






