cmake_minimum_required(VERSION 3.0)
project(sdk-ota)


# 参与编译的源码
set(SDK_SOURCE_CODE_PATH
        "${SDK_SOURCE_CODE_PATH}"
        "${PROJECT_SOURCE_DIR}/src/ota_api.c"
        "${PROJECT_SOURCE_DIR}/src/ota_crc32.c"
        "${PROJECT_SOURCE_DIR}/src/ota_srec_parser.c"
        "${PROJECT_SOURCE_DIR}/src/ota_upgrade_mcu.c"
        "${PROJECT_SOURCE_DIR}/src/ota_bin_parser.c"
        "${PROJECT_SOURCE_DIR}/src/ota_upgrade_bt.c"
        PARENT_SCOPE)

# 需要install的头文件
set(SDK_INSTALL_HEADER_PATH 
        "${SDK_INSTALL_HEADER_PATH}"
        "${PROJECT_SOURCE_DIR}/inc/ota_api.h"
        PARENT_SCOPE)

