cmake_minimum_required(VERSION 3.0)
project(carota-cantp)

include_directories(
    ${PROJECT_SOURCE_DIR}/
    # ${PROJECT_SOURCE_DIR}/../server/server-out/include
)
# 添加共享库
add_library(carota-cantp SHARED
    ${PROJECT_SOURCE_DIR}/src/can_tp.c
)



# 链接所需的库
if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(carota-cantp  
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
install(TARGETS carota-cantp DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)
# 安装头文件
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/inc/can_tp.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/)
