#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "https.h"
#include "https_interface.h"
#include "cjson_interface.h"

#include "file_interface.h"
#include "cjson_interface.h"
#include "certificate_interface.h"
#include "sr_log.h"
#include "cjson_interface.h"
#include "queue_msg.h"
#include "sr_ipc.h"
#include <openssl/engine.h>
#include "sr_engine.h"
#include "thread_interface.h"
#include "system_interface.h"


int CertificateResponseHandler(void *response, size_t size, size_t nmemb, void *userdata) 
{
	Pki_Response *component = {0};
    int ret = 0;

	if(response == NULL)
	{
		LOG_Error("TboxCycleResponseHandler response NULL \n");
		return -1;
	}

    if ((ret = ParseJsonInterface((const char *)response,&component)) == 0) 
    {
        
        ret = OpenWriteFile(DEVICECERTFILE ,component->certificate,strlen(component->certificate));
        if(ret < 0)
        {
            LOG_Error("write %s fail",DEVICECERTFILE);
            FreeParseJson(&component);
            return -1;
        }

        if(VerifyCertificateChain(DEVICECERTFILE, DEVICECACERTCHAINFILE) != 0 )
        {   
            LOG_Error("Device certificate verification failed");
            FreeParseJson(&component);
            return -1;
        } 
        
        //ret = OpenWriteFile(DEVICEKEYFILE ,component->private_key,strlen(component->private_key));
        ret = TeePrivateKeyImport(component->private_key,strlen(component->private_key),2);
        if(ret < 0)
        {
            LOG_Error("PrivateKeyImport fail");
        }

    }else{

        LOG_Error("CertificateResponseHandler ParseJsonInterface Fail");
        ret = -1;
        if(component)
        {
            LOG_Debug("component not null ...");
            
        }
        return ret;
    }

    if(component)
    {
        LOG_Debug("FreeParseJson ...");
        FreeParseJson(&component);
    }
    
    return ret;
}


int TboxCertificateApplication()
{

	// 定义 HTTPS 客户端参数
	char urlarry[256] = {0};
	int ret = 0;
    int i = 0;
	HTTPSClientParams params = {0};
    display_engine_list();
	sprintf(urlarry,"%s",CERTIFICATEAPPLICATIONLINK);
	params.url = urlarry;
	params.header1 = HEADER;
    params.post_data = CreateJsonInterface();
    
    if(!params.post_data)
    {
       LOG_Error("Create Json FAIL ........\n");
    }

    LOG_Debug("params.post_data = %s\n",params.post_data);
	params.httpscallback = CertificateResponseHandler;
    
    
    if ((ret = HttpsPostClient(&params)) == 0)
    {
	    LOG_Debug("HttpPostClient POST request succeeded.");
	} else {
        
        ret = GetSystemRetValue("rm -rf /oemdata/cert/private.txt");
        if(ret != 0 )
        {
            LOG_Error("rm -rf /oemdata/cert/private.txt fail");
        }
        ret = CAPrivateKeyImport();
        if(ret != 0)
        {
            LOG_Error("Private Key Import Fail");
        }
	    LOG_Error("HttpPostClient POST request failed. ret : %d",ret);
        sleep(2);
        ret = -1;
        
	}
    

    if(params.post_data)
    {
        LOG_Debug("free data \n");
	    free(params.post_data);
        params.post_data = NULL;
    }

    return ret;
}

int TboxCertificateVerification()
{
    char urlarry[256] = {0};
	int ret = 0;
	HTTPSClientParams params;

	sprintf(urlarry,"%s",CERTVERIFICATIONLINK);
	params.url = urlarry;
    
    ret = HttpsPostClientCertVer(&params);
    if(ret != 0)
    {
        LOG_Error("HttpsPostClientCertVer fail\n");
    } 

    return ret;
}


void ENGINE_DeInit_xx_sirun()
{
    ENGINE *e = NULL;
    e = ENGINE_by_id("engine-sirun");
    if (!e)
    {
        LOG_Debug("ENGINE_DeInit_ca_sirun err load engine failed");
        return;
    }
    ENGINE_unregister_RSA(e);
    // 清理引擎资源
     if (ENGINE_get_finish_function(e))
    {
        ENGINE_get_finish_function(e)(e);
    }else{
        LOG_Error("ENGINE_get_finish_function NULL");
    }
    ENGINE_remove(e);
    ENGINE_free(e);
    ENGINE_cleanup();
    LOG_Debug("ENGINE_DeInit_ca_sirun");
}

//PmSetSleepEvent(PM_SLEEP_EVENT_UPGRADE);   PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE);

void display_engine_list()
{
    ENGINE *h;
    int loop;

    h = ENGINE_get_first();
    loop = 0;
    LOG_Debug("start:");
    while (h)
    {
        LOG_Debug("engine %i, id = \"%s\", name = \"%s\"",
                  loop++, ENGINE_get_id(h), ENGINE_get_name(h));
        h = ENGINE_get_next(h);
    }
    LOG_Debug("end of list");
    ENGINE_free(h);
}

int CertificateApplicationInterface()
{
    int ret = 0;
    
    ret = ENGINE_Init_ca_sirun();
    if(ret != 0)
    {
        LOG_Error("ENGINE_Init_ca_sirun fail");
        //return -1;
    }
    //申请证书
    ret = TboxCertificateApplication();
    display_engine_list();
    if(ret != 0 )
    {
        LOG_Error("TboxCertificateApplication fail");
        ENGINE_DeInit_ca_sirun();
        return -1;
    }

    ENGINE_DeInit_ca_sirun();
    display_engine_list();
    return 0;
}


int CertificateInspectionAndApplication() 
{
    int ret = 0;

    if(access(DEVICECERTFILE,F_OK) == 0)
    {
        //ENGINE_DeInit_sirun();
        SiRunPKIEngineInit();
        //证书验证
        LOG_Debug("CertificateInspectionAndApplication start ");
        ret = TboxCertificateVerification();
        ENGINE_DeInit_sirun();
        display_engine_list();
        if(ret == 0)
        {
            LOG_Debug("Tbox Certificate Verification success");
            return 0;

        }else{
            //ENGINE_DeInit_sirun();
            LOG_Error("Tbox Certificate Verification fail");
            ret  = CertificateApplicationInterface();
            if(ret != 0)
            {
                LOG_Error("Certificate Application fail");
                return -1;
            }
        }
    }else{
        //ENGINE_DeInit_sirun();
        ret  = CertificateApplicationInterface();
        if(ret != 0)
        {
            LOG_Error("Certificate Application fail");
            return -1;
        }
    }

    return 0;
}
