#include "cycle_common.h"
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/**pm state*/
static volatile guint8 g_pmState = PM_STATE_NORMAL;
static pthread_rwlock_t g_MQTTPmStateMutex = PTHREAD_RWLOCK_INITIALIZER;

unsigned char *StrToHex(unsigned char *pbDest, const unsigned char *pbSrc,int nLen) 
{
    char h1, h2;
    char s1, s2;
    int i;

    for (i = 0; i < nLen; i++)
    {
        h1 = pbSrc[2 * i];
        h2 = pbSrc[2 * i + 1];

        s1 = toupper(h1) - 0x30;
        if (s1 > 9)
        {
            s1 -= 7;
        }

        s2 = toupper(h2) - 0x30;
        if (s2 > 9)
        {
            s2 -= 7;
        }
        pbDest[i] = s1 * 16 + s2;
    }
    return pbDest;
}


unsigned char *HexToStr(unsigned char *pbDest, const unsigned char *pbSrc,int nLen) 
{
    char ddl, ddh;
    int i;

    for (i = 0; i < nLen; i++)
    {
        ddh = 48 + pbSrc[i] / 16;
        ddl = 48 + pbSrc[i] % 16;
        if (ddh > 57)
        {
            ddh = ddh + 39;
        }
        if (ddl > 57)
        {
            ddl = ddl + 39;
        }
        pbDest[i * 2] = ddh;
        pbDest[i * 2 + 1] = ddl;
    }
    pbDest[nLen * 2] = '\0';
    return pbDest;
}



int HexToStr2(unsigned char *pbDest, int destLen, const unsigned char *pbSrc,int nLen) 
{
    char ddl, ddh;
    int i;

    if((nLen * 2 ) < destLen)
    {
        return RET_ERR;
    }

    for (i = 0; i < nLen; i++)
    {
        ddh = 48 + pbSrc[i] / 16;
        ddl = 48 + pbSrc[i] % 16;
        if (ddh > 57)
        {
            ddh = ddh + 7;
        }
        if (ddl > 57)
        {
            ddl = ddl + 7;
        }
        
        pbDest[i * 2] = ddh;
        pbDest[i * 2 + 1] = ddl;
    }
    
    return RET_OK;
}

int GetFotaTmpFileContentAndEmptyFile(const char *filePath, char *content, int contentLen)
{
	int fileFd = -1;
	int readSize = -1;
	struct stat fileStat;
	
	if(NULL == content)
	{
		return RET_ERR;
	}
	
	if(0 != access(filePath, F_OK))
    {
        LOG_Error("%s does not exist", filePath);
        return RET_ERR;
    }

    fileFd = open(filePath,  O_RDONLY);
    if(-1 == fileFd)
    {
        LOG_Debug("open %s failed\n",filePath);
        return RET_ERR;
    }

    if(0 != fstat(fileFd, &fileStat))
    {
        LOG_Error("get file stat Error");
		close(fileFd);
        return RET_ERR;
    }

    if(!S_ISREG(fileStat.st_mode))
    {
        LOG_Error("get file not a file");
		close(fileFd);
        return RET_ERR;
    }

    if(fileStat.st_size > contentLen)
    {
        LOG_Error("get file size > %d", contentLen);
		close(fileFd);
        return RET_ERR;
    }

    readSize = read(fileFd, content, contentLen);
    if(readSize < 0)
    {
        LOG_Error("read file error");
		close(fileFd);
        return RET_ERR;
    }
	LOG_Error("read file success");
	close(fileFd);

	if(truncate(filePath, 0) < 0)
    {
        LOG_Error("Empty file error");
		close(fileFd);
        return RET_ERR;
    }

	return readSize;
}


int GetFileContent(const char *filePathName,char *content, int contentLen)
{
	int fileFd = -1;
	int readSize = -1;
	struct stat fileStat;
	
	if(NULL == content || NULL == filePathName)
	{
		return RET_ERR;
	}
	
	if(0 != access(filePathName, F_OK))
    {
        LOG_Error("%s does not exist", filePathName);
        return RET_ERR;
    }

    fileFd = open(filePathName,  O_RDONLY);
    if(fileFd < 0)
    {
        LOG_Debug("open %s failed",filePathName);
        return RET_ERR;
    }

    if(0 != fstat(fileFd, &fileStat))
    {
        LOG_Error("get file stat Error");
		close(fileFd);
        return RET_ERR;
    }

    if(!S_ISREG(fileStat.st_mode))
    {
        LOG_Error("get file not a file");
		close(fileFd);
        return RET_ERR;
    }

    if(fileStat.st_size > contentLen)
    {
        LOG_Error("get file size > %d", contentLen);
		close(fileFd);
        return RET_ERR;
    }

    readSize = read(fileFd, content, contentLen);
    if(readSize < 0)
    {
        LOG_Error("read file error");
		close(fileFd);
        return RET_ERR;
    }
	LOG_Debug("file content: %s", content);
	close(fileFd);
	
	return readSize;
}


int GetSystemRetValue(const char *cmd)
{
	pid_t status;
	
	if(NULL == cmd)
	{
		return RET_ERR;
	}
	LOG_Debug("%s exe start", cmd);
	status = system(cmd);
	LOG_Debug("%s exe end:%d", cmd, status);
	if(-1 == status)
	{
		return RET_ERR;
	}
	
	if(!WIFEXITED(status))
	{
		return RET_ERR;
	}
	
	return WEXITSTATUS(status);
}

gint NumToBCDCode(char *numData, int numDataLen, char *bcdData, int bcdDataLen)
{
    gint retValue = -1;
    gint count = 0;

    if(NULL == numData || NULL == bcdData)
    {
        return RET_ERR;
    }

    if(numDataLen <= 0)
    {
        return RET_ERR;
    }

    if(numDataLen > bcdDataLen)
    {
        return RET_ERR;
    }

    for(count = 0; count < numDataLen; count++)
    {
        bcdData[count] = (numData[count] / 10) << 4;
        bcdData[count] |= (numData[count] % 10);
    }

    return RET_OK;
}

int GetMd5sumValue(char *md5SumValue, int md5SumLen, char *md5Data, int md5DataLen)
{
    MD5_CTX ctx;
    
    if(NULL == md5SumValue || NULL == md5Data)
    {
        return RET_ERR;
    }

    if(md5SumLen < 33)
    {
        return RET_ERR;
    }

    MD5_Init(&ctx);
    MD5_Update(&ctx, md5Data, md5DataLen);
    MD5_Final((unsigned char *)md5SumValue, &ctx);

    return RET_OK;
}

int GetBJTime(char *data, int dataLen)
{
    time_t stTmpTime;
	struct tm *pstRetTime;
    int offset = 0;

	if(NULL == data || dataLen < 6)
	{
		return RET_ERR;
	}

	time(&stTmpTime);
	stTmpTime += 28800;
	pstRetTime = gmtime(&stTmpTime);
    if(NULL == pstRetTime)
    {
        return RET_ERR;
    }

	data[offset++] = ((pstRetTime->tm_year + 1900) % 100);
	data[offset++] = pstRetTime->tm_mon + 1;
	data[offset++] = pstRetTime->tm_mday;
	data[offset++] = pstRetTime->tm_hour;
	data[offset++] = pstRetTime->tm_min;
	data[offset++] = pstRetTime->tm_sec;

	return offset;
}

/**
 * @brief Get the Pm State object
 * 
 * @retval	RET_ERR     Error
 *          SGMW_PM_STATE_INIT     INIT
 *          SGMW_PM_STATE_SLEEP    Sleep
 *          SGMW_PM_STATE_NORMAL   normal
 */
uint8_t GetPmState()
{
    pthread_rwlock_rdlock(&g_MQTTPmStateMutex);
    uint8_t pmState = g_pmState;
	pthread_rwlock_unlock(&g_MQTTPmStateMutex);
    return pmState;
}

/**
 * @brief Set the Power State object
 * 
 * @param[in]	pmState	
 */
void SetPmState(uint8_t pmState)
{
    pthread_rwlock_wrlock(&g_MQTTPmStateMutex);
    g_pmState = pmState;
    LOG_Info("%s, g_pmState:%d", __FUNCTION__, g_pmState);
	pthread_rwlock_unlock(&g_MQTTPmStateMutex);
}

#ifdef __cplusplus
}
#endif