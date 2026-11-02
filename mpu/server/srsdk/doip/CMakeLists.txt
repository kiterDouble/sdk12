cmake_minimum_required(VERSION 3.0)
project(sdk-doip)


# 参与编译的源码
set(SDK_SOURCE_CODE_PATH
        "${SDK_SOURCE_CODE_PATH}"
        "${PROJECT_SOURCE_DIR}/comm/src/message_0000.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_0001.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_0002.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_0003.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_0004.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_0005.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_0006.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_0007.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_0008.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_4001.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_4002.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_4003.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_4004.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_8001.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_8002.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_8003.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message.c"
        "${PROJECT_SOURCE_DIR}/comm/src/message_proc.c"
        "${PROJECT_SOURCE_DIR}/entity/src/doip_entity.c"
        "${PROJECT_SOURCE_DIR}/entity/src/entity_announce.c"
        "${PROJECT_SOURCE_DIR}/entity/src/entity_diagnostic.c"
        "${PROJECT_SOURCE_DIR}/tester/src/doip_tester.c"
        "${PROJECT_SOURCE_DIR}/tester/src/tester_diagnostic.c"
        "${PROJECT_SOURCE_DIR}/tester/src/tester_discovery.c"
        "${PROJECT_SOURCE_DIR}/utils/src/ring_buffer.c"
        PARENT_SCOPE)

# 需要install的头文件
set(SDK_INSTALL_HEADER_PATH 
        "${SDK_INSTALL_HEADER_PATH}"
        "${PROJECT_SOURCE_DIR}/include/doip_tester.h"
        "${PROJECT_SOURCE_DIR}/include/doip_entity.h"
        PARENT_SCOPE)

