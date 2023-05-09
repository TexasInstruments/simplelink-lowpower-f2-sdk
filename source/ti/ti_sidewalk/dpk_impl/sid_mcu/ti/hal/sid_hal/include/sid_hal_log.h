/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_HAL_LOG_H
#define SID_HAL_LOG_H

#include <sid_pal_log_ifc.h>

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Below macros are for compatibility only, very brittle
 */
#define SID_HAL_EVENT_LOG_ID(tag_id_, module_id_, message_id_) ((SID_HAL_EVENT_LOG_TAG_##tag_id_ << 24) | (CONCAT4(SID_HAL_EVENT_LOG_MODULE_, tag_id_ , _,  module_id_) << 16) | message_id_)

#if defined(SID_EVENT_LOG_RENDERER) && SID_EVENT_LOG_RENDERER
#define SID_HAL_EVENT_LOG_DEFINE(id_, name_, raw_message_) static const char *name_ = raw_message_;
#include SID_EVENT_LOG_DATABASE_FILE_NAME
#endif

#define SID_HAL_LOG_SEVERITY_ERROR      0
#define SID_HAL_LOG_SEVERITY_WARNING    1
#define SID_HAL_LOG_SEVERITY_INFO       2
#define SID_HAL_LOG_SEVERITY_DEBUG      3

#ifndef SID_HAL_LOG_OUTPUT_LEVEL
#define SID_HAL_LOG_OUTPUT_LEVEL  SID_HAL_LOG_SEVERITY_INFO
#endif

#if SID_HAL_DISABLE_LOGS

#define SID_HAL_LOG_INFO(...)
#define SID_HAL_LOG_DEBUG(...)
#define SID_HAL_LOG_ERROR(...)
#define SID_HAL_LOG_WARNING(...)
#define SID_HAL_LOG_PUSH_STR(...)
#define SID_HAL_LOG_HEXDUMP_WARNING(data, len)
#define SID_HAL_LOG_HEXDUMP_INFO(data, len)
#define SID_HAL_LOG_HEXDUMP_DEBUG(data, len)
#define SID_HAL_LOG_FLUSH()

#else
#include <ti_itm.h>
#include <stdio.h>
#include <string.h>

#define XSTR_(s) #s
#define XSTR(s) XSTR_(s)

#ifdef SID_HAL_LOG_PREFIX

#define SID_HAL_LOG_MAKE_STRING(level_, fmt_, ...) do {                          \
    if (level_ <= SID_HAL_LOG_OUTPUT_LEVEL) {                                    \
        char str_[strlen(XSTR(SID_HAL_LOG_PREFIX)) + strlen(fmt_) + 3];          \
        snprintf(str_, sizeof(str_), "%s: %s", XSTR(SID_HAL_LOG_PREFIX), fmt_);  \
        SID_PAL_LOG(level_, str_, ##__VA_ARGS__);                                \
    }                                                                            \
} while(0)

#define SID_HAL_LOG_INFO(fmt_, ...)  \
    SID_HAL_LOG_MAKE_STRING(SID_HAL_LOG_SEVERITY_INFO, fmt_, ##__VA_ARGS__)

#define SID_HAL_LOG_ERROR(fmt_, ...)  \
    SID_HAL_LOG_MAKE_STRING(SID_HAL_LOG_SEVERITY_ERROR, fmt_, ##__VA_ARGS__)

#define SID_HAL_LOG_DEBUG(fmt_, ...)  \
    SID_HAL_LOG_MAKE_STRING(SID_HAL_LOG_SEVERITY_DEBUG, fmt_, ##__VA_ARGS__)

#define SID_HAL_LOG_WARNING(fmt_, ...)  \
    SID_HAL_LOG_MAKE_STRING(SID_HAL_LOG_SEVERITY_WARNING, fmt_, ##__VA_ARGS__)

#else
#define SID_HAL_LOG_INFO    SID_PAL_LOG_INFO
#define SID_HAL_LOG_DEBUG    SID_PAL_LOG_DEBUG
#define SID_HAL_LOG_ERROR    SID_PAL_LOG_ERROR
#define SID_HAL_LOG_WARNING    SID_PAL_LOG_WARNING
#endif

#define SID_HAL_LOG_PUSH_STR
#define SID_HAL_LOG_HEXDUMP_WARNING(...)    SID_PAL_HEXDUMP(SID_PAL_LOG_SEVERITY_WARNING, __VA_ARGS__)
#define SID_HAL_LOG_HEXDUMP_DEBUG(...)    SID_PAL_HEXDUMP(SID_PAL_LOG_SEVERITY_DEBUG, __VA_ARGS__)
#define SID_HAL_LOG_HEXDUMP_INFO(...)    SID_PAL_HEXDUMP(SID_PAL_LOG_SEVERITY_INFO, __VA_ARGS__)
#define SID_HAL_LOG_FLUSH()
#endif

#ifdef __cplusplus
}
#endif

#endif // SID_HAL_LOG_H
