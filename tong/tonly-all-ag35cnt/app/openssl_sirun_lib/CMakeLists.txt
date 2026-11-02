# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10)
project(openssl_sirun_lib)

include_directories(
    ../sr_inc
    ../app_rpc/inc
    ../app_util/inc
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
target_link_libraries(sirun_ssl_hsm_provider PUBLIC app app_rpc crypto)
target_link_directories(sirun_ssl_hsm_provider PRIVATE ${COMMON_LINK_DIRECTORIES})
set_target_properties(sirun_ssl_hsm_provider PROPERTIES PREFIX "" OUTPUT_NAME "sirun_ssl_hsm_provider") # 去掉 lib 前缀，生成 sirun_ssl_hsm_provider.so
install(TARGETS sirun_ssl_hsm_provider LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)


# ====== OpenSSL Engine Static Lib ======
add_library(sirun_ssl_hsm_engine STATIC src/hsm_openssl_engine/hsm_openssl_engine.c)
target_compile_options(sirun_ssl_hsm_engine PRIVATE -Wno-error)
target_link_libraries(sirun_ssl_hsm_engine PUBLIC app app_rpc crypto)
target_link_directories(sirun_ssl_hsm_engine PRIVATE ${COMMON_LINK_DIRECTORIES})
install(TARGETS sirun_ssl_hsm_engine LIBRARY DESTINATION bin)


# ========== rpc_demo_hsm_openssl_engine ==========
add_executable(rpc_demo_hsm_openssl_engine  src/demo/rpc_demo_hsm_openssl_engine.c ../app_util/src/openssl_util.c)
target_compile_options(rpc_demo_hsm_openssl_engine PRIVATE -Wno-error)
target_link_libraries(rpc_demo_hsm_openssl_engine PUBLIC app app_rpc sirun_ssl_hsm_engine)
target_link_directories(rpc_demo_hsm_openssl_engine PRIVATE ${COMMON_LINK_DIRECTORIES})
install(TARGETS rpc_demo_hsm_openssl_engine DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)


# ========== rpc_demo_hsm_openssl_provider ==========
add_executable(rpc_demo_hsm_openssl_provider src/demo/rpc_demo_hsm_openssl_provider.c)
target_compile_options(rpc_demo_hsm_openssl_provider PRIVATE -Wno-error)
target_link_libraries(rpc_demo_hsm_openssl_provider PUBLIC app app_rpc sirun_ssl_hsm_provider)
target_link_directories(rpc_demo_hsm_openssl_provider PRIVATE ${COMMON_LINK_DIRECTORIES})
install(TARGETS rpc_demo_hsm_openssl_provider DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)
