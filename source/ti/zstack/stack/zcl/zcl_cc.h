/**************************************************************************************************
  Filename:       zcl_cc.h
  Revised:        $Date: 2011-04-13 10:12:34 -0700 (Wed, 13 Apr 2011) $
  Revision:       $Revision: 25678 $

  Description:    Zigbee Cluster Library - Commissioning Cluster


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

#ifndef ZCL_CC_H
#define ZCL_CC_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ti_zstack_config.h"

#ifdef ZCL_CC
/*********************************************************************
 * MACROS
 */
// Startup Mode
#define zcl_CCStartupMode( a )                   ( (a) & CC_STARTUP_MODE )
#define zcl_CCImmediate( a )                     ( (a) & CC_IMMEDIATE )

/*********************************************************************
 * CONSTANTS
 */

/**********************************************/
/*** Commissioning Clusters Attributes List ***/
/**********************************************/

// Commissioning Clusters Attribute Set
#define ATTRID_MASK_CC_STARTUP_PARAMS_STACK      0x0000
#define ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY   0x0010
#define ATTRID_MASK_CC_JOIN_PARAMS               0x0020
#define ATTRID_MASK_CC_END_DEVICE_PARAMS         0x0030
#define ATTRID_MASK_CC_CONCENTRATOR_PARAMS       0x0040

// Startup Parameters Attribute Set - Stack 0x000
#define ATTRID_CC_SHORT_ADDRESS                  ( 0x0000 | ATTRID_MASK_CC_STARTUP_PARAMS_STACK )
#define ATTRID_CC_EXTENDED_PANID                 ( 0x0001 | ATTRID_MASK_CC_STARTUP_PARAMS_STACK )
#define ATTRID_CC_PANID                          ( 0x0002 | ATTRID_MASK_CC_STARTUP_PARAMS_STACK )
#define ATTRID_CC_CHANNEL_MASK                   ( 0x0003 | ATTRID_MASK_CC_STARTUP_PARAMS_STACK )
#define ATTRID_CC_PROTOCOL_VERSION               ( 0x0004 | ATTRID_MASK_CC_STARTUP_PARAMS_STACK )
#define ATTRID_CC_STACK_PROFILE                  ( 0x0005 | ATTRID_MASK_CC_STARTUP_PARAMS_STACK )
#define ATTRID_CC_STARTUP_CONTROL                ( 0x0006 | ATTRID_MASK_CC_STARTUP_PARAMS_STACK )

// Startup Parameters Attribute Set - Security 0x001
#define ATTRID_CC_TRUST_CENTER_ADDRESS           ( 0x0000 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )
#define ATTRID_CC_TRUST_CENTER_MASTER_KEY        ( 0x0001 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )
#define ATTRID_CC_NETWORK_KEY                    ( 0x0002 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )
#define ATTRID_CC_USE_INSECURE_JOIN              ( 0x0003 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )
#define ATTRID_CC_PRECONFIGURED_LINK_KEY         ( 0x0004 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )
#define ATTRID_CC_NETWORK_KEY_SEQ_NUM            ( 0x0005 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )
#define ATTRID_CC_NETWORK_KEY_TYPE               ( 0x0006 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )
#define ATTRID_CC_NETWORK_MANAGER_ADDRESS        ( 0x0007 | ATTRID_MASK_CC_STARTUP_PARAMS_SECURITY )

// Join Parameters Attribute Set 0x002
#define ATTRID_CC_SCAN_ATTEMPTS                  ( 0x0000 | ATTRID_MASK_CC_JOIN_PARAMS )
#define ATTRID_CC_TIME_BETWEEN_SCANS             ( 0x0001 | ATTRID_MASK_CC_JOIN_PARAMS )
#define ATTRID_CC_REJOIN_INTERVAL                ( 0x0002 | ATTRID_MASK_CC_JOIN_PARAMS )
#define ATTRID_CC_MAX_REJOIN_INTERVAL            ( 0x0003 | ATTRID_MASK_CC_JOIN_PARAMS )

// End Device Parameters Attribute Set 0x003
#define ATTRID_CC_INDIRECT_POLL_RATE             ( 0x0000 | ATTRID_MASK_CC_END_DEVICE_PARAMS )
#define ATTRID_CC_PARENT_RETRY_THRESHOLD         ( 0x0001 | ATTRID_MASK_CC_END_DEVICE_PARAMS )

// Concentrator Parameters Attribute Set 0x004
#define ATTRID_CC_CONCENTRATOR_FLAG              ( 0x0000 | ATTRID_MASK_CC_CONCENTRATOR_PARAMS )
#define ATTRID_CC_CONCENTRATOR_RADIUS            ( 0x0001 | ATTRID_MASK_CC_CONCENTRATOR_PARAMS )
#define ATTRID_CC_CONCENTRATOR_DISCOVERY_TIME    ( 0x0002 | ATTRID_MASK_CC_CONCENTRATOR_PARAMS )

/************************************************************/
/***    Commissioning Cluster Command ID                  ***/
/************************************************************/

// Commands Received by Commissioning Cluster Server

#define COMMAND_CC_RESTART_DEVICE                0x0000
#define COMMAND_CC_SAVE_STARTUP_PARAMS           0x0001
#define COMMAND_CC_RESTORE_STARTUP_PARAMS        0x0002
#define COMMAND_CC_RESET_STARTUP_PARAMS          0x0003

// Commands generated by Commissioning Cluster Server

#define COMMAND_CC_RESTART_DEVICE_RSP            0x0000
#define COMMAND_CC_SAVE_STARTUP_PARAMS_RSP       0x0001
#define COMMAND_CC_RESTORE_STARTUP_PARAMS_RSP    0x0002
#define COMMAND_CC_RESET_STARTUP_PARAMS_RSP      0x0003

/******************************************************************/
/***        Enumerations                                        ***/
/******************************************************************/

// StartupControl attribute values
#define CC_STARTUP_CONTROL_OPTION_0              0x00 // Silent join
#define CC_STARTUP_CONTROL_OPTION_1              0x01 // Form network
#define CC_STARTUP_CONTROL_OPTION_2              0x02 // Rejoin network
#define CC_STARTUP_CONTROL_OPTION_3              0x03 // MAC Associate

/******************************************************************/
/***        BitMap                                              ***/
/******************************************************************/

// Restart Device command Options bit masks:
//  - Startup Mode (bits: 0..2)
//  - Immediate (bit: 3)
#define CC_STARTUP_MODE                          0x07
#define CC_IMMEDIATE                             0x08

// Startup Mode Sub-Field Values
#define CC_STARTUP_MODE_REPLACE_RESTART          0x00
#define CC_STARTUP_MODE_ONLY_RESTART             0x01

// Reset Startup Parameters command Options
#define CC_RESET_CURRENT                         0x01
#define CC_RESET_ALL                             0x02
#define CC_ERASE_INDEX                           0x04

/******************************************************************/
/***        Other Constants                                     ***/
/******************************************************************/

// Default Attribute Values
#define CC_DEFAULT_SHORT_ADDR                    0xFFFF
#define CC_DEFAULT_PANID                         0xFFFF
#define CC_DEFAULT_PROTOCOL_VERSION              0x02
#define CC_DEFAULT_NETWORK_KEY_SEQ_NUM           0x00
#define CC_DEFAULT_NETWORK_MANAGER_ADDR          0x00
#define CC_DEFAULT_SCAN_ATTEMPTS                 0x05
#define CC_DEFAULT_TIME_BETWEEN_SCANS            0x64
#define CC_DEFAULT_REJOIN_INTERVAL               0x3C
#define CC_DEFAULT_MAX_REJOIN_INTERVAL           0x0E10
#define CC_DEFAULT_CONCENTRATOR_RADIUS           0x0F
#define CC_DEFAULT_CONCENTRATOR_DISCOVERY_TIME   0x00

// Max Attribute Values
#define CC_MAX_INDIRECT_POLL_RATE                0xFFFF
#define CC_MAX_PARENT_RETRY_THRESHOLD            0xFF
#define CC_MAX_CONCENTRATOR_RADIUS               0xFF
#define CC_MAX_CONCENTRATOR_DISCOVERY_TIME       0xFF

// Command Packet Length
#define CC_PACKET_LEN_RESTART_DEVICE             0x03
#define CC_PACKET_LEN_STARTUP_PARAMS_CMD         0x02
#define CC_PACKET_LEN_SERVER_RSP                 0x01


/********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*** Structures used for callback functions ***/

// Restart Device command
typedef struct
{
  uint8_t options;
  uint8_t delay;
  uint8_t jitter;
} zclCCRestartDevice_t;

// Startup Parameters command - Save, Restore, Reset
typedef struct
{
  uint8_t options;
  uint8_t index;
} zclCCStartupParams_t;

// Server Parameters Response command
typedef struct
{
  uint8_t status;
} zclCCServerParamsRsp_t;

/*********************************************************************
 * CALLBACKS
 */

/* Commands */

// This callback is called to process an incoming Restart Device command
typedef void (*zclCC_Restart_Device_t)( zclCCRestartDevice_t *pCmd, afAddrType_t *srcAddr, uint8_t seqNum );

// This callback is called to process an incoming Save Startup Parameters command
typedef void (*zclCC_Save_StartupParams_t)( zclCCStartupParams_t *pCmd, afAddrType_t *srcAddr, uint8_t seqNum );

// This callback is called to process an incoming Restore Startup Parameters command
typedef void (*zclCC_Restore_StartupParams_t)( zclCCStartupParams_t *pCmd, afAddrType_t *srcAddr, uint8_t seqNum );

// This callback is called to process an incoming Reset Startup Parameters command
typedef void (*zclCC_Reset_StartupParams_t)( zclCCStartupParams_t *pCmd, afAddrType_t *srcAddr, uint8_t seqNum ) ;

/* Response */

// This callback is called to process an incoming Restart Device Response command
typedef void (*zclCC_Restart_DeviceRsp_t)( zclCCServerParamsRsp_t *pRsp, afAddrType_t *srcAddr, uint8_t seqNum );

// This callback is called to process an incoming Save Startup Parameters Response command
typedef void (*zclCC_Save_StartupParamsRsp_t)( zclCCServerParamsRsp_t *pRsp, afAddrType_t *srcAddr, uint8_t seqNum );

// This callback is called to process an incoming Restore Startup Parameters Response command
typedef void (*zclCC_Restore_StartupParamsRsp_t)( zclCCServerParamsRsp_t *pRsp, afAddrType_t *srcAddr, uint8_t seqNum );

// This callback is called to process an incoming Reset Startup Parameters Response command
typedef void (*zclCC_Reset_StartupParamsRsp_t)( zclCCServerParamsRsp_t *pRsp, afAddrType_t *srcAddr, uint8_t seqNum ) ;

// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive

typedef struct
{
  zclCC_Restart_Device_t            pfnRestart_Device;
  zclCC_Save_StartupParams_t        pfnSave_StartupParams;
  zclCC_Restore_StartupParams_t     pfnRestore_StartupParams;
  zclCC_Reset_StartupParams_t       pfnReset_StartupParams;
  zclCC_Restart_DeviceRsp_t         pfnRestart_DeviceRsp;
  zclCC_Save_StartupParamsRsp_t     pfnSave_StartupParamsRsp;
  zclCC_Restore_StartupParamsRsp_t  pfnRestore_StartupParamsRsp;
  zclCC_Reset_StartupParamsRsp_t    pfnReset_StartupParamsRsp;
} zclCC_AppCallbacks_t;

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * FUNCTION MACROS
 */

/*
 *  Send a Save Startup Parameters command
 *  Use like:
 *      ZStatus_t zclCC_Send_SaveStartupParams( uint8_t srcEP, afAddrType_t *dstAddr, zclCCStartupParams_t *pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclCC_Send_SaveStartupParams(a,b,c,d,e) zclCC_Send_StartupParamsCmd( (a), (b), (c), COMMAND_CC_SAVE_STARTUP_PARAMS, (d), (e) )

/*
 *  Send a Restore Startup Parameters command
 *  Use like:
 *      ZStatus_t zclCC_Send_RestoreStartupParams( uint8_t srcEP, afAddrType_t *dstAddr, zclCCStartupParams_t *pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclCC_Send_RestoreStartupParams(a,b,c,d,e) zclCC_Send_StartupParamsCmd( (a), (b), (c), COMMAND_CC_RESTORE_STARTUP_PARAMS, (d), (e) )

/*
 *  Send a Reset Startup Parameters command
 *  Use like:
 *      ZStatus_t zclCC_Send_ResetStartupParams( uint8_t srcEP, afAddrType_t *dstAddr, zclCCStartupParams_t *pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclCC_Send_ResetStartupParams(a,b,c,d,e) zclCC_Send_StartupParamsCmd( (a), (b), (c), COMMAND_CC_RESET_STARTUP_PARAMS, (d), (e) )

/*
 *  Send a Restart Device Response
 *  Use like:
 *      ZStatus_t zclCC_Send_RestartDeviceRsp( uint8_t srcEP, afAddrType_t *dstAddr, zclCCServerParamsRsp_t *pRsp, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclCC_Send_RestartDeviceRsp(a,b,c,d,e) zclCC_Send_ServerParamsRsp( (a), (b), (c), COMMAND_CC_RESTART_DEVICE_RSP, (d), (e) )

/*
 *  Send a Save Startup Parameters Response
 *  Use like:
 *      ZStatus_t zclCC_Send_SaveStartupParamsRsp( uint8_t srcEP, afAddrType_t *dstAddr, zclCCServerParamsRsp_t *pRsp, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclCC_Send_SaveStartupParamsRsp(a,b,c,d,e) zclCC_Send_ServerParamsRsp( (a), (b), (c), COMMAND_CC_SAVE_STARTUP_PARAMS_RSP, (d), (e) )

/*
 *  Send a Restore Startup Parameters Response
 *  Use like:
 *      ZStatus_t zclCC_Send_RestoreStartupParamsRsp( uint8_t srcEP, afAddrType_t *dstAddr, zclCCServerParamsRsp_t *pRsp, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclCC_Send_RestoreStartupParamsRsp(a,b,c,d,e) zclCC_Send_ServerParamsRsp( (a), (b), (c), COMMAND_CC_RESTORE_STARTUP_PARAMS_RSP, (d), (e) )

/*
 *  Send a Reset Startup Parameters Response
 *  Use like:
 *      ZStatus_t zclCC_Send_ResetStartupParamsRsp( uint8_t srcEP, afAddrType_t *dstAddr, zclCCServerParamsRsp_t *pRsp, uint8_t disableDefaultRsp, uint8_t seqNum );
 */
#define zclCC_Send_ResetStartupParamsRsp(a,b,c,d,e) zclCC_Send_ServerParamsRsp( (a), (b), (c), COMMAND_CC_RESET_STARTUP_PARAMS_RSP, (d), (e) )


/*********************************************************************
 * FUNCTIONS
 */

/*
 * Register for callbacks from this cluster library
 */
extern ZStatus_t zclCC_RegisterCmdCallbacks( uint8_t endpoint, zclCC_AppCallbacks_t *callbacks );

/*
 * Send Restart Device Command
 */
ZStatus_t zclCC_Send_RestartDevice( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zclCCRestartDevice_t *pCmd,
                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * Send Startup Parameters Command (Save, Restore or Reset)
 */
ZStatus_t zclCC_Send_StartupParamsCmd( uint8_t srcEP, afAddrType_t *dstAddr,
                                       zclCCStartupParams_t *pCmd, uint8_t cmdId,
                                       uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * Send Server Response (Restart Device, Save, Restore or Reset)
 */
ZStatus_t zclCC_Send_ServerParamsRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                      zclCCServerParamsRsp_t *pCmd, uint8_t cmdId,
                                      uint8_t disableDefaultRsp, uint8_t seqNum );

#endif // ZCL_CC

#ifdef __cplusplus
}
#endif


#endif /* ZCL_CC_H */
