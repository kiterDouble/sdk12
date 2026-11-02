cmake_minimum_required(VERSION 3.0)
project(propertyServer)

add_compile_options(-Wno-error)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../../sripc/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/hsm/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/property/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/sdkinit/inc
)

include_directories(
    ${SDKTARGETSYSROOT}/usr/include
)

include_directories(
    ${GLIB_INCLUDE_DIRS}
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/rpc_bus SRCS)

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME}
        sr-hal
        sr-ipc
        sr-sdk
        ${ZLOG_LIBRARY}
        ${EVENTCORE_LIBRARY}
        ${EVENTPTHREADS_LIBRARY}
        glib-2.0
        gobject-2.0
        gmodule-2.0
        gthread-2.0
        pthread
        sqlite3
        prop2uci
)

target_link_directories(${PROJECT_NAME} PRIVATE
    /oemapp/lib
    /oemapp/third_lib
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)
