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
#define  CAN_DATA_MEM_MAP_SIZE          (20480)


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

int32_t LoadCanDataFromFile(std::unordered_map<uint64_t ,uint64_t>& canMap)
{
    std::string result((char *)g_memMap, CAN_DATA_MEM_MAP_SIZE);
    std::istringstream iss(result);
    std::string line;
    // uint8_t *canData = NULL;
    uint8_t canData[8];

    printf("g_memMap: %s\n", (char *)g_memMap);
    printf("result: %s\n", result.c_str());

    while (std::getline(iss, line)) 
    {
        printf("line: %s\n", line.c_str());
        std::istringstream lineData(line);
        uint64_t key;
        uint64_t value;
        // canData = (char *)&value;
        if (lineData >> key >> value) 
        {
            canMap[key] = value;
            memcpy(canData, (uint8_t *)&value, 8);
            printf("canMap: key:%lu, value :%lu\n", key, value);
            printf("get can id[%lx][%02x %02x %02x %02x %02x %02x %02x %02x]\n", 
                                key, canData[0], canData[1], canData[2], canData[3], 
                                canData[4], canData[5], canData[6], canData[7]);
        }
    }
    return RET_OK;
}


int32_t SaveCanDataFromFile(std::unordered_map<uint64_t ,uint64_t>& canMap)
{
    std::string result;

    for (const auto& item : canMap) 
    {
        result += std::to_string(item.first) + " " + std::to_string(item.second) + "\n";
    }

    memset((char *)g_memMap, 0, CAN_DATA_MEM_MAP_SIZE);
    memcpy((char *)g_memMap, result.c_str(), result.length());
    msync(g_memMap, CAN_DATA_MEM_MAP_SIZE, MS_SYNC);

    return RET_OK;
}


int32_t TestCase()
{
    std::unordered_map<uint64_t ,uint64_t> canMap;

    canMap[123] = 78999282891;
    canMap[77534534525] = 789992824522891;
    canMap[77534534525] = 43452352345234;

    InitCanDataMemMap();

    LoadCanDataFromFile(canMap);

    SaveCanDataFromFile(canMap);


    DeInitCanDataMemMap();

    return RET_OK;
}


#ifdef __cplusplus
}
#endif
