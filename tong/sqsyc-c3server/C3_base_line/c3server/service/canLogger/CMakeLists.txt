cmake_minimum_required(VERSION 3.0)
project(canLogger)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++11 -D_GNU_SOURCE -Wno-deprecated -Wno-psabi")

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../../sripc/inc
    ${PROJECT_SOURCE_DIR}/../../common_inc
)

add_executable(canLogger
    src/main.cpp
    src/blf_writer.cpp
    src/frame_queue.cpp
    src/logger_cfg.cpp
)

add_dependencies(canLogger sr-ipc)

target_link_libraries(canLogger
    ${ZLOG_LIBRARY}
    sr-ipc
    glib-2.0
    gio-2.0
    gobject-2.0
    gthread-2.0
    rt
    pthread
    z
)

set_target_properties(canLogger PROPERTIES OUTPUT_NAME "canLoggerServer")

install(TARGETS canLogger DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)
