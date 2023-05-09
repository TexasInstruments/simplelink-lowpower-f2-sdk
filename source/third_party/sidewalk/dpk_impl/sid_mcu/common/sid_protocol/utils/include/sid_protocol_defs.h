/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PROTOCOL_DEFS_H
#define SID_PROTOCOL_DEFS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define U8 uint8_t
#define U16 uint16_t
#define U32 uint32_t
#define U64 uint64_t
#define S8 int8_t
#define S16 int16_t
#define S32 int32_t
#define S64 int64_t

typedef enum {
    RNET_LINK_UNDEF,    // Not specified
    RNET_LINK_APP,
    RNET_LINK_RADIO,
    RNET_LINK_BLE,
    RNET_LINK_SRL,
    RNET_LINK_STACK
} rnet_link_t;

/*!
 * \brief Structure defining the Scheduler State
*/
typedef enum
{
    NO_SUBMODULE = -1,
    BEACON_SUBMODULE = 0,
    HDR_SUBMODULE = 1,
    LDR_SUBMODULE = 2,
    DFU_SUBMODULE = 3,
    DISCO_SUBMODULE = 4,
    HDR_LORA_SUBMODULE = 5, //HDR1.0 module
    SDB_SUBMODULE = 6,
    PROBE_SUBMODULE = 7,
    NUM_SUBMODULES = 8, //This is the MAX number of submodule
} rnet_mac_submodule_t;

// Maximum protocol frame length.
#define RNET_MAX_FRAME_SZ 255

// Max LDR MTU size
#define RNET_MAX_LDR_PSDU_SIZE 44

// Max FSK MTU size
#define RNET_MAX_FSK_PSDU_SIZE RNET_MAX_FRAME_SZ

// Max application payload size in LDR
#define RNET_MAX_LDR_APP_PLD_SIZE 19
// Max application payload size in FSK
#define RNET_MAX_FSK_APP_PLD_SIZE 202  // 200B max FSK MTU + 1B expl_data_len + 1B status code
// Max application payload size in FSK (Non-UASL)
#define RNET_MAX_PAYLOAD_SZ_UASL_NO_SUPPORT 94
#define RNET_MAX_PAYLOAD_SZ RNET_MAX_FSK_APP_PLD_SIZE

#define RNET_MAX_ASL_HEADER_SZ 5
#define RNET_MAX_UASL_HEADER_SZ 10 // 1B control + 1B version + 2B seqn + 1B status_code + 5B addr_enc + addr

#define RNET_SCR_BUF_SZ 512

#define RNET_NULL_IDX   (-1)

#ifndef MS_TO_MICRO_SEC
#define MS_TO_MICRO_SEC(X) ((X)*1000)
#endif

#if defined( __CC_ARM )
#define PACKED(TYPE) __packed TYPE
#define PACKED_STRUCT PACKED(struct)
#elif defined   ( __GNUC__ )
#define PACKED __attribute__((packed))
#define PACKED_STRUCT struct PACKED
#elif defined (__ICCARM__)
#define PACKED_STRUCT __packed struct
#endif

#define SID_UTIL_CONCAT_(a_, b_)                   a_ ## b_
#define SID_UTIL_CONCAT(a_, b_)                    SID_UTIL_CONCAT_(a_,b_)

#define SID_UTIL_CONTAINER_OF(ptr, type, member) \
    ({ \
        const __typeof__(((type *)0)->member) * tmp_member_ = (ptr); \
        ((type *)((uintptr_t)(tmp_member_) - offsetof(type, member))); \
        })

#define SID_UTIL_MAX(X, Y)      ((X) > (Y) ? (X) : (Y))
#define SID_UTIL_MIN(X, Y)      ((X) < (Y) ? (X) : (Y))


#ifdef __cplusplus
}
#endif

#endif
