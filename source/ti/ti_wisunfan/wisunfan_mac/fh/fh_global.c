/******************************************************************************

 @file fh_global.c

 @brief TIMAC 2.0 FH API

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2023, Texas Instruments Incorporated

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

 ******************************************************************************
 
 
 *****************************************************************************/

/******************************************************************************
 Includes
 *****************************************************************************/

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif
#include <stddef.h>

#include <stdint.h>
#include "hal_types.h"
#include "fh_api.h"
#include "fh_data.h"
#include "fh_dh1cf.h"
#include "fh_ie.h"
#include "fh_mgr.h"
#include "fh_nt.h"
#include "fh_pib.h"
#include "fh_sm.h"
#include "fh_util.h"
#include "mac_data.h"

#ifdef FEATURE_FREQ_HOP_MODE
/*!
 This module is the FH module interface for the HMAC and LMAC. All function
 APIS in this file can be used by HMAC and LMAC.
 */

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/******************************************************************************
 Local variables
 *****************************************************************************/

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/

/******************************************************************************
 Glocal variables
 *****************************************************************************/
/*!
 FH_hnd is the FH module handler. It is used to store the FH internal data
 structure
 */
FH_HND_s FH_hnd;
/*!
 FH NT handle
 */
FHNT_HND_s FHNT_hnd;
/*!
 FH_txTiming
 */
FH_TX_TIMING_TABLE_s FH_txTiming =
{
 .maxNumEntry = FH_TX_TIMING_TABLE_SIZE,
};

/*!
 FH temp table
 */
FHNT_TEMP_TABLE_s FHNT_tempTable;
/*!
 FH channel Plan (fh_ie.c)
 */

// channel Plan table size is one
FHIE_channelPlan_t FHIE_channelPlan[NO_OF_CHANNEL_PLAN] =
{
#ifdef FEATURE_WISUN_SUPPORT
    {1, 1,  902200, 0, 129}
#else
    {1, 7,  902200, 0, 129}
#endif
};

#if 0
FHIE_channelPlan_t FHIE_channelPlan[NO_OF_CHANNEL_PLAN] =
{
#ifdef FEATURE_WISUN_SUPPORT
    {1, 1,  902200, 0, 129},
#else
    {1, 7,  902200, 0, 129},
#endif
    {1, 2,  902400, 1, 64},
    {1, 3,  902600, 2, 42},
    {2, 1,  920600, 0, 38},
    {2, 2,  920900, 1, 18},
    {2, 3,  920800, 2, 12},
    {3, 2,  863125, 0, 34}
};
#endif

/*!
 FH NT debug
 */
FHNT_DEBUG_s FHNT_Dbg;

/*!
 FH PIB database
 */
FHPIB_DB_t FHPIB_db;

#ifdef FEATURE_WISUN_SUPPORT
FHPIB_DB_new_t FHPIB_db_new;
#endif

FHNT_OPT_Table_s FHNT_tableOpt;

const FSM_actionCell_t FH_stateMatrix[FH_ST_NUM_STATES][FH_EVT_NUM_EVENTS] =
{
    /* HOP state */
    {
         /* Next State */                /* Action */
         /*! UC_TIMER_EVT */
         {FH_ST_HOP,                     FHMGR_updateRadioUCChannel},
         /*! BC_TIMER_EVT */
         {FH_ST_HOP,                     FHMGR_updateRadioBCChannel},
         /*! RX_START from LMAC */
         {FH_ST_RX,                      FHUTIL_noAction},
         /*! RX_DONE */
         {FH_ST_HOP,                     FHUTIL_assert},
         /*! TX_START */
         {FH_ST_TX,                      FHUTIL_noAction},
         /*! TX_DONE */
         {FH_ST_HOP,                     FHUTIL_assert},
         /*! ASYNC START */
         {FH_ST_ASYNC,                   FHDATA_procASYNCReq},
         /*! ASYNC DONE/STOP */
         {FH_ST_HOP,                     FHUTIL_assert},
         /*! EDFE_REQ, EDFE_START, Req frame type and parsed FCIE */
         {FH_ST_EDFE,                    FHDATA_startEDFEReq},
         /*! EDFE_RCV, EDFE_DONE, EDFE TIMER EXP/DONE */
         {FH_ST_EDFE,                    FHDATA_procEDFEReq},
         /*! EDFE_FIN */
         {FH_ST_HOP,                     FHUTIL_noAction},
    },

    /* TX state */
    {
         /*! UC_TIMER_EVT */
         {FH_ST_TX,                      FHMGR_pendUcChUpdate},
         /*! BC_TIMER_EVT */
         {FH_ST_TX,                      FHMGR_pendBcChUpdate},
         /*! RX_START */
         {FH_ST_TX,                      FHUTIL_assert},
         /*! RX_DONE  */
         {FH_ST_TX,                      FHUTIL_assert},
         /*! TX_START */
         {FH_ST_TX,                      FHUTIL_assert},
         /*! TX_DONE  */
         {FH_ST_HOP,                     FHMGR_updateHopping},
         /*! ASYNC_START */
         {FH_ST_TX,                      FHUTIL_assert},
         /*! ASYNC DONE/STOP */
         {FH_ST_TX,                      FHUTIL_assert},
         /*! EDFE from higher layer/Poll Req */
         {FH_ST_TX,                      FHUTIL_noAction},
         /*! EDFE TIMER EXP */
         {FH_ST_TX,                      FHUTIL_assert},
         /*! EDFE_FIN */
         {FH_ST_TX,                      FHUTIL_assert},
    },

    /* RX state */
    {
         /*! UC_TIMER_EVT */
         {FH_ST_RX,                      FHMGR_pendUcChUpdate},
         /*! BC_TIMER_EVT */
         {FH_ST_RX,                      FHMGR_pendBcChUpdate},
         /*! RX_START */
         {FH_ST_RX,                      FHUTIL_assert},
         /*! RX_DONE */
         {FH_ST_HOP,                     FHMGR_updateHopping},
         /*! TX_START */
         {FH_ST_RX,                      FHUTIL_assert},
         /*! TX_DONE */
         {FH_ST_RX,                      FHUTIL_assert},
         /*! ASYNC START */
         {FH_ST_RX,                      FHDATA_requeuePkt},
         /*! ASYNC DONE/STOP */
         {FH_ST_RX,                      FHUTIL_assert},
         /*! EDFE_REQ, EDFE START based on RX frame FCIE, higher layer control */
         {FH_ST_RX,                      FHUTIL_noAction},
         /*! EDFE_RCV, EDFE TIMER EXP */
         {FH_ST_EDFE,                    FHDATA_procEDFEReq},
         /*! EDFE_FIN */
         {FH_ST_RX,                      FHUTIL_assert},
    },

    /* ASYNC  state */
    {
         /*! UC_TIMER_EVT */
         {FH_ST_ASYNC,                   FHMGR_pendUcChUpdate},
         /*! BC_TIMER_EVT */
         {FH_ST_ASYNC,                   FHMGR_pendBcChUpdate},
         /*! RX_START */
         {FH_ST_ASYNC,                   FHUTIL_noAction},
         /*! RX_DONE   */
         {FH_ST_ASYNC,                   FHUTIL_noAction},
         /*! TX_START */
         {FH_ST_ASYNC,                   FHUTIL_assert},
         /*! TX_DONE */
         {FH_ST_ASYNC,                   FHDATA_procASYNCReq},
         /*! ASYNC START */
         {FH_ST_ASYNC,                   FHUTIL_assert},
         /*! ASYNC DONE/STOP */
         {FH_ST_HOP,                     FHMGR_updateHopping},
         /*! EDFE START based on RX frame FCIE, higher layer control */
         {FH_ST_ASYNC,                   FHUTIL_assert},
         /*! EDFE TIMER EXP */
         {FH_ST_ASYNC,                   FHUTIL_assert},
         /*! EDFE_FIN */
         {FH_ST_ASYNC,                   FHUTIL_assert},
    },
    /* EDFE   state */
    {
         /*! UC_TIMER_EVT */
         {FH_ST_EDFE,                    FHMGR_pendUcChUpdate},
         /*! BC_TIMER_EVT */
         {FH_ST_EDFE,                    FHMGR_pendBcChUpdate},
         /*! RX_START */
         {FH_ST_EDFE,                    FHUTIL_noAction},
         /*! RX_DONE */
         {FH_ST_EDFE,                    FHUTIL_noAction},
         /*! TX_START */
         {FH_ST_EDFE,                    FHUTIL_noAction},
         /*! TX_DONE  */
         {FH_ST_EDFE,                    FHUTIL_noAction},
         /*! ASYNC START  //based on frame type from HMAC */
         {FH_ST_EDFE,                    FHDATA_requeuePkt},
         /*! ASYNC DONE/STOP */
         {FH_ST_EDFE,                    FHUTIL_assert},
         /*! EDFE_REQ, EDFE START based on RX frame FCIE, higher layer control */
         {FH_ST_EDFE,                    FHUTIL_noAction},
         /*! EDFE_RCV, EDFE TIMER EXP */
         {FH_ST_EDFE,                    FHDATA_procEDFEReq},
         /*! EDFE_FIN */
         {FH_ST_HOP,                     FHMGR_updateHopping},
    },
};
FHFSM_debugBuffer_t FHFSM_Dbg;

/*****************************************************************************/
/* CSMA SM State machine                                                     */
/*****************************************************************************/
FSM_stateMachine_t FH_FSM =
{
    &FH_stateMatrix[0][0],
    FH_ST_NUM_STATES,
    FH_EVT_NUM_EVENTS,

    FH_ST_HOP,  /* prev state */
    FH_ST_HOP,  /* current state */
    /* for debug */
    FSM_transaction_debug
};

/*!
 FH_Dbg is the FH module debug handler. It is used to store the FH debug
 information
 */
#ifdef FH_HOP_DEBUG
FH_DEBUG_s FH_Dbg;
#endif

uint8 macAddrSize[4] = {0, 0, 2, 8};
const uint8  fhSupFrameTypes[FH_SUPPORTED_FRAME_TYPES] = {MAC_FRAME_TYPE_ASYNC, MAC_FRAME_TYPE_EDFE,
                                                          MAC_FRAME_TYPE_BROADCAST, MAC_FRAME_TYPE_UNICAST};
#endif
