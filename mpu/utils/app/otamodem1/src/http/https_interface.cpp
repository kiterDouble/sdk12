#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "https.h"
#include "https_interface.h"
#include "cjson_interface.h"
#include "system_interface.h"
#include "http_queue.h"
#include "file_interface.h"
#include "cjson_interface.h"
#include "https_interface.h"
#include "sr_log.h"
#include "cjson_interface.h"
#include "queue_msg.h"
#include "sr_ipc.h"
#include "http_thread.h"
#include "completion_timeout.h"
#include "time_interface.h"

static int otaflag = 0;
static pthread_rwlock_t g_RWLock = PTHREAD_RWLOCK_INITIALIZER;

void SetOtaFlag(int data)
{
    pthread_rwlock_wrlock(&g_RWLock);
    otaflag = data;
    pthread_rwlock_unlock(&g_RWLock);
}

int GetOtaFlag()
{
    return otaflag;
}

int ReportVehicleInformation(ComponentData *data,UpgradeData *upgradedata,char *jsonstring,int flag)
{
    int ret = 0;
    char httpsurl[256] = {0};
    char vehiclename[30]= {0};
    char token[1024] = {0};
	char headr3[1024] = {0};
    char swversion[64] = {0};

    HTTPSClientParams params = {0};
    memset(&params,0,sizeof(HTTPSClientParams));
    params.httpscallback = NULL;
    params.file = NULL;
    ret = GetVehicleName(vehiclename);
    if(ret < 0 )
    {
        LOG_Error("Get Vehicle Name Fail\n");
        return -1;
    }
    sprintf(httpsurl,"%s%s",REPORTVEHICLEINFORMATION,vehiclename);
    params.url = httpsurl;
    ret = GetSWVersion(swversion);
    if(ret < 0)
    {
        LOG_Error("GetSWVersion fail \n");
    }
    params.header1 = HEADER;
    params.header2 = HEADER1;
    ret =  OpenReadFile(TOKENFILEPATH ,token,1024);
    if(ret < 0)
    {
        LOG_Error("OpenReadFile fail \n");
        return -1;
    }
    sprintf(headr3,"token:%s",token);
    params.header3 = headr3;

    if(data)
    {
        upgradedata->upgradeComponent = data->component;
        upgradedata->upgradeVersionId = data->versionId;
        upgradedata->upgradeVersionName = data->versionName;
        params.post_data = CreateVehicleInformationJsonData(upgradedata);
        LOG_Debug("params.post_data = %s\n",params.post_data);
        if(!params.post_data)
        {
            LOG_Error("Create Vehicle Information Json Data Error \n");
            return -1;
        }
    }else{

        params.post_data = jsonstring;
        LOG_Debug("params.post_data = %s\n",params.post_data);
    }
    printf("params.post_data = %s\n",params.post_data);
    ret = HttpPostClient(&params);
    if(ret < 0)
    {
        LOG_Error("Https Post Client Fail \n");
        printf("Https Post Client Fail \n");
        if(flag == 1)
        {
            ret = OpenWriteFile(UPGRADERESULTS ,params.post_data,strlen(params.post_data));
            if(ret < 0)
            {
                LOG_Error("Open Write File Error\n");
            }
        }
        
    }

    if(data)
    {
        free(params.post_data);
    }         
    

    return ret;

}




int TokenResponseHandler(void *response, size_t size, size_t nmemb, void *userdata) 
{
   
    int ret = 0;
	if(response == NULL)
	{
		LOG_Error("TokenResponseHandler response NULL \n");
		return -1;
	}
	ResponseData  data = TokenParseJson((char *)response);
   
    if(data.success && data.token)
    {
       
        ret = GetSystemRetValue(RMTOKENFILE);
        if(-1 == ret)
        {
            LOG_Error("ret %d, cmd : %s", ret,RMTOKENFILE);

        }
        ret = OpenWriteFile(TOKENFILEPATH, data.token,strlen(data.token));
        if(ret < 0 )
        {
            LOG_Error("TokenResponseHandler OpenWriteFile fail \n");
            return -1;
        }else{
            LOG_Error("TokenResponseHandler OpenWriteFile success \n");
        }
    }

    TokenFreeJsonData(&data);
    return 0;
}


int TboxCycleResponseHandler(void *response, size_t size, size_t nmemb, void *userdata) 
{
	ComponentData *components = NULL;
    int component_count = 0;
   
	if(response == NULL)
	{
		LOG_Error("TboxCycleResponseHandler response NULL \n");
		return -1;
	}

    if (ParsePlatformJson((char *)response, &components, &component_count) == 0) 
    {
        //printf("ParsePlatformJson start \n");
        for (int i = 0; i < component_count; i++) {
            printf("Component: %s, Version ID: %d, Version Name: %s, URL: %s\n",
                   components[i].component, components[i].versionId,
                   components[i].versionName, components[i].url);
            LOG_Debug("Component: %s, Version ID: %d, Version Name: %s, URL: %s\n",
                   components[i].component, components[i].versionId,
                   components[i].versionName, components[i].url);

			PushHttpDataToQue(components);
        }

    }else{
        LOG_Error("TboxCycleResponseHandler ParsePlatformJson fail\n");

    }

    return 0;
}


int TboxCycleRequest()
{

	// 定义 HTTPS 客户端参数
	char token[1024] = {0};
	char headr2[1024] = {0};
	char vehiclename[32] = {0};
	char urlarry[256] = {0};
	int ret = 0;
	HTTPSClientParams params = {0};

	//获取车辆号
	ret = GetVehicleName(vehiclename);
	if(ret < 0)
	{
		LOG_Error("GetVehicleName fail\n");
        return -1;
	}

    if(strncmp(vehiclename,"000000",strlen("000000")) == 0)
    {
        LOG_Debug("vehiclename = %s",vehiclename);
        return -1;
    }

    PthreadMutexLock();
	sprintf(urlarry,"%s%s",PollingVersionInformation,vehiclename);
	params.url = urlarry;
    params.post_data = NULL;
    params.hardler = NULL; 
    params.file = NULL; 
    params.header1 = TOKENHEADER1;
	params.header2 = HEADER1;
	params.httpscallback = TboxCycleResponseHandler;

	//获取文件Token
	ret =  OpenReadFile(TOKENFILEPATH ,token,1024);
	if(ret < 0)
	{
		LOG_Error("OpenReadFile fail \n");
        PthreadMutexUnLock();
        return -1;
	}

    LOG_Debug("token = %s\n",token);
	sprintf(headr2,"token:%s",token);
	params.header3 = headr2;
	
	if (HttpPostClient(&params) == 0) 
    {
		LOG_Debug("HttpPostClient POST request succeeded.\n");
        printf("HttpPostClient POST request succeeded.\n");
        //SetOtaFlag(1);
        PthreadMutexUnLock();
        
	} else {

		LOG_Error("HttpPostClient POST request failed.\n");
        LOG_Error("HttpPostClient POST request failed.\n");
        PthreadMutexUnLock();
		return -1;
	}
	
	return ret;
}


void * HTTPSThread(void *arg) 
{
    int ret;
    int count = 0;
    int flag = 1;
    char buf[512] = {0};
    //char appotasta;
    sleep(6);
    //sleep(60 *20);
    while(1)
    {
        if(access(TOKENFILEPATH,F_OK) == 0)
        {
            if(GetKl15Status() == KL15_IS_ON  && flag == 1)
            {
                //SetAppOtaStatus(1);
                //appotasta = 1;
                //SendMsgToAllQueue(SYS_QUE_TYPE_APP_OTA, (void *)&appotasta, sizeof(appotasta));
                //PmSetSleepEvent(PM_SLEEP_EVENT_UPGRADE);
                if(access(UPGRADERESULTS,F_OK) == 0)
                {
                    LOG_Debug("UPGRADERESULTS start ...\n");
                    ret = OpenReadFile(UPGRADERESULTS, buf, sizeof(buf));
                    if(ret < 0)
                    {
                        LOG_Error("Open Read File %s Error",UPGRADERESULTS);
                        //continue;
                    }

                    ret = ReportVehicleInformation(NULL,NULL,buf,0);
                    if(ret < 0 )
                    {
                        LOG_Error("Report Vehicle Information Error");
                    }

                    ret = GetSystemRetValue(DELETEUPGRADERESULTS);
                    if(ret == -1)
                    {
                        LOG_Error("ret %d, cmd : %s", ret, DELETEUPGRADERESULTS);
                    }
                }

                sleep(30);
                if(count < 10)
                {
                    //发送周期报文
                    LOG_Debug("TboxCycleRequest start ...\n");
                    ret = TboxCycleRequest();
                    if(ret < 0 )
                    {
                        LOG_Error("TboxCycleRequest fail ...\n");
                    }
                 
                }else{

                    flag = 0;
                    //SetAppOtaStatus(0);
                    //appotasta = 0;
                    //SendMsgToAllQueue(SYS_QUE_TYPE_APP_OTA, (void *)&appotasta, sizeof(appotasta));
                    //PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE);
                    count = 0;
                    LOG_Debug("TboxCycleRequest count:%d flag:%d ...\n",count,flag);
                }
                count++;

            }else if(GetKl15Status() == KL15_IS_OFF)
            {
                LOG_Debug("KL15 Status IS OFF");
                count = 0;
                flag = 1;
            }
            LOG_Debug("TboxCycleRequest count:%d flag:%d ...\n",count,flag);
            DelayMS(600);
        }else{
            
            LOG_Debug("token 文件不存在... \n");
            sleep(30);
        }
        //LOG_Debug("sleep 1s ... \n");
        //sleep(1);
    }

    return NULL;
}
