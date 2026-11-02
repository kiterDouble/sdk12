cmake_minimum_required(VERSION 3.0)
project(sr-ipc)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc
)
# 添加共享库
add_library(sr-ipc SHARED
    ${PROJECT_SOURCE_DIR}/src/sr_ipc.c
)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd" OR ${MODEMTYPE} STREQUAL "ag519")
target_link_libraries(sr-ipc  
    ${ZLOG_LIBRARY}
    glib-2.0
    # ${FLATCC_LIBRARY}
    rt
    pthread
)
endif()

if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35cet-openssl3" OR ${MODEMTYPE} STREQUAL "ag35eut")
target_link_libraries(sr-ipc  
    ${ZLOG_LIBRARY}
    ${GLIB2_LIBRARY}
    # ${FLATCC_LIBRARY}
    rt
    pthread
)
endif()

if (${MODEMTYPE} STREQUAL "A7805")
target_link_libraries(sr-ipc  
    ${ZLOG_LIBRARY}
    ${GLIB2_LIBRARY}
    ${PCRE_LIBRARY}
    # ${FLATCC_LIBRARY}
    rt
    pthread
)
endif()
if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(sr-ipc  
    ${ZLOG_LIBRARY}
    glib-2.0
)
endif()

if (${MODEMTYPE} STREQUAL "ZM8201")
target_link_libraries(sr-ipc  
    ${ZLOG_LIBRARY}
    rt
    glib-2.0
)
endif()
# 安装目标
install(TARGETS sr-ipc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)
# 安装头文件
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/inc/sr_ipc.h 
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/queue_msg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/gnss_ipc_msg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/mcu_ipc_msg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/modem_ipc_msg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/net_monitor_ipc_msg.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/)

add_subdirectory(demo/pub)
add_subdirectory(demo/sub)
add_subdirectory(demo/rpcClientDemo)
add_subdirectory(demo/rpcServerDemo)
