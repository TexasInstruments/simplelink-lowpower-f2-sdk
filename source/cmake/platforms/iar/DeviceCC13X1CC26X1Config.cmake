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

set(SUPPORTED_COMPONENTS ${SUPPORTED_COMPONENTS} driverlib_cc13xx_cc26xx)
project(driverlib_cc13xx_cc26xx LANGUAGES)

set(ARCH_cc13x1 m4)
set(ARCH_cc26x1 m4)
list(APPEND SUPPORTED_PLATFORMS cc13x1 cc26x1)
list(REMOVE_DUPLICATES SUPPORTED_PLATFORMS)
list(APPEND SUPPORTED_ARCHITECTURES m4)
list(REMOVE_DUPLICATES SUPPORTED_ARCHITECTURES)

add_library(device_cc13x1 INTERFACE IMPORTED)
add_library(Device::cc13x1 ALIAS device_cc13x1)
target_compile_definitions(device_cc13x1 INTERFACE DeviceFamily_CC13X1)

add_library(device_cc26x1 INTERFACE IMPORTED)
add_library(Device::cc26x1 ALIAS device_cc26x1)
target_compile_definitions(device_cc26x1 INTERFACE DeviceFamily_CC26X1)
