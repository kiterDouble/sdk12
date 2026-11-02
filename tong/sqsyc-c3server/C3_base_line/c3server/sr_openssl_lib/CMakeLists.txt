# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10)
project(sr_openssl_lib)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../common_inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../srsdk/hsm/inc
    src/hsm_openssl_engine
    src/hsm_openssl_provider
    src/util
    ${GLIB_INCLUDE_DIRS}
)

# ====== OpenSSL Provider Shared Lib ======
add_library(sirun_ssl_hsm_provider SHARED
    src/hsm_openssl_provider/hsm_openssl_provider.c
    src/hsm_openssl_provider/hsm_openssl_provider_keygen.c
    src/hsm_openssl_provider/hsm_openssl_provider_random.c
    src/hsm_openssl_provider/hsm_openssl_provider_signature.c
    src/hsm_openssl_provider/hsm_openssl_provider_cipher.c
)
target_compile_options(sirun_ssl_hsm_provider PRIVATE -Wno-error)
target_link_libraries(sirun_ssl_hsm_provider PUBLIC sr-ipc sr-sdk crypto)
set_target_properties(sirun_ssl_hsm_provider PROPERTIES PREFIX "" OUTPUT_NAME "sirun_ssl_hsm_provider") # 去掉 lib 前缀，生成 sirun_ssl_hsm_provider.so
install(TARGETS sirun_ssl_hsm_provider LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)
install(FILES src/hsm_openssl_provider/hsm_openssl_provider.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/sr_openssl_lib/)


# ====== OpenSSL Engine Static Lib ======
add_library(sirun_ssl_hsm_engine STATIC src/hsm_openssl_engine/hsm_openssl_engine.c)
target_compile_options(sirun_ssl_hsm_engine PRIVATE -Wno-error)
target_link_libraries(sirun_ssl_hsm_engine PUBLIC sr-ipc sr-sdk crypto)
install(TARGETS sirun_ssl_hsm_engine ARCHIVE DESTINATION bin)


# ========== rpc_demo_hsm_openssl_engine ==========
add_executable(rpc_demo_hsm_openssl_engine src/demo/rpc_demo_hsm_openssl_engine.c)
target_compile_options(rpc_demo_hsm_openssl_engine PRIVATE -Wno-error)
target_link_libraries(rpc_demo_hsm_openssl_engine PUBLIC sr-ipc sr-sdk sirun_ssl_hsm_engine)
install(TARGETS rpc_demo_hsm_openssl_engine DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)


# ========== rpc_demo_hsm_openssl_provider ==========
add_executable(rpc_demo_hsm_openssl_provider src/demo/rpc_demo_hsm_openssl_provider.c)
target_compile_options(rpc_demo_hsm_openssl_provider PRIVATE -Wno-error)
target_link_libraries(rpc_demo_hsm_openssl_provider PUBLIC sr-ipc sr-sdk sirun_ssl_hsm_provider)
install(TARGETS rpc_demo_hsm_openssl_provider DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)
