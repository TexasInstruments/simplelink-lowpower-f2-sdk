/**************************************************************************************************
  Filename:       gpd_sw_data.c
  Revised:        $Date: 2014-05-12 13:14:02 -0700 (Mon, 12 May 2014) $
  Revision:       $Revision: 38502 $


  Description:    Green Power Device Switch data for application


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "gpd.h"

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

gpdfReq_t gpdfReq =
{
    GPD_FRAME_CONTROL,                        //NWKFrameControl
    GPD_EXT_FRAME_CONTROL,                    //NWKExtFC
    {
      (uint8_t)GPD_APP_ID,                      //gpd_ID_t.AppID
      {
#if (GPD_APP_ID == GPD_APP_TYPE_SRC_ID)
        .SrcID = GPD_ID,                      //union GPDId.SrcID
#elif (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
        .GPDExtAddr = (uint8_t*)GPD_ID,         //union GPDId.GPDExtAddr
#endif
      },
    },
#if (GPD_APP_ID == GPD_APP_TYPE_IEEE_ID)
    0x0A,                                     //endPoint
#endif
#if (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT)
    0x00000002,                               //securityFrameCounter
#endif
    GP_COMMAND_TOGGLE,                        //gpdCmdID
    0,                                        //payloadSize, calculated during command send
    NULL                                      //*payload
};

const uint8_t gpdDeviceId = DEVICE_ID;
const uint8_t frameDuplicates = GPDF_FRAME_DUPLICATES;
const uint8_t gpdChannel = GPD_CHANNEL;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */
#if (GP_SECURITY_LEVEL!=GP_SECURITY_LVL_NO_SEC)
#warning: GPD Switch device is not meant to have any type of security. Enabling it will require several changes into the app and might not be compliant with GP Specification
#endif
/****************************************************************************
****************************************************************************/
