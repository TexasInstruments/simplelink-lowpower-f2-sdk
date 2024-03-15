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
include(CMakePackageConfigHelpers)

# Helper for setting a variable from a number of options, with priority
# _ti_set(output_variable FROM_ANY_OF variable [variable ...])
#
# output_variable: The name of the variable to configure
# variable(s): Variable names (not values) to use as data sources, in order
#
# This function evaluates each variable passed to FROM_ANY_OF in order. If the variable
# has any content, the output variable is set to that content and the function exits.
# Following variables are ignored, even if they have valid values. If no passed variable
# had any content, a fatal error will be triggered.
function(_ti_set output_variable)
    set(options OPTIONAL VALIDATE)
    set(multi_value_args FROM_ANY_OF)
    cmake_parse_arguments(TI_SET "${options}" "" "${multi_value_args}" ${ARGN})

    # Argument validation
    if(NOT TI_SET_FROM_ANY_OF)
        message(FATAL_ERROR "You must provide a set of variables to FROM_ANY_OF")
    endif()

    foreach(variable_source ${TI_SET_FROM_ANY_OF})
        # Shorthand for "is this variable defined and has content that is not the empty string"
        # Note that because "variable_source" contains the variable name as a string, we need to
        # interpolate it to evaluate it correctly
        if(${variable_source})
            # Bubble the variable up and exit
            set(${output_variable} ${${variable_source}} PARENT_SCOPE)
            message(TRACE "Setting ${output_variable} to ${${variable_source}} from ${variable_source}")
            return()
        endif()
    endforeach()
    message(FATAL_ERROR "Couldn't set ${output_variable} from any of: ${TI_SET_FROM_ANY_OF}")
endfunction()

# Exit with a fatal error if a variable is not set
# _ti_require(variable_name)
#
# variable_name: The name of the variable to check
function(_ti_require variable_name)
    if(NOT ${variable_name})
        message(FATAL_ERROR "Expected ${variable_name} to be set")
    endif()
endfunction()

function(_ti_quiet_find package_name)
    if(NOT ${package_name}_FOUND)
        # Attempt to find the specified package. If not available (for example, the libraries are
        # defined locally) just silently accept it. This will cause configure errors e.g. "XYZ::zyx
        # could not be found".
        find_package(${package_name} QUIET)

        if(NOT ${package_name}_FOUND)
            message(DEBUG "_ti_quiet_find: requested package ${package_name} but it was not found. "
                    "If this package is internal, this is not a problem."
            )
        else()
            message(DEBUG "_ti_quiet_find: loaded ${package_name}")
        endif()
    else()
        message(DEBUG "_ti_quiet_find: skipped loading ${package_name}")
    endif()
endfunction()

# Get the root component location represented by this variable
# get_install_dir(component_install_dir)
#
# component_install_dir: The INSTALL_DIR variable, e.g. TIUTILS_INSTALL_DIR
#
# If the variable is set, and this is an internal build, the existing value is
# returned. Otherwise, CMAKE_SOURCE_DIR is returned.
function(get_install_dir component_install_dir)
    if(TI_INTERNAL_BUILD AND ${component_install_dir})
        set(${component_install_dir} ${${component_install_dir}} PARENT_SCOPE)
        message(DEBUG "get_install_dir: setting ${component_install_dir} to ${${component_install_dir}}")
    else()
        set(${component_install_dir} ${CMAKE_SOURCE_DIR} PARENT_SCOPE)
        message(DEBUG "get_install_dir: setting ${component_install_dir} to CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
    endif()
endfunction()

# Configures per-file configuration variables
# ti_setup(NAMESPACE namespace [PACKAGE_NAME package_name] [PACKAGE_TYPE <COMPONENT|PLATFORM>]
#          [DEPENDS_ON package [package ...])
#
# NAMESPACE: A product or component namespace, e.g. "Drivers" or "Rflib" or "ThirdPartyMbedTls"
# PACKAGE_NAME: Your component package name to be appended to NAMESPACE. Optional, see below.
# PACKAGE_TYPE: Either COMPONENT or PLATFORM. See below. Optional, defaults to COMPONENT.
# DEPENDS_ON: List all packages that you use libraries from. They will be imported as part of
#             this function, and later exported as dependencies into the package itself.
#
# Call this function at the top of a CMakeLists.txt file, after a project() call. It resets the
# variables TI_LIBRARIES and TI_COMPONENT_TARGETS, which are then populated by add_ti_library
# and ti_create_targets. You can use TI_LIBRARIES for iterating over generated libraries, but
# it is not used internally. TI_COMPONENT_TARGETS is used by ti_create_package to list targets.
#
# COMPONENT packages are most common, and export libraries for building or use by other components.
# PLATFORM packages are explicitly for device support, and only one should be exported per device,
# usually by the component that implements it (i.e. driverlib).
#
# If PACKAGE_NAME is not provided, then it will be set to NAMESPACE, so you can have a
# single-package namespace like "TiUtils" if you are exporting generic components.
function(ti_init_package)
    set(single_value_args NAMESPACE PACKAGE_NAME PACKAGE_TYPE)
    set(multi_value_args DEPENDS_ON)
    cmake_parse_arguments(TI_SETUP "" "${single_value_args}" "${multi_value_args}" ${ARGN})

    if(TI_SETUP_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed arguments: ${TI_SETUP_UNPARSED_ARGUMENTS}")
    endif()
    if(TI_SETUP_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "Arguments missing values: ${TI_SETUP_KEYWORDS_MISSING_VALUES}")
    endif()

    # NAMESPACE is mandatory, and must be provided by the user in the argument
    _ti_require(TI_SETUP_NAMESPACE)

    # Package type defaults to component, if the user has not provided one
    if(NOT TI_SETUP_PACKAGE_TYPE)
        set(TI_SETUP_PACKAGE_TYPE "COMPONENT")
    endif()

    if(TI_SETUP_PACKAGE_NAME)
        # The PACKAGE_NAME should not start with the namespace
        if(${TI_SETUP_PACKAGE_NAME} MATCHES "^${TI_SETUP_NAMESPACE}")
            message(FATAL_ERROR "Package ${TI_SETUP_PACKAGE_NAME} must not start with namespace ${TI_SETUP_NAMESPACE}")
        endif()
        # Concatenate with namespace, e.g. "TiUtilsLoggers"
        set(TI_SETUP_PACKAGE_NAME "${TI_SETUP_NAMESPACE}${TI_SETUP_PACKAGE_NAME}")
    else()
        # If we don't have a package name, reuse the namespace
        set(TI_SETUP_PACKAGE_NAME ${TI_SETUP_NAMESPACE})
    endif()

    # Note: This does not affect the find_package emitted into the resulting package config,
    # which is always `find_package(XYZ REQUIRED)`.
    foreach(dependency ${TI_SETUP_DEPENDS_ON})
        _ti_quiet_find(${dependency})
    endforeach()

    # These variables track state for the current package
    set_property(DIRECTORY PROPERTY TI_NAMESPACE ${TI_SETUP_NAMESPACE})
    set_property(DIRECTORY PROPERTY TI_PACKAGE_NAME ${TI_SETUP_PACKAGE_NAME})
    set_property(DIRECTORY PROPERTY TI_PACKAGE_TYPE ${TI_SETUP_PACKAGE_TYPE})
    set_property(DIRECTORY PROPERTY TI_PACKAGE_DEPENDENCIES ${TI_SETUP_DEPENDS_ON})

    set_property(DIRECTORY PROPERTY TI_PACKAGE_LIBRARIES "")
    set_property(DIRECTORY PROPERTY TI_PACKAGE_TARGET_FILES "")
endfunction()

function(ti_add_package_dependency package_name)
    set_property(DIRECTORY APPEND PROPERTY TI_PACKAGE_DEPENDENCIES ${package_name})
    _ti_quiet_find(${package_name})
endfunction()

# Configures per-file configuration variables
# ti_add_library(library_name [LIBRARY_EXISTS] [INTERFACE] [NO_PACKAGE]
# [OUTPUT_NAME name] [EXPORT_NAME name] [ARCH arch] [ADD_SDK_INCLUDE_PATH path]
# [SOURCES file ...] [LINK_PRIVATE lib ...]] [LINK_PUBLIC lib ...]] [LINK_INTERFACE lib ...])
#
# library_name: The CMake library to define or reference; must be globally unique
# LIBRARY_EXISTS: This library already exists; this function will not call add_library
# INTERFACE: This library should be defined as INTERFACE (SOURCES will be ignored)
# NO_PACKAGE: Do not automatically call ti_create_targets()
# NAMESPACE: A product or component namespace, e.g. "Drivers" or "Rflib"
# OUTPUT_NAME: Set the final filename.By default, this is the library name.
# EXPORT_NAME: Set the exported library name. By default, this is library_name or OUTPUT_NAME.
# ARCH: An architecture string, e.g. "m0"
# ADD_SDK_INCLUDE_PATH: A single relative path from the SDK root to add as a PUBLIC include
# OUTPUT_FOLDER: The library folder, relative to current list dir. Optional, defaults to lib/tool/arch/
# SOURCES: A list of source files, for creating non-INTERFACE libraries
# LINK_PRIVATE: Libraries to link against with the PRIVATE keyword
# LINK_PUBLIC: Libraries to link against with the PUBLIC keyword
# LINK_INTERFACE: Libraries to link against with the INTERFACE keyword
#
# Notes:
#  - You must set one of SOURCES, LIBRARY_EXIST or INTERFACE to define a library.
#  - If both OUTPUT_NAME and EXPORT_NAME are set, the export and output name will be different.
#  - OUTPUT_FOLDER will be added to the default clean goal, so the installed targets are removed on clean
#
# See Also:
#  - ti_create_targets()
function(ti_add_library library_name)
    set(options LIBRARY_EXISTS INTERFACE NO_PACKAGE)
    set(single_value_args OUTPUT_NAME EXPORT_NAME ARCH ADD_SDK_INCLUDE_PATH OUTPUT_FOLDER)
    set(multi_value_args SOURCES LINK_PRIVATE LINK_PUBLIC LINK_INTERFACE)
    cmake_parse_arguments(CONFIG_LIB "${options}" "${single_value_args}" "${multi_value_args}" ${ARGN})

    if(CONFIG_LIB_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed arguments: ${CONFIG_LIB_UNPARSED_ARGUMENTS}")
    endif()
    if(CONFIG_LIB_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "Arguments missing values: ${CONFIG_LIB_KEYWORDS_MISSING_VALUES}")
    endif()

    # Argument validation
    _ti_set(TI_ARCH FROM_ANY_OF CONFIG_LIB_ARCH TI_ARCH ARCH_${TI_PLATFORM})
    get_property(TI_NAMESPACE DIRECTORY PROPERTY TI_NAMESPACE)

    if(NOT CONFIG_LIB_SOURCES AND NOT CONFIG_LIB_INTERFACE AND NOT CONFIG_LIB_LIBRARY_EXISTS)
        message(FATAL_ERROR "You must either set SOURCES, INTERFACE or LIBRARY_EXISTS")
    endif()

    # Determine output name and export name, usually the same as the library name
    _ti_set(output_name FROM_ANY_OF CONFIG_LIB_OUTPUT_NAME library_name)
    _ti_set(export_name FROM_ANY_OF CONFIG_LIB_EXPORT_NAME library_name)

    # LIBRARY_EXISTS allows skipping the add_library call entirely
    # We should still check the library does exist though
    if(CONFIG_LIB_LIBRARY_EXISTS)
        if(NOT TARGET ${library_name})
            message(FATAL_ERROR "LIBRARY_EXISTS was set, but could not find ${library_name}")
        endif()
    else()
        if(CONFIG_LIB_INTERFACE)
            add_library(${library_name} INTERFACE)
        else()
            add_library(${library_name} ${CONFIG_LIB_SOURCES})
        endif()
    endif()

    add_library(${TI_NAMESPACE}::${export_name} ALIAS ${library_name})
    message(DEBUG "ti_add_library: Added ${library_name} with alias ${TI_NAMESPACE}::${export_name}")

    # We keep track of all defined libraries; we don't use this variable internally
    # but it may be useful to implementing functions to iterate over all libraries
    # e.g. to add unconditional include paths
    set_property(DIRECTORY APPEND PROPERTY TI_PACKAGE_LIBRARIES ${library_name})

    set_target_properties(
        ${library_name}
        PROPERTIES # This prefix has to be set, otherwise all names start with `lib`
                   PREFIX ""
                   # Configure names as the user wants them
                   OUTPUT_NAME ${output_name}
                   EXPORT_NAME ${export_name}
                   # If we don't export these to subdirectories, same-name libs will conflict
                   # This does NOT impact the final library locations on disk, only their location
                   # relative to the CMake build folder
                   LIBRARY_OUTPUT_DIRECTORY lib/${TI_TOOLCHAIN_NAME}/${TI_ARCH}/${output_name}
                   ARCHIVE_OUTPUT_DIRECTORY lib/${TI_TOOLCHAIN_NAME}/${TI_ARCH}/${output_name}
    )

    if(CONFIG_LIB_OUTPUT_FOLDER)
        set(CLEAN_PATH ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_LIB_OUTPUT_FOLDER}/)
    else()
        set(CLEAN_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/)
    endif()

    # Make sure this lib/ dir is cleaned by cmake --clean
    set_property(TARGET ${library_name} APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${CLEAN_PATH})

    target_link_libraries(
        ${library_name} PRIVATE ${CONFIG_LIB_LINK_PRIVATE} PUBLIC ${CONFIG_LIB_LINK_PUBLIC}
        INTERFACE ${CONFIG_LIB_LINK_INTERFACE}
    )

    if(CONFIG_LIB_ADD_SDK_INCLUDE_PATH)
        if(CONFIG_LIB_INTERFACE)
            ti_export_sdk_include(${library_name} INTERFACE ${CONFIG_LIB_ADD_SDK_INCLUDE_PATH})
        else()
            ti_export_sdk_include(${library_name} PUBLIC ${CONFIG_LIB_ADD_SDK_INCLUDE_PATH})
        endif()
    endif()

    if(NOT CONFIG_LIB_NO_PACKAGE)
        if(CONFIG_LIB_OUTPUT_FOLDER)
            ti_create_targets(ARCH ${TI_ARCH} TARGETS ${library_name} OUTPUT_FOLDER ${CONFIG_LIB_OUTPUT_FOLDER})
        else()
            ti_create_targets(ARCH ${TI_ARCH} TARGETS ${library_name})
        endif()
    endif()
endfunction()

# Exports a PUBLIC or INTERFACE include to [relative_path]
# ti_export_sdk_include(library_name include_type relative_path)
#
# library_name: The CMake name of your library, as passed to ti_add_library
# include_type: PUBLIC or INTERFACE, to be applied to the include path
# relative_path: A path from the repository/SDK root to export as an include (e.g. "kernel/tirtos7/packages")
#
# PUBLIC means it will be applied both when building the library internally, as well as inherited by targets depending
# on your library to build. This enables distributed builds (i.e. from the conan cache) as well as SDK customer builds.
# INTERFACE is for interface libraries, which cannot have PUBLIC properties, and is only inherited.
function(ti_export_sdk_include library_name include_type relative_path)
    target_include_directories(
        ${library_name} ${include_type} "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/${relative_path}>"
        "$<INSTALL_INTERFACE:${relative_path}>"
    )
endfunction()

# Generates an architecture-specific ConfigPackageMxxTargets.cmake package, and exports it
# ti_create_targets([ARCH arch] [OUTPUT_FOLDER folder] [TARGETS target target ...])
#
# ARCH: An architecture string, e.g. "m0". Optional, defaults to TI_ARCH or ARCH_${TI_PLATFORM}
# OUTPUT_FOLDER: The library folder, relative to current list dir. Optional, defaults to lib/tool/arch/
# TARGETS: A list of target library names for export (all the same architecture)
#
# Notes:
#  - This function will be called automatically by add_ti_library(). You do not normally need to call it manually.
#  - You must have called ti_init_package to use this function.
function(ti_create_targets)
    set(single_value_args ARCH OUTPUT_FOLDER)
    set(multi_value_args TARGETS)
    cmake_parse_arguments(CREATE_TARGETS "" "${single_value_args}" "${multi_value_args}" ${ARGN})

    if(CREATE_TARGETS_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed arguments: ${CREATE_TARGETS_UNPARSED_ARGUMENTS}")
    endif()
    if(CREATE_TARGETS_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "Arguments missing values: ${CREATE_TARGETS_KEYWORDS_MISSING_VALUES}")
    endif()

    _ti_set(TI_ARCH FROM_ANY_OF CREATE_TARGETS_ARCH TI_ARCH)
    get_property(TI_NAMESPACE DIRECTORY PROPERTY TI_NAMESPACE)
    get_property(TI_PACKAGE_NAME DIRECTORY PROPERTY TI_PACKAGE_NAME)
    get_property(TI_PACKAGE_TYPE DIRECTORY PROPERTY TI_PACKAGE_TYPE)
    _ti_require(TI_NAMESPACE)
    _ti_require(TI_PACKAGE_NAME)
    _ti_require(TI_PACKAGE_TYPE)

    if(NOT CREATE_TARGETS_TARGETS)
        message(FATAL_ERROR "You must provide TARGETS")
    endif()

    # This controls the 'relative root' of generated packages, and should probably always be CMAKE_SOURCE_DIR
    # If the user has already configured it, we can skip this
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}" CACHE PATH "..." FORCE)
    endif()

    # Get the relative path from top-level cmake to current, so it can be used or transplanted to install prefix
    # Destination must be relative, and becomes "INSTALL_PREFIX/<whatever>" in final use, so we make "current" be only the relative
    # part from the project root to here, then finally normalize the end result to get rid of any `../..`s.
    cmake_path(
        RELATIVE_PATH CMAKE_CURRENT_LIST_DIR BASE_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE
        CMAKE_CURRENT_LIST_DIR_RELATIVE_SOURCE_DIR
    )
    if(CREATE_TARGETS_OUTPUT_FOLDER)
        set(FINAL_OUTPUT_FOLDER ${CMAKE_CURRENT_LIST_DIR_RELATIVE_SOURCE_DIR}/${CREATE_TARGETS_OUTPUT_FOLDER})
    else()
        set(FINAL_OUTPUT_FOLDER ${CMAKE_CURRENT_LIST_DIR_RELATIVE_SOURCE_DIR}/lib/${TI_TOOLCHAIN_NAME}/${TI_ARCH})
    endif()
    cmake_path(NORMAL_PATH FINAL_OUTPUT_FOLDER)

    # It looks strange to have Driversm0Targets, so uppercase the architecture
    string(TOUPPER ${TI_ARCH} arch_upper)

    # Construct the resulting filename e.g. DriversM0Targets
    set(target_set_name ${TI_PACKAGE_NAME}${arch_upper}Targets)

    # Install the actual targets into DriversM0Targets.cmake
    message(DEBUG "ti_create_targets: exported ${CREATE_TARGETS_TARGETS} into ${target_set_name}")
    install(TARGETS ${CREATE_TARGETS_TARGETS} EXPORT ${target_set_name} DESTINATION ${FINAL_OUTPUT_FOLDER})

    if(${TI_PACKAGE_TYPE} STREQUAL "COMPONENT")
        set(cmake_folder components)
    else()
        set(cmake_folder platforms)
    endif()

    # We don't want to export the same package twice, as it prints a warning message
    get_property(package_target_files DIRECTORY PROPERTY TI_PACKAGE_TARGET_FILES)

    if(NOT ${target_set_name} IN_LIST package_target_files)
        # Install relocatable package file into source/cmake/components
        install(EXPORT ${target_set_name} FILE ${target_set_name}.cmake NAMESPACE ${TI_NAMESPACE}::
                DESTINATION source/cmake/${cmake_folder}/${TI_TOOLCHAIN_NAME}
        )

        # Keep track of the resulting list of exported packages
        set_property(DIRECTORY APPEND PROPERTY TI_PACKAGE_TARGET_FILES ${target_set_name})
    endif()
endfunction()

# Generates a ConfigPackage.cmake package referencing previously defined target sets, and exports it
# ti_create_package([ALLOW_EMPTY] [CUSTOM_TEMPLATE])
#
# ALLOW_EMPTY: Suppress the warning generated if there were no target sets (not for normal use)
# CUSTOM_TEMPLATE: Looks for a .cmake.in file in the local folder, instead of generating one
#
# This function looks for a Config[TI_PACKAGE_NAME].cmake.in template in the current directory. If this
# does not exist, the function will generate one for you, which can be git-ignored. Templates are
# also available in the cmake-common repository.
function(ti_export_package)
    set(options ALLOW_EMPTY CUSTOM_TEMPLATE)
    cmake_parse_arguments(CREATE_PACKAGE "${options}" "" "" ${ARGN})

    if(CREATE_PACKAGE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unparsed arguments: ${CREATE_PACKAGE_UNPARSED_ARGUMENTS}")
    endif()
    if(CREATE_PACKAGE_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "Arguments missing values: ${CREATE_PACKAGE_KEYWORDS_MISSING_VALUES}")
    endif()

    get_property(TI_PACKAGE_NAME DIRECTORY PROPERTY TI_PACKAGE_NAME)
    get_property(TI_PACKAGE_TYPE DIRECTORY PROPERTY TI_PACKAGE_TYPE)
    _ti_require(TI_PACKAGE_NAME)
    _ti_require(TI_PACKAGE_TYPE)

    # Check if we have any targets to generate from TI_PACKAGE_TARGET_FILES
    get_property(component_targets DIRECTORY PROPERTY TI_PACKAGE_TARGET_FILES)
    if(NOT component_targets)
        if(NOT ALLOW_EMPTY)
            # This is considered a warning because there is no reason to parse this file
            # Maybe a component is being shipped in an SDK where it supports no devices?
            message(WARNING "No libraries generated for ${TI_PACKAGE_NAME} from ${CMAKE_CURRENT_LIST_FILE}")
            return()
        else()
            # But we allow the warning to be overriden, for special cases
            message(DEBUG "${TI_PACKAGE_NAME}: No libraries generated")
        endif()
    endif()

    # This controls the 'relative root' of generated packages, and should probably always be CMAKE_SOURCE_DIR
    # If the user has already configured it, we can skip this
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}" CACHE PATH "..." FORCE)
    endif()

    # This function has two modes: creating platform packages (device support) and components
    if(${TI_PACKAGE_TYPE} STREQUAL "PLATFORM")
        set(cmake_folder platforms)

        if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Config${TI_PACKAGE_NAME}.cmake.in")
            message(
                FATAL_ERROR
                    "You need a device configuration file to export a platform package. Expected to find:"
                    "    ${CMAKE_CURRENT_SOURCE_DIR}/Config${TI_PACKAGE_NAME}.cmake.in.\n"
                    "See ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ConfigPlatformName.cmake.in for a template."
            )
        endif()

        file(READ Config${TI_PACKAGE_NAME}.cmake.in package_content)
        string(FIND "${package_content}" "APPEND SUPPORTED_PLATFORMS" index1)
        string(FIND "${package_content}" "APPEND SUPPORTED_DEVICES" index2)
        string(FIND "${package_content}" "APPEND SUPPORTED_ARCHITECTURES" index3)
        if(index1 EQUAL -1 OR index2 EQUAL -1 OR index3 EQUAL -1)
            message(FATAL_ERROR "A platform package must append SUPPORTED_PLATFORMS, DEVICES and ARCHITECTURES! "
                                "See ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ConfigPlatformName.cmake.in for a template."
            )
        endif()

    elseif(${TI_PACKAGE_TYPE} STREQUAL "COMPONENT")
        set(cmake_folder components)

        if(NOT CREATE_PACKAGE_CUSTOM_TEMPLATE)
            # We need to perform the TI_PACKAGE_NAME replacement in the file, but we want these
            # patterns to be in the final generated copy deployed to the user. To accomplish this we
            # replace them with themselves, including the replacement symbols.
            # @var@ is a special syntax equivalent to ${var} for configure_file
            set(PACKAGE_INIT "@PACKAGE_INIT@")
            set(GENERATED_TARGET_DEPENDENCIES "@GENERATED_TARGET_DEPENDENCIES@")
            set(GENERATED_TARGET_INCLUDES "@GENERATED_TARGET_INCLUDES@")

            configure_file(
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ConfigComponentTemplate.cmake.in
                ${CMAKE_CURRENT_SOURCE_DIR}/Config${TI_PACKAGE_NAME}.cmake.in
            )
        endif()

        # This variable is used to generate content inside the Targets file
        set(GENERATED_TARGET_INCLUDES_LIST)
        # Iterate over all the component_targets defined since the last call to ti_setup
        foreach(target ${component_targets})
            list(APPEND GENERATED_TARGET_INCLUDES_LIST "include(\"\${CMAKE_CURRENT_LIST_DIR}/${target}.cmake\")")
        endforeach()
        # Join the list with newlines, otherwise it will be emitted with semicolons
        list(JOIN GENERATED_TARGET_INCLUDES_LIST "\n" GENERATED_TARGET_INCLUDES)

        # This variable is used to generate content inside the Targets file
        set(GENERATED_TARGET_DEPENDENCIES_LIST)

        # Iterate over all the component_targets defined since the last call to ti_setup
        get_property(package_dependencies DIRECTORY PROPERTY TI_PACKAGE_DEPENDENCIES)
        foreach(dependency ${package_dependencies})
            list(APPEND GENERATED_TARGET_DEPENDENCIES_LIST "find_package(\"${dependency}\" REQUIRED)")
        endforeach()

        # Join the list with newlines, otherwise it will be emitted with semicolons
        list(JOIN GENERATED_TARGET_DEPENDENCIES_LIST "\n" GENERATED_TARGET_DEPENDENCIES)
    endif()

    # Create and install config file from template
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Config${TI_PACKAGE_NAME}.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/source/cmake/${cmake_folder}/${TI_TOOLCHAIN_NAME}/${TI_PACKAGE_NAME}Config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/source/cmake/${cmake_folder}/${TI_TOOLCHAIN_NAME}"
    )
    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/source/cmake/${cmake_folder}/${TI_TOOLCHAIN_NAME}/${TI_PACKAGE_NAME}Config.cmake"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/source/cmake/${cmake_folder}/${TI_TOOLCHAIN_NAME}"
    )
endfunction()

################################################################################
# target_linker_file
#
# Sets the TI_LINKER_COMMAND_FILE property on the target, used by the compiler
# interface library to configure linker file
#
# Argument target: Which target to add linker file to
# Argument linkerfile: Path to linker command file appropriate for the toolchain
macro(target_linker_file target linkerfile)
    set_target_properties(${target} PROPERTIES TI_LINKER_COMMAND_FILE "${linkerfile}")
endmacro()

################################################################################
# target_map_file
#
# Configures map-file output on the target, is emitted in build directory alongside target
#
# Argument target: Which target to add linker file to
macro(target_map_file target)
    get_filename_component(basename "${target}" NAME_WE)
    get_filename_component(dirname "${target}" DIRECTORY)
    set_target_properties(
        ${target} PROPERTIES TI_LINKER_MAP_FILE "${CMAKE_CURRENT_BINARY_DIR}/${dirname}/${basename}.map"
    )
    set_target_properties(
        ${target} PROPERTIES TI_LINKER_REFERENCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/${dirname}/${basename}_reference"
    )
endmacro()
