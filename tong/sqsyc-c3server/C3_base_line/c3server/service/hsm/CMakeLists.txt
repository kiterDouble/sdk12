cmake_minimum_required(VERSION 3.0)
project(hsmServer)

add_compile_options(-Wno-error)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../../sripc/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/hsm/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/pm/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/sdkinit/inc
    ${PROJECT_SOURCE_DIR}/../../hal/tee/include
)


# HED SDK 头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HedSDK
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HedSDK/extern
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HedSDK/port/linux
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HedSDK/port/linux/raspberrypi_cnf
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HedSDK/src/api
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HedSDK/src/cmd
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HedSDK/src/ial
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HedSDK/src/proto
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HedSDK/src/util
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hsm_api
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hsm_simulate_api
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hsm_tee_api
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rpc_bus
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pm_manager
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hsm_com
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/rpc_bus SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/HSM SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/pm_manager SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/hsm_com SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/util SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/config SRCS)

# HED SDK 源文件 - 使用 GLOB_RECURSE 递归收集子目录
file(GLOB_RECURSE HED_SDK_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HedSDK/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HedSDK/port/linux/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HedSDK/port/linux/raspberrypi_cnf/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HedSDK/extern/*.c
)
list(APPEND SRCS ${HED_SDK_SRCS})

# RPC Bus 源文件
list(APPEND SRCS
    src/rpc_bus/hsm_rpc_handler.c
)

# HSM 配置文件
list(APPEND SRCS
    src/config/hsm_config.c
)

# HSM 源文件
list(APPEND SRCS
    src/hsm_api/hsm_api.c
    src/hsm_api/hsm_crt_util.c
    src/hsm_simulate_api/hsm_simulate_api.c
)

# TEE 后端仅在支持 TEE 的目标上编译
if ("${MODEMTYPE}" STREQUAL "ag35cet" OR "${MODEMTYPE}" STREQUAL "ag35eut")
    list(APPEND SRCS
        src/hsm_tee_api/hsm_tee_api.c
    )
    add_compile_definitions(HSM_TEE_SUPPORTED)
endif()

# 主程序
list(APPEND SRCS
    src/main.c
)

# 共享库
add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME}
        sr-hal
        sr-ipc
        sr-sdk
        ${ZLOG_LIBRARY}
        ${CRYPTO_LIBRARY}
        ${SSL_LIBRARY}
        ${EVENTCORE_LIBRARY}
        ${EVENTPTHREADS_LIBRARY}
        glib-2.0
        gobject-2.0
        gmodule-2.0
        gthread-2.0
        pthread
)

target_link_directories(${PROJECT_NAME} PRIVATE
    /oemapp/lib
    /oemapp/third_lib
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/server)
