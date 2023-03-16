# This file describes how to build the nortos libraries. It is organized by device and compiler.
cmake_minimum_required(VERSION 3.21.3)
include(${CMAKE_CURRENT_LIST_DIR}/CMakeSources.cmake)

project(nortos LANGUAGES C)
ti_init_package(NAMESPACE NoRtos DEPENDS_ON Driverlib)

# TFM disabled or not supported
foreach (TI_PLATFORM ${SUPPORTED_PLATFORMS})
    if (${TI_PLATFORM} MATCHES "cc(13|26).1")
        set(SOURCES ${SOURCES_CC13X1_CC26X1})
        ti_add_package_dependency(DriverlibCC13XXCC26XX)
    elseif (${TI_PLATFORM} MATCHES "cc(13|26).2")
        set(SOURCES ${SOURCES_CC13X2_CC26X2})
        ti_add_package_dependency(DriverlibCC13XXCC26XX)
    elseif (${TI_PLATFORM} MATCHES "cc(13|26).4")
        set(SOURCES ${SOURCES_CC13X4_CC26X4})
        ti_add_package_dependency(DriverlibCC13XXCC26XX)
    elseif (${TI_PLATFORM} STREQUAL "cc23x0")
        ti_add_package_dependency(DriverlibCC23X0)
        set(SOURCES ${SOURCES_CC23X0})
    elseif (${TI_PLATFORM} STREQUAL "cc23x0r2")
        ti_add_package_dependency(DriverlibCC23X0R2)
        set(SOURCES ${SOURCES_CC23X0R2})
    elseif (${TI_PLATFORM} MATCHES "cc32")
        ti_add_package_dependency(DriverlibCC32XX)
        set(SOURCES ${SOURCES_CC32XX})
    else ()
        continue()
    endif ()

    ti_add_library(
        nortos_${TI_PLATFORM} SOURCES ${SOURCES} LINK_PUBLIC Driverlib::${TI_PLATFORM}
                                                             CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
        ADD_SDK_INCLUDE_PATH source
    )
endforeach ()

# TFM enabled
foreach (TI_PLATFORM ${SUPPORTED_PLATFORMS})
    if (${TI_PLATFORM} MATCHES "cc(13|26).4")
        set(SOURCES ${SOURCES_CC13X4_CC26X4_NS})
    else ()
        continue()
    endif ()

    ti_add_library(
        nortos_${TI_PLATFORM}_ns SOURCES ${SOURCES} LINK_PUBLIC Driverlib::${TI_PLATFORM}
                                                                CMakeCommon::${TI_TOOLCHAIN_NAME}_${ARCH_${TI_PLATFORM}}
        ADD_SDK_INCLUDE_PATH source
    )
endforeach ()

ti_export_package()
