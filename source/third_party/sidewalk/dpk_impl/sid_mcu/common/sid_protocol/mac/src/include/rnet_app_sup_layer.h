/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNET_APP_SUP_LAYER_H
#define RNET_APP_SUP_LAYER_H

#include <rnet_errors.h>
#include "rnet_app_layer.h"

#define RNET_APP_SUP_LAYER_VERSION	(0x01)

typedef struct {
  U8                    opc;
  U32                   addr;
} cmd_map_t;


S8 rnet_app_sup_on_receive(U8 tr_idx);
void rnet_asl_on_transmit(U8 tr_idx);

void log_asl_frame(asl_frame_t* p_asl_frame);

#endif // RNET_APP_SUP_LAYER_H
