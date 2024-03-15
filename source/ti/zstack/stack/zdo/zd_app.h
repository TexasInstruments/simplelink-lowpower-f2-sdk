/**************************************************************************************************
  Filename:       zd_app.h
  Revised:        $Date: 2015-02-12 15:03:08 -0800 (Thu, 12 Feb 2015) $
  Revision:       $Revision: 42535 $

  Description:    This file contains the interface to the Zigbee Device Application. This is the
                  Application part that the use can change. This also contains the Task functions.


  Copyright 2004-2015 Texas Instruments Incorporated.

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
**************************************************************************************************/

#ifndef ZDAPP_H
#define ZDAPP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "zmac.h"
#include "nl_mede.h"
#include "aps.h"
#include "af.h"
#include "zd_profile.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Set this value for use in choosing a PAN to join
// modify ZDApp.c to enable this...
#define ZDO_CONFIG_MAX_BO             15

  // Task Events
#define ZDO_NETWORK_INIT          0x0001
#define ZDO_NETWORK_START         0x0002
#define ZDO_DEVICE_RESET          0x0004
#define ZDO_COMMAND_CNF           0x0008
#define ZDO_STATE_CHANGE_EVT      0x0010
#define ZDO_ROUTER_START          0x0020
#define ZDO_NEW_DEVICE            0x0040
#define ZDO_DEVICE_AUTH           0x0080
#define ZDO_NWK_UPDATE_NV         0x0100
#define ZDO_FRAMECOUNTER_CHANGE   0x0200
#define ZDO_TCLK_FRAMECOUNTER_CHANGE  0x0400
#define ZDO_APS_FRAMECOUNTER_CHANGE   0x0800
#if defined ( ZDP_BIND_VALIDATION )
#define ZDO_PENDING_BIND_REQ_EVT      0x1000
#endif
#define ZDO_PARENT_ANNCE_EVT      0x4000

// Incoming to ZDO
#define ZDO_NWK_DISC_CNF        0x01
#define ZDO_NWK_JOIN_IND        0x02
#define ZDO_NWK_JOIN_REQ        0x03
#define ZDO_TRANSPORT_KEY_IND   0x04
#define ZDO_UPDATE_DEVICE_IND   0x05
#define ZDO_REMOVE_DEVICE_IND   0x06
#define ZDO_REQUEST_KEY_IND     0x07
#define ZDO_SWITCH_KEY_IND      0x08
#define ZDO_VERIFY_KEY_IND      0x09

//  ZDO command message fields
#define ZDO_CMD_ID     0
#define ZDO_CMD_ID_LEN 1

//  ZDO security message fields
#define ZDO_TRANSPORT_KEY_IND_LEN   \
  ((uint8_t)                          \
   (sizeof(ZDO_TransportKeyInd_t) ) )

#define ZDO_UPDATE_DEVICE_IND_LEN   \
  ((uint8_t)                          \
   (sizeof(ZDO_UpdateDeviceInd_t) ) )

#define ZDO_REMOVE_DEVICE_IND_LEN   \
  ((uint8_t)                          \
   (sizeof(ZDO_RemoveDeviceInd_t) ) )

#define ZDO_REQUEST_KEY_IND_LEN   \
  ((uint8_t)                        \
   (sizeof(ZDO_RequestKeyInd_t) ) )

#define ZDO_VERIFY_KEY_IND_LEN   \
  ((uint8_t)                        \
   (sizeof(ZDO_VerifyKeyInd_t) ) )

#define ZDO_SWITCH_KEY_IND_LEN   \
  ((uint8_t)                       \
   (sizeof(ZDO_SwitchKeyInd_t) ) )

#define ZDO_CONFIRM_KEY_IND_LEN     \
  ((uint8_t)                          \
   (sizeof(APSME_ConfirmKeyReq_t) ) )

#define NWK_RETRY_DELAY                 1000   // in milliseconds

#if defined ( ZDP_BIND_VALIDATION )
#define AGE_OUT_PEND_BIND_REQ_DELAY     1000   // One second interval to age out the record

#if !defined MAX_TIME_ADDR_REQ
#define MAX_TIME_ADDR_REQ           5      // Maximum number of seconds
#endif
#endif

#define ZDO_MATCH_DESC_ACCEPT_ACTION    1   // Message field

#if !defined NUM_DISC_ATTEMPTS
#define NUM_DISC_ATTEMPTS           4
#endif

// ZDOInitDevice init modes
#define ZDO_INITDEV_CENTRALIZED  0x00
#define ZDO_INITDEV_DISTRIBUTED  0x01

// ZDOInitDevice return values
#define ZDO_INITDEV_RESTORED_NETWORK_STATE      0x00
#define ZDO_INITDEV_NEW_NETWORK_STATE           0x01
#define ZDO_INITDEV_LEAVE_NOT_STARTED           0x02

#if defined ( MANAGED_SCAN )
  // Only use in a battery powered device

  // This is the number of times a channel is scanned at the shortest possible
  // scan time (which is 30 MS (2 x 15).  The idea is to scan one channel at a
  // time (from the channel list), but scan it multiple times.
  #define MANAGEDSCAN_TIMES_PRE_CHANNEL         5
  #define MANAGEDSCAN_DELAY_BETWEEN_SCANS       150   // milliseconds

extern uint8_t zdoDiscCounter;

#endif // MANAGED_SCAN

// Use the following to delay application data request after startup.
#define ZDAPP_HOLD_DATA_REQUESTS_TIMEOUT        0 // in milliseconds

// Init ZDO, but hold and wait for application to start the joining or
// forming network
#define ZDO_INIT_HOLD_NWK_START       0xFFFF

#if !defined( MAX_NWK_FRAMECOUNTER_CHANGES )
  // The number of times the frame counter can change before
  // saving to NV
  #define MAX_NWK_FRAMECOUNTER_CHANGES    1000
#endif

#if !defined( NWK_FRAMECOUNTER_CHANGES_RESTORE_DELTA )
// Additional counts to add to the frame counter when restoring from NV
// This amount is in addition to MAX_NWK_FRAMECOUNTER_CHANGES
#define NWK_FRAMECOUNTER_CHANGES_RESTORE_DELTA    250
#endif

#define STACK_PROFILE_MAX 2

/*********************************************************************
 * TYPEDEFS
 */
typedef enum
{
  DEV_HOLD,                                // Initialized - not started automatically
  DEV_INIT,                                // Initialized - not connected to anything
  DEV_NWK_DISC,                            // Discovering PAN's to join
  DEV_NWK_JOINING,                         // Joining a PAN
  DEV_NWK_SEC_REJOIN_CURR_CHANNEL,         // ReJoining a PAN in secure mode scanning in current channel, only for end devices
  DEV_END_DEVICE_UNAUTH,                   // Joined but not yet authenticated by trust center
  DEV_END_DEVICE,                          // Started as device after authentication
  DEV_ROUTER,                              // Device joined, authenticated and is a router
  DEV_COORD_STARTING,                      // Started as Zigbee Coordinator
  DEV_ZB_COORD,                            // Started as Zigbee Coordinator
  DEV_NWK_ORPHAN,                          // Device has lost information about its parent..
  DEV_NWK_KA,                              // Device is sending KeepAlive message to its parent
  DEV_NWK_SEC_REJOIN_ALL_CHANNEL,          // ReJoining a PAN in secure mode scanning in all channels, only for end devices
  DEV_NWK_TC_REJOIN_CURR_CHANNEL,          // ReJoining a PAN in Trust center mode scanning in current channel, only for end devices
  DEV_NWK_TC_REJOIN_ALL_CHANNEL            // ReJoining a PAN in Trust center mode scanning in all channels, only for end devices
} devStates_t;

typedef enum
{
  ZDO_SUCCESS,
  ZDO_FAIL
} zdoStatus_t;


typedef struct
{
  OsalPort_EventHdr hdr;
  uint8_t       dstAddrDstEP;
  zAddrType_t dstAddr;
  uint8_t       dstAddrClusterIDLSB;
  uint8_t       dstAddrClusterIDMSB;
  uint8_t       dstAddrRemove;
  uint8_t       dstAddrEP;
} ZDO_NewDstAddr_t;

//  ZDO security message types
typedef struct
{
  OsalPort_EventHdr hdr;
  uint16_t           srcAddr;
  uint8_t            keyType;
  uint8_t            keySeqNum;
  uint8_t            key[SEC_KEY_LEN];
  uint8_t            srcExtAddr[Z_EXTADDR_LEN];
  uint8_t            initiator;
  uint8_t            secure;
} ZDO_TransportKeyInd_t;

typedef struct
{
  OsalPort_EventHdr hdr;
  uint16_t           srcAddr;
  uint8_t            devExtAddr[Z_EXTADDR_LEN];
  uint16_t           devAddr;
  uint8_t            status;
} ZDO_UpdateDeviceInd_t;

typedef struct
{
  OsalPort_EventHdr hdr;
  uint16_t           srcAddr;
  uint8_t            childExtAddr[Z_EXTADDR_LEN];
} ZDO_RemoveDeviceInd_t;

typedef struct
{
  OsalPort_EventHdr hdr;
  uint16_t           srcAddr;
  uint8_t            keyType;
  uint8_t            partExtAddr[Z_EXTADDR_LEN];
} ZDO_RequestKeyInd_t;

typedef struct
{
  OsalPort_EventHdr hdr;
  uint16_t           srcAddr;
  uint8_t            keySeqNum;
} ZDO_SwitchKeyInd_t;

typedef struct
{
  OsalPort_EventHdr hdr;
  uint16_t           srcAddr;
  uint8_t            keyType;
  uint8_t            extAddr[Z_EXTADDR_LEN];
  uint8_t            keyHash[SEC_KEY_LEN];
  uint8_t            verifyKeyStatus;
} ZDO_VerifyKeyInd_t;

typedef struct
{
  OsalPort_EventHdr hdr;
  uint16_t nwkAddr;
  uint8_t numInClusters;
  uint16_t *pInClusters;
  uint8_t numOutClusters;
  uint16_t *pOutClusters;
} ZDO_MatchDescRspSent_t;

typedef struct
{
  OsalPort_EventHdr hdr;
  uint16_t shortAddr;
} ZDO_AddrChangeInd_t;

/* ZDO Indication Callback Registration */
typedef void* (*pfnZdoCb)( void *param );

/* ZDO Indication callback ID */
enum
{
  ZDO_SRC_RTG_IND_CBID,
  ZDO_CONCENTRATOR_IND_CBID,
  ZDO_NWK_DISCOVERY_CNF_CBID,
  ZDO_BEACON_NOTIFY_IND_CBID,
  ZDO_JOIN_CNF_CBID,
  ZDO_LEAVE_CNF_CBID,
  ZDO_LEAVE_IND_CBID,
  ZDO_PERMIT_JOIN_CBID,
  ZDO_TC_DEVICE_CBID,
  MAX_ZDO_CB_FUNC               // Must be at the bottom of the list
};

typedef struct
{
  uint16_t srcAddr;
  uint8_t  relayCnt;
  uint16_t *pRelayList;
} zdoSrcRtg_t;

typedef struct
{
  uint16_t nwkAddr;
  uint8_t  *extAddr;
  uint8_t  pktCost;
} zdoConcentratorInd_t;

/* Keep the same format as NLME_beaconInd_t */
typedef struct
{
  uint16_t sourceAddr;  /* Short address of the device sends the beacon */
  uint16_t panID;
  uint8_t  logicalChannel;
  uint8_t	 permitJoining;
  uint8_t	 routerCapacity;
  uint8_t	 deviceCapacity;
  uint8_t  protocolVersion;
  uint8_t  stackProfile ;
  uint8_t	 LQI ;
  uint8_t  depth ;
  uint8_t  updateID;
  uint8_t  extendedPanID[8];
} zdoBeaconInd_t;

typedef struct
{
  uint8_t  status;
  uint16_t deviceAddr;
  uint16_t parentAddr;
} zdoJoinCnf_t;

typedef struct
{
  uint8_t       srcAddress[Z_EXTADDR_LEN];
  uint8_t       srcEndpoint;
  uint16_t      clusterID;
  zAddrType_t dstAddress;
  uint8_t       dstEndpoint;
} ZDO_BindUnbindReq_t;

typedef struct
{
  uint16_t nwkAddr;
  uint8_t extAddr[Z_EXTADDR_LEN];
  uint16_t parentAddr;
} ZDO_TC_Device_t;

#if defined ( ZDP_BIND_VALIDATION )
typedef struct
{
  ZDO_BindUnbindReq_t bindReq;
  zAddrType_t         srcAddr;
  uint8_t               securityUse;
  uint8_t               transSeq;
  uint8_t               age;
} ZDO_PendingBindReq_t;
#endif

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint8_t ZDAppTaskID;
extern uint8_t nwkStatus;
extern devStates_t devState;

/* Always kept up to date by the network state changed logic, so use this addr
 * in function calls the require my network address as one of the parameters.
 */
extern zAddrType_t ZDAppNwkAddr;
extern uint8_t saveExtAddr[];  // Loaded with value by ZDApp_Init().

extern uint8_t zdappMgmtNwkDiscRspTransSeq;
extern uint8_t zdappMgmtNwkDiscReqInProgress;
extern zAddrType_t zdappMgmtNwkDiscRspAddr;
extern uint8_t zdappMgmtNwkDiscStartIndex;
extern uint8_t zdappMgmtSavedNwkState;

extern uint8_t ZDO_UseExtendedPANID[Z_EXTADDR_LEN];

extern uint32_t runtimeChannel;

/*********************************************************************
 * FUNCTIONS - API
 */

extern void ZDO_AddrChangeIndicationCB( uint16_t newAddr );

/*********************************************************************
 * Task Level Control
 */
  /*
   * ZdApp Task Initialization Function
   */
  extern void ZDApp_Init( uint8_t task_id );

  /*
   * ZdApp Task Event Processing Function
   */
  extern uint32_t ZDApp_event_loop( uint8_t task_id, uint32_t events );

/*********************************************************************
 * Application Level Functions
 */

/*
 *  Start the device in the network.  This function will read
 *   ZCD_NV_STARTUP_OPTION (NV item) to determine whether or not to
 *   restore the network state of the device.
 *
 *  startDelay - timeDelay to start device (in milliseconds).
 *      There is a jitter added to this delay:
 *              ((NWK_START_DELAY + startDelay)
 *              + (OsalPort_rand() & EXTENDED_JOINING_RANDOM_MASK))
 *
 *  NOTE:   If the application would like to force a "new" join, the
 *          application should set the ZCD_STARTOPT_DEFAULT_NETWORK_STATE
 *          bit in the ZCD_NV_STARTUP_OPTION NV item before calling
 *          this function.
 *
 *  returns:
 *    ZDO_INITDEV_RESTORED_NETWORK_STATE  - The device's network state was
 *          restored.
 *    ZDO_INITDEV_NEW_NETWORK_STATE - The network state was initialized.
 *          This could mean that ZCD_NV_STARTUP_OPTION said to not restore, or
 *          it could mean that there was no network state to restore.
 */
#define ZDOInitDevice(a)  ZDOInitDeviceEx(a,ZDO_INITDEV_CENTRALIZED)

/*
 *  Start the device in the network.  This function will read
 *   ZCD_NV_STARTUP_OPTION (NV item) to determine whether or not to
 *   restore the network state of the device.
 *
 *  startDelay - timeDelay to start device (in milliseconds).
 *      There is a jitter added to this delay:
 *              ((NWK_START_DELAY + startDelay)
 *              + (OsalPort_rand() & EXTENDED_JOINING_RANDOM_MASK))
 *
 *  mode - ZDO_INITDEV_CENTRALIZED or ZDO_INITDEV_DISTRIBUTED to specify
 *         which mode should the device start with (only has effect on Router devices)
 *
 *
 *  NOTE:   If the application would like to force a "new" join, the
 *          application should set the ZCD_STARTOPT_DEFAULT_NETWORK_STATE
 *          bit in the ZCD_NV_STARTUP_OPTION NV item before calling
 *          this function.
 *
 *  returns:
 *    ZDO_INITDEV_RESTORED_NETWORK_STATE  - The device's network state was
 *          restored.
 *    ZDO_INITDEV_NEW_NETWORK_STATE - The network state was initialized.
 *          This could mean that ZCD_NV_STARTUP_OPTION said to not restore, or
 *          it could mean that there was no network state to restore.
 */
uint8_t ZDOInitDeviceEx( uint16_t startDelay, uint8_t mode);


/*
 * Sends an osal message to ZDApp with parameter as the msg data byte.
 */
extern void ZDApp_SendEventMsg( uint8_t cmd, uint8_t len, uint8_t *buf );

/*
 * Start the network formation process
 *    delay - millisecond wait before
 */
extern void ZDApp_NetworkInit( uint16_t delay );

/*
 * Request a network discovery
 */
extern ZStatus_t ZDApp_NetworkDiscoveryReq( uint32_t scanChannels, uint8_t scanDuration);

/*
 * Request the device to join a parent on a network
 */
extern ZStatus_t ZDApp_JoinReq( uint8_t channel, uint16_t panID,
                                uint8_t *extendedPanID, uint16_t chosenParent,
                                uint8_t parentDepth, uint8_t stackProfile);

#if defined ( ZDP_BIND_VALIDATION )
/*
 * Find an empty slot to store pending Bind Request
 */
extern ZDO_PendingBindReq_t *ZDApp_GetEmptyPendingBindReq( void );
#endif

/*********************************************************************
 * Callback FUNCTIONS - API
 */
/*********************************************************************
 * Call Back Functions from NWK  - API
 */

/*
 * ZDO_NetworkDiscoveryConfirmCB - scan results
 */
extern ZStatus_t ZDO_NetworkDiscoveryConfirmCB( uint8_t status );

/*
 * ZDO_NetworkFormationConfirm - results of the request to
 *              initialize a coordinator in a network
 */
extern void ZDO_NetworkFormationConfirmCB( ZStatus_t Status );

/*
 * ZDApp_beaconIndProcessing - processes the incoming beacon
 *              indication.
 */
extern void ZDO_beaconNotifyIndCB( NLME_beaconInd_t *beacon );

/*
 * ZDO_JoinConfirmCB - results of its request to join itself or another
 *              device to a network
 */
extern void ZDO_JoinConfirmCB( uint16_t PanId, ZStatus_t Status );

/*
 * ZDO_JoinIndicationCB - notified of a remote join request
 */
ZStatus_t ZDO_JoinIndicationCB(uint16_t ShortAddress, uint8_t *ExtendedAddress,
                                 uint8_t CapabilityFlags, uint8_t type);

/*
 * ZDO_ConcentratorIndicationCB - notified of a concentrator existence
 */
extern void ZDO_ConcentratorIndicationCB( uint16_t nwkAddr, uint8_t *extAddr, uint8_t pktCost );

/*
 * ZDO_StartRouterConfirm -  results of the request to
 *              start functioning as a router in a network
 */
extern void ZDO_StartRouterConfirmCB( ZStatus_t Status );

/*
 * ZDO_LeaveCnf - results of the request for this or a child device
 *                to leave
 */
extern void ZDO_LeaveCnf( NLME_LeaveCnf_t* cnf );

/*
 * ZDO_LeaveInd - notified of a remote leave request or indication
 */
extern void ZDO_LeaveInd( NLME_LeaveInd_t* ind );

/*
 * ZDO_SyncIndicationCB - notified of sync loss with parent
 */
extern void ZDO_SyncIndicationCB( uint8_t type, uint16_t shortAddr );

/*
 * ZDO_ManytoOneFailureIndicationCB - notified a many-to-one route failure
 */
extern void ZDO_ManytoOneFailureIndicationCB( void );

/*
 * ZDO_PollConfirmCB - notified of poll confirm
 */
extern void ZDO_PollConfirmCB( uint8_t status );

/*
 * ZDO_NetworkStatusCB - notified of received network status messages.
 */
extern void ZDO_NetworkStatusCB( uint16_t nwkDstAddr, uint8_t statusCode, uint16_t dstAddr );

/*
 * ZDO_PermitJoinCB - When there is a change in Permit Join status
 */
extern void ZDO_PermitJoinCB( uint8_t duration );




/*********************************************************************
 * Call Back Functions from Security  - API
 */
extern ZStatus_t ZDO_UpdateDeviceIndication( uint8_t *extAddr, uint8_t status );



/*
 * ZDApp_InMsgCB - Allow the ZDApp to handle messages that are not supported
 */
extern void ZDApp_InMsgCB( zdoIncomingMsg_t *inMsg );

extern void ZDO_StartRouterConfirm( ZStatus_t Status );

/*********************************************************************
 * Call Back Functions from Apllication  - API
 */
/*
 * ZDO_NwkUpdateCB - Network state info has changed
 */
extern void ZDApp_NwkStateUpdateCB( void );

/*********************************************************************
 * ZDO Control Functions
 */

/*
 * ZDApp_ChangeMatchDescRespPermission
 *    - Change the Match Descriptor Response permission.
 */
extern void ZDApp_ChangeMatchDescRespPermission( uint8_t endpoint, uint8_t action );

/*
 * ZDApp_SaveNwkKey
 *     - Save off the Network key information.
 */
extern void ZDApp_SaveNwkKey( void );

/*
 * ZDApp_ResetNwkKey
 *    - Re initialize the NV Nwk Key
 */
extern void ZDApp_ResetNwkKey( void );

/*
 * ZDApp_AnnounceNewAddress
 *   - Announce a new address
 */
extern void ZDApp_AnnounceNewAddress( void );

/*
 * ZDApp_SendParentAnnce
 *   - Send Parent Announce message
 */
extern void ZDApp_SendParentAnnce( void );

/*
 * ZDApp_NVUpdate - Initiate an NV update
 */
extern void ZDApp_NVUpdate( void );

/*
 * Callback from network layer when coordinator start has a conflict with
 * an existing PAN ID.
 */
extern uint16_t ZDApp_CoordStartPANIDConflictCB( uint16_t panid );

/*
 * ZDApp_PauseNwk
 *    - Pauses the network of the device.
 */
extern void ZDApp_PauseNwk();

/*
 * ZDApp_ResumeNwk
 *    - Resume the network of the device the network has been paused
 *    by ZDApp_PauseNwk.
 */
extern void ZDApp_ResumeNwk();

/*
 * ZDApp_LeaveReset
 *    - Setup a device reset due to a leave indication/confirm
 */
extern void ZDApp_LeaveReset( uint8_t ra );

/*
 * ZDApp_LeaveCtrlReset
 *    - Re-initialize the leave control logic
 */
extern void ZDApp_LeaveCtrlReset( void );

/*
 * ZDApp_DeviceConfigured
 *    - Check to see if the local device is configured
 */
extern uint8_t ZDApp_DeviceConfigured( void );

/*
 * ZDApp_ForceConcentratorChange
 *    - reset zgConcentratorEnable and zgConcentratorDiscoveryTime from NV and set nwk event.
 */
extern void ZDApp_ForceConcentratorChange( void );

/*********************************************************************
 * @fn          ZDApp_SecInit
 *
 * @brief       ZDApp initialize security.
 *
 * @param       state - device initialization state
 *
 * @return      none
 */
extern void ZDApp_SecInit( uint8_t state );

/*********************************************************************
 * @fn          ZDO_SrcRtgIndCB
 *
 * @brief       This function notifies the ZDO available src route record received.
 *
 * @param       srcAddr - source address of the source route
 * @param       relayCnt - number of devices in the relay list
 * @param       relayList - relay list of the source route
 *
 * @return      none
 */
extern void ZDO_SrcRtgIndCB (uint16_t srcAddr, uint8_t relayCnt, uint16_t* pRelayList );

/*********************************************************************
 * @fn          ZDO_RegisterForZdoCB
 *
 * @brief       Call this function to register the higher layer (for
 *              example, the Application layer or MT layer) with ZDO
 *              callbacks to get notified of some ZDO indication like
 *              existence of a concentrator or receipt of a source
 *              route record.
 *
 * @param       indID - ZDO Indication ID
 * @param       pFn   - Callback function pointer
 *
 * @return      ZSuccess - successful, ZInvalidParameter if not
 */
extern ZStatus_t ZDO_RegisterForZdoCB( uint8_t indID, pfnZdoCb pFn );

/*********************************************************************
 * @fn          ZDO_DeregisterForZdoCB
 *
 * @brief       Call this function to de-register the higher layer (for
 *              example, the Application layer or MT layer) with ZDO
 *              callbacks to get notified of some ZDO indication like
 *              existence of a concentrator or receipt of a source
 *              route record.
 *
 * @param       indID - ZDO Indication ID
 *
 * @return      ZSuccess - successful, ZInvalidParameter if not
 */
extern ZStatus_t ZDO_DeregisterForZdoCB( uint8_t indID );

/*********************************************************************
 * @fn          ZDApp_ChangeState
 *
 * @brief       Call this function to change the device state.
 *
 * @param       state - new state
 *
 * @return      none
 */
extern void ZDApp_ChangeState( devStates_t state );

/*
 * @brief   Restore the network frame counter associated to this ExtPanID and
 *          increment it if found. This can only happens once per reset
 *
 *    returns  none
 */
extern void ZDApp_RestoreNwkSecMaterial(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZDOBJECT_H */
