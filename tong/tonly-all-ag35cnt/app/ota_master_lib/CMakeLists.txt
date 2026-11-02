cmake_minimum_required(VERSION 3.0)
project(ota_master)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(READ version OTA_MASTER_SDK_VER)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/private
    ${PROJECT_SOURCE_DIR}/inc/public
    ${PROJECT_SOURCE_DIR}/inc/private/uds
    ${PROJECT_SOURCE_DIR}/inc/private/ihex
    ${PROJECT_SOURCE_DIR}/inc/private/srecord
)

set(SRC_LIST
    src/ota_master_api.cpp
    src/ota_master_app_callback.cpp
    src/ota_master_config.cpp
    src/ota_master_curl_file_client.cpp
    src/ota_master_archive_manage.cpp
    src/ota_master_file_download.cpp
    src/ota_master_log.cpp
    src/ota_master_safety.cpp
    src/ota_master_state_manage.cpp
    src/ota_master_security_algorithm.cpp
    src/ota_master_fota_ecu.cpp
    src/ota_master_ecu_brush.cpp
    src/ota_master_flash_info.cpp
    src/uds/msg_queue.cpp
    src/uds/uds_service.cpp
    src/uds/uds_tp.cpp
    src/ihex/ihex_copy.cpp
    src/ihex/ihex_parse.cpp
    src/ihex/ihex_record.cpp
    src/srecord/srec_impl.cpp
    src/srecord/srecord.cpp
    src/ota_master_timer.cpp
    src/ota_master_upgrade.cpp
    src/ota_master_utils.cpp
    src/ota_master_pm_helper.c
)
add_library(${PROJECT_NAME} SHARED ${SRC_LIST})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/inc/public)
target_link_libraries(${PROJECT_NAME}
    pthread
    curl
    ${JSONCPP_LIBRARIES}
    ${CRYPTO_LIBRARY}
)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES VERSION ${OTA_MASTER_SDK_VER})

# 安装目标
install(DIRECTORY inc/public/ DESTINATION include/ota_master)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)

# 设置变量
set(OTA_PRECONDITIONS_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/config/ota_preconditions_config.json")
set(SUPPORT_OTA_ECU_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/config/support_ota_ecu_config.json")

# 安装配置文件到指定目录
install(FILES ${OTA_PRECONDITIONS_CONFIG} DESTINATION ${CMAKE_INSTALL_PREFIX}/ext_config RENAME ota_preconditions_config.json)
install(FILES ${SUPPORT_OTA_ECU_CONFIG} DESTINATION ${CMAKE_INSTALL_PREFIX}/ext_config RENAME support_ota_ecu_config.json)
