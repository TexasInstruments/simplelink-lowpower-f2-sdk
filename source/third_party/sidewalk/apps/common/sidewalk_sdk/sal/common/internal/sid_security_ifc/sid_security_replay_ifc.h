/*
* Copyright 2021-2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
*
* AMAZON PROPRIETARY/CONFIDENTIAL
*
* You may not use this file except in compliance with the terms and conditions
* set forth in the accompanying LICENSE.TXT file.
*
* THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
* DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
*/

#ifndef SID_SECURITY_REPLAY_IFC_H
#define SID_SECURITY_REPLAY_IFC_H

#include <sid_error.h>
#include <stdint.h>

#ifndef SID_SECURITY_REPLAY_SEQN_CACHE_MAX
#define SID_SECURITY_REPLAY_SEQN_CACHE_MAX 32
#endif

#ifdef __cplusplus
extern "C" {
#endif

void sid_security_reinit_sqn_cache(uint32_t time_ref);

sid_error_t sid_security_check_replay(uint32_t time_ref, uint32_t seqn, uint32_t auth_tag);

#ifdef __cplusplus
}
#endif

#endif
