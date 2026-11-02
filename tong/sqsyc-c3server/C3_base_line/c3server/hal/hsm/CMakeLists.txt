cmake_minimum_required(VERSION 3.0)
project(hal-hsm)

file(GLOB HAL_SOURCE_CODE_FILES
    "${PROJECT_SOURCE_DIR}/ce_cet_evd_a7805/*.c"
)

# 参与编译的源码
set(HAL_SOURCE_CODE_PATH
        "${HAL_SOURCE_CODE_PATH}"
        ${HAL_SOURCE_CODE_FILES}
        PARENT_SCOPE)

# 需要install的头文件
set(HAL_INSTALL_HEADER_PATH 
        "${HAL_INSTALL_HEADER_PATH}"
        "${PROJECT_SOURCE_DIR}/include/sr_hal_hsm.h"
        PARENT_SCOPE)

