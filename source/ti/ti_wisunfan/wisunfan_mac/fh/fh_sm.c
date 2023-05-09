/******************************************************************************

 @file fh_sm.c

 @brief TIMAC 2.0 FH finite state machine API

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

#include "mac_pib.h"
#include "fh_api.h"
#include "fh_util.h"
#include "fh_mgr.h"
#include "fh_data.h"
#include "fh_sm.h"

#ifdef FEATURE_FREQ_HOP_MODE

/******************************************************************************
  ROM jump table will support
  1. TIMAC ROM image build
  2. TIMAC stack build without ROM image
  3. TIMAC stack with calling ROM image
 *****************************************************************************/
#include "rom_jt_154.h"


/**************************************************************************************************
 * @fn          FSM_transaction_debug
 *
 * @brief       This function tracks the state machine transaction and record
 * 			    the current state and events
 *
 * input parameters
 *
 * @param		currState - current state of FH state machine
 * @param		event - FH event
 * @param		pdata - input data, it can be casted to FH handle
 *
 * output parameters
 *
 * None
 *
 * @return      the status of transaction function. It is always FHAPI_STATUS_SUCCESS
 **************************************************************************************************
 */
FHAPI_status FSM_transaction_debug(uint16_t currState, uint16_t event, void* pdata)
{
    FHFSM_debugBuffer_t *pDbg;
    uint16_t idx,temp;

    /* avoid compiler warning */
    (void) pdata;

    pDbg = &FHFSM_Dbg;

    /* get index */
    idx = pDbg->idx;

    /* save the current state and event */
    temp = event & 0xFF;
    temp |= (currState & 0xFF) << 8 ;
    pDbg->state_evt[ idx++ ] = temp;

    /* handle buffer wrap around */
    if (idx >= FHFSM_MAX_DEBUG_EVENTS)
        idx = 0;

    pDbg->idx = idx;

    return FHAPI_STATUS_SUCCESS;
}

/******************************************************************************
* FUNCTION NAME: FSM_event
*
* DESCRIPTION:   This function perform event transition in the matrix
*
* Return Value:
*
* Input Parameters:  fsm_p - pointer to the state machine structure
*                    event - event causing transition
*                    data_p - data for activation function
*
* Output Parameters: update current state after action taken
* Functions Called:
*
* RLiang:
* we may need to add the mutex code when update FSM current state
******************************************************************************/
FHAPI_status FHSM_event(FSM_stateMachine_t *fsm_p,
        uint16_t         event,
        void             *data_p)
{
    FHAPI_status status = FHAPI_STATUS_SUCCESS;
    uint32_t     idx;

    /* boundary check */
    if ((fsm_p->currentState >= fsm_p->maxStates) || (event >= fsm_p->maxEvents))
    {
        return(FHAPI_STATUS_ERR);
    }

    /* Disable interrupts to enter critical section*/
    /* update FHSM Dbg History only on state transitions */
    if(fsm_p->currentState != fsm_p->previousState)
    {
        if (fsm_p->transitionFunc_p)
        {   /* user provide transition function */
            (fsm_p->transitionFunc_p)(fsm_p->currentState, event, data_p);
        }
    }

    fsm_p->previousState = fsm_p->currentState;

    /* Index to cell */
    idx = (fsm_p->currentState * fsm_p->maxEvents) + event;

    /* update current state */
    fsm_p->currentState = fsm_p->stateEventMatrix[idx].nextState;

    /* Restore interrupts to exit critical section  */

    /* activate transition function */
    (*fsm_p->stateEventMatrix[idx].actionFunc_p)(data_p);
    return(status);
}

#endif
