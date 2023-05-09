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

#ifndef INCLUDE_APPV1__IFC_H_
#define INCLUDE_APPV1__IFC_H_

#include <halo/lib/hall/appv1/message.h>
#include <halo/lib/hall/appv1/generic_command_ifc.h>
#include <halo/lib/hall/interface.h>
#include <appv1/common_ifc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct halo_hall_appv1_generic_ext_ifc_s halo_hall_appv1_generic_ext_ifc;

struct halo_hall_appv1_generic_ext_ifc_s {
    const void * opaque_send;
    const void * opaque_receive;
    halo_hall_appv1_generic_command_ifc generic;
};

typedef struct {
    const void * opaque_send;
    const halo_hall_cmd_processing_t * receive_hndl;
    halo_hall_appv1_class_version_t class_version;
    size_t receive_hndl_count;
    uint16_t class_id;
} halo_hall_appv1_generic_ifc_config_t;

typedef sid_error_t (* serialize_wrapper_t)(uint8_t* buffer, const size_t len, const void* params, hall_size_t* out_size);

typedef struct {
    serialize_wrapper_t serialize_wrapper;
    uint16_t     command_id;
    uint16_t     size;
    uint8_t      type;
} halo_hall_serialize_handle_t;

typedef sid_error_t (* deserialize_wrapper_t)(const uint8_t* buffer, const size_t len, void* pld, hall_size_t* out_size);

typedef struct {
    deserialize_wrapper_t deserialize_wrapper;
    uintptr_t offset;
} halo_hall_deserialize_handle_t;

sid_error_t halo_hall_appv1_generic_serialize_helper(const void * opaque_send,
                        halo_hall_message_descriptor_t* mdesc,
                        const void* serialize_wrapper_params,
                        const halo_hall_serialize_handle_t* class_def);
sid_error_t halo_hall_appv1_generic_handler(const halo_hall_appv1_generic_command_ifc* cmd_this,
                        const halo_all_cmd_t* msg,
                        halo_hall_message_descriptor_t* descriptor,
                        uintptr_t offset);
sid_error_t halo_hall_appv1_generic_handler_pld(const halo_hall_appv1_generic_command_ifc* cmd_this,
                        const halo_all_cmd_t* msg,
                        halo_hall_message_descriptor_t* descriptor,
                        uintptr_t offset);
sid_error_t halo_hall_appv1_generic_handler_ptr(const halo_hall_appv1_generic_command_ifc* cmd_this,
                        const halo_all_cmd_t* msg,
                        halo_hall_message_descriptor_t* descriptor,
                        const halo_hall_deserialize_handle_t * handle,
                        void * data);
sid_error_t halo_hall_appv1_generic_handler_uint8(const halo_hall_appv1_generic_command_ifc* cmd_this,
                        const halo_all_cmd_t* msg,
                        halo_hall_message_descriptor_t* descriptor,
                        uintptr_t offset);
sid_error_t halo_hall_appv1_generic_handler_uint16(const halo_hall_appv1_generic_command_ifc* cmd_this,
                        const halo_all_cmd_t* msg,
                        halo_hall_message_descriptor_t* descriptor,
                        uintptr_t offset);
sid_error_t halo_hall_appv1_generic_handler_uint32(const halo_hall_appv1_generic_command_ifc* cmd_this,
                        const halo_all_cmd_t* msg,
                        halo_hall_message_descriptor_t* descriptor,
                        uintptr_t offset);
sid_error_t halo_hall_appv1_generic_create(const halo_hall_appv1_generic_ext_ifc** _this,
                        const void * opaque_receive, halo_hall_appv1_user_ctx_t user_ctx, const void* lib_ctx,
                        const halo_hall_appv1_generic_ifc_config_t * config);
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INCLUDE_APPV1__IFC_H_ */
