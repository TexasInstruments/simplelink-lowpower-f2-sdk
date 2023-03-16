#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

option(ENABLE_PROTECTED_STORAGE_SERVICE_TESTS "Option for protected storage service tests" TRUE)
option(ENABLE_INTERNAL_TRUSTED_STORAGE_SERVICE_TESTS "Option for internal trusted storage services tests" TRUE)
option(ENABLE_AUDIT_LOGGING_SERVICE_TESTS "Option for audit logging service tests" TRUE)
option(ENABLE_CRYPTO_SERVICE_TESTS "Option for crypto service tests" TRUE)
option(ENABLE_ATTESTATION_SERVICE_TESTS "Option for attestation service tests" TRUE)
option(ENABLE_PLATFORM_SERVICE_TESTS "Option for platform service tests" TRUE)
option(ENABLE_QCBOR_TESTS "Option for QCBOR tests" TRUE)
option(ENABLE_T_COSE_TESTS "Option for T_COSE tests" TRUE)

# If a partition is not enabled, then neither should its tests.
if (NOT TFM_PARTITION_PROTECTED_STORAGE)
	set(ENABLE_PROTECTED_STORAGE_SERVICE_TESTS FALSE)
endif()

if (NOT TFM_PARTITION_INTERNAL_TRUSTED_STORAGE)
	set(ENABLE_INTERNAL_TRUSTED_STORAGE_SERVICE_TESTS FALSE)
endif()

if (NOT TFM_PARTITION_CRYPTO)
	set(ENABLE_CRYPTO_SERVICE_TESTS FALSE)
endif()

if (NOT TFM_PARTITION_INITIAL_ATTESTATION)
	set(ENABLE_ATTESTATION_SERVICE_TESTS FALSE)
	set(ENABLE_QCBOR_TESTS FALSE)
	set(ENABLE_T_COSE_TESTS FALSE)
endif()

# Disable external library test if SERVICES_TEST_ENABLED is OFF
if (NOT SERVICES_TEST_ENABLED)
	set(ENABLE_QCBOR_TESTS FALSE)
	set(ENABLE_T_COSE_TESTS FALSE)
endif()

if (NOT TFM_PARTITION_PLATFORM)
	set(ENABLE_PLATFORM_SERVICE_TESTS FALSE)
endif()

if (NOT TFM_PARTITION_AUDIT_LOG)
	set(ENABLE_AUDIT_LOGGING_SERVICE_TESTS FALSE)
endif()
