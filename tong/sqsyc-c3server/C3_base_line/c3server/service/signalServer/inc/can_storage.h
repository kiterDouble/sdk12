/**
 * @file can_storage.h
 * @author fxl 
 * @brief 
 * @version 1.0
 * @date 2024-08-16
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef _CAN_STORAGE_H_
#define _CAN_STORAGE_H_

#include <stdint.h>
#include <unordered_map>

#ifdef __cplusplus
extern "C"
{
#endif

int32_t InitCanDataMemMap();

int32_t DeInitCanDataMemMap();

int32_t LoadCanDataFromFile(std::unordered_map<uint64_t ,std::vector<uint8_t>>& canMap);

int32_t SaveCanDataFromFile(std::unordered_map<uint64_t ,std::vector<uint8_t>>& canMap);

int32_t TestCase();

#ifdef __cplusplus
}
#endif
#endif
