/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.  This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef DIAG_FREQ_HOP_H
#define DIAG_FREQ_HOP_H

#ifdef __cplusplus
extern "C" {
#endif

void diag_fqhop_update_time(uint32_t current_time);
uint8_t diag_fqhop_required(uint32_t curren_time, uint32_t time_on_air);
int8_t diag_fqhop_table_generate(uint32_t freq_start, uint32_t freq_end, uint32_t separation,
                                 uint16_t dwell_time);
uint32_t diag_fqhop_get_next_hop(void);
void diag_fqhop_disable_hybrid_mode(void);
#ifdef __cplusplus
}
#endif

#endif
