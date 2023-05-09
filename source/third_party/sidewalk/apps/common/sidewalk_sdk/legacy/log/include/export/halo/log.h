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

#ifndef HALO_LIB_HALO_LOG__EXPORT_HALO_LOG_H
#define HALO_LIB_HALO_LOG__EXPORT_HALO_LOG_H

#include <stdint.h>
#include <sid_pal_log_ifc.h>

int  halo_log_init(uint32_t (*log_timestamp_func)(void));
void halo_log_final_flush(void);
int  halo_log_process(void);
void halo_log_default_backends_init(void);

#ifndef HALO_LOG_LEVEL
#ifdef HALO_LOG_DEFAULT_LEVEL
#define HALO_LOG_LEVEL                     HALO_LOG_DEFAULT_LEVEL
#else
#define HALO_LOG_LEVEL                     NRF_LOG_SEVERITY_INFO
#endif //HALO_LOG_DEFAULT_LEVEL
#endif //HALO_LOG_LEVEL

#if defined(HALO_WITH_MODULE_HALO_PLATFORM_NORDIC_NRF52)
#include <nrf_log.h>

#undef NRF_LOG_LEVEL
#define NRF_LOG_LEVEL HALO_LOG_LEVEL

#ifdef HALO_LOG_DEFAULT_LEVEL
#undef NRF_LOG_DEFAULT_LEVEL
#define NRF_LOG_DEFAULT_LEVEL HALO_LOG_DEFAULT_LEVEL
#endif

#ifdef HALO_LOG_DEBUG_COLOR
#undef NRF_LOG_DEBUG_COLOR
#define NRF_LOG_DEBUG_COLOR HALO_LOG_DEBUG_COLOR
#endif

#ifdef HALO_LOG_INFO_COLOR
#undef NRF_LOG_INFO_COLOR
#define NRF_LOG_INFO_COLOR HALO_LOG_INFO_COLOR
#endif

#ifdef HALO_LOG_PREFIX
#undef NRF_LOG_MODULE_NAME
#define NRF_LOG_MODULE_NAME HALO_LOG_PREFIX
NRF_LOG_MODULE_REGISTER();
#endif

const char * halo_log_push_str(char* p_str);
#  define HALO_LOG_PUSH_STR(str)            halo_log_push_str(str)

#define HALO_LOG_ERROR(...)     NRF_LOG_ERROR(__VA_ARGS__)
#define HALO_LOG_WARNING(...)   NRF_LOG_WARNING(__VA_ARGS__)
#define HALO_LOG_INFO(...)      NRF_LOG_INFO(__VA_ARGS__)
#define HALO_LOG_DEBUG(...)     NRF_LOG_DEBUG(__VA_ARGS__)
#else
#define HALO_LOG_ERROR(fmt_, ...)           SID_PAL_LOG(SID_PAL_LOG_SEVERITY_ERROR,   fmt_, ##__VA_ARGS__)
#define HALO_LOG_WARNING(fmt_, ...)         SID_PAL_LOG(SID_PAL_LOG_SEVERITY_WARNING, fmt_, ##__VA_ARGS__)
#define HALO_LOG_INFO(fmt_, ...)            SID_PAL_LOG(SID_PAL_LOG_SEVERITY_INFO,    fmt_, ##__VA_ARGS__)
#define HALO_LOG_DEBUG(fmt_, ...)           SID_PAL_LOG(SID_PAL_LOG_SEVERITY_DEBUG,   fmt_, ##__VA_ARGS__)
#endif // defined(HALO_WITH_MODULE_HALO_PLATFORM_NORDIC_NRF52)

#define HALO_LOG_FINAL_FLUSH()              halo_log_final_flush()
#define HALO_LOG_HEXDUMP_ERROR(prefix, ...)   /* Stub */
#define HALO_LOG_HEXDUMP_WARNING(prefix, ...) /* Stub */
#define HALO_LOG_HEXDUMP_INFO(prefix, ...)    /* Stub */
#define HALO_LOG_HEXDUMP_DEBUG(prefix, ...)   /* Stub */

#define HALO_LOG_PROCESS()                  halo_log_process()

#define HALO_LOG_INIT(...)                  halo_log_init(__VA_ARGS__)
#define HALO_LOG_TRACE()                    HALO_LOG_INFO("%s:%i %s() TRACE --", __FILENAME__, __LINE__, __FUNCTION__)

#define HALO_LOG_DEFAULT_BACKENDS_INIT()    halo_log_default_backends_init()

#endif
