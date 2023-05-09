/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef HALO_LIB_HALO_LOG_INCLUDE_EXPORT_HALO_EVENT_LOG_H
#define HALO_LIB_HALO_LOG_INCLUDE_EXPORT_HALO_EVENT_LOG_H

#include <halo/error.h>
#include <halo/utils.h>

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HALO_EVENT_LOG_DEFAULT_BUFFER_SIZE
#define HALO_EVENT_LOG_DEFAULT_BUFFER_SIZE   800
#endif

#ifndef HALO_EVENT_LOG_OUTPUT_BUFFER_SIZE
#define HALO_EVENT_LOG_OUTPUT_BUFFER_SIZE    256   /* Enough for 128B hex dump in base64 encoding*/
#endif

#if defined(HALO_EVENT_LOG_ENABLED) && HALO_EVENT_LOG_ENABLED

#ifndef HALO_EVENT_LOG_DATABASE_FILE_NAME
#define HALO_EVENT_LOG_DATABASE_FILE_NAME "halo/event_log_database.h"
#endif // HALO_EVENT_LOG_DATABASE_FILE_NAME

#define HALO_EVENT_LOG_ID(tag_id_, module_id_, message_id_) ((HALO_EVENT_LOG_TAG_##tag_id_ << 24) | (CONCAT4(HALO_EVENT_LOG_MODULE_, tag_id_ , _,  module_id_) << 16) | message_id_)

#if defined(HALO_EVENT_LOG_RENDERER) && HALO_EVENT_LOG_RENDERER

#define HALO_EVENT_LOG_DEFINE(id_, name_, raw_message_) static const char *name_ = raw_message_;

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include HALO_EVENT_LOG_DATABASE_FILE_NAME

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

typedef const char * halo_event_log_id_t;
#undef HALO_EVENT_LOG_DEFINE

#else

/* Declare IDs for events */
#define HALO_EVENT_LOG_DEFINE(id_, name_, raw_message_) name_ = id_,
typedef enum  {
    #include HALO_EVENT_LOG_DATABASE_FILE_NAME
} halo_event_log_id_t;
#undef HALO_EVENT_LOG_DEFINE

#endif /* defined(HALO_EVENT_LOG_RENDERER) && HALO_EVENT_LOG_RENDERER */

#define GET_HALO_EVENT_LOG_N(_0, _1,_2,_3,_4,NAME,...) NAME
#define HALO_EVENT_LOG_0(id_, ...)  (id_, 0)
#define HALO_EVENT_LOG_1(id_, ...) halo_event_log_internal(id_, 1, __VA_ARGS__)
#define HALO_EVENT_LOG_2(id_, ...) halo_event_log_internal(id_, 2, __VA_ARGS__)
#define HALO_EVENT_LOG_3(id_, ...) halo_event_log_internal(id_, 3, __VA_ARGS__)
#define HALO_EVENT_LOG_4(id_, ...) halo_event_log_internal(id_, 4, __VA_ARGS__)

/* Main APIs */
#define HALO_EVENT_LOG_INIT(...) halo_event_log_init(halo_event_log_default_buffer, HALO_EVENT_LOG_DEFAULT_BUFFER_SIZE, __VA_ARGS__)
#define HALO_EVENT_LOG(...) GET_HALO_EVENT_LOG_N(__VA_ARGS__, HALO_EVENT_LOG_4, HALO_EVENT_LOG_3,HALO_EVENT_LOG_2,HALO_EVENT_LOG_1,HALO_EVENT_LOG_0)(__VA_ARGS__)
#define HALO_EVENT_LOG_HEXDUMP(...) halo_event_log_hexdump_internal(__VA_ARGS__)
#define HALO_EVENT_LOG_PROCESS(...) halo_event_log_process(__VA_ARGS__)
#define HALO_EVENT_LOG_DUMP_QUEUE(...) halo_event_log_dump_queue(__VA_ARGS__)

halo_error_t halo_event_log_init(void *buffer, size_t buffer_size, uint32_t (*log_timestamp_func)(void));
halo_error_t halo_event_log_deinit(void);
halo_error_t halo_event_log_internal(halo_event_log_id_t event_id, int n, ...);
halo_error_t halo_event_log_hexdump_internal(halo_event_log_id_t event_id, const uint8_t *buffer, size_t length);
halo_error_t halo_event_log_process(void);

extern uint32_t halo_event_log_default_buffer[];

#else

typedef uint32_t halo_event_log_id_t;

#define HALO_EVENT_LOG_INIT(...)    ({HALO_ERROR_NONE;})
#define HALO_EVENT_LOG(...)         ({HALO_ERROR_NONE;})
#define HALO_EVENT_LOG_HEXDUMP(...) ({HALO_ERROR_NONE;})
#define HALO_EVENT_LOG_PROCESS(...) ({HALO_ERROR_NONE;})

#endif /* !HALO_EVENT_LOG_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /*! HALO_LIB_HALO_LOG_INCLUDE_EXPORT_HALO_EVENT_LOG_H */
