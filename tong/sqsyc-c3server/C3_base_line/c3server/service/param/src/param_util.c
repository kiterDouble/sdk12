/**
 * @file param_util.c
 * @brief 参数工具函数实现
 */

#include "param_util.h"
#include <stdio.h>
#include <stdlib.h>
#include "sr_log.h"



int32_t ParamUtilGetMpuProjectNo(void)
{
    FILE *filePointer = NULL;
    uint8_t readBuffer[MPU_PROJECT_VER_LEN + 1] = {0};
    int32_t mpuVersion = 0;
    size_t readLength = 0;

    do
    {
        filePointer = fopen(MPU_VERSION_FILE_PATH, "r");
        if (filePointer == NULL)
        {
            LOG_Warn("paramUtilGetMpuVersion: open %s failed", MPU_VERSION_FILE_PATH);
            break;
        }

        readLength = fread(readBuffer, 1, MPU_PROJECT_VER_LEN, filePointer);
        (void)fclose(filePointer);

        if (readLength != MPU_PROJECT_VER_LEN)
        {
            LOG_Warn("paramUtilGetMpuVersion: read %s failed or too short, readLength=%zu",
                     MPU_VERSION_FILE_PATH, readLength);
            break;
        }

        readBuffer[MPU_PROJECT_VER_LEN] = '\0';
        mpuVersion = (int32_t)atoi((const char *)readBuffer);

        LOG_Debug("paramUtilGetMpuVersion: mpu version=%d", mpuVersion);
    } while (0);

    return mpuVersion;
}
