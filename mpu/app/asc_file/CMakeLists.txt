cmake_minimum_required(VERSION 3.0)
project(asc_file)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
)

add_executable(asc_file
    ${PROJECT_SOURCE_DIR}/src/asc_file_processor.c
    ${PROJECT_SOURCE_DIR}/src/hander_asc.c
    ${PROJECT_SOURCE_DIR}/src/sirunMdlSafety.c
)


target_link_libraries(asc_file
    pthread
    
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
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

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2 -rdynamic -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -rdynamic -fno-omit-frame-pointer")
# 设置可执行文件名称
set_target_properties(asc_file PROPERTIES OUTPUT_NAME "asc_file")

# 安装目标
install(TARGETS asc_file DESTINATION ${CMAKE_INSTALL_PREFIX}/app/)

