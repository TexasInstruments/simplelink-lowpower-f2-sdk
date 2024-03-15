/***************************************************************************************************
  Filename:       MTEL.h
  Revised:        $Date: 2013-05-17 11:05:33 -0700 (Fri, 17 May 2013) $
  Revision:       $Revision: 34353 $

  Description:

  Copyright 2007-2011 Texas Instruments Incorporated.

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

 ***************************************************************************************************/
#ifndef MT_DEBUG_H
#define MT_DEBUG_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/
#include "rom_jt_154.h"

/***************************************************************************************************
 * TYPEDEFs
 ***************************************************************************************************/

typedef struct
{
  OsalPort_EventHdr  hdr;
  uint8_t             compID;
  uint8_t             severity;
  uint8_t             numParams;
  uint16_t            param1;
  uint16_t            param2;
  uint16_t            param3;
  uint16_t            timestamp;
} mtDebugMsg_t;

typedef struct
{
  OsalPort_EventHdr  hdr;
  uint8_t             strLen;
  uint8_t             *pString;
} mtDebugStr_t;

typedef struct {
#ifdef FEATURE_PACKET_FILTER_STATS
  uint32_t nwkInvalidPackets;
  uint32_t rxCrcFailure;
  uint32_t rxCrcSuccess;
#endif
  uint8_t  fsmstat0;
  uint8_t  fsmstat1;
  uint8_t  macData_rxCount;
  uint8_t  macData_directCount;
  uint8_t  macMain_state;
  uint8_t  macRxActive;
  uint8_t  macTxActive;
} mtDebugMacDataDump_t;

/***************************************************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************************************************/

#if defined (MT_DEBUG_FUNC)
/*
 * Process MT_DEBUG commands
 */
extern uint8_t MT_DebugCommandProcessing(uint8_t *pBuf);
#endif /* MT_DEBUG_FUNC */

/*
 * Process MT_DEBUG messages
 */
extern void MT_ProcessDebugMsg(mtDebugMsg_t *pData);

/*
 * Process MT_DEBUG strings
 */
extern void MT_ProcessDebugStr(mtDebugStr_t *pData);



#ifdef __cplusplus
}
#endif

#endif /* MTEL_H */

/***************************************************************************************************
 ***************************************************************************************************/
