cmake_minimum_required(VERSION 3.0)
project(can_file_save)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
)

add_executable(can_file_save
    ${PROJECT_SOURCE_DIR}/src/socket_interface.cpp
    ${PROJECT_SOURCE_DIR}/src/kfifo.c
    ${PROJECT_SOURCE_DIR}/src/can_submodule.cpp
    ${PROJECT_SOURCE_DIR}/src/system_interface.cpp
    ${PROJECT_SOURCE_DIR}/src/ethernet_interface.cpp
    ${PROJECT_SOURCE_DIR}/src/file_interface.cpp
    ${PROJECT_SOURCE_DIR}/src/thread_interface.cpp
    ${PROJECT_SOURCE_DIR}/src/timer_interface.cpp
    ${PROJECT_SOURCE_DIR}/src/completion_timeout.cpp
    ${PROJECT_SOURCE_DIR}/src/can_file_save.cpp
    ${PROJECT_SOURCE_DIR}/src/sirunMdlSafety.c
    
    
)


target_link_libraries(can_file_save
    pthread
    sqlite3
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    curl
    rt
    ${ARCHIVE_LIBRARY}
    ${UTILS_LIBRARY}
    ${ZLOG_LIBRARY}
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
    

)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -rdynamic -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -rdynamic -fno-omit-frame-pointer")

# 设置可执行文件名称
set_target_properties(can_file_save PROPERTIES OUTPUT_NAME "can_file_save")

# 安装目标
install(TARGETS can_file_save DESTINATION ${CMAKE_INSTALL_PREFIX}/app/)

