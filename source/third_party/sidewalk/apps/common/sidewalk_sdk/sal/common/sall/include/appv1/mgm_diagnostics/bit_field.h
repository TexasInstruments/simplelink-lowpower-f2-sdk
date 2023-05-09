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

#ifndef RNETC_INCLUDE_APPV1_MGM_DIAGNOSTICS_BIT_FIELD_H_
#define RNETC_INCLUDE_APPV1_MGM_DIAGNOSTICS_BIT_FIELD_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 *   BUDDY: |3b unused| |2b data rate| |2b test role| |1b direction|
 *          |2B transmit interval (ms)| |2B test duration (s)| |5B address||     1B RF channel|
 *   V1:    |5b unused               | |2b test role| |1b direction| |2B transmit interval (ms)|
 */
typedef enum {
    HALO_HALL_APPV1_LQ_TEST_RATE_MASK = 0x3,
    HALO_HALL_APPV1_LQ_TEST_RATE_OFFSET = 3,
    HALO_HALL_APPV1_LQ_TEST_ROLE_MASK = 0x3,
    HALO_HALL_APPV1_LQ_TEST_ROLE_OFFSET = 1,
    HALO_HALL_APPV1_LQ_TEST_DIRECTION_MASK = 0x1,
    HALO_HALL_APPV1_LQ_TEST_DIRECTION_OFFSET = 0,
    HALO_HALL_APPV1_LQ_OUTPUT_POWER_MASK = 0x1F,
    HALO_HALL_APPV1_LQ_OUTPUT_POWER_OFFSET = 0,
} halo_hall_appv1_lq_test_field;

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RNETC_INCLUDE_APPV1_MGM_DIAGNOSTICS_BIT_FIELD_H_ */
