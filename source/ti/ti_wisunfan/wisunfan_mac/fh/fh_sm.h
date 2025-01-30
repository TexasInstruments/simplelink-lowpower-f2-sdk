/******************************************************************************

 @file fh_sm.h

 @brief TIMAC 2.0 FH finite state machine API

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated

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

#ifndef FH_SM_H
#define FH_SM_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

typedef enum __fh_state
{
  FH_ST_HOP             = 0,
  FH_ST_TX              = 1,
  FH_ST_RX              = 2,
  FH_ST_ASYNC           = 3,
  FH_ST_EDFE            = 4,

  FH_ST_NUM_STATES
} FH_state_t;


typedef enum __fh_event
{

  FH_EVT_UC_TIMER       = 0,
  FH_EVT_BC_TIMER       = 1,
  FH_EVT_RX_START       = 2,
  FH_EVT_RX_DONE        = 3,
  FH_EVT_TX_START       = 4,
  FH_EVT_TX_DONE        = 5,
  FH_EVT_ASYNC_START    = 6,
  FH_EVT_ASYNC_DONE     = 7,
  FH_EVT_EDFE_REQ       = 8,
  FH_EVT_EDFE_RCV       = 9,
  FH_EVT_EDFE_FIN       = 10,
  FH_EVT_NUM_EVENTS
} FH_smEvent_t;

#define FHFSM_MAX_DEBUG_EVENTS				(32)

/* FH State Machine debugging */
typedef struct
{
    uint16_t state_evt[FHFSM_MAX_DEBUG_EVENTS];
    uint8_t  idx;
} FHFSM_debugBuffer_t;

/* State transition function */
typedef FHAPI_status (*FSM_eventActivation_t)(uint16_t currState_p, uint16_t event, void* data_p);

//typedef void (*FSM_action_t)(void);
typedef void (*FSM_action_t)(void* data_p);
/* Action cell */
typedef  struct __attribute__((__packed__))
{
  uint8_t                nextState;      /* next state */
  FSM_action_t          actionFunc_p;   /**< action function */
}FSM_actionCell_t;

/* Matrix type */
typedef FSM_actionCell_t*       FSM_matrix_t;

/* General FSM structure */
typedef struct
{
  const FSM_actionCell_t  *stateEventMatrix; /* State and Event matrix */
  uint8_t                  maxStates;        /* Max Number of states in the matrix */
  uint8_t                  maxEvents;        /* Max Number of events in the matrix */
  uint8_t                  previousState;    /* Previous state */
  uint8_t                  currentState;     /* Current state */
  FSM_eventActivation_t   transitionFunc_p; /* State transition function */

}FSM_stateMachine_t;

/* ------------------------------------------------------------------------------------------------
 *                                           Global Externs
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Prototypes
 * ------------------------------------------------------------------------------------------------
 */
FHAPI_status FHSM_event(FSM_stateMachine_t *fsm_p,
                        uint16_t event, void *data_p);

FHAPI_status FSM_transaction_debug(uint16_t currState,
                                   uint16_t event, void *pdata);

#endif
