cmake_minimum_required(VERSION 3.0)
project(hal-ecall)

if (${MODEMTYPE} STREQUAL "ag35cet-openssl3")
set(HAL_SOURCE_CODE_PATH
        "${HAL_SOURCE_CODE_PATH}"
        "${PROJECT_SOURCE_DIR}/ag35cet/hal_modem_ecall.c"
        PARENT_SCOPE)
else()
# 参与编译的源码
set(HAL_SOURCE_CODE_PATH
        "${HAL_SOURCE_CODE_PATH}"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_ecall.c"
        PARENT_SCOPE)
endif()

# 需要install的头文件
set(HAL_INSTALL_HEADER_PATH 
        "${HAL_INSTALL_HEADER_PATH}"
        "${PROJECT_SOURCE_DIR}/include/hal_modem_ecall.h"
        PARENT_SCOPE)

