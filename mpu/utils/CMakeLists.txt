cmake_minimum_required(VERSION 3.0)

project(sr_utils)

include_directories(inc)
aux_source_directory(src SRC)

set(CMAKE_C_COMPILER ${CC})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAG} -Wall -fPIC -std=c11")
message(STATUS "CMAKE_C_COMPILER = " ${CMAKE_C_COMPILER})
message(STATUS "CMAKE_C_FLAGS = " ${CMAKE_C_FLAGS})

if (${MODEMTYPE} STREQUAL "A7805")
    set(LIB_PATH $ENV{LIB_PATH})
    set(CROSSTOOL_PATH $ENV{CROSSTOOL_PATH})
    include_directories(
        ${LIB_PATH}/usr/include
    )
    link_directories(${LIB_PATH}/usr/lib)
    find_library(CRYPTO_LIBRARY crypto  ${CROSSTOOL_PATH}/../../A7805lib/lib)
endif()

if (${MODEMTYPE} STREQUAL "ag35cet-openssl3")
    link_directories(${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
    find_library(CRYPTO_LIBRARY crypto  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
endif()

add_library(sr_utils SHARED ${SRC})
if (${MODEMTYPE} STREQUAL "A7805")
    target_link_libraries(sr_utils ${CRYPTO_LIBRARY} dl)
elseif(${MODEMTYPE} STREQUAL "ag35cet-openssl3")
target_link_libraries(sr_utils ${CRYPTO_LIBRARY} dl pthread)
else()
    target_link_libraries(sr_utils crypto dl)
endif()

add_executable(sirunUtilsTutorial tutorial/sirunUtilsTutorial.c)
target_link_libraries(sirunUtilsTutorial sr_utils)

install(DIRECTORY inc/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/utils_inc)
install(TARGETS sr_utils DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)
