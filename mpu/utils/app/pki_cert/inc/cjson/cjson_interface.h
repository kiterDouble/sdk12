#ifndef _CJSON_INTERFACE_H_
#define _CJSON_INTERFACE_H_

#include <iostream>
#include <queue>
#include <string>
#include <cstring>


// 定义数据结构以存储解析后的数据
typedef struct {
    char *car_id;
    char *part_type;
} Pki_ComponentData;

// 定义结构体 ApiResponse
typedef struct {
    char *private_key;   // 动态分配内存的指针
    char *certificate;   // 动态分配内存的指针
    char *status;        // 动态分配内存的指针
} Pki_Response;

int ParseJsonInterface(const char *string,Pki_Response **response);
void FreeParseJson(Pki_Response **response);
char* CreateJsonInterface();

#endif