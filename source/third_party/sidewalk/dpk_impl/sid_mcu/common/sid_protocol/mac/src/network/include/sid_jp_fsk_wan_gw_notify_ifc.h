/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SID_JP_FSK_WAN_GW_NOTIFY_IFC_H_
#define SID_JP_FSK_WAN_GW_NOTIFY_IFC_H_

/**
 * @brief Consumes the signal from the HDR module about transmission of CMD_ID_RNET_JOIN_PROC_RESP
 *
 * @param[in]   dest_id     The ID for which JOIN_PROC_RESP is sent.
 */
sid_error_t sid_jp_notify_join_resp_sent(uint8_t *dest_id);

#endif /* SID_JP_FSK_WAN_GW_NOTIFY_IFC_H_ */
