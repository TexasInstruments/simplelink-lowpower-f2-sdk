/*
 * Copyright (c) 2021, Nordic Semiconductor ASA. All rights reserved.
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __EXCEPTION_INFO_H__
#define __EXCEPTION_INFO_H__

#include <stdint.h>

#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
#define TRUSTZONE_PRESENT
#endif

#if defined(__ARM_ARCH_8_1M_MAIN__) || defined(__ARM_ARCH_8M_MAIN__) \
    || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#define FAULT_STATUS_PRESENT
#endif

/* Arguments to EXCEPTION_INFO() */
#define EXCEPTION_TYPE_SECUREFAULT 0
#define EXCEPTION_TYPE_HARDFAULT   1
#define EXCEPTION_TYPE_MEMFAULT    2
#define EXCEPTION_TYPE_BUSFAULT    3
#define EXCEPTION_TYPE_USAGEFAULT  4
#define EXCEPTION_TYPE_PLATFORM    5

/* This level of indirection is needed to fully resolve exception info when it's
 * a macro
 */
#ifndef __STRINGIFY /* TI-TFM: Added #ifndef to avoid redefinition warning */
#define __STRINGIFY(exception_info) #exception_info
#endif

/* Store context for an exception, and print an error message with the context.
 *
 * @param[in]  exception_type  One of the EXCEPTION_TYPE_* values defined above. Any
 *                             other value will result in printing "Unknown".
 */
#ifdef TFM_EXCEPTION_INFO_DUMP

/* Store context for an exception, then print the info.
 * Call EXCEPTION_INFO() instead of calling this directly.
 */
void store_and_dump_context(uint32_t LR_in, uint32_t MSP_in, uint32_t PSP_in,
                            uint32_t exception_type);

/* IAR Specific */
#if defined(__ICCARM__)
#pragma required = store_and_dump_context
#endif

#define EXCEPTION_INFO(exception_type)                  \
    __ASM volatile(                                     \
        "MOV     r0, lr\n"                              \
        "MRS     r1, MSP\n"                             \
        "MRS     r2, PSP\n"                             \
        "MOVS    r3, #" __STRINGIFY(exception_type) "\n"\
        "BL      store_and_dump_context\n"              \
    )

#else /* TFM_EXCEPTION_INFO_DUMP */
#define EXCEPTION_INFO(exception_type)
#endif /* TFM_EXCEPTION_INFO_DUMP */

#endif /* __EXCEPTION_INFO_H__ */
