/**************************************************************************************************
  Filename:       per_test.h
  Revised:        $Date: 2016-05-23 11:51:49 -0700 (Mon, 23 May 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the Packet Error Rate test interface definitions.


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

#ifndef PER_TEST_H
#define PER_TEST_H

#ifdef PER_TEST

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef PER_TEST


#include "rom_jt_154.h"
#include <ti/sysbios/knl/Semaphore.h>
#include "zstackmsg.h"
#include "zcl_sampleapps_ui.h"
#include "cui.h"

/*********************************************************************
 * INCLUDES
 */



/*********************************************************************
 * CONSTANTS
 */
#if 0
/* These flags must be use as compile flags */
#define PER_TEST                             //Enable per test code code
#define PER_TEST_DISABLE_FINDING_BINDING     //Disable F&B to allow easy commissioning of the intended devices
#define PER_TEST_SEND_TO_PARENT              //Force Switch device to send commands only to parent
#define PER_TEST_SEND_USE_FINDING_BINDING    //Force Switch device to use Binds entry to send commands
#define PER_TEST_SET_FINDING_BINDING_1_MIN   //Change F&B timeout to 1 minute to allow easy commissioning
#define PER_TEST_ENABLE_FWD_NOTIFICATION     //Enable router feature to notify about frames being forwarded
#endif

/*********************************************************************
 * MACROS
 */


#ifndef SAMPLEAPP_PER_TEST_SEND_CMD_DELAY
    #define SAMPLEAPP_PER_TEST_SEND_CMD_DELAY     5000
#endif
#define SAMPLEAPP_PER_TEST_SEND_CMD_EVT           0x1000
#ifndef PER_TEST_APS_ACKS
    #define PER_TEST_APS_ACKS                     3
#endif
#ifndef PER_TEST_APS_ACKS_WAIT_TIME
    #define PER_TEST_APS_ACKS_WAIT_TIME           3000
#endif

/*********************************************************************
 * TYPEDEFS
 */




/*********************************************************************
 * VARIABLES
 */



/*********************************************************************
 * FUNCTIONS
 */


extern void PERTest_init(Semaphore_Handle appSem, uint8_t stEnt, CUI_clientHandle_t gCuiHandle);
extern void PERTest_process(void);
extern void PERTest_processZStackMsg(zstackmsg_genericReq_t *pMsg);
extern void PERTest_uiRefresh(uint8_t UiState, char * line[3]);

#endif


#ifdef __cplusplus
}

#endif

#endif /* PER_TEST */
#endif /* PER_TEST_H */
