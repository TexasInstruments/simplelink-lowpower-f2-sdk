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

# This is a top-level toolchain file for the iar compiler, defining the most general options and variables as well
# as any special commands or invocations needed by the compiler.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(TI_TOOLCHAIN_NAME iar)
set(TI_TOOLCHAIN_LINKER_FILE_EXTENSION icf)

#Set compilers and archiver
if(WIN32)
    set(CMAKE_C_COMPILER "${IAR_ARMCOMPILER}/bin/iccarm.exe")
    set(CMAKE_CXX_COMPILER "${IAR_ARMCOMPILER}/bin/iccarm.exe")
    set(CMAKE_ASM_COMPILER "${IAR_ARMCOMPILER}/bin/iasmarm.exe")
    set(CMAKE_AR "${IAR_ARMCOMPILER}/bin/iarchive.exe")
    set(CMAKE_C_LINKER "${IAR_ARMCOMPILER}/bin/ilinkarm.exe")
else()
    set(CMAKE_C_COMPILER "${IAR_ARMCOMPILER}/bin/iccarm")
    set(CMAKE_CXX_COMPILER "${IAR_ARMCOMPILER}/bin/iccarm")
    set(CMAKE_ASM_COMPILER "${IAR_ARMCOMPILER}/bin/iasmarm")
    set(CMAKE_AR "${IAR_ARMCOMPILER}/bin/iarchive")
    set(CMAKE_C_LINKER "${IAR_ARMCOMPILER}/bin/ilinkarm")
endif()

# Specify how the compilers should be invoked
if(NOT IAR_AR_FLAGS)
    set(IAR_AR_FLAGS --create) # Need this option to avoid error on rebuild
endif()
set(CMAKE_ASM_CREATE_STATIC_LIBRARY "<CMAKE_AR> ${IAR_AR_FLAGS} <LINK_FLAGS> <OBJECTS> -o <TARGET>")
set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> ${IAR_AR_FLAGS} <LINK_FLAGS> <OBJECTS> -o <TARGET>")
set(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_AR> ${IAR_AR_FLAGS} <LINK_FLAGS> <OBJECTS> -o <TARGET>")
set(CMAKE_AR_FLAGS)
set(CMAKE_C_FLAGS -r)
if(WIN32)
    set(CMAKE_C_LINK_EXECUTABLE
        "${IAR_ARMCOMPILER}/bin/ilinkarm.exe <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>"
    )
    set(CMAKE_CXX_LINK_EXECUTABLE
        "${IAR_ARMCOMPILER}/bin/ilinkarm.exe <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>"
    )
else()
    set(CMAKE_C_LINK_EXECUTABLE "${IAR_ARMCOMPILER}/bin/ilinkarm <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")
    set(CMAKE_CXX_LINK_EXECUTABLE "${IAR_ARMCOMPILER}/bin/ilinkarm <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")
endif()
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_C_RESPONSE_FILE_LINK_FLAG "-f")
set(CMAKE_CXX_RESPONSE_FILE_LINK_FLAG "-f")

# Needed otherwise the CMake compiler checks will complain and abort
set(CMAKE_C_COMPILER_ID_RUN TRUE)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER_WORKS TRUE)

set(CMAKE_CXX_COMPILER_ID_RUN TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

if(NOT TARGET TOOLCHAIN_iar)
    add_library(TOOLCHAIN_iar INTERFACE IMPORTED)
    target_compile_options(
        TOOLCHAIN_iar
        INTERFACE
            $<$<STREQUAL:$<TARGET_PROPERTY:TI_CFLAGS_OVERRIDE>,>:
            # This part included if TI_CFLAGS_OVERRIDE not defined
            $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:--aeabi
            --endian=little
            -e
            --thumb
            --diag_suppress=Pa050,Go005
            --silent>
            $<$<COMPILE_LANGUAGE:ASM>:--endian
            little>
            $<$<AND:$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>,$<CONFIG:Release>>:-Oh>
            $<$<AND:$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>,$<CONFIG:Debug>>:-On>
            $<$<AND:$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>,$<BOOL:${TI_CMAKE_COMMON_WARNINGS_AS_ERRORS}>>:--warnings_are_errors>
            >
            $<$<COMPILE_LANGUAGE:CXX>:--c++>
            # If TI_CFLAGS_OVERRIDE, use it exclusively
            $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:TI_CFLAGS_OVERRIDE>,>>:$<TARGET_PROPERTY:TI_CFLAGS_OVERRIDE>>
    )
    target_link_options(
        TOOLCHAIN_iar
        INTERFACE
        $<$<STREQUAL:$<TARGET_PROPERTY:TI_LFLAGS_OVERRIDE>,>:
        # This part included if TI_LFLAGS_OVERRIDE not defined
        # If linker-file property exists, add linker file
        $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:TI_LINKER_COMMAND_FILE>,>>:--config
        $<TARGET_PROPERTY:TI_LINKER_COMMAND_FILE>>
        # If map-file property exists, set map file
        $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:TI_LINKER_MAP_FILE>,>>:--map
        $<TARGET_PROPERTY:TI_LINKER_MAP_FILE>>
        $<$<BOOL:${TI_CMAKE_COMMON_WARNINGS_AS_ERRORS}>:--warnings_are_errors>
        >
        # If TI_CFLAGS_OVERRIDE, use it exclusively
        $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:TI_CFLAGS_OVERRIDE>,>>:$<TARGET_PROPERTY:TI_LFLAGS_OVERRIDE>>
    )

    add_library(TOOLCHAIN_iar_m0p INTERFACE IMPORTED)
    target_link_libraries(TOOLCHAIN_iar_m0p INTERFACE TOOLCHAIN_iar)
    target_compile_options(TOOLCHAIN_iar_m0p INTERFACE --cpu Cortex-M0+ --fpu none)
    add_library(CMakeCommon::iar_m0p ALIAS TOOLCHAIN_iar_m0p)

    add_library(TOOLCHAIN_iar_m3 INTERFACE IMPORTED)
    target_link_libraries(TOOLCHAIN_iar_m3 INTERFACE TOOLCHAIN_iar)
    target_compile_options(TOOLCHAIN_iar_m3 INTERFACE --cpu Cortex-M3 --fpu none)
    add_library(CMakeCommon::iar_m3 ALIAS TOOLCHAIN_iar_m3)

    add_library(TOOLCHAIN_iar_m4 INTERFACE IMPORTED)
    target_link_libraries(TOOLCHAIN_iar_m4 INTERFACE TOOLCHAIN_iar)
    target_compile_options(TOOLCHAIN_iar_m4 INTERFACE --cpu Cortex-M4 --fpu none)
    add_library(CMakeCommon::iar_m4 ALIAS TOOLCHAIN_iar_m4)

    add_library(TOOLCHAIN_iar_m4f INTERFACE IMPORTED)
    target_link_libraries(TOOLCHAIN_iar_m4f INTERFACE TOOLCHAIN_iar)
    target_compile_options(TOOLCHAIN_iar_m4f INTERFACE --cpu Cortex-M4F --fpu VFPv4_sp)
    add_library(CMakeCommon::iar_m4f ALIAS TOOLCHAIN_iar_m4f)

    add_library(TOOLCHAIN_iar_m33f INTERFACE IMPORTED)
    target_link_libraries(TOOLCHAIN_iar_m33f INTERFACE TOOLCHAIN_iar)
    target_compile_options(TOOLCHAIN_iar_m33f INTERFACE --cpu Cortex-M33.fp --fpu FPv5_sp)
    add_library(CMakeCommon::iar_m33f ALIAS TOOLCHAIN_iar_m33f)
endif()
