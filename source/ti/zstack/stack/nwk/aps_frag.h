/**************************************************************************************************
  Filename:       aps_frag.h
  Revised:        $Date: 2011-05-27 16:00:27 -0700 (Fri, 27 May 2011) $
  Revision:       $Revision: 26145 $

  Description:    Implements APS Application Data Unit Fragmentation


  Copyright 2006-2011 Texas Instruments Incorporated.

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
#ifndef APSF_H
#define APSF_H

#ifdef __cplusplus
extern "C"
{
#endif

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "zcomdef.h"
#include "af.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

#define APSF_SCHED_EVT             0x0001
#define APSF_MAX_WINDOW_SIZE       8  // Maximum allowable fragmentation window size.

/* ------------------------------------------------------------------------------------------------
 *                                            Macros
 * ------------------------------------------------------------------------------------------------
 */

#define APSF_Enabled              (APSF_taskID != 0xff)

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

extern uint8_t APSF_taskID;

/* ------------------------------------------------------------------------------------------------
 *                                          Functions
 * ------------------------------------------------------------------------------------------------
 */

/*
 * Task Initialization
 */
extern void APSF_Init(uint8_t task_id);

/*
 * Task Event Processor
 */
extern void APSF_Init(uint8_t task_id);
extern uint32_t APSF_ProcessEvent( uint8_t task_id, uint32_t events );
extern afStatus_t APSF_SendFragmented(APSDE_DataReq_t *pReq);
extern void APSF_ProcessAck(aps_FrameFormat_t *aff, uint16_t srcAddr, uint8_t status);
extern void APSF_SendOsalMsg(uint8_t *msgPtr);

typedef afStatus_t APSF_SendFragmented_t(APSDE_DataReq_t *pReq);
typedef void APSF_ProcessAck_t(aps_FrameFormat_t *aff, uint16_t srcAddr, uint8_t status);
typedef void APSF_SendOsalMsg_t(uint8_t *msgPtr);

extern APSF_SendFragmented_t *apsfSendFragmented;
extern APSF_ProcessAck_t *apsfProcessAck;
extern APSF_SendOsalMsg_t *apsfSendOsalMsg;

#ifdef __cplusplus
}
#endif
#endif
/**************************************************************************************************
*/
