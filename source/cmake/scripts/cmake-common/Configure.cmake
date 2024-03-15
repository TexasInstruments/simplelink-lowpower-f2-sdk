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
set(TI_MIN_CMAKE_VERSION "3.21.3" CACHE STRING INTERNAL)
cmake_minimum_required(VERSION ${TI_MIN_CMAKE_VERSION})

set(CMAKE_COLOR_MAKEFILE 0 CACHE BOOL INTERNAL)
set(CMAKE_STATIC_LIBRARY_PREFIX "" CACHE STRING INTERNAL)

set(TI_ALL_SUPPORTED_ARCHITECTURES "m0p;m3;m4;m4f;m33f" CACHE STRING INTERNAL)

# This can be overridden to Debug from the command line with -DCONFIG=Debug
set(CMAKE_BUILD_TYPE Release)

################################################################################
## STEP 1: Internal build configuration and toolchain setup

# Internally we have the dependencies in conan, so we can use a generated file
# to check if we are in that environment
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/imports.conan.mak)
    include(${CMAKE_COMMON_INSTALL_DIR}/source/cmake/scripts/cmake-common/ReadVariables.cmake)
    # imports.conan.mak specifies the location of many SDK packages
    # Dump those variables into the CMake environment
    read_makefile_vars(${CMAKE_CURRENT_SOURCE_DIR}/imports.conan.mak)
    # And set the internal build flag so components can make decisions
    set(TI_INTERNAL_BUILD 1 CACHE BOOL INTERNAL)
    message(VERBOSE "This is an internal build")
else()
    # Clear the internal build flag
    unset(TI_INTERNAL_BUILD CACHE)
    # For external builds, we always want to check local platforms
    # They are the only platforms available without imports.conan.mak
    set(CMAKE_ONLY_LOCAL_PLATFORMS 1)
    set(CMAKE_ONLY_WRAPPER_COMPONENTS 1)
endif()

include(${CMAKE_COMMON_INSTALL_DIR}/source/cmake/scripts/cmake-common/LibraryTools.cmake)

# Doing this automatically simplifies the user command line
if(NOT DEFINED CMAKE_C_COMPILER) # If compiling for ARM, help the user
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        if(${CMAKE_BINARY_DIR} MATCHES "/ticlang")
            set(CMAKE_TOOLCHAIN_FILE "${CMAKE_COMMON_INSTALL_DIR}/source/cmake/toolchains/ticlang.cmake" CACHE PATH
                                                                                                               INTERNAL
            )
        elseif(${CMAKE_BINARY_DIR} MATCHES "/gcc")
            set(CMAKE_TOOLCHAIN_FILE "${CMAKE_COMMON_INSTALL_DIR}/source/cmake/toolchains/gcc.cmake" CACHE PATH
                                                                                                           INTERNAL
            )
        elseif(${CMAKE_BINARY_DIR} MATCHES "/iar")
            set(CMAKE_TOOLCHAIN_FILE "${CMAKE_COMMON_INSTALL_DIR}/source/cmake/toolchains/iar.cmake" CACHE PATH
                                                                                                           INTERNAL
            )
        else()
            message(
                FATAL_ERROR
                    "The build directory (${CMAKE_BINARY_DIR}) should end with ticlang/gcc/iar, for example build/ticlang. "
                    "You can also -DCMAKE_TOOLCHAIN_FILE=[file] on the command line to override."
            )
        endif()
    endif()

    # Include the toolchain to define the interface library
    include(${CMAKE_TOOLCHAIN_FILE})
else()
    # Compiler is specified, so fall back to blank "empty"
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_COMMON_INSTALL_DIR}/source/cmake/toolchains/native.cmake" CACHE PATH INTERNAL)
    include(${CMAKE_TOOLCHAIN_FILE})
endif()

# This project call is mandatory - it configures the toolchain files and checks the ASM compiler
project(common_cmake LANGUAGES C ASM)

define_property(
    DIRECTORY PROPERTY TI_NAMESPACE BRIEF_DOCS "Current package namespace"
    FULL_DOCS "Set by ti_init_package, used by ti_add_library and ti_export_package"
)
define_property(
    DIRECTORY PROPERTY TI_PACKAGE_NAME BRIEF_DOCS "Current package name"
    FULL_DOCS "Set by ti_init_package, used by ti_add_library and ti_export_package"
)
define_property(
    DIRECTORY PROPERTY TI_PACKAGE_TYPE BRIEF_DOCS "Current package type"
    FULL_DOCS "Set by ti_init_package, used by ti_add_library and ti_export_package"
)
define_property(
    DIRECTORY PROPERTY TI_PACKAGE_DEPENDENCIES BRIEF_DOCS "Current package dependencies"
    FULL_DOCS "All packages in this list will have find_package() called by the generated Config file"
)

define_property(
    DIRECTORY PROPERTY TI_PACKAGE_LIBRARIES BRIEF_DOCS "Current package generated libraries"
    FULL_DOCS "A list of libraries created by ti_add_library. This property can be used by callers to "
              "perform operations on all current libraries. Reset by ti_init_package()."
)
define_property(
    DIRECTORY PROPERTY TI_PACKAGE_TARGET_FILES BRIEF_DOCS "Current package generated target files"
    FULL_DOCS "A list of Targets.cmake files created by ti_add_library"
)

################################################################################
## STEP 2: Locate device files and generate SUPPORTED_PLATFORMS/SUPPORTED_ARCHITECTURES

# Locate any devices supported by the current component or in components exporting CMAKE_PLATFORM_DIRS
# Note CMAKE_PLATFORM_DIRS will only be populated for internal builds
string(REPLACE ":" ";" CMAKE_PLATFORM_DIRS "$ENV{CMAKE_PLATFORM_DIRS}")

# Always search the local targets folder, which is the only relevant folder for customer builds
if(DEFINED CMAKE_ONLY_LOCAL_PLATFORMS)
    set(CMAKE_PLATFORM_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/source/cmake/platforms")
endif()

# Reset the list of supported features
set(SUPPORTED_DEVICES "")
set(SUPPORTED_PLATFORMS "")
set(SUPPORTED_ARCHITECTURES "")

# Add all first so pacakges can find each other
foreach(platform_dir ${CMAKE_PLATFORM_DIRS})
    list(APPEND CMAKE_PREFIX_PATH "${platform_dir}/${TI_TOOLCHAIN_NAME}")
endforeach()

# Glob each target directory for cmake files, which we assume are target definitions
message(DEBUG "CMAKE_PLATFORM_DIRS=${CMAKE_PLATFORM_DIRS}")
foreach(platform_dir ${CMAKE_PLATFORM_DIRS})
    message(DEBUG "Scanning folder ${platform_dir}")
    file(GLOB pkgconfigs ${platform_dir}/${TI_TOOLCHAIN_NAME}/*Config.cmake)
    foreach(pkgconfig ${pkgconfigs})
        get_filename_component(confbase ${pkgconfig} NAME_WE)
        string(REGEX REPLACE "(.*)Config" "\\1" pkgname ${confbase})
        message(DEBUG "-- Found ${pkgname}")
        unset(${pkgname}_DIR CACHE) # Force a new search in case new conan install
        find_package(${pkgname})
    endforeach()
endforeach()

if(SET_PLATFORMS)
    # SET_PLATFORMS is primarily for debugging and static analysis, not for production builds
    message(WARNING "Platforms are restricted by the SET_PLATFORMS variable: ${SET_PLATFORMS}")

    # Convert from space-separated argument into a CMake List
    string(REPLACE " " ";" SET_PLATFORMS ${SET_PLATFORMS})

    # We need to rebuild this list from scratch
    set(SUPPORTED_ARCHITECTURES "")

    # Validate that all the request platforms are actually supported by device/driverlib files
    foreach(platform ${SET_PLATFORMS})
        if(NOT ${platform} IN_LIST SUPPORTED_PLATFORMS)
            message(FATAL_ERROR "Platform ${platform} is not supported! Supported platforms: ${SUPPORTED_PLATFORMS}")
        endif()

        # Construct the SUPPORTED_ARCHITECTURES for this configuration
        list(APPEND SUPPORTED_ARCHITECTURES ${ARCH_${platform}})
    endforeach()

    # We know all the platforms in this list exist, so just dump it out as the final list
    set(SUPPORTED_PLATFORMS ${SET_PLATFORMS})
endif()

# Trim any duplicates (ARCHITECTURES especially ends up very duplicated)
list(REMOVE_DUPLICATES SUPPORTED_DEVICES)
list(REMOVE_DUPLICATES SUPPORTED_PLATFORMS)
list(REMOVE_DUPLICATES SUPPORTED_ARCHITECTURES)

set(SUPPORTED_DEVICES "${SUPPORTED_DEVICES}" CACHE STRING INTERNAL)
set(SUPPORTED_PLATFORMS "${SUPPORTED_PLATFORMS}" CACHE STRING INTERNAL)
set(SUPPORTED_ARCHITECTURES "${SUPPORTED_ARCHITECTURES}" CACHE STRING INTERNAL)

# Print the list of located platforms and architectures, for convenience
string(REPLACE ";" ", " PLATFORMS_PRINTABLE "${SUPPORTED_PLATFORMS}")
message(VERBOSE "Supported platforms: ${PLATFORMS_PRINTABLE}")
string(REPLACE ";" ", " ARCHITECTURES_PRINTABLE "${SUPPORTED_ARCHITECTURES}")
message(VERBOSE "Supported architectures: ${ARCHITECTURES_PRINTABLE}")

################################################################################
## STEP 4: Locate device files and generate SUPPORTED_COMPONENTS/COMPONENT_CMAKEFILES

# Iterate through all files in components folders defined by internal dependencies
string(REPLACE ":" ";" CMAKE_COMPONENT_FOLDERS "${CMAKE_COMPONENT_DIRS}")

# Avoid CMake including any file twice, which we are not set up for
# Duplicate targets will error out the build process
list(REMOVE_DUPLICATES CMAKE_COMPONENT_FOLDERS)

if(CMAKE_ONLY_WRAPPER_COMPONENTS)
    set(CMAKE_COMPONENT_FOLDERS "${CMAKE_CURRENT_SOURCE_DIR}/source/cmake/wrappers")
endif()

# Add all first so pacakges can find each other
foreach(component_dir ${CMAKE_COMPONENT_FOLDERS})
    list(APPEND CMAKE_PREFIX_PATH "${component_dir}/${TI_TOOLCHAIN_NAME}")
endforeach()

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} CACHE STRING INTERNAL)

# Print the list of located components
string(REPLACE ";" "\n   -- " PREFIX_PATH_PRINTABLE "${CMAKE_PREFIX_PATH}")
message(VERBOSE "Package search paths: \n   -- ${PREFIX_PATH_PRINTABLE}")
