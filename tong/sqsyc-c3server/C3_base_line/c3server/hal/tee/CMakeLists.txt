if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35eut")
    set(HAL_SOURCE_CODE_PATH
        "${HAL_SOURCE_CODE_PATH}"
        "${CMAKE_CURRENT_SOURCE_DIR}/ag35xxT/sr_hal_tee.c"
        PARENT_SCOPE
    )

    set(HAL_INSTALL_HEADER_PATH
        "${HAL_INSTALL_HEADER_PATH}"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/sr_hal_tee.h"
        PARENT_SCOPE
    )

    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${SDKTARGETSYSROOT}/usr/include
    )
endif()
