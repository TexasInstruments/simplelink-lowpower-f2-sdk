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

/*
 * rnetc_log.h
 *
 * RingNet logging interface
 *
 * XXX
 * This is simple version which should be reworked in nearest future
 */

#ifndef INCLUDE_RNETC_LOG_H_
#define INCLUDE_RNETC_LOG_H_

#include <sid_network_address.h>
#include <halo/lib/hall/appv1/cmd.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NDEBUG

#define RN_LOG_D(frm, ...)
#define RN_LOG_I(frm, ...)
#define RN_LOG_W(frm, ...)
#define RN_LOG_E(frm, ...)
#define RN_LOG_C(frm, ...)

#else

#include <sid_pal_log_ifc.h>

#define RN_LOG_D(frm, ...) \
    SID_PAL_LOG_INFO(frm, ##__VA_ARGS__) // HALO_LOG_DEBUG - some issues in compile time warning: implicit declaration of function 'NRF_LOG_1';
#define RN_LOG_I(frm, ...) SID_PAL_LOG_INFO(frm, ##__VA_ARGS__)
#define RN_LOG_W(frm, ...) SID_PAL_LOG_WARNING(frm, ##__VA_ARGS__)
#define RN_LOG_E(frm, ...) SID_PAL_LOG_ERROR(frm, ##__VA_ARGS__)
#define RN_LOG_C(frm, ...) SID_PAL_LOG_ERROR(frm, ##__VA_ARGS__)

#endif // NDEBUG

#ifdef __cplusplus
} // extern "C"
#endif
#endif /* INCLUDE_RNETC_LOG_H_ */
