cmake_minimum_required(VERSION 3.0)
project(audio_app)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -std=c++11 -D_GNU_SOURCE -Wno-deprecated -Wno-psabi")

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/inc/tts/
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/../libexternal/
    ${PROJECT_SOURCE_DIR}/../libexternal/proto_image/
)

link_directories(
    ${PROJECT_SOURCE_DIR}/libs/
)

add_executable(audio_app
        "${PROJECT_SOURCE_DIR}/src/audio_common.cpp"
        "${PROJECT_SOURCE_DIR}/src/audio_capture.cpp"
        "${PROJECT_SOURCE_DIR}/src/audio_playback.cpp"
        "${PROJECT_SOURCE_DIR}/src/audio_main.cpp"
        "${PROJECT_SOURCE_DIR}/src/audio_config_file.cpp"
        "${PROJECT_SOURCE_DIR}/src/audio_control.cpp"
        "${PROJECT_SOURCE_DIR}/src/audio_file_upload.cpp"
        "${PROJECT_SOURCE_DIR}/src/audio_aikit.cpp"
        "${PROJECT_SOURCE_DIR}/src/audio_http.cpp"
        "${PROJECT_SOURCE_DIR}/src/audio_get_times.cpp"
)

if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(audio_app
        ${EVENTCORE_LIBRARY}
        ${EVENTPTHREADS_LIBRARY}
        pthread
        sqlite3
        sr-hal
        sr-ipc
        sr-sdk
        sr-comfunc
        ${PROTOBUF_LIBRARY}
        ${CRYPTO_LIBRARY}
        ${SSL_LIBRARY}
        curl
        ${ZLOG_LIBRARY}
        ${GLIB2_LIBRARY}
        ${GIO_LIBRARY}
        ${GOBJECT_LIBRARY}
        ${MQTT_LIBRARY}
        ${CIVETWEB}
        ${ICONV}
        rt
        ql_sdk
        jsoncpp
        aikit
        ssl
        dl
        glib-2.0
        crypto
        ssl 
        curl
        mpg123
        sndfile
)

get_target_property(linked_libraries audio_app INTERFACE_LINK_LIBRARIES)
message("Linked libraries: ${linked_libraries}")

endif()


# 设置可执行文件名称
set_target_properties(audio_app PROPERTIES OUTPUT_NAME "audio_app")

add_custom_command(
   TARGET audio_app                # 关联目标：audio_app可执行文件
   POST_BUILD                       # 执行时机：编译/链接完成后
   COMMAND ${CMAKE_STRIP} $<TARGET_FILE:audio_app>  # 核心strip命令（此处语法原本正确，无需修改）
   COMMENT "Stripping debug symbols from audio_app (target file)"  # 修正：去掉错误的花括号表达式，改用通用提示
)
# 安装目标
install(TARGETS audio_app DESTINATION ${CMAKE_INSTALL_PREFIX}/app/)

