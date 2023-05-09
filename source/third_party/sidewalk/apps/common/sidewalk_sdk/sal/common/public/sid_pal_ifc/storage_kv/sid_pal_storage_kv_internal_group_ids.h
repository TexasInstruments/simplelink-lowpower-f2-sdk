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


/* SDK users may define these macros themselves to override the values used here,
 * in order to control which group IDs the Sidewalk code uses. This allows users
 * to deconflict any group ID collisions between application code and Sidewalk
 * code that also uses this PAL.
 */


#if !defined(SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID)
#define SID_PAL_STORAGE_KV_INTERNAL_PROTOCOL_GROUP_ID 0x2000
#endif


#if !defined(SID_PAL_STORAGE_KV_INTERNAL_CONFIG_GROUP_ID)
#define SID_PAL_STORAGE_KV_INTERNAL_CONFIG_GROUP_ID 0x3456
#endif