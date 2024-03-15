/**************************************************************************************************
  Filename:       gp_Interface.h
  Revised:        $Date: 2016-05-23 11:51:49 -0700 (Mon, 23 May 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the Green Power interface.


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

#ifndef GP_INTERFACE_H
#define GP_INTERFACE_H

#ifdef __cplusplus
extern "C"
{
#endif



/*********************************************************************
 * INCLUDES
 */
#include "zd_object.h"

/*********************************************************************
 * MACROS
 */




 /*********************************************************************
 * CONSTANTS
 */

/* The time the Green Power EndPoint of the proxy keeps the information on the received
GPDF, in order to filter out duplicates. By default 2 seconds */

#define gpDuplicateTimeout    2000


/* Time that the Basic proxy device will be absent of the operational network due
  to bidirectional commissioning Section A.3.9.1 step 8*/
#define gpBirectionalCommissioningChangeChannelTimeout    5000

/* Per GP spec section A.1.5.2.1.1 the minimum gpTxQueue entry is 1 */
/* NOTE: Z-Stack only supports 1 entry in the queue at this moment */
#define GP_TX_QUEUE_MAX_ENTRY          1

 /*********************************************************************
 * TYPEDEFS
 */

typedef uint8_t (*gpChangeChannelReq_t) (void);
typedef void  (*gpCommissioningMode_t) (bool, bool, uint16_t);

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * FUNCTION MACROS
 */



/*********************************************************************
 * FUNCTIONS
 */

/*
 * @brief GP Register for chaching channel
 */
extern void gp_RegisterGPChangeChannelReqForBDBCB(gpChangeChannelReq_t gpChangeChannelReqCB);

/*
 * @brief GP Task Event Processing Function
 */
extern uint32_t gp_event_loop( uint8_t task_id, uint32_t events );


/*
 * @brief       Initialization function for the Green Power Stubs.
 */
extern void gp_Init( byte task_id );

/*
 * @brief       Register a callback in which the application will be notified about a change
 *              of channel for at most gpBirectionalCommissioningChangeChannelTimeout ms
 *              to perform GP bidirectional commissioning in the channel parameter.
 */
extern void gp_RegisterGPChangeChannelReqCB(gpChangeChannelReq_t gpChangeChannelReqCB);

#ifdef __cplusplus
}
#endif


#endif /* GP_INTERFACE_H */










