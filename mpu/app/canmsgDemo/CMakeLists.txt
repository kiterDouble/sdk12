cmake_minimum_required(VERSION 3.0)
project(canmsgDemo)

# 1. 指定头文件目录
include_directories(
    ${PROJECT_SOURCE_DIR}/../canmsg/inc
    # ${PROJECT_SOURCE_DIR}
)

# 2. 指定库文件的搜索目录
# ${PROJECT_SOURCE_DIR} 就是 CMakeLists.txt 所在的目录
link_directories(
    ${PROJECT_SOURCE_DIR}
)

# 3. 添加可执行文件
add_executable(canmsgDemo
    canmsg_demo.c
)

# 4. 链接所需的库
# CMake 会去 link_directories 指定的目录里找 libcarota-cantp.so
target_link_libraries(canmsgDemo
    # pthread
    # rt
    # sr-hal
    # sr-ipc
    # sr-sdk
    # glib-2.0
    # gio-2.0
    # gobject-2.0
    # ${ZLOG_LIBRARY}
    # ${CRYPTO_LIBRARY}
    # ${EVENTCORE_LIBRARY}
    # ${EVENTPTHREADS_LIBRARY}
    # ${UTILS_LIBRARY}
    sr-sdk
    glib-2.0
    ${ZLOG_LIBRARY}
    carota-canmsg
    ${UTILS_LIBRARY}
    
)

# 设置可执行文件名称
set_target_properties(canmsgDemo PROPERTIES OUTPUT_NAME "canmsgDemo")

# 安装目标
install(TARGETS canmsgDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)