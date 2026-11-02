cmake_minimum_required(VERSION 3.0)
project(hsmDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../hsm/inc
)

add_executable(hsmDemo hsm_demo.c openssl_util.c)
target_compile_options(hsmDemo PRIVATE -Wno-error)
target_link_libraries(hsmDemo
    sr-sdk
)
set_target_properties(hsmDemo PROPERTIES
    INSTALL_RPATH "/oemdata/lib:/oemapp/lib"
    BUILD_RPATH   "/oemdata/lib:/oemapp/lib"
)
install(TARGETS hsmDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)


