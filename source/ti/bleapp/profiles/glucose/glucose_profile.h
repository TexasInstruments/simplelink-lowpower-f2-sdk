/******************************************************************************

 @file  glucose_profile.h

 @brief This file contains the glucose profile definitions and prototypes.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2010-2025, Texas Instruments Incorporated
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

 ******************************************************************************
 
 
 *****************************************************************************/
#ifndef GLUCOSEPROFILE_H
#define GLUCOSEPROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <ti/bleapp/ble_app_util/inc/bleapputil_api.h>
#include <ti/bleapp/services/glucose/glucose_server.h>

/*********************************************************************
 * DEFINES
 */

/************************************
 ********** GLP Measurement *********
 ************************************/

#define GLP_TYPE_MIN_VALUE           0x1
#define GLP_TYPE_MAX_VALUE           0xA
#define GLP_LOCATION_MIN_VALUE       0x1
#define GLP_LOCATION_MIN_VALUE       0x1
#define GLP_LOCATION_MAX_VALUE       0x4
#define GLP_LOCATION_NOT_AVAILABLE   0xF
#define GLP_TIME_BASE_LEN            0x7

/*********************************************************************
 * TYPEDEFS
 */

//Glucose Measurment struct
typedef struct
{
  uint16 year;
  uint8 month;
  uint8 day;
  uint8 hours;
  uint8 minutes;
  uint8 seconds;
} GLP_timeBase;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * Profile Callback
 */

//Callback to indicate client characteristic configuration has been updated for Measurement characteristic
typedef void ( *GLP_measOnCccUpdate_t )( uint16 connHandle, uint16 pValue );

//Callback to indicate client characteristic configuration has been updated for RACP characteristic
typedef void ( *GLP_racpOnCccUpdate_t )( uint16 connHandle, uint16 pValue );

typedef struct
{
  GLP_measOnCccUpdate_t   pfnMeasOnCccUpdateCB;   // Called when client characteristic configuration has been updated on Measurement characteristic
  GLP_racpOnCccUpdate_t   pfnRACPOnCccUpdateCB;   // Called when client characteristic configuration has been updated on RACP characteristic
} GLP_cb_t;

/*********************************************************************
 * API FUNCTIONS
 */


/*********************************************************************
 * @fn      GLP_start
 *
 * @brief   This function adds the Glucose Server service,
 *          registers the service callback function and initializes
 *          the service characteristics
 *
 * @param   appCallbacks - pointer to application callback
 * @param   feat - CGM Feature parameters
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t GLP_start( GLP_cb_t *appCallbacks, uint16 feat, GLP_timeBase* glp_timebase);

/*
 * @fn      GLP_addMeaserment
 *
 * @brief   Add glucose measurement to data base and send it
 *          to any registered device
 *
 * @param   glucoseConcen - a Glucose concentration in mg/dL units
 * @param   timeOffset - Minutes passed from the TimeBase
 * @param   type - sample type, mandatory if glucose concentration is included.
 * @param   location - location of sample, mandatory if glucose concentration is included.
 * @param   sensorStatus - device status, not mandatory.
 * @return  SUCCESS or stack call status
 */
bStatus_t GLP_addMeaserment( uint16  glucoseConcen, uint16  timeOffset, uint8 type, uint8 location, uint16 sensorStatus);


#endif
