/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNET_NETWORK_DISTRESS_GW_OPERATION_H
#define RNET_NETWORK_DISTRESS_GW_OPERATION_H

#include <stdbool.h>
#include <sid_protocol_defs.h>
#include <sid_protocol_opts.h>
#include "rnet_config.h"

typedef enum {
    GW_BACK_ONLINE = 0,
    GW_IN_DISTRESS = 1,
    START_FTP_SESSION,
    STOP_FTP_SESSION,
    DEDICATED_LINK_READY,
    DEDICATED_LINK_SETUP_FAILED,
    DEDICATED_LINK_TERMINATE
} rnet_dev_status_t;

#if defined(GW_SUPPORT) && GW_SUPPORT
/*!
 * @brief: function API to call from the app layer when cloud ack is received
 * Example: Device announcement could be: device is in distress, FTP requested, etc.
 * Once the command is received by the destination, it responds to the sender with an ACK.
 * Once the ack is received, this function takes the burden to process the ACK correctly.
 * @param[in] sequence number: sequence number, which works as message ID, is used to
 * cross validate the ACK with initial command.
 */
void rnet_announcement_ack_rcved_frm_cld(uint16_t msg_sqn);
/*!
 * @brief: Starts distress mode operation
 */
void rnet_start_distress_mode_of_operation(void);
/*!
 * @brief: stops distress mode of operation
 */
void rnet_stop_distress_mode(void);

/*!
 * @brief: start a session to send the ftp request to the cloud
 * @return true if the start of FTP request process is successful
 * otherwise returns: false
 */
bool rnet_start_ftp_request_session(void);

/*!
 * @brief: Function to send the "connection restored status" to the cloud
 */
void rnet_send_conn_restore_notification_to_cld(void);

/*!
 * @brief: Function to get default configuration
 */
rnet_gw_distress_config_t* rnet_get_dflt_distress_cfg(void);

#endif //#if defined(GW_SUPPORT) && GW_SUPPORT
#endif /* RNET_NETWORK_DISTRESS_GW_OPERATION_H */
