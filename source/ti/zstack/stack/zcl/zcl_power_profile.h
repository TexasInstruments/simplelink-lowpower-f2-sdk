/**************************************************************************************************
  Filename:       zcl_power_profile.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Power Profile definitions.


  Copyright (c) 2019, Texas Instruments Incorporated
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  *  Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

  *  Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

  *  Neither the name of Texas Instruments Incorporated nor the names of
      its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/

#ifndef ZCL_POWER_PROFILE_H
#define ZCL_POWER_PROFILE_H


#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ti_zstack_config.h"

#ifdef ZCL_POWER_PROFILE
/******************************************************************************
 * CONSTANTS
 */

/*****************************************/
/***  Power Profile Cluster Attributes ***/
/*****************************************/

// Server Attributes
#define ATTRID_POWER_PROFILE_TOTAL_PROFILE_NUM                                        0x0000  // M, R, uint8_t
#define ATTRID_POWER_PROFILE_MULTIPLE_SCHEDULING                                      0x0001  // M, R, BOOLEAN
#define ATTRID_POWER_PROFILE_ENERGY_FORMATTING                                        0x0002  // M, R, 8-BIT BITMAP
#define ATTRID_POWER_PROFILE_ENERGY_REMOTE                                            0x0003  // M, R, BOOLEAN
#define ATTRID_POWER_PROFILE_SCHEDULE_MODE                                            0x0004  // M, R/W, 8-BIT BITMAP

// Server Attribute Defaults
#define ATTR_DEFAULT_POWER_PROFILE_TOTAL_PROFILE_NUM                                  1
#define ATTR_DEFAULT_POWER_PROFILE_MULTIPLE_SCHEDULING                                0x00
#define ATTR_DEFAULT_POWER_PROFILE_ENERGY_FORMATTING                                  0x01
#define ATTR_DEFAULT_POWER_PROFILE_ENERGY_REMOTE                                      0x00
#define ATTR_DEFAULT_POWER_PROFILE_SCHEDULE_MODE                                      0x00

// Server commands received (Client-to-Server in ZCL Header)
#define COMMAND_POWER_PROFILE_POWER_PROFILE_REQUEST                                   0x00  // M, powerProfileID
#define COMMAND_POWER_PROFILE_POWER_PROFILE_STATE_REQUEST                             0x01  // M, no payload
#define COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE_RESPONSE                        0x02  // M, zclPowerProfileGetPowerProfilePriceRsp_t
#define COMMAND_POWER_PROFILE_GET_OVERALL_SCHEDULE_PRICE_RESPONSE                     0x03  // M, zclPowerProfileGetOverallSchedulePriceRsp_t
#define COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_NOTIFICATION                     0x04  // M, zclPowerProfileEnergyPhasesSchedule_t
#define COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_RESPONSE                         0x05  // M, zclPowerProfileEnergyPhasesSchedule_t
#define COMMAND_POWER_PROFILE_POWER_PROFILE_SCHEDULE_CONSTRAINTS_REQUEST              0x06  // M, powerProfileID
#define COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_REQUEST                    0x07  // M, powerProfileID
#define COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE_EXTENDED_RESPONSE               0x08  // M, zclPowerProfileGetPowerProfilePriceExtRsp_t

// Server commands generated (Server-to-Client in ZCL Header)
#define COMMAND_POWER_PROFILE_POWER_PROFILE_NOTIFICATION                              0x00  // M, zclPowerProfileNotification_t
#define COMMAND_POWER_PROFILE_POWER_PROFILE_RESPONSE                                  0x01  // M, zclPowerProfileRsp_t
#define COMMAND_POWER_PROFILE_POWER_PROFILE_STATE_RESPONSE                            0x02  // M, zclPowerProfileStateRsp_t
#define COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE                                 0x03  // O, powerProfileID
#define COMMAND_POWER_PROFILE_POWER_PROFILE_STATE_NOTIFICATION                        0x04  // M, zclPowerProfileStateNotification_t
#define COMMAND_POWER_PROFILE_GET_OVERALL_SCHEDULE_PRICE                              0x05  // O, no payload
#define COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_REQUEST                          0x06  // M, powerProfileID
#define COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_RESPONSE                   0x07  // M, zclPowerProfileEnergyPhasesSchedule_t
#define COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_NOTIFICATION               0x08  // M, zclPowerProfileEnergyPhasesSchedule_t
#define COMMAND_POWER_PROFILE_POWER_PROFILE_SCHEDULE_CONSTRAINTS_NOTIFICATION         0x09  // M, zclPowerProfileScheduleConstraintsNotification_t
#define COMMAND_POWER_PROFILE_POWER_PROFILE_SCHEDULE_CONSTRAINTS_RESPONSE             0x0A  // M, zclPowerProfileScheduleConstraintsRsp_t
#define COMMAND_POWER_PROFILE_GET_POWER_PROFILE_PRICE_EXTENDED                        0x0B  // O, zclPowerProfileGetPowerProfilePriceExt_t

// PowerProfileState enumeration field
#define POWER_PROFILE_IDLE                        0x00  // The PP is not defined in its parameters
#define POWER_PROFILE_PROGRAMMED                  0x01  // The PP is defined in its parameters but without a scheduled time reference
#define ENERGY_PHASE_RUNNING                      0x03  // An energy phase is running
#define ENERGY_PHASE_PAUSED                       0x04  // The current energy phase is paused
#define ENERGY_PHASE_WAITING_TO_START             0x05  // The Power Profile is in between two energy phases (see spec for details)
#define ENERGY_PHASE_WAITING_PAUSED               0x06  // The Power Profile is set to pause when being in the ENERGY_PHASE_WAITING_TO_START state
#define POWER_PROFILE_ENDED                       0x07  // The whole Power Profile is terminated

/*******************************************************************************
 * TYPEDEFS
 */

/*** ZCL Power Profile Cluster: Get Power Profile Price Response payload ***/
typedef struct
{
  uint8_t powerProfileID;
  uint16_t currency;
  uint32_t price;
  uint8_t priceTrailingDigit;
} zclPowerProfileGetPowerProfilePriceRsp_t;

// Duplicate structure for Get Power Profile Price Extended Response cmd
typedef zclPowerProfileGetPowerProfilePriceRsp_t zclPowerProfileGetPowerProfilePriceExtRsp_t;

/*** ZCL Power Profile Cluster: Power Profile Get Overall Schedule Price Response payload ***/
typedef struct
{
  uint16_t currency;
  uint32_t price;
  uint8_t priceTrailingDigit;
} zclPowerProfileGetOverallSchedulePriceRsp_t;

/*** ZCL Power Profile Cluster: Energy Phases Schedule Notification and Response payloads ***/
typedef struct
{
  uint8_t energyPhaseID;
  uint16_t scheduledTime;
} scheduledPhasesRecord_t;

typedef struct
{
  uint8_t powerProfileID;
  uint8_t numOfScheduledPhases;
  scheduledPhasesRecord_t *pScheduledPhasesRecord;   // variable length array based off numOfScheduledPhases
} zclPowerProfileEnergyPhasesSchedule_t;

/*** Structures for Server Generated Commands ***/

/*** ZCL Power Profile Cluster: Power Profile Notification and Response payloads ***/
typedef struct
{
  uint8_t energyPhaseID;
  uint8_t macroPhaseID;
  uint16_t expectedDuration;
  uint16_t peakPower;
  uint16_t energy;
  uint16_t maxActivationDelay;
} transferredPhasesRecord_t;

typedef struct
{
  uint8_t totalProfileNum;
  uint8_t powerProfileID;
  uint8_t numOfTransferredPhases;
  transferredPhasesRecord_t *pTransferredPhasesRecord;  // variable length array based off of numOfTransferredPhases
} zclPowerProfile_t;

// Duplicate structure for multiple commands
typedef zclPowerProfile_t zclPowerProfileNotification_t;
typedef zclPowerProfile_t zclPowerProfileRsp_t;

/*** ZCL Power Profile Cluster: Power Profile State Response and Notification payloads ***/
typedef struct
{
  uint8_t powerProfileID;
  uint8_t energyPhaseID;
  bool powerProfileRemoteControl;
  uint8_t powerProfileState;          // e.g. POWER_PROFILE_IDLE
} powerProfileStateRecord_t;

typedef struct
{
  uint8_t powerProfileCount;
  powerProfileStateRecord_t *pPowerProfileStateRecord;   // variable length array based off of powerProfileCount
} zclPowerProfileState_t;

// Duplicate structure for multiple commands
typedef zclPowerProfileState_t zclPowerProfileStateRsp_t;
typedef zclPowerProfileState_t zclPowerProfileStateNotification_t;

/*** ZCL Power Profile Cluster: Power Profile Schedule Constraints Notification and Response payloads ***/
typedef struct
{
  uint8_t powerProfileID;
  uint16_t startAfter;
  uint16_t stopBefore;
} zclPowerProfileScheduleConstraints_t;

// Duplicate structure for multiple commands
typedef zclPowerProfileScheduleConstraints_t zclPowerProfileScheduleConstraintsNotification_t;
typedef zclPowerProfileScheduleConstraints_t zclPowerProfileScheduleConstraintsRsp_t;

/*** ZCL Power Profile Cluster: Get Power Profile Price Extended payload ***/
typedef struct
{
  uint8_t options;
  uint8_t powerProfileID;
  uint16_t powerProfileStartTime;
} zclPowerProfileGetPowerProfilePriceExt_t;


typedef ZStatus_t (*zclPower_Profile_PowerProfileReq_t)( uint8_t powerProfileID, afAddrType_t *pSrcAddr, uint8_t transSeqNum );
typedef ZStatus_t (*zclPower_Profile_PowerProfileStateReq_t)( afAddrType_t *pSrcAddr, uint8_t transSeqNum );
typedef ZStatus_t (*zclPower_Profile_GetPowerProfilePriceRsp_t)( zclPowerProfileGetPowerProfilePriceRsp_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_GetOverallSchedulePriceRsp_t)( zclPowerProfileGetOverallSchedulePriceRsp_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_EnergyPhasesScheduleNotification_t)( zclPowerProfileEnergyPhasesSchedule_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_EnergyPhasesScheduleRsp_t)( zclPowerProfileEnergyPhasesSchedule_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_PowerProfileScheduleConstraintsReq_t)( uint8_t powerProfileID, afAddrType_t *pSrcAddr, uint8_t transSeqNum );
typedef ZStatus_t (*zclPower_Profile_EnergyPhasesScheduleStateReq_t)( uint8_t powerProfileID, afAddrType_t *pSrcAddr, uint8_t transSeqNum );
typedef ZStatus_t (*zclPower_Profile_GetPowerProfilePriceExtRsp_t)( zclPowerProfileGetPowerProfilePriceExtRsp_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_PowerProfileNotification_t)( zclPowerProfileNotification_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_PowerProfileRsp_t)( zclPowerProfileRsp_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_PowerProfileStateRsp_t)( zclPowerProfileStateRsp_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_GetPowerProfilePrice_t)( uint8_t powerProfileID, afAddrType_t *pSrcAddr, uint8_t transSeqNum );
typedef ZStatus_t (*zclPower_Profile_PowerProfileStateNotification_t)( zclPowerProfileStateNotification_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_GetOverallSchedulePrice_t)( afAddrType_t *pSrcAddr, uint8_t transSeqNum );
typedef ZStatus_t (*zclPower_Profile_EnergyPhasesScheduleReq_t)( uint8_t powerProfileID, afAddrType_t *pSrcAddr, uint8_t transSeqNum );
typedef ZStatus_t (*zclPower_Profile_EnergyPhasesScheduleStateRsp_t)( zclPowerProfileEnergyPhasesSchedule_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_EnergyPhasesScheduleStateNotification_t)( zclPowerProfileEnergyPhasesSchedule_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_PowerProfileScheduleConstraintsNotification_t)( zclPowerProfileScheduleConstraintsNotification_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_PowerProfileScheduleConstraintsRsp_t)( zclPowerProfileScheduleConstraintsRsp_t *pCmd );
typedef ZStatus_t (*zclPower_Profile_GetPowerProfilePriceExt_t)( zclPowerProfileGetPowerProfilePriceExt_t *pCmd, afAddrType_t *pSrcAddr, uint8_t transSeqNum );


// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclPower_Profile_PowerProfileReq_t                                pfnPowerProfile_PowerProfileReq;
  zclPower_Profile_PowerProfileStateReq_t                           pfnPowerProfile_PowerProfileStateReq;
  zclPower_Profile_GetPowerProfilePriceRsp_t                        pfnPowerProfile_GetPowerProfilePriceRsp;
  zclPower_Profile_GetOverallSchedulePriceRsp_t                     pfnPowerProfile_GetOverallSchedulePriceRsp;
  zclPower_Profile_EnergyPhasesScheduleNotification_t               pfnPowerProfile_EnergyPhasesScheduleNotification;
  zclPower_Profile_EnergyPhasesScheduleRsp_t                        pfnPowerProfile_EnergyPhasesScheduleRsp;
  zclPower_Profile_PowerProfileScheduleConstraintsReq_t             pfnPowerProfile_PowerProfileScheduleConstraintsReq;
  zclPower_Profile_EnergyPhasesScheduleStateReq_t                   pfnPowerProfile_EnergyPhasesScheduleStateReq;
  zclPower_Profile_GetPowerProfilePriceExtRsp_t                     pfnPowerProfile_GetPowerProfilePriceExtRsp;
  zclPower_Profile_PowerProfileNotification_t                       pfnPowerProfile_PowerProfileNotification;
  zclPower_Profile_PowerProfileRsp_t                                pfnPowerProfile_PowerProfileRsp;
  zclPower_Profile_PowerProfileStateRsp_t                           pfnPowerProfile_PowerProfileStateRsp;
  zclPower_Profile_GetPowerProfilePrice_t                           pfnPowerProfile_GetPowerProfilePrice;
  zclPower_Profile_PowerProfileStateNotification_t                  pfnPowerProfile_PowerProfileStateNotification;
  zclPower_Profile_GetOverallSchedulePrice_t                        pfnPowerProfile_GetOverallSchedulePrice;
  zclPower_Profile_EnergyPhasesScheduleReq_t                        pfnPowerProfile_EnergyPhasesScheduleReq;
  zclPower_Profile_EnergyPhasesScheduleStateRsp_t                   pfnPowerProfile_EnergyPhasesScheduleStateRsp;
  zclPower_Profile_EnergyPhasesScheduleStateNotification_t          pfnPowerProfile_EnergyPhasesScheduleStateNotification;
  zclPower_Profile_PowerProfileScheduleConstraintsNotification_t    pfnPowerProfile_PowerProfileScheduleConstraintsNotification;
  zclPower_Profile_PowerProfileScheduleConstraintsRsp_t             pfnPowerProfile_PowerProfileScheduleConstraintsRsp;
  zclPower_Profile_GetPowerProfilePriceExt_t                        pfnPowerProfile_GetPowerProfilePriceExt;
} zclPowerProfile_AppCallbacks_t;

/******************************************************************************
 * FUNCTION MACROS
 */

/*
 * Send Energy Phases Schedule Notification cmd - COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_NOTIFICATION
 *
 * Use like:
 *  ZStatus_t zclPowerProfile_Send_EnergyPhasesScheduleNotification( uint8_t srcEP, afAddrType_t *dstAddr,
 *                                                                   zclPowerProfileEnergyPhasesSchedule_t *pPayload,
 *                                                                   uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclPowerProfile_Send_EnergyPhasesScheduleNotification(a, b, c, d, e) \
        zclPowerProfile_Send_EnergyPhasesSchedule( (a), (b), \
                                                   COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_NOTIFICATION, \
                                                   (c), ZCL_FRAME_CLIENT_SERVER_DIR, (d), (e) )


/*
 * Send Energy Phases Schedule Response cmd - COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_RESPONSE
 *
 * Use like:
 *  ZStatus_t zclPowerProfile_Send_EnergyPhasesScheduleRsp( uint8_t srcEP, afAddrType_t *dstAddr,
 *                                                          zclPowerProfileEnergyPhasesSchedule_t *pPayload,
 *                                                          uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclPowerProfile_Send_EnergyPhasesScheduleRsp(a, b, c, d, e) \
        zclPowerProfile_Send_EnergyPhasesSchedule( (a), (b), \
                                                   COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_RESPONSE, \
                                                   (c), ZCL_FRAME_CLIENT_SERVER_DIR, (d), (e) )

/*
 * Send Energy Phases Schedule State Response cmd - COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_RESPONSE
 *
 * Use like:
 *  ZStatus_t zclPowerProfile_Send_EnergyPhasesScheduleStateRsp( uint8_t srcEP, afAddrType_t *dstAddr,
 *                                                               zclPowerProfileEnergyPhasesSchedule_t *pPayload,
 *                                                               uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclPowerProfile_Send_EnergyPhasesScheduleStateRsp(a, b, c, d, e) \
        zclPowerProfile_Send_EnergyPhasesSchedule( (a), (b), \
                                                   COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_RESPONSE, \
                                                   (c), ZCL_FRAME_SERVER_CLIENT_DIR, (d), (e) )

/*
 * Send Energy Phases Schedule State Notification cmd - COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_NOTIFICATION
 *
 * Use like:
 *  ZStatus_t zclPowerProfile_Send_EnergyPhasesScheduleStateNotification( uint8_t srcEP, afAddrType_t *dstAddr,
 *                                                                        zclPowerProfileEnergyPhasesSchedule_t *pPayload,
 *                                                                        uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclPowerProfile_Send_EnergyPhasesScheduleStateNotification(a, b, c, d, e) \
        zclPowerProfile_Send_EnergyPhasesSchedule( (a), (b), \
                                                   COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_NOTIFICATION, \
                                                   (c), ZCL_FRAME_SERVER_CLIENT_DIR, (d), (e) )

/******************************************************************************
 * VARIABLES
 */

/******************************************************************************
 * FUNCTIONS
 */


/*** Register for callbacks from this cluster library ***/
extern ZStatus_t zclPowerProfile_RegisterCmdCallbacks( uint8_t endpoint, zclPowerProfile_AppCallbacks_t *callbacks );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileReq
 *
 * @brief   Request sent to server for Power Profile info.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                                       uint8_t powerProfileID,
                                                       uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileStateReq
 *
 * @brief   Generated in order to retrieve the identifiers of current Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileStateReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                                            uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetPowerProfilePriceRsp
 *
 * @brief   Allows a client to communicate the cost associated with a defined
 *          Power Profile to a server requesting it.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   currency - identifies the local unit of currency
 * @param   price - the price of the energy of a specific Power Profile
 * @param   priceTrailingDigit - determines the decimal location for price
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetPowerProfilePriceRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                                               uint8_t powerProfileID, uint16_t currency,
                                                               uint32_t price, uint8_t priceTrailingDigit,
                                                               uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetOverallSchedulePriceRsp
 *
 * @brief   Allows a client to communicate the cost associated with all
 *          Power Profiles to a server requesting it.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   currency - identifies the local unit of currency
 * @param   price - the price of the energy of a specific Power Profile
 * @param   priceTrailingDigit - determines the decimal location for price
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetOverallSchedulePriceRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                  uint16_t currency, uint32_t price, uint8_t priceTrailingDigit,
                                                                  uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_EnergyPhasesSchedule
 *
 * @brief   Used for Power Profile Energy Phases Schedule commands.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   cmdID - COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_NOTIFICATION,
 *                  COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_RESPONSE,
 *                  COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_RESPONSE,
 *                  COMMAND_POWER_PROFILE_ENERGY_PHASES_SCHEDULE_STATE_NOTIFICATION
 * @param   pPayload:
 *          powerProfileID - specifies the Power Profile in question
 *          numOfScheduledPhases - total number of scheduled energy phases
 *          energyPhaseID - identifier of the specific energy phase
 *          scheduledTime - relative time scheduled in respect to previous energy phase (in minutes)
 * @param   direction - send command client-to-server or server-to-client
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_EnergyPhasesSchedule( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t cmdID,
                                                            zclPowerProfileEnergyPhasesSchedule_t *pCmd,
                                                            uint8_t direction, uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileScheduleConstraintsReq
 *
 * @brief   Request sent to server to request constraints of the Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileScheduleConstraintsReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                          uint8_t powerProfileID,
                                                                          uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_EnergyPhasesScheduleStateReq
 *
 * @brief   Request sent to server to check the states of the scheduling of a Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_EnergyPhasesScheduleStateReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                    uint8_t powerProfileID,
                                                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetPowerProfilePriceExtRsp
 *
 * @brief   Allows a client to communicate the cost associated with all
 *          Power Profiles scheduled to a server.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   currency - identifies the local unit of currency
 * @param   price - the price of the energy of a specific Power Profile
 * @param   priceTrailingDigit - determines the decimal location for price
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetPowerProfilePriceExtRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                  uint8_t powerProfileID, uint16_t currency,
                                                                  uint32_t price, uint8_t priceTrailingDigit,
                                                                  uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileNotification
 *
 * @brief   Server sends information of specific parameters belonging to each phase.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          totalProfileNum - total number of profiles supported by the device
 *          powerProfileID - identifier of the specific Power Profile
 *          numOfTransferredPhases - number of phases transferred
 *          energyPhaseID - identifier of the specific Power Profile energy phase
 *          macroPhaseID - identifier of the specific Power Profile phase
 *          expectedDuration - estimated duration of the specific phase
 *          peakPower - estimated power of the specific phase
 *          energy - estimated energy consumption for the accounted phase
 *          maxActivationDelay - maximum interruption time between end of previous phase and start of next phase
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileNotification( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                zclPowerProfile_t *pPayload,
                                                                uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileRsp
 *
 * @brief   A response from the server to the PowerProfileReq command.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          totalProfileNum - total number of profiles supported by the device
 *          powerProfileID - identifier of the specific Power Profile
 *          numOfTransferredPhases - number of phases transferred
 *          energyPhaseID - identifier of the specific Power Profile energy phase
 *          macroPhaseID - identifier of the specific Power Profile phase
 *          expectedDuration - estimated duration of the specific phase
 *          peakPower - estimated power of the specific phase
 *          energy - estimated energy consumption for the accounted phase
 *          maxActivationDelay - maximum interruption time between end of previous phase and start of next phase
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                                       zclPowerProfile_t *pPayload,
                                                       uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileStateRsp
 *
 * @brief   Server communicates its current Power Profile(s) to requesting client.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          powerProfileCount - number of Power Profile Records that follow in the message
 *          powerProfileRecord - includes: powerProfileID, energyPhaseID, powerProfileRemoteControl, powerProfileState
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileStateRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                                            zclPowerProfileState_t *pPayload,
                                                            uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetPowerProfilePrice
 *
 * @brief   Used by server to retrieve the cost associated to a specific Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetPowerProfilePrice( uint8_t srcEP, afAddrType_t *dstAddr,
                                                            uint8_t powerProfileID,
                                                            uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileStateNotification
 *
 * @brief   Generated by server to update the state of the power profile and
 *          current energy phase.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          powerProfileCount - number of Power Profile Records that follow in the message
 *          powerProfileRecord - includes: powerProfileID, energyPhaseID, powerProfileRemoteControl, powerProfileState
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileStateNotification( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                     zclPowerProfileState_t *pPayload,
                                                                     uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetOverallSchedulePrice
 *
 * @brief   Generated by server to retrieve the overall cost associated to
 *          all Power Profiles scheduled by the scheduler for the next 24 hrs.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetOverallSchedulePrice( uint8_t srcEP, afAddrType_t *dstAddr,
                                                               uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_EnergyPhasesScheduleReq
 *
 * @brief   Generated by server to retrieve from scheduler the schedule of
 *          specific Power Profile carried in the payload.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   powerProfileID - specifies the Power Profile in question
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_EnergyPhasesScheduleReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                                               uint8_t powerProfileID,
                                                               uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileScheduleConstraintsNotification
 *
 * @brief   Generated by server to notify client of imposed constraints and
 *          allow scheduler to set proper boundaries for scheduler.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - parameters for the Schedule Constraints Notification
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileScheduleConstraintsNotification( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                                   zclPowerProfileScheduleConstraintsNotification_t *pCmd,
                                                                                   uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_PowerProfileScheduleConstraintsRsp
 *
 * @brief   Generated by server in response to PowerProfileScheduleConstraintsReq.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - parameters for the Schedule Constraints Rsp cmd
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_PowerProfileScheduleConstraintsRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                                                          zclPowerProfileScheduleConstraintsRsp_t *pCmd,
                                                                          uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclPowerProfile_Send_GetPowerProfilePriceExt
 *
 * @brief   Generated by server to retrieve cost associated to a specific
 *          Power Profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - parameters for the Get Power Profile Price Extended cmd
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclPowerProfile_Send_GetPowerProfilePriceExt( uint8_t srcEP, afAddrType_t *dstAddr,
                                                               zclPowerProfileGetPowerProfilePriceExt_t *pCmd,
                                                               uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
*********************************************************************/
#endif // ZCL_POWER_PROFILE

#ifdef __cplusplus
}
#endif

#endif /* ZCL_POWER_PROFILE_H */
