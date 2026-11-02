#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "sr_ipc.h"
#include "sr_sdk.h"
#include "https.h"
#include "sr_log.h"
#include "time_status_api.h"
#include "time_status.h"
#include "https_interface.h"
#include "system_interface.h"
#include "thread_interface.h"
#include "sr_engine.h"
#include "file_interface.h"
#include "certificate_interface.h"
#include <openssl/engine.h>
#include <openssl/ssl.h>
#include <openssl/engine.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include "https_interface.h"
#include "pm_server_api.h"

static int g_PmState = PM_STATE_NORMAL;
void HandlePmSleepCmd(const uint8_t *data, const uint16_t len, void *userdata)
{

    MPUPmStateChange_t PmState = {0};
    LOG_DebugHex(data,len,"HandlePmSleepCmd");
    if(data ==NULL)
    {
        LOG_Error("data NULL");
        return;
    }
    memset(&PmState, 0x00, sizeof(MPUPmStateChange_t));
    if (sizeof(MPUPmStateChange_t) == len)
    {
        memmove(&PmState, data, len);
    }
    g_PmState = PmState.PmState;
    
    LOG_Debug("pm status pmstate:%d  PmState.PmState:%d ",g_PmState,PmState.PmState);
   
}


int GetPmState()
{
    LOG_Debug("PmState = %d",g_PmState);
    return g_PmState;
}

void * ReceiveFunction(void *arg)
{
    int ret = 0;
    while(1)
    {
        if((GetPmState() == PM_STATE_SLEEP) && (PmGetWakeupSrc() != PM_WAKEUP_SOURCE_TIMER_HEARTBEAT))
        {
            sleep(1);
            continue;
        }

        if(access(FILE_CHECK,F_OK) == 0)
        {
            ret = CertificateOfVehicleNumberChangeApplication();
            if(ret == 1)
            {
                ret  = CertificateApplicationInterface();
                if(ret != 0)
                {
                    LOG_Error("Certificate Application fail");
                }
            }

            ret = VerifyCertificateChain(DEVICECERTFILE, DEVICECACERTCHAINFILE);
            if((ret == 0))
            {
                if (remove(FILE_CHECK) == 0) 
                {
                    LOG_Error("%s deleted  successfully.",FILE_CHECK);
                } else {
                    LOG_Error("Error deleting file %s",FILE_CHECK);
                }

                continue;
            }

            if (ret != 0)
            {
                ret  = CertificateApplicationInterface();
                if(ret != 0)
                {
                    LOG_Error("Certificate Application fail");
                    
                }else{
                    continue;
                }
                //LOG_Error("Certificate Inspection And Application Fail ");
                for(;;)
                {
                    LOG_Error("Certificate Application fail... ");
                    sleep(300);
                    LOG_Error("Certificate Application start ");
                    ret = CertificateInspectionAndApplication();
                    if(ret == 0 )
                    {
                        LOG_Error("Certificate Inspection ok ");
                        break;
                    }else{

                        LOG_Error("Certificate Inspection Fail ");
                    }
                }
            }

            if (remove(FILE_CHECK) == 0) 
            {
                LOG_Error("%s deleted  successfully.",FILE_CHECK);
            } else {
                LOG_Error("Error deleting file %s",FILE_CHECK);
            }

        }
        
        usleep(1000000);
    }

}

void ThreadInit()
{
    int ret;
    pthread_t cert_threadId;
    ret = CreateThread(&cert_threadId, ReceiveFunction, NULL);
    if(ret < 0)
    {
        LOG_Error("Create Receive Thread Fail \n");
    }
}



int main()
{
    int ret = 0;
    LOG_INIT("pki_cert");
    InitSRSdk();
    StartQueueMsgRcvThread("pki_cert");


    ret = SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, HandlePmSleepCmd, NULL);
    if(ret != 0)
    {
        LOG_Error("SubscriptionQueueMsg SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY Fail ");
    }

    ret = CheckTimeSync();
    if(ret < 0 )
    {
        LOG_Error("CheckTimeSync Fail ");
    }

    if(access(KEYFILEFLAG,F_OK) != 0)
    {
        //3次
        for (size_t i = 0; i < 3; i++)
        {
            ret = CAPrivateKeyImport();
            if(ret < 0)
            {
                LOG_Error("CAPrivateKeyImport Fail ");

            }else{

                break;
            }
        }
        
        
    }
    
    ret = HttpsInit();
    if(ret != 0)
    {
        LOG_Error("Https Init Fail ");
    }
    
    ret = CertificateInspectionAndApplication();
    if (ret < 0)
    {
        LOG_Error("Certificate Inspection And Application Fail \n");
      
    }
    ThreadInit();
    while (1)
    {
       sleep(100);
    }
    
    HttpsDeinit();
    
    return 0;
}
