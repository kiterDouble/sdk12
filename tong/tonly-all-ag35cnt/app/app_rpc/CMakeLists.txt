cmake_minimum_required(VERSION 3.0)
project(app_rpc)

include_directories(
    inc
    ${GLIB_INCLUDE_DIRS}
)

aux_source_directory(src SRCS)

add_library(${PROJECT_NAME} SHARED ${SRCS})

target_link_libraries(${PROJECT_NAME}
        sr-ipc
        ${ZLOG_LIBRARY}
)

target_link_directories(${PROJECT_NAME} PRIVATE 
    /oemapp/lib
    /oemapp/third_lib 
)

install(TARGETS ${PROJECT_NAME}  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)


