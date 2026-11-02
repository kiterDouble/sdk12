cmake_minimum_required(VERSION 3.0)
project(carota-canmsg)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc
    # ${PROJECT_SOURCE_DIR}/../server/server-out/include
)
# 添加共享库
add_library(carota-canmsg SHARED
    ${PROJECT_SOURCE_DIR}/src/can_msg.c
)
#SHARED代表要进行动态链接
# add_library(carota-canmsg
#     ${PROJECT_SOURCE_DIR}/src/can_tp.c
# )
# 参与编译的源码


# 链接所需的库
if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(carota-canmsg  
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
install(TARGETS carota-canmsg DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)
# 安装头文件
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/inc/can_msg.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/)