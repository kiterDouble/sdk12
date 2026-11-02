#include <time.h>
#include <string.h>
#include "http_thread.h"
#include "err_code.h"
#include "queue_msg.h"
#include "thread_interface.h"
#include "http_queue.h"
#include "https.h"
#include "mcu_api.h"
#include "cjson_interface.h"
#include "ota_api.h"
#include "file_interface.h"
#include <unistd.h>
#include "system_interface.h"
#include <openssl/md5.h>
#include "https_interface.h"
#include "sr_log.h" 
#include "completion_timeout.h"
#include "otamodule_interface.h"
#include "sr_ipc.h"
#include "time_interface.h"

static pthread_t httpreceive_threadId;
static pthread_t otamodule2_threadId;
static pthread_t otafile_threadId;
static unsigned char appotasta = 0;
pthread_rwlock_t RWLock = PTHREAD_RWLOCK_INITIALIZER;
void SetAppOtaStatus(unsigned char data)
{
    pthread_rwlock_wrlock(&RWLock);
    appotasta = data;
    pthread_rwlock_unlock(&RWLock);
    LOG_Debug("SetAppOtaStatus %d",data);
}

unsigned char GetAppOtaStatus()
{
    return appotasta;
}

static void HexStringToBytes(const char *hex_string, unsigned char *bytes, size_t length) 
{
    for (size_t i = 0; i < length; i++) {
        sscanf(hex_string + 2 * i, "%2hhx", &bytes[i]);
    }
}

static void RemoveLine(char *str) 
{
    str[strcspn(str, "\n")] = '\0'; // 用字符串结束符替换换行符
}


int CalculateMd5(const char *filename, unsigned char output[MD5_DIGEST_LENGTH]) 
{

    unsigned char buffer[1024];
    size_t bytes;

    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        LOG_Error("Unable to open file");
        return -1;
    }


    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);
    
    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) != 0) {
        MD5_Update(&md5_ctx, buffer, bytes);
    }

    MD5_Final(output, &md5_ctx);
    fclose(file);

    return 0;
}

static int DoUpgrade(ComponentData *data,long long starttime)
{
    int ret = 0;
    char cmd[1024] = {0};
    int upgradeFlag = 0;
    UpgradeStatus_E upgradeSta = (UpgradeStatus_E)0;
    int mcuupgradesta = 0;
    int btupgradesta = 0;
    UpgradeData upgradedata;
    unsigned char output[MD5_DIGEST_LENGTH +1];
    char swversion[64] = {0};
    struct timespec ts = {0};
    long long milliseconds = 0;
    FILE *file;
    char line[64]= {0};
    unsigned char outbytes[64] = {0};
    int reterr = 0;
    LOG_Debug("DoUpgrade Start...\n");

    if((access(BT1BINFILEPATH,F_OK) == 0) )
    {
        sprintf(cmd, "rm -rf %s*; sync ;", BT1BINFILEPATH);
        ret = GetSystemRetValue(cmd);
        if(-1 == ret)
        {
            LOG_Error("ret %d, cmd : %s", ret, cmd);
        }
    }

    sprintf(cmd, "rm -rf %s*; rm -rf %s*;  rm -rf %s*;  rm -rf %s*; sync ;", MD5DATA,MCU1BINFILEPATH,MCU2BINFILEPATH,MPUBINFILEPATHMPUBIN);

    ret = GetSystemRetValue(cmd);
    if(-1 == ret)
    {
        LOG_Error("ret %d, cmd : %s", ret, cmd);
    }

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "tar -zxvf  %s -C %s; sync ;", DOWNLOADUPGRADEPACKAGEFILEPATH, PATHUPGRADE);
    ret = GetSystemRetValue(cmd);
    if(-1 == ret)
    {
        LOG_Error("ret %d, cmd : %s", ret, cmd);
        reterr = -1;
        goto err;
    }

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "rm -rf %s",DOWNLOADUPGRADEPACKAGEFILEPATH);
    ret = GetSystemRetValue(cmd);
    if(-1 == ret)
    {
        LOG_Error("ret %d, cmd : %s", ret, cmd);
    }
    if(access(MD5DATA,F_OK) != 0)
    {
        LOG_Error("%s does not exist",MD5DATA);
        reterr = -1;
        goto err;
    }

    memset(cmd, 0, sizeof(cmd));
    memset(outbytes, 0, sizeof(outbytes));

    if(access(BT1BINFILEPATH,F_OK) == 0)
    {
        if (ReadSpecificLine(MD5DATA,4,line, 64) == 0) {
            LOG_Debug("第四行: %s", line);
        } else {
            LOG_Error("文件为空或读取错误\n");
        }

        HexStringToBytes(line,outbytes,strlen(line));
        ret = CalculateMd5(BT1BINFILEPATH,output);
        if(ret < 0 )
        {
            LOG_Error("BT1BINFILEPATH CalculateMd5 Error \n");
            reterr = -1;
            goto err;
        }

        if(memcmp((unsigned char *)outbytes,output,MD5_DIGEST_LENGTH) != 0)
        {
            LOG_Error("BT1BIN 升级包 md5对比不一致...\n");
            reterr = -1;
            goto err;
        }else{
            LOG_Debug("BT CalculateMd5 memcmp ok \n");
        }
    }
    
    if(access(MCU1BINFILEPATH,F_OK) == 0)
    {
        if (ReadSpecificLine(MD5DATA,1,line, 64) == 0) {
            LOG_Debug("第一行: %s", line);
        } else {
            LOG_Error("文件为空或读取错误\n");
        }

        HexStringToBytes(line,outbytes,strlen(line));
        ret = CalculateMd5(MCU1BINFILEPATH,output);
        if(ret < 0 )
        {
            LOG_Error("MCU1BINFILEPATH CalculateMd5 Error \n");
            reterr = -1;
            goto err;
        }

        if(memcmp((unsigned char *)outbytes,output,MD5_DIGEST_LENGTH) != 0)
        {
            LOG_Error("MCU1BIN 升级包 md5对比不一致...\n");
            reterr = -1;
            goto err;
        }else{
            LOG_Debug("CalculateMd5 memcmp ok \n");
        }
    }
    
    if(access(MCU2BINFILEPATH,F_OK) == 0)
    {
        memset(cmd, 0, sizeof(cmd));
        memset(line, 0, sizeof(line));
        memset(outbytes, 0, sizeof(outbytes));

        if (ReadSpecificLine(MD5DATA,2,line, 64) == 0) {
            LOG_Debug("第二行: %s", line);
        } else {
            LOG_Error("文件没有第二行或读取错误\n");
        }

        ret = CalculateMd5(MCU2BINFILEPATH,output);
        if(ret < 0 )
        {
            LOG_Error("MCU2BINFILEPATH CalculateMd5 Error \n");
            reterr = -1;
            goto err;
        }

        HexStringToBytes(line,outbytes,strlen(line));
        if(memcmp((unsigned char *)outbytes,output,MD5_DIGEST_LENGTH) != 0)
        {
            LOG_Error("MCU1BIN 升级包 md5对比不一致...\n");
            reterr = -1;
            goto err;
        }else{
            LOG_Debug("CalculateMd5 memcmp ok \n");
        }
    }

    if(access(MPUBINFILEPATHMPUBIN,F_OK) == 0)
    {
        memset(cmd, 0, sizeof(cmd));
        memset(line, 0, sizeof(line));
        memset(outbytes, 0, sizeof(outbytes));

        if (ReadSpecificLine(MD5DATA,3,line, 64) == 0) {
            LOG_Debug("第三行: %s", line);
        } else {
            LOG_Error("文件没有第三行或读取错误\n");
        }

        ret = CalculateMd5(MPUBINFILEPATHMPUBIN,output);
        if(ret < 0 )
        {
            LOG_Error("MPUBINFILEPATHMPUBIN CalculateMd5 Error \n");
            reterr = -1;
            goto err;
        }
        
        HexStringToBytes(line,outbytes,strlen(line));
        if(memcmp((unsigned char *)outbytes,output,MD5_DIGEST_LENGTH) != 0)
        {
            LOG_Error("Tbox 升级包 md5对比不一致...\n");
            reterr = -1;
            goto err;
        }else{
            LOG_Debug("CalculateMd5 memcmp ok \n");
        }
    }

    memset(cmd, 0, sizeof(cmd));
    memset(line, 0, sizeof(line));
    memset(outbytes, 0, sizeof(outbytes));
   
    upgradedata.startTime = starttime;
    LOG_Debug("开始升级 .....\n");
    StartQueueMsgRcvThread("upgrade");
    if(access(BT1BINFILEPATH,F_OK) == 0 )
    {
	    McuSrvSetSendLock(1);
        ret = OtaStartUpgrade("/oemdata/upgrade/", UPGRADE_BT_FLAG);
        if(ret < 0)
        {
            McuSrvSetSendLock(0);
            LOG_Error("OtaStartUpgrade return error ret = %d",ret);
            goto err;
        }
        upgradeFlag = UPGRADE_BT_FLAG;

        while(1)
        {
            OtaGetUpgradeStatus("/oemdata/upgrade/", &upgradeSta);
            
            if(UPGRADE_BT_FLAG == (upgradeFlag) && \
            (UPGRADE_BT_SUCCESS == upgradeSta || \
            UPGRADE_BT_FAILED  == upgradeSta))
            {
                btupgradesta = upgradeSta;
                LOG_Debug("bt upgrade status %d \n",upgradeSta);
                break;
            }

            LOG_Debug("bt upgrade status : %d", upgradeSta);
            sleep(1);
        }

	    McuSrvSetSendLock(0);


        if(btupgradesta == UPGRADE_BT_SUCCESS)
        {
            LOG_Debug("bt upgrade success");
	        McuSrvSetSendLock(1);
            ret = OtaStartUpgrade("/oemdata/upgrade/", UPGRADE_MCU_FLAG);
            if(ret < 0)
            {
                McuSrvSetSendLock(0);
                LOG_Error("OtaStartUpgrade return error ret = %d",ret);
                goto err;
            }
            upgradeFlag = UPGRADE_MCU_FLAG;

            while(1)
            {
                OtaGetUpgradeStatus("/oemdata/upgrade/", &upgradeSta);

                if(UPGRADE_MCU_FLAG == (upgradeFlag) && \
                (UPGRADE_MCU_SUCCESS == upgradeSta || \
                UPGRADE_MCU_FAILED  == upgradeSta))
                {
                    mcuupgradesta = upgradeSta;
                    LOG_Debug("mcu upgrade status %d \n",upgradeSta);
                    break;
                }

                LOG_Debug("mcu upgrade status : %d", upgradeSta);
                sleep(1);
            }

	        McuSrvSetSendLock(0);
        }else{
            LOG_Error("BT Upgrade Fail...");
        }

    }else{
	
	    McuSrvSetSendLock(1);
        ret = OtaStartUpgrade("/oemdata/upgrade/", UPGRADE_MCU_FLAG);
        if(ret < 0)
        {
            McuSrvSetSendLock(0);
            LOG_Error("OtaStartUpgrade return error ret = %d",ret);
            goto err;
        }
        upgradeFlag = UPGRADE_MCU_FLAG;

        while(1)
        {
            OtaGetUpgradeStatus("/oemdata/upgrade/", &upgradeSta);
           
            if(UPGRADE_MCU_FLAG == (upgradeFlag) && \
            (UPGRADE_MCU_SUCCESS == upgradeSta || \
            UPGRADE_MCU_FAILED  == upgradeSta))
            {
                mcuupgradesta = upgradeSta;
                
                LOG_Debug("mcu upgrade status %d \n",upgradeSta);
                break;
            }
            LOG_Debug("mcu upgrade status : %d", upgradeSta);
            sleep(1);
        }
	
	    McuSrvSetSendLock(0);
    }
    
    
    if(mcuupgradesta == UPGRADE_MCU_SUCCESS)
    {
        ret = OtaStartUpgrade("/oemdata/upgrade/", UPGRADE_MPU_FLAG);
        if(ret < 0)
        {
            LOG_Error("OtaStartUpgrade return error ret = %d",ret);
            goto err;
        }
        upgradeFlag = UPGRADE_MPU_FLAG;
        LOG_Debug("mcu upgrade success");
        while (1)
        {
            OtaGetUpgradeStatus("/oemdata/upgrade/", (UpgradeStatus_E*)&upgradeSta);
            if(UPGRADE_MPU_FLAG == (upgradeFlag & UPGRADE_MPU_FLAG) && \
            (UPGRADE_MPU_FAILED == upgradeSta || UPGRADE_MPU_NEED_BACKUP == upgradeSta || UPGRADE_IS_BUSY == upgradeSta))
            {
                LOG_Debug("mpu upgrade status %d \n",upgradeSta);
                break;
            }
            LOG_Debug("mpu upgrade status : %d", upgradeSta);
            sleep(1);
        }
    }else if(mcuupgradesta == UPGRADE_MCU_FAILED)
    {
        LOG_Error("MCU Upgrade Fail...");
    }

    if(UPGRADE_MPU_FAILED == upgradeSta || UPGRADE_IS_BUSY == upgradeSta)
    {
        LOG_Error("MPU Upgrade Fail...");
    }
err:
    clock_gettime(CLOCK_REALTIME, &ts);
    milliseconds = (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
    upgradedata.endTime = milliseconds;

    ret = GetSWVersion(swversion);
    if(ret < 0)
    {
        LOG_Error("GetSWVersion fail \n");
    }

    RemoveLine((char *)(swversion));
    if(access(BT1BINFILEPATH,F_OK) == 0)
    {
        if((btupgradesta == UPGRADE_BT_SUCCESS) && (mcuupgradesta == UPGRADE_MCU_SUCCESS) && (upgradeSta == UPGRADE_MPU_NEED_BACKUP))
        {
            LOG_Debug("升级 bt mcu mpu ok");
            upgradedata.currVersions.Tbox = data->versionName;
            upgradedata.currVersions.status = (char *)OTASTATUSSUCCESS;
            writeIntWithTimestampToFile(OTAUPGRADETIMESTAMP,0);
        }else{
        
            upgradedata.currVersions.Tbox = (char *)swversion;
            upgradedata.currVersions.status = (char *)OTASTATUSFAIL;
            writeIntWithTimestampToFile(OTAUPGRADETIMESTAMP,1);

        }
    }else
    {
        if(mcuupgradesta == UPGRADE_MCU_SUCCESS && upgradeSta == UPGRADE_MPU_NEED_BACKUP)
        {
            LOG_Debug("升级 mcu mpu ok");
            upgradedata.currVersions.Tbox = data->versionName;
            upgradedata.currVersions.status = (char *)OTASTATUSSUCCESS;
            writeIntWithTimestampToFile(OTAUPGRADETIMESTAMP,0);
        }else{
        
            upgradedata.currVersions.Tbox = (char *)swversion;
            upgradedata.currVersions.status = (char *)OTASTATUSFAIL;
            writeIntWithTimestampToFile(OTAUPGRADETIMESTAMP,1);

        }
    }
   

    ret = ReportVehicleInformation(data,&upgradedata,NULL,1);
    if(ret < 0 )
    {
        LOG_Error("ReportVehicleInformation fail \n");
    }

    if((access(BT1BINFILEPATH,F_OK) == 0))
    {
        sprintf(cmd, "rm -rf %s*; sync ;", BT1BINFILEPATH);
        ret = GetSystemRetValue(cmd);
        if(-1 == ret)
        {
            LOG_Error("ret %d, cmd : %s", ret, cmd);
        }
    }

    sprintf(cmd, "rm -rf %s*; rm -rf %s*;  rm -rf %s*;  rm -rf %s*; rm -rf %s; sync ;", MD5DATA,MCU1BINFILEPATH,MCU2BINFILEPATH,MPUBINFILEPATHMPUBIN,DOWNLOADUPGRADEPACKAGEFILEPATH);
    ret = GetSystemRetValue(cmd);
    if(-1 == ret)
    {
        LOG_Error("ret %d, cmd : %s", ret, cmd);
    }

    if(UPGRADE_MPU_NEED_BACKUP == upgradeSta)
    {
        OtaSwitchAOrBPartition();
        sleep(10);
    }
    
    if(reterr == -1)
    {
        return reterr;
    }
    LOG_Debug("upgrade status : %d", upgradeSta);
    return upgradeSta;
}


int DownloadFile(void *response, size_t size, size_t nmemb, void *userdata) 
{
    HTTPSClientParams *params = (HTTPSClientParams *)userdata;
    int ret = 0;
    LOG_Debug("DownloadFile Start ...");;
	if(response == NULL)
	{
		LOG_Error("DownloadFile response NULL \n");
        params->response = -1;
		return -1;
	}
	
    if(params->hardler)
    {
    
        ret = WriteFile(params->hardler , (unsigned char *)response, (size * nmemb));
        if(ret < 0 )
        {
            LOG_Error("OpenWriteFile fail \n");
            return -1;
        }
        
    }
    return 0;
}

void * ReceiveFunction(void *arg)
{

    int ret;
    char swversion[64] = {0};
    ComponentData *data;
    HTTPSClientParams params = {0};
    struct timespec ts;
    long long milliseconds = 0;
    UpgradeData upgradedata;
    char cmd[256] = {0};
    //char appotasta = 0;

    while(1)
    {
        sleep(1);
        memset(&params,0,sizeof(HTTPSClientParams));
        ret = PopHttpDataFromQue(data);
        if(ret == -1)
        {
            continue;

        }else if(ret == 0)
        {
            LOG_Debug("data->component = %s \n",data->component);
            LOG_Debug("data->versionName = %s \n",data->versionName);
            //LOG_Debug("swversion = %s data->versionName = %s \n",swversion,data->versionName);
            //SetOtaFlag(1);
            PthreadMutexLock();

            if(strcmp(data->component,"TBOX") == 0)
            {

                ret = GetSWVersion(swversion);
                if(ret < 0)
                {
                    LOG_Error("GetSWVersion fail \n");
                    PthreadMutexUnLock();
                    PlatformJsonFree(data, 1);
                    continue;
                }

            	
                LOG_Debug("swversion = %s \n",swversion);
                RemoveLine((char *)(swversion));
                LOG_Debug("xxx swversion = %s data->versionName = %s",swversion,data->versionName);
                if(strncmp((char *)swversion,data->versionName,strlen(data->versionName)) != 0)
                {
                    LOG_Debug("swversion equal to versionName ");
                    sprintf(cmd, "rm -rf %s/*;sync ", DOWNLOADUPGRADEPACKAGEFILEPATH);
                    ret = GetSystemRetValue(cmd);
                    if(-1 == ret)
                    {
                        LOG_Error("ret %d, cmd : %s", ret, cmd);
                    }

                    clock_gettime(CLOCK_REALTIME, &ts);
                    milliseconds = (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
                    upgradedata.startTime = milliseconds;
                    upgradedata.endTime = milliseconds;
                    upgradedata.currVersions.Tbox = (char *)swversion;
                    upgradedata.currVersions.status = OTADOWNLOADSTATUSSTART;
                    ret = ReportVehicleInformation(data,&upgradedata,NULL,0);
                    if(ret < 0 )
                    {
                        LOG_Error("ReportVehicleInformation fail \n");
                    }

                    for(int i = 0 ; i<3; i++)
                    {
                        params.httpscallback = DownloadFile;
                        params.file = DOWNLOADUPGRADEPACKAGEFILEPATH;
                        params.url = data->url;
                        clock_gettime(CLOCK_REALTIME, &ts);
                        milliseconds = (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
                        upgradedata.startTime = milliseconds;
                        ret = HttpsGetClient(&params);
                        if(ret != 0)
                        {
                            LOG_Error("HttpGetClient fail... \n");
                        }

                        if(ret == 0)
                        {
                            clock_gettime(CLOCK_REALTIME, &ts);
                            milliseconds = (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
                            upgradedata.endTime = milliseconds;
                            break;
                        }
                        clock_gettime(CLOCK_REALTIME, &ts);
                        milliseconds = (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
                        upgradedata.endTime = milliseconds;
                    }


                    if(ret == 0)
                    {
                        PmSetSleepEvent(PM_SLEEP_EVENT_UPGRADE);
			            CheckFileExists();
                        SetAppOtaStatus(1);
                        //appotasta = 1;
                        //SendMsgToAllQueue(SYS_QUE_TYPE_APP_OTA, (void *)&appotasta, sizeof(appotasta));
                        LOG_Debug("下载成功....\n");
                        upgradedata.currVersions.Tbox = data->versionName;
                        upgradedata.currVersions.status = (char *)OTADOWNLOADSTATUSSUCCESS;
                        //SetModule2TtaFlag(1);
                        Complete(GetOtaComp());
                        
                    }else if(ret < 0)
                    {
                        LOG_Error("下载失败....\n");
                        upgradedata.currVersions.Tbox = (char *)swversion;
                        upgradedata.currVersions.status = (char *)OTADOWNLOADSTATUSFAIL;
                    }

                    ret = ReportVehicleInformation(data,&upgradedata,NULL,0);
                    if(ret < 0 )
                    {
                        LOG_Error("ReportVehicleInformation fail \n");
                    }

                    if(strncmp(upgradedata.currVersions.status,OTADOWNLOADSTATUSFAIL,strlen(OTADOWNLOADSTATUSFAIL)) == 0)
                    {
                        //SetOtaFlag(0);
                        PthreadMutexUnLock();
                        PlatformJsonFree(data, 1);
                        LOG_Error("Error downloading the upgrade package, exiting this upgrade");
                        continue;
                    }

                    clock_gettime(CLOCK_REALTIME, &ts);
                    milliseconds = (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
                    

                    ret = WaitForCompletionTimeout(GetComp(), TIMEOUT_SECONDS);
                    if(ret)
                    {
                        printf("Completed successfully!\n");
                        LOG_Debug("modem2 ota successfully!\n");

                    }else{
                        //上报升级失败
                        LOG_Error("modem2 ota timeout occurred! \n");
                        SetAppOtaStatus(0);
                        upgradedata.startTime = milliseconds;
                        clock_gettime(CLOCK_REALTIME, &ts);
                        milliseconds = (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
                        upgradedata.endTime = milliseconds;
                        upgradedata.currVersions.Tbox = (char *)swversion;
                        upgradedata.currVersions.status = (char *)OTASTATUSFAIL;
                        ret = ReportVehicleInformation(data,&upgradedata,NULL,0);
                        if(ret < 0 )
                        {
                            LOG_Error("ReportVehicleInformation fail \n");
                        }
                        
                        writeIntWithTimestampToFile(OTAUPGRADETIMESTAMP,1);
                        //appotasta = 0;
                        //SendMsgToAllQueue(SYS_QUE_TYPE_APP_OTA, (void *)&appotasta, sizeof(appotasta));
                        PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE);
                        //SetOtaFlag(0);
                        PthreadMutexUnLock();
                        PlatformJsonFree(data, 1);
                        continue;
                    }

                    ret = DoUpgrade(data,milliseconds);
                    if(ret < 0) 
                    {
                        LOG_Error("Do Upgrade fail ...\n");
                    }

                    PlatformJsonFree(data, 1);
                    SetAppOtaStatus(0);
                    //appotasta = 0;
                    //SendMsgToAllQueue(SYS_QUE_TYPE_APP_OTA, (void *)&appotasta, sizeof(appotasta));
                    PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE);
                    //SetOtaFlag(0);
                    PthreadMutexUnLock();
                }else{

                    LOG_Debug("No need to upgrade");
                    PlatformJsonFree(data, 1);
                    PthreadMutexUnLock();
                }
            }else{
                //SetOtaFlag(0);
                LOG_Debug("no is tbox");
                PlatformJsonFree(data, 1);
                PthreadMutexUnLock();
                SetAppOtaStatus(0);
            }    
        }
    }
   
    return NULL;
}




int ThreadInit()
{
    int ret;
    ret = CreateThread(&otafile_threadId, OtaFileHander, NULL);
    if(ret < 0)
    {
        LOG_Error("Create ReceiveFunction Thread Fail \n");
    }

    ret = CreateThread(&httpreceive_threadId, ReceiveFunction, NULL);
    if(ret < 0)
    {
        LOG_Error("Create ReceiveFunction Thread Fail \n");
        return -1;
    }

    ret = CreateThread(&otamodule2_threadId, OtaModule2, NULL);
    if(ret < 0)
    {
        LOG_Error("Create ReceiveFunction Thread Fail \n");
        return -1;
    }

    
    return 0;
}

void DeInit()
{
    pthread_join(otafile_threadId, NULL);
    pthread_join(httpreceive_threadId, NULL);
    pthread_join(otamodule2_threadId, NULL);
}


