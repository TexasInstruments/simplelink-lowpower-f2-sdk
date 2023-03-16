# Copyright (c) 2022, Texas Instruments Incorporated
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# *  Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# *  Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# *  Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
cmake_minimum_required(VERSION ${TI_MIN_CMAKE_VERSION})
include(CMakeSources.cmake)

ti_init_package(NAMESPACE Tirtos7Generic)
ti_add_library(tirtos7 INTERFACE ARCH All ADD_SDK_INCLUDE_PATH kernel/tirtos7/packages/)
ti_export_package()

ti_init_package(NAMESPACE Tirtos7 DEPENDS_ON Tirtos7Generic)

set(TIRTOS_SUPPORTED_ARCHITECTURES m0p m4 m4f m33f)
set(FAMILY_FOLDER_m0p v6m)
set(FAMILY_FOLDER_m4 m3)
set(FAMILY_FOLDER_m4f m3)
set(FAMILY_FOLDER_m33f v8m)

foreach (TI_ARCH ${TIRTOS_SUPPORTED_ARCHITECTURES})
    set(PREPEND_STR family/arm/${FAMILY_FOLDER_${TI_ARCH}}/)
    # For each source file, prepend the family folder using the variables above
    # e.g. for m0p, this emits the list into SOURCES_m0p and all files get family/arm/v6m
    list(TRANSFORM SOURCES_${TI_TOOLCHAIN_NAME} PREPEND ${PREPEND_STR} OUTPUT_VARIABLE SOURCES_${TI_ARCH})

    ti_add_library(
        tirtos7_${TI_ARCH} OUTPUT_NAME sysbios SOURCES ${SOURCES_${TI_ARCH}} ${SOURCES_COMMON_${TI_TOOLCHAIN_NAME}}
        LINK_PUBLIC CMakeCommon::${TI_TOOLCHAIN_NAME}_${TI_ARCH} Tirtos7Generic::tirtos7
    )

    if (TI_ARCH STREQUAL m33f)
        # Add the TFM-enabled library (for m33f) separately
        ti_add_library(
            tirtos7_m33f_ns OUTPUT_NAME sysbios_ns ARCH m33f SOURCES ${SOURCES_${TI_TOOLCHAIN_NAME}_ns}
                                                                         ${SOURCES_COMMON_${TI_TOOLCHAIN_NAME}}
            LINK_PUBLIC CMakeCommon::${TI_TOOLCHAIN_NAME}_m33f Tirtos7Generic::tirtos7
        )
    endif ()
endforeach ()

# Apply the common compile options to all the libraries
# For applications, these defines are controlled by ti_sysbios_config.h
get_property(libraries DIRECTORY PROPERTY TI_PACKAGE_LIBRARIES)
foreach (LIBRARY ${libraries})
    target_compile_definitions(
        ${LIBRARY} PRIVATE BIOS_mpeEnabled_D=0 BIOS_smpEnabled_D=0 TaskSupport_usesMonitors__D
                           Hwi_dispatcherSwiSupport_D
    )
endforeach ()

ti_export_package()
