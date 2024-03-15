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

# Don't ignore empty list elements when counting length
if(POLICY CMP0007)
    cmake_policy(SET CMP0007 NEW)
endif()

################################################################################
# read_makefile_vars
#
# Reads lines from given file
# Parses assignment statements
# Makes the same assignments in the CMake cache
#
# Argument MKFile: Makefile name (current directory) or full path
# Output: Makes variable assignments in the cache
#
function(read_makefile_vars MKFile)
    file(STRINGS "${MKFile}" FileContents REGEX "=")
    foreach(line ${FileContents})
        # Replace = and := with semicolons and split the line
        string(REGEX REPLACE "(:?=)" ";" line_split "${line}")
        list(LENGTH line_split count)

        # Ignore anything that isn't A=B, this is not supposed to be generic
        if(NOT count EQUAL 2)
            continue()
        endif()

        # Extract variable name and value from the list
        list(GET line_split 0 var_name)
        string(STRIP ${var_name} var_name)
        list(GET line_split 1 value)
        string(STRIP ${value} value)

        # Set during this run only, re-run import next configure step
        set(${var_name} "${value}" PARENT_SCOPE)
    endforeach()
endfunction()
