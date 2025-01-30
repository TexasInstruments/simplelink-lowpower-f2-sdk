/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "rss_provisioning_bundle.h"

const struct cm_provisioning_data data = {
    /* BL1_2_IMAGE_HASH */
    {},
    /* BL1_2_IMAGE */
    {},
    /* DMA ICS */
    {},
    /* RSS_ID */
    0,
    /* SAM config placeholder */
    {0},
    /* GUK */
    {
        0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45,
        0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01,
        0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67,
        0x89, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23,
    },
    /* CCA system properties placeholder */
    0xDEADBEEF,
};
