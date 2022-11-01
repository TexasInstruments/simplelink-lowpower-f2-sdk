# This file describes how to build the display libraries. It is organized by device and compiler.
cmake_minimum_required(VERSION 3.21.3)
include(${CMAKE_CURRENT_LIST_DIR}/CMakeSources.cmake)

project(display LANGUAGES C)
ti_init_package(NAMESPACE Drivers PACKAGE_NAME Display)

foreach (TI_PLATFORM ${SUPPORTED_PLATFORMS})
    if (${TI_PLATFORM} MATCHES "cc(13|26)")
        set(SOURCES ${SOURCES_CC13XX_CC26XX})
    else ()
        set(SOURCES ${SOURCES_COMMON})
    endif ()

    # No dedicated libraries for cc26x3, we just reuse the cc26x4 ones
    if (${TI_PLATFORM} STREQUAL "cc26x3")
        continue()
    endif ()

    ti_add_library(
        display_${TI_PLATFORM} SOURCES ${SOURCES}
        LINK_PUBLIC Driverlib::${TI_PLATFORM} Grlib::grlib_${ARCH_${TI_PLATFORM}}
                    CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}} ADD_SDK_INCLUDE_PATH "source"
    )
endforeach ()

ti_export_package()
