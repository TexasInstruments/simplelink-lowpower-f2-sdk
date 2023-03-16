# This file describes how to build the display libraries. It is organized by device and compiler.
cmake_minimum_required(VERSION 3.21.3)
include(${CMAKE_CURRENT_LIST_DIR}/CMakeSources.cmake)

project(display LANGUAGES C)
ti_init_package(NAMESPACE Drivers PACKAGE_NAME Display DEPENDS_ON GrlibWrapper)

foreach (TI_PLATFORM ${SUPPORTED_PLATFORMS})
    if (${TI_PLATFORM} MATCHES "cc(13|26)")
        ti_add_package_dependency(DriverlibCC13XXCC26XX)
        set(SOURCES ${SOURCES_CC13XX_CC26XX})
    elseif (${TI_PLATFORM} MATCHES "cc23")
        ti_add_package_dependency(DriverlibCC23X0)
        ti_add_package_dependency(DriverlibCC23X0R2)
        set(SOURCES ${SOURCES_COMMON})
    elseif (${TI_PLATFORM} MATCHES "cc32")
        ti_add_package_dependency(DriverlibCC32XX)
        set(SOURCES ${SOURCES_COMMON})
    else ()
        continue()
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
