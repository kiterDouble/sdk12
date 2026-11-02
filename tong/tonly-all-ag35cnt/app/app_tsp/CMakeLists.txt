cmake_minimum_required(VERSION 3.10)
project(tspApp C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Werror -Wno-unused-parameter)

option(USE_LIBCURL "Enable libcurl download mode" OFF)

file(GLOB SRCS src/*.c)

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE
    inc
    ${CMAKE_SOURCE_DIR}/ota_master_lib/inc/public
    ${CMAKE_SOURCE_DIR}/ota_master_lib/inc/private
    ${CMAKE_SOURCE_DIR}/ota_master_lib/inc/private/uds
    ${CMAKE_SOURCE_DIR}/../C3_base_line/c3server/srsdk/mcu/inc
    ${CMAKE_SOURCE_DIR}/../C3_base_line/c3server/srsdk/pm/inc
    ${CMAKE_SOURCE_DIR}/../C3_base_line/c3server/sripc/inc
    ${CMAKE_SOURCE_DIR}/lib/include
)

target_link_libraries(${PROJECT_NAME}
    ota_master
    sr-hal
    sr-ipc
    sr-sdk
    event
    z
    stdc++
    sqlite3
    pthread
    ${ZLOG_LIBRARY}
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
    glib-2.0
    gthread-2.0
)

if(USE_LIBCURL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_LIBCURL)
    target_link_libraries(${PROJECT_NAME} curl)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/apps)

#install(DIRECTORY cfg/ DESTINATION etc/tly)
