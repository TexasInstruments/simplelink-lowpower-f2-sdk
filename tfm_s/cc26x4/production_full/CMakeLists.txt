#-------------------------------------------------------------------------------
# Copyright (c) 2019-2022 Texas Instruments Incorporated
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.7)

get_filename_component(TFM_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../source/third_party/tfm" ABSOLUTE)

#Tell cmake where our modules can be found
list(APPEND CMAKE_MODULE_PATH ${TFM_ROOT_DIR}/cmake)

#Include common stuff to control cmake.
include("Common/BuildSys")

#Include functionality to enable building the documentation.
#include("Common/BuildDoxygenDoc")

#Start an embedded project.
embedded_project_start(CONFIG "${TFM_ROOT_DIR}/configs/ConfigDefault.cmake")
project(tfm_s LANGUAGES ASM C)
embedded_project_fixup()

set(SECURE_FW_DIR "${TFM_ROOT_DIR}/secure_fw")
set(TEST_DIR      "${TFM_ROOT_DIR}/test")
set(INTERFACE_DIR "${TFM_ROOT_DIR}/interface")

if (NOT DEFINED TFM_LVL)
	message(FATAL_ERROR "Incomplete build configuration: TFM_LVL is undefined. ")
endif()

#Involve all IPC related sources in ipc's CMakeLists.inc, and switch core between IPC and Library.
if(TFM_PSA_API)
	include(${SECURE_FW_DIR}/spm/model_ipc/CMakeLists.inc)
else()
	include(${SECURE_FW_DIR}/spm/model_func/CMakeLists.inc)
endif()

include(${SECURE_FW_DIR}/spm/arch/CMakeLists.inc)

set(BUILD_CMSIS_CORE On)
set(BUILD_RETARGET On)
set(BUILD_NATIVE_DRIVERS On)
set(BUILD_STARTUP On)
set(BUILD_TARGET_CFG On)
# FIXME: The following TARGET flags are platform dependent.
#        It is required to add a mechanism to expose the
#        target capabilities and, based on them, set the
#        flags properly.
set(BUILD_TARGET_HARDWARE_KEYS On)
set(BUILD_TARGET_NV_COUNTERS On)
set(BUILD_CMSIS_DRIVERS On)
set(BUILD_TIME Off)
set(BUILD_UART_STDOUT On)
set(BUILD_FLASH On)

if(NOT DEFINED PLATFORM_CMAKE_FILE)
	message (FATAL_ERROR "Platform specific CMake is not defined. Please set PLATFORM_CMAKE_FILE.")
elseif(NOT EXISTS ${PLATFORM_CMAKE_FILE})
	message (FATAL_ERROR "Platform specific CMake \"${PLATFORM_CMAKE_FILE}\" file does not exist. Please fix value of PLATFORM_CMAKE_FILE.")
else()
	include(${PLATFORM_CMAKE_FILE})
endif()

if(NOT DEFINED S_SCATTER_FILE_NAME)
	message(FATAL_ERROR "ERROR: Incomplete Configuration: S_SCATTER_FILE_NAME not defined, Include this file from a Config*.cmake")
endif()
embedded_set_target_linker_file(TARGET ${PROJECT_NAME} PATH "${S_SCATTER_FILE_NAME}")

embedded_target_include_directories(TARGET ${PROJECT_NAME} PATH ${TFM_ROOT_DIR} ABSOLUTE APPEND)

#Create an object library to avoid compiling all source files twice, when two executables
#with different memory map need to be linked(BL2 non-swapping)
set(PROJECT_OBJ_LIB ${PROJECT_NAME}_obj_lib)
# add partition source files
include(CMakeLists.inc)
add_library(${PROJECT_OBJ_LIB} OBJECT ${ALL_SRC_C} ${ALL_SRC_C_S} ${ALL_SRC_ASM_S})

#Set common compiler flags
if (DEFINED CMSE_FLAGS)
	embedded_set_target_compile_flags(TARGET ${PROJECT_OBJ_LIB} LANGUAGE C APPEND FLAGS ${CMSE_FLAGS})
endif()
config_setting_shared_compiler_flags(${PROJECT_OBJ_LIB})

if(NOT DEFINED TARGET_NV_COUNTERS_ENABLE)
	set(TARGET_NV_COUNTERS_ENABLE OFF)
endif()

if(TARGET_NV_COUNTERS_ENABLE)
	embedded_set_target_compile_defines(TARGET ${PROJECT_OBJ_LIB} LANGUAGE C DEFINES TFM_NVCOUNTERS_ENABLE APPEND)
endif()

if (NOT DEFINED CORE_TEST)
	message(FATAL_ERROR "Incomplete build configuration: CORE_TEST is undefined.")
elseif(CORE_TEST)
	embedded_set_target_compile_defines(TARGET ${PROJECT_OBJ_LIB} LANGUAGE C DEFINES TFM_CORE_DEBUG TFM_PARTITION_TEST_CORE APPEND)
endif()

if (NOT DEFINED TFM_NS_CLIENT_IDENTIFICATION)
	message(FATAL_ERROR "Incomplete build configuration: TFM_NS_CLIENT_IDENTIFICATION is undefined.")
elseif (TFM_NS_CLIENT_IDENTIFICATION)
	target_compile_definitions(${PROJECT_OBJ_LIB} PRIVATE TFM_NS_CLIENT_IDENTIFICATION)
endif()

#Set include directories
if (DEFINED REMOTE_GEN_DIR)
	embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${REMOTE_GEN_DIR}/platform/ext/target/ti/cc26x4/partition ABSOLUTE APPEND)
	embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${REMOTE_GEN_DIR}/secure_fw/core/ipc ABSOLUTE APPEND)
	embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${REMOTE_GEN_DIR}/secure_fw/spm ABSOLUTE APPEND)
endif()
embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${TFM_ROOT_DIR} ABSOLUTE APPEND)
embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${SOURCE_REPO} ABSOLUTE APPEND)

# For the non-swapping BL2 configuration two executables need to be built.
# One can be executed from flash partition slot_0 and other from slot_1.
# Only the linking phase is different. This function captures common settings
# and eliminates copy-paste.
function(set_up_secure_fw_build)
	set( _OPTIONS_ARGS)                                #Option (on/off) arguments (e.g. IGNORE_CASE)
	set( _ONE_VALUE_ARGS S_TARGET VENEER_NAME POSTFIX) #Single option arguments (e.g. PATH "./foo/bar")
	set( _MULTI_VALUE_ARGS LINK_DEFINES)               #List arguments (e.g. LANGUAGES C ASM CXX)
	cmake_parse_arguments(_MY_PARAMS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})

	if (NOT DEFINED _MY_PARAMS_S_TARGET)
		message(FATAL_ERROR "set_up_secure_fw_build(): mandatory parameter 'S_TARGET' missing.")
	endif()

	if (NOT DEFINED _MY_PARAMS_VENEER_NAME)
		message(FATAL_ERROR "set_up_secure_fw_build(): mandatory parameter 'VENEER_NAME' missing.")
	endif()

	set(EXE_NAME ${_MY_PARAMS_S_TARGET}${_MY_PARAMS_POSTFIX})
	set(VENEER_NAME ${_MY_PARAMS_VENEER_NAME}${_MY_PARAMS_POSTFIX}.o)

	#Create linker target: add object library to executable
	add_executable(${EXE_NAME} $<TARGET_OBJECTS:${PROJECT_OBJ_LIB}>)

	#Set common linker flags
	config_setting_shared_linker_flags(${EXE_NAME})

	#Indicates to secure target(s) already created
	set(TARGET_TFM_S_EXISTED True PARENT_SCOPE)

	#Set individual linker flags per linker target/executable
	foreach(flag ${_MY_PARAMS_LINK_DEFINES})
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "${flag}")
	endforeach(flag)

	#[TI-TFM] TI CC26x4
	if(${TARGET_PLATFORM} STREQUAL "CC26X4")
		add_dependencies(${PROJECT_OBJ_LIB} ti_sysconfig)
		embedded_set_target_compile_defines(TARGET ${PROJECT_OBJ_LIB} LANGUAGE C APPEND DEFINES "TFM_ENABLED=1")

		if (ENABLE_KEY_STORAGE)
			#Add a dependency on the installed mbedCrypto target at build time
			#as we need to include certain files during compilation of TFM
			add_dependencies(${PROJECT_OBJ_LIB} ${MBEDCRYPTO_TARGET_NAME}_install)
			#Link mbedTLS library for secure key storage functions
			target_link_libraries(${EXE_NAME} ${MBEDCRYPTO_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX_C}mbedcrypto${CMAKE_STATIC_LIBRARY_SUFFIX_C})
		endif()

		#Link driverlib library for device trim support and flash functions
		target_link_libraries(${EXE_NAME} "${SOURCE_REPO}/ti/devices/cc13x4_cc26x4/driverlib/bin/gcc/driverlib.lib")
		#Remote generation directory
		if (DEFINED REMOTE_GEN_DIR)
			#Include directory for generated flash_layout.h
			embedded_set_target_link_includes(TARGET ${EXE_NAME} INCLUDES "${REMOTE_GEN_DIR}/platform/ext/target/ti/cc26x4/partition")
		endif()
	endif()

	embedded_set_target_linker_file(TARGET ${EXE_NAME} PATH "${S_SCATTER_FILE_NAME}")

	#Set macro definitions for the project.
	set(TARGET_COMPILE_DEFINITIONS __thumb2__ __DOMAIN_NS=0 DOMAIN_NS=__DOMAIN_NS TFM_LVL=${TFM_LVL} ${DEBUG_AUTHENTICATION})
	embedded_set_target_compile_defines(TARGET ${PROJECT_OBJ_LIB} LANGUAGE C DEFINES ${TARGET_COMPILE_DEFINITIONS} APPEND)

	if (REGRESSION OR CORE_TEST OR CORE_IPC)
		if (DEFINED TFM_PARTITION_TEST_SECURE_SERVICES AND TFM_PARTITION_TEST_SECURE_SERVICES)
			#The test service veneers in the tfm_secure_tests library may not be
			#referenced in the secure binary so the veneer objects are explicitly loaded
			#from the secure tests library. However by generating the veneer files from
			#the manifests, all the iovec interfaced veneers are in a single file in the
			#secure_fw directory. The core test partitions use the veneers with the
			#iovec API, so we only need the explicit load in case the secure client test
			#partition is present.
			#FIXME Remove the explicit load and the above comment once the secure client
			#test partition uses the generated veneers.
			if(${COMPILER} STREQUAL "ARMCLANG")
				target_link_libraries(${EXE_NAME} tfm_attest tfm_secure_tests tfm_attest tfm_crypto tfm_storage tfm_audit tfm_platform $<TARGET_LINKER_FILE:tfm_secure_tests>\(*veneers.o\) tfm_attest)
			elseif(${COMPILER} STREQUAL "GNUARM")
#				target_link_libraries(${EXE_NAME})
			elseif(${COMPILER} STREQUAL "TICLANGARM")
				target_link_libraries(${EXE_NAME} tfm_secure_tests tfm_crypto tfm_storage tfm_audit tfm_platform tfm_attest)
			else()
				message(FATAL_ERROR "unknown compiler" )
			endif()
		else()
#			target_link_libraries(${EXE_NAME} tfm_attest tfm_crypto tfm_storage tfm_audit tfm_platform tfm_secure_tests tfm_attest)
		endif()
	else()
		target_link_libraries(${EXE_NAME} tfm_attest tfm_crypto tfm_storage tfm_audit tfm_platform tfm_attest)
	endif()

	if (TFM_PARTITION_CRYPTO)
		target_link_libraries(${EXE_NAME} tfm_crypto)
	endif()

	if (TFM_PARTITION_INTERNAL_TRUSTED_STORAGE)
		target_link_libraries(${EXE_NAME} tfm_internal_trusted_storage)
	endif()

	if (TFM_PARTITION_PROTECTED_STORAGE)
		target_link_libraries(${EXE_NAME} tfm_storage)
	endif()

	if (TFM_PARTITION_AUDIT_LOG)
		target_link_libraries(${EXE_NAME} tfm_audit)
	endif()

	if (TFM_PARTITION_PLATFORM)
		target_link_libraries(${EXE_NAME} tfm_platform)
	endif()

	if (TFM_PARTITION_INITIAL_ATTESTATION)
		target_link_libraries(${EXE_NAME} tfm_attest)
	endif()

	target_link_libraries(${EXE_NAME} libtfmsprt)

	embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_LVL=${TFM_LVL}")

	if (NOT DEFINED TFM_PARTITION_TEST_CORE)
		message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_TEST_CORE is undefined. ")
	elseif (TFM_PARTITION_TEST_CORE)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_TEST_CORE")
	endif()

	if (NOT DEFINED TFM_PARTITION_TEST_SECURE_SERVICES)
		message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_TEST_SECURE_SERVICES is undefined. ")
	elseif (TFM_PARTITION_TEST_SECURE_SERVICES)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_TEST_SECURE_SERVICES")
	endif()

	if (NOT DEFINED TEST_FRAMEWORK_S)
		message(FATAL_ERROR "Incomplete build configuration: TEST_FRAMEWORK_S is undefined.")
	elseif (TEST_FRAMEWORK_S)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TEST_FRAMEWORK_S")
	endif()

	if (NOT DEFINED TEST_FRAMEWORK_NS)
		message(FATAL_ERROR "Incomplete build configuration: TEST_FRAMEWORK_NS is undefined.")
	elseif (TEST_FRAMEWORK_NS)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TEST_FRAMEWORK_NS")
	endif()

	if (NOT DEFINED BL2)
		message(FATAL_ERROR "Incomplete build configuration: BL2 is undefined. ")
	elseif (BL2)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "BL2")
	endif()

	if (NOT DEFINED TFM_PSA_API)
		message(FATAL_ERROR "Incomplete build configuration: TFM_PSA_API is undefined. ")
	elseif (TFM_PSA_API)
		embedded_set_target_link_defines(TARGET ${PROJECT_NAME} DEFINES "TFM_PSA_API")
	endif()

	if(CORE_TEST)
		set(SECURE_AXF_DIR_PREFIX "${CMAKE_BINARY_DIR}/unit_test/")
		set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${SECURE_AXF_DIR_PREFIX})
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_TEST_CORE")
	endif()

	if(NOT DEFINED PLATFORM_LINK_INCLUDES)
		message(FATAL_ERROR "ERROR: Incomplete Configuration: PLATFORM_LINK_INCLUDES is not defined.")
	endif()
	embedded_set_target_link_includes(TARGET ${EXE_NAME} INCLUDES "${PLATFORM_LINK_INCLUDES}")

	#Generate binary file from executable
	if (${COMPILER} STREQUAL "TICLANGARM")
		message("TICLANG-ARM bug CODEGEN-6042: skipping compiler_generate_binary_output()")
	else ()
#		compiler_generate_binary_output(${EXE_NAME})
	endif ()

	#Configure where we put the CMSE veneers generated by the compiler.
	if (DEFINED S_VENEER_FILE_LOCATION)
		set(S_VENEER_FILE "${S_VENEER_FILE_LOCATION}/${VENEER_NAME}")
	else()
		set(S_VENEER_FILE "${CMAKE_CURRENT_BINARY_DIR}/${VENEER_NAME}")
	endif()
	compiler_set_cmse_output(${EXE_NAME} "${S_VENEER_FILE}")

	#Configure what file shall be installed.
	#Set install location. Keep original value to avoid overriding command line settings.
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Default install location for secure_fw." FORCE)
	endif()

	#Export files needed to interface external applications at: <build_dir>/install/export/tfm/
	install(DIRECTORY ${TFM_ROOT_DIR}/interface/include/
			DESTINATION export/tfm/inc)

	install(DIRECTORY ${TFM_ROOT_DIR}/interface/src/
			DESTINATION export/tfm/src)

	install(FILES ${S_VENEER_FILE} DESTINATION export/tfm/veneers)

	#Collect executables to common location: <build_dir>/install/outputs/
	if (DEFINED SECURE_AXF_DIR_PREFIX)
		set(MY_BINARY_DIR ${SECURE_AXF_DIR_PREFIX})
	else()
		set(MY_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
	endif()

	install(FILES ${MY_BINARY_DIR}/${EXE_NAME}.axf
#				  ${MY_BINARY_DIR}/${EXE_NAME}.bin
			DESTINATION outputs/${TARGET_PLATFORM}/)

	install(FILES ${MY_BINARY_DIR}/${EXE_NAME}.axf
#				  ${MY_BINARY_DIR}/${EXE_NAME}.bin
			DESTINATION outputs/fvp/)
endfunction()

#Adds the test directory
#add_subdirectory(${TFM_ROOT_DIR}/test ${CMAKE_BINARY_DIR}/test)

if (TFM_PARTITION_CRYPTO)
	#Add the crypto library target
	add_subdirectory(${SECURE_FW_DIR}/partitions/crypto ${CMAKE_BINARY_DIR}/secure_fw/partitions/crypto)
endif()

if (TFM_PARTITION_INTERNAL_TRUSTED_STORAGE)
	#Add the internal trusted storage library target
	add_subdirectory(${SECURE_FW_DIR}/partitions/internal_trusted_storage ${CMAKE_BINARY_DIR}/secure_fw/partitions/internal_trusted_storage)
endif()

if (TFM_PARTITION_PROTECTED_STORAGE)
	#Add the protected storage library target
	add_subdirectory(${SECURE_FW_DIR}/partitions/protected_storage ${CMAKE_BINARY_DIR}/secure_fw/partitions/protected_storage)
endif()

if (TFM_PARTITION_AUDIT_LOG)
	#Add the audit logging library target
	add_subdirectory(${SECURE_FW_DIR}/partitions/audit_logging ${CMAKE_BINARY_DIR}/secure_fw/partitions/audit_logging)
endif()

if (TFM_PARTITION_PLATFORM)
	#Add the platform service library target
	add_subdirectory(${SECURE_FW_DIR}/partitions/platform ${CMAKE_BINARY_DIR}/secure_fw/partitions/platform)
endif()

if (TFM_PARTITION_INITIAL_ATTESTATION)
	#Add the initial attestation service library target
	add_subdirectory(${SECURE_FW_DIR}/partitions/initial_attestation ${CMAKE_BINARY_DIR}/secure_fw/partitions/initial_attestation)
endif()

#Add the secure runtime library target
add_subdirectory(${SECURE_FW_DIR}/partitions/lib/sprt ${CMAKE_BINARY_DIR}/secure_fw/partitions/lib/sprt)

if (LINK_TO_BOTH_MEMORY_REGION)
	#Link to primary memory region
	set_up_secure_fw_build(S_TARGET      ${PROJECT_NAME}
						   VENEER_NAME   s_veneers
						   POSTFIX       "_0")

	#Link to secondary memory region(add extra linker flag)
	set_up_secure_fw_build(S_TARGET      ${PROJECT_NAME}
						   LINK_DEFINES  "LINK_TO_SECONDARY_PARTITION"
						   VENEER_NAME   s_veneers
						   POSTFIX       "_1")
else()
	#Link to primary memory region only
	set_up_secure_fw_build(S_TARGET      ${PROJECT_NAME}
						   VENEER_NAME   s_veneers)
endif()

#Finally let CMake system apply changes after the whole project is defined.
if (TARGET ${PROJECT_NAME})
	embedded_project_end(${PROJECT_NAME})
endif()

if (TARGET ${PROJECT_NAME}_0)
	embedded_project_end(${PROJECT_NAME}_0)
endif()

if (TARGET ${PROJECT_NAME}_1)
	embedded_project_end(${PROJECT_NAME}_1)
endif()

message("The CC26x4 production_full compile configuration is as follows:")
message("- BUILD_TARGET_HARDWARE_KEYS: " ${BUILD_TARGET_HARDWARE_KEYS})
message("- BUILD_TARGET_NV_COUNTERS:   " ${BUILD_TARGET_NV_COUNTERS})

embedded_project_end(${PROJECT_OBJ_LIB})