/**************************************************************************************************
  Filename:       DebugTrace.h
  Revised:        $Date: 2008-10-07 14:47:15 -0700 (Tue, 07 Oct 2008) $
  Revision:       $Revision: 18212 $

  Description:    This interface provides quick one-function-call functions to
                  Monitor and Test reporting mechanisms.


  Copyright 2007 Texas Instruments Incorporated.

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

#ifndef DEBUGTRACE_H
#define DEBUGTRACE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */


/*********************************************************************
 * MACROS
 */

/*
 * Windows Print String
 */
// command id's
#define CMDID_RTG_ADD					1
#define CMDID_RTG_EXP					0x81
#define CMDID_RREQ_SEND				2
#define CMDID_RREQ_DROP				0x82
#define CMDID_RREP_SEND				3
#define CMDID_RREP_DROP				0x83
#define CMDID_RREQ_EXP				4

#define CMDID_DATA_SEND					6
#define CMDID_DATA_FORWARD			7
#define CMDID_DATA_RECEIVE			8

#define CMDID_BCAST_RCV				0x10
#define CMDID_BCAST_ACK				0x11
#define CMDID_BCAST_RETX			0x12

#define CMDID_BCAST_EXP				0x13
#define CMDID_BCAST_ERR				0x15

#define WPRINTSTR( s )

#if defined ( MT_TASK )
  /*
   * Trace Message
   *       - Same as debug_msg with SEVERITY_TRACE already filled in
   *       - Used to stand out in the source code.
   */
  #define TRACE_MSG( compID, nParams, p1, p2, p3 )  debug_msg( compID, SEVERITY_TRACE, nParams, p1, p2, p3 )


  /*
   * Debug Message (SEVERITY_INFORMATION)
   *      - Use this macro instead of calling debug_msg directly
   *      - So, it can be easily compiled out later
   */
  #define DEBUG_INFO( compID, subCompID, nParams, p1, p2, p3 )  debug_msg( compID, subCompID, nParams, p1, p2, p3 )


  /*** TEST MESSAGES ***/
  #define DBG_NWK_STARTUP           debug_msg( COMPID_TEST_NWK_STARTUP,         SEVERITY_INFORMATION, 0, 0, 0, 0 )
  #define DBG_SCAN_CONFIRM          debug_msg( COMPID_TEST_SCAN_CONFIRM,        SEVERITY_INFORMATION, 0, 0, 0, 0 )
  #define DBG_ASSOC_CONFIRM         debug_msg( COMPID_TEST_ASSOC_CONFIRM,       SEVERITY_INFORMATION, 0, 0, 0, 0 )
  #define DBG_REMOTE_DATA_CONFIRM   debug_msg( COMPID_TEST_REMOTE_DATA_CONFIRM, SEVERITY_INFORMATION, 0, 0, 0, 0 )

#else

  #define TRACE_MSG( compID, nParams, p1, p2, p3 )
  #define DEBUG_INFO( compID, subCompID, nParams, p1, p2, p3 )
  #define DBG_NWK_STARTUP
  #define DBG_SCAN_CONFIRM
  #define DBG_ASSOC_CONFIRM
  #define DBG_REMOTE_DATA_CONFIRM

#endif

/*********************************************************************
 * CONSTANTS
 */

#define SEVERITY_CRITICAL     0x01
#define SEVERITY_ERROR        0x02
#define SEVERITY_INFORMATION  0x03
#define SEVERITY_TRACE        0x04

#define NO_PARAM_DEBUG_LEN   5
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

  /*
   * Debug Message - Sent out serial port
   */


extern void debug_msg( uint8_t compID, uint8_t severity, uint8_t numParams,
                       uint16_t param1, uint16_t param2, uint16_t param3 );

extern void debug_str( uint8_t *str_ptr );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* DEBUGTRACE_H */
