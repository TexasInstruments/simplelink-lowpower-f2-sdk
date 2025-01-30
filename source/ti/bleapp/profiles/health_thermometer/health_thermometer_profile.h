/******************************************************************************

 @file  health_thermometer_profile.h

 @brief This file contains the health thermometer profile definitions and prototypes.

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
#ifndef HTPROFILE_H
#define HTPROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <ti/bleapp/ble_app_util/inc/bleapputil_api.h>
#include <health_thermometer_server.h>

/*********************************************************************
 * DEFINES
 */

/************************************
 ********** HTP Measurement *********
 ************************************/
#define HTP_MEAS_MIN_TEMP         -273.15         // Minimum Temperature Measurement value
#define HTP_MEAS_MAX_TEMP         327.67          // Maximum Temperature Measurement value
#define HTP_MEAS_MIN_TYPE         0               // Minimum Temperature Type value
#define HTP_MEAS_MAX_TYPE         255             // Maximum Temperature Type value
#define HTP_MEAS_MIN_TIME_STAMP   0
#define HTP_TIME_STAMP_LEN        0x7

/*********************************************************************
 * TYPEDEFS
 */
//Temperature Measurement struct
typedef struct
{
  uint16 year;
  uint8 month;
  uint8 day;
  uint8 hours;
  uint8 minutes;
  uint8 seconds;
} HTP_timeStamp;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * Profile Callback
 */
//Callback to indicate client characteristic configuration has been updated for Measurement characteristic
typedef void ( *HTP_measOnCccUpdate_t )( uint16 connHandle, uint16 pValue );

typedef struct
{
  HTP_measOnCccUpdate_t   pfnMeasOnCccUpdateCB;   // Called when client characteristic configuration has been updated on Measurement characteristic
} HTP_cb_t;

/*********************************************************************
 * API FUNCTIONS
 */


/*********************************************************************
 * @fn      HTP_start
 *
 * @brief   This function adds the Health Thermometer Server service,
 *          registers the service callback function and initializes
 *          the service characteristics
 *
 * @param   appCallbacks - pointer to application callback
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t HTP_start( HTP_cb_t *appCallbacks, HTP_timeStamp* htp_timeStamp);

/*
 * @fn      HTP_sendMeaserment
 *
 * @brief   Send Health Thermometer measurement to any registered device
 *
 * @param   tempMeasur - a Temperature measurement in Celsius units
 * @param   timeStamp - Minutes passed from the TimeStamp
 * @param   type - sample type, mandatory if glucose concentration is included.
 * @param   location - location of sample, mandatory if glucose concentration is included.
 * @param   sensorStatus - device status, not mandatory.
 * @return  SUCCESS or stack call status
 */
bStatus_t HTP_sendMeasurement( uint32 tempMeasur,uint16  timeStamp, uint8 type );

#ifdef __cplusplus
}
#endif

#endif /* HTPROFILE_H */
