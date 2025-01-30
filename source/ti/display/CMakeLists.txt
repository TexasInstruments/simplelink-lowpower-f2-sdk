# This file describes how to build the display libraries. It is organized by device and compiler.
cmake_minimum_required(VERSION 3.21.3)
include(${CMAKE_CURRENT_LIST_DIR}/CMakeSources.cmake)

project(display LANGUAGES C)
ti_init_package(NAMESPACE Drivers PACKAGE_NAME Display DEPENDS_ON GrlibWrapper)

foreach(TI_PLATFORM ${SUPPORTED_PLATFORMS})
    # Ignore platform CC1308
    if(${TI_PLATFORM} STREQUAL "cc1308")
        continue()
    endif()

    if(${TI_PLATFORM} MATCHES "cc(13|26)")
        ti_add_package_dependency(DriverlibCC13X1CC26X1)
        ti_add_package_dependency(DriverlibCC13X2CC26X2)
        ti_add_package_dependency(DriverlibCC13X2X7CC26X2X7)
        ti_add_package_dependency(DriverlibCC13X4CC26X3CC26X4)
        set(SOURCES ${SOURCES_CC13XX_CC26XX})
    elseif(${TI_PLATFORM} MATCHES "cc23")
        ti_add_package_dependency(DriverlibCC23X0R5)
        ti_add_package_dependency(DriverlibCC23X0R2)
        set(SOURCES ${SOURCES_COMMON})
    elseif(${TI_PLATFORM} MATCHES "cc27")
        ti_add_package_dependency(DriverlibCC27XX)
        set(SOURCES ${SOURCES_COMMON})
    elseif(${TI_PLATFORM} MATCHES "cc35")
        ti_add_package_dependency(DriverlibCC35XX)
        set(SOURCES ${SOURCES_COMMON})
    else()
        continue()
    endif()

    # No dedicated libraries for cc26x3, we just reuse the cc26x4 ones
    if(${TI_PLATFORM} STREQUAL "cc26x3")
        continue()
    endif()

    ti_add_library(
        display_${TI_PLATFORM} SOURCES ${SOURCES}
        LINK_PUBLIC Driverlib::${TI_PLATFORM} Grlib::grlib_${ARCH_${TI_PLATFORM}}
                    CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}} ADD_SDK_INCLUDE_PATH "source"
    )
endforeach()

ti_export_package()
