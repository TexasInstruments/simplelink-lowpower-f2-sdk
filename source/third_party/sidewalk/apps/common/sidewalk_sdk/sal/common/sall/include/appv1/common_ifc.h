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

#ifndef INCLUDE_APPV1_COMMON_IFC_H_
#define INCLUDE_APPV1_COMMON_IFC_H_

#include <halo/lib/hall/appv1/message.h>
#include <halo/lib/hall/appv1/generic_command_ifc.h>
#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/appv1/cmd.h>
#include <appv1/appv1.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef sid_error_t (*on_message_handle)(const halo_hall_appv1_generic_command_ifc* generic_ifc,
                                          const halo_all_cmd_t* msg,
                                          halo_hall_message_descriptor_t* descriptor);

typedef struct {
    uint16_t cmd_id;
    on_message_handle handles[_RNET_LL_OPC_COUNT];

} halo_hall_cmd_processing_t;

const halo_hall_appv1_class_version_t default_class_version();
sid_error_t on_message_invalid_op(const halo_hall_appv1_generic_command_ifc* generic_ifc,
                                   const halo_all_cmd_t* msg,
                                   halo_hall_message_descriptor_t* descriptor);
sid_error_t generic_on_message(const halo_hall_appv1_generic_command_ifc* generic_ifc,
                                halo_all_cmd_t* msg,
                                const halo_hall_cmd_processing_t* hndl, size_t count);
void receive_ifc_cpy(void** src, void** dst, const size_t count);
sid_error_t halo_hall_appv1_default_handler(const halo_all_cmd_t* msg, halo_hall_message_descriptor_t* descriptor);
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INCLUDE_APPV1_COMMON_IFC_H_ */
