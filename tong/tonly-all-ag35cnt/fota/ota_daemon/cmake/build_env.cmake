
# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -g -D_GNU_SOURCE")

# 编译警告改为编译报错, 特殊不影响逻辑的可以添加进行屏蔽
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-local-typedefs -Wno-error=stringop-truncation -Wno-error=format-truncation")

# 设置RUN PATH
set(CMAKE_INSTALL_RPATH "/oemapp/lib")
set(SDK_PATH $ENV{SDK_PATH})
message("SDK_PATH: ${SDK_PATH}")

set(SDKTARGETSYSROOT $ENV{SDKTARGETSYSROOT})
message("SDKTARGETSYSROOT: ${SDKTARGETSYSROOT}")

set(MODEMTYPE $ENV{MODEMTYPE})
message("MODEMTYPE: ${MODEMTYPE}")

execute_process(
    COMMAND ${CMAKE_C_COMPILER} -dumpversion
    OUTPUT_VARIABLE GCC_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("GCC Version: ${GCC_VERSION}")

if (${MODEMTYPE} STREQUAL "ag35cet")
    include_directories(
        # ${PROJECT_SOURCE_DIR}/common_inc
        ${SDKTARGETSYSROOT}/usr/include/ql-sdk
        ${SDKTARGETSYSROOT}/usr/include/glib-2.0/
        ${SDKTARGETSYSROOT}/usr/include/gio-unix-2.0/
        ${SDKTARGETSYSROOT}/usr/lib/glib-2.0/include/
        ${PROJECT_SOURCE_DIR}/../../C3_base_line/C3BaseLineOut/server-out/include
        ${PROJECT_SOURCE_DIR}/../../C3_base_line/C3BaseLineOut/server-out/include/common_inc
        ${PROJECT_SOURCE_DIR}/../../C3_base_line/C3BaseLineOut/server-out/include/hal
        ${PROJECT_SOURCE_DIR}/../../C3_base_line/C3BaseLineOut/server-out/include/sdk
        ${PROJECT_SOURCE_DIR}/../../C3_base_line/C3BaseLineOut/thrid-lib-out/include
        ${PROJECT_SOURCE_DIR}/../../C3_base_line/C3BaseLineOut/utils-out/include/utils_inc
    )
endif()

link_directories(
    ${PROJECT_SOURCE_DIR}/../../C3_base_line/C3BaseLineOut/server-out/lib/
    ${PROJECT_SOURCE_DIR}/../../C3_base_line/C3BaseLineOut/thrid-lib-out/lib/
    ${PROJECT_SOURCE_DIR}/../../C3_base_line/C3BaseLineOut/utils-out/lib/
)

get_property(INCLUDE_DIRS DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
set(MISSING_DIRS "")

foreach(INCLUDE_DIR ${INCLUDE_DIRS})
    if(NOT EXISTS ${INCLUDE_DIR})
        message(FATAL_ERROR "Include directory does not exist: ${INCLUDE_DIR}")
        list(APPEND MISSING_DIRS ${INCLUDE_DIR})
    endif()
endforeach()

string(REPLACE ";" "\n" INCLUDE_DIRS_STR "${INCLUDE_DIRS}")
message("include_directories:\n${INCLUDE_DIRS_STR}")

if(MISSING_DIRS)
    string(REPLACE ";" "\n" MISSING_DIRS_STR "${MISSING_DIRS}")
    message("Missing include directories:\n${MISSING_DIRS_STR}")
endif()


## 设置动态库选型
find_library(ZLOG_LIBRARY zlog  ${PROJECT_SOURCE_DIR}/../../C3_base_line/C3BaseLineOut/thrid-lib-out/lib/)
find_library(FLATCC_LIBRARY flatccrt  ${PROJECT_SOURCE_DIR}/../../C3_base_line/C3BaseLineOut/thrid-lib-out/lib/)
find_library(UTILS_LIBRARY sr_utils  ${PROJECT_SOURCE_DIR}/../../C3_base_line/C3BaseLineOut/utils-out/lib/)
find_library(EVENTCORE_LIBRARY event_core  ${PROJECT_SOURCE_DIR}/../../C3_base_line/C3BaseLineOut/thrid-lib-out/lib/)
find_library(EVENTPTHREADS_LIBRARY event_pthreads  ${PROJECT_SOURCE_DIR}/../../C3_base_line/C3BaseLineOut/thrid-lib-out/lib/)



