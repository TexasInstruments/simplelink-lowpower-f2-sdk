# Copyright (c) 2022-2023, Texas Instruments Incorporated
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

# This is a top-level toolchain file for the ticlang compiler, defining the most general options and variables as well
# as any special commands or invocations needed by the compiler.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(TI_TOOLCHAIN_NAME ticlang)
set(TI_TOOLCHAIN_LINKER_FILE_EXTENSION cmd)

# Set compilers and archiver
if(WIN32)
    cmake_path(SET CMAKE_C_COMPILER "${TICLANG_ARMCOMPILER}/bin/tiarmclang.exe")
    cmake_path(SET CMAKE_CXX_COMPILER "${TICLANG_ARMCOMPILER}/bin/tiarmclang.exe")
    cmake_path(SET CMAKE_ASM_COMPILER "${TICLANG_ARMCOMPILER}/bin/tiarmclang.exe")
    cmake_path(SET CMAKE_AR "${TICLANG_ARMCOMPILER}/bin/tiarmar.exe")
else()
    cmake_path(SET CMAKE_C_COMPILER "${TICLANG_ARMCOMPILER}/bin/tiarmclang")
    cmake_path(SET CMAKE_CXX_COMPILER "${TICLANG_ARMCOMPILER}/bin/tiarmclang")
    cmake_path(SET CMAKE_ASM_COMPILER "${TICLANG_ARMCOMPILER}/bin/tiarmclang")
    cmake_path(SET CMAKE_AR "${TICLANG_ARMCOMPILER}/bin/tiarmar")
endif()

# Specify how the compilers should be invoked
set(CMAKE_C_COMPILE_OBJECT "<CMAKE_C_COMPILER> -c <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -MD -MF <OBJECT>.d <SOURCE>")
set(CMAKE_CXX_COMPILE_OBJECT
    "<CMAKE_CXX_COMPILER> -c <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -MD -MF <OBJECT>.d <SOURCE>"
)
set(CMAKE_ASM_COMPILE_OBJECT
    "<CMAKE_ASM_COMPILER> <DEFINES> <INCLUDES> -c -x assembler-with-cpp <FLAGS> -o <OBJECT> -c <SOURCE>"
)
set(CMAKE_AR_FLAGS -c -q)
set(CMAKE_C_FLAGS "" CACHE INTERNAL "C Compiler options")
set(CMAKE_STATIC_LIBRARY_PREFIX "")

# Needed otherwise the CMake compiler checks will complain and abort
set(CMAKE_C_COMPILER_ID_RUN TRUE CACHE PATH "" FORCE)
set(CMAKE_C_COMPILER_FORCED TRUE CACHE PATH "" FORCE)
set(CMAKE_C_COMPILER_WORKS TRUE CACHE PATH "" FORCE)

set(CMAKE_CXX_COMPILER_ID_RUN TRUE CACHE PATH "" FORCE)
set(CMAKE_CXX_COMPILER_FORCED TRUE CACHE PATH "" FORCE)
set(CMAKE_CXX_COMPILER_WORKS TRUE CACHE PATH "" FORCE)

# Ensure that cmake doesn't try to run ranlib on archives from TICLANG
# This can avoid a situation where the host's ranlib is invoked on ARM compiled binaries
set(CMAKE_C_ARCHIVE_FINISH "")
set(CMAKE_CXX_ARCHIVE_FINISH "")

if(NOT TARGET TOOLCHAIN_ticlang)
    add_library(TOOLCHAIN_ticlang INTERFACE IMPORTED)
    target_compile_options(
        TOOLCHAIN_ticlang
        INTERFACE
            $<$<STREQUAL:$<TARGET_PROPERTY:TI_CFLAGS_OVERRIDE>,>:
            # This part included if TI_CFLAGS_OVERRIDE not defined
            $<$<COMPILE_LANGUAGE:C>:-std=c11>
            $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
            $<$<AND:$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>,$<CONFIG:Release>>:-Oz
            -flto>
            $<$<AND:$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>,$<CONFIG:Debug>>:-Og>
            -g
            $<$<AND:$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>,$<BOOL:${TI_CMAKE_COMMON_ENABLE_ALL_WARNINGS}>>:-Wall>
            $<$<AND:$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>,$<BOOL:${TI_CMAKE_COMMON_WARNINGS_AS_ERRORS}>>:-Werror>
            >
            # If TI_CFLAGS_OVERRIDE, use it exclusively
            $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:TI_CFLAGS_OVERRIDE>,>>:$<TARGET_PROPERTY:TI_CFLAGS_OVERRIDE>>
    )
    target_link_options(
        TOOLCHAIN_ticlang
        INTERFACE
        $<$<STREQUAL:$<TARGET_PROPERTY:TI_LFLAGS_OVERRIDE>,>:
        # This part included if TI_LFLAGS_OVERRIDE not defined
        -Wl,--rom_model
        # If linker-file property exists, add linker file
        $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:TI_LINKER_COMMAND_FILE>,>>:-Wl,$<TARGET_PROPERTY:TI_LINKER_COMMAND_FILE>>
        # If map-file property exists, set map file
        $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:TI_LINKER_MAP_FILE>,>>:-Wl,-m,$<TARGET_PROPERTY:TI_LINKER_MAP_FILE>>
        $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:TI_LINKER_REFERENCE_FILE>,>>:-Wl,--emit_references:file=$<TARGET_PROPERTY:TI_LINKER_REFERENCE_FILE>>
        $<$<BOOL:${TI_CMAKE_COMMON_WARNINGS_AS_ERRORS}>:-Wl,--emit_warnings_as_errors>
        >
        # If TI_CFLAGS_OVERRIDE, use it exclusively
        $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:TI_CFLAGS_OVERRIDE>,>>:$<TARGET_PROPERTY:TI_LFLAGS_OVERRIDE>>
    )

    add_library(TOOLCHAIN_ticlang_m0p INTERFACE IMPORTED)
    target_link_libraries(TOOLCHAIN_ticlang_m0p INTERFACE TOOLCHAIN_ticlang)
    target_compile_options(TOOLCHAIN_ticlang_m0p INTERFACE -mcpu=cortex-m0plus -mfloat-abi=soft)
    add_library(CMakeCommon::ticlang_m0p ALIAS TOOLCHAIN_ticlang_m0p)

    add_library(TOOLCHAIN_ticlang_m3 INTERFACE IMPORTED)
    target_link_libraries(TOOLCHAIN_ticlang_m3 INTERFACE TOOLCHAIN_ticlang)
    target_compile_options(TOOLCHAIN_ticlang_m3 INTERFACE -mcpu=cortex-m3 -mfloat-abi=soft)
    add_library(CMakeCommon::ticlang_m3 ALIAS TOOLCHAIN_ticlang_m3)

    add_library(TOOLCHAIN_ticlang_m4 INTERFACE IMPORTED)
    target_link_libraries(TOOLCHAIN_ticlang_m4 INTERFACE TOOLCHAIN_ticlang)
    target_compile_options(TOOLCHAIN_ticlang_m4 INTERFACE -mcpu=cortex-m4 -mfloat-abi=soft)
    add_library(CMakeCommon::ticlang_m4 ALIAS TOOLCHAIN_ticlang_m4)

    add_library(TOOLCHAIN_ticlang_m4f INTERFACE IMPORTED)
    target_link_libraries(TOOLCHAIN_ticlang_m4f INTERFACE TOOLCHAIN_ticlang)
    target_compile_options(TOOLCHAIN_ticlang_m4f INTERFACE -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16)
    add_library(CMakeCommon::ticlang_m4f ALIAS TOOLCHAIN_ticlang_m4f)

    add_library(TOOLCHAIN_ticlang_m33f INTERFACE IMPORTED)
    target_link_libraries(TOOLCHAIN_ticlang_m33f INTERFACE TOOLCHAIN_ticlang)
    target_compile_options(TOOLCHAIN_ticlang_m33f INTERFACE -mcpu=cortex-m33 -mfloat-abi=hard -mfpu=fpv5-sp-d16)
    add_library(CMakeCommon::ticlang_m33f ALIAS TOOLCHAIN_ticlang_m33f)
endif()
