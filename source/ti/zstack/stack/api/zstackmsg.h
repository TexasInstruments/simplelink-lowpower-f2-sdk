/**
   @file  zstackmsg.h
   $Date: 2015-02-18 09:35:16 -0800 (Wed, 18 Feb 2015) $
   $Revision: 42724 $

   @brief ZStack API messages

   <!--
   Copyright 2014 - 2015 Texas Instruments Incorporated.

   All rights reserved not granted herein.
   Limited License.

   Texas Instruments Incorporated grants a world-wide, royalty-free,
   non-exclusive license under copyrights and patents it now or hereafter
   owns or controls to make, have made, use, import, offer to sell and sell
   ("Utilize") this software subject to the terms herein. With respect to the
   foregoing patent license, such license is granted solely to the extent that
   any such patent is necessary to Utilize the software alone. The patent
   license shall not apply to any combinations which include this software,
   other than combinations with devices manufactured by or for TI ("TI
   Devices"). No hardware patent is licensed hereunder.

   Redistributions must preserve existing copyright notices and reproduce
   this license (including the above copyright notice and the disclaimer and
   (if applicable) source code license limitations below) in the documentation
   and/or other materials provided with the distribution.

   Redistribution and use in binary form, without modification, are permitted
   provided that the following conditions are met:

     * No reverse engineering, decompilation, or disassembly of this software
       is permitted with respect to any software provided in binary form.
     * Any redistribution and use are licensed by TI for use only with TI Devices.
     * Nothing shall obligate TI to provide you with source code for the software
       licensed and provided to you in object code.

   If software source code is provided to you, modification and redistribution
   of the source code are permitted provided that the following conditions are
   met:

     * Any redistribution and use of the source code, including any resulting
       derivative works, are licensed by TI for use only with TI Devices.
     * Any redistribution and use of any object code compiled from the source
       code and any resulting derivative works, are licensed by TI for use
       only with TI Devices.

   Neither the name of Texas Instruments Incorporated nor the names of its
   suppliers may be used to endorse or promote products derived from this
   software without specific prior written permission.

   DISCLAIMER.

   THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
   OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   -->
 */
#ifndef ZSTACKMSG_H
#define ZSTACKMSG_H

#include <stdbool.h>
#include <stdint.h>

#include "zstack.h"
#include "bdb.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
// Constants and definitions
//*****************************************************************************

/**
 * Command IDs - each of these are associated with Request,
 * Responses, Indications, and Confirm messages
 */
typedef enum
{
    zstackmsg_CmdIDs_SYS_RESET_REQ = 0x00,
    zstackmsg_CmdIDs_SYS_VERSION_REQ = 0x02,
    zstackmsg_CmdIDs_SYS_CONFIG_READ_REQ = 0x03,
    zstackmsg_CmdIDs_SYS_CONFIG_WRITE_REQ = 0x04,
    zstackmsg_CmdIDs_SYS_APP_MSG_REQ = 0x05,
    zstackmsg_CmdIDs_SYS_OTA_MSG_REQ = 0x06,
    zstackmsg_CmdIDs_SYS_SET_TX_POWER_REQ = 0x07,
    zstackmsg_CmdIDs_SYS_NWK_INFO_READ_REQ = 0x08,
    zstackmsg_CmdIDs_SYS_NWK_FRAME_FWD_NOTIFICATION_REQ = 0x09,
    zstackmsg_CmdIDs_DEV_START_REQ = 0x10,
    zstackmsg_CmdIDs_DEV_NWK_DISCOVERY_REQ = 0x11,
    zstackmsg_CmdIDs_DEV_JOIN_REQ = 0x12,
    zstackmsg_CmdIDs_DEV_REJOIN_REQ = 0x13,
    zstackmsg_CmdIDs_DEV_ZDO_CBS_REQ = 0x14,
    zstackmsg_CmdIDs_DEV_NWK_ROUTE_REQ = 0x15,
    zstackmsg_CmdIDs_DEV_NWK_CHECK_ROUTE_REQ = 0x16,
    // 0x17 is available for use. Was used for zstackmsg_CmdIDs_DEV_JAMMER_IND
    zstackmsg_CmdIDs_APS_REMOVE_GROUP = 0x18,
    zstackmsg_CmdIDs_APS_REMOVE_ALL_GROUPS = 0x19,
    zstackmsg_CmdIDs_APS_FIND_ALL_GROUPS_REQ = 0x1B,
    zstackmsg_CmdIDs_APS_FIND_GROUP_REQ = 0x1C,
    zstackmsg_CmdIDs_APS_ADD_GROUP = 0x1E,
    zstackmsg_CmdIDs_APS_COUNT_ALL_GROUPS = 0x1F,
    zstackmsg_CmdIDs_AF_REGISTER_REQ = 0x20,
    zstackmsg_CmdIDs_AF_UNREGISTER_REQ = 0x21,
    zstackmsg_CmdIDs_AF_DATA_REQ = 0x22,
    zstackmsg_CmdIDs_ZDO_NODE_DESC_REQ = 0x28,
    zstackmsg_CmdIDs_ZDO_POWER_DESC_REQ = 0x29,
    zstackmsg_CmdIDs_ZDO_SIMPLE_DESC_REQ = 0x2A,
    zstackmsg_CmdIDs_ZDO_ACTIVE_ENDPOINT_REQ = 0x2B,
    zstackmsg_CmdIDs_ZDO_MATCH_DESC_REQ = 0x2C,
    zstackmsg_CmdIDs_ZDO_COMPLEX_DESC_REQ = 0x38,
    zstackmsg_CmdIDs_ZDO_SERVER_DISC_REQ = 0x39,
    zstackmsg_CmdIDs_ZDO_END_DEVICE_BIND_REQ = 0x3A,
    zstackmsg_CmdIDs_ZDO_BIND_REQ = 0x3B,
    zstackmsg_CmdIDs_ZDO_UNBIND_REQ = 0x3C,
    zstackmsg_CmdIDs_ZDO_MGMT_NWK_DISC_REQ = 0x3D,
    zstackmsg_CmdIDs_ZDO_MGMT_LQI_REQ = 0x3E,
    zstackmsg_CmdIDs_ZDO_MGMT_RTG_REQ = 0x3F,
    zstackmsg_CmdIDs_ZDO_MGMT_BIND_REQ = 0x40,
    zstackmsg_CmdIDs_ZDO_MGMT_LEAVE_REQ = 0x41,
    zstackmsg_CmdIDs_ZDO_MGMT_DIRECT_JOIN_REQ = 0x42,
    zstackmsg_CmdIDs_ZDO_MGMT_PERMIT_JOIN_REQ = 0x43,
    zstackmsg_CmdIDs_ZDO_MGMT_NWK_UPDATE_REQ = 0x44,
    zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE_REQ = 0x45,
    zstackmsg_CmdIDs_ZDO_USER_DESCR_SET_REQ = 0x46,
    zstackmsg_CmdIDs_ZDO_USER_DESC_REQ = 0x47,
    zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE = 0x48,
    zstackmsg_CmdIDs_AF_INTERPAN_CTL_REQ = 0x49,
    zstackmsg_CmdIDs_AF_CONFIG_GET_REQ = 0x4A,
    zstackmsg_CmdIDs_AF_CONFIG_SET_REQ = 0x4B,
    zstackmsg_CmdIDs_ZDO_NWK_ADDR_REQ = 0x4C,
    zstackmsg_CmdIDs_ZDO_IEEE_ADDR_REQ = 0x4D,
    zstackmsg_CmdIDs_ZDO_NWK_ADDR_RSP = 0x60,
    zstackmsg_CmdIDs_ZDO_IEEE_ADDR_RSP = 0x61,
    zstackmsg_CmdIDs_ZDO_NODE_DESC_RSP = 0x62,
    zstackmsg_CmdIDs_ZDO_POWER_DESC_RSP = 0x63,
    zstackmsg_CmdIDs_ZDO_SIMPLE_DESC_RSP = 0x64,
    zstackmsg_CmdIDs_ZDO_ACTIVE_EP_RSP = 0x65,
    zstackmsg_CmdIDs_ZDO_MATCH_DESC_RSP = 0x66,
    zstackmsg_CmdIDs_ZDO_COMPLEX_DESC_RSP = 0x67,
    zstackmsg_CmdIDs_ZDO_USER_DESC_RSP = 0x68,
    zstackmsg_CmdIDs_ZDO_USER_DESC_SET_RSP = 0x69,
    zstackmsg_CmdIDs_ZDO_SERVER_DISC_RSP = 0x6A,
    zstackmsg_CmdIDs_ZDO_END_DEVICE_BIND_RSP = 0x6B,
    zstackmsg_CmdIDs_ZDO_BIND_RSP = 0x6C,
    zstackmsg_CmdIDs_ZDO_UNBIND_RSP = 0x6D,
    zstackmsg_CmdIDs_ZDO_MGMT_NWK_DISC_RSP = 0x6E,
    zstackmsg_CmdIDs_ZDO_MGMT_LQI_RSP = 0x6F,
    zstackmsg_CmdIDs_ZDO_MGMT_RTG_RSP = 0x70,
    zstackmsg_CmdIDs_ZDO_MGMT_BIND_RSP = 0x71,
    zstackmsg_CmdIDs_ZDO_MGMT_LEAVE_RSP = 0x72,
    zstackmsg_CmdIDs_ZDO_MGMT_DIRECT_JOIN_RSP = 0x73,
    zstackmsg_CmdIDs_ZDO_MGMT_PERMIT_JOIN_RSP = 0x74,
    zstackmsg_CmdIDs_ZDO_MGMT_NWK_UPDATE_NOTIFY = 0x75,
    zstackmsg_CmdIDs_ZDO_SRC_RTG_IND = 0x76,
    zstackmsg_CmdIDs_ZDO_CONCENTRATOR_IND = 0x77,
    zstackmsg_CmdIDs_ZDO_NWK_DISC_CNF = 0x78,
    zstackmsg_CmdIDs_ZDO_BEACON_NOTIFY_IND = 0x79,
    zstackmsg_CmdIDs_ZDO_JOIN_CNF = 0x7A,
    zstackmsg_CmdIDs_ZDO_LEAVE_CNF = 0x7B,
    zstackmsg_CmdIDs_ZDO_LEAVE_IND = 0x7C,
    zstackmsg_CmdIDs_TL_SCAN_REC_IND = 0x80,
    zstackmsg_CmdIDs_TL_TARGET_NWK_START_REC_IND = 0x81,
    zstackmsg_CmdIDs_TOUCHLINK_NWK_JOIN_REC_IND = 0x82,
    zstackmsg_CmdIDs_TL_TARGET_RESET_TO_FN_REC_IND = 0x83,
    zstackmsg_CmdIDs_TOUCHLINK_NWK_UPDATE_REC_IND = 0x84,
    zstackmsg_CmdIDs_TL_GET_SCAN_BASE_TIME = 0x85,
    zstackmsg_CmdIDs_TL_INITIATOR_SCAN_RSP_IND = 0x86,
    zstackmsg_CmdIDs_TOUCHLINK_DEV_INFO_RSP_IND = 0x87,
    zstackmsg_CmdIDs_TL_INITIATOR_NWK_START_RSP_IND = 0x88,
    zstackmsg_CmdIDs_TL_INITIATOR_NWK_JOIN_RSP_IND = 0x89,
    zstackmsg_CmdIDs_SYS_RESET_IND = 0x90,
    zstackmsg_CmdIDs_AF_DATA_CONFIRM_IND = 0x91,
    zstackmsg_CmdIDs_AF_INCOMING_MSG_IND = 0x92,
    zstackmsg_CmdIDs_AF_REFLECT_ERROR_IND = 0x93,
    zstackmsg_CmdIDs_DEV_STATE_CHANGE_IND = 0x94,
    zstackmsg_CmdIDs_ZDO_TC_DEVICE_IND = 0x95,
    zstackmsg_CmdIDs_DEV_PERMIT_JOIN_IND = 0x96,
    zstackmsg_CmdIDs_SEC_NWK_KEY_GET_REQ = 0xA0,
    zstackmsg_CmdIDs_SEC_NWK_KEY_SET_REQ = 0xA1,
    zstackmsg_CmdIDs_SEC_NWK_KEY_UPDATE_REQ = 0xA2,
    zstackmsg_CmdIDs_SEC_NWK_KEY_SWITCH_REQ = 0xA3,
    zstackmsg_CmdIDs_SEC_APS_LINKKEY_GET_REQ = 0xA4,
    zstackmsg_CmdIDs_SEC_APS_LINKKEY_SET_REQ = 0xA5,
    zstackmsg_CmdIDs_SEC_APS_LINKKEY_REMOVE_REQ = 0xA6,
    zstackmsg_CmdIDs_SEC_APS_REMOVE_REQ = 0xA7,
    zstackmsg_CmdIDs_SYS_FORCE_LINK_STATUS_REQ = 0xA8,
    zstackmsg_CmdIDs_DEV_UPDATE_NEIGHBOR_TXCOST_REQ = 0xA9,
    zstackmsg_CmdIDs_DEV_FORCE_NETWORK_SETTINGS_REQ = 0xAA,
    zstackmsg_CmdIDs_DEV_FORCE_NETWORK_UPDATE_REQ = 0xAB,
    zstackmsg_CmdIDs_DEV_FORCE_MAC_PARAMS_REQ = 0xAC,
    zstackmsg_CmdIDs_ZDO_SET_BIND_UNBIND_AUTH_ADDR_REQ = 0xAD,
    zstackmsg_CmdIDs_BDB_ZCL_IDENTIFY_CMD_IND_REQ = 0xAE,
    zstackmsg_CmdIDs_BDB_SET_EP_DESC_LIST_ACTIVE_EP = 0xAF,
    zstackmsg_CmdIDs_BDB_START_COMMISSIONING_REQ = 0xB0,
    zstackmsg_CmdIDs_BDB_SET_IDENTIFY_ACTIVE_ENDPOINT_REQ = 0xB1,
    zstackmsg_CmdIDs_BDB_GET_IDENTIFY_ACTIVE_ENDPOINT_REQ = 0xB2,
    zstackmsg_CmdIDs_BDB_STOP_INITIATOR_FINDING_BINDING_REQ = 0xB3,
    zstackmsg_CmdIDs_GET_ZCL_FRAME_COUNTER_REQ = 0xB4,
    zstackmsg_CmdIDs_BDB_RESET_LOCAL_ACTION_REQ = 0xB5,
    zstackmsg_CmdIDs_BDB_SET_ATTRIBUTES_REQ = 0xB6,
    zstackmsg_CmdIDs_BDB_GET_ATTRIBUTES_REQ = 0xB7,
    zstackmsg_CmdIDs_BDB_GET_FB_INITIATOR_STATUS_REQ = 0xB8,
    zstackmsg_CmdIDs_BDB_GENERATE_INSTALL_CODE_CRC_REQ = 0xB9,
    zstackmsg_CmdIDs_BDB_REP_ADD_ATTR_CFG_RECORD_DEFAULT_TO_LIST_REQ = 0xBA,
    zstackmsg_CmdIDs_BDB_REP_CHANGED_ATTR_VALUE_REQ = 0xBB,
    zstackmsg_CmdIDs_BDB_ADD_INSTALL_CODE_REQ = 0xBC,
    zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_ENABLE_COMMISSIONING_REQ = 0xBD,
    zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_DISABLE_COMMISSIONING_REQ = 0xBE,
    zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_GETT_IMER_REQ = 0xBF,
    zstackmsg_CmdIDs_BDB_SET_ACTIVE_CENTRALIZED_LINK_KEY_REQ = 0xC0,
    zstackmsg_CmdIDs_BDB_CBKE_TC_LINK_KEY_EXCHANGE_ATTEMPT_REQ = 0xC1,
    zstackmsg_CmdIDs_BDB_TOUCHLINK_SET_ALLOW_STEALING_REQ = 0xC2,
    zstackmsg_CmdIDs_BDB_TOUCHLINK_GET_ALLOW_STEALING_REQ = 0xC3,
    zstackmsg_CmdIDs_BDB_ZED_ATTEMPT_RECOVER_NWK_REQ = 0xC4,
    zstackmsg_CmdIDs_BDB_NOTIFICATION = 0xC5,
    zstackmsg_CmdIDs_BDB_IDENTIFY_TIME_CB = 0xC6,
    zstackmsg_CmdIDs_BDB_BIND_NOTIFICATION_CB = 0xC7,
    zstackmsg_CmdIDs_BDB_TC_LINK_KEY_EXCHANGE_NOTIFICATION_IND = 0xC8,
    zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_ENABLE_IND = 0xC9,
    zstackmsg_CmdIDs_BDB_CBKE_TC_LINK_KEY_EXCHANGE_IND = 0xCA,
    zstackmsg_CmdIDs_BDB_FILTER_NWK_DESCRIPTOR_IND = 0xCB,
    zstackmsg_CmdIDs_BDB_FILTER_NWK_DESC_COMPLETE_REQ = 0xCD,
    zstackmsg_CmdIDs_BDB_NWK_DESC_FREE_REQ = 0xCE,
    zstackmsg_CmdIDs_BDB_PROCESS_IN_CONFIG_REPORT_REQ = 0xD7,
    zstackmsg_CmdIDs_BDB_PROCESS_IN_READ_REPORT_CFG_REQ = 0xD8,
    zstackmsg_CmdIDs_GP_COMMISSIONING_MODE_IND = 0xE0,
    zstackmsg_CmdIDs_GP_ALLOW_CHANNEL_CHANGE_REQ = 0xE1,
    zstackmsg_CmdIDs_GP_DATA_IND = 0xE2,
    zstackmsg_CmdIDs_GP_SECURITY_REQ = 0xE3,
    zstackmsg_CmdIDs_GP_SECURITY_RSP = 0xE4,
    zstackmsg_CmdIDs_GP_CHECK_ANNCE = 0xE5,
    zstackmsg_CmdIDs_GP_ADDRESS_CONFLICT = 0xE6,
    zstackmsg_CmdIDs_GP_CCM_STAR_DATA_IND = 0xE7,
    zstackmsg_CmdIDs_GP_CCM_STAR_COMMISSIONING_KEY = 0xE8,
    zstackmsg_CmdIDs_GP_COMMISSIONING_SUCCESS = 0xE9,
    zstackmsg_CmdIDs_GP_SEND_DEV_ANNOUNCE = 0xEA,
    zstackmsg_CmdIDs_SYS_NWK_FRAME_FWD_NOTIFICATION_IND = 0xCC,
    zstackmsg_CmdIDs_PAUSE_DEVICE_REQ = 0xEB,
    zstackmsg_CmdIDs_RESERVED_1A = 0x1A,
    zstackmsg_CmdIDs_RESERVED_23 = 0x23,
    zstackmsg_CmdIDs_RESERVED_24 = 0x24,
    zstackmsg_CmdIDs_RESERVED_25 = 0x25,
    zstackmsg_CmdIDs_RESERVED_26 = 0x26,
    zstackmsg_CmdIDs_RESERVED_27 = 0x27,
    zstackmsg_CmdIDs_RESERVED_31 = 0x31,
    zstackmsg_CmdIDs_RESERVED_32 = 0x32,
    zstackmsg_CmdIDs_RESERVED_33 = 0x33,
    zstackmsg_CmdIDs_RESERVED_34 = 0x34,
    zstackmsg_CmdIDs_RESERVED_35 = 0x35,
    zstackmsg_CmdIDs_RESERVED_36 = 0x36,
    zstackmsg_CmdIDs_RESERVED_D0 = 0xD0,
    zstackmsg_CmdIDs_RESERVED_D1 = 0xD1,
    zstackmsg_CmdIDs_RESERVED_D2 = 0xD2,
    zstackmsg_CmdIDs_RESERVED_D3 = 0xD3,
    zstackmsg_CmdIDs_RESERVED_D4 = 0xD4,
    zstackmsg_CmdIDs_RESERVED_D5 = 0xD5,
    zstackmsg_CmdIDs_RESERVED_D6 = 0xD6,
    zstackmsg_CmdIDs_SEC_MGR_APP_KEY_REQ = 0xF9,
    zstackmsg_CmdIDs_SEC_MGR_SETUP_PARTNER_REQ = 0xFA,
    zstackmsg_CmdIDs_SEC_MGR_APP_KEY_TYPE_SET_REQ = 0xFB,
    zstackmsg_CmdIDs_DEV_NWK_MANAGER_SET_REQ = 0xFC,
    zstackmsg_CmdIDs_RESERVED_FD = 0xFD,
    zstackmsg_CmdIDs_RESERVED_FE = 0xFE,
} zstack_CmdIDs;

//*****************************************************************************
// Structures - Building blocks for messages
//*****************************************************************************

/**
 * Event message header.
 * This is to support ICALL_CMD_EVENT
 */
typedef struct _zstackmsg_HDR_t
{
    /** event */
    uint8_t event;
    /** Will hold the default response status field. */
    uint8_t status;
#ifdef OSAL_PORT2TIRTOS
    uint8_t srcServiceTask;
#endif
} zstackmsg_HDR_t;

//*****************************************************************************
// System Interface Messages
//*****************************************************************************

/**
 * Generic structure to use receive an unknown message.
 */
typedef struct _zstackmsg_genericreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    void *pReq;

} zstackmsg_genericReq_t;

/**
 * Send this message to Reset the ZStack Thread.  The command ID for this
 * message is zstackmsg_CmdIDs_SYS_RESET_REQ.
 */
typedef struct _zstackmsg_sysresetreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_sysResetReq_t *pReq;

} zstackmsg_sysResetReq_t;

/**
 * Send this message to request the version information from the ZStack Thread.
 * The command ID for this message is zstackmsg_CmdIDs_SYS_VERSION_REQ.
 */
typedef struct _zstackmsg_sysversionreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no fields for this request */
    // void *pReq;
    /** Response fields (immediate response) */
    zstack_sysVersionRsp_t *pRsp;

} zstackmsg_sysVersionReq_t;

/**
 * Send this message to request the configuration information from the
 * ZStack Thread.  Each bool item is individually selected.
 * The command ID for this message is zstackmsg_CmdIDs_SYS_CONFIG_READ_REQ.
 */
typedef struct _zstackmsg_sysconfigreadreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_sysConfigReadReq_t *pReq;

    /**
     * Response fields (immediate response) -
     * The pUserDesc will be allocated if the has_userDesc == true and
     * n_userDesc is > 0, so the application must deallocate it with
     * OsalPort_free(pRsp->pUserDesc).<br>
     * The pPreConfigKey will be allocated if the has_preConfigKey == true,
     * so the application must deallocate it with OsalPort_free(pRsp->pPreConfigKey).
     */
    zstack_sysConfigReadRsp_t *pRsp;

} zstackmsg_sysConfigReadReq_t;

/**
 * Send this message to write configuration information to the ZStack Thread.
 * Since all of the items are optional, you can write one or many
 * configuration items.
 * The command ID for this message is zstackmsg_CmdIDs_SYS_CONFIG_WRITE_REQ.
 */
typedef struct _zstackmsg_sysconfigwritereq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_sysConfigWriteReq_t *pReq;

} zstackmsg_sysConfigWriteReq_t;

/**
 * Send this message to set the TX Power on the ZStack Thread.
 * The command ID for this message is zstackmsg_CmdIDs_SYS_SET_TX_POWER_REQ.
 */
typedef struct _zstackmsg_syssettxpowerreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_sysSetTxPowerReq_t *pReq;

    /** Response fields (immediate response) */
    zstack_sysSetTxPowerRsp_t *pRsp;

} zstackmsg_sysSetTxPowerReq_t;

/**
 * Send this message to request the Network Information the ZStack Thread.
 * The command ID for this message is zstackmsg_CmdIDs_SYS_NWK_INFO_READ_REQ.
 */
typedef struct _zstackmsg_sysnwkinforeadreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no fields for this request */
    // zstack_sysNwkInfoReadReq_t *pReq;
    /** Response fields (immediate response) */
    zstack_sysNwkInfoReadRsp_t *pRsp;

} zstackmsg_sysNwkInfoReadReq_t;

/**
 * Send this message to request the version information from the ZStack Thread.
 * The command ID for this message is zstackmsg_CmdIDs_SYS_FORCE_LINK_STATUS_REQ.
 */
typedef struct _zstackmsg_sysforcelinkstatusreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no fields for this request */
    // void *pReq;
} zstackmsg_sysForceLinkStatusReq_t;

/**
 * Send this message to send MT Sys App message from the ZStack Thread.
 * The command ID for this message is zstackmsg_CmdIDs_SYS_APP_MSG_REQ.
 */
typedef struct _zstackmsg_sysappmsg_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_sysAppMsg_t Req;

} zstackmsg_sysAppMsg_t;

/**
 * Send this message to send MT Sys OTA message from the ZStack Thread.
 * The command ID for this message is zstackmsg_CmdIDs_SYS_APP_MSG_REQ.
 */
typedef struct _zstackmsg_sysotamsg_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_sysOtaMsg_t Req;

} zstackmsg_sysOtaMsg_t;

//*****************************************************************************
// Device Interface Messages
//*****************************************************************************

/**
 * Send this message to the ZStack Thread to request it to start.
 * The command ID for this message is zstackmsg_CmdIDs_DEV_START_REQ.
 */
typedef struct _zstackmsg_devstartreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_devStartReq_t *pReq;

} zstackmsg_devStartReq_t;

/**
 * Send this message to the ZStack Thread to initiate a network discovery.
 * The command ID for this message is zstackmsg_CmdIDs_DEV_NWK_DISCOVERY_REQ.
 */
typedef struct _zstackmsg_devnwkdiscreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_devNwkDiscReq_t *pReq;

} zstackmsg_devNwkDiscReq_t;

/**
 * Send this message to the ZStack Thread to enable/disable notifications on
 * frames being forwarded by this local device.
 * The command ID for this message is zstackmsg_CmdIDs_SYS_NWK_FRAME_FWD_NOTIFICATION_REQ.
 */
typedef struct _zstackmsg_setnwkframefwdnotificationreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_setNwkFrameFwdNotificationReq_t *pReq;

} zstackmsg_setNwkFrameFwdNotificationReq_t;


/**
 * Send this message to the ZStack Thread to initiate a network join.
 * The command ID for this message is zstackmsg_CmdIDs_DEV_JOIN_REQ.
 */
typedef struct _zstackmsg_devjoinreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_devJoinReq_t *pReq;

} zstackmsg_devJoinReq_t;

/**
 * Send this message to the ZStack Thread to initiate a network rejoin.
 * The command ID for this message is zstackmsg_CmdIDs_DEV_REJOIN_REQ.
 */
typedef struct _zstackmsg_devrejoinreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_devRejoinReq_t *pReq;

} zstackmsg_devRejoinReq_t;

/**
 * Send this message to the ZStack Thread to subscribe to one or more ZDO
 * callback indications, confirmations, or response messages.
 * The command ID for this message is zstackmsg_CmdIDs_DEV_ZDO_CBS_REQ.
 */
typedef struct _zstackmsg_devzdocbreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_devZDOCBReq_t *pReq;

} zstackmsg_devZDOCBReq_t;

/**
 * Send this message to the ZStack Thread to initiate a route request.
 * The command ID for this message is zstackmsg_CmdIDs_DEV_NWK_ROUTE_REQ.
 */
typedef struct _zstackmsg_devnwkroutereq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_devNwkRouteReq_t *pReq;

} zstackmsg_devNwkRouteReq_t;

/**
 * Send this message to the ZStack Thread to check a route request.
 * The command ID for this message is zstackmsg_CmdIDs_DEV_NWK_CHECK_ROUTE_REQ.
 */
typedef struct _zstackmsg_devnwkcheckroutereq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_devNwkCheckRouteReq_t *pReq;

} zstackmsg_devNwkCheckRouteReq_t;

/**
 * This message is sent from ZStack Thread whenever the Permit Join state
 * changes (Off to on, on to off).
 * The command ID for this message is zstackmsg_CmdIDs_DEV_PERMIT_JOIN_IND.
 */
typedef struct _zstackmsg_devpermitjoinind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_devPermitJoinInd_t Req;

} zstackmsg_devPermitJoinInd_t;

/**
 * Send this message to the ZStack Thread to Update a neighbor's txCost. If the
 * neighbor exists (in the neighbor table), it will update the cost, if it doesn't
 * exist, one will be created.  This function will only exist on routers.  Also,
 * you shouldn't call this function to add end devices to the neighbor table.
 * The command ID for this message is zstackmsg_CmdIDs_DEV_UPDATE_NEIGHBOR_TXCOST_REQ.
 */
typedef struct _zstackmsg_devupdateneighbortxcostreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_devUpdateNeighborTxCostReq_t *pReq;

} zstackmsg_devUpdateNeighborTxCostReq_t;

/**
 * Send this message to the ZStack Thread to Force Network Settings. DON'T USE
 * this function unless you know exactly what you are doing and can live with
 * the unpredictable consequences.  When this message is received, the ZStack
 * thread will force the values in the NIB then save the NIB.  It would be
 * better to let the ZStack thread set these items as they are determined.
 * The command ID for this message is
 * zstackmsg_CmdIDs_DEV_FORCE_NETWORK_SETTINGS_REQ.
 */
typedef struct _zstackmsg_devforcenetworksettingsreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_devForceNetworkSettingsReq_t *pReq;

} zstackmsg_devForceNetworkSettingsReq_t;

/**
 * Send this message to the ZStack Thread to Force Network Update. DON'T USE
 * this function unless you know exactly what you are doing and can live with
 * the unpredictable consequences.  When this message is received, the ZStack
 * thread will force the values in the NIB then save the NIB.  It would be
 * better to let the ZStack thread set these items as they are determined.
 * The command ID for this message is
 * zstackmsg_CmdIDs_DEV_FORCE_NETWORK_UPDATE_REQ.
 */
typedef struct _zstackmsg_devforcenetworkupdatereq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_devForceNetworkUpdateReq_t *pReq;

} zstackmsg_devForceNetworkUpdateReq_t;

/**
 * Send this message to the ZStack Thread to Force MAC Parameters. DON'T USE
 * this function unless you know exactly what you are doing and can live with
 * the unpredictable consequences.  When this message is received, the ZStack
 * thread will force the values in the MAC.  It would be better to let the
 * ZStack thread set these items as they are determined.
 * The command ID for this message is
 * zstackmsg_CmdIDs_DEV_FORCE_MAC_PARAMS_REQ.
 */
typedef struct _zstackmsg_devforcemacparamsreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_devForceMacParamsReq_t *pReq;

} zstackmsg_devForceMacParamsReq_t;

//*****************************************************************************
// APS Interface Request Messages
//*****************************************************************************

/**
 * Send this message to the ZStack Thread to remove an APS group.
 * The command ID for this message is zstackmsg_CmdIDs_APS_REMOVE_GROUP.
 */
typedef struct _zstackmsg_apsremovegroup_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_apsRemoveGroup_t *pReq;

} zstackmsg_apsRemoveGroup_t;

/**
 * Send this message to the ZStack Thread to remove all APS group.
 * The command ID for this message is zstackmsg_CmdIDs_APS_REMOVE_ALL_GROUPS.
 */
typedef struct _zstackmsg_apsremoveallgroups_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_apsRemoveAllGroups_t *pReq;

} zstackmsg_apsRemoveAllGroups_t;

/**
 * Send this message to the ZStack Thread to find all APS groups.
 * The command ID for this message is zstackmsg_CmdIDs_APS_FIND_ALL_GROUPS_REQ.
 */
typedef struct _zstackmsg_apsfindallgroupsreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_apsFindAllGroupsReq_t *pReq;

    /** Response fields (immediate response) */
    zstack_apsFindAllGroupsRsp_t *pRsp;

} zstackmsg_apsFindAllGroupsReq_t;

/**
 * Send this message to the ZStack Thread to find an APS group for a
 * given endpoint and group ID.
 * The command ID for this message is zstackmsg_CmdIDs_APS_FIND_GROUP_REQ.
 */
typedef struct _zstackmsg_apsfindgroupreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_apsFindGroupReq_t *pReq;

    /** Response fields (immediate response) */
    zstack_apsFindGroupRsp_t *pRsp;

} zstackmsg_apsFindGroupReq_t;

/**
 * Send this message to the ZStack Thread to add a group for a given endpoint.
 * The command ID for this message is zstackmsg_CmdIDs_APS_ADD_GROUP.
 */
typedef struct _zstackmsg_apsaddgroup_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_apsAddGroup_t *pReq;

} zstackmsg_apsAddGroup_t;

/**
 * Send this message to the ZStack Thread to count all APS groups for a given
 * endpoint.
 * The command ID for this message is zstackmsg_CmdIDs_APS_COUNT_ALL_GROUPS.
 */
typedef struct _zstackmsg_apscountallgroups_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no command fields, NULL field */
    void *pReq;

} zstackmsg_apsCountAllGroups_t;

//*****************************************************************************
// AF Interface Request Messages
//*****************************************************************************

/**
 * Send this message to the ZStack Thread to register an AF Enpoint
 * (including Simple Descriptor).
 * The command ID for this message is zstackmsg_CmdIDs_AF_REGISTER_REQ.
 */
typedef struct _zstackmsg_afregisterreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_afRegisterReq_t *pReq;

} zstackmsg_afRegisterReq_t;

/**
 * Send this message to the ZStack Thread to unregister an AF Enpoint.
 * The command ID for this message is zstackmsg_CmdIDs_AF_UNREGISTER_REQ.
 */
typedef struct _zstackmsg_afunregisterreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_afUnRegisterReq_t *pReq;

} zstackmsg_afUnRegisterReq_t;

/**
 * Send this message to the ZStack Thread to send a data message from an
 * endpoint.
 * The command ID for this message is zstackmsg_CmdIDs_AF_DATA_REQ.
 */
typedef struct _zstackmsg_afdatareq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_afDataReq_t *pReq;

} zstackmsg_afDataReq_t;

/**
 * Send this message to the ZStack Thread to setup the Inter-PAN controller.
 * The command ID for this message is zstackmsg_CmdIDs_AF_INTERPAN_CTL_REQ.
 */
typedef struct _zstackmsg_afinterpanctlreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_afInterPanCtlReq_t *pReq;

} zstackmsg_afInterPanCtlReq_t;

/**
 * Send this message to the ZStack Thread to get the
 * AF/APS Fragmentation parameters.
 * The command ID for this message is zstackmsg_CmdIDs_AF_CONFIG_GET_REQ.
 */
typedef struct _zstackmsg_afconfiggetreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_afConfigGetReq_t *pReq;

    /** Response fields (immediate response) */
    zstack_afConfigGetRsp_t *pRsp;

} zstackmsg_afConfigGetReq_t;

/**
 * Send this message to the ZStack Thread to set the
 * AF/APS Fragmentation parameters.
 * The command ID for this message is zstackmsg_CmdIDs_AF_CONFIG_SET_REQ.
 */
typedef struct _zstackmsg_afconfigsetreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_afConfigSetReq_t *pReq;

} zstackmsg_afConfigSetReq_t;

//*****************************************************************************
// AF Interface Indication Structures
//*****************************************************************************

/**
 * This message is sent from ZStack Thread to indicate that the message
 * (zstackmsg_afDataReq) was sent or not sent.
 * The command ID for this message is zstackmsg_CmdIDs_AF_DATA_CONFIRM_IND.
 */
typedef struct _zstackmsg_afdataconfirmind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_afDataConfirmInd_t req;

} zstackmsg_afDataConfirmInd_t;

/**
 * This message is sent from ZStack Thread to indicate an incoming
 * endpoint data message.
 * The command ID for this message is zstackmsg_CmdIDs_AF_INCOMING_MSG_IND.
 */
typedef struct _zstackmsg_afincomingmsgind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_afIncomingMsgInd_t req;

} zstackmsg_afIncomingMsgInd_t;

/**
 * This message is sent from ZStack Thread to indicate a reflection error.
 * The command ID for this message is zstackmsg_CmdIDs_AF_REFLECT_ERROR_IND.
 */
typedef struct _zstackmsg_afreflecterrorind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_afReflectErrorInd_t req;

} zstackmsg_afReflectErrorInd_t;

//*****************************************************************************
// ZDO Interface Request Structures
//*****************************************************************************

/**
 * Send this message to the ZStack Thread to send a broadcast
 * ZDO Network Address Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_NWK_ADDR_REQ.
 * When successful, the responding device will send back an asynchronous
 * ZStackmsg_zdoNwkAddrRspInd_t.
 */
typedef struct _zstackmsg_zdonwkaddrreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoNwkAddrReq_t *pReq;

} zstackmsg_zdoNwkAddrReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast
 * ZDO IEEE Address Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_IEEE_ADDR_REQ.
 * When successful, the responding device will send back an
 * asynchronous zstackmsg_zdoIeeeAddrRspInd_t.
 */
typedef struct _zstackmsg_zdoieeeaddrreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoIeeeAddrReq_t *pReq;

} zstackmsg_zdoIeeeAddrReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast
 * ZDO Node Descriptor Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_NODE_DESC_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoNodeDescRspInd_t.
 */
typedef struct _zstackmsg_zdonodedescreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoNodeDescReq_t *pReq;

} zstackmsg_zdoNodeDescReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast
 * ZDO Power Descriptor Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_POWER_DESC_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoPowerDescRspInd_t.
 */
typedef struct _zstackmsg_zdopowerdescreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoPowerDescReq_t *pReq;

} zstackmsg_zdoPowerDescReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast
 * ZDO Simple Descriptor Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_SIMPLE_DESC_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoSimpleDescRspInd_t.
 */
typedef struct _zstackmsg_zdosimpledescreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoSimpleDescReq_t *pReq;

} zstackmsg_zdoSimpleDescReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast
 * ZDO Active Endpoint Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_ACTIVE_ENDPOINT_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoActiveEndpointRspInd_t.
 */
typedef struct _zstackmsg_zdoactiveendpointreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoActiveEndpointReq_t *pReq;

} zstackmsg_zdoActiveEndpointReq_t;

/**
 * Send this message to the ZStack Thread to send a ZDO Match Descriptor Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MATCH_DESC_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoMatchDescRspInd_t.
 */
typedef struct _zstackmsg_zdomatchdescreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMatchDescReq_t *pReq;

} zstackmsg_zdoMatchDescReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast
 * ZDO Complex Descriptor Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_COMPLEX_DESC_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoComplexDescRspInd_t.
 */
typedef struct _zstackmsg_zdocomplexdescreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoComplexDescReq_t *pReq;

} zstackmsg_zdoComplexDescReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast
 * ZDO User Descriptor Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_USER_DESC_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoUserDescRspInd_t.
 */
typedef struct _zstackmsg_zdouserdescreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoUserDescReq_t *pReq;

} zstackmsg_zdoUserDescReq_t;


/**
 * Send this message to the ZStack Thread to make the stack notify
 * of a remote Bind or Unbind request and allow the application to
 * authenticate the request, if the request is authenticated by the
 * application it hast to add the bind manually using. The notifications
 * from the stack will come in zstackmsg_CmdIDs_ZDO_BIND_UNBIND_APP_AUTH_IND.
 */
typedef struct _zstackmsg_zdosetbindunbindappauthreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoSetBindUnbindAuthAddr_t *pReq;

} zstackmsg_zdoSetBindUnbindAuthAddrReq_t;


/**
 * Send this message to the ZStack Thread to send a broadcast
 * ZDO Device Announce.<BR>
 * This command contains the device?s new 16-bit NWK address and its
 * 64-bit IEEE address, as well as the capabilities of the ZigBee device.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE_REQ.
 */
typedef struct _zstackmsg_zdodeviceannouncereq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoDeviceAnnounceReq_t *pReq;

} zstackmsg_zdoDeviceAnnounceReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast
 * ZDO Set User Descriptor Request.  The User Descriptor is a text string
 * describing the device in the network (ex. "Porch Light" ).
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_USER_DESCR_SET_REQ.
 */
typedef struct _zstackmsg_zdouserdescsetreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoUserDescSetReq_t *pReq;

} zstackmsg_zdoUserDescSetReq_t;

/**
 * Send this message to the ZStack Thread to send a
 * ZDO Server Discovery Request broadcast
 * to all RxOnWhenIdle devices.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_SERVER_DISC_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoServerDiscRspInd_t.
 */
typedef struct _zstackmsg_zdoserverdiscreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoServerDiscReq_t *pReq;

} zstackmsg_zdoServerDiscReq_t;

/**
 * Send this message to the ZStack Thread to send a ZDO End Device Bind Request
 * to the coordinator.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_END_DEVICE_BIND_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoEndDeviceBindRspInd_t.
 */
typedef struct _zstackmsg_zdoenddevicebindreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoEndDeviceBindReq_t *pReq;

} zstackmsg_zdoEndDeviceBindReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast ZDO Bind Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_BIND_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoBindRspInd_t.
 */
typedef struct _zstackmsg_zdobindreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoBindReq_t *pReq;

} zstackmsg_zdoBindReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast ZDO Unbind Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_UNBIND_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoUnbindRspInd_t.
 */
typedef struct _zstackmsg_zdounbindreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoUnbindReq_t *pReq;

} zstackmsg_zdoUnbindReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast
 * ZDO Management Network Discovery Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_NWK_DISC_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoMgmtNwkDiscRspInd_t.
 */
typedef struct _zstackmsg_zdomgmtnwkdiscreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtNwkDiscReq_t *pReq;

} zstackmsg_zdoMgmtNwkDiscReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast
 * ZDO Management LQI Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_LQI_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoMgmtLqiRspInd_t.
 */
typedef struct _zstackmsg_zdomgmtlqireq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtLqiReq_t *pReq;

} zstackmsg_zdoMgmtLqiReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast
 * ZDO Management Routing Table Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_RTG_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoMgmtRtgRspInd_t.
 */
typedef struct _zstackmsg_zdomgmtrtgreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtRtgReq_t *pReq;

} zstackmsg_zdoMgmtRtgReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast
 * ZDO Management Binding Table Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_BIND_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoMgmtBindRspInd_t.
 */
typedef struct _zstackmsg_zdomgmtbindreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtBindReq_t *pReq;

} zstackmsg_zdoMgmtBindReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast
 * ZDO Management Leave Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_LEAVE_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoMgmtLeaveRspInd_t.
 */
typedef struct _zstackmsg_zdomgmtleavereq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtLeaveReq_t *pReq;

} zstackmsg_zdoMgmtLeaveReq_t;

/**
 * Send this message to the ZStack Thread to send a unicast
 * ZDO Management Direct Join Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_DIRECT_JOIN_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoMgmtDirectJoinRspInd_t.
 */
typedef struct _zstackmsg_zdomgmtdirectjoinreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtDirectJoinReq_t *pReq;

} zstackmsg_zdoMgmtDirectJoinReq_t;

/**
 * Send this message to the ZStack Thread to send a
 * ZDO Management Permit Join Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_PERMIT_JOIN_REQ.
 * When successful, the responding device will send back an asynchronous
 * zstackmsg_zdoMgmtPermitJoinRspInd_t.
 */
typedef struct _zstackmsg_zdomgmtpermitjoinreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtPermitJoinReq_t *pReq;

} zstackmsg_zdoMgmtPermitJoinReq_t;

/**
 * Send this message to the ZStack Thread to send a ZDO Management
 * Network Update Request.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_NWK_UPDATE_REQ.
 */
typedef struct _zstackmsg_zdomgmtnwkupdatereq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtNwkUpdateReq_t *pReq;

} zstackmsg_zdoMgmtNwkUpdateReq_t;

//*****************************************************************************
// ZDO Interface Response Structures
//*****************************************************************************

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Network Address Response message is received.<BR>
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_NWK_ADDR_RSP.<BR>
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoNwkAddrReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdonwkaddrrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoNwkAddrRspInd_t rsp;

} zstackmsg_zdoNwkAddrRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO IEEE Address Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_IEEE_ADDR_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoIeeeAddrReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdoieeeaddrrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoIeeeAddrRspInd_t rsp;

} zstackmsg_zdoIeeeAddrRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Node Descriptor Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_NODE_DESC_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoNodeDescReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdonodedescrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoNodeDescRspInd_t rsp;

} zstackmsg_zdoNodeDescRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Power Descriptor Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_POWER_DESC_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoPowerDescReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdopowerdescrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoPowerDescRspInd_t rsp;

} zstackmsg_zdoPowerDescRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Simple Descriptor Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_SIMPLE_DESC_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoSimpleDescReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdosimpledescrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoSimpleDescRspInd_t rsp;

} zstackmsg_zdoSimpleDescRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Active Endpoints Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_ACTIVE_EP_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoActiveEndpointsReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdoactiveendpointsrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoActiveEndpointsRspInd_t rsp;

} zstackmsg_zdoActiveEndpointsRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Match Descriptor Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MATCH_DESC_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoMatchDescReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdomatchdescrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMatchDescRspInd_t rsp;

} zstackmsg_zdoMatchDescRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO User Descriptor Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_USER_DESC_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoUserDescReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdouserdescrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoUserDescRspInd_t rsp;

} zstackmsg_zdoUserDescRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Server Discovery Descriptor Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_SERVER_DISC_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoServerDiscoveryReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdoserverdiscoveryrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoServerDiscoveryRspInd_t rsp;

} zstackmsg_zdoServerDiscoveryRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Bind Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_BIND_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoBindReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdobindrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoBindRspInd_t rsp;

} zstackmsg_zdoBindRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO End Device Bind Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_END_DEVICE_BIND_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoEndDeviceBindReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdoenddevicebindrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoEndDeviceBindRspInd_t rsp;

} zstackmsg_zdoEndDeviceBindRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Unbind Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_UNBIND_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoUnbindReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdounbindrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoUnbindRspInd_t rsp;

} zstackmsg_zdoUnbindRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Device Announce message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdodeviceannounceind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoDeviceAnnounceInd_t req;

} zstackmsg_zdoDeviceAnnounceInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Management Network Discovery Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_NWK_DISC_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoMgmtNwkDiscReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdomgmtnwkdiscrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtNwkDiscRspInd_t rsp;

} zstackmsg_zdoMgmtNwkDiscRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Management LQI Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_LQI_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoMgmtLqiReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdomgmtlqirspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtLqiRspInd_t rsp;

} zstackmsg_zdoMgmtLqiRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Management Routing Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_RTG_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoMgmtRtgReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdomgmtrtgrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtRtgRspInd_t rsp;

} zstackmsg_zdoMgmtRtgRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Management Binding Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_BIND_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoMgmtBindReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdomgmtbindrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtBindRspInd_t rsp;

} zstackmsg_zdoMgmtBindRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Management Leave Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_LEAVE_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoMgmtLeaveReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdomgmtleaverspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtLeaveRspInd_t rsp;

} zstackmsg_zdoMgmtLeaveRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Management Direct Join Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_DIRECT_JOIN_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoMgmtDirectJoinReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdomgmtdirectjoinrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtDirectJoinRspInd_t rsp;

} zstackmsg_zdoMgmtDirectJoinRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Management Permit Join Response message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_PERMIT_JOIN_RSP.
 * The source device sent this message in response to this device sending
 * zstackmsg_zdoMgmtPermitJoinReq_t.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdomgmtpermitjoinrspind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtPermitJoinRspInd_t rsp;

} zstackmsg_zdoMgmtPermitJoinRspInd_t;

/**
 * This response indication message is sent from ZStack Thread when a
 * ZDO Management Network Update Notify message is received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_MGMT_NWK_UPDATE_NOTIFY.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdomgmtnwkupdatenotifyind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoMgmtNwkUpdateNotifyInd_t rsp;

} zstackmsg_zdoMgmtNwkUpdateNotifyInd_t;

/**
 * This response indication message is sent from ZStack Thread whenever the
 * Trust Center allows a device to join the network.  This message is will
 * only occur on a coordinator/trust center device.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_TC_DEVICE_IND.
 * <BR>
 * To receive this message, in your application, you must subscribe for it with
 * a call to Zstackapi_DevZDOCBReq().
 */
typedef struct _zstackmsg_zdotcdeviceind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoTcDeviceInd_t req;

} zstackmsg_zdoTcDeviceInd_t;

//*****************************************************************************
// ZDO Interface Confirm/Indication Structures
//*****************************************************************************

/**
 * This message is sent from ZStack Thread to indicate that a source route
 * was received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_SRC_RTG_IND.
 */
typedef struct _zstackmsg_zdosrcrtgind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoSrcRtgInd_t req;

} zstackmsg_zdoSrcRtgInd_t;

/**
 * This message is sent from ZStack Thread to indicate that concentrator
 * indication was received.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_CONCENTRATOR_IND.
 */
typedef struct _zstackmsg_zdocncntrtrind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoCncntrtrInd_t req;

} zstackmsg_zdoCncntrtrInd_t;

/**
 * This message is sent from ZStack Thread to indicate a
 * ZDO Network Discovery Confirmation.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_NWK_DISC_CNF.
 */
typedef struct _zstackmsg_zdonwkdisccnf_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoNwkDiscCnf_t req;

} zstackmsg_zdoNwkDiscCnf_t;

/**
 * This message is sent from ZStack Thread to indicate a beacon notification.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_BEACON_NOTIFY_IND.
 */
typedef struct _zstackmsg_zdobeaconnotifyind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoBeaconNotifyInd_t req;

} zstackmsg_zdoBeaconNotifyInd_t;

/**
 * This message is sent from ZStack Thread to indicate a join confirmation.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_JOIN_CNF.
 */
typedef struct _zstackmsg_zdojoinconf_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoJoinConf_t req;

} zstackmsg_zdoJoinConf_t;

/**
 * This message is sent from ZStack Thread to indicate a leave confirmation.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_LEAVE_CNF.
 */
typedef struct _zstackmsg_zdoleavecnf_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoLeaveCnf_t req;

} zstackmsg_zdoLeaveCnf_t;

/**
 * This message is sent from ZStack Thread to indicate a leave.
 * The command ID for this message is zstackmsg_CmdIDs_ZDO_LEAVE_IND.
 */
typedef struct _zstackmsg_zdoleaveind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_zdoLeaveInd_t req;

} zstackmsg_zdoLeaveInd_t;


/**
 * This message is sent from ZStack Thread to notify about a frame being forwared.
 * This can be used to analyze the nwk traffic. This is enabled with a request to the stack
 * with Zstackapi_SetNwkFrameFwdNotificationReq()
 * The command ID for this message is zstackmsg_CmdIDs_SYS_NWK_FRAME_FWD_NOTIFICATION_IND.
 */
typedef struct _zstackmsg_nwkframefwdnotification_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_nwkFrameFwdNotification_t req;

} zstackmsg_nwkFrameFwdNotification_t;




//*****************************************************************************
// Device Interface Indication Structures
//*****************************************************************************

/**
 * This message is sent from ZStack Thread whenever a change in state occurs.
 * The command ID for this message is zstackmsg_CmdIDs_DEV_STATE_CHANGE_IND.
 */
typedef struct _zstackmsg_devstatechangeind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_devStateChangeInd_t req;

} zstackmsg_devStateChangeInd_t;

//*****************************************************************************
// Security Interface Request Structures
//*****************************************************************************

/**
 * Send this message to the ZStack Thread to retrieve a network key.
 * The command ID for this message is zstackmsg_CmdIDs_SEC_NWK_KEY_GET_REQ.
 */
typedef struct _zstackmsg_secnwkkeygetreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_secNwkKeyGetReq_t *pReq;

    /** Response fields (immediate response) */
    zstack_secNwkKeyGetRsp_t *pRsp;

} zstackmsg_secNwkKeyGetReq_t;

/**
 * Send this message to the ZStack Thread to set a network key.
 * The command ID for this message is zstackmsg_CmdIDs_SEC_NWK_KEY_SET_REQ.
 */
typedef struct _zstackmsg_secnwkkeysetreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_secNwkKeySetReq_t *pReq;

} zstackmsg_secNwkKeySetReq_t;

/**
 * Send this message to the ZStack Thread to update a key in the network.
 * The command ID for this message is zstackmsg_CmdIDs_SEC_NWK_KEY_UPDATE_REQ.
 */
typedef struct _zstackmsg_secnwkkeyupdatereq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_secNwkKeyUpdateReq_t *pReq;

} zstackmsg_secNwkKeyUpdateReq_t;

/**
 * Send this message to the ZStack Thread to switch a key in the network.
 * The command ID for this message is zstackmsg_CmdIDs_SEC_NWK_KEY_SWITCH_REQ.
 */
typedef struct _zstackmsg_secnwkkeyswitchReq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_secNwkKeySwitchReq_t *pReq;

} zstackmsg_secNwkKeySwitchReq_t;

/**
 * Send this message to the ZStack Thread to retrieve an APS Link
 * Key (including TC Link Key).
 * The command ID for this message is zstackmsg_CmdIDs_SEC_APS_LINKKEY_GET_REQ.
 */
typedef struct _zstackmsg_secapslinkkeygetreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_secApsLinkKeyGetReq_t *pReq;

    /** Response fields (immediate response) */
    zstack_secApsLinkKeyGetRsp_t *pRsp;

} zstackmsg_secApsLinkKeyGetReq_t;

/**
 * Send this message to the ZStack Thread to set an APS Link
 * Key (including TC Link Key).
 * The command ID for this message is zstackmsg_CmdIDs_SEC_APS_LINKKEY_SET_REQ.
 */
typedef struct _zstackmsg_secapslinkkeysetreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_secApsLinkKeySetReq_t *pReq;

} zstackmsg_secApsLinkKeySetReq_t;

/**
 * Send this message to the ZStack Thread to remove an APS Link
 * Key (including TC Link Key).
 * The command ID for this message is zstackmsg_CmdIDs_SEC_APS_LINKKEY_REMOVE_REQ.
 */
typedef struct _zstackmsg_secapslinkkeyremovereq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_secApsLinkKeyRemoveReq_t *pReq;

} zstackmsg_secApsLinkKeyRemoveReq_t;

/**
 * Send this message to the ZStack Thread to issue an APS Remove
 * Command (Trust Center Only).
 * The command ID for this message is zstackmsg_CmdIDs_SEC_APS_REMOVE_REQ.
 */
typedef struct _zstackmsg_secapsremovereq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_secApsRemoveReq_t *pReq;

} zstackmsg_secApsRemoveReq_t;

/**
 * Send this message to the ZStack Thread to issue a Setup Partner
 * Command (ZR Only).
 * The command ID for this message is zstackmsg_CmdIDs_SEC_MGR_SETUP_PARTNER_REQ.
 */
typedef struct _zstackmsg_secmgrsetuppartnerreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_secMgrSetupPartnerReq_t *pReq;

} zstackmsg_secMgrSetupPartnerReq_t;

/**
 * Send this message to the ZStack Thread to issue a App Key Req
 * Command
 * The command ID for this message is zstackmsg_CmdIDs_SEC_MGR_APP_KEY_REQ.
 */
typedef struct _zstackmsg_secmgrappkeyreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_secMgrAppKeyReq_t *pReq;

} zstackmsg_secMgrAppKeyReq_t;

/**
 * Send this message to the ZStack Thread to issue a App Key Type Set
 * Command (ZR Only).
 * The command ID for this message is zstackmsg_CmdIDs_SEC_MGR_APP_KEY_TYPE_SET_REQ.
 */
typedef struct _zstackmsg_secmgrappkeytypesetreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_secMgrAppKeyTypeSetReq_t *pReq;

} zstackmsg_secMgrAppKeyTypeSetReq_t;

/**
 * Send this message to the ZStack Thread to issue set the device as NWK_MANAGER
 * Command
 * The command ID for this message is zstackmsg_CmdIDs_DEV_NWK_MANAGER_SET_REQ.
 */
typedef struct _zstackmsg_devnwkmanagersetreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

} zstackmsg_devNwkManagerSetReq_t;

/**
 * Send this message to the ZStack Thread to start a commissioning mode with which
 * the a network can be created or joined, or a discovery with matching clusters
 * can be found.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_START_COMMISSIONING_REQ.
 */
typedef struct _zstackmsg_bdbstartcommissioning_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbStartCommissioningReq_t *pReq;

} zstackmsg_bdbStartCommissioningReq_t;

/**
 * Send this message to the ZStack Thread to set the endpoint from which Finding
 * and Binding commissioning mode will be triggered.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_SET_IDENTIFY_ACTIVE_ENDPOINT_REQ.
 */
typedef struct _zstackmsg_bdbsetidentifyactiveendpoint_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbSetIdentifyActiveEndpointReq_t *pReq;

} zstackmsg_bdbSetIdentifyActiveEndpointReq_t;

/**
 * Send this message to the ZStack Thread to get the endpoint from which Finding
 * and binding commissioning mode would be triggered.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_GET_IDENTIFY_ACTIVE_ENDPOINT_REQ.
 */
typedef struct _zstackmsg_bdbgetidentifyactiveendpoint_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no command fields, NULL field */
    void *pReq;

    zstack_bdbGetIdentifyActiveEndpointRsp_t *pRsp;

} zstackmsg_bdbGetIdentifyActiveEndpointReq_t;

/**
 * Send this message to the ZStack Thread to stop Finding and Binding Initiator process.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_STOP_INITIATOR_FINDING_BINDING_REQ.
 */
typedef struct _zstackmsg_bdbstopinitiatorfindingbinding_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no command fields, NULL field */
    void *pReq;

} zstackmsg_bdbStopInitiatorFindingBindingReq_t;

/**
 * Send this message to the ZStack Thread to get the next value of the ZCL frame counter.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_GET_ZCL_FRAME_COUNTER_REQ.
 */
typedef struct _zstackmsg_getzclframecounter_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no command fields, NULL field */
    void *pReq;

    zstack_getZCLFrameCounterRsp_t *pRsp;

} zstackmsg_getZCLFrameCounterReq_t;

/**
 * Send this message to the ZStack Thread to allow BDB Stack process Identify command received.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_ZCL_IDENTIFY_CMD_IND_REQ.
 */
typedef struct _zstackmsg_bdbzclidentifycmdindreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbZCLIdentifyCmdIndReq_t *pReq;

} zstackmsg_bdbZCLIdentifyCmdIndReq_t;

/**
 * Send this message to the ZStack Thread to Set the endpoint list to the active endpoint selected by the application for F&B process
 * The command ID for this message is zstackmsg_CmdIDs_BDB_SET_EP_DESC_LIST_ACTIVE_EP.
 */
typedef struct _zstackmsg_bdbsetepdesclisttoactiveendpoint_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no command fields, NULL field */
    void *pReq;

    /** Message command fields */
    zstack_bdbSetEpDescListToActiveEndpointRsp_t *pRsp;

} zstackmsg_bdbSetEpDescListToActiveEndpointReq_t;

/**
 * Send this message to the ZStack Thread to request the stack to perform a factory new reset. This will erase all Zigbee network parameters but WILL NOT ERASE Application data, that is up to the application.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_RESET_LOCAL_ACTION_REQ.
 */
typedef struct _zstackmsg_bdbresetlocalaction_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no command fields, NULL field */
    void *pReq;

} zstackmsg_bdbResetLocalActionReq_t;

/**
 * Send this message to the ZStack Thread to set BDB attributes that will be
 * using during commissioning process.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_SET_ATTRIBUTES_REQ.
 */
typedef struct _zstackmsg_bdbsetattributes_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbSetAttributesReq_t *pReq;

} zstackmsg_bdbSetAttributesReq_t;

/**
 * Send this message to the ZStack Thread to get BDB attributes that will be
 * using during commissioning process.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_GET_ATTRIBUTES_REQ.
 */
typedef struct _zstackmsg_bdbgetattributes_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no command fields, NULL field */
    void *pReq;

    /** Message command fields */
    zstack_bdbGetAttributesRsp_t *pRsp;

} zstackmsg_bdbGetAttributesReq_t;

/**
 * Send this message to the ZStack Thread to get the remaining time for Finding and
 * Binding as intitator process and the number of attempts left to find matching cluster devices.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_GET_FB_INITIATOR_STATUS_REQ.
 */
typedef struct _zstackmsg_bdbgetfbinitiatorstatus_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no command fields, NULL field */
    void *pReq;

    /** Message command fields */
    zstack_bdbGetFBInitiatorStatusRsp_t *pRsp;

} zstackmsg_bdbGetFBInitiatorStatusReq_t;

/**
 * Send this message to the ZStack Thread to request the stack to generate the CRC from an install code.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_GENERATE_INSTALL_CODE_CRC_REQ.
 */
typedef struct _zstackmsg_bdbgenerateinstallcodecrc_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbGenerateInstallCodeCRCReq_t *pReq;

    /** Response fields (immediate response). This contains the CRC generated from
     *  the given install code */
    zstack_bdbGenerateInstallCodeCRCRsp_t *pRsp;

} zstackmsg_bdbGenerateInstallCodeCRCReq_t;


#ifdef BDB_REPORTING
/**
 * Send this message to the ZStack Thread to indicate the stack the default attribute reporting
 * configuration for an attribute. This default configuration will be used if no Report Configuration command is issued to
 * this device. This configuration will also be used when the device is instructed to return to its default configuration
 * with a Report Configure command or Factory New Reset.
 * If no Report Configuration command is received, this default configuration will be used when to report the attribute
 * automatically if a bind to the cluster indicated is added.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_REP_ADD_ATTR_CFG_RECORD_DEFAULT_TO_LIST_REQ.
 */
typedef struct _zstackmsg_bdbrepaddattrcfgrecorddefaulttolist_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbRepAddAttrCfgRecordDefaultToListReq_t *pReq;

} zstackmsg_bdbRepAddAttrCfgRecordDefaultToListReq_t;

/**
 * Send this message to the ZStack Thread to indicate that a reportable attribute has
 * change its value. The reporting module will check the attribute value and issue a
 * report attribute command if necessary according to the current reporting configuration.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_REP_CHANGED_ATTR_VALUE_REQ.
 */
typedef struct _zstackmsg_bdbrepchangedattrvalue_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbRepChangedAttrValueReq_t *pReq;

} zstackmsg_bdbRepChangedAttrValueReq_t;


/**
 * Send this message to the ZStack Thread to allow update the existing reporting configuration.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_PROCESS_IN_CONFIG_REPORT_REQ.
 */
typedef struct _zstackmsg_bdbprocessinconfigreport_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbProcessInConfigReportReq_t *pReq;

} zstackmsg_bdbProcessInConfigReportReq_t;

/**
 * Send this message to the ZStack Thread to generate the response with the current
 * attribute reporting configuration.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_PROCESS_IN_READ_REPORT_CFG_REQ.
 */
typedef struct _zstackmsg_bdbprocessinreadreportcfg_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbProcessInReadReportCfgReq_t *pReq;

} zstackmsg_bdbProcessInReadReportCfgReq_t;



#endif


/**
 * Send this message to the ZStack Thread to add an install code in the coordinator.
 * This install code will be used with the device indicated by the IEEE address during joining.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_ADD_INSTALL_CODE_REQ.
 */
typedef struct _zstackmsg_bdbaddinstallcode_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbAddInstallCodeReq_t *pReq;

} zstackmsg_bdbAddInstallCodeReq_t;

/**
 * Send this message to the ZStack Thread to to enable TouchLink as Target for the indicated time.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_ENABLE_COMMISSIONING_REQ.
 */
typedef struct _zstackmsg_bdbtouchlinktargetenablecommissioning_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbTouchLinkTargetEnableCommissioningReq_t *pReq;

} zstackmsg_bdbTouchLinkTargetEnableCommissioningReq_t;

/**
 * Send this message to the ZStack Thread to stop TouchLink as Target.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_DISABLE_COMMISSIONING_REQ.
 */
typedef struct _zstackmsg_bdbtouchlinktargetdisablecommissioning_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no command fields, NULL field */
    void *pReq;

} zstackmsg_bdbTouchLinkTargetDisableCommissioningReq_t;

/**
 * Send this message to the ZStack Thread to get the remaining time for TouchLink as Target.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_GETT_IMER_REQ.
 */
typedef struct _zstackmsg_bdbtouchlinktargetgettimer_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no command fields, NULL field */
    void *pReq;

    /** Response fields (immediate response) */
    zstack_bdbTouchLinkTargetGetTimerRsp_t *pRsp;

} zstackmsg_bdbTouchLinkTargetGetTimerReq_t;

/**
 * Send this message to the ZStack Thread to indicate which Link Key will be used
 * when joining a centralized network.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_SET_ACTIVE_CENTRALIZED_LINK_KEY_REQ.
 */
typedef struct _zstackmsg_bdbsetactivecentralizedlinkkey_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbSetActiveCentralizedLinkKeyReq_t *pReq;

} zstackmsg_bdbSetActiveCentralizedLinkKeyReq_t;

/**
 * Send this message to the ZStack Thread to report the stack if the default Trust Center Link
 * Key exchange has to be done (report fail on this) or it has been done by application means (report Success on this).
 * The command ID for this message is zstackmsg_CmdIDs_BDB_CBKE_TC_LINK_KEY_EXCHANGE_ATTEMPT_REQ.
 */
typedef struct _zstackmsg_bdbcbketclinkkeyexchangeattempt_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbCBKETCLinkKeyExchangeAttemptReq_t *pReq;

} zstackmsg_bdbCBKETCLinkKeyExchangeAttemptReq_t;

/**
 * Send this message to the ZStack Thread to release a network descriptor from
 * the list of networks found during the discovery of suitable networks to join.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_NWK_DESC_FREE_REQ.
 */
typedef struct _zstackmsg_bdbnwkdescfree_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbNwkDescFreeReq_t *pReq;

} zstackmsg_bdbNwkDescFreeReq_t;

/**
 * Send this message to the ZStack Thread to indicate the stack that all networks that
 * remains in the list of networks found can be attempted.
 * This must be call even if there are no network descriptors in the list to allow the
 * stack finish Steering process.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_FILTER_NWK_DESC_COMPLETE_REQ.
 */
typedef struct _zstackmsg_bdbfilternwkdesccomplete_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** No parameters */


} zstackmsg_bdbFilterNwkDescCompleteReq_t;



/**
 * Send this message to the ZStack Thread to allow TouchLink stealing
 * The command ID for this message is zstackmsg_CmdIDs_BDB_TOUCHLINK_SET_ALLOW_STEALING_REQ.
 */
typedef struct _zstackmsg_bdbtouchlinksetallowstealing_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_bdbTouchlinkSetAllowStealingReq_t *pReq;

} zstackmsg_bdbTouchlinkSetAllowStealingReq_t;

/**
 * Send this message to the ZStack Thread to get the allow TouchLink allow stealing state.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_TOUCHLINK_GET_ALLOW_STEALING_REQ.
 */
typedef struct _zstackmsg_bdbtouchlinkgetallowstealing_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no command fields, NULL field */
    void *pReq;

    /** Response fields (immediate response) */
    zstack_bdbTouchlinkGetAllowStealingRsp_t *pRsp;

} zstackmsg_bdbTouchlinkGetAllowStealingReq_t;

/**
 * Send this message to the ZStack Thread to trigger the rejoin mechanism for End Devices
 * that has lost its parent.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_RECOVER_NWK_REQ.
 */
typedef struct _zstackmsg_bdbrecovernwk_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /* Message command fields - no command fields, NULL field */
    void *pReq;

    //Indicate if the request was process or not. The request will fail
    //if the device has not been commissioned yet
    zstack_bdbRecoverNwkRsp_t *pRsp;

} zstackmsg_bdbRecoverNwkReq_t;

/**
 * Send this message to the ZStack Thread to indicate if Green Power Commissioning
 * is allowed to change the channel to complete commissioning of GPD.
 * The command ID for this message is zstackmsg_CmdIDs_GP_ALLOW_CHANNEL_CHANGE_REQ.
 */
typedef struct _zstackmsg_gpallowchangechannelreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields  */
    uint8_t AllowChannelChange;

} zstackmsg_gpAllowChangeChannelReq_t;



/**
 * Message send from the ZStack Thread to the application to provide updates on the commissioning process requested.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_NOTIFICATION.
 */
typedef struct _zstackmsg_bdbnotificationind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    bdbCommissioningModeMsg_t Req;

} zstackmsg_bdbNotificationInd_t;


/**
 * Message send from the ZStack Thread to let know the application when identify
 * is active or not in the indicated endpoint.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_IDENTIFY_TIME_CB.
 */
typedef struct _zstackmsg_bdbidentifytimeoutind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Endpoint in which the time is being updated */
    uint8_t EndPoint;

} zstackmsg_bdbIdentifyTimeoutInd_t;

/**
 * Message send from the ZStack Thread to indicate that a bind has been created.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_BIND_NOTIFICATION_CB.
 */
typedef struct _zstackmsg_bdbbindnotificationind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    bdbBindNotificationData_t Req;

} zstackmsg_bdbBindNotificationInd_t;


/**
 * Message send from the ZStack Thread to indicate the application that a new device
 * has joined and performed the Trust Center Link Key Exchange.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_TC_LINK_KEY_EXCHANGE_NOTIFICATION_IND.
 */
typedef struct _zstackmsg_bdbtclinkkeyexchangeind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    bdb_TCLinkKeyExchProcess_t Req;

} zstackmsg_bdbTCLinkKeyExchangeInd_t;

/**
 * Message send from the ZStack Thread to notify a change in the state of TouchLink as target (enabled or disabled)
 * The command ID for this message is zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_ENABLE_IND.
 */
typedef struct _zstackmsg_bdbtouchlinktargetenableind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    uint8_t Enable;

} zstackmsg_bdbTouchLinkTargetEnableInd_t;


/**
 * Message send from the ZStack Thread to notify the application to perform its own Trust
 * Center Link Key Exchange procedure or to report fail on this to allow the default procedure
 * to be performed. Refer to Zstackapi_bdbCBKETCLinkKeyExchangeAttemptReq().
 * The command ID for this message is zstackmsg_CmdIDs_BDB_CBKE_TC_LINK_KEY_EXCHANGE_IND.
 */
typedef struct _zstackmsg_bdbcbketclinkkeyexchangeind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** There are no parameters for this call */
    /** NOTE: only the first application registered will receive this notification */


} zstackmsg_bdbCBKETCLinkKeyExchangeInd_t;


/**
 * Message send from the ZStack Thread to indicate the networks found. The networks
 * are provided as a list of network descriptors. Refer to Zstackapi_bdbFilterNwkDescComplete()
 * and Zstackapi_bdbNwkDescFreeReq();
 * The command ID for this message is zstackmsg_CmdIDs_BDB_FILTER_NWK_DESCRIPTOR_IND.
 */
typedef struct _zstackmsg_bdbfilternwkdescriptorind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields NOTE: only the first application registered will receive this notification  */
    bdbFilterNetworkDesc_t bdbFilterNetworkDesc;

} zstackmsg_bdbFilterNwkDescriptorInd_t;

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/**
 * Message send from the ZStack Thread to indicate that this device has enter into
 * GP commissioning mode.
 * The command ID for this message is zstackmsg_CmdIDs_GP_COMMISSIONING_MODE_IND.
 */
typedef struct _zstackmsg_gpcommissioningmodeind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_gpCommissioningMode_t Req;

} zstackmsg_gpCommissioningModeInd_t;

/**
 * Message send from the ZStack Thread to process a data indication command from gp stub.
 * The command ID for this message is zstackmsg_CmdIDs_GP_DATA_IND.
 */
typedef struct _zstackmsg_gpdataind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_gpDataInd_t Req;

} zstackmsg_gpDataInd_t;

/**
 * Message send from the ZStack Thread to process a security request command from gp stub.
 * The command ID for this message is zstackmsg_CmdIDs_GP_SECURITY_REQ.
 */
typedef struct _zstackmsg_gpsecreq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_gpSecReq_t Req;

} zstackmsg_gpSecReq_t;

/**
 * Message send from the ZStack Thread to process a security request command from gp stub.
 * The command ID for this message is zstackmsg_CmdIDs_GP_SECURITY_RSP.
 */
typedef struct _zstackmsg_gpsecrsp_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_gpSecRsp_t *pReq;

} zstackmsg_gpSecRsp_t;

/**
 * Message send from the ZStack Thread to check device announce with proxy table.
 * The command ID for this message is zstackmsg_CmdIDs_GP_CHECK_ANNCE.
 */
typedef struct _zstackmsg_gpcheckannounce_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_gpCheckAnnounce_t Req;

} zstackmsg_gpCheckAnnounce_t;

/**
 * Message send from the ZStack Thread to solve address conflict with proxy table.
 * The command ID for this message is zstackmsg_CmdIDs_GP_ADDRESS_CONFLICT.
 */
typedef struct _zstackmsg_gpaliasconflict_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_gpAddrConflict_t *pReq;

} zstackmsg_gpAliasConflict_t;


/**
 * Message send from the ZStack Thread to encrypt or decrypt the GPDF commissioning key.
 * The command ID for this message is zstackmsg_CmdIDs_GP_CCM_STAR_COMMISSIONING_KEY.
 */
typedef struct _zstackmsg_gpdecryptdataind_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_gpDecryptDataInd_t *pReq;

    /** Decrypted key pointer **/
    zstack_gpDecryptDataIndRsp_t *pRsp;

} zstackmsg_gpDecryptDataInd_t;

/**
 * Message send from the ZStack Thread to encrypt or decrypt the GPDF commissioning key.
 * The command ID for this message is zstackmsg_CmdIDs_GP_CCM_STAR_COMMISSIONING_KEY.
 */
typedef struct _zstackmsg_gpencryptdecryptcommissioningkey_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_gpEncryptDecryptCommissioningKey_t *pReq;

    /** Decrypted key pointer **/
    zstack_gpEncryptDecryptCommissioningKeyRsp_t *pRsp;

} zstackmsg_gpEncryptDecryptCommissioningKey_t;


/**
 * Message send from the ZStack Thread to process gp commissioning success.
 * The command ID for this message is zstackmsg_CmdIDs_GP_COMMISSIONING_SUCCESS.
 */
typedef struct _zstackmsg_gpcommissioningsucess_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_gpCommissioningSuccess_t *pReq;

} zstackmsg_gpCommissioningSuccess_t;

/**
 * Message send from the ZStack Thread to send device announce as result of gp commissioning
 * The command ID for this message is zstackmsg_CmdIDs_GP_SEND_DEV_ANNOUNCE.
 */
typedef struct _zstackmsg_gpsenddevannounce_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_gpAliasNwkAddress_t *pReq;

} zstackmsg_gpSendDeviceAnnounce_t;
#endif


#if defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )

/**
 * Message send from the ZStack Thread to process a scan request command.
 * The command ID for this message is zstackmsg_CmdIDs_TL_SCAN_REC_IND.
 */
typedef struct _zstackmsg_tlScanReq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_tlScanReq_t *pReq;

}zstackmsg_tlScanReq_t;

/**
 * Message send from the ZStack Thread to process a target network start request command.
 * The command ID for this message is zstackmsg_CmdIDs_TL_TARGET_NWK_START_REC_IND.
 */
typedef struct _zstackmsg_tlTargetNwkStartReq_t

{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_tlTargetNwkStartReq_t *pReq;

}zstackmsg_tlTargetNwkStartReq_t;

/**
 * Message send from the ZStack Thread to process a target network start request command.
 * The command ID for this message is zstackmsg_CmdIDs_TOUCHLINK_NWK_JOIN_REC_IND.
 */
typedef struct _zstackmsg_tlTargetNwkJointReq_t

{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_touchlinkNwkJointReq_t *pReq;

}zstackmsg_touchlinkNwkJointReq_t;

/**
 * Message send from the ZStack Thread to process a target network start request command.
 * The command ID for this message is zstackmsg_CmdIDs_TOUCHLINK_NWK_UPDATE_REC_IND.
 */
typedef struct _zstackmsg_touchlinkNwkUpdateReq_t

{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_touchlinkNwkUpdateReq_t *pReq;

}zstackmsg_touchlinkNwkUpdateReq_t;

/**
 * Message send from the ZStack Thread to process a scan base time.
 * The command ID for this message is zstackmsg_CmdIDs_TL_GET_SCAN_BASE_TIME.
 */
typedef struct _zstackmsg_touchlinkGetScanBaseTime_t

{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_touchlinkGetScanBaseTime_t *pReq;

}zstackmsg_touchlinkGetScanBaseTime_t;

/**
 * Message send from the ZStack Thread to process a initiator scan response indication.
 * The command ID for this message is zstackmsg_CmdIDs_TL_INITIATOR_SCAN_RSP_IND.
 */
typedef struct _zstackmsg_touchlinkScanRsp_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_touchlinkScanRsp_t *pReq;

}zstackmsg_touchlinkScanRsp_t;

/**
 * Message send from the ZStack Thread to process a device information response indication.
 * The command ID for this message is zstackmsg_CmdIDs_TOUCHLINK_DEV_INFO_RSP_IND.
 */
typedef struct _zstackmsg_touchlinkDevInfoRsp_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_touchlinkDevInfoRsp_t *pReq;

}zstackmsg_touchlinkDevInfoRsp_t;

/**
 * Message send from the ZStack Thread to process a network start response indication.
 * The command ID for this message is zstackmsg_CmdIDs_TL_INITIATOR_NWK_START_RSP_IND.
 */
typedef struct _zstackmsg_touchlinkNwkStartRsp_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_touchlinkNwkStartRsp_t *pReq;

}zstackmsg_touchlinkNwkStartRsp_t;

/**
 * Message send from the ZStack Thread to process a network start response indication.
 * The command ID for this message is zstackmsg_CmdIDs_TL_INITIATOR_NWK_JOIN_RSP_IND.
 */
typedef struct _zstackmsg_touchlinkNwkJoinRsp_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_touchlinkNwkJoinRsp_t *pReq;

}zstackmsg_touchlinkNwkJoinRsp_t;

#endif

/**
 * Send this message to the ZStack Thread to release a network descriptor from
 * the list of networks found during the discovery of suitable networks to join.
 * The command ID for this message is zstackmsg_CmdIDs_BDB_NWK_DESC_FREE_REQ.
 */
typedef struct _zstackmsg_pauseResumeDeviceReq_t
{
    /** message header<br>
     * event field must be set to @ref zstack_CmdIDs
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    zstack_pauseResumeDeviceReq_t *pReq;

} zstackmsg_pauseResumeDeviceReq_t;

//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* ZSTACKMSG_H */

