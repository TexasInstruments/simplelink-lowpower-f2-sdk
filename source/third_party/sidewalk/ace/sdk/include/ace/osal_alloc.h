/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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
/**
 * @file osal_alloc.h
 * @brief ACE OSAL memory allocation APIs
 *
 * The aceAlloc_* APIs implement a common memory allocation abstraction.
 *
 * Calling aceAlloc_init() is optional, and will result in malloc being used for
 * the allocator (the default behavior). Standard malloc() and free() to init
 * and deinit all the supported memory buffer type. Calling
 * aceAlloc_initWithAllocator(), for the specified memory buffer type in the
 * allocator, it will use alloc and free in the allocator to init and deinit the
 * supported memory buffer type. In this case, customized memory could be used
 * to initialize a specific memory buffer type.
 *
 * Note that the size of each memory type can be configured using the
 * appropriate ACE_ALLOC_ options. So, if an allocator is provided for a
 * specific memory buffer type, it should be able to allocate the configured
 * size of memory for the memory buffer type.
 *
 * @addtogroup ACE_OSAL_ALLOC
 * @{
 */
#ifndef OSAL_ALLOC_H
#define OSAL_ALLOC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ace/ace_modules.h>
#include <ace/ace_status.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type representing memory buffer type
 *
 * For example,
 * ACE_ALLOC_BUFFER_GENERIC means there is no specified buffer type of the
 * buffer allocation.
 * ACE_ALLOC_BUFFER_NETWORK means the buffer allocation is network request.
 *
 * When performing a network request, the ACE_ALLOC_BUFFER_NETWORK type should
 * be used to allocate any associated memory (e.g. request/response buffers).
 * When using an asynchronous network client, such as ACM, you should do all of
 * your ACE_ALLOC_BUFFER_NETWORK allocations from within the async callbacks,
 * wherever possible. Make sure not to hold onto the network buffers after your
 * network request has finished (even if there was an error).
 *
 * ACE_ALLOC_BUFFER_TEST means the buffer allocation is for use within ACS
 * tests.
 */
typedef enum {
    ACE_ALLOC_BUFFER_GENERIC = 0,
    ACE_ALLOC_BUFFER_NETWORK,
    ACE_ALLOC_BUFFER_TEST,
    ACE_ALLOC_BUFFER_MAX,
} aceAlloc_bufferType_t;

/**
 * @brief Type representing allocator used to initialize memory buffer types
 *
 * It must specify the buffer type and memory allocation functions.
 */
typedef struct aceAlloc_allocator {
    aceAlloc_bufferType_t buf_type;
    void* (*alloc)(size_t size, void* ctx);
    void (*free)(void* p, void* ctx);
    void* ctx;
} aceAlloc_allocator_t;

/**
 *  @brief Initialize ace alloc
 *
 *  Initialize ace alloc related data structures and memory types with default
 *  allocator. It would use the default allocator to initialize each supported
 *  buffer type.
 *
 *  @return ACE_STATUS_OK on success, and a negative value from
 *          @ref ace_status_t otherwise
 */
ace_status_t aceAlloc_init(void);

/**
 *  @brief Initialize ace alloc with allocators
 *
 *  Initialize ace alloc related data structures and memory types with specified
 *  allocators. The types initialization and handling would be based on the
 *  passed-in allocators. If no passed-in allocators, it would use the default
 *  allocator to initialize each supported buffer type.
 *
 *  @param[in] allocators Pointer to the allocators array
 *  @param[in] count Number of allocators passed in
 *  @return ACE_STATUS_OK on success
 *          ACE_STATUS_NOT_SUPPORTED if API not supported,
 *          and a negative value from @ref ace_status_t otherwise.
 */
ace_status_t aceAlloc_initWithAllocator(aceAlloc_allocator_t* allocators,
                                        size_t count);

/**
 *  @brief De-initialize ace alloc
 *
 *  Release ace alloc related data structures and free memory
 *
 *  @return ACE_STATUS_OK on success, and a negative value from
 *          @ref ace_status_t otherwise
 */
ace_status_t aceAlloc_deInit(void);

/**
 *  @brief Allocate a buffer
 *
 *  Allocate a buffer with a specified module id, buffer type and size
 *
 *  @param[in] module_id Module id
 *  @param[in] buf_type Memory buffer type
 *  @param[in] size Buffer size to be allocated
 *  @return the pointer to the allocated buffer on success, else NULL
 */
void* aceAlloc_alloc(aceModules_moduleId_t module_id,
                     aceAlloc_bufferType_t buf_type, size_t size);

/**
 *  @brief Allocate a buffer for an array
 *
 *  Allocate a buffer for an array of nmemb elements of size bytes each
 *  with a specified module id, buffer type and size.
 *  The buffer is set to zero.
 *
 *  @param[in] module_id The ACE module id
 *  @param[in] buf_type The memory buffer type
 *  @param[in] nmemb The number of array elements
 *  @param[in] size The size of each array element
 *  @return the pointer to the allocated buffer on success, else NULL
 */
void* aceAlloc_calloc(aceModules_moduleId_t module_id,
                      aceAlloc_bufferType_t buf_type, size_t nmemb,
                      size_t size);

/**
 *  @brief Free a buffer back
 *
 *  This function free a buffer back with a specified module id and buffer type
 *
 *  @param[in] module_id The ACE module id
 *  @param[in] buf_type The memory buffer type
 *  @param[in] p Pointer to the buffer to be freed
 */
void aceAlloc_free(aceModules_moduleId_t module_id,
                   aceAlloc_bufferType_t buf_type, void* p);

#ifdef __cplusplus
}
#endif

#endif /* OSAL_ALLOC_H */
/** @} */
