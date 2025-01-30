/******************************************************************************

 @file  health_thermometer_profile.c

 @brief This file contains the Health Thermometer profile sample for use with the Basic BLE
         Profiles sample application.

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

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <icall.h>
/* This Header file contains all BLE API and icall structure definition */
#include "icall_ble_api.h"

#include "util.h"

#include <health_thermometer_profile.h>
#include <health_thermometer_server.h>

/*********************************************************************
 * DEFINES
 */

/*********************************************************************
* MACROS
*/
#define HTP_SENSOR_STATUS_IS_SUPPORTED(x)   x & ~ BV(9)

/*********************************************************************/

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static HTP_cb_t *htp_appCB = NULL;
HTP_timeStamp htp_timeStamp = {0};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void HTP_measOnCccUpdateCB( char *pValue );
static void HTP_SendMeasur( HTS_TempMeas_t *measur );
static bStatus_t HTP_validateTimeStamp(HTP_timeStamp timeStamp);

/*********************************************************************
 * SERVER CALLBACKS
 */
// HT profile callback functions
static HTS_cb_t htp_serverCB =
{
  HTP_measOnCccUpdateCB,
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      HTP_start
 *
 * @brief   This function adds the HT Server service,
 *          registers the service callback function and initializes
 *          the service characteristics
 *
 * @param   appCallbacks - pointer to application callback
 * @param   htp_timeStamp - HT initial time offset parameter
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t HTP_start( HTP_cb_t *appCallbacks, HTP_timeStamp* ht_timeStamp)
{
  uint8 status = SUCCESS;

  // Registers the application callback function
  if ( appCallbacks )
  {
    htp_appCB = appCallbacks;
  }
  else
  {
    return ( INVALIDPARAMETER );
  }

  status = HTP_validateTimeStamp(*ht_timeStamp);
  if ( status != SUCCESS )
  {
      return status;
  }
  // Add time stamp
  memcpy(&htp_timeStamp, ht_timeStamp, HTP_TIME_STAMP_LEN);

  // Add Health Thermometer service
  status = HTS_addService();
  if ( status != SUCCESS )
  {
    // Return status value
    return ( status );
  }

  // Register to service callback function
  status = HTS_registerProfileCBs( &htp_serverCB );
  if ( status != SUCCESS )
  {
    // Return status value
    return ( status );
  }

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      HTP_measOnCccUpdateCB
 *
 * @brief   Callback from HT Server indicating CCC has been update
 *          for Measurement characteristic
 *
 * @param   pValue - pointer to data
 *
 * @return  None
 */
static void HTP_measOnCccUpdateCB( char *pValue )
{
  HTS_cccUpdate_t *cccUpdate = (HTS_cccUpdate_t *)pValue;

  // Verify input parameters
  if ( pValue == NULL )
  {
    return;
  }

  // If the callback function is not NULL, notify the application
  if ( htp_appCB && htp_appCB->pfnMeasOnCccUpdateCB )
  {
    htp_appCB->pfnMeasOnCccUpdateCB( cccUpdate->connHandle, cccUpdate->value );
  }
}

/*********************************************************************
 * @fn      HTP_sendMeasurement
 *
 * @brief   Create HTP measurement and send it
 *
 * @param   tempMeasur - a Temperature Measurement value
 * @param   timeStamp - Minutes since the Session Start Time
 * @param   type - type of measurement - represent to the location of a temperature measurement

 * @return  SUCCESS or stack call status
 */
bStatus_t HTP_sendMeasurement( uint32 tempMeasur, uint16 timeStamp, uint8 type )
{
  uint8 status = SUCCESS;
  HTS_TempMeas_t measur = {0};
  // Update the temperature of the measurement
  measur.temp = tempMeasur;

  //  Verify the Temperature Measurement value
  if ( tempMeasur < HTP_MEAS_MIN_TEMP || tempMeasur > HTP_MEAS_MAX_TEMP)
  {
    status = INVALIDPARAMETER;
    return status;
  }

  // Verify that type is included
  if ( type > HTP_MEAS_MIN_TYPE && type < HTP_MEAS_MAX_TYPE ){
  measur.flags |= HTS_MEAS_FLAGS_TYPE;
  // Update the type (location) of the measurement
  measur.tempType = type;
  }

  // Verify that timeStamp is included
  if ( timeStamp > HTP_MEAS_MIN_TIME_STAMP )
  {
    // Add timeStamp
    memcpy(&measur.timeStamp, &htp_timeStamp, HTP_TIME_STAMP_LEN);
    measur.flags |= HTS_MEAS_FLAGS_TIME_STAMP;
  }

  //  Send measurement to registered devices.
  HTP_SendMeasur(&measur);
  //  Return status value
  return ( status );
}

/*********************************************************************
 * @fn      HTP_SendMeas
 *
 * @brief   Send HT measurement using GATT notifications
 *
 * @param   measur - measurement to send
 *
 * @return  none
 */
static void HTP_SendMeasur( HTS_TempMeas_t *measur )
{
  uint8 pBuf[HTS_MEAS_MAX_LEN] = {0};
  uint8 len = HTS_MEAS_MIN_LEN;

  // Allocate buffer with the actual size of the HT measurement record
  VOID memcpy( pBuf, &measur->flags, sizeof( measur->flags ));
  VOID memcpy( pBuf+1, &measur->temp, sizeof( measur->temp ));

  // Send time stamp only if the timeStamp flag is set to 1.
  if ( measur->flags & HTS_MEAS_FLAGS_TIME_STAMP )
  {
    VOID memcpy( pBuf + len, &measur->timeStamp, sizeof( measur->timeStamp ));
    len = len + HTP_TIME_STAMP_LEN;
  }

  // Send temperature type (location) only if temperature type flag is set to 1.
  if ( measur->flags & HTS_MEAS_FLAGS_TYPE )
  {
    VOID memcpy( pBuf + len, &measur->tempType, sizeof( measur->tempType ));
    len += sizeof( measur->tempType );
  }

  HTS_setParameter( HTS_MEAS_ID, pBuf, len);
}

/*********************************************************************
 * @fn      HTP_validateTimeStamp
 *
 * @brief   Validate timeStamp values, return invalid parameter in case of a validation
 *
 * @param   timeStamp - timeStamp given by the App.
 *
 * @return  none
 */
bStatus_t HTP_validateTimeStamp(HTP_timeStamp timeStamp)
{
  uint8 status = SUCCESS;

  if (timeStamp.day < HTS_BT_DAY_MIN || timeStamp.day > HTS_BT_DAY_MAX || timeStamp.hours > HTS_BT_HOUR_MAX ||
          timeStamp.minutes > HTS_BT_MINUTE_MAX || timeStamp.month < HTS_BT_MONTH_MIN || timeStamp.month > HTS_BT_MONTH_MAX ||
          timeStamp.seconds > HTS_BT_SECOND_MAX || timeStamp.year > HTS_BT_YEAR_MAX ||
          ((timeStamp.year < HTS_BT_YEAR_MIN) & (HTS_BT_YEAR_UNKNOWN != HTS_BT_YEAR_UNKNOWN)))
  {
    return ( INVALIDPARAMETER );
  }
  return status;
}
