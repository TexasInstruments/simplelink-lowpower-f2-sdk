/**************************************************************************************************
  Filename:       sampleapp.h
  Revised:        $Date: 2019-08-05 15:52:00 -0700 (Mon, 5 Aug 2019) $
  Revision:       $Revision: Unknown $

  Description:    Zigbee Application Builder - Sample Device Application.

  Copyright 2006-2019 Texas Instruments Incorporated. All rights reserved.

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
****************************************************************************************************/

#ifndef SAMPLEAPP_H
#define SAMPLEAPP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 *********************************************************************/
#include "zcl.h"
#include "nvintf.h"

#include "zcl_config.h"

/*********************************************************************
 * CONSTANTS
 *********************************************************************/
#define SAMPLEAPP_END_DEVICE_REJOIN_EVT        0x0001

#define SAMPLEAPP_END_DEVICE_REJOIN_DELAY	     10000

#if defined (BDB_TL_TARGET) || defined (BDB_TL_INITIATOR)
#define TL_BDB_FB_EVT                          0x0100
#define TL_BDB_FB_DELAY                        16000
#endif // defined ( BDB_TL_TARGET ) || defined (BDB_TL_INITIATOR)

/*********************************************************************
 * REFERENCED EXTERNALS
 *********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* SAMPLEAPP_H */
