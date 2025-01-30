/*****************************************************************************

 @file  health_thermometer_server.h

 @brief This file contains the Health Thermometer service definitions and prototypes.

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
#ifndef HTSERVER_H
#define HTSERVER_H

#ifdef __cplusplus
extern "C"
{
#endif
/*********************************************************************
 * INCLUDES
 */

#include <ti/bleapp/ble_app_util/inc/bleapputil_api.h>

/*********************************************************************
 * DEFINES
 */
//service UUID
#define HTS_SERV_UUID             0x1809

/************************************
 ***** Characteristic defines *******
 ************************************/
// HTS Measurement
#define HTS_MEAS_ID               0
#define HTS_MEAS_UUID             0x2A1C

/************************************
 ********** HTS Measurement *********
 ************************************/
#define HTS_MEAS_MIN_LEN          5                       // Minimum length of Temperature Measurement
#define HTS_MEAS_MAX_LEN          sizeof(HTS_TempMeas_t)  // Maximum length of Temperature Measurement

/************************************
 ******* HTS Time Stamp  ************
 ************************************/
#define HTS_BT_YEAR_MIN            1582    // Minimum value
#define HTS_BT_YEAR_MAX            9999    // Maximum value
#define HTS_BT_YEAR_UNKNOWN        0       // Year is not known

#define HTS_BT_MONTH_MIN           1       // Minimum value
#define HTS_BT_MONTH_MAX           12      // Maximum value
#define HTS_BT_MONTH_UNKNOWN       0       // Month is not known

#define HTS_BT_DAY_MIN             1       // Minimum value
#define HTS_BT_DAY_MAX             31      // Maximum value
#define HTS_BT_DAY_UNKNOWN         0       // Day is not known

#define HTS_BT_HOUR_MAX            23      // Maximum value

#define HTS_BT_MINUTE_MAX          59      // Maximum value

#define HTS_BT_SECOND_MAX          59      // Maximum value

// Error codes
#define HTS_RACP_PROC_IN_PROG      0x80    // The Client Characteristic Configuration descriptor is not configured according to the requirements of the service.

/*********************************************************************
 * TYPEDEFS
 */
// HTS Measurement Flags field
typedef enum
{
  HTS_MEAS_FLAGS_UNITS_CELSIUS       = (uint8)BV(0),    // Temperature Measurement value in units of Celsius
  HTS_MEAS_FLAGS_UNITS_FAHRENHEIT    = (uint8)~BV(0),   // Temperature Measurement value in units of Fahrenheit
  HTS_MEAS_FLAGS_TIME_STAMP          = (uint8)BV(1),    // Time Stamp
  HTS_MEAS_FLAGS_TYPE                = (uint8)BV(2),    // Temperature type (location of the measurement)
} HTS_measFlags_e;

//Temperature Measurment struct
typedef struct
{
  uint8   flags;
  uint32  temp;
  struct
  {
    uint16 year;
    uint8 month;
    uint8 day;
    uint8 hours;
    uint8 minutes;
    uint8 seconds;
  } timeStamp;
  uint8 tempType;
} HTS_TempMeas_t;

/************************************
 *********** General ****************
 ************************************/
// Data structure used to store CCC update
typedef struct
{
  uint16 connHandle;
  uint8 value;
} HTS_cccUpdate_t;

/*********************************************************************
 * Profile Callbacks
 */
// Callback to indicate client characteristic configuration has been updated on Measurement characteristic
typedef void ( *HTS_measOnCccUpdate_t )( char *cccUpdate );

//Server CallBack struct
typedef struct
{
  HTS_measOnCccUpdate_t   pfnMeasOnCccUpdateCB;   // Called when client characteristic configuration has been updated on Measurement characteristic
} HTS_cb_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * API FUNCTIONS
 */

/*********************************************************************

 * @fn      HTS_registerProfileCBs
 *
 * @brief   Registers the profile callback function. Only call
 *          this function once.
 *
 * @param   profileCallback - pointer to profile callback.
 *
 * @return  SUCCESS or INVALID PARAMETER
 */
bStatus_t HTS_registerProfileCBs( HTS_cb_t *profileCallback );

/*
 * @fn      HTS_setParameter
 *
 * @brief   Set a HTS Service parameter.
 *
 * @param   param - Characteristic UUID
 * @param   pValue - pointer to data to write.  This is dependent on
 *          the parameter ID WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 * @param   len - length of data to write
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t HTS_setParameter( uint8 param, void *pValue, uint16 len );

/* @fn      HTS_addService
 *
 * @brief   This function initializes the Health Thermometer Server service
 *          by registering GATT attributes with the GATT server.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t HTS_addService( void );
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* HTSERVER_H */
