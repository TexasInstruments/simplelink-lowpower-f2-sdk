# This file describes how to build the display libraries. It is organized by device and compiler.
cmake_minimum_required(VERSION 3.21.3)
include(${CMAKE_CURRENT_LIST_DIR}/CMakeSources.cmake)

project(log LANGUAGES C)
ti_init_package(NAMESPACE Drivers PACKAGE_NAME log DEPENDS_ON TiUtils)

foreach (TI_PLATFORM ${SUPPORTED_PLATFORMS})
    if (${TI_PLATFORM} MATCHES "cc(13|26)")
        set(SOURCES ${SOURCES_ITM})
        ti_add_package_dependency(DriverlibCC13XXCC26XX)
        ti_add_library(
            log_${TI_PLATFORM} SOURCES ${SOURCES} LINK_PUBLIC Driverlib::${TI_PLATFORM} TiUtils::tiutils
                                                              CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
            ADD_SDK_INCLUDE_PATH "source"
        )
    elseif (${TI_PLATFORM} MATCHES "cc32")
        set(SOURCES ${SOURCES_ITM})
        ti_add_package_dependency(DriverlibCC32XX)
        ti_add_library(
            log_${TI_PLATFORM} SOURCES ${SOURCES} LINK_PUBLIC Driverlib::${TI_PLATFORM} TiUtils::tiutils
                                                              CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
            ADD_SDK_INCLUDE_PATH "source"
        )
    else ()
        set(SOURCES ${SOURCES_COMMON})
        if (${TI_PLATFORM} STREQUAL "cc23x0")
            ti_add_package_dependency(DriverlibCC23X0)
        elseif (${TI_PLATFORM} STREQUAL "cc23x0r2")
            ti_add_package_dependency(DriverlibCC23X0R2)
        endif ()
        ti_add_library(
            log_${TI_PLATFORM} SOURCES ${SOURCES} LINK_PUBLIC Driverlib::${TI_PLATFORM} TiUtils::tiutils
                                                              CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
            ADD_SDK_INCLUDE_PATH "source"
        )
    endif ()
endforeach ()

ti_export_package()
