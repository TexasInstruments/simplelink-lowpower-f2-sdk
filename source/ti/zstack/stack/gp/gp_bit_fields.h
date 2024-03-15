/**************************************************************************************************
  Filename:       gp_bitFields.h
  Revised:        $Date: 2018-09-13 11:51:49 -0700 (Thu, 13 Feb 2018) $
  Revision:       $Revision: - $

  Description:    This file contains the Green Power bit fields deffinitions.


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

#ifndef GP_BIT_FIELDS_H
#define GP_BIT_FIELDS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

 /*********************************************************************
 * MACROS
 */

// Bit shift
#define GP_BIT(n)                  ( 1<<(n) )

// Create a bitmask of specified len.
#define GP_BIT_MASK(len)           ( GP_BIT(len)-1 )

// Create a bitfield mask of length starting at start bit.
#define GP_BF_MASK(start, len)     ( GP_BIT_MASK(len)<<(start) )

// Prepare a bitmask for insertion or combining.
#define GP_BF_PREP(x, start, len)  ( (x) & ( GP_BIT_MASK(len) << (start) ) )

// Extract a bitfield of length starting at start bit from y.
#define GP_BF_GET(y, start, len)   ( ((y)>>(start)) & GP_BIT_MASK(len) )

// Extract a bitfield of length starting at start bit from y.
#define GP_BIT_GET(y, start)   ( GP_BF_GET(y, start, 1) )

// Set bit on bit value of x into y.
#define GP_BIT_SET(y, x, start)    \
    ( *y= ((*y) &~ GP_BF_MASK(start, 1)) | x << ( start ) )

// Insert a new bitfield value x into y.
#define GP_BIT_FIELD_SET(y, x, starty, leny, startx, lenx)    \
    ( *y= ((*y) &~ GP_BF_MASK(starty, leny)) | ( GP_BF_GET(x, startx, lenx) ) << ( starty ) )

/*******************************************************************************
*  Actions field of the GP Pairing Configuration command
*
* Bits: 0..2            3                4..7s
*   Action        Send GP Pairing      Reserved
*
********************************************************************************/

#define GP_PAIRING_CONFIG_ACTION_IS_NO_ACTION(x)            ( GP_BF_GET(x, GP_ACTION_BIT, \
                                                              GP_ACTION_LEN) == \
                                                              GP_ACTION_NO_ACTION )

#define GP_PAIRING_CONFIG_ACTION_IS_EXTEND(x)               ( GP_BF_GET(x, GP_ACTION_BIT, \
                                                              GP_ACTION_LEN) == \
                                                              GP_ACTION_EXTEND )

#define GP_PAIRING_CONFIG_ACTION_IS_REPLACE(x)              ( GP_BF_GET(x, GP_ACTION_BIT, \
                                                              GP_ACTION_LEN) == \
                                                              GP_ACTION_REPLACE )

#define GP_PAIRING_CONFIG_ACTION_IS_REMOVE_PAIRING(x)       ( GP_BF_GET(x, GP_ACTION_BIT, \
                                                              GP_ACTION_LEN) == \
                                                              GP_ACTION_NO_REMOVE_PAIRING )

#define GP_PAIRING_CONFIG_ACTION_IS_REMOVE_GPD(x)           ( GP_BF_GET(x, GP_ACTION_BIT, \
                                                              GP_ACTION_LEN) == \
                                                              GP_ACTION_NO_REMOVE_GPD )

#define GP_PAIRING_CONFIG_ACTION_SEND_PAIRING(x)            ( GP_BIT_GET(x, GP_ACTION_SEND_PAIRING_BIT) == TRUE )

#define GP_GET_GPD_SEQ_CAP_BIT(x)                           ( GP_BIT_GET(x, GP_OPT_GPD_SEC_CAP) )


/*******************************************************************************
*  Options field of the GP Pairing Configuration command
*
* Bits: 0..2               3..4                5               6                 7
* ApplicationID        Communication        Sequence     RxOnCapability    Fixed Location
*                          Mode              Number
*                                         Capabilities
*
*            8                    9               10             11
*      Assigned Alias       Security use     Application      Reserved
*                                            Information
*                                              present
*
********************************************************************************/

#define GP_PAIRING_CONFIG_OPT_IS_APPLICATION_ID_GPD(x)      ( GP_BF_GET(x, GP_OPT_APP_ID_BIT, \
                                                              GP_OPT_APP_ID_LEN) == \
                                                              GP_OPT_APP_ID_GPD )

#define GP_PAIRING_CONFIG_OPT_IS_APPLICATION_ID_IEEE(x)     ( GP_BF_GET(x, GP_OPT_APP_ID_BIT, \
                                                              GP_OPT_APP_ID_LEN) == \
                                                              GP_OPT_APP_ID_IEEE )

#define GP_PAIRING_CONFIG_IS_FULL_UNICAST(x)                ( GP_BF_GET(x, GP_OPT_PAIRING_CONFIG_COMMUNICATION_MODE_BIT, \
                                                              GP_OPT_COMMUNICATION_MODE_FIELD_LEN) == \
                                                              GP_OPT_COMMUNICATION_MODE_FULL_UNICAST )

#define GP_PAIRING_CONFIG_IS_GRPCAST_DGROUP(x)              ( GP_BF_GET(x, GP_OPT_PAIRING_CONFIG_COMMUNICATION_MODE_BIT, \
                                                              GP_OPT_COMMUNICATION_MODE_FIELD_LEN) == \
                                                              GP_OPT_COMMUNICATION_MODE_GRPCAST_DGROUP_ID )

#define GP_PAIRING_CONFIG_IS_GRPCAST_GROUP(x)               ( GP_BF_GET(x, GP_OPT_PAIRING_CONFIG_COMMUNICATION_MODE_BIT, \
                                                              GP_OPT_COMMUNICATION_MODE_FIELD_LEN) == \
                                                              GP_OPT_COMMUNICATION_MODE_GRPCAST_GROUP_ID )

#define GP_PAIRING_CONFIG_IS_LIGHT_UNICAST(x)               ( GP_BF_GET(x, GP_OPT_PAIRING_CONFIG_COMMUNICATION_MODE_BIT, \
                                                              GP_OPT_COMMUNICATION_MODE_FIELD_LEN) == \
                                                              GP_OPT_COMMUNICATION_MODE_LIGHT_UNICAST )

#define GP_PAIRING_CONFIG_OPT_EXTEND_ENTRY(x)               ( GP_BIT_GET(x, GP_OPT_PAIRING_CONFIG_EXTEND_ENTRY_BIT) )

#define GP_PAIRING_CONFIG_OPT_REPLACE_ENTRY(x)              ( GP_BIT_GET(x, GP_OPT_PAIRING_CONFIG_REPLACE_ENTRY_BIT) )

#define GP_PAIRING_CONFIG_OPT_REMOVE_GPD(x)                 ( GP_BIT_GET(x, GP_OPT_PAIRING_CONFIG_REMOVE_GPD_BIT) )

#define GP_GET_GPD_SEQ_CAP_BIT(x)                           ( GP_BIT_GET(x, GP_OPT_GPD_SEC_CAP) )

#define GP_GET_RX_ON_CAP_BIT(x)                             ( GP_BIT_GET(x, GP_OPT_GPD_SEC_CAP) )

#define GP_PAIRING_CONFIG_ALIAS(x)                          ( GP_BIT_GET(x, GP_OPT_PAIRING_CONFIG_ALIAS) )

#define GP_PAIRING_CONFIG_SECURITY_USE(x)                   ( GP_BIT_GET(x, GP_OPT_SECURITY_USE) )

#define GP_PAIRING_CONFIG_APP_INFO(x)                       ( GP_BIT_GET(x, GP_OPT_APP_INFO) )

/*******************************************************************************
*  Options field of the GP Pairing command
*
* Bits: 0..2              3               4             5..6             7
* ApplicationID        AddSink        RemoveGPD     Communication    GPD Fixed
*                                                       mode
*
*
*         8                 9..10             11..13            14
*      GPD MAC          Security level     Security key    GPD security
*   sequence number                            type        frame counter
*    capabilities                                             present
*
*                        15                   16                  17
*                 GPD security key       AssignedAlias      Forwarding radius
*                      present
*
********************************************************************************/

#define GP_PAIRING_OPT_IS_APPLICATION_ID_GPD(x)             ( GP_BF_GET(x, GP_OPT_APP_ID_BIT, \
                                                              GP_OPT_APP_ID_LEN) == \
                                                              GP_OPT_APP_ID_GPD )

#define GP_PAIRING_OPT_IS_APPLICATION_ID_IEEE(x)            ( GP_BF_GET(x, GP_OPT_APP_ID_BIT, \
                                                              GP_OPT_APP_ID_LEN) == \
                                                              GP_OPT_APP_ID_IEEE )

#define GP_PAIRING_OPT_IS_REQ_TYPE_GPD(x)                   ( GP_BF_GET(x, GP_OPT_REQ_TYP_BIT, \
                                                              GP_OPT_REQ_TYP_LEN) == \
                                                              GP_OPT_REQ_TYP_GPD_ID )

#define GP_PAIRING_OPT_IS_REQ_TYPE_INDEX(x)                 ( GP_BF_GET(x, GP_OPT_REQ_TYP_BIT, \
                                                              GP_OPT_REQ_TYP_LEN) == \
                                                              GP_OPT_REQ_TYP_INDEX )

#define GP_PAIRING_OPT_ADD_SINK(x)                          ( GP_BIT_GET(x, GP_OPT_ADD_SINK_BIT) )

#define GP_PAIRING_OPT_REMOVE_GPD(x)                        ( GP_BIT_GET(x, GP_OPT_PAIRING_REMOVE_GPD_BIT) )

#define GP_PAIRING_IS_FULL_UNICAST(x)                       ( GP_BF_GET(x, GP_OPT_PAIRING_COMMUNICATION_MODE_BIT, \
                                                              GP_OPT_COMMUNICATION_MODE_FIELD_LEN) == \
                                                              GP_OPT_COMMUNICATION_MODE_FULL_UNICAST )

#define GP_PAIRING_IS_GRPCAST_DGROUP(x)                     ( GP_BF_GET(x, GP_OPT_PAIRING_COMMUNICATION_MODE_BIT, \
                                                              GP_OPT_COMMUNICATION_MODE_FIELD_LEN) == \
                                                              GP_OPT_COMMUNICATION_MODE_GRPCAST_DGROUP_ID )

#define GP_PAIRING_IS_GRPCAST_CGROUP(x)                      ( GP_BF_GET(x, GP_OPT_PAIRING_COMMUNICATION_MODE_BIT, \
                                                              GP_OPT_COMMUNICATION_MODE_FIELD_LEN) == \
                                                              GP_OPT_COMMUNICATION_MODE_GRPCAST_GROUP_ID )

#define GP_PAIRING_IS_LIGHT_UNICAST(x)                      ( GP_BF_GET(x, GP_OPT_PAIRING_COMMUNICATION_MODE_BIT, \
                                                              GP_OPT_COMMUNICATION_MODE_FIELD_LEN) == \
                                                              GP_OPT_COMMUNICATION_MODE_LIGHT_UNICAST )

#define GP_GET_GPD_FIXED_BIT(x)                             ( GP_BIT_GET(x, GP_OPT_GPD_FIXED) )

#define GP_GET_GPD_MAC_SEQ_CAP_BIT(x)                       ( GP_BIT_GET(x, GP_OPT_GPD_MAC_SEC_CAP) )

#define GP_GET_SEC_LEVEL(x)                                 ( GP_BF_GET(x, GP_OPT_SEC_LEVEL, \
                                                              GP_OPT_SEC_LEVEL_LEN) )

#define GP_GET_SEC_KEY_TYPE(x)                              ( GP_BF_GET(x, GP_OPT_SEC_KEY_TYPE, \
                                                              GP_OPT_SEC_KEY_TYPE_LEN) )

#define GP_SEC_COUNTER(x)                                   ( GP_BIT_GET(x, GP_OPT_GPD_SEC_COUNTER) )

#define GP_SEC_KEY(x)                                       ( GP_BIT_GET(x, GP_OPT_GPD_SEC_KEY ) )

#define GP_PAIRING_ALIAS(x)                                 ( GP_BIT_GET(x, GP_OPT_PAIRING_ALIAS) )

#define GP_FORWARDING_RADIUS(x)                             ( GP_BIT_GET(x, GP_OPT_FORWARDING_RADIUS) )

/*******************************************************************************
* Common Gp tables field management
*
* Bits: 0..2
* ApplicationID
*
********************************************************************************/
#define GP_OPT_IS_APPLICATION_ID_GPD(x)                 ( GP_BF_GET(x, GP_OPT_APP_ID_BIT, \
                                                              GP_OPT_APP_ID_LEN) == \
                                                              GP_OPT_APP_ID_GPD )

#define GP_OPT_IS_APPLICATION_ID_IEEE(x)                ( GP_BF_GET(x, GP_OPT_APP_ID_BIT, \
                                                              GP_OPT_APP_ID_LEN) == \
                                                              GP_OPT_APP_ID_IEEE )

#define GP_TBL_OPT_IS_REQ_TYPE_GPD(x)                       ( GP_BF_GET(x, GP_OPT_REQ_TYP_BIT, \
                                                              GP_OPT_REQ_TYP_LEN) == \
                                                              GP_OPT_REQ_TYP_GPD_ID )

#define GP_GET_APPLICATION_ID(x)                            ( GP_BF_GET(x, GP_OPT_APP_ID_BIT, GP_OPT_APP_ID_LEN) )
#define GP_TBL_COMP_APPLICATION_ID(y,x)                     ( GP_BF_GET(y, GP_OPT_APP_ID_BIT, GP_OPT_APP_ID_LEN) \
                                                              == GP_BF_GET(x, GP_OPT_APP_ID_BIT, GP_OPT_APP_ID_LEN) )

/*******************************************************************************
* Sink Table Entry Options Bitfield managment
*
* Bits: 0..2            3..4              5                  6
* ApplicationID     Communication   Sequence number    RxOnCapability
*                       mode          capability
*
*         7                8                 9
*   FixedLocation    AssignedAlias      SecurityUse
*
********************************************************************************/

#define PAIRING_SINK_TBL_SET_APPLICATION_ID(y, x)       ( GP_BIT_FIELD_SET(y, x, SINK_TBL_OPT_APP_ID_BIT, SINK_TBL_OPT_APP_ID_LEN, \
                                                                                GP_OPT_APP_ID_BIT, GP_OPT_APP_ID_LEN) )

#define SINK_TBL_GET_COMMUNICATION_MODE(x)              ( GP_BF_GET(x, SINK_TBL_OPT_COMMUNICATION_MODE_BIT, SINK_TBL_OPT_COMMUNICATION_MODE_LEN) )
#define SINK_TBL_COMP_COMMUNICATION_MODE(y,x)           ( GP_BF_GET(y, SINK_TBL_OPT_COMMUNICATION_MODE_BIT, SINK_TBL_OPT_COMMUNICATION_MODE_LEN) \
                                                          == GP_BF_GET(x, SINK_TBL_OPT_COMMUNICATION_MODE_BIT, SINK_TBL_OPT_COMMUNICATION_MODE_LEN) )

#define SINK_TBL_SET_COMMUNICATION_MODE(y, x)           ( GP_BIT_FIELD_SET(y, x, SINK_TBL_OPT_COMMUNICATION_MODE_BIT, SINK_TBL_OPT_COMMUNICATION_MODE_LEN, \
                                                                                GP_OPT_APP_ID_BIT, GP_OPT_APP_ID_LEN) )

#define SINK_TBL_GET_MAC_SEQ_CAP(x)                     ( GP_BIT_GET(x, SINK_TBL_OPT_MAC_SEQ_CAP_BIT) )
#define SINK_TBL_SET_MAC_SEQ_CAP(y)                     ( SET_BIT(y, SINK_TBL_OPT_MAC_SEQ_CAP_BIT) )
#define SINK_TBL_CLR_MAC_SEQ_CAP(y)                     ( CLR_BIT(y, SINK_TBL_OPT_MAC_SEQ_CAP_BIT) )

#define SINK_TBL_GET_RX_ON_CAPABILITIES(x)              ( GP_BIT_GET(x, SINK_TBL_OPT_RX_ON_CAPABILITIES_BIT) )
#define SINK_TBL_SET_RX_ON_CAPABILITIES(y)              ( SET_BIT(y, SINK_TBL_OPT_RX_ON_CAPABILITIES_BIT) )
#define SINK_TBL_CLR_RX_ON_CAPABILITIES(y)              ( CLR_BIT(y, SINK_TBL_OPT_RX_ON_CAPABILITIES_BIT) )

#define SINK_TBL_GET_FIXED_LOCATION(x)                  ( GP_BIT_GET(x, SINK_TBL_OPT_FIXED_LOCATION_BIT) )
#define SINK_TBL_SET_FIXED_LOCATION(y)                  ( SET_BIT(y, SINK_TBL_OPT_FIXED_LOCATION_BIT) )
#define SINK_TBL_CLR_FIXED_LOCATION(y)                  ( CLR_BIT(y, SINK_TBL_OPT_FIXED_LOCATION_BIT) )

#define SINK_TBL_GET_ASSIGNED_ALIAS(x)                  ( GP_BIT_GET(x, SINK_TBL_OPT_ASSIGNED_ALIAS_BIT) )
#define SINK_TBL_SET_ASSIGNED_ALIAS(y)                  ( SET_BIT(y, SINK_TBL_OPT_ASSIGNED_ALIAS_BIT) )
#define SINK_TBL_CLR_ASSIGNED_ALIAS(y)                  ( CLR_BIT(y, SINK_TBL_OPT_ASSIGNED_ALIAS_BIT) )

#define SINK_TBL_GET_SEC_USE(x)                         ( GP_BIT_GET(x, SINK_TBL_OPT_SEC_USE_BIT) )
#define SINK_TBL_SET_SEC_USE(y)                         ( SET_BIT(y, SINK_TBL_OPT_SEC_USE_BIT) )
#define SINK_TBL_CLR_SEC_USE(y)                         ( CLR_BIT(y, SINK_TBL_OPT_SEC_USE_BIT) )

#define SINK_TBL_GET_APP_INFO_PRESENT(x)                ( GP_BIT_GET(x, SINK_TBL_OPT_APP_INFO_PRESENT_BIT) )
#define SINK_TBL_SET_APP_INFO_PRESENT(y)                ( SET_BIT(y, SINK_TBL_OPT_APP_INFO_PRESENT_BIT) )
#define SINK_TBL_CLR_APP_INFO_PRESENT(y)                ( CLR_BIT(y, SINK_TBL_OPT_APP_INFO_PRESENT_BIT) )

/*******************************************************************************
* Proxy Table Entry Options Bitfield managment
*
* Bits: 0..2              3               4               5              6
* ApplicationID      EntryActive      EntryValid      Sequence      Lightweight
*                                                      number       Unicast GPS
*                                                   capabilities
*
*             7               8                  9                10             11
*          Derived      Commissioned       FirstToForward      InRange       GPD Fixed
*         Group GPS       Group GPS
*
*                           12                     13                14                  15
*                   HasAllUnicastRoutes      AssignedAlias      SecurityUse      Options Extension
*
********************************************************************************/

#define PAIRING_PROXY_TBL_SET_APPLICATION_ID(y, x)     ( GP_BIT_FIELD_SET(y, x, PROXY_TBL_OPT_APP_ID_BIT, PROXY_TBL_OPT_APP_ID_LEN, \
                                                                                GP_OPT_APP_ID_BIT, GP_OPT_APP_ID_LEN) )

#define PROXY_TBL_GET_ENTRY_ACTIVE(x)                  ( GP_BIT_GET(x, PROXY_TBL_OPT_ENTRY_ACTIVE_BIT) )
#define PROXY_TBL_SET_ENTRY_ACTIVE(y)                  ( SET_BIT(y, PROXY_TBL_OPT_ENTRY_ACTIVE_BIT) )
#define PROXY_TBL_CLR_ENTRY_ACTIVE(y)                  ( CLR_BIT(y, PROXY_TBL_OPT_ENTRY_ACTIVE_BIT) )

#define PROXY_TBL_GET_ENTRY_VALID(x)                   ( GP_BIT_GET(x, PROXY_TBL_OPT_ENTRY_VALID_BIT) )
#define PROXY_TBL_SET_ENTRY_VALID(y)                   ( SET_BIT(y, PROXY_TBL_OPT_ENTRY_VALID_BIT) )
#define PROXY_TBL_CLR_ENTRY_VALID(y)                   ( CLR_BIT(y, PROXY_TBL_OPT_ENTRY_VALID_BIT) )

#define PAIRING_PROXY_TBL_SET_MAC_SEQ_CAP(y, x)        ( GP_BIT_FIELD_SET(y, x, PROXY_TBL_OPT_GPD_MAC_SEQ_CAP_BIT, PROXY_TBL_OPT_GPD_MAC_SEQ_CAP_LEN, \
                                                                                GP_OPT_GPD_MAC_SEC_CAP, GP_OPT_GPD_MAC_SEC_CAP_LEN) )

#define PROXY_TBL_GET_LIGHTWIGHT_UNICAST(x)            ( GP_BIT_GET(x, PROXY_TBL_OPT_LIGHTWIGHT_UNICAST_BIT) )
#define PROXY_TBL_SET_LIGHTWIGHT_UNICAST(y, x)         ( GP_BIT_SET( y, GP_PAIRING_IS_LIGHT_UNICAST(x), PROXY_TBL_OPT_LIGHTWIGHT_UNICAST_BIT ) )

#define PROXY_TBL_GET_DGROUP(x)                        ( GP_BIT_GET(x, PROXY_TBL_OPT_DGROUP_BIT) )
#define PROXY_TBL_SET_DGROUP(y, x)                     ( GP_BIT_SET( y, GP_PAIRING_IS_GRPCAST_DGROUP(x), PROXY_TBL_OPT_DGROUP_BIT ) )

#define PROXY_TBL_GET_CGROUP(x)                        ( GP_BIT_GET(x, PROXY_TBL_OPT_CGROUP_BIT) )
#define PROXY_TBL_SET_CGROUP(y, x)                     ( GP_BIT_SET( y, GP_PAIRING_IS_GRPCAST_CGROUP(x), PROXY_TBL_OPT_CGROUP_BIT ) )

#define PROXY_TBL_GET_FIRST_TO_FORWARD(x)              ( GP_BIT_GET(x, PROXY_TBL_OPT_FIRST_TO_FORWARD_BIT) )
#define PROXY_TBL_SET_FIRST_TO_FORWARD(y, x)           ( GP_BIT_SET( y, x, PROXY_TBL_OPT_FIRST_TO_FORWARD_BIT ) )

#define PROXY_TBL_SET_IN_RANGE(y, x)                   ( GP_BIT_SET( y, x, PROXY_TBL_OPT_IN_RANGE_BIT ) )

#define PROXY_TBL_GET_GPD_FIXED(x)                     ( GP_BIT_GET(x, PROXY_TBL_OPT_GPD_FIXED_BIT) )
#define PROXY_TBL_SET_GPD_FIXED(y, x)                  ( GP_BIT_SET( y, GP_GET_GPD_FIXED_BIT(x), PROXY_TBL_OPT_GPD_FIXED_BIT ) )

#define PROXY_TBL_SET_HAS_ALL_ROUTES(y, x)             ( GP_BIT_SET( y, GP_PAIRING_IS_FULL_UNICAST(x), PROXY_TBL_OPT_HAS_ALL_ROUTES_BIT ) )

#define PROXY_TBL_GET_ASSIGNED_ALIAS(x)                ( GP_BIT_GET(x, PROXY_TBL_OPT_ASSIGNED_ALIAS_BIT) )
#define PROXY_TBL_SET_ASSIGNED_ALIAS(y, x)             ( GP_BIT_SET( y, x, PROXY_TBL_OPT_ASSIGNED_ALIAS_BIT ) )

#define PROXY_TBL_GET_SEC_USE(x)                       ( GP_BIT_GET(x, PROXY_TBL_OPT_SEC_USE_BIT) )
#define PROXY_TBL_SET_SEC_USE(y, x)                    ( GP_BIT_SET( y, x, PROXY_TBL_OPT_SEC_USE_BIT ) )

#define PROXY_TBL_GET_SEC_CAP(x)                       ( GP_BIT_GET(x, PROXY_TBL_OPT_GPD_MAC_SEQ_CAP_BIT) )
#define PROXY_TBL_SET_SEC_CAP(y, x)                    ( GP_BIT_SET( y, x, PROXY_TBL_OPT_GPD_MAC_SEQ_CAP_BIT ) )

#define PROXY_TBL_SET_OPT_EXT(y, x)                    ( GP_BIT_SET( y, x, PROXY_TBL_OPT_EXT_BIT ) )


/*******************************************************************************
* Security-related parameters Options Bitfield managment
*
* Section A.3.3.2.2.2.6
*
* Bits: 0..1             3..4                5..7
* SecurityLevel      SecurityKeyType       Reserved
*
********************************************************************************/
#define SEC_PARAMETERS_GET_SEC_OPT_SECURITY_LVL(x)          ( GP_BF_GET(x, SEC_PARAMETERS_OPT_SECURITY_LVL_BIT, SEC_PARAMETERS_OPT_SECURITY_LVL_LEN) )

#define SEC_PARAMETERS_GET_SEC_OPT_SECURITY_KEY_TYP(x)      ( GP_BF_GET(x, SEC_PARAMETERS_OPT_SECURITY_KEY_TYP_BIT, SEC_PARAMETERS_OPT_SECURITY_KEY_TYP_LEN) )

/*******************************************************************************
* Pairing Configuration app info Bitfield managment
*
* Section A.3.3.2.2.2.6
*
* Bits: 0             1           2              3           4..7
* ManufacturerID   ModelID   GPD commands   Cluster List   Reserved
*    present       present     present        present
*
********************************************************************************/
#define APP_INFO_GET_MANUFACTURER_ID_PRESENT(x)          ( GP_BF_GET(x, APP_INFO_MANUFACTURER_ID, APP_INFO_LEN) )

#define APP_INFO_GET_MODE_ID_PRESENT(x)                  ( GP_BF_GET(x, APP_INFO_MODE_ID, APP_INFO_LEN) )

#define APP_INFO_GET_GPD_COMMANDS_PRESENT(x)             ( GP_BF_GET(x, APP_INFO_GPD_COMMANDS, APP_INFO_LEN) )

#define APP_INFO_GET_CLUSTER_LIST_PRESENT(x)             ( GP_BF_GET(x, APP_INFO_CLUSTER_LIST, APP_INFO_LEN) )

#define APP_INFO_NUMBER_OF_SERVER_CLUSTERS(x)            ( GP_BF_GET(x, APP_INFO_SERVER_IDS, APP_CLUSTERS_FIELD_LEN) )

#define APP_INFO_NUMBER_OF_CLIENT_CLUSTERS(x)            ( GP_BF_GET(x, APP_INFO_CLIENT_IDS, APP_CLUSTERS_FIELD_LEN) )

// GP notification options
#define GP_NTF_SET_ALSO_UNICAST(y, x)                  ( GP_BIT_FIELD_SET(y, x, GP_NTF_OPT_ALSO_UNICAST, GP_NTF_OPT_ALSO_UNICAST_LEN, \
                                                             PROXY_TBL_OPT_LIGHTWIGHT_UNICAST_BIT, PROXY_TBL_OPT_LIGHTWIGHT_UNICAST_LEN) )

#define GP_NTF_SET_ALSO_DGROUP(y, x)                   ( GP_BIT_FIELD_SET(y, x, GP_NTF_OPT_ALSO_DGROUP, GP_NTF_OPT_ALSO_DGROUP_LEN, \
                                                             PROXY_TBL_OPT_DGROUP_BIT, PROXY_TBL_OPT_DGROUP_LEN) )

#define GP_NTF_GET_ALSO_CGROUP(x)                      ( GP_BF_GET(x, GP_NTF_OPT_ALSO_CGROUP, GP_NTF_OPT_ALSO_CGROUP_LEN) )
#define GP_NTF_SET_ALSO_CGROUP(y, x)                   ( GP_BIT_FIELD_SET(y, x, GP_NTF_OPT_ALSO_CGROUP, GP_NTF_OPT_ALSO_CGROUP_LEN, \
                                                             PROXY_TBL_OPT_CGROUP_BIT, PROXY_TBL_OPT_CGROUP_LEN) )

#define GP_NTF_SET_SEC_LEVEL(y, x)                     ( GP_BIT_FIELD_SET(y, x, GP_NTF_OPT_SEC_LEVEL, GP_NTF_OPT_SEC_LEVEL_LEN, \
                                                             SEC_OPT_SEC_LEVEL, SEC_OPT_SEC_LEVEL_LEN) )

#define GP_NTF_SET_SEC_KEY_TYPE(y, x)                  ( GP_BIT_FIELD_SET(y, x, GP_NTF_OPT_SEC_KEY_TYPE, GP_NTF_OPT_SEC_KEY_TYPE_LEN, \
                                                             SEC_OPT_SEC_KEY_TYPE, SEC_OPT_SEC_KEY_TYPE_LEN) )

// GP commissioning notification options
#define GP_CNTF_GET_SEC_LEVEL(y)                       ( GP_BF_GET(y, GP_CNTF_OPT_SEC_LEVEL, GP_CNTF_OPT_SEC_LEVEL_LEN) )

#define GP_CNTF_SET_SEC_LEVEL(y, x)                    ( GP_BIT_FIELD_SET(y, x, GP_CNTF_OPT_SEC_LEVEL, GP_CNTF_OPT_SEC_LEVEL_LEN, \
                                                             SEC_OPT_SEC_LEVEL, SEC_OPT_SEC_LEVEL_LEN) )

#define GP_CNTF_GET_SEC_KEY_TYPE(x)                    ( GP_BF_GET(x, GP_CNTF_OPT_SEC_KEY_TYPE, GP_CNTF_OPT_SEC_KEY_TYPE_LEN) )

#define GP_CNTF_SET_SEC_KEY_TYPE(y, x)                 ( GP_BIT_FIELD_SET(y, x, GP_CNTF_OPT_SEC_KEY_TYPE, GP_CNTF_OPT_SEC_KEY_TYPE_LEN, \
                                                             SEC_OPT_SEC_KEY_TYPE, SEC_OPT_SEC_KEY_TYPE_LEN) )

#define GP_CNTF_GET_SEC_FAIL(y)                        ( GP_BIT_GET(y, GP_CNTF_OPT_SEC_FAIL) )
#define GP_CNTF_SET_SEC_FAIL(y)                        ( SET_BIT(y, GP_CNTF_OPT_SEC_FAIL) )
#define GP_CNTF_CLR_SEC_FAIL(y)                        ( CLR_BIT(y, GP_CNTF_OPT_SEC_FAIL) )

#define GP_CNTF_GET_BIDIRECTIONAL_CAP(y)               ( GP_BIT_GET(y, GP_CNTF_OPT_BIDIRECTIONAL_CAP) )
#define GP_CNTF_SET_BIDIRECTIONAL_CAP(y)               ( SET_BIT(y, GP_CNTF_OPT_BIDIRECTIONAL_CAP) )

#define GP_CNTF_GET_PROXY_INFO(y)                      ( GP_BIT_GET(y, GP_CNTF_OPT_PROXY_INFO) )
#define GP_CNTF_SET_PROXY_INFO(y)                      ( SET_BIT(y, GP_CNTF_OPT_PROXY_INFO) )

#define GP_GET_RX_AFTER_TX(y)                          ( GP_BIT_GET(y, GP_NTF_OPT_RX_AFTER_TX) )
#define GP_SET_RX_AFTER_TX(y)                          ( SET_BIT(y, GP_NTF_OPT_RX_AFTER_TX) )
#define GP_CLR_RX_AFTER_TX(y)                          ( CLR_BIT(y, GP_NTF_OPT_RX_AFTER_TX) )

#define PROXY_TBL_SET_TX_QUEUE_FULL(y)                 ( SET_BIT(y, GP_NTF_OPT_TX_QUEUE_FULL) )
#define PROXY_TBL_CLR_TX_QUEUE_FULL(y)                 ( CLR_BIT(y, GP_NTF_OPT_TX_QUEUE_FULL) )

#define PROXY_TBL_SET_BIDIRECTIONAL_CAP(y)             ( SET_BIT(y, GP_NTF_OPT_BIDIRECTIONAL_CAP) )
#define PROXY_TBL_CLR_BIDIRECTIONAL_CAP(y)             ( CLR_BIT(y, GP_NTF_OPT_BIDIRECTIONAL_CAP) )
#define PROXY_TBL_SET_PROXY_INFO(y)                    ( SET_BIT(y, GP_NTF_OPT_PROXY_INFO) )

/*********************************************************************
 * CONSTANTS
 */
#define GP_TBL_OPT                   0
#define GP_TBL_GPD_ID                2
#define GP_TBL_SRC_ID                6
#define GP_TBL_OPT_AND_GPD_ID_LEN    10

// Sink Table
#define SINK_TBL_GRP_ADDR_LEN        4
#define SINK_TBL_ENTRY_LEN          45   // in bytes

#define SINK_TBL_OPT                GP_TBL_OPT
#define SINK_TBL_GPD_ID             GP_TBL_GPD_ID
#define SINK_TBL_SRC_ID             GP_TBL_SRC_ID
#define SINK_TBL_GPD_EP             10
#define SINK_TBL_DEVICE_ID          11
#define SINK_GROUP_COUNT            12
#define SINK_TBL_1ST_GRP_ADDR       13
#define SINK_TBL_2ND_GRP_ADDR       17
#define SINK_TBL_ALIAS              21
#define SINK_TBL_RADIUS             23
#define SINK_TBL_SEC_OPT            24
#define SINK_TBL_SEC_FRAME          25
#define SINK_TBL_GPD_KEY            29

// Proxy Table
#define LSINK_ADDR_LEN              10   // in bytes
#define PROXY_TBL_LEN               65   // in bytes

#define PROXY_TBL_OPT               GP_TBL_OPT
#define PROXY_TBL_GPD_ID            GP_TBL_GPD_ID
#define PROXY_TBL_GPD_EP            10
#define PROXY_TBL_ALIAS             11
#define PROXY_TBL_SEC_OPT           13
#define PROXY_TBL_SEC_FRAME         14
#define PROXY_TBL_GPD_KEY           18
#define PROXY_TBL_1ST_LSINK_ADDR    34
#define PROXY_TBL_2ND_LSINK_ADDR    44
#define PROXY_TBL_GRP_TBL_ENTRIES   54
#define PROXY_TBL_1ST_GRP_ADDR      55
#define PROXY_TBL_2ND_GRP_ADDR      59
#define PROXY_TBL_RADIUS            63
#define PROXY_TBL_SEARCH_COUNTER    64

#define PROXY_TBL_1ST_GRP_BIT        0
#define PROXY_TBL_2ND_GRP_BIT        1

/*************************************************************
* Sink Table Entry Options Bitfield
**************************************************************/
// Application ID bitfied
#define SINK_TBL_OPT_APP_ID_LEN                          0x03 // length of bitfield
#define SINK_TBL_OPT_APP_ID_BIT                          0x00

// Entry Active Bit
#define SINK_TBL_OPT_COMMUNICATION_MODE_LEN              0x02
#define SINK_TBL_OPT_COMMUNICATION_MODE_BIT              0x03

// GPD MAC sequence capablities
#define SINK_TBL_OPT_MAC_SEQ_CAP_LEN                     0x01
#define SINK_TBL_OPT_MAC_SEQ_CAP_BIT                     0x05

// Lightweight Unicast GPS Bit
#define SINK_TBL_OPT_RX_ON_CAPABILITIES_LEN              0x01
#define SINK_TBL_OPT_RX_ON_CAPABILITIES_BIT              0x06

// Fixed location Bit
#define SINK_TBL_OPT_FIXED_LOCATION_LEN                  0x01
#define SINK_TBL_OPT_FIXED_LOCATION_BIT                  0x07

// Assigned Alias
#define SINK_TBL_OPT_ASSIGNED_ALIAS_LEN                  0x01
#define SINK_TBL_OPT_ASSIGNED_ALIAS_BIT                  0x00

// SecurityUse Bit
#define SINK_TBL_OPT_SEC_USE_LEN                         0x01
#define SINK_TBL_OPT_SEC_USE_BIT                         0x01

// SecurityUse Bit
#define SINK_TBL_OPT_APP_INFO_PRESENT_LEN                0x01
#define SINK_TBL_OPT_APP_INFO_PRESENT_BIT                0x02

/*************************************************************
* Proxy Table Entry Options Bitfield
**************************************************************/
// Application ID bitfied
#define PROXY_TBL_OPT_APP_ID_LEN                          0x03 // length of bitfield
#define PROXY_TBL_OPT_APP_ID_BIT                          0x00

// Entry Active Bit
#define PROXY_TBL_OPT_ENTRY_ACTIVE_LEN                    0x01
#define PROXY_TBL_OPT_ENTRY_ACTIVE_BIT                    0x03

// Entry Valid Bit
#define PROXY_TBL_OPT_ENTRY_VALID_LEN                     0x01
#define PROXY_TBL_OPT_ENTRY_VALID_BIT                     0x04

// GPD MAC sequence capablities
#define PROXY_TBL_OPT_GPD_MAC_SEQ_CAP_LEN                 0x01
#define PROXY_TBL_OPT_GPD_MAC_SEQ_CAP_BIT                 0x05

// Lightweight Unicast GPS Bit
#define PROXY_TBL_OPT_LIGHTWIGHT_UNICAST_LEN              0x01
#define PROXY_TBL_OPT_LIGHTWIGHT_UNICAST_BIT              0x06

// Derived Group GPS Bit
#define PROXY_TBL_OPT_DGROUP_LEN                          0x01
#define PROXY_TBL_OPT_DGROUP_BIT                          0x07

// Commissioned Group GPS Bit
#define PROXY_TBL_OPT_CGROUP_LEN                          0x01
#define PROXY_TBL_OPT_CGROUP_BIT                          0x00

// FirstToForward Bit
#define PROXY_TBL_OPT_FIRST_TO_FORWARD_LEN                0x01
#define PROXY_TBL_OPT_FIRST_TO_FORWARD_BIT                0x01

// InRange Bit
#define PROXY_TBL_OPT_IN_RANGE_LEN                        0x01
#define PROXY_TBL_OPT_IN_RANGE_BIT                        0x02

// GPD Fixed Bit
#define PROXY_TBL_OPT_GPD_FIXED_LEN                       0x01
#define PROXY_TBL_OPT_GPD_FIXED_BIT                       0x03

// HasAllUnicastRoutes Bit
#define PROXY_TBL_OPT_HAS_ALL_ROUTES_LEN                  0x01
#define PROXY_TBL_OPT_HAS_ALL_ROUTES_BIT                  0x04

// Assigned Alias
#define PROXY_TBL_OPT_ASSIGNED_ALIAS_LEN                  0x01
#define PROXY_TBL_OPT_ASSIGNED_ALIAS_BIT                  0x05

// SecurityUse Bit
#define PROXY_TBL_OPT_SEC_USE_LEN                         0x01
#define PROXY_TBL_OPT_SEC_USE_BIT                         0x06

// Options Extension Bit
#define PROXY_TBL_OPT_EXT_LEN                             0x01
#define PROXY_TBL_OPT_EXT_BIT                             0x07

/*************************************************************
* Proxy Table Entry Options Bitfield
**************************************************************/
#define SEC_PARAMETERS_OPT_SECURITY_LVL_LEN                    0x02
#define SEC_PARAMETERS_OPT_SECURITY_LVL_BIT                    0x00

#define SEC_PARAMETERS_OPT_SECURITY_KEY_TYP_LEN                0x03
#define SEC_PARAMETERS_OPT_SECURITY_KEY_TYP_BIT                0x02

/*************************************************************
* Pairing bit fields
**************************************************************/
// Action bitfield
#define GP_ACTION_LEN                                     0x03
#define GP_ACTION_BIT                                     0x00
// Values
#define GP_ACTION_NO_ACTION                               0x00
#define GP_ACTION_EXTEND                                  0x01
#define GP_ACTION_REPLACE                                 0x02
#define GP_ACTION_NO_REMOVE_PAIRING                       0x03
#define GP_ACTION_NO_REMOVE_GPD                           0x04

#define GP_ACTION_SEND_PAIRING_BIT                        0x03

// Application ID bitfied
#define GP_OPT_APP_ID_LEN                                 0x03 // length of bitfield
#define GP_OPT_APP_ID_BIT                                 0x00
// Values
#define GP_OPT_APP_ID_GPD                                 0x00
#define GP_OPT_APP_ID_IEEE                                0x02

// Request type bitfied
#define GP_OPT_REQ_TYP_LEN                                0x02 // length of bitfield
#define GP_OPT_REQ_TYP_BIT                                0x03
// Values
#define GP_OPT_REQ_TYP_GPD_ID                             0x00
#define GP_OPT_REQ_TYP_INDEX                              0x01

// Add Sink bitfield
#define GP_OPT_ADD_SINK_BIT_FIELD_LEN                     0x01
#define GP_OPT_ADD_SINK_BIT                               0x03
// Values
#define GP_OPT_ADD_SINK_REMOVE_PAIRING                    0x00
#define GP_OPT_ADD_SINK_ADD_PAIRING                       0x01

// Remove GPD bitfield
#define GP_OPT_REMOVE_GPD_FIELD_LEN                       0x01
#define GP_OPT_PAIRING_CONFIG_EXTEND_ENTRY_BIT            0x00
#define GP_OPT_PAIRING_CONFIG_REPLACE_ENTRY_BIT           0x01
#define GP_OPT_PAIRING_CONFIG_REMOVE_GPD_BIT              0x02
#define GP_OPT_PAIRING_REMOVE_GPD_BIT                     0x04

// Values
#define GP_OPT_GPD_NO_REMOVE                              0x00
#define GP_OPT_GPD_REMOVE                                 0x01

// Communication Mode bitfield
#define GP_OPT_COMMUNICATION_MODE_FIELD_LEN               0x02
#define GP_OPT_PAIRING_CONFIG_COMMUNICATION_MODE_BIT      0x03
#define GP_OPT_PAIRING_COMMUNICATION_MODE_BIT             0x05
// Values
#define GP_OPT_COMMUNICATION_MODE_FULL_UNICAST            0x00
#define GP_OPT_COMMUNICATION_MODE_GRPCAST_DGROUP_ID       0x01
#define GP_OPT_COMMUNICATION_MODE_GRPCAST_GROUP_ID        0x02
#define GP_OPT_COMMUNICATION_MODE_LIGHT_UNICAST           0x03

// GPD Fixed
#define GP_OPT_GPD_FIXED_LEN                              0x01
#define GP_OPT_GPD_FIXED                                  0x07

// GPD sequence capablities
#define GP_OPT_GPD_MAC_SEC_CAP_LEN                        0x01
#define GP_OPT_GPD_SEC_CAP                                0x05   // Pairing Configuration options field
#define GP_OPT_GPD_MAC_SEC_CAP                            0x08   // Pairing command options field

// Security Use
// Security Level
#define GP_OPT_SEC_LEVEL_LEN                              0x02
#define GP_OPT_SEC_LEVEL                                  0x09

// Security Key Type
#define GP_OPT_SEC_KEY_TYPE_LEN                           0x03
#define GP_OPT_SEC_KEY_TYPE                               0x0B

// GPD Security Frame Counter Present
#define GP_OPT_GPD_SEC_COUNTER_LEN                        0x01
#define GP_OPT_GPD_SEC_COUNTER                            0x0E

// GPD Security Key Present
#define GP_OPT_GPD_SEC_KEY_LEN                            0x01
#define GP_OPT_GPD_SEC_KEY                                0x0F

// GPD Assigned Alias Present
#define GP_OPT_ALIAS_LEN                                  0x01
#define GP_OPT_PAIRING_CONFIG_ALIAS                       0x08
#define GP_OPT_PAIRING_ALIAS                              0x10

// Security Use
#define GP_OPT_SECURITY_USE_LEN                           0x01
#define GP_OPT_SECURITY_USE                               0x09

// App Info Present
#define GP_OPT_APP_INFO_LEN                               0x01
#define GP_OPT_APP_INFO                                   0x0A

// GPD Forwarding Radius Present
#define GP_OPT_FORWARDING_RADIUS_LEN                      0x01
#define GP_OPT_FORWARDING_RADIUS                          0x11

/*************************************************************
* App Info Parameters
**************************************************************/
#define APP_INFO_LEN                                      0x01
#define APP_INFO_MANUFACTURER_ID                          0x00
#define APP_INFO_MODE_ID                                  0x01
#define APP_INFO_GPD_COMMANDS                             0x02
#define APP_INFO_CLUSTER_LIST                             0x03

#define APP_CLUSTERS_FIELD_LEN                            0x04
#define APP_INFO_SERVER_IDS                               0x00
#define APP_INFO_CLIENT_IDS                               0x04

/*************************************************************
* Security Related Parameters
**************************************************************/
// Security Level
#define SEC_OPT_SEC_LEVEL_LEN                             0x02
#define SEC_OPT_SEC_LEVEL                                 0x00

// Security Key Type
#define SEC_OPT_SEC_KEY_TYPE_LEN                          0x03
#define SEC_OPT_SEC_KEY_TYPE                              0x02

/*************************************************************
* Gp Notification options bit field
**************************************************************/
#define GP_NTF_OPT_ALSO_UNICAST_LEN                       0x01
#define GP_NTF_OPT_ALSO_UNICAST                           0x03

#define GP_NTF_OPT_ALSO_DGROUP_LEN                        0x01
#define GP_NTF_OPT_ALSO_DGROUP                            0x04

#define GP_NTF_OPT_ALSO_CGROUP_LEN                        0x01
#define GP_NTF_OPT_ALSO_CGROUP                            0x05

// Security Level
#define GP_NTF_OPT_SEC_LEVEL_LEN                          0x02
#define GP_NTF_OPT_SEC_LEVEL                              0x06

// Security Key Type
#define GP_NTF_OPT_SEC_KEY_TYPE_LEN                       0x03
#define GP_NTF_OPT_SEC_KEY_TYPE                           0x00

#define GP_NTF_OPT_RX_AFTER_TX_LEN                        0x01
#define GP_NTF_OPT_RX_AFTER_TX                            0x03

#define GP_NTF_OPT_TX_QUEUE_FULL_LEN                      0x01
#define GP_NTF_OPT_TX_QUEUE_FULL                          0x04

#define GP_NTF_OPT_BIDIRECTIONAL_CAP_LEN                  0x01
#define GP_NTF_OPT_BIDIRECTIONAL_CAP                      0x05

#define GP_NTF_OPT_PROXY_INFO_LEN                         0x01
#define GP_NTF_OPT_PROXY_INFO                             0x06

/*************************************************************
* Gp Commissioning Notification options bit field
**************************************************************/

// Security Level
#define GP_CNTF_OPT_SEC_LEVEL_LEN                         0x02
#define GP_CNTF_OPT_SEC_LEVEL                             0x04

// Security Key Type
#define GP_CNTF_OPT_SEC_KEY_TYPE_LEN                      0x03
#define GP_CNTF_OPT_SEC_KEY_TYPE                          0x06

#define GP_CNTF_OPT_SEC_FAIL_LEN                          0x01
#define GP_CNTF_OPT_SEC_FAIL                              0x01

#define GP_CNTF_OPT_BIDIRECTIONAL_CAP_LEN                 0x01
#define GP_CNTF_OPT_BIDIRECTIONAL_CAP                     0x02

#define GP_CNTF_OPT_PROXY_INFO_LEN                        0x01
#define GP_CNTF_OPT_PROXY_INFO                            0x03

/*********************************************************************
* TYPEDEFS
*/


/*** GP Proxy Commissioning Mode Options ***/
typedef struct
{
  uint8_t action : 1;                           //!< The Action sub-field, if set to 0b1, indicates
                                              //!< a request to enter commissioning mode. If set
                                              //!< to 0b0, it indicates a request to exit
                                              //!< commissioning mode.
  uint8_t onCommissioningWindowExpires : 1;     //!< Exit on commissioning window timeout.
  uint8_t onFirstPairingSuccess : 1;            //!< Exit on pairing success.
  uint8_t onGpProxyCommissioningModeExit : 1;   //!< Exit on commissioning mode exit command.
  uint8_t channelPresent : 1;                   //!< If set to 0b0, indicates that the devices
                                              //!< SHOULD go to (or stay on) the operational
                                              //!< channel.
  uint8_t unicastCommunication : 1;             //!< If set to 0b0, indicates that the receiving
                                              //!< proxies SHALL send the GP Commissioning
                                              //!< Notification commands in broadcast. If set
                                              //!< to 0b1, it indicates that the receiving
                                              //!< proxies SHALL send the GP Commissioning
                                              //!< Notification commands in unicast to the
                                              //!< originator of the GP Proxy Commissioning
                                              //!< Mode command.
  uint8_t reserved : 2;                         //!< Reserved.
} gpProxyCommissioningModeOptions;

typedef struct _gpProxyTableOptions_t
{
  uint8_t appId : 3;                 //!< Contains the information about the application
  uint8_t entryActive : 1;           //!< if set to 0b1, indicates that the entry is active
  uint8_t entryValid : 1;            //!< if set to 0b1, indicates that the entry is valid
  uint8_t sequenceNumberCap : 1;     //!< Contains the information on the sequence number
                                   //!< capabilities of this GPD.
  uint8_t lightUnicast : 1;          //!< if set to 0b1, indicates that lightUnicast is supported
  uint8_t derivedGroupGPS : 1;       //!< if set to 0b1, indicates that derivedGroupGPS
                                   //!< is supported
  uint8_t commissionedGroupGPS : 1;  //!< if set to 0b1, indicates that commissionedGroupGPS
                                   //!, is supported
  uint8_t firstToForward : 1;        //!< if set to 0b1, this proxy is firstToForward
  uint8_t inRange : 1;               //!< if set to 0b1, GPD is in range
  uint8_t gpdFixed : 1;              //!< Contains information if the location of this
                                   //!< GPD is expected to change.
  uint8_t hasAllUnicastRoutes : 1;   //!< if set to 0b1, indicates that the proxy has active
                                   //!< routes to all full unicast sinks for this GPD;
                                   //!< if set to 0b0, it indicates that at least one full
                                   //!< unicast route is missing.
  uint8_t assignedAlias : 1;         //!< if set to 0b1, indicates that the assigned alias as
                                   //!< stored in the GPD Assigned Alias parameter SHALL
                                   //!< be used instead of the alias derived from the GPD ID.
  uint8_t securityUse : 1;           //!< if set to 0b1, indicates that security-related
                                   //!< parameters of the Sink Table entry are present.
  uint8_t optionsExtension: 1;       //!< if set to 0b1, indicates that the Extended Options
                                   //!< field is present.
} gpProxyTableOptions_t;

/*** GP Proxy Commissioning Mode Command ***/
typedef struct
{
  gpProxyCommissioningModeOptions options;  //!< Options field of the GP Proxy
                                            //!< Commissioning Mode command.
  uint16_t commissioningWindow;               //!< Carries the value of
                                            //!< gpsCommissioningWindow attribute.
  uint8_t channel;                            //!< Carries value of commissioning channel.
} gpProxyCommissioningModeCmd_t;
/** @} End GP_COMMANDS */

/**
 * @defgroup GPS_TABLE_OPTIONS GP Sink Table Options field
 * @{
 * @brief Options field of Sink Table
 */
 /*** GP Sink Table Options ***/
typedef struct _gpSinkTableOptions_t
{
  uint8_t appId : 3;             //!< Contains the information about the application
                               //!< used by the GPD.
  uint8_t communicationMode : 2; //!< Contains the information about the accepted
                               //!< tunneling mode for this GPD.
  uint8_t sequenceNumberCap : 1; //!< Contains the information on the sequence number
                               //!< capabilities of this GPD.
  uint8_t rxOnCapability : 1;    //!< Contains the information about reception
                               //!< capability on this GPD.
  uint8_t gpdFixed : 1;          //!< Contains information if the location of this
                               //!< GPD is expected to change.
  uint8_t assignedAlias : 1;     //!< If set to 0b1, indicates that the assigned
                               //!< alias as stored in the GPD Assigned Alias
                               //!< parameter SHALL be used instead of the alias
                               //!< derived from the GPD ID in case of derived
                               //!< groupcast or full unicast communication.
                               //!< If set to 0b0, the derived alias is used
                               //!< for those communication modes.
  uint8_t securityUse : 1;       //!< If set to 0b1, indicates that security-related
                               //!< parameters of the Sink Table entry are present.
} gpSinkTableOptions_t;
/** @} End GPS_TABLE_OPTIONS */

/**
 * @defgroup COMMISSIONING_COMMANDS_FIELDS Commissioning Commands fields
 * @{
 * @brief Structs used in commissioning commands
 */
/*** GP Commissioning Reply Options ***/
typedef struct _gpCommissioningReplyOptions_t
{
  uint8_t panIdPresent : 1;          //!< if set to 0b1, indicates that the PANId
                                   //!< field is present, and carries the value
                                   //<! of the network operational PANId.
  uint8_t gpdSecurityKeyPresent : 1; //!< Indicates that GPD key is present.
  uint8_t gpdKeyEncryption : 1;      //!< If set to 0b1, indicates that security key in
                                   //!< in the payload is encrypted.
  uint8_t securityLevel :2;          //!< Indicates the device security capabilities
                                   //!< during normal operation.
  uint8_t keyType: 3;                //!< Contains the type of the key to be used
                                   //!< for GPDF protection in operation
} gpCommissioningReplyOptions_t;

/*** GPD Commissioning Options ***/
typedef struct _gpdCommissioningOptions_t
{
  uint8_t sequenceNumberCap : 1;    //!< If the value of this sub-field is 0b1, then
                                  //!< it indicates the GPD uses incremental MAC
                                  //!< sequence number. If the value of this
                                  //!< sub-field is 0b0, then it indicates that
                                  //!< the GPD uses random MAC sequence number.
  uint8_t rxOnCap : 1;              //!< If set to 0b1, it indicates that the GPD
                                  //!< has receiving  capabilities in operational mode.
                                  //!< If set to 0b0, it indicates that the GPD
                                  //!< does not enable its receiver in operational mode.
  uint8_t appInfoPresent : 1;       //!< If set to 0b1, it indicates that the Application
                                  //!< information field is present. If set to 0b0,
                                  //!< it indicates that the Application information
                                  //!< field is absent.
  uint8_t reserved : 1;             //!< Reserved byte.
  uint8_t panIdRequest : 1;         //!< If the value of this sub-field is 0b1, then
                                  //!< the GPD requests to receive the PAN ID value
                                  //!< of the network. If the value of this sub-field
                                  //!< is 0b0, then the GPD does not request to
                                  //!< receive the PAN ID value.
  uint8_t securityKeyRequest : 1;   //!< If the value of this sub-field is set to 0b1,
                                  //!< then the GPD requests to receive the GP Security
                                  //!< Key. If the value of this sub-field is 0b0,
                                  //!< then the GPD does not request to receive
                                  //!< the GP Security Key.
  uint8_t fixedLocation : 1;        //!< If the value of this sub-field is 0b0, then
                                  //!< it indicates that the GPD can change its
                                  //!< position during its operation in the network.
                                  //!< If the value of this sub-field is 0b1, then
                                  //!< the GPD is not expected to change its position
                                  //!< during its operation in the network.
  uint8_t extendedOptionsField : 1; //!< If the value of this sub-field is 0b1, then
                                  //!< it indicates that the Extended Options
                                  //!< field is present.
} gpdCommissioningOptions_t;

/*** GPD Commissioning Extended Options ***/
typedef struct _gpdCommissioningExtOptions_t
{
  uint8_t securityLevel : 2;          //!< Indicates the device’s security capabilities
                                    //!< during normal operation.
  uint8_t keyType : 3;                //!< Indicates the type of the security key
                                    //!< this GPD is configured with.
  uint8_t gpdKeyPresent : 1;          //!< Indicates that GPD key is present.
  uint8_t keyEncryption : 1;          //!< If set to 0b1, indicates that security key in
                                    //!< in the payload is encrypted.
  uint8_t outgoingCounterPresent : 1; //!< If set to 0b1, indicates that the
                                    //!< GPDoutgoingCounter is present.
} gpdCommissioningExtOptions_t;

/*** GPD Commissioning App Info ***/
typedef struct _gpdCommissioningAppInfo_t
{
  uint8_t manufacturerIdPresent : 1;  //!< If set to 0b1, it indicates that the
                                    //!< ManufacturerID field is present. If set
                                    //!< to 0b0, it indicates that the ManufacturerID
                                    //!< field is absent.
  uint8_t modeIdPresent : 1;          //!< If set to 0b1, it indicates that the ModelID
                                    //!< field is present. If set to 0b0, it indicates
                                    //!< that the ModelID field is absent.
  uint8_t gpdCommandsPresent : 1;     //!< If set to 0b1, it indicates that the fields
                                    //!< Number of GPD commands and GPD CommandID list
                                    //!< are present. If set to 0b0, it indicates
                                    //!< that both those field are absent.
  uint8_t clusterListPresent : 1;     //!< If set to 0b1, it indicates that the
                                    //!< Cluster List field is present. If set to
                                    //!< 0b0, it indicates that this field is absent.
} gpdCommissioningAppInfo_t;

/*** GPD Commissioning Cluster Length ***/
typedef struct _gpdCommissioningClusterLength_t
{
  uint8_t numberOfServerIds : 4;  //!< numberOfServerIds value.
  uint8_t numberOfClientIds : 4;  //!< numberOfClientIds value.
} gpdCommissioningClusterLength_t;
/** @} End COMMISSIONING_COMMANDS_FIELDS */

/*** GPD Channel Request ***/
typedef struct _gpdChannelRequest_t
{
  uint8_t rxInNextAttempt : 4;        //!< Channel for next commissioning attempt.
  uint8_t rxInSeccondAttempt : 4;     //!< Channel for second commissioning attempt.
} gpdChannelRequest_t;

/*** GPD Channel Configuration ***/
typedef struct _gpdChannelConfiguration_t
{
  uint8_t operationalChannel : 4; //!< Value of operational channel.
  uint8_t basic : 1;              //!< Indicates if it supports bidirectional operation.
  uint8_t reserved : 3;           //!< Reserved field.
} gpdChannelConfiguration_t;

/*** GP Commissioning Notification Options ***/
typedef struct _gpCommissioningNotificationOptions_t
{
  uint8_t appId : 3;                    //!< Contains the information about the
                                      //!< application used by the GPD.
  uint8_t rxAfterTx : 1;                //!< Indicates that Rx will be active after
                                      //!< transmission of a frame on GPD.
  uint8_t securityLevel : 2;            //!< Indicates the device’s security
                                      //!< capabilities during normal operation.
  uint8_t securityKeyType : 3;          //!< Contains the type of the key to be used
                                      //!< for GPDF protection in operation
  uint8_t securityProcessingFailed : 1; //!< If set to 0b1 indicates security failure.
  uint8_t bidirectionalCapability : 1;  //!< If set to 0b1 indicates bidirectional
                                      //!< capabilities are supported on GPD.
  uint8_t proxyInfoPresent : 1;         //!< If set to 0b1 indicates Proxy info is present.
  uint8_t reserved : 3;                 //!< Reserved field.
} gpCommissioningNotificationOptions_t;

/*** GP Security Options ***/
typedef struct _gpSecurityOptions_t
{
  uint8_t securityLevel : 2;    //!< Indicates the device’s security capabilities
                              //!< during normal operation.
  uint8_t securityKeyType : 3;  //!< Contains the type of the key to be used
} gpSecurityOptions_t;

/*** GP Pairing Options ***/
typedef struct _gpPairingOptions_t
{
  uint8_t appId : 3;                       //!< Contains the information about the
                                         //!< application used by the GPD.
  uint8_t addSink : 1;                     //!< If set to 0b1 the pairing is being
                                         //!< added. If set to 0b0 the pairing is
                                         //!< being removed;
  uint8_t removeGpd : 1;                   //!< If set to 0b1, indicates that the
                                         //!< GPD identified by the GPD ID is being
                                         //!< removed from the network.
  uint8_t communicationMode : 2;           //!< Communication mode requested by the sink.
  uint8_t gpdFixed : 1;                    //!< Contains information if the location
                                         //!< of this GPD is expected to change.
  uint8_t sequenceNumberCap : 1;           //!< If set to 0b1 GPD support sequence
                                         //!< number for outgoing frames.
  uint8_t securityLevel : 2;               //!< Indicates the device’s security capabilities
                                         //!< during normal operation.
  uint8_t securityKeyType : 3;             //!< Contains the type of the key to be used
  uint8_t securityFrameCounterPresent : 1; //!< If set to 0b1, indicates that the
                                         //!< securityFrameCounterPresent is present.
  uint8_t securityKeyPresent : 1;          //!< If set to 0b1, indicates that the
                                         //!< securityKeyPresent is present.
  uint8_t assignedAliasPresent : 1;        //!< If set to 0b1, indicates that the
                                         //!< assignedAliasPresent is present.
  uint8_t forwardingRadiusPresent : 1;     //!< If set to 0b1, indicates that the
                                         //!< forwardingRadiusPresent is present.
} gpPairingOptions_t;
/** @} End COMMAND_OPTIONS */

/*********************************************************************
 * FUNCTIONS
 */

/*
* @brief   Converts Commissioning Notification options field from
*          uint16_t to gpCommissioningNotificationOptions_t struct
*/
extern void gp_bfParse_commissioningNotificationOptions( gpCommissioningNotificationOptions_t* pOptions, uint16_t bitfield );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* GP_BIT_FIELDS_H */
