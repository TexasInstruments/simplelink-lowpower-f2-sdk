/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef ACE_CONFIG_H
#define ACE_CONFIG_H

#define RTOS_PACKAGE

#define ACE_RELEASE

#define ACE_PKG_GEN_UTC_TIMESTAMP 1603826512

#define ACE_PKG_GEN_UTC_TIMESTAMP 1603826512

#ifdef ACE_CONFIG_OVERRIDES
#include ACE_CONFIG_OVERRIDES
#endif

#define ACE_CLI
#ifdef SIDEWALK_WORKAROUND_ACS
// Disable ACS logging for Sidewalk SDK builds
#define ACE_LOG
#endif // SIDEWALK_WORKAROUND_ACS
#define ACE_LOG_MAX_LOG_LENGTH 256
#define ACE_LOG_CONFIG_MIN_LVL 0
#define ACE_KVS_DS_DEFAULT_GROUP "kvsDefault"
#define ACE_API_TEST_FILES
#define OSAL_DEFAULT_LOG_LEVEL 1
#define ACE_API_TEST_FILE_NAME "/tmp/test.txt"
#define BuildType "OSAL_FREERTOS"
#define OSAL_FREERTOS
#define PriorityModel "PRIORITY_OTHER"
#define PRIORITY_OTHER
#define ACE_ALLOC_NETWORK_BUFFER_SIZE 128
#define ACE_OSAL_THREADS_STACK_MIN 0
#define ACE_OSAL_THREADS_TLS_INDEX 0
#define ACE_API_TEST
#define ACE_KVS_DS_DEFAULT_GROUP "kvsDefault"

#endif /* ACE_CONFIG_H */
