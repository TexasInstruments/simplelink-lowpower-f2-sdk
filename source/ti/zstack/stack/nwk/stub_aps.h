/**************************************************************************************************
  Filename:       stub_aps.h
  Revised:        $Date: 2008-1-04 13:13:13 -0700 (Fri, 04 Jan 2008) $
  Revision:       $Revision: 1 $

  Description:    Primitives of the Stub Application Support Sub Layer Task functions.


  Copyright 2004-2011 Texas Instruments Incorporated.

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

#ifndef SAPS_H
#define SAPS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

#include "mac_api.h"
#include "zmac.h"
#include "aps_mede.h"
#include "af.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Inter-PAN frame doesn't include endpoints. Use this endpoint to distinguish the
// Inter-PAN frames passed between the Stub APS and Application.
// Note: Endpoints 0xF1 - 0xFE are currently reserved.
#define STUBAPS_INTER_PAN_EP        0xFE

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint8_t StubAPS_TaskID;

/*********************************************************************
 * STUB APS FUNCTIONS
 */

/*
 * Stub APS Task Initialization
 */
extern void StubAPS_Init( uint8_t task_id );

/*
 * Stub APS Event Loop
 */
extern uint32_t StubAPS_ProcessEvent( uint8_t task_id, uint32_t events );

/*
 * This function changes the device's channel for inter-PAN communication.
 */
extern ZStatus_t StubAPS_SetInterPanChannel( uint8_t channel );

/*
 * This function sets the device's channel back to the NIB channel.
 */
extern ZStatus_t StubAPS_SetIntraPanChannel( void );

/*
 * This function checks to see if a PAN is an Inter-PAN.
 */
extern uint8_t StubAPS_InterPan( uint16_t panId, uint8_t endPoint );

/*
 * This function is used to register the application with Stub APS.
 */
extern void StubAPS_RegisterApp( endPointDesc_t *epDesc );

/*********************************************************************
 * INTER-PAN FUNCTIONS
 */

/*
 * This function requests the transfer of data from the next higher layer
 * to a single peer entity.
 */
extern ZStatus_t INTERP_DataReq( APSDE_DataReq_t *req );

/*
 * This function requests the MTU (Max Transport Unit) of the Inter-PAN
 * Data Service.
 */
extern uint8_t INTERP_DataReqMTU( void );

/*
 * This function processes the data confirm from the MAC layer.
 */
extern void INTERP_DataConfirm( ZMacDataCnf_t *dataCnf );

/*
 * This function indicates the transfer of a data SPDU (MSDU)
 * from the MAC layer to the local application layer entity.
 */
extern void INTERP_DataIndication( macMcpsDataInd_t *dataInd );


/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* SAPS_H */
