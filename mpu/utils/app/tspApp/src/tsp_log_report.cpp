#include "tsp_log_report.h"
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "err_code.h"
#include "tsp_gettoken.h"

#define  FTP_DOMAIN_NAME    "iov-staging.sgmwcloud.com.cn"
#define  LOG_PATH_NAME      "/oemdata/log/"

static gint g_pthreadFlag = -1;
static gint g_logDayNum = 0;

static size_t ReadCallback(char *ptr, size_t size, size_t nmemb, void *stream)
{
  curl_off_t nread;
  
  size_t retcode = fread(ptr, size, nmemb, (FILE *)stream);
 
  nread = (curl_off_t)retcode;
 
  LOG_Debug("ftp read %" CURL_FORMAT_CURL_OFF_T" bytes from file", nread);

  return retcode;
}
 
gint FileReportToFtp(const char *filePathName, gint nameLen, const char *fileName, gint fileNameLen)
{
    gint        retValue = 0;
    CURLcode    res;
    FILE        *fileFp;
    struct stat fileInfo;
    curl_off_t  fsize;
    CURL        *curl;
    char        ftpUrlBuff[128] = {0};
    char        ftpIpAddr[128] = {0};
 
    LOG_Debug("%s", filePathName);
    
    if(NULL == filePathName || NULL == fileName)
    {
        LOG_Error("filePathName is NULL ");
        return RET_ERR;
    }

	/* get the file size of the local file */
	if(stat(filePathName, &fileInfo))
	{
		LOG_Error("Couldn't open '%s': %s", filePathName, strerror(errno));
		return RET_ERR;
	}

	fsize = (curl_off_t)fileInfo.st_size;

	LOG_Debug("Local file size: %" CURL_FORMAT_CURL_OFF_T " bytes.", fsize);

	/* get a FILE * of the same file */
    fileFp = fopen(filePathName, "rb");
	if(NULL == fileFp)
	{
        LOG_Error("fopen error");
        return RET_ERR;
	}

	curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */
	curl = curl_easy_init();
	if(NULL == curl) 
    {
        LOG_Error("curl_easy_init error");
        fclose(fileFp);
        return RET_ERR;
    }

    retValue = GetHostIpAddr(FTP_DOMAIN_NAME, strlen(FTP_DOMAIN_NAME), ftpIpAddr, 128);
    if(retValue < 0)
    {
        LOG_Error("ftp get ipaddr error");
        fclose(fileFp);
        return RET_ERR;
    }

    retValue = snprintf(ftpUrlBuff, 128, "ftp://%s:2231/%s", ftpIpAddr, fileName);
    if(retValue <= 0 || retValue >= 128)
    {
        LOG_Error("ftpUrlBuff Assembly error");
        fclose(fileFp);
        return RET_ERR;
    }
	/* specify target */
	curl_easy_setopt(curl, CURLOPT_URL, ftpUrlBuff);
	LOG_Debug("%s", ftpUrlBuff);
	
    /* we want to use our own read function */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadCallback);
 
    /* enable uploading */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
 
    /* now specify which file to upload */
    curl_easy_setopt(curl, CURLOPT_READDATA, fileFp);
 
	curl_easy_setopt(curl, CURLOPT_USERNAME, "sirun"); 
	curl_easy_setopt(curl, CURLOPT_PASSWORD, "1FxdlSVV"); 
	
	//curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);
 
    /* Set the size of the file to upload (optional).  If you give a *_LARGE
       option you MUST make sure that the type of the passed-in argument is a
       curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
       make sure that to pass in a type 'long' argument. */
    //curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
    //                 (curl_off_t)fsize);
 
    /* Now run off and do what you've been told! */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
    {
        LOG_Error( "curl_easy_perform() failed: %s", curl_easy_strerror(res));
    }

    /* always cleanup */
    curl_easy_cleanup(curl);

    fclose(fileFp); /* close the local file */
 
    curl_global_cleanup();
    return RET_OK;
}

gint GenerateLogName(gchar *fileNameBuff,  gint nameBuffLen)
{
    gint retValue = 0;
    uint8_t vinBuff[32] = {0};
    time_t nowTime = 0;
    struct tm *timeBuff = NULL;

    if(NULL == fileNameBuff)
    {
        return RET_ERR;
    }

    if(nameBuffLen < 32)
    {
        return RET_ERR;
    }

    //retValue = GetParamsValue(enumVin, vinBuff);
    if (!GetVin(vinBuff, sizeof(vinBuff) - 1))
    {
        LOG_Error("get vin Error");
        return RET_ERR;
    }

    time(&nowTime);
    timeBuff = localtime(&nowTime);
    if(NULL == timeBuff)
    {
        LOG_Error("get localtime Error");
        return RET_ERR;
    }

    memcpy(fileNameBuff, &vinBuff[9], 8);
    retValue = snprintf(&fileNameBuff[8], nameBuffLen - 8, "_%04d%02d%02d_%02d%02d%02d.tar.gz",
                        timeBuff->tm_year + 1900, timeBuff->tm_mon + 1, timeBuff->tm_mday,
                        timeBuff->tm_hour, timeBuff->tm_min, timeBuff->tm_sec);
    if(retValue <= 0 || retValue >= (nameBuffLen - 8))
    {
        LOG_Error("GenerateLogName Error");
        return RET_ERR;
    }
    LOG_Debug("fileNameBuff: %s", fileNameBuff);
    return retValue;
}



int GetLogFileHeade(char *filehead, int fileheadeLen, int numDay)
{
    int retValue = -1;
	time_t nowTime = 0;
	struct tm *pstRetTime = NULL;
	
	time(&nowTime);
	nowTime += 28800;
	
	nowTime -= (86400 * numDay);
	
	pstRetTime = gmtime(&nowTime);


	retValue = snprintf(filehead, fileheadeLen, "20%02d-%02d-%02d", 
                                     ((pstRetTime->tm_year + 1900) % 100), 
                                     pstRetTime->tm_mon + 1, 
                                     pstRetTime->tm_mday);
	LOG_Debug("log filehead: %s", filehead);
	return retValue;
}

gint CompressionLogFile(gchar *filePathName, gint nameLen)
{
    gchar  compressionCmd[256] = {0};
    char filehead[64] = {0};
    gint retValue = -1;
    if(NULL == filePathName)
    {
        return RET_ERR;
    }
    // 如果大于7天或小于7天则打包所有文件，否则只打包前某一天日志
    if(g_logDayNum > 7 || g_logDayNum < 0)
    {
        retValue = snprintf(compressionCmd, 256, "tar -zcvf %s  %s/sirunlog  %s/logstore /oemdata/log/bootup.txt /oemdata/flash/mdalog/*", 
                                                  filePathName, LOG_PATH_NAME, LOG_PATH_NAME);
    }
    else
    {   
        //确定日志时间
        GetLogFileHeade(filehead, 64, g_logDayNum);
        retValue = snprintf(compressionCmd, 256, "tar -zcvf %s  %s/logstore/%s*  /oemdata/log/bootup.txt /oemdata/flash/mdalog/*", 
                                                  filePathName, LOG_PATH_NAME, filehead);
    }

    if(retValue <= 0 || retValue >= 256)
    {
        return RET_ERR;
    }

    //日志打包
    // 获取指令执行结果
    retValue = GetSystemRetValue(compressionCmd);
    if(retValue != 0)
    {
        LOG_Error("cmd %s  exec error", compressionCmd);
        return RET_ERR;
    }
    LOG_Debug("compressionCmd: %s success ", compressionCmd);
    return RET_OK;
}



static void *LogReportThread(void *argv)
{
    gint  retValue = -1;
    gchar filePathName[128] = {0};
    gchar rmCmd[128] = {0};
    gchar fileName[64] = {0};
    // 删除以前打成的文件
    GetSystemRetValue("rm /oemdata/flash/*.tar.gz && sync && sync");
    // 拼接打成的包名
    retValue = GenerateLogName(fileName, 64);
    if(retValue < 0)
    {
        g_pthreadFlag = 0;
        LOG_Error("GenerateLogName exec error");
        return NULL;
    }
    // 拼接路径
    retValue = snprintf(filePathName, 128, "%s%s", LOG_PATH_NAME, fileName);
    if(retValue <= 0 || retValue >= 128)
    {
        g_pthreadFlag = 0;
        LOG_Error("snprintf  filePathName exec error");
        return NULL;
    }

    //日志打包
    retValue = CompressionLogFile(filePathName, 128);
    if(retValue < 0)
    {
        g_pthreadFlag = 0;
        LOG_Error("CompressionLogFile  exec error");
        return NULL;
    }
    // 使用FTP协议上传打包的文件
    retValue = FileReportToFtp(filePathName, 128, fileName, 64);
    if(retValue < 0)
    {
        g_pthreadFlag = 0;
        LOG_Error("FileReportToFtp  exec error");
        return NULL;
    }
    // 上传完成后删除打包的文件
    retValue = snprintf(rmCmd, 128, "rm %s && sync && sync", filePathName);
    if(retValue <= 0 || retValue >= 128)
    {
        g_pthreadFlag = 0;
        LOG_Error("snprintf  rmCmd exec error");
        return NULL;
    }
    
    //删除日志压缩包，是否执行成功
    retValue = GetSystemRetValue(rmCmd);
    if(retValue != 0)
    {
        g_pthreadFlag = 0;
        LOG_Error("cmd %s  exec error", rmCmd);
        return NULL;
    }
    g_pthreadFlag = -1;
    LOG_Debug("LogReportTask exe success");
    PmCleanSleepEvent(PM_SLEEP_EVENT_LOG);
    return NULL;
}


static gint LogReportTask()
{
    gint retValue = -1;
    pthread_attr_t attr;
    pthread_t pthreadId;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	//创建线程
	retValue = pthread_create(&pthreadId, &attr, LogReportThread, NULL);
	if(retValue != 0)
	{
		LOG_Error("LogReportTask create Error %s",strerror(errno));
        pthread_attr_destroy(&attr);
        PmCleanSleepEvent(PM_SLEEP_EVENT_LOG);
        return RET_ERR;
	}
    g_pthreadFlag = 1;
    pthread_attr_destroy(&attr);
	LOG_Debug("national create recv thread success");
	return RET_OK;
}

gint GetLogThreadStat()
{
    return g_pthreadFlag;
}


gint TspReportLog(gint logDayNum)
{
    // 检测是否已经存在打包上传线程
    if(g_pthreadFlag > 0)
    {
        LOG_Debug("TspReportLog have task");
        return RET_OK;
    }

    if(g_pthreadFlag == 0)
    {
        g_pthreadFlag = -1;
        LOG_Debug("TspReportLog exe task faild");
        return RET_ERR;
    }
    // 如果不存在则创建打包线程
    g_logDayNum = logDayNum;
    return LogReportTask();
}