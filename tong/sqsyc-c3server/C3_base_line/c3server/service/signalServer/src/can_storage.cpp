#include "sr_log.h"
#include "data_pool.h"
#include <vector>
#include <deque>
#include <mutex>
#include <thread>
#include <errno.h>
#include "err_code.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sstream>
#include <unordered_map>
#include "common.h"

using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

#define  CAN_DATA_MEM_MAP_FILE_NAME     "/oemdata/config/can_data_mem.map"
#define  CAN_DATA_MEM_MAP_SIZE          (40960)


static void *g_memMap = MAP_FAILED;
static int32_t g_fileFd;


int32_t InitCanDataMemMap()
{
    struct stat fileStat;

    // 打开文件
    g_fileFd = open(CAN_DATA_MEM_MAP_FILE_NAME, O_RDWR | O_CREAT);
    if (g_fileFd < 2) 
    {
        LOG_Error("%s open error", CAN_DATA_MEM_MAP_FILE_NAME);
        return RET_ERR;
    }

    // 获取文件大小
    if (fstat(g_fileFd, &fileStat) == -1) 
    {
        LOG_Error("%s get size error", CAN_DATA_MEM_MAP_FILE_NAME);
        return RET_ERR;
    }

    if(fileStat.st_size < CAN_DATA_MEM_MAP_SIZE)
    {
        if(-1 == ftruncate(g_fileFd, CAN_DATA_MEM_MAP_SIZE))
        {
            LOG_Error("%s get size error", CAN_DATA_MEM_MAP_FILE_NAME);
            return RET_ERR;
        }
        fileStat.st_size = CAN_DATA_MEM_MAP_SIZE;
    }
    // 映射文件到内存
    g_memMap = mmap(NULL, fileStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_fileFd, 0);
    if(g_memMap == MAP_FAILED) 
    {
        LOG_Error("%s mmap error", CAN_DATA_MEM_MAP_FILE_NAME);
        return RET_ERR;
    }
    
    return RET_OK;
}

int32_t DeInitCanDataMemMap()
{
    if(MAP_FAILED == g_memMap)
    {
        return RET_OK;
    }

    if (munmap(g_memMap, CAN_DATA_MEM_MAP_SIZE) == -1) 
    {
        LOG_Error("%s munmap error", CAN_DATA_MEM_MAP_FILE_NAME);
        return RET_ERR;
    }

    if (close(g_fileFd) == -1) 
    {
        LOG_Error("%s close error", CAN_DATA_MEM_MAP_FILE_NAME);
        return RET_ERR;
    }

    g_fileFd = -1;
    g_memMap = MAP_FAILED;
    return RET_OK;
}

int32_t LoadCanDataFromFile(std::unordered_map<uint64_t ,std::vector<uint8_t>>& canMap)
{
    std::string result((char *)g_memMap, CAN_DATA_MEM_MAP_SIZE);
    std::istringstream iss(result);
    std::string line;

    // printf("g_memMap:\n%.*s\n", CAN_DATA_MEM_MAP_SIZE, (char *)g_memMap);
    // printf("result: %s\n", result.c_str());

    while (std::getline(iss, line)) 
    {
        // printf("line: %s\n", line.c_str());
        std::istringstream lineData(line);
        uint64_t key;
        std::vector<uint8_t> canData;
        if (lineData >> key) 
        {
            canData.clear();
            uint64_t value;
            while (lineData >> value)
            {
                canData.insert(canData.end(), (uint8_t *)&value, (uint8_t *)&value + sizeof(value));
            }
            if(canData.size() > 0)
            {
                canMap[key] = canData;
                // printf("canMap: key:%lu, data len :%lu\n", key, canData.size());
            }
        }
    }
    return RET_OK;
}


int32_t SaveCanDataFromFile(std::unordered_map<uint64_t ,std::vector<uint8_t>>& canMap)
{
    std::string result;

    for (const auto &item : canMap)
    {
        result += std::to_string(item.first);

        const std::vector<uint8_t> &canData = item.second;

        size_t dataSize = canData.size();
        size_t num64Values = (dataSize + 7) / 8;
        uint64_t value = 0;

        for (size_t i = 0; i < 8; ++i)
        {
            if (i < num64Values)
            {
                size_t startIdx = i * 8;
                size_t endIdx = std::min(startIdx + 8, dataSize);
                size_t copySize = endIdx - startIdx;
                memcpy(&value, &canData[startIdx], copySize);
            }
            else
            {
                value = 0;
            }
            result += std::string(" ");
            result += std::to_string(value);
        }
        result += "\n";
    }

    memset((char *)g_memMap, 0, CAN_DATA_MEM_MAP_SIZE);
    if ( result.length() > CAN_DATA_MEM_MAP_SIZE ) 
    {
        LOG_Error("result length is too long,result.length:%ld",result.length());
        memcpy((char *)g_memMap, result.c_str(), CAN_DATA_MEM_MAP_SIZE);
    }
    else
    {
        memcpy((char *)g_memMap, result.c_str(), result.length());
    }
    msync(g_memMap, CAN_DATA_MEM_MAP_SIZE, MS_SYNC);

    return RET_OK;
}


int32_t TestCase()
{
    std::unordered_map<uint64_t ,std::vector<uint8_t>> canMap;

    canMap[123] = {1,2,3,4,5,6,7,8};
    canMap[77534534525] = {12};
    canMap[77534534525] = {21};

    InitCanDataMemMap();

    LoadCanDataFromFile(canMap);

    SaveCanDataFromFile(canMap);


    DeInitCanDataMemMap();

    return RET_OK;
}


#ifdef __cplusplus
}
#endif
