/**
   @headerfile  zstackapi.h
   $Date: 2018-12-18 12:13:13 -0800 (Tue, 18 Dec 2018) $
   $Revision: 42528 $

   @mainpage Z-Stack TI-RTOS API

   Overview
   ============================

   To write an application for the Texas Instruments SimpleLink CC13x2 / CC26x2
   Software Development Kit you first need to understand that the embedded
   system for the CC13x2/CC26x2 is organized as follows
   <BR>
   \image html fig-example-application-block-diagram.PNG
   <BR><BR>
   The functions defined in zstackapi.h are functions that communicate
   with the Z-Stack image through OsalPort messaging.  If an immediate response,
   not over the air, is expected, the function will block for the
   response message.
   <BR><BR>
   To setup communicate with the Z-Stack thread, your application
   should do the following initialization (simplified):
   - Register one or more Zigbee Device Endpoints by calling
   Zstackapi_AfRegisterReq().
   - Register for Z-Stack Callback (asynchronous) messages by calling
   Zstackapi_DevZDOCBReq() with the wanted callbacks. For example, if you
   would like to receive device state change notifications, you will have to
   set the has_devStateChange and devStateChange fields to true in a
   [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t), then call Zstackapi_DevZDOCBReq().
   - Write Z-Stack parameters by calling Zstackapi_sysConfigWriteReq() with
   the wanted parameters.  Example parameters are channel mask, PAN ID, poll
   rates, timeouts, etc...
   - Call Zstackapi_DevStartReq() to start the Z-Stack Thread's automatic
   joining process.

   When the joining process is complete, you'll receive a
   [zstackmsg_CmdIDs_DEV_STATE_CHANGE_IND](@ref zstack_CmdIDs)
   message with a state of:
   - [zstack_DevState_DEV_ZB_COORD](@ref zstack_DevState) - the
   device started as a coordinator.
   - [zstack_DevState_DEV_ROUTER](@ref zstack_DevState) - the device
   joined as a router.
   - [zstack_DevState_DEV_END_DEVICE](@ref zstack_DevState) - the
   device joined as an end device.

   You should then call zclport_getDeviceInfo(), or
   Zstackapi_sysNwkInfoReadReq() if your application isn't a ZCL project,
   to retrieve the device's joined network parameters.
   <BR>

   You're free to do device discovery through ZDO commands (ie.
   Zstackapi_ZdoMatchDescReq()) and send data
   either through ZCL commands or by calling Zstackapi_AfDataReq() to send
   application proprietary messages.
   <BR>

   Once you've registed an endpoint, your application will recieve any data
   message addressed to that endpoint as an AF Incoming Data Indication
   [[zstackmsg_afIncomingMsgInd_t](@ref _zstackmsg_afincomingmsgind_t) with a
   message event of [zstackmsg_CmdIDs_AF_INCOMING_MSG_IND](@ref zstack_CmdIDs)]
   through an OsalPort message.
   <BR>

   Also, any indications or response message that you have signed up for using
   Zstackapi_DevZDOCBReq() will be delivered to your application as an OsalPort
   message.  For example, the Device State Change message
   [[zstackmsg_devStateChangeInd_t](@ref _zstackmsg_devstatechangeind_t) with a
   message event of
   [zstackmsg_CmdIDs_DEV_STATE_CHANGE_IND](@ref zstack_CmdIDs)].
   <BR><BR>

   Z-Stack API
   ============================
   - <b>System Messages</b> - Configuration and miscellaneous commands
       - Zstackapi_sysVersionReq()
       - Zstackapi_sysConfigReadReq()
       - Zstackapi_sysConfigWriteReq()
       - Zstackapi_sysSetTxPowerReq()
       - Zstackapi_sysNwkInfoReadReq()
       - Zstackapi_sysForceLinkStatusReq()
       - Zstackapi_SetNwkFrameFwdNotificationReq()
   <BR><BR>
   - <b>Device Control</b><BR>
     The following group of APIs allow the application control the device
     in a Zigbee network: network discovery, device start, and control routing.
       - Zstackapi_DevStartReq()
       - Zstackapi_DevNwkDiscReq()
       - Zstackapi_DevJoinReq()
       - Zstackapi_DevZDOCBReq()
       - Zstackapi_DevNwkRouteReq()
       - Zstackapi_DevNwkCheckRouteReq()
       - Zstackapi_DevUpdateNeighborTxCostReq()
       - Zstackapi_DevForceNetworkSettingsReq()
       - Zstackapi_DevForceNetworkUpdateReq()
       - Zstackapi_DevForceMacParamsReq()
   <BR><BR>
   - <b>APS Inferace</b><BR>
       The APS group table is a link list defined with allocated RAM in the
       Z-Stack Thread, so as groups are added to the table the amount of heap
       used increases. The maximum table size can be changed by adjusting
       APS_MAX_GROUPS in ti_zstack_config.h (Z-Stack Thread Image), if this value
       is changed the Z-Stack Thread image must be recompiled and the Group
       table will take up more space in NV.
       - Zstackapi_ApsRemoveGroupReq()
       - Zstackapi_ApsRemoveAllGroupsReq()
       - Zstackapi_ApsFindAllGroupsReq()
       - Zstackapi_ApsFindGroupReq()
       - Zstackapi_ApsAddGroupReq()
       - Zstackapi_ApsCountAllGroupsReq()
   <BR><BR>
   - <b>Application Framework (AF)</b><BR>
     The Application Framework layer (withing the Z-Stack Thread) is the
     over-the-air data interface to the APS layer. This interface sends data out
     over the air (through the APS and NWK) layers. This layer is also the
     endpoint multiplexer for incoming data messages.<BR>
     Each device is a node in the Zigbee. Each node has a long and short
     address, the short address of the node is used by other nodes to send it
     data. Each node has 241 endpoints (0 reserved, 1-240 application assigned).
     Each endpoint is separately addressable; when a device sends data it
     must specify the destination node's short address and the endpoint that
     will receive that data.<BR>
     An application must register one or more endpoints to send and receive
     data in a Zigbee network.<BR>
       - Zstackapi_AfRegisterReq()
       - Zstackapi_AfUnRegisterReq()
       - Zstackapi_AfDataReq()
       - Zstackapi_AfConfigGetReq()
       - Zstackapi_AfConfigSetReq()
   <BR><BR>
   - <b>Zigbee Device Objects (ZDO)</b><BR>
       This section enumerates all the function calls provided by the ZDO layer
       that are necessary for the implementation of all commands defined in
       Zigbee Device Profile (ZDP). <BR>
       ZDP describes how general Zigbee Device features are implemented within
       ZDO. It defines Device Description and Clusters which employ command.
       Through the definition of messages in command structure, ZDP provides
       the following functionality to the ZDO and applications: Device and
       Service Discovery; End Device Bind, Bind and Unbind Service; and Network
       Management Service. Device discovery is the process for a Zigbee device
       to discover other Zigbee devices. One example of device discovery is the
       NWK address request which is broadcast and carries the known IEEE
       address as data payload. The device of interest should respond and
       inform its NWK address. Service discovery provides the ability for a
       device to discover the services offered by other Zigbee devices on the
       PAN. It utilizes various descriptors to specify device capabilities.<BR>
       End device bind, bind and unbind services offer Zigbee devices the
       binding capabilities. Typically, binding is used during the installation
       of a network when the user needs to bind controlling devices with
       devices being controlled, such as switches and lights. Particularly,
       end device bind supports a simplified binding method where user input
       is utilized to identify controlling/controlled device pairs. Bind and
       unbind services provide the ability for creation and deletion of
       binding table entry that map control messages to their intended
       destination.<BR>
       Network management services provide the ability to retrieve management
       information from the devices, including network discovery results,
       routing table contents, link quality to neighbor nodes, and binding
       table contents. It also provides the ability to control the network
       association by disassociating devices from the PAN. Network management
       services are designed majorly for user or commissioning tools to manage
       the network.<BR>
       - Zstackapi_ZdoNwkAddrReq()
       - Zstackapi_ZdoIeeeAddrReq()
       - Zstackapi_ZdoNodeDescReq()
       - Zstackapi_ZdoPowerDescReq()
       - Zstackapi_ZdoSimpleDescReq()
       - Zstackapi_ZdoActiveEndpointReq()
       - Zstackapi_ZdoMatchDescReq()
       - Zstackapi_ZdoComplexDescReq()
       - Zstackapi_ZdoServerDiscReq()
       - Zstackapi_ZdoEndDeviceBindReq()
       - Zstackapi_ZdoBindReq()
       - Zstackapi_ZdoUnbindReq()
       - Zstackapi_ZdoSetBindUnbindAuthAddrReq()
       - Zstackapi_ZdoMgmtNwkDiscReq()
       - Zstackapi_ZdoMgmtLqiReq()
       - Zstackapi_ZdoMgmtRtgReq()
       - Zstackapi_ZdoMgmtBindReq()
       - Zstackapi_ZdoMgmtLeaveReq()
       - Zstackapi_ZdoMgmtDirectJoinReq()
       - Zstackapi_ZdoMgmtPermitJoinReq()
       - Zstackapi_ZdoMgmtNwkUpdateReq()
       - Zstackapi_ZdoDeviceAnnounceReq()
       - Zstackapi_ZdoUserDescSetReq()
       - Zstackapi_ZdoUserDescReq()
   <BR><BR>
   - <b>Security Interface</b> - Network and Link Key access commands
       - Zstackapi_secNwkKeyGetReq()
       - Zstackapi_secNwkKeySetReq()
       - Zstackapi_secNwkKeyUpdateReq()
       - Zstackapi_secNwkKeySwitchReq()
       - Zstackapi_secApsLinkKeyGetReq()
       - Zstackapi_secApsLinkKeySetReq()
       - Zstackapi_secApsLinkKeyRemoveReq()
       - Zstackapi_secApsRemoveReq()
   <BR><BR>
   - <b>Base Device Behavior Interface</b> - Commissioning procedures through the Zigbee BDB Specification
       - Zstackapi_bdbStartCommissioningReq()
       - Zstackapi_bdbSetIdentifyActiveEndpointReq()
       - Zstackapi_bdbGetIdentifyActiveEndpointReq()
       - Zstackapi_bdbStopInitiatorFindingBindingReq()
       - Zstackapi_bdbZclIdentifyCmdIndReq()
       - Zstackapi_getZCLFrameCounterReq()
       - Zstackapi_bdbSetEpDescListToActiveEndpoint()
       - Zstackapi_bdbResetLocalActionReq()
       - Zstackapi_bdbSetAttributesReq()
       - Zstackapi_bdbGetAttributesReq()
       - Zstackapi_bdbGetFBInitiatorStatusReq()
       - Zstackapi_bdbGenerateInstallCodeCRCReq()
       - Zstackapi_bdbRepAddAttrCfgRecordDefaultToListReq()
       - Zstackapi_bdbRepChangedAttrValueReq()
       - Zstackapi_bdbAddInstallCodeReq()
       - Zstackapi_bdbTouchLinkTargetEnableCommissioningReq()
       - Zstackapi_bdbTouchLinkTargetDisableCommissioningReq()
       - Zstackapi_bdbTouchLinkTargetGetTimerReq()
       - Zstackapi_bdbSetActiveCentralizedLinkKeyReq()
       - Zstackapi_bdbCBKETCLinkKeyExchangeAttemptReq()
       - Zstackapi_bdbNwkDescFreeReq()
       - Zstackapi_bdbTouchlinkSetAllowStealingReq()
       - Zstackapi_bdbTouchlinkGetAllowStealingReq()
       - Zstackapi_bdbRecoverNwkReq()


   ZStack Indications (callbacks)
   ============================
   The following messages will be delivered to your application through an
   OsalPort message after you register at least one endpoint with
   Zstackapi_AfRegisterReq(), you must call Zstackapi_freeIndMsg() to
   free the message when you are done processing:
   <BR>

   - zstackmsg_CmdIDs_AF_DATA_CONFIRM_IND - [zstackmsg_afDataConfirmInd_t]
     (@ref _zstackmsg_afdataconfirmind_t)
   - zstackmsg_CmdIDs_AF_INCOMING_MSG_IND - [zstackmsg_afIncomingMsgInd_t]
     (@ref _zstackmsg_afincomingmsgind_t)

   The following messages will be delivered to your application through an
   OsalPort message after you request them by setting the correct fields
   (has and flag) to 'true' in
   [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) and calling
   Zstackapi_DevZDOCBReq(), you must call Zstackapi_freeIndMsg() to
   free the message when you are done processing:
   <BR>

   - zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE - [zstackmsg_zdoDeviceAnnounceReq_t]
     (@ref _zstackmsg_zdodeviceannouncereq_t)
   - zstackmsg_CmdIDs_ZDO_NWK_ADDR_RSP - [zstackmsg_zdoNwkAddrRspInd_t]
     (@ref _zstackmsg_zdonwkaddrrspind_t)
   - zstackmsg_CmdIDs_ZDO_IEEE_ADDR_RSP - [zstackmsg_zdoIeeeAddrRspInd_t]
     (@ref _zstackmsg_zdoieeeaddrrspind_t)
   - zstackmsg_CmdIDs_ZDO_NODE_DESC_RSP - [zstackmsg_zdoNodeDescRspInd_t]
     (@ref _zstackmsg_zdonodedescrspind_t)
   - zstackmsg_CmdIDs_ZDO_POWER_DESC_RSP - [zstackmsg_zdoPowerDescRspInd_t]
     (@ref _zstackmsg_zdopowerdescrspind_t)
   - zstackmsg_CmdIDs_ZDO_SIMPLE_DESC_RSP - [zstackmsg_zdoSimpleDescRspInd_t]
     (@ref _zstackmsg_zdosimpledescrspind_t)
   - zstackmsg_CmdIDs_ZDO_ACTIVE_EP_RSP - [zstackmsg_zdoActiveEndpointsRspInd_t]
     (@ref _zstackmsg_zdoactiveendpointsrspind_t)
   - zstackmsg_CmdIDs_ZDO_MATCH_DESC_RSP - [zstackmsg_zdoMatchDescRspInd_t]
     (@ref _zstackmsg_zdomatchdescrspind_t)
   - zstackmsg_CmdIDs_ZDO_USER_DESC_RSP - [zstackmsg_zdoUserDescRspInd_t]
     (@ref _zstackmsg_zdouserdescrspind_t)
   - zstackmsg_CmdIDs_ZDO_SERVER_DISC_RSP - [zstackmsg_zdoServerDiscoveryRspInd_t]
     (@ref _zstackmsg_zdoserverdiscoveryrspind_t)
   - zstackmsg_CmdIDs_ZDO_END_DEVICE_BIND_RSP - [zstackmsg_zdoEndDeviceBindRspInd_t]
     (@ref _zstackmsg_zdoenddevicebindrspind_t)
   - zstackmsg_CmdIDs_ZDO_BIND_RSP - [zstackmsg_zdoBindRspInd_t]
     (@ref _zstackmsg_zdobindrspind_t)
   - zstackmsg_CmdIDs_ZDO_UNBIND_RSP - [zstackmsg_zdoUnbindRspInd_t]
     (@ref _zstackmsg_zdounbindrspind_t)
   - zstackmsg_CmdIDs_ZDO_MGMT_NWK_DISC_RSP - [zstackmsg_zdoMgmtNwkDiscRspInd_t]
     (@ref _zstackmsg_zdomgmtnwkdiscrspind_t)
   - zstackmsg_CmdIDs_ZDO_MGMT_LQI_RSP - [zstackmsg_zdoMgmtLqiRspInd_t]
     (@ref _zstackmsg_zdomgmtlqirspind_t)
   - zstackmsg_CmdIDs_ZDO_MGMT_RTG_RSP - [zstackmsg_zdoMgmtRtgRspInd_t]
     (@ref _zstackmsg_zdomgmtrtgrspind_t)
   - zstackmsg_CmdIDs_ZDO_MGMT_BIND_RSP - [zstackmsg_zdoMgmtBindRspInd_t]
     (@ref _zstackmsg_zdomgmtbindrspind_t)
   - zstackmsg_CmdIDs_ZDO_MGMT_LEAVE_RSP - [zstackmsg_zdoMgmtLeaveRspInd_t]
     (@ref _zstackmsg_zdomgmtleaverspind_t)
   - zstackmsg_CmdIDs_ZDO_MGMT_DIRECT_JOIN_RSP - [zstackmsg_zdoMgmtDirectJoinRspInd_t]
     (@ref _zstackmsg_zdomgmtdirectjoinrspind_t)
   - zstackmsg_CmdIDs_ZDO_MGMT_PERMIT_JOIN_RSP - [zstackmsg_zdoMgmtPermitJoinRspInd_t]
     (@ref _zstackmsg_zdomgmtpermitjoinrspind_t)
   - zstackmsg_CmdIDs_ZDO_MGMT_NWK_UPDATE_NOTIFY - [zstackmsg_zdoMgmtNwkUpdateNotifyInd_t]
     (@ref _zstackmsg_zdomgmtnwkupdatenotifyind_t)
   - zstackmsg_CmdIDs_ZDO_SRC_RTG_IND - [zstackmsg_zdoSrcRtgInd_t]
     (@ref _zstackmsg_zdosrcrtgind_t)
   - zstackmsg_CmdIDs_ZDO_CONCENTRATOR_IND - [zstackmsg_zdoCncntrtrInd_t]
     (@ref _zstackmsg_zdocncntrtrind_t)
   - zstackmsg_CmdIDs_ZDO_NWK_DISC_CNF - [zstackmsg_zdoNwkDiscCnf_t]
     (@ref _zstackmsg_zdonwkdisccnf_t)
   - zstackmsg_CmdIDs_ZDO_BEACON_NOTIFY_IND - [zstackmsg_zdoBeaconNotifyInd_t]
     (@ref _zstackmsg_zdobeaconnotifyind_t)
   - zstackmsg_CmdIDs_ZDO_JOIN_CNF - [zstackmsg_zdoJoinConf_t]
     (@ref _zstackmsg_zdojoinconf_t)
   - zstackmsg_CmdIDs_ZDO_LEAVE_CNF - [zstackmsg_zdoLeaveCnf_t]
     (@ref _zstackmsg_zdoleavecnf_t)
   - zstackmsg_CmdIDs_ZDO_LEAVE_IND - [zstackmsg_zdoLeaveInd_t]
     (@ref _zstackmsg_zdoleaveind_t)
   - zstackmsg_CmdIDs_AF_REFLECT_ERROR_IND - [zstackmsg_afReflectErrorInd_t]
     (@ref _zstackmsg_afreflecterrorind_t)
   - zstackmsg_CmdIDs_DEV_STATE_CHANGE_IND - [zstackmsg_devStateChangeInd_t]
     (@ref _zstackmsg_devstatechangeind_t)
   - zstackmsg_CmdIDs_ZDO_TC_DEVICE_IND - [zstackmsg_zdoTcDeviceInd_t]
     (@ref _zstackmsg_zdotcdeviceind_t)
   - zstackmsg_CmdIDs_DEV_PERMIT_JOIN_IND - [zstackmsg_devPermitJoinInd_t]
     (@ref _zstackmsg_devpermitjoinind_t)

   The following messages will be delivered to your application through an
   OsalPort message after a request to perform any BDB commissioning process,
   for instance, Zstackapi_bdbStartCommissioningReq() calls will trigger
   zstackmsg_CmdIDs_BDB_NOTIFICATION on the requested commissioning methods,
   you must call Zstackapi_freeIndMsg() to free the message when you are
   done processing:
   <BR>
   - zstackmsg_CmdIDs_BDB_NOTIFICATION - [zstackmsg_bdbNotificationInd_t]
     (@ref _zstackmsg_bdbnotificationind_t)
   - zstackmsg_CmdIDs_BDB_IDENTIFY_TIME_CB - [zstackmsg_bdbIdentifyTimeoutInd_t]
     (@ref _zstackmsg_bdbidentifytimeoutind_t)
   - zstackmsg_CmdIDs_BDB_BIND_NOTIFICATION_CB - [zstackmsg_bdbBindNotificationInd_t]
     (@ref _zstackmsg_bdbbindnotificationind_t)
   - zstackmsg_CmdIDs_BDB_CBKE_TC_LINK_KEY_EXCHANGE_IND - [zstack_bdbCBKETCLinkKeyExchangeAttemptReq_t]
     (@ref _zstack_bdbcbketclinkkeyexchangeattemptreq_t)
   - zstackmsg_CmdIDs_BDB_FILTER_NWK_DESCRIPTOR_IND - [zstackmsg_bdbFilterNwkDescriptorInd_t]
     (@ref _zstackmsg_bdbfilternwkdescriptorind_t)
   - zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_ENABLE_IND - [zstackmsg_bdbTouchLinkTargetEnableInd_t]
     (@ref _zstackmsg_bdbtouchlinktargetenableind_t)


   The following messages will be delivered to your application through an
   OsalPort message when GP messages are process by the stack and must be process by Green Power Proxy
   or sink application side, which by default are already handled by example apps to the respective
   green power application.
   you must call Zstackapi_freeIndMsg() to free the message when you are
   done processing:
   <BR>
   - zstackmsg_CmdIDs_GP_DATA_IND - [zstackmsg_gpDataInd_t]
     (@ref _zstackmsg_gpdataind_t)
   - zstackmsg_CmdIDs_GP_SECURITY_REQ - [zstackmsg_gpSecReq_t]
     (@ref _zstackmsg_gpsecreq_t)
   - zstackmsg_CmdIDs_GP_CHECK_ANNCE - [zstackmsg_gpCheckAnnounce_t]
     (@ref _zstackmsg_gpcheckannounce_t)
   - zstackmsg_CmdIDs_SYS_NWK_FRAME_FWD_NOTIFICATION - [zstackmsg_nwkFrameFwdNotification_t]
     (@ref _zstack_nwkframefwdnotification_t)

   <BR><BR>

   ZCL Modules
   ===========
    - [ZCL](@ref ZCL) - All ZCL clusters IDs, common functions/typedefs
    - [ZCL General](@ref ZCL_GENERAL) ZCL clusters macros, typedefs, and functions
    for general device types
    - [ZCL Closures](@ref ZCL_CLOSURES) ZCL cluster macros, typedefs, and functions
    for closure device types
    - [ZCL Safety and Security](@ref ZCL_SS) ZCL cluster macros, typedefs, and functions
    for safety and security device types
    - [ZCL Green Power](@ref ZCL_GREEN_POWER) Operation of the ZCL Green Power feature
    - [ZCL Over the Air](@ref ZCL_OTA_CLUSTER) Macros, typedefs, and functions for
    the OTA cluster
    - [ZCL Porting functions](@ref ZclPort) - Functions needed by ZCL to
    communication with the Z-Stack Thread.

   <BR><BR>

   --------------------------------------------------------------------------------
   Copyright 2014 - 2018 Texas Instruments Incorporated.

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
 */
#ifndef ZSTACKAPI_H
#define ZSTACKAPI_H

#include <stdbool.h>
#include <stdint.h>

#include "rom_jt_154.h"

#include "zstackmsg.h"
#include "nvintf.h"

#ifdef __cplusplus
extern "C"
{
#endif

void Zstackapi_init(uint8_t stackTaskId);

/**
 * @brief       Call to send a System Reset Request to the ZStack Thread.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_sysResetReq(
    uint8_t        appEntity,
    zstack_sysResetReq_t *pReq);

/**
 * @brief       Call to send a System Version Request to the stack task
 *              Returns the core ZStack version (major, minor, maintenance).
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pRsp - Pointer to a place to put the response message.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_sysVersionReq(
    uint8_t appEntity, zstack_sysVersionRsp_t *pRsp);

/**
 * @brief       Call to send a System Config Read Request to the stack task
 *              Setup pReq (zstack_sysConfigReadReq_t) with the parameters that
 *              you would like to read (you can set one or many),
 *              call this function, pRsp
 *              (zstack_sysConfigReadRsp_t) will have the requested parameters
 *              if the return value is zstack_ZStatusValues_ZSuccess.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 * @param       pRsp - Pointer to a place to put the response message.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_sysConfigReadReq(
    uint8_t appEntity, zstack_sysConfigReadReq_t *pReq,
    zstack_sysConfigReadRsp_t *pRsp);

/**
 * @brief       Call to send a System Config Write Request to the stack task
 *              contains all the parameters that an application can change.
 *              You can set as many parameters as you like, one, two, ten or
 *              all, in the request structure, with one API call.<BR>
 *              Each of the parameters has a "has_" field that must be set to
 *              true, along with the parameter data, for the field to be used
 *              by the ZStack thread.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_sysConfigWriteReq(
    uint8_t appEntity, zstack_sysConfigWriteReq_t *pReq);

/**
 * @brief       Call to send a System Set TX Power Request to the stack task
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 * @param       pRsp - Pointer to a place to put the response message.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_sysSetTxPowerReq(
    uint8_t appEntity, zstack_sysSetTxPowerReq_t *pReq,
    zstack_sysSetTxPowerRsp_t *pRsp);

/**
 * @brief       Call to send a System Network Information Read Request. After
 *              calling this function, pRsp will contain the device's
 *              network parameters if the return value is
 *              [zstack_ZStatusValues_ZSuccess](@ref zstack_ZStatusValues)
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pRsp - Pointer to a place to put the response message.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_sysNwkInfoReadReq(
    uint8_t appEntity, zstack_sysNwkInfoReadRsp_t *pRsp);

/**
 * @brief       Call to send a System Force Link Status Request. Calling
 *              this function forces a Link Status to be sent, but it doesn't
 *              interrupt the normal Link Status cycle.
 *
 * @param       appEntity - Calling thread's task ID.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_sysForceLinkStatusReq(
    uint8_t appEntity);

/**
 * @brief       Call to send a Device Start Request to the stack task
 *              start the device in the network. All of the network paramters
 *              [Zstackapi_sysConfigWriteReq()] must be set before calling
 *              this function.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_DevStartReq(
    uint8_t        appEntity,
    zstack_devStartReq_t *pReq);

/**
 * @brief       Call to send a Device Network Discovery Request.
 *
 *  NOTES:      Calling this request will temporarily set the
 *              beacon notification callback while the scan is active.
 *              When the scan is done, the beacon notification callback
 *              will be deactivated.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_DevNwkDiscReq(
    uint8_t appEntity, zstack_devNwkDiscReq_t *pReq);


/**
 * @brief       Call to send a Set Network Frame Forward Notification Request.
 *
 *  NOTES:      Calling this request will temporarily set the
 *              beacon notification callback while the scan is active.
 *              When the scan is done, the beacon notification callback
 *              will be deactivated.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_SetNwkFrameFwdNotificationReq(
        uint8_t appEntity, zstack_setNwkFrameFwdNotificationReq_t *pReq);

/**
 * @brief       Call to send a Device Join Request,
 *              Use this function call to join a specific device in the
 *              manual joining process, don't call this if you are using
 *              the ZStack Thread's automatic joining process.  Also, make
 *              sure to request a Join Confirm, by setting the has_joinCnfCB and
 *              joinCnfCB fields to true in zstack_devZDOCBReq_t can call
 *              Zstackapi_DevZDOCBReq(), to know when the join process is
 *              done.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_DevJoinReq(
    uint8_t       appEntity,
    zstack_devJoinReq_t *pReq);

/**
 * @brief       Call to send a Device Rejoin Request
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_DevRejoinReq(
    uint8_t         appEntity,
    zstack_devRejoinReq_t *pReq);

/**
 * @brief       Call to send a Device ZDO Callback Request
 *              The zstack_devZDOCBReq_t contains all the
 *              callbacks/indications that an application can subscribe to.
 *              You can set as many callback/indications as you like, one,
 *              two, ten or all, in the request structure, with one API call.
 *              <BR>
 *              Each of the parameters has a "has_" field that must be set to
 *              true, along with the parameter data, for the field to be used
 *              by the ZStack thread.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_DevZDOCBReq(
    uint8_t        appEntity,
    zstack_devZDOCBReq_t *pReq);

/**
 * @brief       Call to send a Device Network Route Request
 *              Use this command to force a Route Request.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_DevNwkRouteReq(
    uint8_t appEntity, zstack_devNwkRouteReq_t *pReq);

/**
 * @brief       Call to send a Device Network Check Request,
 *              Use this command to check if a route is active or not.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues_ZSuccess if active,
 *              zstack_ZStatusValues_ZFailure if not active
 */
extern zstack_ZStatusValues Zstackapi_DevNwkCheckRouteReq(
    uint8_t appEntity, zstack_devNwkCheckRouteReq_t *pReq);

/**
 * @brief       Call to send a Device Update Neighbor's TxCost Request,
 *              Use this command to change a neighbor's TX Cost value.
 *              This command is only available in a router or coordinator.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_DevUpdateNeighborTxCostReq(
    uint8_t appEntity, zstack_devUpdateNeighborTxCostReq_t *pReq);

/**
 * @brief       Call to send a Device Force Network Settings Request,
 *              DON'T USE this function unless you know exactly what you are
 *              doing and can live the unpredictable consequences.  When this
 *              message is received, the ZStack thread will force the values
 *              in the NIB then save the NIB.  It would be better to let the
 *              ZStack thread set these items as they are determined.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_DevForceNetworkSettingsReq(
    uint8_t appEntity, zstack_devForceNetworkSettingsReq_t *pReq);

/**
 * @brief       Call to send a Device Force Network Update Request,
 *              DON'T USE this function unless you know exactly what you are
 *              doing and can live the unpredictable consequences.  When this
 *              message is received, the ZStack thread will force the values
 *              in the NIB then save the NIB.  It would be better to let the
 *              ZStack thread set these items as they are determined.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_DevForceNetworkUpdateReq(
    uint8_t appEntity, zstack_devForceNetworkUpdateReq_t *pReq);

/**
 * @brief       Call to send a Device Force MAC Parameters Request,
 *              DON'T USE this function unless you know exactly what you are
 *              doing and can live the unpredictable consequences.  When this
 *              message is received, the ZStack thread will force the values
 *              in the MAC.  It would be better to let the ZStack thread set
 *              these items as they are determined.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_DevForceMacParamsReq(
    uint8_t appEntity, zstack_devForceMacParamsReq_t *pReq);

/**
 * @brief       Call to send an APS Remove Group Request,
 *              Use this command to remove a group from an endpoint.
 *
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ApsRemoveGroupReq(
    uint8_t appEntity, zstack_apsRemoveGroup_t *pReq);

/**
 * @brief       Call to send an APS Remove All Groups Request,
 *              Use this command to remove all groups from an endpoint.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ApsRemoveAllGroupsReq(
    uint8_t appEntity, zstack_apsRemoveAllGroups_t *pReq);

/**
 * @brief       Call to send an APS Find All Groups Request to the stack task
 *              Use this command to return a list of groups that
 *              exists for the given endpoint.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 * @param       pRsp - Pointer to a place to put the response message.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ApsFindAllGroupsReq(
    uint8_t appEntity, zstack_apsFindAllGroupsReq_t *pReq,
    zstack_apsFindAllGroupsRsp_t *pRsp);

/**
 * @brief       Call to send an APS Find Group Request to the stack task
 *              Use this command to return the group information (name) for
 *              a given group ID and endpoint.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 * @param       pRsp - Pointer to a place to put the response message.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ApsFindGroupReq(
    uint8_t appEntity, zstack_apsFindGroupReq_t *pReq,
    zstack_apsFindGroupRsp_t *pRsp);

/**
 * @brief       Call to send an APS Add Group Request,
 *              Use this command to add a group to an endpoint.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ApsAddGroupReq(
    uint8_t appEntity, zstack_apsAddGroup_t *pReq);

/**
 * @brief       Call to send an APS Count All Groups Request,
 *              Use this command to retrieve the number of entries in
 *              group table.  For example if endpoint 1 and 2 both belong
 *              to group 1, then the count will be 2.
 *
 * @param       appEntity - Calling thread's task ID.
 *
 * @return      number of groups
 */
extern int Zstackapi_ApsCountAllGroupsReq(uint8_t appEntity);

/**
 * @brief       Call to send an AF Register Request,
 *              Use this command to register an endpoint descriptor with
 *              the ZStack thread.  This will allow the application to
 *              send and receive data messages.  These command is your
 *              registry for AF Data Indications.<BR>
 *              This command must be sent after every boot, this information
 *              is NOT saved in the ZStack Thread's non-volitile memory.<BR>
 *              The simple descriptor, which is part of the endpoint descriptor,
 *              is used during device discovery.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_AfRegisterReq(
    uint8_t appEntity, zstack_afRegisterReq_t *pReq);

/**
 * @brief       Call to send an AF Unregister Request,
 *              Use this command to remove an endpoint descriptor from
 *              the ZStack Threads memory.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_AfUnRegisterReq(
    uint8_t appEntity, zstack_afUnRegisterReq_t *pReq);

/**
 * @brief       Call to send an AF Data Request,
 *              Use this command to send over-the-air raw data messages.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_AfDataReq(uint8_t appEntity,
                                                zstack_afDataReq_t *pReq);

/**
 * @brief       Call to send an AF InterPAN Control Request,
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_AfInterpanCtlReq(
    uint8_t appEntity, zstack_afInterPanCtlReq_t *pReq);

/**
 * @brief       Call to send an AF Config Get Request to the stack task
 *              Use this command to retrieve the AF fragmentation
 *              configuration information for the given endpoint.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 * @param       pRsp - Pointer to a place to put the response message.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_AfConfigGetReq(
    uint8_t appEntity, zstack_afConfigGetReq_t *pReq,
    zstack_afConfigGetRsp_t *pRsp);

/**
 * @brief       Call to send an AF Config Set Request,
 *              Use this command to set the AF fragmentation
 *              configuration information for the given endpoint.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_AfConfigSetReq(
    uint8_t appEntity, zstack_afConfigSetReq_t *pReq);

/**
 * @brief       Call to send a ZDO Network Address Request,
 *              Use this command to send an over-the-air Network Address
 *              Request message.  This function will not wait for the
 *              over-the-air response message, you will need to subscribe
 *              to response message by setting the has_nwkAddrRsp and
 *              nwkAddrRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              Calling this function will generate a message to ask for the
 *              16 bit address of the Remote Device based on its known IEEE
 *              address. This message is sent as a broadcast message to all
 *              devices in the network.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoNwkAddrReq(
    uint8_t appEntity, zstack_zdoNwkAddrReq_t *pReq);

/**
 * @brief       Call to send a ZDO IEEE Address Request,
 *              Use this command to send an over-the-air IEEE Address
 *              Request message.  This function will not wait for the
 *              over-the-air response message, you will need to subscribe
 *              to response message by setting the has_ieeeAddrRsp and
 *              ieeeAddrRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              Calling this function will generate a message to ask for the
 *              64 bit address of the Remote Device based on its known 16 bit
 *              network address.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoIeeeAddrReq(
    uint8_t appEntity, zstack_zdoIeeeAddrReq_t *pReq);

/**
 * @brief       Call to send a ZDO Node Descriptor Request,
 *              Use this command to send an over-the-air Node Descriptor
 *              Request message.  This function will not wait for the
 *              over-the-air response message, you will need to subscribe
 *              to response message by setting the has_nodeDescRsp and
 *              nodeDescRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              This call will build and send a Node Descriptor Request to the
 *              Remote Device specified in t he destination address field.<BR>
 *              You can send this command to this device's address to perform
 *              the command locally.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoNodeDescReq(
    uint8_t appEntity, zstack_zdoNodeDescReq_t *pReq);

/**
 * @brief       Call to send a ZDO Power Descriptor Request,
 *              Use this command to send an over-the-air Power Descriptor
 *              Request message.  This function will not wait for the
 *              over-the-air response message, you will need to subscribe
 *              to response message by setting the has_powerDescRsp and
 *              powerDescRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              You can send this command to this device's address to perform
 *              the command locally.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoPowerDescReq(
    uint8_t appEntity, zstack_zdoPowerDescReq_t *pReq);

/**
 * @brief       Call to send a ZDO Simple Descriptor Request,
 *              Use this command to send an over-the-air Simple Descriptor
 *              Request message.  This function will not wait for the
 *              over-the-air response message, you will need to subscribe
 *              to response message by setting the has_simpleDescRsp and
 *              simpleDescRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoSimpleDescReq(
    uint8_t appEntity, zstack_zdoSimpleDescReq_t *pReq);

/**
 * @brief       Call to send a ZDO Active Endpoint Request,
 *              Use this command to send an over-the-air Active Endpoint
 *              Request message.  This function will not wait for the
 *              over-the-air response message, you will need to subscribe
 *              to response message by setting the has_activeEndpointRsp and
 *              activeEndpointRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              You can send this command to this device's address to perform
 *              the command locally.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoActiveEndpointReq(
    uint8_t appEntity, zstack_zdoActiveEndpointReq_t *pReq);

/**
 * @brief       Call to send a ZDO Match Descriptor Request,
 *              Use this command to send an over-the-air Match Descriptor
 *              Request message.  This function will not wait for the
 *              over-the-air response message, you will need to subscribe
 *              to response message by setting the has_matchDescRsp and
 *              matchDescRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoMatchDescReq(
    uint8_t appEntity, zstack_zdoMatchDescReq_t *pReq);

/**
 * @brief       Call to send a ZDO Complex Descriptor Request,
 *              Use this command to send an over-the-air Complex Descriptor
 *              Request message.  This function will not wait for the
 *              over-the-air response message, you will need to subscribe
 *              to response message by setting the has_complexDescRsp and
 *              complexDescRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              You can send this command to this device's address to perform
 *              the command locally.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoComplexDescReq(
    uint8_t appEntity, zstack_zdoComplexDescReq_t *pReq);

/**
 * @brief       Call to send a ZDO User Descriptor Request,
 *              Use this command to send an over-the-air User Descriptor
 *              Request message.  This function will not wait for the
 *              over-the-air response message, you will need to subscribe
 *              to response message by setting the has_userDescRsp and
 *              userDescRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq(). <BR>
 *              You can send this command to this device's address to perform
 *              the command locally.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoUserDescReq(
    uint8_t appEntity, zstack_zdoUserDescReq_t *pReq);

/**
 * @brief       Call to send a ZDO Set BindUnbind Authentication Request,
 *              With this command the application can restrict the permissions to
 *              bind/unbinds to a certain cluster to an especific device by
 *              setting its IEEE address
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoSetBindUnbindAuthAddrReq(
    uint8_t appEntity, zstack_zdoSetBindUnbindAuthAddr_t *pReq);

/**
 * @brief       Call to send a ZDO Device Announce Request,
 *              Use this command to send an over-the-air Device Announce
 *              message.  There is no over-the-air response message.<BR>
 *              This function builds and sends a Device_annce command for
 *              Zigbee end device to notify other Zigbee devices on the network
 *              that the end device has joined or rejoined the network. The
 *              command contains the device's new 16-bit NWK address and its
 *              64-bit IEEE address, as well as the capabilities of the Zigbee
 *              device. It is sent out as broadcast message.<BR>
 *              On receipt of the Device_annce, all receivers shall check all
 *              internal references to the IEEE address supplied in the
 *              announce, and substitute the corresponding NWK address with the
 *              new one. No response will be sent back for Device_annce.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoDeviceAnnounceReq(
    uint8_t appEntity, zstack_zdoDeviceAnnounceReq_t *pReq);

/**
 * @brief       Call to send a ZDO User Descriptor Set Request,
 *              Use this command to send an over-the-air User Descriptor Set
 *              Request message.  This function will not wait for the
 *              over-the-air response message, you will need to subscribe
 *              to response message by setting the has_userDescCnf and
 *              userDescCnf fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              You can send this command to this device's address to set the
 *              local user descriptor.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoUserDescSetReq(
    uint8_t appEntity, zstack_zdoUserDescSetReq_t *pReq);

/**
 * @brief       Call to send a ZDO Server Discovery Request,
 *              Use this command to send an over-the-air Server Discovery
 *              Request message.  This function will not wait for the
 *              over-the-air response message, you will need to subscribe
 *              to response message by setting the has_serverDiscoveryRsp and
 *              serverDiscoveryRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq(). <BR>
 *              The purpose of this request is to discover the locations of a
 *              particular system server or servers as indicated by the server
 *              mask. The message is broadcast to all device with RxOnWhenIdle.
 *              Remote devices will send responses back only if a match bit is
 *              found when comparing the received server mask with the mask
 *              stored in the local node descriptor, using unicast
 *              transmission.<BR>
 *              You can send this command to this device's address to perform
 *              the command locally.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoServerDiscReq(
    uint8_t appEntity, zstack_zdoServerDiscReq_t *pReq);

/**
 * @brief       Call to send a ZDO End Device Bind Request,
 *              Use this command to send an over-the-air End Device Bind
 *              Request message.  This function will not wait for the
 *              over-the-air response message, you will need to subscribe
 *              to response message by setting the has_endDeviceBindRsp and
 *              endDeviceBindRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              Send this message to attempt a hand bind for this device. After
 *              hand binding your can send indirect (no address) message to the
 *              coordinator and the coordinator will send the message to the
 *              device that this message is bound to, or you will receive
 *              messages from your new bound device.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoEndDeviceBindReq(
    uint8_t appEntity, zstack_zdoEndDeviceBindReq_t *pReq);

/**
 * @brief       Call to send a ZDO Bind Request,
 *              Use this command to send an over-the-air Bind
 *              Request message.  This function will not wait for the
 *              over-the-air response message, you will need to subscribe
 *              to response message by setting the has_bindRsp and
 *              bindRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              Send this message to a device to build a binding record for/on
 *              that device.<BR>
 *              You can send this command to this device's address to create
 *              a local binding entry.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoBindReq(
    uint8_t       appEntity,
    zstack_zdoBindReq_t *pReq);

/**
 * @brief       Call to send a ZDO Unbind Request,
 *              Use this command to send an over-the-air Unbind
 *              Request message.  This function will not wait for the
 *              over-the-air response message, you will need to subscribe
 *              to response message by setting the has_unbindRsp and
 *              unbindRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              Send this message to a device to remove a binding record on
 *              that device.<BR>
 *              You can send this command to this device's address to unbind
 *              a local entry.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoUnbindReq(
    uint8_t         appEntity,
    zstack_zdoUnbindReq_t *pReq);

/**
 * @brief       Call to send a ZDO Mgmt Network Discovery Request,
 *              Use this command to send an over-the-air Management Network
 *              Discovery Request message.  This function will not wait for
 *              the over-the-air response message, you will need to subscribe
 *              to response message by setting the has_mgmtNwkDiscRsp and
 *              mgmtNwkDiscRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              If the destination device supports this command (optional),
 *              calling this function will generate the request for the
 *              destination device to perform a network scan and return the
 *              result in the response message.<BR>
 *              You can send this command to this device's address to perform
 *              the command locally.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoMgmtNwkDiscReq(
    uint8_t appEntity, zstack_zdoMgmtNwkDiscReq_t *pReq);

/**
 * @brief       Call to send a ZDO Mgmt LQI Request,
 *              Use this command to send an over-the-air Management LQI
 *              Discovery Request message.  This function will not wait for
 *              the over-the-air response message, you will need to subscribe
 *              to response message by setting the has_mgmtLqiRsp and
 *              mgmtLqiRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              If the destination device supports this command (optional),
 *              calling this function will generate the request for the
 *              destination device to return its neighbor list in the
 *              response message.<BR>
 *              You can send this command to this device's address to perform
 *              the command locally.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoMgmtLqiReq(
    uint8_t appEntity, zstack_zdoMgmtLqiReq_t *pReq);

/**
 * @brief       Call to send a ZDO Mgmt Routing Request to the stack task
 *              Use this command to send an over-the-air Management Routing
 *              Request message.  This function will not wait for
 *              the over-the-air response message, you will need to subscribe
 *              to response message by setting the has_mgmtRtgRsp and
 *              mgmtRtgRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              If the destination device supports this command (optional),
 *              calling this function will generate the request for the
 *              destination device to return its routing table in the
 *              response message.<BR>
 *              You can send this command to this device's address to perform
 *              the command locally.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoMgmtRtgReq(uint8_t appEntity,
                                             zstack_zdoMgmtRtgReq_t *pReq);

/**
 * @brief       Call to send a ZDO Mgmt Bind Request,
 *              Use this command to send an over-the-air Management Binding
 *              Request message.  This function will not wait for
 *              the over-the-air response message, you will need to subscribe
 *              to response message by setting the has_mgmtBindRsp and
 *              mgmtBindRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              If the destination device supports this command (optional),
 *              calling this function will generate the request for the
 *              destination device to return its binding table in the
 *              response message.<BR>
 *              You can send this command to this device's address to perform
 *              a local Mgmt Bind command.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoMgmtBindReq(
    uint8_t appEntity, zstack_zdoMgmtBindReq_t *pReq);

/**
 * @brief       Call to send a ZDO Mgmt Leave Request,
 *              Use this command to send an over-the-air Management Leave
 *              Request message.  This function will not wait for
 *              the over-the-air response message, you will need to subscribe
 *              to response message by setting the has_mgmtLeaveRsp and
 *              mgmtLeaveRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              If the destination device supports this command (optional),
 *              calling this function will generate the request for the
 *              destination device or another device to leave the network.<BR>
 *              You can send this command to this device's address to make
 *              this device leave.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoMgmtLeaveReq(
    uint8_t appEntity, zstack_zdoMgmtLeaveReq_t *pReq);

/**
 * @brief       Call to send a ZDO Mgmt Direct Join Request,
 *              Use this command to send an over-the-air Management Direct Join
 *              Request message.  This function will not wait for
 *              the over-the-air response message, you will need to subscribe
 *              to response message by setting the has_mgmtDirectJoinRsp and
 *              mgmtDirectJoinRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              If the destination device supports this command (optional),
 *              calling this function will generate the request for the
 *              destination device to direct join another device.<BR>
 *              You can send this command to this device's address to make a
 *              local direct join.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoMgmtDirectJoinReq(
    uint8_t appEntity, zstack_zdoMgmtDirectJoinReq_t *pReq);

/**
 * @brief       Call to send a ZDO Mgmt Permit Join Request,
 *              Use this command to send an over-the-air Management Permit Join
 *              Request message.  This function will not wait for
 *              the over-the-air response message, you will need to subscribe
 *              to response message by setting the has_mgmtPermitJoinRsp and
 *              mgmtPermitJoinRsp fields of
 *              [zstack_devZDOCBReq_t](@ref _zstack_devzdocbreq_t) to true and
 *              calling Zstackapi_DevZDOCBReq().<BR>
 *              If the destination device supports this command (optional),
 *              calling this function will generate the request for the
 *              destination device control permit joining.<BR>
 *              You can send this command to this device's address to locally
 *              control permit joining.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoMgmtPermitJoinReq(
    uint8_t appEntity, zstack_zdoMgmtPermitJoinReq_t *pReq);

/**
 * @brief       Call to send a ZDO Mgmt Network Update Request,
 *              This command is provided to allow updating of network
 *              configuration parameters or to request information from devices
 *              on network conditions in the local operating environment.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_ZdoMgmtNwkUpdateReq(
    uint8_t appEntity, zstack_zdoMgmtNwkUpdateReq_t *pReq);

/**
 * @brief       Call to send a Security Network Key Get Request
 *              Use this command to retrieve the active or alternate network
 *              key from the ZStack Thread.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 * @param       pRsp - Pointer to a place to put the response message.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_secNwkKeyGetReq(
    uint8_t appEntity, zstack_secNwkKeyGetReq_t *pReq,
    zstack_secNwkKeyGetRsp_t *pRsp);

/**
 * @brief       Call to send a Security Network Key Set Request,
 *              Use this command to set a network key (active or
 *              alternate).
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_secNwkKeySetReq(
    uint8_t appEntity, zstack_secNwkKeySetReq_t *pReq);

/**
 * @brief       Call to send a Security Network Key Update Request,
 *              Use this command to send an over-the-air network
 *              key update message.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_secNwkKeyUpdateReq(
    uint8_t appEntity, zstack_secNwkKeyUpdateReq_t *pReq);

/**
 * @brief       Call to send a Security Network Key Switch Request,
 *              Use this command to send an over-the-air network
 *              key switch message.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_secNwkKeySwitchReq(
    uint8_t appEntity, zstack_secNwkKeySwitchReq_t *pReq);

/**
 * @brief       Call to send a Security APS Link Key Get Request
 *              Use this command to get an APS Link key or TC Link key from
 *              the ZStack Thread's non-volatile memory.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 * @param       pRsp - Pointer to a place to put the response message.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_secApsLinkKeyGetReq(
    uint8_t appEntity, zstack_secApsLinkKeyGetReq_t *pReq,
    zstack_secApsLinkKeyGetRsp_t *pRsp);

/**
 * @brief       Call to send a Security APS Link Key Set Request
 *              Use this command to set an APS Link key or TC Link key into
 *              the ZStack Thread's non-volatile memory.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_secApsLinkKeySetReq(
    uint8_t appEntity, zstack_secApsLinkKeySetReq_t *pReq);

/**
 * @brief       Call to send a Security APS Link Key Remove Request
 *              Use this command to delete an APS Link key or TC Link key from
 *              the ZStack Thread's non-volatile memory.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_secApsLinkKeyRemoveReq(
    uint8_t appEntity, zstack_secApsLinkKeyRemoveReq_t *pReq);

/**
 * @brief       Call to send a Security APS Remove Request (to remove a
 *              a device from the network) This only
 *              works if this device is a coordinator (Trust Center).
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_secApsRemoveReq(
    uint8_t appEntity, zstack_secApsRemoveReq_t *pReq);

/**
 * @brief       Call to send a Security Setup Partner Request. This only
 *              works if this device is a coordinator (Trust Center).
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_secMgrSetupPartnerReq(
    uint8_t appEntity, zstack_secMgrSetupPartnerReq_t *pReq);

/**
 * @brief       Call to send a Security Mgr App Key Type Set Request. This only
 *              works if this device is a coordinator (Trust Center).
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_secMgrAppKeyTypeSetReq(
    uint8_t appEntity, zstack_secMgrAppKeyTypeSetReq_t *pReq);

/**
 * @brief       Call to send a Security Mgr App Key Request.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_secMgrAppKeyReq(
    uint8_t appServiceTaskId, zstack_secMgrAppKeyReq_t *pReq);

/**
 * @brief       Call to set device as NWK MANAGER
 *
 * @param       appEntity - Calling thread's task ID.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_devNwkManagerSetReq(
    uint8_t appEntity);

/**
 * @brief       Call to send a BDB Start Commissioning Request (to start the
 *              commissioning process according to the commissioning mask given).
 *
 * @param       appEntity - Calling thread's iCall entity ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
 extern zstack_ZStatusValues  Zstackapi_bdbStartCommissioningReq(
    uint8_t srcEntityID, zstack_bdbStartCommissioningReq_t *pReq );

/**
 * @brief       Call to send a BDB Get Identify Active Endpoint Request (to set the
 *              endpoint which will perform the finding and binding, either Target or
 *              Initiator)
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbSetIdentifyActiveEndpointReq(
    uint8_t srcEntityID, zstack_bdbSetIdentifyActiveEndpointReq_t *pReq );

/**
 * @brief       Call to send a BDB Get Identify Active Endpoint Request (to get the
 *              endpoint which will perform the finding and binding, either Target or
 *              Initiator)
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pRsp - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbGetIdentifyActiveEndpointReq(
    uint8_t srcEntityID, zstack_bdbGetIdentifyActiveEndpointRsp_t *pRsp );

/**
 * @brief       Call to send a BDB Stop Initiator Finding and Binding Request (to Stops
 *              Finding & Binding for initiator devices.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       none
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbStopInitiatorFindingBindingReq(
    uint8_t srcEntityID );

/**
 * @brief       Call to send a Get ZCL Frame Counter Request (to get the next ZCL frame
 *              counter for packet sequence number)
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pRsp - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_getZCLFrameCounterReq(
    uint8_t srcEntityID, zstack_getZCLFrameCounterRsp_t *pRsp );

/**
 * @brief       Call to send a BDB Zcl Identify Command Indication to process identify command
 *              in the stack. This is necessary to allow BDB callbacks and notifications to
 *              work properly.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbZclIdentifyCmdIndReq(
    uint8_t srcEntityID, zstack_bdbZCLIdentifyCmdIndReq_t *pReq );


/**
 * @brief       Call to send a BDB Set Endpoint Descriptor List to Active Endpoint (Set active
 *              endpoint selected by the application for F&B process).
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       none
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_bdbSetEpDescListToActiveEndpoint(
        uint8_t appEntity,zstack_bdbSetEpDescListToActiveEndpointRsp_t *pRsp);

/**
 * @brief       Call to send a BDB Reset Local Action Request (to call application
 *              interface to perform BDB Reset to factory new).
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       none
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbResetLocalActionReq(
    uint8_t srcEntityID );

/**
 * @brief       Call to send a BDB Set Attribute Request (to set the Base
 *              Device Behavior attributes used by the commissioning methods
 *              invoked with Zstackapi_bdbStartCommissioningReq)
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbSetAttributesReq(
    uint8_t srcEntityID, zstack_bdbSetAttributesReq_t *pReq );

/**
 * @brief       Call to send a BDB Get Attributes Request (to get the Base
 *              Device Behavior attributes used by the commissioning methods
 *              invoked with Zstackapi_bdbStartCommissioningReq)
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pRsp - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbGetAttributesReq(
    uint8_t srcEntityID, zstack_bdbGetAttributesRsp_t *pRsp );

/**
 * @brief       Call to send a BDB Get Finding and Binding Initiator Status
 *              Request (to get the remaining time and attempts to search for
 *              Finding and Binding Target devices when periodic Finding
 *              And Binding commissioning process is enabled)
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 * @param       pRsp - Pointer to the Response structure.  Make sure
 *                    the structure is zeroed.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbGetFBInitiatorStatusReq(
        uint8_t appEntity, zstack_bdbGetFBInitiatorStatusRsp_t *pRsp);

/**
 * @brief       Call to send a BDB Generate Install Code CRC Request (to generate
 *              CRC codes using as input an install code)
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 * @param       pRsp - Pointer to the Response structure.  Make sure
 *                    the structure is zeroed.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbGenerateInstallCodeCRCReq(
        uint8_t appEntity, zstack_bdbGenerateInstallCodeCRCReq_t *pReq,
        zstack_bdbGenerateInstallCodeCRCRsp_t *pRsp );

#ifdef BDB_REPORTING
/**
 * @brief       Call to send a BDB Reporting Attribute Configuration Record
 *              Default Request (to add a default configuration value for a reportable attribute)
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbRepAddAttrCfgRecordDefaultToListReq(
    uint8_t srcEntityID, zstack_bdbRepAddAttrCfgRecordDefaultToListReq_t *pReq );

/**
 * @brief       Call to send a BDB Report Changed Attribute Value Request (to
 *              notify the stack that an attribute has changed and allow the stack
 *              to validate the triggering of a reporting attribute message)
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbRepChangedAttrValueReq(
    uint8_t srcEntityID, zstack_bdbRepChangedAttrValueReq_t *pReq );

/**
 * @brief       Call to process an Incomming Configure Report command (to
 *              adjust the reporting mechanism and generate the response to
 *              this command)
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_bdbProcessInConfigReportCmd(
        uint8_t appEntity, zstack_bdbProcessInConfigReportReq_t *pReq);


/**
 * @brief       Call to process an Incomming Read Reporting Configuration (to
 *              generate a response with the current report configuration)
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_bdbProcessInReadReportCfgCmd(
        uint8_t appEntity, zstack_bdbProcessInReadReportCfgReq_t *pReq);

#endif

/**
 * @brief       Call to send a BDB Add Install Code Request (to add an install
 *              code to be used in the commissioning process of a new joining device)
 *              ZC interface only.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbAddInstallCodeReq(
    uint8_t srcEntityID, zstack_bdbAddInstallCodeReq_t *pReq );

/**
 * @brief       Call to send a BDB Touchlink Target Enable Commissioning Request (to
 *              enable the reception of Touchlink commissioning commands)
 *              ZR/ZED interface only.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbTouchLinkTargetEnableCommissioningReq(
    uint8_t srcEntityID, zstack_bdbTouchLinkTargetEnableCommissioningReq_t *pReq );

/**
 * @brief       Call to send a BDB Touchlink Target Disable Commissioning Request (to
 *              Disable the reception of commissioning commands)
 *              ZR/ZED interface only.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbTouchLinkTargetDisableCommissioningReq(
    uint8_t srcEntityID);

/**
 * @brief       Call to send a BDB Touchlink Target Get Timer Request (to get the remaining time enabled
 *              of Touchlink commissioning as Target)
 *              ZR/ZED interface only.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbTouchLinkTargetGetTimerReq(
    uint8_t srcEntityID, zstack_bdbTouchLinkTargetGetTimerRsp_t *pRsp );

/**
 * @brief       Call to send a BDB Set Active Centralized Link Key Request (to set the
 *              active centralized key to be used: default global trust center link key
 *              Or install code derived preconfigured key)
 *              ZR/ZED interface only.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbSetActiveCentralizedLinkKeyReq(
    uint8_t srcEntityID, zstack_bdbSetActiveCentralizedLinkKeyReq_t *pReq );

/**
 * @brief       Call to send a BDB CBKE TustCenter Link Key Exchange Attempt Request (to indicate
 *              to the stack the result of the Certificate Base Key Exchange, if any. If CBKE is
 *              not used, then just report not-success and default TrustCenterLinkKey exchange
 *              mechanism will be excecuted)
 *              ZR/ZED interface only.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbCBKETCLinkKeyExchangeAttemptReq(
    uint8_t srcEntityID, zstack_bdbCBKETCLinkKeyExchangeAttemptReq_t *pReq );

/**
 * @brief       Call to send a BDB Network Descriptor Free Request (to release the network
 *              descriptors of those networks that are not intended to be joined)
 *              ZR/ZED interface only.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbNwkDescFreeReq(
    uint8_t srcEntityID, zstack_bdbNwkDescFreeReq_t *pReq );


/**
 * @brief       Call to send a BDB Filter Network Descriptor Comple Request (to indicate the
 *              stack that the networks that are not of interest to the application has been
 *              removed and the remaining network descritptors must be attempted to joined)
 *              ZR/ZED interface only.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues Zstackapi_bdbFilterNwkDescComplete(
    uint8_t appEntity);

/**
 * @brief       Call to send a BDB Touchlink Set Allow Stealing Request (to allow
 *              Touchlink stealing when Touchlink commissioning is enabled as target)
 *              ZR/ZED interface only.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbTouchlinkSetAllowStealingReq(
    uint8_t srcEntityID, zstack_bdbTouchlinkSetAllowStealingReq_t *pReq );

/**
 * @brief       Call to send a BDB Touchlink Get Allow Stealing Request (to know if allow
 *              stealing is enabled or not)
 *              ZR/ZED interface only.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pRsp - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbTouchlinkGetAllowStealingReq(
    uint8_t srcEntityID, zstack_bdbTouchlinkGetAllowStealingRsp_t *pRsp );

/**
 * @brief       Call to send a BDB Recover Network Request (to instruct the
 *              joiner to try to rejoin its previews network)
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
extern zstack_ZStatusValues  Zstackapi_bdbRecoverNwkReq(
    uint8_t srcEntityID, zstack_bdbRecoverNwkRsp_t *pRsp );

/**
 * @brief       Call to free the memory used by an Indication message, messages
 *              sent asynchronously from the ZStack thread.
 *
 * @param       pMsg - Pointer to the indication message
 *
 * @return      true if processed (memory freed), false if not processed
 */
extern bool Zstackapi_freeIndMsg(void *pMsg);

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)

/**
 * Call to send a Zstackapi_gpSecRsp to the stack task
 *
 * Public function defined in zstackapi.h
 */
extern zstack_ZStatusValues Zstackapi_gpSecRsp(
    uint8_t appEntity, zstack_gpSecRsp_t *pReq);

/**
 * Call to send a Zstackapi_gpAliasConflict to the stack task
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_gpAliasConflict(
    uint8_t appEntity, zstack_gpAddrConflict_t *pReq);

/**
 * Call to send a Zstackapi_gpDecryptCommissioningKey
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_gptDecryptDataInd(
    uint8_t appEntity, zstack_gpDecryptDataInd_t *pReq, zstack_gpDecryptDataIndRsp_t* pRsp);

/**
 * Call to send a Zstackapi_gpEncryptDecryptCommissioningKey
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_gpEncryptDecryptCommissioningKey(
    uint8_t appEntity, zstack_gpEncryptDecryptCommissioningKey_t *pReq, zstack_gpEncryptDecryptCommissioningKeyRsp_t* pRsp);

/**
 * Call to send a Zstackapi_gpCommissioningSucess
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_gpCommissioningSucess(
    uint8_t appEntity, zstack_gpCommissioningSuccess_t *pReq);

/**
 * Call to send a Zstackapi_gpSendDeviceAnnounce
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_gpSendDeviceAnnounce(
    uint8_t appEntity, zstack_gpAliasNwkAddress_t *pReq);
#endif

#if ( BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE )
/**
 * Call to send a Zstackapi_tlScanReqInd
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_tlScanReqInd(
    uint8_t appEntity, zstack_tlScanReq_t *pReq);

/**
 * Call to send a Zstackapi_tlTargetNwkJoinReqInd
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_touchlinkNwkJoinReqInd(
    uint8_t appEntity, zstack_touchlinkNwkJointReq_t *pReq);

/**
 * Call to send a Zstackapi_touchlinkNwkUpdateReqInd
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_touchlinkNwkUpdateReqInd(
    uint8_t appEntity, zstack_touchlinkNwkUpdateReq_t *pReq);
#endif


#if defined BDB_TL_TARGET

/**
 * Call to send a Zstackapi_tlTargetNwkStartReqInd
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_tlTargetNwkStartReqInd(
    uint8_t appEntity, zstack_tlTargetNwkStartReq_t *pReq);

/**
 * Call to send a Zstackapi_tlTargetNwkJoinReqInd to the stack task
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_tlTargetResetToFNReqInd(uint8_t appEntity);

#endif

#if defined ( BDB_TL_INITIATOR )
/**
 * Call to send a Zstackapi_touchlinkGetScanBaseTime to the stack task
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_touchlinkGetScanBaseTime(
    uint8_t appEntity, zstack_touchlinkGetScanBaseTime_t *pReq);

/**
 * Call to send a Zstackapi_tlInitiatorScanRspInd to the stack task
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_tlInitiatorScanRspInd(
    uint8_t appEntity, zstack_touchlinkScanRsp_t *pReq);

/**
 * Call to send a Zstackapi_tlInitiatorDevInfoRspInd to the stack task
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_tlInitiatorDevInfoRspInd(
    uint8_t appEntity, zstack_touchlinkDevInfoRsp_t *pReq);

/**
 * Call to send a Zstackapi_tlInitiatorNwkStartRspInd to the stack task
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_tlInitiatorNwkStartRspInd(
    uint8_t appEntity, zstack_touchlinkNwkStartRsp_t *pReq);

/**
 * Call to send a Zstackapi_tlInitiatorNwkJoinRspInd to the stack task
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_tlInitiatorNwkJoinRspInd(
    uint8_t appEntity, zstack_touchlinkNwkJoinRsp_t *pReq);
#endif

/**
 * @brief       Call to pause/resume the device on the nwk.
 *
 * @param       appEntity - Calling thread's task ID.
 * @param       pReq - Pointer to the Request structure.  Make sure
 *                    the structure is zeroed before filling in.
 *
 * @return      zstack_ZStatusValues
 */
 extern zstack_ZStatusValues  Zstackapi_pauseResumeDeviceReq(
    uint8_t srcEntityID, zstack_pauseResumeDeviceReq_t *pReq );

#ifdef __cplusplus
}
;
#endif

#endif /* ZSTACKAPI_H */

