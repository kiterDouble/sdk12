cmake_minimum_required(VERSION 3.10)
project(useCanSignalDemo C)

add_compile_options(-Wall -Wextra -Werror -Wno-unused-parameter)

file(GLOB SRCS src/*.c)

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE
    inc
    ${CMAKE_SOURCE_DIR}/lib/include
    ${CMAKE_SOURCE_DIR}/../C3_base_line/c3server/sripc/inc
)

target_link_libraries(${PROJECT_NAME}
    sr-hal
    sr-ipc
    sr-sdk
    zlog
    event_core
    event_pthreads
    pthread
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/apps)
