#include "tsp_common.h"
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/**pm state*/
static volatile guint8 g_pmState = SGMW_PM_STATE_INIT;
static pthread_rwlock_t g_TSPPmStateMutex = PTHREAD_RWLOCK_INITIALIZER;

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
            ddh = ddh + 7;
        }
        if (ddl > 57)
        {
            ddl = ddl + 7;
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

    // 获取文件状态
    if(0 != fstat(fileFd, &fileStat))
    {
        LOG_Error("get file stat Error");
		close(fileFd);
        return RET_ERR;
    }
    // 是否是一个常规文件
    if(!S_ISREG(fileStat.st_mode))
    {
        LOG_Error("get file not a file");
		close(fileFd);
        return RET_ERR;
    }
    // 文件内的内容是否超过接收数组的大小
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
    // 修改文件的大小，擦除文件
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
        LOG_Debug("open %s failed\n",filePathName);
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
        LOG_Error("get file size %d > %d", fileStat.st_size,contentLen);
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

/**
 * @brief Get the Pm State object
 * 
 * @retval	RET_ERR     Error
 *          PM_STATE_SLEEP   Sleep
 *          PM_STATE_NORMAL   normal
 */
uint8_t GetPmState()
{
    pthread_rwlock_rdlock(&g_TSPPmStateMutex);
    uint8_t pmState = g_pmState;
	pthread_rwlock_unlock(&g_TSPPmStateMutex);
    return pmState;
}

/**
 * @brief Set the Power State object
 * 
 * @param[in]	pmState	
 */
void SetPmState(uint8_t pmState)
{
    pthread_rwlock_wrlock(&g_TSPPmStateMutex);
    g_pmState = pmState;
    LOG_Debug("%s, g_pmState:%d", __FUNCTION__, g_pmState);
	pthread_rwlock_unlock(&g_TSPPmStateMutex);
}

