/**************************************************************************************************
  Filename:       mt_af.h
  Revised:        $Date: 2012-11-01 15:23:04 -0700 (Thu, 01 Nov 2012) $
  Revision:       $Revision: 32011 $

  Description:    MonitorTest functions for AF.

  Copyright 2007-2012 Texas Instruments Incorporated.

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

#ifndef MT_AF_H
#define MT_AF_H

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "zcomdef.h"
#include "mt.h"
#include "af.h"

//#define AFCB_CHECK(cbi,task) ((_afCallbackSub & cbi) && (task == MT_TaskID))
#define AFCB_CHECK(cbi,task) ((1) && (task == MT_TaskID))

/***************************************************************************************************
 * CONSTANTS
 ***************************************************************************************************/
#if defined ( MT_AF_CB_FUNC )
#define CB_ID_AF_DATA_IND               0x0001
#define CB_ID_AF_DATA_CNF               0x0002
#define CB_ID_AF_REFLECT_ERROR          0x0004

#define SPI_AF_CB_TYPE                  0x0900
#endif

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
typedef enum {
  InterPanClr,
  InterPanSet,
  InterPanReg,
  InterPanChk
} InterPanCtl_t;
#endif
/***************************************************************************************************
 * GLOBAL VARIABLES
 ***************************************************************************************************/
extern uint16_t _afCallbackSub;

/*
 * AF housekeeping executive.
 */
extern void MT_AfExec(void);

/*
 * Process AF commands
 */
extern uint8_t MT_AfCommandProcessing(uint8_t *pBuf);

/*
 * Process the callback subscription for AF Incoming data.
 */
extern void MT_AfIncomingMsg(afIncomingMSGPacket_t *pMsg);

/*
 * Process the callback subscription for Data confirm
 */
extern void MT_AfDataConfirm(afDataConfirm_t *pMsg);

/*
 * Process the callback subscription for Reflect Error
 */
extern void MT_AfReflectError(afReflectError_t *pMsg);

/*********************************************************************
*********************************************************************/
#endif
