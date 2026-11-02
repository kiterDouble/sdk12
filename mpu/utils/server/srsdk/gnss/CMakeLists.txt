cmake_minimum_required(VERSION 3.0)
project(sdk-gnss)


# 参与编译的源码
set(SDK_SOURCE_CODE_PATH
        "${SDK_SOURCE_CODE_PATH}"
        "${PROJECT_SOURCE_DIR}/src/sr_gnss.c"
        PARENT_SCOPE)

# 需要install的头文件
set(SDK_INSTALL_HEADER_PATH 
        "${SDK_INSTALL_HEADER_PATH}"
        "${PROJECT_SOURCE_DIR}/inc/sr_gnss.h"
        PARENT_SCOPE)

