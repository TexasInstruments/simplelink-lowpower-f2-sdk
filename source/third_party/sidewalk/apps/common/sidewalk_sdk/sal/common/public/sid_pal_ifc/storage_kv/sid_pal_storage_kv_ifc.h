/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_STORAGE_KV_IFC_H
#define SID_PAL_STORAGE_KV_IFC_H

/** @file
 *
 * @defgroup sid_pal_storage_kv sid persistent storage interface for key mapped values
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details     Provides persistent storage interface for key mapped values to be implemented by platform.
 *              Keys are comprised of two components: the "group" and the "key". They are passed to
 *              the functions in this interface as two parameters. Groups allow related data entries
 *              to be addressed with the same group, but different keys. Both groups and keys are
 *              unsigned integer values and together their values map to a single value called a
 *              "record". Group value 0xFFFF and key value 0 are reserved, and must not be used by
 *              consumers of this interface. The group values behind the macros in
 *              sid_pal_storage_kv_internal_group_ids.h are also reserved, but users may redefine those
 *              macros to different values as desired. The maximum data size is defined by the implementation.
 */

#include <sid_error.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the key value storage subsystem
 *
 * @retval SID_ERROR_NONE in case of success
 */
sid_error_t sid_pal_storage_kv_init(void);


/**
 * Get a value using its group and key IDs
 *
 * @param[in]   group    Group
 * @param[in]   key      Key
 * @param[out]  p_data   Pointer to output buffer to contain the value
 * @param[in]   len      Maximum length of buffer pointed to by p_data in bytes
 *
 * @retval SID_ERROR_NONE in case of success
 */
sid_error_t sid_pal_storage_kv_record_get(uint16_t group, uint16_t key, void * p_data, uint32_t len);


/**
 * Get the size of a value using its group and key IDs
 *
 * @param[in]   group   Group
 * @param[in]   key     Key
 * @param[out]  p_len   Pointer to integer to contain the size of the value in bytes
 *
 * @retval SID_ERROR_NONE in case of success
 */
sid_error_t sid_pal_storage_kv_record_get_len(uint16_t group, uint16_t key, uint32_t * p_len);


/**
 * Set a value using its group and key IDs
 *
 * @param[in]  group    Group
 * @param[in]  key      Key
 * @param[in]  p_data   Pointer to input buffer which contains the value
 * @param[in]  len      The size of the input value in bytes
 *
 * @retval SID_ERROR_NONE in case of success
 */
sid_error_t sid_pal_storage_kv_record_set(uint16_t group, uint16_t key, void const * p_data, uint32_t len);


/**
 * Delete a value using its group and key IDs
 *
 * @param[in]  group    Group
 * @param[in]  key      Key
 *
 * @retval SID_ERROR_NONE in case of success
 */
sid_error_t sid_pal_storage_kv_record_delete(uint16_t group, uint16_t key);


/**
 * Delete all values in a group
 *
 * @param[in]  group    Group
 *
 * @retval SID_ERROR_NONE in case of success
 */
sid_error_t sid_pal_storage_kv_group_delete(uint16_t group);


#ifdef __cplusplus
}
#endif

/** @} */

#endif
