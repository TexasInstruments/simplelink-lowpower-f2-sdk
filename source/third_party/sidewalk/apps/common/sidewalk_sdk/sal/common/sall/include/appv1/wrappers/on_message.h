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

#ifndef INCLUDE_APPV1_WRAPPERS_ON_MESSAGE_H_
#define INCLUDE_APPV1_WRAPPERS_ON_MESSAGE_H_

#include <appv1/wrappers/generic.h>
#include <appv1/wrappers/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HALL_ON_MSG_DEFAULT_NAME(class_name_, function_name_) CONCAT4(halo_hall_appv1_, class_name_, _on_msg_default_, function_name_)
#define HALL_ON_MSG_DEFAULT_NAME(class_name_, function_name_) CONCAT4(halo_hall_appv1_, class_name_, _on_msg_default_, function_name_)
#define HALL_ON_MSG_HANDLER_WRAPPER_NAME(class_name_, class_method_member_) CONCAT4(halo_hall_appv1_, class_name_, _on_msg_, class_method_member_)

#if SALL_ENABLE_GW_ONLY_CMDS
#   define HALL_ON_MSG_HANDLER_WRAPPER_NAME_GW_ONLY(class_name_, class_method_member_) HALL_ON_MSG_HANDLER_WRAPPER_NAME(class_name_, class_method_member_)
#   define HALL_ON_MSG_HANDLER_WRAPPER_NAME_ENDPOINT_ONLY(class_name_, class_method_member_) on_message_invalid_op
#elif SALL_ENABLE_ENDPOINT_ONLY_CMDS
#   define HALL_ON_MSG_HANDLER_WRAPPER_NAME_GW_ONLY(class_name_, class_method_member_) on_message_invalid_op
#   define HALL_ON_MSG_HANDLER_WRAPPER_NAME_ENDPOINT_ONLY(class_name_, class_method_member_) HALL_ON_MSG_HANDLER_WRAPPER_NAME(class_name_, class_method_member_)
#elif SALL_ENABLE_ALL_CMDS
#   define HALL_ON_MSG_HANDLER_WRAPPER_NAME_GW_ONLY(class_name_, class_method_member_) HALL_ON_MSG_HANDLER_WRAPPER_NAME(class_name_, class_method_member_)
#   define HALL_ON_MSG_HANDLER_WRAPPER_NAME_ENDPOINT_ONLY(class_name_, class_method_member_) HALL_ON_MSG_HANDLER_WRAPPER_NAME(class_name_, class_method_member_)
#else
#   error "Please define one profile: { SALL_ENABLE_GW_ONLY_CMDS, SALL_ENABLE_ENDPOINT_ONLY_CMDS, SALL_ENABLE_ALL_CMDS }"
#endif

#define HALL_GENERIC_ON_MESSAGE_WRAPPER_NAME(class_name_) CONCAT3(halo_hall_appv1_, class_name_, _on_message)

#define HALL_ON_MSG_DEFAULT_WRAPPER_DEFINE(class_name_, function_name_, ...)                                          \
    static sid_error_t HALL_ON_MSG_DEFAULT_NAME(class_name_, function_name_)(                                        \
        const HALL_CLASS_IFC_TYPE_NAME(class_name_) * _this, halo_hall_message_descriptor_t * mdesc, ##__VA_ARGS__) { \
        return SID_ERROR_NOSUPPORT;                                                                                  \
    }

#define HALL_ON_MSG_HANDLER_WRAPPER_DEFINE_0(class_name_, class_method_member_, ...)                                                 \
    static sid_error_t HALL_ON_MSG_HANDLER_WRAPPER_NAME(class_name_, class_method_member_)(                                         \
        const halo_hall_appv1_generic_command_ifc* _this, const halo_all_cmd_t* msg, halo_hall_message_descriptor_t* descriptor) {   \
        const uintptr_t offset = offset_of_method(HALL_CLASS_STRUCT_TYPE_NAME(class_name_), class_method_member_);                   \
        return halo_hall_appv1_generic_handler(_this, msg, descriptor, offset);                                                    \
    }

#define HALL_ON_MSG_HANDLER_WRAPPER_DEFINE_1(class_name_, class_method_member_, pld__...)                                            \
    static sid_error_t HALL_ON_MSG_HANDLER_WRAPPER_NAME(class_name_, class_method_member_)(                                         \
        const halo_hall_appv1_generic_command_ifc* _this, const halo_all_cmd_t* msg, halo_hall_message_descriptor_t* descriptor) {   \
        const uintptr_t offset = offset_of_method(HALL_CLASS_STRUCT_TYPE_NAME(class_name_), class_method_member_);                   \
        return halo_hall_appv1_generic_handler_pld(_this, msg, descriptor, offset);                                                  \
    }

#define HALL_ON_MSG_HANDLER_WRAPPER_DEFINE_2(class_name_, class_method_member_, deserialize_function_name_, deserialize_data_type_, ...)   \
    static sid_error_t HALL_ON_MSG_HANDLER_WRAPPER_NAME(class_name_, class_method_member_)(                                               \
        const halo_hall_appv1_generic_command_ifc* _this, const halo_all_cmd_t* msg, halo_hall_message_descriptor_t* descriptor) {         \
        static const halo_hall_deserialize_handle_t handle = {                                                                             \
                           .offset = offset_of_method(HALL_CLASS_STRUCT_TYPE_NAME(class_name_), class_method_member_),                     \
                           .deserialize_wrapper = (deserialize_wrapper_t)deserialize_function_name_                                        \
        };                                                                                                                                 \
        deserialize_data_type_ result = {};                                                                                                \
        return halo_hall_appv1_generic_handler_ptr(_this, msg, descriptor, &handle, &result);                                              \
}

#define HALL_ON_MSG_HANDLER_WRAPPER_DEFINE_3(                                                                                        \
    class_name_, class_method_member_, deserialize_function_name_, deserialize_data_type_, output_data_type_...)                     \
    static sid_error_t HALL_ON_MSG_HANDLER_WRAPPER_NAME(class_name_, class_method_member_)(                                         \
        const halo_hall_appv1_generic_command_ifc* _this, const halo_all_cmd_t* msg, halo_hall_message_descriptor_t* descriptor) {   \
        HALL_CLASS_IMPL_GET(class_name_, generic, impl);                           \
        deserialize_data_type_ result = 0;                                                                                           \
        sid_error_t res = SID_ERROR_NONE;                                                                                          \
        typedef sid_error_t (*on_msg_handler_t)(                                                                                    \
            const HALL_CLASS_IFC_TYPE_NAME(class_name_) * ifc, halo_hall_message_descriptor_t * mdesc, const output_data_type_ arg); \
        on_msg_handler_t handler = methodof(impl->ifc.receive, HALL_CLASS_STRUCT_TYPE_NAME(class_name_), class_method_member_);      \
        if (!handler) {                                                                                                              \
            return halo_hall_appv1_default_handler(msg, descriptor);                                                                 \
        }                                                                                                                            \
        if (descriptor->status == SALL_APP_NO_ERROR) {                                                               \
            res = (deserialize_function_name_)(                                                                                      \
                    (const uint8_t*)msg->pld, (size_t)msg->pld_size, (deserialize_data_type_*)&result, (hall_size_t*)NULL);          \
        }                                                                                                                            \
        if (res == SID_ERROR_NONE) {                                                                                                \
            res = handler(&impl->ifc.receive, descriptor, (const output_data_type_)result);                                          \
        }                                                                                                                            \
        return res;                                                                                                                  \
    }

#define HALL_ON_MSG_HANDLER_WRAPPER_DEFINE(class_name_, class_method_member_, ...) \
    CONCAT(HALL_ON_MSG_HANDLER_WRAPPER_DEFINE_, VA_COUNT_ARGS(class_name_, ##__VA_ARGS__))(class_name_, class_method_member_, ##__VA_ARGS__)

#if SALL_ENABLE_GW_ONLY_CMDS
#   define HALL_ON_MSG_HANDLER_WRAPPER_DEFINE_GW_ONLY(class_name_, class_method_member_, ...) HALL_ON_MSG_HANDLER_WRAPPER_DEFINE(class_name_, class_method_member_, ##__VA_ARGS__)
#   define HALL_ON_MSG_HANDLER_WRAPPER_DEFINE_ENDPOINT_ONLY(class_name_, class_method_member_, ...)
#elif SALL_ENABLE_ENDPOINT_ONLY_CMDS
#   define HALL_ON_MSG_HANDLER_WRAPPER_DEFINE_GW_ONLY(class_name_, class_method_member_, ...)
#   define HALL_ON_MSG_HANDLER_WRAPPER_DEFINE_ENDPOINT_ONLY(class_name_, class_method_member_, ...) HALL_ON_MSG_HANDLER_WRAPPER_DEFINE(class_name_, class_method_member_, ##__VA_ARGS__)
#elif SALL_ENABLE_ALL_CMDS
#   define HALL_ON_MSG_HANDLER_WRAPPER_DEFINE_GW_ONLY(class_name_, class_method_member_, ...) HALL_ON_MSG_HANDLER_WRAPPER_DEFINE(class_name_, class_method_member_, ##__VA_ARGS__)
#   define HALL_ON_MSG_HANDLER_WRAPPER_DEFINE_ENDPOINT_ONLY(class_name_, class_method_member_, ...) HALL_ON_MSG_HANDLER_WRAPPER_DEFINE(class_name_, class_method_member_, ##__VA_ARGS__)
#else
#   error "Please define one profile: { SALL_ENABLE_GW_ONLY_CMDS, SALL_ENABLE_ENDPOINT_ONLY_CMDS, SALL_ENABLE_ALL_CMDS }"
#endif

#define HALL_GENERIC_ON_MESSAGE_WRAPPER_DEFINE(class_name_, cmd_class__)                                                                 \
    static sid_error_t HALL_GENERIC_ON_MESSAGE_WRAPPER_NAME(class_name_)(const halo_hall_appv1_generic_command_ifc* cmd_this, halo_all_cmd_t* msg) { \
        HALL_ASSERT(msg->cmd.cls == cmd_class__);                                                                                             \
        return generic_on_message(cmd_this, msg, processing_handles, countof(processing_handles));                                       \
    }

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INCLUDE_APPV1_WRAPPERS_ON_MESSAGE_H_ */
