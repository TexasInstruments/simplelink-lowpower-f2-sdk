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

set(SUPPORTED_COMPONENTS ${SUPPORTED_COMPONENTS} grlib)
project(grlib LANGUAGES)
get_install_dir(GRLIB_INSTALL_DIR)

add_library(grlib INTERFACE IMPORTED)
add_library(Grlib::grlib ALIAS grlib)

target_include_directories(grlib INTERFACE ${GRLIB_INSTALL_DIR}/source)

add_library(grlib_m3 INTERFACE IMPORTED)
target_link_libraries(grlib_m3 INTERFACE grlib)
target_link_libraries(grlib_m3 INTERFACE ${GRLIB_INSTALL_DIR}/source/ti/grlib/lib/${TI_TOOLCHAIN_NAME}/m3/grlib)
add_library(Grlib::grlib_m3 ALIAS grlib_m3)

add_library(grlib_m4 INTERFACE IMPORTED)
target_link_libraries(grlib_m4 INTERFACE grlib)
target_link_libraries(grlib_m4 INTERFACE ${GRLIB_INSTALL_DIR}/source/ti/grlib/lib/${TI_TOOLCHAIN_NAME}/m4/grlib)
add_library(Grlib::grlib_m4 ALIAS grlib_m4)

add_library(grlib_m4f INTERFACE IMPORTED)
target_link_libraries(grlib_m4f INTERFACE grlib)
target_link_libraries(grlib_m4f INTERFACE ${GRLIB_INSTALL_DIR}/source/ti/grlib/lib/${TI_TOOLCHAIN_NAME}/m4f/grlib)
add_library(Grlib::grlib_m4f ALIAS grlib_m4f)

# No binaries for these architectures, but may still want to include headers
add_library(grlib_m0p INTERFACE IMPORTED)
target_link_libraries(grlib_m0p INTERFACE grlib)
add_library(Grlib::grlib_m0p ALIAS grlib_m0p)

add_library(grlib_m33f INTERFACE IMPORTED)
target_link_libraries(grlib_m33f INTERFACE grlib)
add_library(Grlib::grlib_m33f ALIAS grlib_m33f)
