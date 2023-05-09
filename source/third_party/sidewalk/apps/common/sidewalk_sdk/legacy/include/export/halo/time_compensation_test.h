/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef TIME_COMPENSATION_TEST_H
#define TIME_COMPENSATION_TEST_H

#include <stdint.h>

/**
 * Set max time offset
 *
 * @param[in] ppm offset in PPM
 */
void halo_time_set_max_time_ppm(int16_t ppm);

/**
 * Get max time offset
 *
 * @retval offset in PPM
 */
int16_t halo_time_get_max_time_ppm(void);

#endif
