cmake_minimum_required(VERSION 3.0)
project(hal-gnss)

if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd")
        set(HAL_SOURCE_CODE_PATH
                "${HAL_SOURCE_CODE_PATH}"
                "${PROJECT_SOURCE_DIR}/ag35ce_evd/sr_hal_gnss.c"
                PARENT_SCOPE)
endif()

if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35eut")
        set(HAL_SOURCE_CODE_PATH
                "${HAL_SOURCE_CODE_PATH}"
                "${PROJECT_SOURCE_DIR}/ag35cet_eut/sr_hal_gnss.c"
                PARENT_SCOPE)
endif()

if (${MODEMTYPE} STREQUAL "ag35cet-openssl3")
        set(HAL_SOURCE_CODE_PATH
                "${HAL_SOURCE_CODE_PATH}"
                "${PROJECT_SOURCE_DIR}/ag35cet_eut/sr_hal_gnss.c"
                PARENT_SCOPE)
endif()

if (${MODEMTYPE} STREQUAL "A7805")
        set(HAL_SOURCE_CODE_PATH
                "${HAL_SOURCE_CODE_PATH}"
                "${PROJECT_SOURCE_DIR}/A7805/sr_hal_gnss.c"
                PARENT_SCOPE)
endif()

if (${MODEMTYPE} STREQUAL "ag519")
        set(HAL_SOURCE_CODE_PATH
                "${HAL_SOURCE_CODE_PATH}"
                "${PROJECT_SOURCE_DIR}/ag519/sr_hal_gnss.c"
                PARENT_SCOPE)
endif()

if (${MODEMTYPE} STREQUAL "AG591")
        set(HAL_SOURCE_CODE_PATH
                "${HAL_SOURCE_CODE_PATH}"
                "${PROJECT_SOURCE_DIR}/AG591/sr_hal_gnss.c"
                PARENT_SCOPE)
endif()

if (${MODEMTYPE} STREQUAL "ZM8201")
        set(HAL_SOURCE_CODE_PATH
                "${HAL_SOURCE_CODE_PATH}"
                "${PROJECT_SOURCE_DIR}/ZM8201/sr_hal_gnss.c"
                PARENT_SCOPE)
endif()

# 需要install的头文件
set(HAL_INSTALL_HEADER_PATH 
        "${HAL_INSTALL_HEADER_PATH}"
        "${PROJECT_SOURCE_DIR}/include/sr_hal_gnss.h"
        PARENT_SCOPE)

