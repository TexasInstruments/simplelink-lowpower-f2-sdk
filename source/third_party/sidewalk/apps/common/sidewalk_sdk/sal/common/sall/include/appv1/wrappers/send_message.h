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

#ifndef INCLUDE_APPV1_WRAPPERS_SEND_MESSAGE_H_
#define INCLUDE_APPV1_WRAPPERS_SEND_MESSAGE_H_

#include <stdint.h>
#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/appv1/cmd.h>
#include <appv1/wrappers/generic.h>
#include <appv1/wrappers/types.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline sid_error_t default_serialize_wrapper(uint8_t* buffer, const size_t len, const void* params, hall_size_t* out_size) {
    return SID_ERROR_NONE;
}

#define HALL_SERIALIZE_WRAPPER_NAME(function_) CONCAT(hall_wrapper_, function_)
#define HALO_HALL_GENERIC_SERIALIZE_HELPER_NAME(class_name_) CONCAT3(halo_hall_appv1_, class_name_, _generic_serialize_helper)
#define HALL_SEND_WRAPPER_NAME(class_name_, class_type_member_) CONCAT4(halo_hall_appv1_, class_name_, _send_msg_, class_type_member_)

#define HALL_SEND_MSG_WRAPPER_DEFINE_0(class_name_, class_type_member_, command_id_, type_opc_)             \
    static sid_error_t HALL_SEND_WRAPPER_NAME(class_name_, class_type_member_)(                            \
        const HALL_CLASS_IFC_TYPE_NAME(class_name_) * _this, halo_hall_message_descriptor_t* mdesc) {       \
        static const halo_hall_serialize_handle_t hndl = {                                                  \
            .serialize_wrapper = (serialize_wrapper_t)default_serialize_wrapper,                            \
            .command_id = command_id_,                                                                      \
            .type = type_opc_,                                                                              \
            .size = 0,                                                                                      \
        };                                                                                                  \
        return halo_hall_appv1_generic_serialize_helper(_this, mdesc, NULL, &hndl);             \
    }

#define HALL_SEND_MSG_WRAPPER_DEFINE_1(class_name_, class_type_member_, command_id_, type_opc_, pld__)                                          \
                                                                                                                                                \
    static sid_error_t HALL_SEND_WRAPPER_NAME(class_name_, class_type_member_)(                                                                \
        const HALL_CLASS_IFC_TYPE_NAME(class_name_) * _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size) {  \
        halo_hall_serialize_handle_t hndl = {                                                                                                   \
            .serialize_wrapper = (serialize_wrapper_t)halo_hall_appv1_pld_buffer_serialize,                                                     \
            .command_id = command_id_,                                                                                                          \
            .type = type_opc_,                                                                                                                  \
            .size = size,                                                                                                                       \
        };                                                                                                                                      \
        return halo_hall_appv1_generic_serialize_helper(_this, mdesc, data, &hndl);                                                             \
    }

#define HALL_SEND_MSG_WRAPPER_DEFINE_3(                                                                                                        \
    class_name_, class_type_member_, command_id_, type_opc_, size_, serialize_function_name_, serialize_member_type_)                          \
                                                                                                                                               \
    static sid_error_t HALL_SEND_WRAPPER_NAME(class_name_, class_type_member_)(                                                               \
        const HALL_CLASS_IFC_TYPE_NAME(class_name_) * _this, halo_hall_message_descriptor_t* mdesc, const serialize_member_type_ data) {       \
        if (__builtin_constant_p(size_)) {                                                                                                     \
            static const halo_hall_serialize_handle_t hndl = {                                                                                 \
                .serialize_wrapper = (serialize_wrapper_t)serialize_function_name_,                                                            \
                .command_id = command_id_,                                                                                                     \
                .type = type_opc_,                                                                                                             \
                .size = (__builtin_constant_p(size_))? size_ : 0,                                                                              \
            };                                                                                                                                 \
            return halo_hall_appv1_generic_serialize_helper(_this, mdesc, (void*)((uintptr_t)data), &hndl);                                    \
        }                                                                                                                                      \
        else {                                                                                                                                 \
            halo_hall_serialize_handle_t hndl = {                                                                                              \
                .serialize_wrapper = (serialize_wrapper_t)serialize_function_name_,                                                            \
                .command_id = command_id_,                                                                                                     \
                .type = type_opc_,                                                                                                             \
                .size = size_,                                                                                                                 \
            };                                                                                                                                 \
            return halo_hall_appv1_generic_serialize_helper(_this, mdesc, (void*)((uintptr_t)data), &hndl);                        \
        }                                                                                                                                      \
    }

#define HALL_SEND_MSG_WRAPPER_DEFINE_4(                                                                                                  \
    class_name_, class_type_member_, command_id_, type_opc_, size_, serialize_function_name_, serialize_member_type_, input_data_type_)  \
                                                                                                                                         \
    static sid_error_t HALL_SEND_WRAPPER_NAME(class_name_, class_type_member_)(                                                         \
        const HALL_CLASS_IFC_TYPE_NAME(class_name_) * _this, halo_hall_message_descriptor_t* mdesc, const input_data_type_ data) {       \
        static const halo_hall_serialize_handle_t hndl =                                                                                 \
        {                                                                                                                                \
            .serialize_wrapper = (serialize_wrapper_t)serialize_function_name_,                                                          \
            .command_id = command_id_,                                                                                                   \
            .type = type_opc_,                                                                                                           \
            .size = size_,                                                                                                               \
        };                                                                                                                               \
        return halo_hall_appv1_generic_serialize_helper(_this, mdesc, (void*)((uintptr_t)data), &hndl);                      \
    }

#define HALL_SEND_MSG_WRAPPER_DEFINE(class_name_, class_type_member_, command_id_, type_opc_, ...) \
    CONCAT(HALL_SEND_MSG_WRAPPER_DEFINE_, VA_COUNT_ARGS(class_name_, ##__VA_ARGS__))               \
    (class_name_, class_type_member_, command_id_, type_opc_, ##__VA_ARGS__)


#define HALL_UNUSED_SEND_MSG_WRAPPER_DEFINE_0(class_name_, class_type_member_, command_id_, type_opc_)      \
    static sid_error_t HALL_SEND_WRAPPER_NAME(class_name_, class_type_member_)(                            \
        const HALL_CLASS_IFC_TYPE_NAME(class_name_) * _this, halo_hall_message_descriptor_t* mdesc) {       \
        (void)_this;                                                                                        \
        (void)mdesc;                                                                                        \
        return SID_ERROR_NOSUPPORT;                                                                        \
    }

#define HALL_UNUSED_SEND_MSG_WRAPPER_DEFINE_1(class_name_, class_type_member_, command_id_, type_opc_, pld__)                                   \
                                                                                                                                                \
    static sid_error_t HALL_SEND_WRAPPER_NAME(class_name_, class_type_member_)(                                                                \
        const HALL_CLASS_IFC_TYPE_NAME(class_name_) * _this, halo_hall_message_descriptor_t* mdesc, const uint8_t* data, const uint8_t size) {  \
        (void)data;                                                                                                                             \
        (void)size;                                                                                                                             \
        (void)_this;                                                                                                                            \
        (void)mdesc;                                                                                                                            \
        return SID_ERROR_NOSUPPORT;                                                                                                            \
    }

#define HALL_UNUSED_SEND_MSG_WRAPPER_DEFINE_3(                                                                                                 \
    class_name_, class_type_member_, command_id_, type_opc_, size_, serialize_function_name_, serialize_member_type_)                          \
                                                                                                                                               \
    static sid_error_t HALL_SEND_WRAPPER_NAME(class_name_, class_type_member_)(                                                               \
        const HALL_CLASS_IFC_TYPE_NAME(class_name_) * _this, halo_hall_message_descriptor_t* mdesc, const serialize_member_type_ data) {       \
        (void)data;                                                                                                                            \
        (void)_this;                                                                                                                           \
        (void)mdesc;                                                                                                                           \
        return SID_ERROR_NOSUPPORT;                                                                                                           \
    }

#define HALL_UNUSED_SEND_MSG_WRAPPER_DEFINE_4(                                                                                           \
    class_name_, class_type_member_, command_id_, type_opc_, size_, serialize_function_name_, serialize_member_type_, input_data_type_)  \
                                                                                                                                         \
    static sid_error_t HALL_SEND_WRAPPER_NAME(class_name_, class_type_member_)(                                                         \
        const HALL_CLASS_IFC_TYPE_NAME(class_name_) * _this, halo_hall_message_descriptor_t* mdesc, const input_data_type_ data) {       \
        (void)data;                                                                                                                      \
        (void)_this;                                                                                                                     \
        (void)mdesc;                                                                                                                     \
        return SID_ERROR_NOSUPPORT;                                                                                                     \
    }

#define HALL_UNUSED_SEND_MSG_WRAPPER_DEFINE(class_name_, class_type_member_, command_id_, type_opc_, ...) \
    CONCAT(HALL_UNUSED_SEND_MSG_WRAPPER_DEFINE_, VA_COUNT_ARGS(class_name_, ##__VA_ARGS__))               \
    (class_name_, class_type_member_, command_id_, type_opc_, ##__VA_ARGS__)


#if SALL_ENABLE_GW_ONLY_CMDS
#   define HALL_SEND_MSG_WRAPPER_DEFINE_GW_ONLY(class_name_, class_type_member_, command_id_, type_opc_, ...) HALL_SEND_MSG_WRAPPER_DEFINE(class_name_, class_type_member_, command_id_, type_opc_, ##__VA_ARGS__)
#   define HALL_SEND_MSG_WRAPPER_DEFINE_ENDPOINT_ONLY(class_name_, class_type_member_, command_id_, type_opc_, ...) HALL_UNUSED_SEND_MSG_WRAPPER_DEFINE(class_name_, class_type_member_, command_id_, type_opc_, ##__VA_ARGS__)
#elif SALL_ENABLE_ENDPOINT_ONLY_CMDS
#   define HALL_SEND_MSG_WRAPPER_DEFINE_GW_ONLY(class_name_, class_type_member_, command_id_, type_opc_, ...) HALL_UNUSED_SEND_MSG_WRAPPER_DEFINE(class_name_, class_type_member_, command_id_, type_opc_, ##__VA_ARGS__)
#   define HALL_SEND_MSG_WRAPPER_DEFINE_ENDPOINT_ONLY(class_name_, class_type_member_, command_id_, type_opc_, ...) HALL_SEND_MSG_WRAPPER_DEFINE(class_name_, class_type_member_, command_id_, type_opc_, ##__VA_ARGS__)
#elif SALL_ENABLE_ALL_CMDS
#   define HALL_SEND_MSG_WRAPPER_DEFINE_GW_ONLY(class_name_, class_type_member_, command_id_, type_opc_, ...) HALL_SEND_MSG_WRAPPER_DEFINE(class_name_, class_type_member_, command_id_, type_opc_, ##__VA_ARGS__)
#   define HALL_SEND_MSG_WRAPPER_DEFINE_ENDPOINT_ONLY(class_name_, class_type_member_, command_id_, type_opc_, ...) HALL_SEND_MSG_WRAPPER_DEFINE(class_name_, class_type_member_, command_id_, type_opc_, ##__VA_ARGS__)
#else
#   error "Please define one profile: { SALL_ENABLE_GW_ONLY_CMDS, SALL_ENABLE_ENDPOINT_ONLY_CMDS, SALL_ENABLE_ALL_CMDS }"
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INCLUDE_APPV1_WRAPPERS_SEND_MESSAGE_H_ */
