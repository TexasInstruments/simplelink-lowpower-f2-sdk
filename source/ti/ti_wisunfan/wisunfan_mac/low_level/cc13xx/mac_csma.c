/******************************************************************************

 @file  mac_csma.c

 @brief This file contains the MAC CSMA features.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2015-2023, Texas Instruments Incorporated

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

 ******************************************************************************
 
 
 *****************************************************************************/

/*******************************************************************************
 * INCLUDES
 */

#ifndef LIBRARY
#include "ti_154stack_features.h"
#endif

#include "mac.h"
#include "mac_low_level.h"
#include "mac_radio.h"

#ifdef FEATURE_PLATFORM_BLOCK

#include "hw_memmap.h"
#include "hw_fcfg1.h"
#include "flash.h"
#include "hal_defs.h"
#include "hal_mcu.h"
#include "mac_csma.h"


/*******************************************************************************
 * MACROS
 */


/*******************************************************************************
 * CONSTANTS
 */

#define FACTORY_CFG_USER_ID_PROTOCOL_RF4CE           0x00002000
#define FACTORY_CFG_USER_ID_PROTOCOL_TIMAC_ZIGBEE    0x00004000
#define FACTORY_CFG_USER_ID_PROTOCOL_BLE_PROPRIETORY 0x00009000


/*******************************************************************************
 * LOCAL VARIABLES
 */


/*******************************************************************************
 * GLOBAL VARIABLES
 */


/*******************************************************************************
 * LOCAL FUNCTIONS
 */
static uint8 msgData[28];


/*******************************************************************************
 * @fn          macValidateProtocolID
 *
 * @brief       This call is used to check if chip protocol ID is allowed 
 *              for TIMAC to run.
 *
 * input parameters
 *
 * @param       None
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
static void macValidateProtocolID( void )
{
  uint32 ui32ChipId = HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_USER_ID);
  
  if ( (ui32ChipId & FACTORY_CFG_USER_ID_PROTOCOL_TIMAC_ZIGBEE) == 0 )
  {
    uint32 ui32DeviceId = HWREG(FLASH_CFG_BASE + FCFG1_OFFSET + FCFG1_O_ICEPICK_DEVICE_ID);

    /* Platform lock, for any CSMA data transmission, replace it with 
     * Beacon Frame from 0x11CC to 0xAABB with Beacon payload of 
     * "WrongChip"+DEVICE_ID+USER_ID
     */
    msgData[0]  = 0x00;
    msgData[1]  = 0x80;
    msgData[2]  = 0x01;
    msgData[3]  = 0xCC;
    msgData[4]  = 0x11;
    msgData[5]  = 0xBB;
    msgData[6]  = 0xAA;
    msgData[7]  = 0xFF;
    msgData[8]  = 0xCF;
    msgData[9]  = 0x00;
    msgData[10] = 0x00;
    msgData[11] = 'W';
    msgData[12] = 'r';
    msgData[13] = 'o';
    msgData[14] = 'n';
    msgData[15] = 'g';
    msgData[16] = 'C';
    msgData[17] = 'h';
    msgData[18] = 'i';
    msgData[19] = 'p';
    msgData[20] = BREAK_UINT32( ui32DeviceId, 3 );
    msgData[21] = BREAK_UINT32( ui32DeviceId, 2 );
    msgData[22] = BREAK_UINT32( ui32DeviceId, 1 );
    msgData[23] = BREAK_UINT32( ui32DeviceId, 0 );
    msgData[24] = BREAK_UINT32( ui32ChipId, 3 );
    msgData[25] = BREAK_UINT32( ui32ChipId, 2 );
    msgData[26] = BREAK_UINT32( ui32ChipId, 1 );
    msgData[27] = BREAK_UINT32( ui32ChipId, 0 );
    
    /* Override payload */
    macTxCmd.payloadLen = sizeof(msgData);;
    macTxCmd.pPayload = msgData;
  }
}


/**************************************************************************************************
 * @fn          macRadioSendCsmaCaCmd
 *
 * @brief       Send CMD_IEEE_CSMA radio command to CM0. If the chip protocol ID
 *              check failed, this will still return CMDSTA_DONE status. The
 *              macValidateProtocolID() will override TX payload and reset after
 *              certain TX count.
 *
 * @param       none
 *
 * @return      A CMDSTA value, or CMDSTA_MAILBOX_BUSY. 
 **************************************************************************************************
 */
uint8 macRadioSendCsmaCaCmd(void)
{
  /* Validate chip protocol ID */
  macValidateProtocolID();

  /* TODO: To be implemented */
  return (0);
}

#else /* FEATURE_PLATFORM_BLOCK */

uint8 macRadioSendCsmaCaCmd(void)
{
  /* TODO: To be implemented */
  return (0);
}

#endif /* FEATURE_PLATFORM_BLOCK */

/*******************************************************************************
 */


