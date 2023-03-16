# Copyright (c) 2022, Texas Instruments Incorporated All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of Texas Instruments Incorporated nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
cmake_minimum_required(VERSION ${TI_MIN_CMAKE_VERSION})

set(SUPPORTED_COMPONENTS ${SUPPORTED_COMPONENTS} driverlib_cc13xx_cc26xx)
list(REMOVE_DUPLICATES SUPPORTED_COMPONENTS)
project(driverlib_cc13xx_cc26xx LANGUAGES)

# Assume nobody is actively misbehaving and defining partial target sets
if(NOT TARGET driverlib_cc13x1)
  #
  # Driverlib for CC13X1
  #
  add_library(driverlib_cc13x1 INTERFACE IMPORTED)
  add_library(Driverlib::cc13x1 ALIAS driverlib_cc13x1)
  add_library(DriverlibCC13XXCC26XX::cc13x1 ALIAS driverlib_cc13x1)
  target_link_libraries(
    driverlib_cc13x1
    INTERFACE
      Device::cc13x1
      ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x1_cc26x1/driverlib/bin/${TI_TOOLCHAIN_NAME}/driverlib.lib
  )
  target_include_directories(
    driverlib_cc13x1
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../../..
              ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x1_cc26x1)

  #
  # Driverlib for CC26X1
  #
  add_library(driverlib_cc26x1 INTERFACE IMPORTED)
  add_library(Driverlib::cc26x1 ALIAS driverlib_cc26x1)
  add_library(DriverlibCC13XXCC26XX::cc26x1 ALIAS driverlib_cc26x1)
  target_link_libraries(
    driverlib_cc26x1
    INTERFACE
      Device::cc26x1
      ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x1_cc26x1/driverlib/bin/${TI_TOOLCHAIN_NAME}/driverlib.lib
  )
  target_include_directories(
    driverlib_cc26x1
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../../..
              ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x1_cc26x1)

  #
  # Driverlib for CC13X2
  #
  add_library(driverlib_cc13x2 INTERFACE IMPORTED)
  add_library(Driverlib::cc13x2 ALIAS driverlib_cc13x2)
  add_library(DriverlibCC13XXCC26XX::cc13x2 ALIAS driverlib_cc13x2)
  target_link_libraries(
    driverlib_cc13x2
    INTERFACE
      Device::cc13x2
      ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x2_cc26x2/driverlib/bin/${TI_TOOLCHAIN_NAME}/driverlib.lib
  )
  target_include_directories(
    driverlib_cc13x2
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../../..
              ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x2_cc26x2)

  #
  # Driverlib for CC26X2
  #
  add_library(driverlib_cc26x2 INTERFACE IMPORTED)
  add_library(Driverlib::cc26x2 ALIAS driverlib_cc26x2)
  add_library(DriverlibCC13XXCC26XX::cc26x2 ALIAS driverlib_cc26x2)
  target_link_libraries(
    driverlib_cc26x2
    INTERFACE
      Device::cc26x2
      ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x2_cc26x2/driverlib/bin/${TI_TOOLCHAIN_NAME}/driverlib.lib
  )
  target_include_directories(
    driverlib_cc26x2
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../../..
              ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x2_cc26x2)

  #
  # Driverlib for CC13X2X7
  #
  add_library(driverlib_cc13x2x7 INTERFACE IMPORTED)
  add_library(Driverlib::cc13x2x7 ALIAS driverlib_cc13x2x7)
  add_library(DriverlibCC13XXCC26XX::cc13x2x7 ALIAS driverlib_cc13x2x7)
  target_link_libraries(
    driverlib_cc13x2x7
    INTERFACE
      Device::cc13x2x7
      ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x2x7_cc26x2x7/driverlib/bin/${TI_TOOLCHAIN_NAME}/driverlib.lib
  )
  target_include_directories(
    driverlib_cc13x2x7
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../../..
              ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x2x7_cc26x2x7)

  #
  # Driverlib for CC26X2X7
  #
  add_library(driverlib_cc26x2x7 INTERFACE IMPORTED)
  add_library(Driverlib::cc26x2x7 ALIAS driverlib_cc26x2x7)
  add_library(DriverlibCC13XXCC26XX::cc26x2x7 ALIAS driverlib_cc26x2x7)
  target_link_libraries(
    driverlib_cc26x2x7
    INTERFACE
      Device::cc26x2x7
      ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x2x7_cc26x2x7/driverlib/bin/${TI_TOOLCHAIN_NAME}/driverlib.lib
  )
  target_include_directories(
    driverlib_cc26x2x7
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../../..
              ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x2x7_cc26x2x7)

  #
  # Driverlib for CC13X4
  #
  add_library(driverlib_cc13x4 INTERFACE IMPORTED)
  add_library(Driverlib::cc13x4 ALIAS driverlib_cc13x4)
  add_library(DriverlibCC13XXCC26XX::cc13x4 ALIAS driverlib_cc13x4)
  target_link_libraries(
    driverlib_cc13x4
    INTERFACE
      Device::cc13x4
      ${CMAKE_CURRENT_LIST_DIR}/../../../source/ti/devices/cc13x4_cc26x4/driverlib/bin/${TI_TOOLCHAIN_NAME}/driverlib.lib
  )
  target_include_directories(
    driverlib_cc13x4
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../../..
              ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x4_cc26x4)

  #
  # Driverlib for CC26X3
  #
  add_library(driverlib_cc26x3 INTERFACE IMPORTED)
  add_library(Driverlib::cc26x3 ALIAS driverlib_cc26x3)
  add_library(DriverlibCC13XXCC26XX::cc26x3 ALIAS driverlib_cc26x3)
  target_link_libraries(
    driverlib_cc26x3
    INTERFACE
      Device::cc26x3
      ${CMAKE_CURRENT_LIST_DIR}/../../../source/ti/devices/cc13x4_cc26x4/driverlib/bin/${TI_TOOLCHAIN_NAME}/driverlib.lib
  )
  target_include_directories(
    driverlib_cc26x3
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../../..
              ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x4_cc26x4)

  #
  # Driverlib for CC26X4
  #
  add_library(driverlib_cc26x4 INTERFACE IMPORTED)
  add_library(Driverlib::cc26x4 ALIAS driverlib_cc26x4)
  add_library(DriverlibCC13XXCC26XX::cc26x4 ALIAS driverlib_cc26x4)
  target_link_libraries(
    driverlib_cc26x4
    INTERFACE
      Device::cc26x4
      ${CMAKE_CURRENT_LIST_DIR}/../../../source/ti/devices/cc13x4_cc26x4/driverlib/bin/${TI_TOOLCHAIN_NAME}/driverlib.lib
  )
  target_include_directories(
    driverlib_cc26x4
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../../..
              ${CMAKE_CURRENT_LIST_DIR}/../../../ti/devices/cc13x4_cc26x4)
endif()
