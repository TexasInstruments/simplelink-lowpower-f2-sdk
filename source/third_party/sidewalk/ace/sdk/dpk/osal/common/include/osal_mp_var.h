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
#ifndef OSAL_MP_VAR_H
#define OSAL_MP_VAR_H

#ifdef __cplusplus
extern "C" {
#endif

aceMp_handle_t aceMP_initWithAllocatorAlignmentVar(struct aceMp_allocator* allocator,
                                                          size_t size, size_t align,
                                                          uint32_t flags);
ace_status_t aceMP_destroyForceVar(aceMp_handle_t mp, bool force);
void* aceMP_allocVar(aceMp_handle_t mp, size_t size);
void aceMP_freeVar(aceMp_handle_t mp, void* p);
bool aceMP_isVar(aceMp_handle_t mp);

#ifdef __cplusplus
}
#endif

#endif /* OSAL_MP_VAR_H */
