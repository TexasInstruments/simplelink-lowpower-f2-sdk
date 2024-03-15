/**************************************************************************************************
  Filename:       DebugTrace.c
  Revised:        $Date: 2012-11-28 00:37:02 -0800 (Wed, 28 Nov 2012) $
  Revision:       $Revision: 32329 $


  Description:    This interface provides quick one-function-call functions to
                  Monitor and Test reporting mechanisms.


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

#if defined( MT_TASK ) || defined( APP_DEBUG )

/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "rom_jt_154.h"
#include "mt.h"
#include "mt_task.h"
#include "mt_debug.h"
#include "debug_trace.h"

#if defined ( APP_DEBUG )
  #include "DebugApp.h"
#endif

 /*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

 /*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      debug_msg
 *
 * @brief
 *
 *   This feature allows modules to display debug information as
 *   applications execute in real-time.  This feature will work similar
 *   to "printf()" but will output to the serial port for display in
 *   the Z-Test tool.
 *
 *   This feature will most likely be compiled out in the production code
 *   to save code space.
 *
 * @param   byte compID - Component ID
 * @param   byte severity - CRITICAL(0x01), ERROR(0x02), INFORMATION(0x03)
 *                          or TRACE(0x04)
 * @param   byte numParams - number of parameter fields (param1-3)
 * @param   uint16_t param1 - user defined data
 * @param   uint16_t param2 - user defined data
 * @param   uint16_t param3 - user defined data
 *
 * @return  void
 */
void debug_msg( byte compID, byte severity, byte numParams, uint16_t param1, uint16_t param2, uint16_t param3 )
{

  mtDebugMsg_t *mtDebugMsg;
  uint16_t timestamp;

  if ( debugThreshold == 0 || debugCompId != compID )
    return;

  // Fill in the timestamp
  timestamp = 0;

  // Get a message buffer to build the debug message
  mtDebugMsg = (mtDebugMsg_t *)OsalPort_msgAllocate( sizeof( mtDebugMsg_t ) );
  if ( mtDebugMsg )
  {
      mtDebugMsg->hdr.event = CMD_DEBUG_MSG;
      mtDebugMsg->compID = compID;
      mtDebugMsg->severity = severity;
      mtDebugMsg->numParams = numParams;

      mtDebugMsg->param1 = param1;
      mtDebugMsg->param2 = param2;
      mtDebugMsg->param3 = param3;
      mtDebugMsg->timestamp = timestamp;

      OsalPort_msgSend( MT_TaskID, (uint8_t *)mtDebugMsg );
  }

} /* debug_msg() */

/*********************************************************************
 * @fn      debug_str
 *
 * @brief
 *
 *   This feature allows modules to display a debug text string as
 *   applications execute in real-time. This feature will output to
 *   the serial port for display in the Z-Test tool.
 *
 *   This feature will most likely be compiled out in the production
 *   code in order to save code space.
 *
 * @param   byte *str_ptr - pointer to null-terminated string
 *
 * @return  void
 */
void debug_str( byte *str_ptr )
{
  mtDebugStr_t *msg;
  byte mln;
  byte strLen;

  // Text string length
  strLen = (byte)strlen( (void*)str_ptr );

  // Debug string message length
  mln = sizeof ( mtDebugStr_t ) + strLen;

  // Get a message buffer to build the debug message
  msg = (mtDebugStr_t *)OsalPort_msgAllocate( mln );
  if ( msg )
  {
    // Message type, length
    msg->hdr.event = CMD_DEBUG_STR;
    msg->strLen = strLen;

    // Append message, no terminator
    msg->pString = (uint8_t *)(msg+1);
    OsalPort_memcpy ( msg->pString, str_ptr, strLen );

    OsalPort_msgSend( MT_TaskID, (uint8_t *)msg );
  }
} // debug_str()

/*********************************************************************
*********************************************************************/
#endif  // MT_TASK
