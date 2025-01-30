/*
 * Copyright (c) 2024, Texas Instruments Incorporated. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __CMSE_H__
#define __CMSE_H__

#include <arm_cmse.h>

#if defined(__GNUC__)
#ifndef __STATIC_INLINE
#define __STATIC_INLINE static inline
#endif
#elif defined(__clang__) && defined(__ti__)
#define __STATIC_INLINE static __inline
#endif

__STATIC_INLINE void *cmse_has_unpriv_nonsecure_rw_access(void *p, size_t size)
{
    return cmse_check_address_range(
        p, size, (CMSE_MPU_UNPRIV | CMSE_MPU_READWRITE | CMSE_NONSECURE));
}

__STATIC_INLINE void *cmse_has_unpriv_nonsecure_read_access(void *p, size_t size)
{
    return cmse_check_address_range(
        p, size, (CMSE_MPU_UNPRIV | CMSE_MPU_READ | CMSE_NONSECURE));
}

#endif /* __CMSE_H__ */
