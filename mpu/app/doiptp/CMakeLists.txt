cmake_minimum_required(VERSION 3.0)
project(carota-doiptp)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc
)
# 添加共享库
add_library(carota-doiptp SHARED
    ${PROJECT_SOURCE_DIR}/src/doip_tp.c
)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(carota-doiptp  
    pthread
    rt
    sr-hal
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    ${ZLOG_LIBRARY}
    ${CRYPTO_LIBRARY}
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
)
endif()

if (${MODEMTYPE} STREQUAL "ag35cet")
target_link_libraries(carota-doiptp  
    pthread
    rt
    sr-hal
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    ${ZLOG_LIBRARY}
    ${CRYPTO_LIBRARY}
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
)
endif()
# 安装目标
install(TARGETS carota-doiptp DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)
# 安装头文件
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/inc/doip_tp.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/)