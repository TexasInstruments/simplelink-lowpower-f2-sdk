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

#ifndef HALO_HALL_APPV1_APPV1_H_
#define HALO_HALL_APPV1_APPV1_H_

#include <halo/lib/hall/appv1/appv1.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 1B value */
#define RNET_APP_LAYER_VERSION (0x01)

#define RNET_CMD_MAX_LEN (RNET_CMD_MIN_LEN + 8 + 255)

#define PAN_ID_SIZE (5)
#define P2P_BROADCAST_KEY_SIZE (16)
#define AUTH_TOKEN_SIZE (37)

typedef enum {
    RNET_LL_OPC_RD = 0x0, //!< RNET_LL_OPC_RD
    RNET_LL_OPC_WR = 0x1, //!< RNET_LL_OPC_WR
    RNET_LL_OPC_NTFY = 0x2, //!< RNET_LL_OPC_NTFY
    RNET_LL_OPC_RSP = 0x3, //!< RNET_LL_OPC_RSP

    _RNET_LL_OPC_COUNT = 4,
    _RNET_LL_OPC_MASK = 0x03 //!< _RNET_LL_OPC_MASK
} halo_hall_cmd_opc;

#define RNET_CMD_READ(X) (((X) << 2) | RNET_LL_OPC_RD)
#define RNET_CMD_WRITE(X) (((X) << 2) | RNET_LL_OPC_WR)
#define RNET_CMD_NOTIFY(X) (((X) << 2) | RNET_LL_OPC_NTFY)
#define RNET_CMD_RESP(X) (((X) << 2) | RNET_LL_OPC_RSP)

#define RNET_CMD(X, OPC) (((X) << 2) | (OPC))

#define RNET_GET_CMD_TYPE(X) ((X)&_RNET_LL_OPC_MASK)
#define RNET_GET_CMD(X) (((X) >> 2) & 0x3FFF)

#define RNET_MERGE_CMD_ID_OP_CODE(X, Y) ((X << 2) | (Y & _RNET_LL_OPC_MASK))

#define RNET_CMD_HDR_SCI_ONLY 0x80
#define RNET_CMD_HDR_NULL 0x00

// |1b status code inc.| |1b additional status data inc.| |1b log entries inc.| |1b battery level inc.| |1b RSSI/SNR inc.| |1b timestamp inc.| |2b
// reserved|
typedef enum {
    RNET_CMD_DH_STATUS_CODE = 0x80,
    RNET_CMD_DH_ADDITIONAL_STATUS_CODE = 0x40,
    RNET_CMD_DH_LOG_ENTRIES = 0x20,
    RNET_CMD_DH_BATTERY_LEVEL = 0x10,
    RNET_CMD_DH_SNR_RSSI = 0x08,
    RNET_CMD_DH_TIMESTAMP = 0x04,

    _RNET_CMD_DH_MASK = RNET_CMD_DH_STATUS_CODE | RNET_CMD_DH_ADDITIONAL_STATUS_CODE | RNET_CMD_DH_LOG_ENTRIES | RNET_CMD_DH_BATTERY_LEVEL |
                        RNET_CMD_DH_SNR_RSSI | RNET_CMD_DH_TIMESTAMP
} rnet_cmd_dh;

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALO_HALL_APPV1_APPV1_H_ */
