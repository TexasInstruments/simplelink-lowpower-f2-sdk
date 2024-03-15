/**************************************************************************************************
  Filename:       mt_zdo.h
  Revised:        $Date: 2012-02-16 15:49:27 -0800 (Thu, 16 Feb 2012) $
  Revision:       $Revision: 29347 $


  Description:    MonitorTest functions for the ZDO layer.


  Copyright 2004-2012 Texas Instruments Incorporated.

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

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "zcomdef.h"
#include "mt.h"
#include "aps_mede.h"
#include "af.h"
#include "zd_profile.h"
#include "zd_object.h"
#include "zd_app.h"

/***************************************************************************************************
 * GLOBAL VARIABLES
 ***************************************************************************************************/
extern uint32_t _zdoCallbackSub;

/***************************************************************************************************
 * MACROS
 ***************************************************************************************************/
#define ZDOCB_CHECK(cbi) (_zdoCallbackSub & (cbi))

/***************************************************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************************************************/

/*
 *  MT ZDO initialization
 */
extern void MT_ZdoInit(void);

/*
 *   Process all the NWK commands that are issued by test tool
 */
extern uint8_t MT_ZdoCommandProcessing(uint8_t* pBuf);

/*
 *  Callback to handle state change OSAL message from ZDO.
 */
extern void MT_ZdoStateChangeCB(OsalPort_EventHdr *pMsg);

/*
 *   Process all the callbacks from ZDO
 */
extern void MT_ZdoDirectCB( afIncomingMSGPacket_t *pData,  zdoIncomingMsg_t *inMsg );

/*
 *   Proxy the ZDO_SendMsgCBs one message at a time.
 */
void MT_ZdoSendMsgCB(zdoIncomingMsg_t *pMsg);

/***************************************************************************************************
***************************************************************************************************/
