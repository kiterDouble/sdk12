cmake_minimum_required(VERSION 3.0)
project(hal-era)

if (${MODEMTYPE} STREQUAL "A7805")
# 参与编译的源码
set(HAL_SOURCE_CODE_PATH
        "${HAL_SOURCE_CODE_PATH}"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_era.c"
        PARENT_SCOPE)

# 需要install的头文件
set(HAL_INSTALL_HEADER_PATH 
        "${HAL_INSTALL_HEADER_PATH}"
        "${PROJECT_SOURCE_DIR}/include/hal_modem_era.h"
        PARENT_SCOPE)
endif()
