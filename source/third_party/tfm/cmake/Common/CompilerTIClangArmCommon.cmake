#-------------------------------------------------------------------------------
# Copyright (c) 2019 Texas Instruments Incorporated
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# This file contains settings to specify how TICLANGARM shall be used

if (NOT DEFINED TICLANGARM_PATH)
    message(FATAL_ERROR "TICLANGARM_PATH not defined: must assign to install directory of TICLANGARM compiler.")
endif ()

#
#  ======== tiarmclang_version ========
#  Extract the compiler version
#
#  Parameters
#      install_dir - [in] pathname to compiler install directory
#      version - [out] contains version string
#
function(tiarmclang_version clang_install version)

    # compute executable pathname
    set (compiler "${clang_install}/bin/tiarmclang")

    # call specified executable
    execute_process(COMMAND "${compiler}" -v OUTPUT_VARIABLE OUTPUT ERROR_VARIABLE OUTPUT)

    #Cut off version number. Just the numbers ignore anything after.
    STRING(REGEX REPLACE ".*clang version ([0-9]+)\.([0-9]+)\..*" "\\1.\\2" VER "${OUTPUT}")

    # if version empty, get version from tiarmclang 
    if (NOT VER)
            message(FATAL_ERROR "tiarmclang_version(): failed to extract version number from compiler")
    endif ()

    set(${version} ${VER} PARENT_SCOPE)
endfunction()

tiarmclang_version(${TICLANGARM_PATH} TICLANGARM_VER)
STRING(REGEX REPLACE "^([0-9]+)\.([0-9]+)(\.[0-9]+)*.*" "CompilerTIClangArm\\1\\2" TICLANGARM_MODULE "${TICLANGARM_VER}")

# tell cmake which compiler we use
if (EXISTS "c:/")
    set (CMAKE_C_COMPILER "${TICLANGARM_PATH}/bin/tiarmclang.exe")
    set (CMAKE_CXX_COMPILER "${TICLANGARM_PATH}/bin/tiarmclang.exe")
    set (CMAKE_ASM_COMPILER "${TICLANGARM_PATH}/bin/tiarmclang.exe")
else ()
    set (CMAKE_C_COMPILER "${TICLANGARM_PATH}/bin/tiarmclang")
    set (CMAKE_CXX_COMPILER "${TICLANGARM_PATH}/bin/tiarmclang")
    set (CMAKE_ASM_COMPILER "${TICLANGARM_PATH}/bin/tiarmclang")
endif ()

if ("CXX" IN_LIST languages)
    set(CMAKE_CXX_COMPILER_ID "TICLANGARM" CACHE INTERNAL "CXX compiler ID" FORCE)
    include(Compiler/TIClangArm-CXX)
endif ()

if ("C" IN_LIST languages)
    set(CMAKE_C_COMPILER_ID "TICLANGARM" CACHE INTERNAL "C compiler ID" FORCE)
    include(Compiler/TIClangArm-C)
endif ()

if ("ASM" IN_LIST languages)
    set(CMAKE_C_COMPILER_ID "TICLANGARM" CACHE INTERNAL "ASM compiler ID" FORCE)
    include(Compiler/TIClangArm-ASM)
endif ()

function(compiler_set_linkercmdfile)
    set( _OPTIONS_ARGS )                        #Option (on/off) arguments.
    set( _ONE_VALUE_ARGS TARGET PATH)           #Single option arguments.
    set( _MULTI_VALUE_ARGS DEFINES INCLUDES)    #List arguments
    cmake_parse_arguments(_MY_PARAMS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN} )

    #Check passed parameters
    if (NOT _MY_PARAMS_TARGET)
        message(FATAL_ERROR "compiler_set_linkercmdfile: mandatory parameter 'TARGET' is missing.")
    endif ()
    if (NOT TARGET ${_MY_PARAMS_TARGET})
        message(FATAL_ERROR "compiler_set_linkercmdfile: value of parameter 'TARGET' is invalid.")
    endif ()

    if (NOT _MY_PARAMS_PATH)
        message(FATAL_ERROR "compiler_set_linkercmdfile: mandatory parameter 'PATH' is missing.")
    endif ()
    set(_FILE_PATH ${_MY_PARAMS_PATH})

    #Compose additional command line switches from macro definitions.
    set(_FLAGS "")
    if (_MY_PARAMS_DEFINES)
        foreach(_DEFINE IN LISTS _MY_PARAMS_DEFINES)
            string(APPEND _FLAGS " -Wl,--define,${_DEFINE}")
        endforeach()
    endif ()

    #Compose additional command line switches from include paths.
    if (_MY_PARAMS_INCLUDES)
        foreach(_INCLUDE_P IN LISTS _MY_PARAMS_INCLUDES)
            string(APPEND _FLAGS " -I\"${_INCLUDE_P}\"")
        endforeach()
    endif ()

    #Note: the space before the option is important!
    set_property(TARGET ${_MY_PARAMS_TARGET} APPEND_STRING PROPERTY LINK_FLAGS " ${_FLAGS} ${_FILE_PATH}")
    set_property(TARGET ${_MY_PARAMS_TARGET} APPEND PROPERTY LINK_DEPENDS ${_FILE_PATH})

    #Tell cmake .map files shall be removed when project is cleaned (make clean)
    get_filename_component(_TARGET_BASE_NAME ${_MY_PARAMS_TARGET} NAME_WE)
    get_directory_property(_ADDITIONAL_MAKE_CLEAN_FILES DIRECTORY "./" ADDITIONAL_MAKE_CLEAN_FILES)
    set_directory_properties(PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${_ADDITIONAL_MAKE_CLEAN_FILES} ${_TARGET_BASE_NAME}.map")
endfunction()

function(compiler_set_cmse_output TARGET FILE_PATH)
    #Note: the space before the option is important!
    set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--import_cmse_lib_out,${FILE_PATH}")

    #Tell cmake cmse output is a generated object file.
    SET_SOURCE_FILES_PROPERTIES("${FILE_PATH}" PROPERTIES EXTERNAL_OBJECT true GENERATED true)

    #Tell cmake cmse output shall be removed by clean target.
    get_directory_property(_ADDITIONAL_MAKE_CLEAN_FILES DIRECTORY "./" ADDITIONAL_MAKE_CLEAN_FILES)
    set_directory_properties(PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${_ADDITIONAL_MAKE_CLEAN_FILES} ${FILE_PATH}")
endfunction()

function(compiler_merge_library)
    set( _OPTIONS_ARGS )                #Option (on/off) arguments.
    set( _ONE_VALUE_ARGS DEST)          #Single option arguments.
    set( _MULTI_VALUE_ARGS LIBS)        #List arguments
    cmake_parse_arguments(_MY_PARAMS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN} )

    #Check passed parameters
    if (NOT _MY_PARAMS_DEST)
        message(FATAL_ERROR "embedded_merge_library: no destination library target specified.")
    endif ()

    #Check if destination is a target
    if (NOT TARGET ${_MY_PARAMS_DEST})
        message(FATAL_ERROR "embedded_merge_library: parameter DEST must be a target already defined.")
    endif ()

    #Check if destination is a library
    get_target_property(_tmp ${_MY_PARAMS_DEST} TYPE)
    if (NOT "${_tmp}" STREQUAL "STATIC_LIBRARY")
        message(FATAL_ERROR "embedded_merge_library: parameter DEST must be a static library target.")
    endif ()

    #Check list if libraries to be merged
    if (NOT _MY_PARAMS_LIBS)
        message(FATAL_ERROR "embedded_merge_library: no source libraries specified. Please see the LIBS parameter.")
    endif ()

    #Mark each library file as a generated external object. This is needed to
    #avoid error because CMake has no info how these can be built.
    SET_SOURCE_FILES_PROPERTIES(${_MY_PARAMS_LIBS} PROPERTIES EXTERNAL_OBJECT true GENERATED true)

    #Add additional input to target
    target_sources(${_MY_PARAMS_DEST} PRIVATE ${_MY_PARAMS_LIBS})
endfunction()

function(compiler_generate_binary_output TARGET)
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_ARMCCLANG_OBJCOPY} ARGS -O binary $<TARGET_FILE:${TARGET}> $<TARGET_FILE_DIR:${TARGET}>/${TARGET}.bin)
endfunction()
