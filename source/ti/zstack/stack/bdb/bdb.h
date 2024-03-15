/**************************************************************************************************
  Filename:       bdb.h
  Revised:        $Date: 2016-02-25 11:51:49 -0700 (Thu, 25 Feb 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the Base Device Behavior definitions.


  Copyright 2006-2014 Texas Instruments Incorporated.

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

#ifndef BDB_H
#define BDB_H


#ifdef __cplusplus
extern "C"
{
#endif



/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "ssp.h"
#include "rom_jt_154.h"
#include "zglobals.h"
#include "af.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zd_profile.h"
#include "zd_app.h"
#include "zstack.h"
#include "bdb_interface.h"

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS)) &&           \
    !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#include "dgp_stub.h"
#endif

#if ((ZSTACK_DEVICE_BUILD == DEVICE_BUILD_ROUTER) ||                 \
     (ZSTACK_DEVICE_BUILD == DEVICE_BUILD_ENDDEVICE))
//Optional
  #if ( defined BDB_TL_INITIATOR || defined BDB_TL_TARGET )
    #define BDB_TOUCHLINK_CAPABILITY_ENABLED          TRUE
  #else
    #define BDB_TOUCHLINK_CAPABILITY_ENABLED          FALSE
  #endif
#else
  #if defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
    #error TouchLink cannot be enabled for coordinator. Please make sure not to define either BDB_TL_INITIATOR or BDB_TL_TARGET
  #endif
#endif


//Configured per device
#ifndef BDB_FINDING_BINDING_CAPABILITY_ENABLED
  #define BDB_FINDING_BINDING_CAPABILITY_ENABLED    1
#endif
#ifndef BDB_NETWORK_STEERING_CAPABILITY_ENABLED
  #define BDB_NETWORK_STEERING_CAPABILITY_ENABLED   1
#endif
#ifndef BDB_NETWORK_FORMATION_CAPABILITY_ENABLED
  #define BDB_NETWORK_FORMATION_CAPABILITY_ENABLED  1
#endif




/*********************************************************************
 * MACROS
 */

// bdbNodeCommissioningCapability MACROS
#if (ZG_BUILD_COORDINATOR_TYPE)
  #define BDB_NETWORK_STEERING_CAPABILITY      ((uint8_t)(BDB_NETWORK_STEERING_CAPABILITY_ENABLED<<0))
  #define BDB_NETWORK_FORMATION_CAPABILITY     ((uint8_t)(BDB_NETWORK_FORMATION_CAPABILITY_ENABLED<<1))
  #define BDB_FINDING_BINDING_CAPABILITY       ((uint8_t)(BDB_FINDING_BINDING_CAPABILITY_ENABLED<<2))
  #define BDB_TOUCHLINK_CAPABILITY             ((uint8_t)(0<<3))                                       //ZC cannot perform TL proceedure
#endif
#if (ZSTACK_DEVICE_BUILD == DEVICE_BUILD_ROUTER)
  #define BDB_NETWORK_STEERING_CAPABILITY      ((uint8_t)(BDB_NETWORK_STEERING_CAPABILITY_ENABLED<<0))
  #define BDB_NETWORK_FORMATION_CAPABILITY     ((uint8_t)(BDB_ROUTER_FORM_DISTRIBUTED_NWK_ENABLED<<1))
  #define BDB_FINDING_BINDING_CAPABILITY       ((uint8_t)(BDB_FINDING_BINDING_CAPABILITY_ENABLED<<2))
  #define BDB_TOUCHLINK_CAPABILITY             ((uint8_t)(BDB_TOUCHLINK_CAPABILITY_ENABLED<<3))
#endif
#if (ZSTACK_DEVICE_BUILD == DEVICE_BUILD_ENDDEVICE)
  #define BDB_NETWORK_STEERING_CAPABILITY      ((uint8_t)(BDB_NETWORK_STEERING_CAPABILITY_ENABLED<<0))
  #define BDB_NETWORK_FORMATION_CAPABILITY     ((uint8_t)(0<<1))                                       //ZED cannot form nwk
  #define BDB_FINDING_BINDING_CAPABILITY       ((uint8_t)(BDB_FINDING_BINDING_CAPABILITY_ENABLED<<2))
  #define BDB_TOUCHLINK_CAPABILITY             ((uint8_t)(BDB_TOUCHLINK_CAPABILITY_ENABLED<<3))
#endif



//Initialization structure for bdb attributes
#if ((ZSTACK_DEVICE_BUILD & DEVICE_BUILD_COORDINATOR)  &&  (ZSTACK_DEVICE_BUILD & (DEVICE_BUILD_ROUTER | DEVICE_BUILD_ENDDEVICE) ) )
#define BDB_ATTRIBUTES_DEFAULT_CONFIG {.bdbNodeJoinLinkKeyType           = BDB_DEFAULT_NODE_JOIN_LINK_KEY_TYPE,          \
                                       .bdbTCLinkKeyExchangeAttempts     = BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_ATTEMPS,     \
                                       .bdbTCLinkKeyExchangeAttemptsMax  = BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_ATTEMPS_MAX, \
	                                   .bdbTCLinkKeyExchangeMethod       = BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_METHOD,      \
                                       .bdbCommissioningGroupID          = BDB_DEFAULT_COMMISSIONING_GROUP_ID,           \
	                                   .bdbCommissioningMode             = BDB_DEFAULT_COMMISSIONING_MODE,               \
	                                   .bdbCommissioningStatus           = BDB_DEFAULT_COMMISSIONING_STATUS,             \
	                                   .bdbNodeCommissioningCapability   = BDB_DEFAULT_NODE_COMMISSIONING_CAPABILITY,    \
	                                   .bdbNodeIsOnANetwork              = BDB_DEFAULT_NODE_IS_ON_A_NETWORK,             \
	                                   .bdbPrimaryChannelSet             = BDB_DEFAULT_PRIMARY_CHANNEL_SET,              \
	                                   .bdbScanDuration                  = BDB_DEFAULT_SCAN_DURATION,                    \
	                                   .bdbSecondaryChannelSet           = BDB_DEFAULT_SECONDARY_CHANNEL_SET,            \
                                       .bdbJoinUsesInstallCodeKey        = BDB_DEFAULT_JOIN_USES_INSTALL_CODE_KEY,       \
                                       .bdbTrustCenterNodeJoinTimeout    = BDB_DEFAULT_TC_NODE_JOIN_TIMEOUT,             \
                                       .bdbTrustCenterRequireKeyExchange = BDB_DEFAULT_TC_REQUIRE_KEY_EXCHANGE}


#else
#if (ZG_BUILD_COORDINATOR_TYPE)
#define BDB_ATTRIBUTES_DEFAULT_CONFIG {.bdbJoinUsesInstallCodeKey        = BDB_DEFAULT_JOIN_USES_INSTALL_CODE_KEY,       \
                                       .bdbTrustCenterNodeJoinTimeout    = BDB_DEFAULT_TC_NODE_JOIN_TIMEOUT,             \
                                       .bdbTrustCenterRequireKeyExchange = BDB_DEFAULT_TC_REQUIRE_KEY_EXCHANGE,          \
                                       .bdbCommissioningGroupID          = BDB_DEFAULT_COMMISSIONING_GROUP_ID,           \
	                                   .bdbCommissioningMode             = BDB_DEFAULT_COMMISSIONING_MODE,               \
	                                   .bdbCommissioningStatus           = BDB_DEFAULT_COMMISSIONING_STATUS,             \
	                                   .bdbNodeCommissioningCapability   = BDB_DEFAULT_NODE_COMMISSIONING_CAPABILITY,    \
	                                   .bdbNodeIsOnANetwork              = BDB_DEFAULT_NODE_IS_ON_A_NETWORK,             \
	                                   .bdbPrimaryChannelSet             = BDB_DEFAULT_PRIMARY_CHANNEL_SET,              \
	                                   .bdbScanDuration                  = BDB_DEFAULT_SCAN_DURATION,                    \
	                                   .bdbSecondaryChannelSet           = BDB_DEFAULT_SECONDARY_CHANNEL_SET}
#else

#define BDB_ATTRIBUTES_DEFAULT_CONFIG {.bdbNodeJoinLinkKeyType           = BDB_DEFAULT_NODE_JOIN_LINK_KEY_TYPE,          \
                                       .bdbTCLinkKeyExchangeAttempts     = BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_ATTEMPS,     \
                                       .bdbTCLinkKeyExchangeAttemptsMax  = BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_ATTEMPS_MAX, \
	                                   .bdbTCLinkKeyExchangeMethod       = BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_METHOD,      \
                                       .bdbCommissioningGroupID          = BDB_DEFAULT_COMMISSIONING_GROUP_ID,           \
	                                   .bdbCommissioningMode             = BDB_DEFAULT_COMMISSIONING_MODE,               \
	                                   .bdbCommissioningStatus           = BDB_DEFAULT_COMMISSIONING_STATUS,             \
	                                   .bdbNodeCommissioningCapability   = BDB_DEFAULT_NODE_COMMISSIONING_CAPABILITY,    \
	                                   .bdbNodeIsOnANetwork              = BDB_DEFAULT_NODE_IS_ON_A_NETWORK,             \
	                                   .bdbPrimaryChannelSet             = BDB_DEFAULT_PRIMARY_CHANNEL_SET,              \
	                                   .bdbScanDuration                  = BDB_DEFAULT_SCAN_DURATION,                    \
	                                   .bdbSecondaryChannelSet           = BDB_DEFAULT_SECONDARY_CHANNEL_SET}
#endif
#endif

//Commissioning Modes
#define BDB_COMMISSIONING_MODE_IDDLE                0      // No pending commissioning procedures
#define BDB_COMMISSIONING_MODE_INITIATOR_TL         (1<<0)
#define BDB_COMMISSIONING_MODE_NWK_STEERING         (1<<1)
#define BDB_COMMISSIONING_MODE_NWK_FORMATION        (1<<2)
#define BDB_COMMISSIONING_MODE_FINDING_BINDING      (1<<3)
#define BDB_COMMISSIONING_MODE_INITIALIZATION       (1<<4)
#define BDB_COMMISSIONING_MODE_PARENT_LOST          (1<<5)

#define BDB_COMMISSIONING_MODES                     (BDB_COMMISSIONING_MODE_FINDING_BINDING | \
                                                     BDB_COMMISSIONING_MODE_NWK_FORMATION   | \
                                                     BDB_COMMISSIONING_MODE_NWK_STEERING    | \
                                                     BDB_COMMISSIONING_MODE_INITIATOR_TL    | \
                                                     BDB_COMMISSIONING_MODE_INITIALIZATION  | \
                                                     BDB_COMMISSIONING_MODE_PARENT_LOST)

#define BDB_COMMISSIONING_REJOIN_EXISTING_NETWORK_ON_STARTUP 0x00

// bdbNodeJoinLinkKeyType
#define BDB_DEFAULT_GLOBAL_TRUST_CENTER_LINK_KEY           0x00
#define BDB_DISTRIBUTED_SECURITY_GLOBAL_LINK_KEY           0x01
#define BDB_INSTALL_CODE_DERIVED_PRECONFIGURED_LINK_KEY    0x02
#define BDB_TOUCHLINK_PRECONFIGURED_LINK_KEY               0x03

#define BDB_FINDING_AND_BINDING_INITIATOR                  0x01
#define BDB_FINDING_AND_BINDING_TARGET                     0x02

#define BDB_ZIGBEE_RESERVED_ENDPOINTS_START                241
#define BDB_ZIGBEE_RESERVED_ENDPOINTS_END                  254



// New respondents require to get simple descritor request from remote device.
// If the respondent has a matching cluster a bind request is created, for which
// if IEEE addr is missing then the entry is process to get its IEEE Addr by
// sending an IEEE Addr Req
// Attempt counter also is used to designed which type of request will be send
// by the usage of the mask FINDING_AND_BINDING_MISSING_IEEE_ADDR and the
// assupmtion that the retries will not excede 36 attempts

#define FINDING_AND_BINDING_NEW_RESPONDENT       0x00
#define FINDING_AND_BINDING_MISSING_IEEE_ADDR    0x80
#define FINDING_AND_BINDING_PARENT_LOST          0x40
#define FINDING_AND_BINDING_FOR_TOUCHLINK_ADDED  0xFD  //Being added
#define FINDING_AND_BINDING_FOR_TOUCHLINK_SENDED 0xFE  //Already send
#define FINDING_AND_BINDING_RESPONDENT_COMPLETE  0xFF

 /*********************************************************************
 * CONSTANTS
 */

// Zigbee Home Automation Profile Identification
#define Z3_PROFILE_ID                                       0x0104


// Define if Touchlink Target device will use fixed or random
// channel from bdbcTLPrimaryChannelSet during commissioning
// when is Factory New (development only).
#define TOUCHLINK_FIXED_CHANNEL_ENABLE                     FALSE
#define TOUCHLINK_FIXED_CHANNEL                            TOUCHLINK_FIRST_CHANNEL

// set TOUCHLINK_CH_OFFSET to Ch_Plus_1, Ch_Plus_2 or Ch_Plus_3 to shift
// the primary channel set (development only), allowing testing of multiple
// touchlink devices without interference ONLY for testing propouses. if set
// to No_Ch_offset (default) then no shift is applied.
#define TOUCHLINK_CH_OFFSET                                No_Ch_offset

//BDB Attribute initialization constants
#define BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_ATTEMPS           0
#define BDB_DEFAULT_TC_LINK_KEY_EXCHANGE_ATTEMPS_MAX       0x03
#define BDB_DEFAULT_NODE_JOIN_LINK_KEY_TYPE                BDB_DEFAULT_GLOBAL_TRUST_CENTER_LINK_KEY
#define BDB_DEFAULT_NODE_IS_ON_A_NETWORK                   FALSE
#define BDB_DEFAULT_NODE_COMMISSIONING_CAPABILITY         (BDB_NETWORK_STEERING_CAPABILITY  | \
                                                           BDB_NETWORK_FORMATION_CAPABILITY | \
                                                           BDB_FINDING_BINDING_CAPABILITY   | \
                                                           BDB_TOUCHLINK_CAPABILITY)
#define BDB_DEFAULT_COMMISSIONING_STATUS                   BDB_COMMISSIONING_SUCCESS
#define BDB_DEFAULT_COMMISSIONING_MODE                     0x00

#define BDBC_TC_LINK_KEY_EXANGE_TIMEOUT                    5000      // 5 seconds

//Constants for CRC calculations
#define CRC_ORDER    16u
#define CRC_POLYNOM  0x1021u
#define CRC_INIT     0xffffu
#define CRC_XOR      0xffffu
#define CRC_HIGHBIT  0x8000u //for CRC_ORDER =16

// TOUCHLINK Profile Constants
#define BDBCTL_INTER_PAN_TRANS_ID_LIFETIME         8000 // 8s
#define BDBCTL_MIN_STARTUP_DELAY_TIME              2000 // 2s
#define BDBCTL_PRIMARY_CHANNEL_LIST               (0x02108800 << TOUCHLINK_CH_OFFSET)
#define BDBCTL_RX_WINDOW_DURATION                  5000 // 5s
#define BDBCTL_SCAN_TIME_BASE_DURATION             250  // 0.25s
#define BDBCTL_SECONDARY_CHANNEL_LIST             (0x07fff800 ^ BDBCTL_PRIMARY_CHANNEL_LIST) // TOUCHLINK Secondary Channels

// TOUCHLINK Channels (standard)
#define TOUCHLINK_FIRST_CHANNEL                  (11 + TOUCHLINK_CH_OFFSET)
#define TOUCHLINK_SECOND_CHANNEL                 (15 + TOUCHLINK_CH_OFFSET)
#define TOUCHLINK_THIRD_CHANNEL                  (20 + TOUCHLINK_CH_OFFSET)
#define TOUCHLINK_FOURTH_CHANNEL                 (25 + TOUCHLINK_CH_OFFSET)

#if ( TOUCHLINK_CH_OFFSET == No_Ch_offset )
#define TOUCHLINK_SECONDARY_CHANNELS_SET         {12, 13, 14, 16, 17, 18, 19, 21, 22, 23, 24, 26}
#elif ( TOUCHLINK_CH_OFFSET == Ch_Plus_1 )
#define TOUCHLINK_SECONDARY_CHANNELS_SET         {11, 13, 14, 15, 17, 18, 19, 20, 22, 23, 24, 25}
#elif ( TOUCHLINK_CH_OFFSET == Ch_Plus_2 )
#define TOUCHLINK_SECONDARY_CHANNELS_SET         {11, 12, 14, 15, 16, 18, 20, 21, 23, 24, 25, 26}
#elif ( TOUCHLINK_CH_OFFSET == Ch_Plus_3 )
#define TOUCHLINK_SECONDARY_CHANNELS_SET         {11, 12, 13, 15, 16, 17, 19, 20, 21, 22, 25, 26}
#endif

 /*********************************************************************
 * TYPEDEFS
 */

enum
{
BDB_COMMISSIONING_STATE_START_RESUME,            //Start/Resume the commissioning process according to commissionig modes
BDB_COMMISSIONING_STATE_TC_LINK_KEY_EXCHANGE,    //Perform the TC Link key exchange
BDB_COMMISSIONING_STATE_TL,                      //Perform Touchlink procedure as initiator
BDB_COMMISSIONING_STATE_JOINING,                 //Performs nwk discovery, joining attempt and nwk key reception
BDB_COMMISSIONING_STATE_STEERING_ON_NWK,         //Send mgmt permit joining
BDB_COMMISSIONING_STATE_FORMATION,               //Perform formtation procedure
BDB_COMMISSIONING_STATE_FINDING_BINDING,         //Perform Finding and binding procedure
BDB_INITIALIZATION,                              //Initialization process, for ZC/ZR means silent rejoin, for ZED nwk rejoin
BDB_PARENT_LOST,                                 //Parent lost, ask app to nwk Rejoin or giveup and reset

//Non-State related messages
BDB_TC_LINK_KEY_EXCHANGE_PROCESS,                //TC Notifications for TC link key exchange process with joining devices
BDB_NOTIFY_USER,                                 //Message to notify user about processing in BDB
BDB_ZDO_CB_MSG = 0xD3                            //To process ZDO CB Msg
};


typedef struct
{
uint32_t bdbSecondaryChannelSet;
uint32_t bdbPrimaryChannelSet;
uint16_t bdbCommissioningGroupID;
uint8_t  bdbCommissioningStatus;
uint8_t  bdbCommissioningMode;
uint8_t  bdbNodeCommissioningCapability;
uint8_t  bdbScanDuration;
bool   bdbNodeIsOnANetwork;
#if (ZG_BUILD_COORDINATOR_TYPE)
bool   bdbJoinUsesInstallCodeKey;
uint8_t  bdbTrustCenterNodeJoinTimeout;
bool   bdbTrustCenterRequireKeyExchange;
#endif
#if (ZG_BUILD_JOINING_TYPE)
uint8_t  bdbNodeJoinLinkKeyType;
uint8_t  bdbTCLinkKeyExchangeAttempts;
uint8_t  bdbTCLinkKeyExchangeAttemptsMax;
uint8_t  bdbTCLinkKeyExchangeMethod;
#endif
}bdbAttributes_t;


typedef struct respondentData
{
  afAddrType_t               data;
  uint8_t                      attempts;
  SimpleDescriptionFormat_t* SimpleDescriptor;
  struct respondentData*     pNext;
}bdbFindingBindingRespondent_t;

typedef struct
{
  uint8_t status;                  //status: BDB_TC_LK_EXCH_PROCESS_JOINING
  uint8_t extAddr[Z_EXTADDR_LEN];
  uint16_t nwkAddr;
}bdb_TCLinkKeyExchProcess_t;

typedef struct
{
  uint8_t  bdbCommissioningStatus;
  uint8_t  bdbCommissioningMode;
  uint8_t  bdbRemainingCommissioningModes;
}bdbCommissioningModeMsg_t;

typedef struct
{
  networkDesc_t *pBDBListNwk;
  uint8_t         count;
}bdbFilterNetworkDesc_t;


typedef struct bdb_joiningDeviceList_node
{
uint16_t parentAddr;
uint8_t  bdbJoiningNodeEui64[Z_EXTADDR_LEN];
uint8_t  NodeJoinTimeout;
struct bdb_joiningDeviceList_node*  nextDev;
}bdb_joiningDeviceList_t;





//BDB Events
#define BDB_NWK_STEERING_NOT_IN_NWK_SUCCESS       0x0001
#define BDB_TC_LINK_KEY_EXCHANGE_FAIL             0x0002
#define BDB_CHANGE_COMMISSIONING_STATE            0x0004
#define BDB_REPORT_TIMEOUT                        0x0080
#define BDB_FINDING_AND_BINDING_PERIOD_TIMEOUT    0x0040
#define BDB_TC_JOIN_TIMEOUT                       0x0800
#define BDB_PROCESS_TIMEOUT                       0x1000
#define BDB_IDENTIFY_TIMEOUT                      0x2000
#define BDB_RESPONDENT_PROCESS_TIMEOUT            0x4000

//Msg event status
#define BDB_MSG_EVENT_SUCCESS             0
#define BDB_MSG_EVENT_FAIL                1

enum
{
BDB_JOIN_EVENT_NWK_DISCOVERY,
BDB_JOIN_EVENT_ASSOCIATION,
BDB_JOIN_EVENT_NO_NWK_KEY,
BDB_JOIN_EVENT_OTHER,
};

enum
{
BDB_TC_LINK_KEY_EXCHANGE_NOT_ACTIVE,
BDB_REQ_TC_STACK_VERSION,
BDB_REQ_TC_LINK_KEY,
BDB_REQ_VERIFY_TC_LINK_KEY,
BDB_TC_EXCHANGE_NEXT_STATE=1,
};

typedef struct
{
  OsalPort_EventHdr hdr;
  uint8_t  buf[1];
}bdbInMsg_t;


enum
{
BDB_JOIN_STATE_NWK_DISC,
BDB_JOIN_STATE_ASSOC,
BDB_JOIN_STATE_WAITING_NWK_KEY,
};

typedef struct
{
uint8_t    bdbCommissioningState;
uint8_t    bdbTCExchangeState;
uint8_t    bdbJoinState;
uint8_t    bdb_ParentLostSavedState;      //Commissioning state to be restore after parent is found
}bdbCommissioningProcedureState_t;

typedef struct
{
  zAddrType_t dstAddr;
  uint8_t       ep;
  uint16_t      clusterId;
}bdbBindNotificationData_t;


typedef void (*bdbGCB_IdentifyTimeChange_t)( uint8_t endpoint );
typedef void (*bdbGCB_BindNotification_t)( bdbBindNotificationData_t *bindData );
typedef void (*bdbGCB_CommissioningStatus_t)(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);
typedef void (*bdbGCB_CBKETCLinkKeyExchange_t)( void );
typedef void (*bdbGCB_TCLinkKeyExchangeProcess_t) (bdb_TCLinkKeyExchProcess_t* bdb_TCLinkKeyExchProcess);
typedef void (*bdbGCB_FilterNwkDesc_t) (networkDesc_t *pBDBListNwk, uint8_t count);

/*********************************************************************
 * GLOBAL VARIABLES
 */

extern byte bdb_TaskID;

extern bdbAttributes_t bdbAttributes;

extern uint8_t touchLinkInitiator_TaskID;

extern epList_t *bdb_HeadEpDescriptorList;

extern epList_t *bdb_CurrEpDescriptorList;

extern bdbCommissioningProcedureState_t bdbCommissioningProcedureState;

extern uint8_t bdb_ZclTransactionSequenceNumber;

extern uint8_t bdb_FB_InitiatorCurrentCyclesNumber;

extern bdbFindingBindingRespondent_t *pRespondentHead;

extern bdbFindingBindingRespondent_t *pRespondentCurr;

extern bdbFindingBindingRespondent_t *pRespondentNext;

#if ( TOUCHLINK_CH_OFFSET > Ch_Plus_3 )
#error "ERROR! TOUCHLINK_CH_OFFSET can't be bigger than Ch_Plus_3"
#endif

#if defined (BDB_TL_TARGET) && defined (BDB_TL_INITIATOR)
#error "ERROR! a device cannot be Touchlink Target and Initiator at the same time"
#endif

extern bool touchLinkTargetEnabled;

extern uint8_t bdbSecureRejoinAttempts;

extern bool bdb_performingTCRejoin;

extern bool bdb_acceptNewTrustCenterLinkKey;

/*********************************************************************
 * FUNCTION MACROS
 */

#define bdb_NotifyApp(a)   bdb_SendMsg(bdb_TaskID, BDB_NOTIFY_USER, BDB_MSG_EVENT_SUCCESS,sizeof(bdbCommissioningModeMsg_t),(a))


/*********************************************************************
 * FUNCTIONS
 */
extern void bdb_reportCommissioningState(uint8_t bdbCommissioningState, bool didSuccess);
extern void bdb_setFN(void);
extern void bdb_touchlinkSendFNReset( void );
extern void bdb_setNodeIsOnANetwork(bool isOnANetwork);
extern void bdb_nwkFormationAttempt(bool didSucess);
extern void bdb_nwkDiscoveryAttempt(bool didSuccess);
extern void bdb_nwkAssocAttemt(bool didSuccess);
extern ZStatus_t bdb_rejoinNwk(void);
extern void touchLinkInitiator_ResetToFNProcedure( void );
extern void bdb_tcLinkKeyExchangeAttempt(bool didSuccess, uint8_t bdbTCExchangeState);
extern void bdb_changeTCExchangeState(uint8_t bdbTCExchangeState);
extern void bdb_SendMsg(uint8_t taskID, uint8_t toCommissioningState,uint8_t status, uint8_t len, uint8_t *buf);
extern void bdb_setNodeJoinLinkKeyType(uint8_t KeyType);
extern bdbFindingBindingRespondent_t* bdb_AddRespondentNode( bdbFindingBindingRespondent_t **pHead, zclIdentifyQueryRsp_t *pCmd );
extern void bdb_CreateRespondentList( bdbFindingBindingRespondent_t **pHead );


#if (ZG_BUILD_COORDINATOR_TYPE)
extern ZStatus_t bdb_TCAddJoiningDevice(uint16_t parentAddr, uint8_t* JoiningExtAddr);
extern void bdb_TCjoiningDeviceComplete(uint8_t* JoiningExtAddr);
#endif

ZStatus_t bdb_addInstallCode(uint8_t* pInstallCode, uint8_t* pExt);

/*
 * @brief   Register the Simple descriptor. This function also registers
 *          the profile's cluster conversion table.
 */
void bdb_RegisterSimpleDescriptor( SimpleDescriptionFormat_t *simpleDesc );

/*
 * @brief   Sends Identify query from the specified endpoint
 */
extern ZStatus_t bdb_SendIdentifyQuery( uint8_t endpoint );

/*
 * @brief   This function free reserved memory for respondent list
 */
extern void bdb_zclRespondentListClean( bdbFindingBindingRespondent_t **pHead );



/*
 * @brief   Process the respondent list by sending Simple Descriptor request to
 *          devices respondent in the list. Also send IEEE Addr Req to those
 *          device for which a bind is created buy IEEE addr is missing.
 */
extern void bdb_ProcessRespondentList( void );

/*
 * @brief   Gives the Ep Type accourding to application clusters in
 *          simple descriptor
 */
extern uint8_t bdb_zclFindingBindingEpType( endPointDesc_t *epDesc );

/*
 * @brief   Send out a Network Join Router or End Device Request command.
 *          using the selected Target.
 */
extern ZStatus_t bdb_Initiator_SendNwkJoinReq( void );

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED == 1)
/*
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Command for this application.
 */
extern void bdb_ZclIdentifyCmdInd( uint16_t identifyTime, uint8_t endpoint );

/*
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Identity Query Response Command for this
 *          application.
 */
extern void bdb_ZclIdentifyQueryCmdInd( zclIdentifyQueryRsp_t *pCmd );
#endif

/*
 * @brief   Restore nwk parameters to invalid if the device is not on a network
 */
void bdb_ClearNetworkParams(void);

/*
 * @brief       Notify bdb that connection with parent is lost
 */
void bdb_parentLost(void);

/*
 * @brief       Restore the state of child device after parent lost
 */
void bdb_NetworkRestoredResumeState(void);

/*
 * @brief   Set the endpoint list to the active endpoint selected by the application for F&B process
 */
endPointDesc_t* bdb_setEpDescListToActiveEndpoint(void);

/*
 * @brief   Clean the F&B initiator process and reports the status to bdb state machine
 */
void bdb_exitFindingBindingWStatus( uint8_t status);

/*
 * @brief   This function free reserved memory for respondent list
 */
void bdb_zclRespondentListClean( bdbFindingBindingRespondent_t **pHead );

 /*
  * @brief       Set channel and save it in Nv for joining/formation operations
  */
extern void bdb_setChannel(uint32_t channel);



/*****************************
 * GENERAL API
 */

/*
 * @brief   Event Process for the task
 */
uint32_t bdb_event_loop( byte task_id, uint32_t events );

/*
 * @brief   Initialization for the task
 */
void bdb_Init( byte task_id );

/*
 * @brief   Start the commissioning process setting the commissioning mode given.
 */
void bdb_StartCommissioning(uint8_t mode);

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
/*
 * @brief   Set the endpoint which will perform the finding and binding (either Target or Initiator)
 */
ZStatus_t bdb_SetIdentifyActiveEndpoint(uint8_t activeEndpoint);

/*
 * @brief   Get the endpoint which will perform the finding and binding (either Target or Initiator)
 */
uint8_t bdb_GetIdentifyActiveEndpoint(void);
#endif
/*
 * @brief   Stops Finding&Binding for initiator devices. The result of this process
 * is reported in bdb notifications callback.
 */
void bdb_StopInitiatorFindingBinding(void);

/*
 * @brief   Application interface to perform BDB Reset to FN.
 */
void bdb_resetLocalAction(void);

/*
 * @brief   Set the primary or secondary channel for discovery or formation procedure
 */
void bdb_setChannelAttribute(bool isPrimaryChannel, uint32_t channel);

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED == 1)
/*
 * @brief   Register an Application's Identify Time change callback function
 *          to let know the application when identify is active or not.
 */
void bdb_RegisterIdentifyTimeChangeCB( bdbGCB_IdentifyTimeChange_t pfnIdentifyTimeChange );

/*
 * @brief   Register an Application's notification callback function to let
 *          know the application when a new bind is added to the binding table.
 */
void bdb_RegisterBindNotificationCB( bdbGCB_BindNotification_t pfnBindNotification );
#endif

/*
 * @brief   Get the F&B initiator status for periodic requests. FINDING_AND_BINDING_PERIODIC_ENABLE Must be enabled to return the proper values
 */
void bdb_GetFBInitiatorStatus(uint8_t *RemainingTime, uint8_t* AttemptsLeft);

/*
 * @brief   Register a callback in which the status of the procedures done in
 *          BDB commissioning process will be reported
 */
void bdb_RegisterCommissioningStatusCB( bdbGCB_CommissioningStatus_t bdbGCB_CommissioningStatus );

/*
 * @brief   Returns the state of bdbNodeIsOnANetwork attribute
 */
bool bdb_isDeviceNonFactoryNew(void);

/*
 * @brief   Sets the commissioning groupd ID
 */
void bdb_setCommissioningGroupID(uint16_t groupID);

/*
 * @brief   Creates a CRC for the install code passed.
 */
uint16_t bdb_GenerateInstallCodeCRC(uint8_t *installCode);

/*
 * @brief   Returns the state of bdbTrustCenterRequireKeyExchange attribute
 */
bool bdb_doTrustCenterRequireKeyExchange(void);

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS)) && !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/*
 * @brief   Register a callback to receive data indication from Green Power stub
 */
void bdb_zstackTaskRegisterGPDataIndCB(GP_DataIndGCB_t dataIndCB);

/*
 * @brief   Register a callback to receive security request from Green Power stub
 */
void bdb_zstackTaskRegisterGPSecReqCB(GP_SecReqGCB_t secRecCB);

/*
 * @brief   Register a callback to check a device announce
 */
void bdb_zstackTaskRegisterGPCheckDevAnnceCB(GP_CheckAnnouncedDevice_t checkDevAnnceCB);
#endif

/*****************************
 * REPORTING ATTRIBUTES API
 */

#ifdef BDB_REPORTING
/*
 * @brief   Adds default configuration values for a Reportable Attribute Record
 */
ZStatus_t bdb_RepAddAttrCfgRecordDefaultToList(uint8_t endpoint, uint16_t cluster, uint16_t attrID, uint16_t minReportInt, uint16_t maxReportInt, uint8_t* reportableChange);

/*
 * @brief   Notify BDB reporting attribute module about the change of an
 *          attribute value to validate the triggering of a reporting attribute message.
 */
ZStatus_t bdb_RepChangedAttrValue(uint8_t endpoint, uint16_t cluster, uint16_t attrID); //newvalue must a a buffer of size 8
#endif

/*****************************
 * Trust Center API  (ZC)
 */

#if (ZG_BUILD_COORDINATOR_TYPE)

/*
 * @brief   Set BDB attribute bdbJoinUsesInstallCodeKey.
 */
void bdb_setJoinUsesInstallCodeKey(bool set);

/*
 * @brief   Set the bdb_setTCRequireKeyExchange attribute
 */
void bdb_setTCRequireKeyExchange(bool isKeyExchangeRequired);


/*
 * @brief   Register a callback to receive notifications on the joining devices
 *          and its status on TC link key exchange.
 */
void bdb_RegisterTCLinkKeyExchangeProcessCB( bdbGCB_TCLinkKeyExchangeProcess_t bdbGCB_TCLinkKeyExchangeProcess );

#endif

/*
 * bdb_addInstallCode interface.
 */
ZStatus_t bdb_addInstallCode(uint8_t* pInstallCode, uint8_t* pExt);

/*****************************
 * Joining devices API  (ZR ZED)
 */

/*
 * @brief   Enable the reception of Commissioning commands.
 */
void touchLinkTarget_EnableCommissioning( uint32_t timeoutTime );


/*
 * @brief   Disable TouchLink commissioning on a target device.
 */
void touchLinkTarget_DisableCommissioning( void );

/*
 * @brief   Get the remaining time for TouchLink on a target device.
 */
uint32_t touchLinkTarget_GetTimer( void );

#if (ZG_BUILD_JOINING_TYPE)
/*
 * @brief   Set the active centralized key to be used, Global or IC derived based on zstack_CentralizedLinkKeyModes_t
 */
ZStatus_t bdb_setActiveCentralizedLinkKey(uint8_t zstack_CentralizedLinkKeyModes, uint8_t* pKey);


/*
 * @brief   Register a callback in which the TC link key exchange procedure will
 *          be performed by application.  The result from this operation must be notified to using the
 *          bdb_CBKETCLinkKeyExchangeAttempt interface.
 *          NOTE: NOT CERTIFIABLE AT THE MOMENT OF THIS RELEASE
 */
void bdb_RegisterCBKETCLinkKeyExchangeCB( bdbGCB_CBKETCLinkKeyExchange_t bdbGCB_CBKETCLinkKeyExchange );

/*
 * @brief   Tell BDB module the result of the TC link key exchange, to try
 *          the default process or to keep going with the joining process.
 */
void bdb_CBKETCLinkKeyExchangeAttempt(bool didSucces);


/*
 * @brief   Register a callback in which the application gets the list of network
 *          descriptors got from active scan.
 *          Use bdb_nwkDescFree to release the network descriptors that are not
 *          of interest and leave those which are to be attempted.
 */
void bdb_RegisterForFilterNwkDescCB(bdbGCB_FilterNwkDesc_t bdbGCB_FilterNwkDesc);

/*
 * @brief   This function frees a network descriptor.
 */
ZStatus_t bdb_nwkDescFree(networkDesc_t* nodeDescToRemove);

/*
 * @brief   Instruct the joining device to try to join the networks
 *          remaining in the network descriptor list
 */
void bdb_tryNwkAssoc(void);

#endif
/*
 * @brief   General function to allow stealing when performing TL as target
 */
void bdb_TouchlinkSetAllowStealing( bool allow );

/*
 * @brief   General function to get allow stealing value
 */
bool bdb_TouchlinkGetAllowStealing( void );

/*
 * @brief  reinitialize the BDB state machine without power cycling
 *         the device
 */
void bdb_resetStateMachine(void);

/*****************************
 * ZED API
 */
#if (ZG_BUILD_JOINING_TYPE)

/*
 * @brief   Instruct a joiner to try to rejoin its previous network
 */
uint8_t bdb_recoverNwk(void);

#endif

#ifdef __cplusplus
}
#endif


#endif /* BDB_H */
