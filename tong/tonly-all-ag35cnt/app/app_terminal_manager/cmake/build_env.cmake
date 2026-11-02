
# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -D_GNU_SOURCE -Wunused-function")

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


include_directories(
    # ${PROJECT_SOURCE_DIR}/../out/include/
    ${PROJECT_SOURCE_DIR}/../../app-third-lib/include/paho.mqtt.c-1.3.14/
    ${PROJECT_SOURCE_DIR}/../../app-third-lib/include/protobuf/
    ${SDKTARGETSYSROOT}/usr/include/
    # ${SDKTARGETSYSROOT}/usr/include/glib-2.0/
    # ${SDKTARGETSYSROOT}/usr/include/gio-unix-2.0/
)
link_directories(
    ${PROJECT_SOURCE_DIR}/../../out/lib/
    # ${PROJECT_SOURCE_DIR}/../third-static-lib/src/${VENDOR_PROJECT}/protobuf_v3.23.4/lib/
    ${PROJECT_SOURCE_DIR}/../../app-third-lib/lib/
)
find_library(MQTT_LIBRARIES     paho-mqtt3cs   ${PROJECT_SOURCE_DIR}/../../app-third-lib/lib/)




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

## 设置静态库选型
find_library(JSONCPP_LIBRARIES jsoncpp    ${PROJECT_SOURCE_DIR}/../../out/lib/)

# if (${MODEMTYPE} STREQUAL "ag35eut")
#     find_library(GLIB2_LIBRARY glib-2.0  ${SDKTARGETSYSROOT}/usr/lib/)
#     find_library(GIO_LIBRARY gio-2.0  ${SDKTARGETSYSROOT}/usr/lib/)
#     find_library(GOBJECT_LIBRARY gobject-2.0  ${SDKTARGETSYSROOT}/usr/lib/)
#     find_library(FFI_LIBRARY ffi  ${SDKTARGETSYSROOT}/usr/lib/)
#     find_library(GMODULE2_LIBRARY gmodule-2.0  ${SDKTARGETSYSROOT}/usr/lib/)
#     find_library(CRYPTO_LIBRARY crypto  ${SDKTARGETSYSROOT}/usr/lib/)
#     find_library(SSL_LIBRARY ssl  ${SDKTARGETSYSROOT}/usr/lib/)
#     find_library(CURL_LIBRARY curl  ${SDKTARGETSYSROOT}/usr/lib/)
# endif()
