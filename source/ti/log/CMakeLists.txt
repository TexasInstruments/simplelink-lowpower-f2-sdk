# This file describes how to build the log libraries. It is organized by device and compiler.
cmake_minimum_required(VERSION 3.21.3)
include(${CMAKE_CURRENT_LIST_DIR}/CMakeSources.cmake)

project(log LANGUAGES C)
ti_init_package(NAMESPACE Drivers PACKAGE_NAME log DEPENDS_ON TiUtils)

foreach(TI_PLATFORM ${SUPPORTED_PLATFORMS})
    if(${TI_PLATFORM} MATCHES "cc(13|26).[124]")
        set(SOURCES ${SOURCES_COMMON} ${SOURCES_ITM})

        if(${TI_PLATFORM} MATCHES "cc(13|26).1")
            ti_add_package_dependency(DriverlibCC13X1CC26X1)
        elseif(${TI_PLATFORM} MATCHES "cc(13|26).2.7")
            ti_add_package_dependency(DriverlibCC13X2X7CC26X2X7)
        elseif(${TI_PLATFORM} MATCHES "cc(13|26).2")
            ti_add_package_dependency(DriverlibCC13X2CC26X2)
        elseif(${TI_PLATFORM} MATCHES "cc(13|26).4")
            ti_add_package_dependency(DriverlibCC13X4CC26X3CC26X4)
        endif()

        ti_add_library(
            log_${TI_PLATFORM} SOURCES ${SOURCES} LINK_PUBLIC Driverlib::${TI_PLATFORM} TiUtils::tiutils
                                                              CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
            ADD_SDK_INCLUDE_PATH "source"
        )
    elseif(${TI_PLATFORM} MATCHES "cc27")
        set(SOURCES ${SOURCES_COMMON} ${SOURCES_ITM} ${SOURCES_TRACE})
        ti_add_package_dependency(DriverlibCC27XX)
        ti_add_library(
            log_${TI_PLATFORM} SOURCES ${SOURCES} LINK_PUBLIC Driverlib::${TI_PLATFORM} TiUtils::tiutils
                                                              CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
            ADD_SDK_INCLUDE_PATH "source"
        )
    elseif(${TI_PLATFORM} MATCHES "cc35")
        set(SOURCES ${SOURCES_COMMON} ${SOURCES_ITM})
        ti_add_package_dependency(DriverlibCC35XX)
        ti_add_library(
            log_${TI_PLATFORM} SOURCES ${SOURCES} LINK_PUBLIC Driverlib::${TI_PLATFORM} TiUtils::tiutils
                                                              CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
            ADD_SDK_INCLUDE_PATH "source"
        )
    elseif(${TI_PLATFORM} MATCHES "cc23")
        set(SOURCES ${SOURCES_COMMON} ${SOURCES_TRACE})
        if(${TI_PLATFORM} STREQUAL "cc23x0r5")
            ti_add_package_dependency(DriverlibCC23X0R5)
        elseif(${TI_PLATFORM} STREQUAL "cc23x0r2")
            ti_add_package_dependency(DriverlibCC23X0R2)
        endif()
        ti_add_library(
            log_${TI_PLATFORM} SOURCES ${SOURCES} LINK_PUBLIC Driverlib::${TI_PLATFORM} TiUtils::tiutils
                                                              CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
            ADD_SDK_INCLUDE_PATH "source"
        )
    endif()
endforeach()

ti_export_package()
