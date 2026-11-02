cmake_minimum_required(VERSION 3.0)
project(sdk-uds)


# 参与编译的源码
set(SDK_SOURCE_CODE_PATH
        "${SDK_SOURCE_CODE_PATH}"
        "${PROJECT_SOURCE_DIR}/src/uds_comm.c"
        "${PROJECT_SOURCE_DIR}/src/uds_server.c"
        "${PROJECT_SOURCE_DIR}/src/uds_client.c"
        PARENT_SCOPE)

# 需要install的头文件
set(SDK_INSTALL_HEADER_PATH 
        "${SDK_INSTALL_HEADER_PATH}"
        "${PROJECT_SOURCE_DIR}/inc/uds_comm.h"
        "${PROJECT_SOURCE_DIR}/inc/uds_server.h"
        "${PROJECT_SOURCE_DIR}/inc/uds_client.h"
        PARENT_SCOPE)

