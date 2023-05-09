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

#ifndef HALO_LIB_HALO_EXPORT_HALO_SYNC_H
#define HALO_LIB_HALO_EXPORT_HALO_SYNC_H

// FIXME: This should be platform-/arch- defined

#define HALO_SYNC_LOCKFREE_SECTION_CLREX() \
        __CLREX()

#define HALO_SYNC_LOCKFREE_SECTION_START    \
        do { \
            uint8_t dummy_lockfree_guard_ = 0; \
            do { \
                HALO_SYNC_LOCKFREE_SECTION_CLREX(); \
                __LDREXB(&dummy_lockfree_guard_); \

#define HALO_SYNC_LOCKFREE_SECTION_END      \
            } while (__STREXB(0, &dummy_lockfree_guard_)); \
        } while (0);

#define HALO_SYNC_LOCKFREE_SECTION_EXEC(func_, ...) \
        HALO_SYNC_LOCKFREE_SECTION_START \
        func(__VA_ARGS__); \
        HALO_SYNC_LOCKFREE_SECTION_END;

#endif /* !HALO_LIB_HALO_EXPORT_HALO_SYNC_H */
