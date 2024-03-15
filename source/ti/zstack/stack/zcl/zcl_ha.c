/**************************************************************************************************
  Filename:       zcl_ha.c
  Revised:        $Date: 2013-10-21 12:15:10 -0700 (Mon, 21 Oct 2013) $
  Revision:       $Revision: 35738 $

  Description:    Describe the purpose and contents of the file.


  Copyright (c) 2019, Texas Instruments Incorporated
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  *  Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

  *  Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

  *  Neither the name of Texas Instruments Incorporated nor the names of
      its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "rom_jt_154.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_closures.h"
#include "zcl_hvac.h"
#include "zcl_ss.h"
#include "zcl_ms.h"
#include "zcl_lighting.h"
#include "zcl_ha.h"
#include "mac_util.h"

/* HAL */
#include "bdb.h"


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
 * STATUS STRINGS
 */
// NwkInfo displayed on line 1 once on the network
// 0123456789012345
// Zc: PPPP CH ADDR
// Zr: PPPP CH ADDR
// Zed:PPPP CH ADDR
const char NwkInfoZcStr[]  = "ZC: ";
const char NwkInfoZrStr[]  = "ZR: ";
const char NwkInfoZedStr[] = "Zed:";
const char * const NwkInfoStrs[] = { NwkInfoZcStr, NwkInfoZrStr, NwkInfoZedStr };

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      zclHA_isbit
 *
 * @brief   Returns TRUE if the index bit is set
 *
 * @param   pArray   - array with bits in it
 *          bitIndex - index into the array of bits
 *
 * @return  TRUE if index bit is set
 */
bool zclHA_isbit(uint8_t *pArray, uint8_t bitIndex)
{
  uint8_t bit;
  bit = ( 1 << ( bitIndex & 0x7 ) );
  if( pArray[bitIndex >> 3] & bit )
    return TRUE;
  return FALSE;
}

/*********************************************************************
 * @fn      zclHA_setbit
 *
 * @brief   Sets a bit in a bit array
 *
 * @param   pArray   - array with bits in it
 *          bitIndex - index into the array of bits
 *
 * @return  none
 */
void zclHA_setbit(uint8_t *pArray, uint8_t bitIndex)
{
  uint8_t bit;
  bit = ( 1 << ( bitIndex & 0x7 ) );
  pArray[bitIndex >> 3] |= bit;
}

/*********************************************************************
 * @fn      zclHA_clearbit
 *
 * @brief   clears a bit in a bit array
 *
 * @param   pArray   - array with bits in it
 *          bitIndex - index into the array of bits
 *
 * @return  none
 */
void zclHA_clearbit(uint8_t *pArray, uint8_t bitIndex)
{
  uint8_t bit;
  bit = ( 1 << ( bitIndex & 0x7 ) );
  pArray[bitIndex >> 3] &= (~(unsigned)bit);
}

/*********************************************************************
 * @fn      zclHA_uint16toa
 *
 * @brief   Converts from a uint16_t to ascii hex string.
 *          The # will be exactly 4 hex digits (e.g. 0x0000 or 0x1E3F).
 *          NULL terminates the string.
 *
 * @param   u - Number to be converted
 *          string - pointer to coverted string
 *
 * @return  none
 */
void zclHA_uint16toa(uint16_t u, char *string)
{
  // add preceding zeros
  if ( u < 0x1000 )
  {
    *string++ = '0';
  }

  if ( u < 0x0100 )
  {
    *string++ = '0';
  }

  if ( u < 0x0010 )
  {
    *string++ = '0';
  }

  Util_ltoa((unsigned long)u, (unsigned char *)string, 16);
}

/*********************************************************************
 * @fn      zclHA_uint8toa
 *
 * @brief   Converts from a uint8_t to ascii decimal string.
 *          The # will be exactly 3 digits (e.g. "123", " 64", "  0"), preceded by spaces.
 *          No null terminator.
 *
 * @param   b - Byte to be converted
 *          string - pointer to coverted string
 *
 * @return  none
 */
void zclHA_uint8toa(uint8_t b, char *string)
{
  // write the level (up to 3 digits, pre-padded with spaces)
  string[0] = string[1] = string[2] = '0';
  string[2] = '0' + (b % 10);  // 1s
  b = b / 10;
  if( b )
  {
    string[1] = '0' + (b % 10);  // 10s
    b = b / 10;
  }
  if( b )
  {
   string[0] = '0' + (b % 10);  // 100s
  }
}

/*********************************************************************
*********************************************************************/
