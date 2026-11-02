cmake_minimum_required(VERSION 3.0)
project(sr-hal)

add_subdirectory(modem)
add_subdirectory(gnss)
add_subdirectory(hsm)
add_subdirectory(ecall)
add_subdirectory(era)

if (${MODEMTYPE} STREQUAL "ag35cet")
	add_subdirectory(bcall)	
	include_directories(
        ${PROJECT_SOURCE_DIR}/bcall/include
    )
endif()

# 添加头文件
if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd")
    include_directories(
        ${PROJECT_SOURCE_DIR}/modem/include
        ${PROJECT_SOURCE_DIR}/gnss/include
        ${PROJECT_SOURCE_DIR}/hsm/include
        ${PROJECT_SOURCE_DIR}/ecall/include
        ${SDKTARGETSYSROOT}/usr/include/quectel-openlinux-sdk/
    )
endif()
if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag519" OR ${MODEMTYPE} STREQUAL "ag35eut")
    include_directories(
        ${PROJECT_SOURCE_DIR}/modem/include
        ${PROJECT_SOURCE_DIR}/gnss/include
        ${PROJECT_SOURCE_DIR}/hsm/include
        ${PROJECT_SOURCE_DIR}/ecall/include

    )
endif()
if (${MODEMTYPE} STREQUAL "ag35cet-openssl3")
    include_directories(
        ${PROJECT_SOURCE_DIR}/modem/include
        ${PROJECT_SOURCE_DIR}/gnss/include
        ${PROJECT_SOURCE_DIR}/hsm/include
        ${PROJECT_SOURCE_DIR}/ecall/include
    )
endif()
if (${MODEMTYPE} STREQUAL "A7805")
    include_directories(
        ${PROJECT_SOURCE_DIR}/modem/include
        ${PROJECT_SOURCE_DIR}/modem/A7805/sunsea_inc
        ${PROJECT_SOURCE_DIR}/gnss/include
        ${PROJECT_SOURCE_DIR}/hsm/include
        ${PROJECT_SOURCE_DIR}/ecall/include
        ${PROJECT_SOURCE_DIR}/era/include
    )
endif()
if (${MODEMTYPE} STREQUAL "AG591")
    include_directories(
        ${PROJECT_SOURCE_DIR}/modem/include
        ${PROJECT_SOURCE_DIR}/modem/A7805/sunsea_inc
        ${PROJECT_SOURCE_DIR}/gnss/include
        ${PROJECT_SOURCE_DIR}/hsm/include
        ${PROJECT_SOURCE_DIR}/ecall/include
        ${SDKTARGETSYSROOT}/usr/include/ql_lib_utils
    )
endif()

if (${MODEMTYPE} STREQUAL "ZM8201")
    include_directories(
        ${PROJECT_SOURCE_DIR}/modem/include
        ${PROJECT_SOURCE_DIR}/modem/A7805/sunsea_inc
        ${PROJECT_SOURCE_DIR}/gnss/include
        ${PROJECT_SOURCE_DIR}/hsm/include
        ${PROJECT_SOURCE_DIR}/ecall/include
    )
endif()

# 添加共享库
add_library(sr-hal SHARED ${HAL_SOURCE_CODE_PATH})

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd")
    target_link_libraries(sr-hal  
        ${ZLOG_LIBRARY}
        glib-2.0
        rt
        pthread
        dsi_netctrl
        dsutils
        qmiservices
        qmi_cci
        qmi_common_so
        qmi
        mcm
        ql_common_api
        ql_peripheral
        dl
        crypto
        ssl
    )
endif()
if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35eut")
    target_link_libraries(sr-hal  
        ${ZLOG_LIBRARY}
        ${GLIB2_LIBRARY}
        rt
        pthread
        ql_sdk
        ql_sys_log
        ql_lib_ipc
        ql_lib_utils
        ql_lib_fota
        ql_lib_absys
        ubus
        uci
        blobmsg_json
        rilutil
        log
        dl
        ${CRYPTO_LIBRARY}
        ${SSL_LIBRARY}
    )
endif()
if (${MODEMTYPE} STREQUAL "ag35cet-openssl3")
    target_link_libraries(sr-hal  
        ${ZLOG_LIBRARY}
        ${GLIB2_LIBRARY}
        rt
        pthread
        ql_sdk
        ql_sys_log
        ql_lib_ipc
        ql_lib_utils
        ql_lib_fota
        ql_lib_absys
        ubus
        uci
        blobmsg_json
        rilutil
        log
        dl
        ${CRYPTO_LIBRARY}
        ${SSL_LIBRARY}
    )
endif()
if (${MODEMTYPE} STREQUAL "A7805")
    target_link_libraries(sr-hal  
        ${ZLOG_LIBRARY}
        ${GLIB2_LIBRARY}
        rt
        pthread
        sunseasdk
        ubus
        ubox
        rilutil
        uci
        log
        prop2uci
        curl
        mbedtls
        nghttp2
        mbedx509
        mbedcrypto
        dl
        ${CRYPTO_LIBRARY}
        ${SSL_LIBRARY}
    )
endif()
if (${MODEMTYPE} STREQUAL "ag519")
    target_link_libraries(sr-hal  
        ${ZLOG_LIBRARY}
        glib-2.0
        rt
        pthread
        absys
        fotaapi 
        ql_location
        ql_sdk
        ql_sys_log
        dl
        ${CRYPTO_LIBRARY}
        ${SSL_LIBRARY}
    )
endif()
if (${MODEMTYPE} STREQUAL "AG591")
    target_link_libraries(sr-hal  
        ${ZLOG_LIBRARY}
        glib-2.0
        rt
        pthread
        absys
        fotaapi 
        ql_location
        ql_sdk
        ql_sys_log
        dl
        crypto
        ssl
        QteeCrypto
    )
endif()

if (${MODEMTYPE} STREQUAL "ZM8201")
    target_link_libraries(sr-hal  
        ${ZLOG_LIBRARY}
        glib-2.0
        rt
        pthread
        dl
        crypto
        ssl
        ztezsdk
    )
endif()

if (${MODEMTYPE} STREQUAL "ag35cet-openssl3")
    target_compile_options(sr-hal PRIVATE -Wno-deprecated-declarations)
endif()

# 安装hal动态库
install(TARGETS sr-hal DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)

# 安装hal头文件
install(FILES ${HAL_INSTALL_HEADER_PATH}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/hal)
# 安装hal脚本文件
if (${MODEMTYPE} STREQUAL "ag35ce")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modem/${MODEMTYPE}/script/get_upgrade_status.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modem/${MODEMTYPE}/script/start_upgrade_mpu.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
endif()

if (${MODEMTYPE} STREQUAL "AG591")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modem/${MODEMTYPE}/script/get_upgrade_status.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modem/${MODEMTYPE}/script/start_upgrade_mpu.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
endif()

if (${MODEMTYPE} STREQUAL "ag35cet-openssl3")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modem/ag35cet/script/sirun_start.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modem/ag35cet/script/servers_init.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modem/ag35cet/script/init_flash.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modem/ag35cet/script/init_emmc.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modem/ag35cet/script/log_process.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
else()
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modem/${MODEMTYPE}/script/sirun_start.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modem/${MODEMTYPE}/script/servers_init.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modem/${MODEMTYPE}/script/init_flash.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modem/${MODEMTYPE}/script/init_emmc.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modem/${MODEMTYPE}/script/log_process.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
endif()

