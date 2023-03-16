#-------------------------------------------------------------------------------
# Copyright (c) 2017-2020, Arm Limited. All rights reserved.
# Copyright (c) 2019-2021, Texas Instruments Incorporated.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.7)

set(TFM_BUILD_IN_SPE ON)

#Tell cmake where our modules can be found
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../cmake)

set(SECURE_FW_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(TFM_ROOT_DIR  "${SECURE_FW_DIR}/..")
set(TEST_DIR      "${TFM_ROOT_DIR}/test")
set(INTERFACE_DIR "${TFM_ROOT_DIR}/interface")

#Include common stuff to control cmake.
include("Common/BuildSys")

#Start an embedded project.
embedded_project_start(CONFIG "${TFM_ROOT_DIR}/configs/ConfigDefault.cmake")
project(tfm_s LANGUAGES ASM C)
embedded_project_fixup()

#Include functionality to enable building the documentation.
include("Common/BuildDoxygenDoc")
include("Common/BuildSphinxDoc")

if (NOT DEFINED TFM_LVL)
	message(FATAL_ERROR "Incomplete build configuration: TFM_LVL is undefined. ")
endif()

if (NOT DEFINED TFM_PARTITION_AUDIT_LOG)
	message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_AUDIT_LOG is undefined.")
endif()

if (NOT DEFINED TFM_PARTITION_PLATFORM)
	message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_PLATFORM is undefined.")
endif()

if (NOT DEFINED TFM_PARTITION_PROTECTED_STORAGE)
	message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_PROTECTED_STORAGE is undefined.")
endif()

if (NOT DEFINED TFM_PARTITION_INTERNAL_TRUSTED_STORAGE)
	message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_INTERNAL_TRUSTED_STORAGE is undefined.")
endif()

if (NOT DEFINED TFM_PARTITION_CRYPTO)
	message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_CRYPTO is undefined.")
endif()

if (NOT DEFINED TFM_PARTITION_INITIAL_ATTESTATION)
	message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_INITIAL_ATTESTATION is undefined.")
endif()

if (NOT DEFINED TFM_PARTITION_TEST_CORE)
	message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_TEST_CORE is undefined. ")
endif()

if (NOT DEFINED TFM_PARTITION_TEST_CORE_IPC)
	message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_TEST_CORE_IPC is undefined.")
endif()

if (NOT DEFINED TFM_PARTITION_TEST_SECURE_SERVICES)
	message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_TEST_SECURE_SERVICES is undefined. ")
endif()

if (NOT DEFINED TFM_PARTITION_TEST_PS)
	message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_TEST_PS is undefined.")
endif()

if (NOT DEFINED TEST_FRAMEWORK_S)
	message(FATAL_ERROR "Incomplete build configuration: TEST_FRAMEWORK_S is undefined.")
endif()

if (NOT DEFINED TEST_FRAMEWORK_NS)
	message(FATAL_ERROR "Incomplete build configuration: TEST_FRAMEWORK_NS is undefined.")
endif()

if (NOT DEFINED BL2)
	message(FATAL_ERROR "Incomplete build configuration: BL2 is undefined. ")
endif()

if (NOT DEFINED TFM_PSA_API)
	message(FATAL_ERROR "Incomplete build configuration: TFM_PSA_API is undefined. ")
endif()

if(NOT DEFINED PLATFORM_LINK_INCLUDES)
	message(FATAL_ERROR "ERROR: Incomplete Configuration: PLATFORM_LINK_INCLUDES is not defined.")
endif()

if (NOT DEFINED TFM_ENABLE_IRQ_TEST)
	message(FATAL_ERROR "ERROR: Incomplete Configuration: TFM_ENABLE_IRQ_TEST is not defined.")
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
set(BUILD_PLAT_TEST Off)
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
add_library(${PROJECT_OBJ_LIB} OBJECT ${ALL_SRC_C} ${ALL_SRC_C_S} ${ALL_SRC_ASM} ${ALL_SRC_ASM_S})

#Add platform specific definitions in SPE
if (DEFINED TFM_PLATFORM_SECURE_DEFS)
	embedded_set_target_compile_defines(TARGET ${PROJECT_OBJ_LIB} LANGUAGE C DEFINES ${TFM_PLATFORM_SECURE_DEFS} APPEND)
	embedded_set_target_compile_defines(TARGET ${PROJECT_OBJ_LIB} LANGUAGE ASM DEFINES ${TFM_PLATFORM_SECURE_DEFS} APPEND)
endif()

#Set common compiler flags
if (DEFINED CMSE_FLAGS)
	embedded_set_target_compile_flags(TARGET ${PROJECT_OBJ_LIB} LANGUAGE C APPEND FLAGS ${CMSE_FLAGS})
endif()
config_setting_shared_compiler_flags(${PROJECT_OBJ_LIB})

if (NOT DEFINED TARGET_NV_COUNTERS_ENABLE)
	set(TARGET_NV_COUNTERS_ENABLE OFF)
endif()

if (TARGET_NV_COUNTERS_ENABLE)
	embedded_set_target_compile_defines(TARGET ${PROJECT_OBJ_LIB} LANGUAGE C DEFINES TFM_NVCOUNTERS_ENABLE APPEND)
endif()

if (BOOT_DATA_AVAILABLE)
	embedded_set_target_compile_defines(TARGET ${PROJECT_OBJ_LIB} LANGUAGE C DEFINES BOOT_DATA_AVAILABLE APPEND)
endif()

if (LEGACY_TFM_TLV_HEADER)
	embedded_set_target_compile_defines(TARGET ${PROJECT_OBJ_LIB} LANGUAGE C DEFINES LEGACY_TFM_TLV_HEADER APPEND)
endif()

if (NOT DEFINED CORE_TEST)
	message(FATAL_ERROR "Incomplete build configuration: CORE_TEST is undefined.")
elseif(CORE_TEST)
	embedded_set_target_compile_defines(TARGET ${PROJECT_OBJ_LIB} LANGUAGE C DEFINES TFM_CORE_DEBUG APPEND)
endif()

if (NOT DEFINED TFM_NS_CLIENT_IDENTIFICATION)
	message(FATAL_ERROR "Incomplete build configuration: TFM_NS_CLIENT_IDENTIFICATION is undefined.")
elseif (TFM_NS_CLIENT_IDENTIFICATION)
	target_compile_definitions(${PROJECT_OBJ_LIB} PRIVATE TFM_NS_CLIENT_IDENTIFICATION)
endif()

if (NOT DEFINED DEBUG_AUTHENTICATION)
	set(DEBUG_AUTHENTICATION "DAUTH_CHIP_DEFAULT")
endif()

#Set include directories
embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${TFM_ROOT_DIR} ABSOLUTE APPEND)

# For the non-swapping BL2 configuration two executables need to be built.
# One can be executed from the primary slot flash partition and other from the
# the secondary slot. Only the linking phase is different. This function
# captures common settings and eliminates copy-paste.
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

	#Set individual linker flags per linker target/executable
	foreach(flag ${_MY_PARAMS_LINK_DEFINES})
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "${flag}")
	endforeach(flag)

	#[TI-TFM] TI CC26x4
	if(${TARGET_PLATFORM} STREQUAL "CC26X4")
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

	# The order of the libraries linked below is important for GCC
	# Because the GCC linker will search an archive only once.
	# If the a library has some symbols that are defined in the library which is linked before it,
	# it will not cause the linker to search again.
	# So please put a library before what it relies on.
	# If the link dependency is complicated or it is difficult to sort out link
	# dependency by only adjusting the order, it is recommended to explicitly
	# add the dependency in the dedicated CMake file of corresponding module, to
	# simplify the ordering and avoid any potential linking issue.
	if (CORE_TEST OR TFM_PARTITION_TEST_SECURE_SERVICES)
		target_link_libraries(${EXE_NAME} tfm_secure_tests)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_TEST_SECURE_SERVICES")
	endif()

	if (TFM_PARTITION_INITIAL_ATTESTATION)
		target_link_libraries(${EXE_NAME} tfm_attest)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_INITIAL_ATTESTATION")
	endif()

	if (TFM_PARTITION_PROTECTED_STORAGE)
		target_link_libraries(${EXE_NAME} tfm_storage)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_PROTECTED_STORAGE")
	endif()

	if (TFM_PARTITION_CRYPTO)
		target_link_libraries(${EXE_NAME} tfm_crypto)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_CRYPTO")
	endif()

	if (TFM_PARTITION_INTERNAL_TRUSTED_STORAGE)
		target_link_libraries(${EXE_NAME} tfm_internal_trusted_storage)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_INTERNAL_TRUSTED_STORAGE")
	endif()

	if (TFM_PARTITION_AUDIT_LOG)
		target_link_libraries(${EXE_NAME} tfm_audit)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_AUDIT_LOG")
	endif()

	if (TFM_PARTITION_PLATFORM)
		target_link_libraries(${EXE_NAME} tfm_platform)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_PLATFORM")
	endif()

	#Add the PSA API compliance test secure libraries
	if(PSA_API_TEST_IPC)
		add_definitions(-DPSA_API_TEST_IPC)
		target_link_libraries(${EXE_NAME} "${PSA_API_TEST_BUILD_PATH}/partition/client_partition.a")
		target_link_libraries(${EXE_NAME} "${PSA_API_TEST_BUILD_PATH}/partition/server_partition.a")
		target_link_libraries(${EXE_NAME} "${PSA_API_TEST_BUILD_PATH}/partition/driver_partition.a")
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "PSA_API_TEST_IPC")
	endif()

	target_link_libraries(${EXE_NAME} libtfmsprt)

	embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_LVL=${TFM_LVL}")

	if (TFM_PARTITION_TEST_CORE)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_TEST_CORE")
	endif()

	if (TFM_PARTITION_TEST_CORE_IPC)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_TEST_CORE_IPC")
	endif()

	if (TFM_PARTITION_TEST_PS)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_TEST_PS")
	endif()

	if (TEST_FRAMEWORK_S)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TEST_FRAMEWORK_S")
	endif()

	if (TEST_FRAMEWORK_NS)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TEST_FRAMEWORK_NS")
	endif()

	if (BL2)
		#Add BL2 and MCUBOOT_IMAGE_NUMBER defines to linker to resolve symbols in region_defs.h and flash_layout.h
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "BL2" "MCUBOOT_IMAGE_NUMBER=${MCUBOOT_IMAGE_NUMBER}")
	endif()

	if (TFM_PSA_API)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PSA_API")
		if (DEFINED TFM_MULTI_CORE_TOPOLOGY AND TFM_MULTI_CORE_TOPOLOGY)
			embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_MULTI_CORE_TOPOLOGY")
		endif()
	endif()

	if(CORE_TEST)
		set(SECURE_AXF_DIR_PREFIX "${CMAKE_BINARY_DIR}/unit_test/")
		set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${SECURE_AXF_DIR_PREFIX})
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_TEST_CORE")
	endif()

	embedded_set_target_link_includes(TARGET ${EXE_NAME} INCLUDES "${PLATFORM_LINK_INCLUDES}")

	if (TFM_ENABLE_IRQ_TEST)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_ENABLE_IRQ_TEST")
	endif()

	if (TFM_MULTI_CORE_TEST)
		add_definitions(-DTFM_MULTI_CORE_TEST)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_MULTI_CORE_TEST")
	endif()

	#Generate binary file from executable
	if (${COMPILER} STREQUAL "TICLANGARM")
		message("TICLANG-ARM bug CODEGEN-6042: skipping compiler_generate_binary_output()")
	else ()
		compiler_generate_binary_output(${EXE_NAME})

		#Generate intel hex file from axf
		compiler_generate_hex_output(${EXE_NAME})

		#Generate elf file from axf
		compiler_generate_elf_output(${EXE_NAME})
	endif ()

	if (NOT DEFINED TFM_MULTI_CORE_TOPOLOGY OR NOT TFM_MULTI_CORE_TOPOLOGY)
		#Configure where we put the CMSE veneers generated by the compiler.
		set(S_VENEER_FILE "${CMAKE_CURRENT_BINARY_DIR}/${VENEER_NAME}")
		compiler_set_cmse_output(${EXE_NAME} "${S_VENEER_FILE}")
	endif()

	#Configure what file shall be installed.
	#Set install location. Keep original value to avoid overriding command line settings.
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Default install location for secure_fw." FORCE)
	endif()

	#Export files needed to interface external applications at: <build_dir>/install/export/tfm/
	set(INTERFACE_INC_DIR "${INTERFACE_DIR}/include")
	set(INTERFACE_SRC_DIR "${INTERFACE_DIR}/src")
	set(EXPORT_INC_DIR "export/tfm/include")
	set(EXPORT_SRC_DIR "export/tfm/src")
	#Headers
	install(FILES       ${INTERFACE_INC_DIR}/psa/client.h
						${INTERFACE_INC_DIR}/psa/error.h
			DESTINATION ${EXPORT_INC_DIR}/psa)

	install(FILES       ${INTERFACE_INC_DIR}/os_wrapper/common.h
						${INTERFACE_INC_DIR}/os_wrapper/mutex.h
			DESTINATION ${EXPORT_INC_DIR}/os_wrapper)

	install(FILES       ${INTERFACE_INC_DIR}/psa_manifest/sid.h
			DESTINATION ${EXPORT_INC_DIR}/psa_manifest)

	install(FILES       ${INTERFACE_INC_DIR}/tfm_api.h
						${INTERFACE_INC_DIR}/tfm_ns_interface.h
						${INTERFACE_INC_DIR}/tfm_ns_svc.h
			DESTINATION ${EXPORT_INC_DIR})

	if (TFM_MULTI_CORE_TOPOLOGY)
		install(FILES       ${INTERFACE_INC_DIR}/tfm_multi_core_api.h
							${INTERFACE_INC_DIR}/tfm_ns_mailbox.h
							${INTERFACE_INC_DIR}/tfm_mailbox.h
				DESTINATION ${EXPORT_INC_DIR})
	else()
		install(FILES       ${INTERFACE_INC_DIR}/tfm_veneers.h
				DESTINATION ${EXPORT_INC_DIR})
	endif()

	#Sources
	if (TFM_MULTI_CORE_TOPOLOGY)
		install(FILES       ${INTERFACE_SRC_DIR}/tfm_ns_mailbox.c
							${INTERFACE_SRC_DIR}/tfm_multi_core_api.c
							${INTERFACE_SRC_DIR}/tfm_multi_core_psa_ns_api.c
				DESTINATION ${EXPORT_SRC_DIR})
	else()
		install(FILES       ${INTERFACE_SRC_DIR}/tfm_ns_interface.c
				DESTINATION ${EXPORT_SRC_DIR})

		if(TFM_PSA_API)
			install(FILES       ${INTERFACE_SRC_DIR}/tfm_psa_ns_api.c
					DESTINATION ${EXPORT_SRC_DIR})
		endif()
	endif()

	if (TFM_NS_CLIENT_IDENTIFICATION)
		install(FILES       ${INTERFACE_INC_DIR}/tfm_nspm_api.h
							${INTERFACE_INC_DIR}/tfm_nspm_svc_handler.h
				DESTINATION ${EXPORT_INC_DIR})
		install(FILES       ${INTERFACE_SRC_DIR}/tfm_nspm_api.c
							${INTERFACE_SRC_DIR}/tfm_nspm_svc_handler.c
				DESTINATION ${EXPORT_SRC_DIR})
	endif()

	if (TFM_PARTITION_PROTECTED_STORAGE)
		install(FILES       ${INTERFACE_INC_DIR}/psa/protected_storage.h
				DESTINATION ${EXPORT_INC_DIR}/psa)
		if (TFM_PSA_API)
			install(FILES       ${INTERFACE_SRC_DIR}/tfm_ps_ipc_api.c
					DESTINATION ${EXPORT_SRC_DIR})
		else()
			install(FILES       ${INTERFACE_SRC_DIR}/tfm_ps_func_api.c
					DESTINATION ${EXPORT_SRC_DIR})
		endif()
	endif()

	if (TFM_PARTITION_INTERNAL_TRUSTED_STORAGE)
		install(FILES       ${INTERFACE_INC_DIR}/psa/internal_trusted_storage.h
							${INTERFACE_INC_DIR}/psa/storage_common.h
				DESTINATION ${EXPORT_INC_DIR}/psa)
		if (TFM_PSA_API)
			install(FILES       ${INTERFACE_SRC_DIR}/tfm_its_ipc_api.c
					DESTINATION ${EXPORT_SRC_DIR})
		else()
			install(FILES       ${INTERFACE_SRC_DIR}/tfm_its_func_api.c
					DESTINATION ${EXPORT_SRC_DIR})
		endif()
	endif()

	if (TFM_PARTITION_CRYPTO)
		install(FILES       ${INTERFACE_INC_DIR}/psa/crypto_extra.h
							${INTERFACE_INC_DIR}/psa/crypto_compat.h
							${INTERFACE_INC_DIR}/psa/crypto.h
							${INTERFACE_INC_DIR}/psa/crypto_client_struct.h
							${INTERFACE_INC_DIR}/psa/crypto_sizes.h
							${INTERFACE_INC_DIR}/psa/crypto_struct.h
							${INTERFACE_INC_DIR}/psa/crypto_types.h
							${INTERFACE_INC_DIR}/psa/crypto_values.h
				DESTINATION ${EXPORT_INC_DIR}/psa)
		install(FILES       ${INTERFACE_INC_DIR}/tfm_crypto_defs.h
				DESTINATION ${EXPORT_INC_DIR})
		if (TFM_PSA_API)
			install(FILES       ${INTERFACE_SRC_DIR}/tfm_crypto_ipc_api.c
					DESTINATION ${EXPORT_SRC_DIR})
		else()
			install(FILES       ${INTERFACE_SRC_DIR}/tfm_crypto_func_api.c
					DESTINATION ${EXPORT_SRC_DIR})
		endif()
	endif()

	if (TFM_PARTITION_INITIAL_ATTESTATION)
		install(FILES       ${INTERFACE_INC_DIR}/psa/initial_attestation.h
				DESTINATION ${EXPORT_INC_DIR}/psa)
		if (TFM_PSA_API)
			install(FILES       ${INTERFACE_SRC_DIR}/tfm_initial_attestation_ipc_api.c
					DESTINATION ${EXPORT_SRC_DIR})
		else()
			install(FILES       ${INTERFACE_SRC_DIR}/tfm_initial_attestation_func_api.c
					DESTINATION ${EXPORT_SRC_DIR})
		endif()
	endif()

	if(TFM_PARTITION_AUDIT_LOG)
		install(FILES       ${INTERFACE_INC_DIR}/psa_audit_api.h
							${INTERFACE_INC_DIR}/psa_audit_defs.h
				DESTINATION ${EXPORT_INC_DIR})
		install(FILES       ${INTERFACE_SRC_DIR}/tfm_audit_func_api.c
				DESTINATION ${EXPORT_SRC_DIR})
	endif()

	if(TFM_PARTITION_PLATFORM)
		install(FILES       ${INTERFACE_INC_DIR}/tfm_platform_api.h
				DESTINATION ${EXPORT_INC_DIR})
		if(TFM_PSA_API)
			install(FILES       ${INTERFACE_SRC_DIR}/tfm_platform_ipc_api.c
					DESTINATION ${EXPORT_SRC_DIR})
		else()
			install(FILES       ${INTERFACE_SRC_DIR}/tfm_platform_func_api.c
					DESTINATION ${EXPORT_SRC_DIR})
		endif()
	endif()

	if (NOT DEFINED TFM_MULTI_CORE_TOPOLOGY OR NOT TFM_MULTI_CORE_TOPOLOGY)
		install(FILES ${S_VENEER_FILE} DESTINATION export/tfm/veneers)
	endif()

	#Collect executables to common location: <build_dir>/install/outputs/
	if (DEFINED SECURE_AXF_DIR_PREFIX)
		set(MY_BINARY_DIR ${SECURE_AXF_DIR_PREFIX})
	else()
		set(MY_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
	endif()

	install(FILES ${MY_BINARY_DIR}/${EXE_NAME}.axf
				  ${MY_BINARY_DIR}/${EXE_NAME}.bin
				  ${MY_BINARY_DIR}/${EXE_NAME}.hex
				  ${MY_BINARY_DIR}/${EXE_NAME}.elf
			DESTINATION outputs/${TARGET_PLATFORM}/)

	install(FILES ${MY_BINARY_DIR}/${EXE_NAME}.axf
				  ${MY_BINARY_DIR}/${EXE_NAME}.bin
				  ${MY_BINARY_DIR}/${EXE_NAME}.hex
				  ${MY_BINARY_DIR}/${EXE_NAME}.elf
			DESTINATION outputs/fvp/)
endfunction()

#Adds the test directory
add_subdirectory(${TFM_ROOT_DIR}/test ${CMAKE_BINARY_DIR}/test/secure_test)

#Add the crypto library target
if (TFM_PARTITION_CRYPTO)
	add_subdirectory(${SECURE_FW_DIR}/partitions/crypto)
endif()

#Add the protected storage library target
if (TFM_PARTITION_PROTECTED_STORAGE)
	add_subdirectory(${SECURE_FW_DIR}/partitions/protected_storage)
endif()

#Add the internal trusted storage library target
if (TFM_PARTITION_INTERNAL_TRUSTED_STORAGE)
	add_subdirectory(${SECURE_FW_DIR}/partitions/internal_trusted_storage)
endif()

#Add the platform service library target
if (TFM_PARTITION_PLATFORM)
	add_subdirectory(${SECURE_FW_DIR}/partitions/platform)
endif()

#Add the initial attestation service library target
if (TFM_PARTITION_INITIAL_ATTESTATION)
	add_subdirectory(${SECURE_FW_DIR}/partitions/initial_attestation)
endif()

#Add the audit logging library target
if (TFM_PARTITION_AUDIT_LOG)
	add_subdirectory(${SECURE_FW_DIR}/partitions/audit_logging)
endif()

#Add the secure runtime library target
add_subdirectory(${SECURE_FW_DIR}/partitions/lib/sprt)

if (NOT DEFINED TFM_MULTI_CORE_TOPOLOGY OR NOT TFM_MULTI_CORE_TOPOLOGY)
	#Broadcast veneer path in bundled building case
	set(S_VENEER_PATH "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "")
endif()

if (LINK_TO_BOTH_MEMORY_REGION)
	#Link to primary memory region
	set_up_secure_fw_build(S_TARGET      ${PROJECT_NAME}
						   VENEER_NAME   s_veneers)

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

if (TARGET ${PROJECT_NAME}_1)
	embedded_project_end(${PROJECT_NAME}_1)
endif()

embedded_project_end(${PROJECT_OBJ_LIB})
