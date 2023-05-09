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

#ifndef SID_ASL_PARSER_H
#define SID_ASL_PARSER_H

#include <sid_asl_types.h>
#include <sid_error.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Desrializes the buffer into Presentation/Application support layer format
 *
 * Assumes the passed buffer is already decrypted with the network server key
 * Deserializes the Presentation/Application support layer frame first.
 * If the deserialization of presentation layer frame indicates the application
 * command frame is encrypted with application server key, SID_ERROR_SUCCESS is
 * returned and the offset at which the application command frame starts is updated
 * in the out_buffer.
 * The out_buffer's pl_frame_deserialized is set to true and app_cmd_frame_offset is
 * updated with the application layer command frame's start offset
 *
 * The user is expected to check the presentation layer frame structure's app_enc_ena
 * field to determine whether the application command frame is deserialized or not
 *
 * If app_enc_ena is set to false, the deserialized out buffer contains both application
 * comand frame and presentation layer frame deserialized
 *
 * If the app_enc_ena is set to true, the deserialized out buffer contains only the
 * deserialized presentation layer frame but not the application layer command frame.
 *
 * If app_enc_ena is set to true, the application command frame is encrypted
 * and the user has to decrypt the buffer of length pointed by app_cmd_frame_length
 * starting at app_cmd_frame_offset with the application server key
 *
 * The user passes the same serialized buffer but with decrypted with application
 * server key starting at app_cmd_frame_offset
 *
 * The application command frame is then deserialized and the out buffer has both the
 * presentation and application command frames deserialized.
 *
 * @param[in]  buffer      The serialized buffer containing application and presentation layer
 *                         command frames.
 * @param[in]  len         The length of the buffer passed.
 * @param[out] out_buffer  Pointer to the deserialzied application and presentation layer command
 *                         frames.
 *
 * @returns #SID_ERROR_NONE on sucess.
 *          #SID_ERROR_INVALID_ARGS on invalid arguments being passed.
 *          #SID_ERROR_OOM on the output buffer cannot hold the deserialized frame format
 *
 */

sid_error_t sid_asl_deserialize(const uint8_t *const buffer, size_t len,
                                 struct app_pl_frame_deserialized_buffer *const out_buffer);

/**
 * Serializes the applicaton and presentation layer frame format to a serialized buffer
 *
 * Updates the start offset of the network data and application command header
 *
 * The user is expected to encrypt the serialized buffer with
 * The network server key from the start of out buffer till total_used_size, if network server encryption is enabled.
 * The application server key starting at app_cmd_frame_start_offset for a length of
 * (total_used_size - app_cmd_frame_start_offset), if application server encryption is enabled.
 *
 * Unlike deserialization the cmd frame as well presentation layer frame are serialized by this api
 * and offsets and length updated.
 *
 * @param[in]  frame      A pointer to presentation layer frame
 * @param[in]  cmd_frame  A pointer to application cmd frame
 * @param[out] out_buffer A pointer to serialized buffer and offsets
 *
 * @returns #SID_ERROR_NONE on success.
 *          #SID_ERROR_INVALID_ARGS on invalid arguments being passed.
 *          #SID_ERROR_OOM on the output buffer cannot hold the serialized frame.
 *
 */
sid_error_t sid_asl_serialize(const struct app_pl_frame *const frame,
                               const struct app_cmd_frame *const cmd_frame,
                               struct app_pl_frame_serialized_buffer *const out_buffer);
/**
 * Computes the length of the ASL header
 *
 * @param[in]  frame      A pointer to presentation layer frame
 * @param[in]  cmd_frame  A pointer to application cmd frame
 * @param[out] asl_hdr_sz A pointer to variable to carry out the size
 *
 * @returns #SID_ERROR_NONE on success.
 *          #SID_ERROR_INVALID_ARGS on invalid arguments being passed.
 */
sid_error_t sid_get_asl_header_size(const struct app_pl_frame *const frame,
                                    const struct app_cmd_frame *const cmd_frame, size_t *asl_hdr_sz);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SID_ASL_PARSER_H */
