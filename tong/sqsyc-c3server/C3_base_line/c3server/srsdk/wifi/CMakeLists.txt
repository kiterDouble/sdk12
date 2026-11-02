cmake_minimum_required(VERSION 3.0)
project(sdk-wifi)

if (${MODEMTYPE} STREQUAL "ag35cet-openssl3")
set(SDK_SOURCE_CODE_PATH
        "${SDK_SOURCE_CODE_PATH}"
        "${PROJECT_SOURCE_DIR}/ag35cet/sr_wifi_api.c"
        PARENT_SCOPE)
else()
# 参与编译的源码
set(SDK_SOURCE_CODE_PATH
        "${SDK_SOURCE_CODE_PATH}"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/sr_wifi_api.c"
        PARENT_SCOPE)
endif()

# 需要install的头文件
set(SDK_INSTALL_HEADER_PATH 
        "${SDK_INSTALL_HEADER_PATH}"
        "${PROJECT_SOURCE_DIR}/inc/sr_wifi_api.h"
        PARENT_SCOPE)

