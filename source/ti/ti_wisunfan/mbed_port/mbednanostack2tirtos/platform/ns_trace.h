/*
 * Copyright (c) 2015-2017 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file ns_trace.h
 * Trace interface abstraction for NanoStack library as well as application.
 *
 * Actual used trace library is mbed-trace. For usage details check mbed_trace.h.
 *
 */
#ifndef NS_TRACE_H_
#define NS_TRACE_H_

#if defined(HAVE_DEBUG) && !defined(FEA_TRACE_SUPPORT)
#define FEA_TRACE_SUPPORT
#endif

#include "ns_types.h"
#include <stdarg.h>

/** this print is some extra debug information added for FEATURE_WISUN_SUPPORT */
#define TRACE_LEVEL_EXTRA_DEBUG   0x12
/** this print is some deep information for debug purpose */
#define TRACE_LEVEL_DEBUG         0x10
/** Info print, for general purpose prints */
#define TRACE_LEVEL_INFO          0x08
/** warning prints, which shouldn't causes any huge problems */
#define TRACE_LEVEL_WARN          0x04
/** Error prints, which causes probably problems, e.g. out of mem. */
#define TRACE_LEVEL_ERROR         0x02
/** special level for cmdline. Behaviours like "plain mode" */
#define TRACE_LEVEL_CMD           0x01


#ifndef EXCLUDE_TRACE
#ifndef MBED_TRACE_MAX_LEVEL
#define MBED_TRACE_MAX_LEVEL TRACE_LEVEL_DEBUG
#ifdef WISUN_TEST_METRICS
// Set this for MPL measurements and build
//#define MBED_TRACE_MAX_LEVEL TRACE_LEVEL_CMD
#endif
#endif
#else
#define MBED_TRACE_MAX_LEVEL 0
#endif



//usage macros:
#if MBED_TRACE_MAX_LEVEL >= TRACE_LEVEL_EXTRA_DEBUG
#define tr_extra_debug(...)           ns_trace_printf(TRACE_LEVEL_INFO,   TRACE_GROUP, __VA_ARGS__)   //!< Print extra debug message as info
#else
#define tr_extra_debug(...)
#endif

#if MBED_TRACE_MAX_LEVEL >= TRACE_LEVEL_DEBUG
#define tr_debug(...)           ns_trace_printf(TRACE_LEVEL_DEBUG,   TRACE_GROUP, __VA_ARGS__)   //!< Print debug message
#else
#define tr_debug(...)
#endif

#if MBED_TRACE_MAX_LEVEL >= TRACE_LEVEL_INFO
#define tr_info(...)            ns_trace_printf(TRACE_LEVEL_INFO,    TRACE_GROUP, __VA_ARGS__)   //!< Print info message
#else
#define tr_info(...)
#endif

#if MBED_TRACE_MAX_LEVEL >= TRACE_LEVEL_WARN
#define tr_warning(...)         ns_trace_printf(TRACE_LEVEL_WARN,    TRACE_GROUP, __VA_ARGS__)   //!< Print warning message
#define tr_warn(...)            ns_trace_printf(TRACE_LEVEL_WARN,    TRACE_GROUP, __VA_ARGS__)   //!< Alternative warning message
#else
#define tr_warning(...)
#define tr_warn(...)
#endif

#if MBED_TRACE_MAX_LEVEL >= TRACE_LEVEL_ERROR
#define tr_error(...)           ns_trace_printf(TRACE_LEVEL_ERROR,   TRACE_GROUP, __VA_ARGS__)   //!< Print Error Message
#define tr_err(...)             ns_trace_printf(TRACE_LEVEL_ERROR,   TRACE_GROUP, __VA_ARGS__)   //!< Alternative error message
#else
#define tr_error(...)
#define tr_err(...)
#endif

#define tr_cmdline(...)         ns_trace_printf(TRACE_LEVEL_CMD,     TRACE_GROUP, __VA_ARGS__)   //!< Special print for cmdline. See more from TRACE_LEVEL_CMD -level
#define tr_mpl(...)             ns_trace_printf(TRACE_LEVEL_CMD,     TRACE_GROUP, __VA_ARGS__)

#ifdef EXCLUDE_TRACE

#define tracef(dlevel, grp, ...)
#define vtracef(dlevel, grp, fmt, ap)
#define tr_array(buf, len)
#define tr_array16(buf, len)
#define tr_ipv6(addr_ptr)
#define tr_ipv6_prefix(prefix, prefix_len)
#define trace_array(buf, len)   NULL
#define trace_ipv6(addr_ptr)
#define trace_ipv6_prefix(prefix, prefix_len)
void ns_trace_init(void);

#else
//aliases for the most commonly used functions and the helper functions
#define tracef(dlevel, grp, ...)                ns_trace_printf(dlevel, grp, __VA_ARGS__)
#define vtracef(dlevel, grp, fmt, ap)           ns_trace_vprintf(dlevel, grp, fmt, ap)
#define tr_array(buf, len)                      ns_trace_array(buf, len)
#define tr_ipv6(addr_ptr)                       ns_trace_ipv6(addr_ptr)
#define tr_ipv6_prefix(prefix, prefix_len)      ns_trace_ipv6_prefix(prefix, prefix_len)
#define trace_array(buf, len)                   ns_trace_array(buf, len)
#define trace_array16(buf, len)                 ns_trace_array16(buf, len)
#define trace_ipv6(addr_ptr)                    ns_trace_ipv6(addr_ptr)
#define trace_ipv6_prefix(prefix, prefix_len)   ns_trace_ipv6_prefix(prefix, prefix_len)
#endif /* EXCLUDE_TRACE */

void ns_trace_printf(uint8_t dlevel, const char *grp, const char *fmt, ...);
void ns_trace_init(void);
void ns_put_char_blocking(const char ch);
void ns_enable_exception_trace(void);
bool ns_enable_data_trace(const uint32_t *variable);
void ns_enable_cycle_counter(void);
void ns_flush_module(void);
char *ns_trace_ipv6(const void *addr_ptr);
char *ns_trace_ipv6_prefix(const uint8_t *prefix, uint8_t prefix_len);
char *ns_trace_array(const uint8_t *buf, uint16_t len);
char *ns_trace_array16(const uint16_t *buf, uint16_t len);

void ns_enable_module();
void ns_disable_module(void);
void ns_trace_vprintf(uint8_t dlevel, const char *grp, const char *fmt, va_list ap);

#endif /* NS_TRACE_H_ */
