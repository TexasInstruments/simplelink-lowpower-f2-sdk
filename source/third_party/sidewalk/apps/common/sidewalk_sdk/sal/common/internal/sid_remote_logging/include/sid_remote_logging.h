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

#ifndef SID_REMOTE_LOGGING_H
#define SID_REMOTE_LOGGING_H

#include <sid_pal_log_ifc.h>

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void sid_remote_logging_vprintf(const char *prefix, const char *fmt, va_list ap);

bool sid_remote_logging_get_buffer(struct sid_pal_log_buffer *const buffer);

#ifdef __cplusplus
}
#endif

#endif
