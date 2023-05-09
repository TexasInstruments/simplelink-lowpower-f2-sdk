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

#ifndef INCLUDE_APPV1_WRAPPERS_INTERFACE_H_
#define INCLUDE_APPV1_WRAPPERS_INTERFACE_H_

#include <appv1/wrappers/generic.h>
#include <appv1/wrappers/types.h>
#include <appv1/wrappers/on_message.h>
#include <appv1/wrappers/send_message.h>

#ifdef __cplusplus
extern "C" {
#endif

// prototype
#define HALL_RECIVE_INIT_NAME(class_name_) CONCAT3(halo_hall_appv1_, class_name_, _recive_ifc_init)
#define HALL_CLASS_CREATE_NAME(class_name_) CONCAT3(halo_hall_appv1_, class_name_, _create)

// defines
#define methodof(obj_, class_, member_) (((class_*)(obj_))->member_)
#define offset_of_method(class_, member_) ( (uintptr_t)&((class_*)(NULL))->member_)

#define HALL_CLASS_IMPL_DECLARE(class_name_)                     \
    typedef struct {                                             \
        struct HALL_CLASS_STRUCT_EXT_TYPE_NAME(class_name_) ifc; \
        struct halo_hall_appv1_generic_command_ifc_s generic;    \
        const halo_hall_dispatcher_ifc* dispatcher;              \
    } HALL_CLASS_IMPL_TYPE_NAME(class_name_)

// functions wrappers

#define HALL_CLASS_IMPL_GET(class_name_, member_, impl_name_)                             \
    struct HALL_CLASS_STRUCT_EXT_TYPE_NAME(class_name_)* interface =                      \
        containerof(_this, struct HALL_CLASS_STRUCT_EXT_TYPE_NAME(class_name_), member_); \
    HALL_CLASS_IMPL_TYPE_NAME(class_name_)* impl_name_ = containerof(interface, HALL_CLASS_IMPL_TYPE_NAME(class_name_), ifc)

#define HALL_CLASS_CREATE_WRAPPER_DEFINE(class_name_, _class_version_, cmd_class__)                                                       \
    sid_error_t HALL_CLASS_CREATE_NAME(class_name_)(const HALL_CLASS_IFC_EXT_TYPE_NAME(class_name_) * *_this,                                \
        const HALL_CLASS_IFC_TYPE_NAME(class_name_) user_notify_cb, halo_hall_appv1_user_ctx_t user_ctx, const void* lib_ctx) { \
        static const halo_hall_appv1_generic_ifc_config_t generic_ifc_config = {                                                              \
                .opaque_send = &v_table,                                                                                                      \
                .receive_hndl = processing_handles,                                                                                           \
                .receive_hndl_count = countof(processing_handles),                                                                            \
                .class_version = _class_version_,                                                                                             \
                .class_id = cmd_class__                                                                                                       \
        };                                                                                                                                    \
        return halo_hall_appv1_generic_create((const halo_hall_appv1_generic_ext_ifc**)_this, user_notify_cb, user_ctx, lib_ctx,              \
            &generic_ifc_config);                                                                                                             \
}
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INCLUDE_APPV1_WRAPPERS_INTERFACE_H_ */
