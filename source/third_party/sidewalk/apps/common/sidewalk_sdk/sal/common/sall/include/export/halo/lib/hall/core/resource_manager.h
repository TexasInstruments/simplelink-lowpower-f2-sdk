/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef HALL_HALS_RESOURCE_MANAGER_H_
#define HALL_HALS_RESOURCE_MANAGER_H_

#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/appv1/cmd.h>

#include <sid_memory_pool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct halo_hall_resource_manager_ifc_s* halo_hall_resource_manager_ifc;

typedef sid_error_t (*halo_hall_rm_no_memory_cb_t)(void * ctx, const hall_size_t size, const uint8_t priority);

struct halo_hall_resource_manager_ifc_s {
    /**
     * @brief Destroy Resource Manager instance and releases all allocated resources
     *
     * @param[in]  ifc  pointer to resource manager interface
     */
    void         (*destroy)(const halo_hall_resource_manager_ifc* _this);

    /**
     * @brief Try to allocate memory for message with provided message priority
     *
     * See get_with_offset_and_priority() for details.
     *
     * @param[in]  ifc       pointer to resource manager interface (see halo_hall_resource_manager_ifc)
     * @param[out] cmd       pointer to to pointer to allocated message
     * @param[in]  size      size of message data
     * @param[in]  priority  message priority, used for message preemption,
     *                           in case we ran out of memory, the low priority message can be dropped and
     *                           memory will be reallocated for higher priority message
     *
     * @return result of execution
     * @retval SID_ERROR_NONE         in case of success
     * @retval SID_ERROR_NULL_POINTER in case pointer to resource manager interface or message is NULL
     *
     * @note allocate message with default offset, default offset is provided to halo_hall_resource_manager_create(),
     *       see halo_resource_manager_config
     */
    sid_error_t (*get_with_priority           )(const halo_hall_resource_manager_ifc* _this, halo_all_cmd_t** msg,
                                                 const hall_size_t size, const uint8_t priority);

    /**
     * @brief Try to allocate memory for message with provided message offset and priority
     *
     * @param[in]  ifc       pointer to resource manager interface (see halo_hall_resource_manager_ifc)
     * @param[out] cmd       pointer to pointer to allocated message
     * @param[in]  size      size of message data
     * @param[in]  offset    message data offset, size of reserved memory before message data
     * @param[in]  priority  message priority, used for message preemption,
     *                           in case we ran out of memory, the low priority message can be dropped and
     *                           memory will be reallocated for higher priority message
     *
     * @return result of execution
     * @retval SID_ERROR_NONE         in case of success
     * @retval SID_ERROR_OOM          in case there is no free memory available
     * @retval SID_ERROR_NULL_POINTER in case pointer to resource manager interface or message is NULL
     *
     * @note The offset is some reserved memory area before message data, it allows to append (cut) data (headers)
     *       before message without allocating another buffer and copying all new data to new buffer
     *
     * @note Basically resource manager has nothing to do with message priority by itself,
     *       it does not do any message preemption and reallocation, but calls "no_memory_callback" that is
     *       supposed to do this job. no_memory_callback is provided to halo_hall_resource_manager_create(),
     *       see halo_resource_manager_create_param_t
     */
    sid_error_t (*get_with_offset_and_priority)(const halo_hall_resource_manager_ifc* _this,
                                                 halo_all_cmd_t** msg, const hall_size_t size,
                                                 const hall_size_t offset, const uint8_t priority);

    /**
     * @brief Update command offset
     *
     * @param[in]  ifc           pointer to resource manager interface (see halo_hall_resource_manager_ifc)
     * @param[in]  cmd           pointer to message that should be updated
     * @param[out] offset_delta  delta of message data offset
     *
     * @return result of execution
     * @retval SID_ERROR_NONE          in case of success
     * @retval SID_ERROR_INVALID_ARGS  in case of null pointer is provided as one of parameters
     *                                      or provided configuration is invalid
     * @retval SID_ERROR_NULL_POINTER in case pointer to resource menager interface or message is NULL
     *
     * @note The offset is some reserved memory area before message data, it allows to append (cut) data (headers)
     *       before message without allocating another buffer and copying all new data to new buffer
     */
    sid_error_t (*update_offset)(const halo_hall_resource_manager_ifc*, halo_all_cmd_t*, const int offset_delta);

    /**
     * @brief Free message
     *
     * @param[in]  ifc  pointer to resource manager interface (see halo_hall_resource_manager_ifc)
     * @param[in]  cmd  pointer to message to free
     *
     * @return result of execution
     * @retval SID_ERROR_NONE          in case of success
     * @retval SID_ERROR_INVALID_ARGS  in case of null pointer is provided as one of parameters
     *                                      or provided configuration is invalid
     */
    sid_error_t (*free)(const halo_hall_resource_manager_ifc* _this, halo_all_cmd_t* msg);
};

typedef struct {
    uint16_t                  offset;
} halo_resource_manager_config;

typedef struct {
    const struct sid_memory_pool * mem_pool;
    halo_hall_rm_no_memory_cb_t    no_mem_cb;
    void                         * no_mem_cb_ctx;
} halo_resource_manager_create_param_t;

/**
 * @brief Align provided memory by size of pointer
 *
 * @param[in]  ptr pointer to memory that should be aligned
 *
 * @return  pointer to aligned memory
 *
 * @note This method is useful to run this code on different platform,
 *       as far as pointer size on Nordic and x64 (used for running unit tests)
 *       could be different
 */
uint8_t* align_by_ptr_size(uint8_t* ptr);

/**
 * @brief Create Resource Manager instance
 *
 * @param[in]  cfg     configuration for the resource manager to be created
 * @param[out] ifc     pointer to pointer to resource manager interface (see halo_hall_resource_manager_ifc)
 * @param[in]  param   additional parameters required to create resource manager,
 *                         unlike cfg, that can be constant and compile time defined,
 *                         param is usually runtime defined.
 *
 * @return result of execution
 * @retval SID_ERROR_NONE          in case of success
 * @retval SID_ERROR_INVALID_ARGS  in case of null pointer is provided as one of parameters
 *                                     or provided configuration is invalid
 */
sid_error_t halo_hall_resource_manager_create(const halo_resource_manager_config    * cfg,
                                               const halo_hall_resource_manager_ifc ** ifc,
                                               halo_resource_manager_create_param_t  * param);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALL_HALS_RESOURCE_MANAGER_H_ */
