/******************************************************************************

 @file  cgm_server.h

 @brief This file contains the CGM service definitions and prototypes.

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

#ifndef CGMSERVER_H
#define CGMSERVER_H

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

// Service UUID
#define CGMS_SERV_UUID 0x181F

/************************************
 ***** Characteristic defines *******
 ************************************/

// CGM Measurement
#define CGMS_MEAS_ID   0
#define CGMS_MEAS_UUID 0x2AA7

// CGM Feature
#define CGMS_FEAT_ID   1
#define CGMS_FEAT_UUID 0x2AA8

// CGM Status
#define CGMS_STAT_ID   2
#define CGMS_STAT_UUID 0x2AA9

// CGM Session Start Time
#define CGMS_SST_ID   3
#define CGMS_SST_UUID 0x2AAA

// CGM Session Run Time
#define CGMS_SRT_ID   4
#define CGMS_SRT_UUID 0x2AAB

// Record Access Control Point
#define CGMS_RACP_ID   5
#define CGMS_RACP_UUID 0x2A52

// CGM Specific Ops Control Point
#define CGMS_CGMCP_ID   6
#define CGMS_CGMCP_UUID 0x2AAC

/************************************
 ********** CGM Measurement *********
 ************************************/

#define CGMS_MEAS_MIN_RECORD_LEN    6   // Minimum length of CGM Measurement record
#define CGMS_MEAS_MAX_RECORD_LEN    sizeof(CGMS_measRecord_t)  // Maximum length of CGM Measurement record

// Measurement Interval
#define CGMS_MEAS_DEFAUALT_INTERVAL       1                                      // CGM Measurement interval in minutes
#define CGMS_MEAS_DEFAUALT_INTERVAL_MSEC  (CGMS_MEAS_DEFAUALT_INTERVAL * 60000)  // CGM Measurement interval in milliseconds

/************************************
 ************ CGM Feature ***********
 ************************************/

#define CGMS_FEAT_LEN  7  // CGM Feature characteristic length

/** CGM Type Field **/

#define CGMS_FEAT_TYPE_CAP_BLOOD     0x1  // Capillary Whole blood
#define CGMS_FEAT_TYPE_CAP_PLASMA    0x2  // Capillary Plasma
#define CGMS_FEAT_TYPE_VEN_BLOOD     0x3  // Venous Whole blood
#define CGMS_FEAT_TYPE_VEN_PLASMA    0x4  // Venous Plasma
#define CGMS_FEAT_TYPE_ART_BLOOD     0x5  // Arterial Whole blood
#define CGMS_FEAT_TYPE_ART_PLASMA    0x6  // Arterial Plasma
#define CGMS_FEAT_TYPE_UNDET_BLOOD   0x7  // Undetermined Whole blood
#define CGMS_FEAT_TYPE_UNDET_PLASMA  0x8  // Undetermined Plasma
#define CGMS_FEAT_TYPE_ISF           0x9  // Interstitial Fluid (ISF)
#define CGMS_FEAT_TYPE_CONTROL       0xA  // Control Solution

/** CGM Sample Location Field **/

#define CGMS_FEAT_LOC_FINGER      0x1  // Finger
#define CGMS_FEAT_LOC_AST         0x2  // Alternate Site Test (AST)
#define CGMS_FEAT_LOC_EARLOBE     0x3  // Earlobe
#define CGMS_FEAT_LOC_CONTROL     0x4  // Control solution
#define CGMS_FEAT_LOC_SUB_TISSUE  0x5  // Subcutaneous tissue
#define CGMS_FEAT_LOC_NOT_AVAIL   0xF  // Sample Location value not available

/** CGM E2E-CRC **/

#define CGMS_FEAT_E2E_NOT_SUPPORTED   0xFFFF   // If the device doesn’t support E2E-safety

/************************************
 ************ CGM Status ************
 ************************************/

#define CGMS_STAT_LEN  5  // CGM Status characteristic length

/************************************
 **** CGM Session Start Time  *******
 ************************************/

#define CGMS_SST_LEN   9     // CGM Feature characteristic length

#define CGMS_SST_DST_OFFSET_STANDARD_TIME         0    // Standard Time
#define CGMS_SST_DST_OFFSET_HALF_DAYLIGHT_TIME    2    // Half an hour Daylight Time (+ 0.5h)
#define CGMS_SST_DST_OFFSET_DAYLIGHT_TIME         4    // Daylight Time (+ 1h)
#define CGMS_SST_DST_OFFSET_DOUBLE_DAYLIGHT_TIME  8    // Double Daylight Time (+ 2h)
#define CGMS_SST_DST_OFFSET_UNKNOWN               255  // DST offset unknown

#define CGMS_SST_TIMEZONE_MIN        -48     // Minimum value
#define CGMS_SST_TIMEZONE_MAX        56      // Maximum value
#define CGMS_SST_TIMEZONE_UNKNOWN    -128    // Time zone offset is not known

#define CGMS_SST_YEAR_MIN            1582    // Minimum value
#define CGMS_SST_YEAR_MAX            9999    // Maximum value
#define CGMS_SST_YEAR_UNKNOWN        0       // Year is not known

#define CGMS_SST_MONTH_MIN           1       // Minimum value
#define CGMS_SST_MONTH_MAX           12      // Maximum value
#define CGMS_SST_MONTH_UNKNOWN       0       // Month is not known

#define CGMS_SST_DAY_MIN             1       // Minimum value
#define CGMS_SST_DAY_MAX             31      // Maximum value
#define CGMS_SST_DAY_UNKNOWN         0       // Day is not known

#define CGMS_SST_HOUR_MAX            23      // Maximum value

#define CGMS_SST_MINUTE_MAX          59      // Maximum value

#define CGMS_SST_SECOND_MAX          59      // Maximum value

#define CGMS_SST_NOT_VALID           0xFF    // SST value is not valid

/************************************
 **** CGM Session Run Time  *********
 ************************************/

#define CGMS_SRT_INTERVAL          1                               // CGM SRT interval in hours
#define CGMS_SRT_INTERVAL_MSEC     (CGMS_SRT_INTERVAL * 3600000)   // CGM Measurement interval in milliseconds

#define CGMS_SRT_LEN       2        // CGM SRT characteristic length

/************************************
 *** Record Access Control Point ****
 ************************************/

#define CGMS_RACP_REQ_MIN_LEN          2   // Minimum length of RACP Request
#define CGMS_RACP_REQ_MAX_LEN          7   // Maximum length of RACP Request
#define CGMS_RACP_RSP_LEN              4   // RACP Response length
#define CGMS_RACP_OPERAND_TIME_OFFSET  1   // For the CGM service, only Time Offset operand(0x01) is allowed to be used, it is handled like the sequence number.
#define CGMS_RACP_OPERAND_NULL         0   // Null operand

// Error codes
#define CGMS_RACP_CCCD_IMPROPERLY_CONFG     0xFD // a Client attempts to perform an RACP procedure with a CCC descriptor that is improperly configured
#define CGMS_RACP_PROC_IN_PROG              0xFE // a Client attempts to perform an RACP procedure with a CCC descriptor that is improperly configured

/************************************
 ** CGM Specific Ops Control Point **
 ************************************/

#define CGMS_CGMCP_REQ_MIN_LEN          1   // Minimum length of CGMCP Request
#define CGMS_CGMCP_REQ_MAX_LEN          2   // Maximum length of CGMCP Request
#define CGMS_CGMCP_RSP_GENERAL_LEN      3   // CGMCP general response length
#define CGMS_CGMCP_RSP_COMM_INT_LEN     2   // CGMCP communication interval response length

#define CGMS_CGMCP_COMM_INT_VAL_NON        0       // CGMCP disable communication interval
#define CGMS_CGMCP_COMM_INT_VAL_FASTEST    1       // CGMCP fastest communication interval

/*********************************************************************
 * TYPEDEFS
 */

/************************************
 ********** CGM Measurement *********
 ************************************/

// CGM Measurement Flags field
typedef enum
{
  CGMS_MEAS_FLAGS_TREND_INFO_PRESENT = (uint8)BV(0),  // CGM Trend Information present
  CGMS_MEAS_FLAGS_QUALITY_PRESENT    = (uint8)BV(1),  // CGM Quality present
  CGMS_MEAS_FLAGS_WARNING_PRESENT    = (uint8)BV(5),  // Device Specific Alert supported
  CGMS_MEAS_FLAGS_CALIB_TEMP_PRESENT = (uint8)BV(6),  // Sensor Malfunction Detection supported
  CGMS_MEAS_FLAGS_STATUS_PRESENT     = (uint8)BV(7)   // Sensor Temperature High-Low Detection supported
} CGMS_measFlags_e;

// CGM Measurement status field in Sensor Status Annunciation
typedef enum
{
  CGMS_STAT_SESSION_STOPPED          = (uint8)BV(0),  // Session stopped
  CGMS_STAT_DEV_BATTERY_LOW          = (uint8)BV(1),  // Device battery low
  CGMS_STAT_SENSOR_TYPE_INCOERRECT   = (uint8)BV(2),  // Sensor type incorrect for device
  CGMS_STAT_SENSOR_MALFUNCTION       = (uint8)BV(3),  // Sensor malfunction
  CGMS_STAT_DEVICE_SPECIFIC_ALERT    = (uint8)BV(4),  // Device Specific Alert
  CGMS_STAT_GENERAL_DEVICE_FAULT     = (uint8)BV(5)   // General device fault has occurred in the sensor
} CGMS_measStatus_e;

// CGM Measurement Cal/Temp field in Sensor Status Annunciation
typedef enum
{
  CGMS_CALIB_TIME_SYNC_REQUIRED      = (uint8)BV(0),  // Time synchronization between sensor and collector required
  CGMS_CALIB_NOT_ALLOWED             = (uint8)BV(1),  // Calibration not allowed
  CGMS_CALIB_RECOMMENDED             = (uint8)BV(2),  // Calibration recommended
  CGMS_CALIB_REQUIRED                = (uint8)BV(3),  // Calibration required
  CGMS_TEMP_TOO_HIGH                 = (uint8)BV(4),  // Sensor temperature too high for valid test/result at the time of measurement
  CGMS_TEMP_TOO_LOW                  = (uint8)BV(5)   // Sensor temperature too low for valid test/result at the time of measurement
} CGMS_measCalibTemp_e;

// CGM Measurement Record
typedef struct
{
  uint8   size;                  // Represents the size of the CGM Measurement record
  uint8   flags;                 // Indicates the presence of optional fields and the Sensor Status Annunciation Field in the CGM Measurement record
  uint16  glucoseConcen;  // The CGM Glucose concentration unit is in mg/dL
  uint16  timeOffset;            // Minutes since the Session Start Time for the stored glucose concentration value
  struct
  {
    uint8  status;
    uint8  calibTemp;
    uint8  warning;
  } status;                       // CGM Sensor Status Annunciation
} CGMS_measRecord_t;

/************************************
 ********** CGM Feature  ************
 ************************************/

// CGM Feature Field Mask
typedef enum CGMS_featSupported_e
{
  CGMS_FEAT_CALIBRATION_SUPPORTED                           = (uint32)BV(0),  // Calibration supported
  CGMS_FEAT_PATIENT_HIGH_LOW_ALERTS_SUPPORTED               = (uint32)BV(1),  // Patient High/Low Alerts supported
  CGMS_FEAT_HYPO_ALERTS_SUPPORTED                           = (uint32)BV(2),  // Hypo Alerts supported
  CGMS_FEAT_HYPER_ALERTS_SUPPORTED                          = (uint32)BV(3),  // Hyper Alerts supported
  CGMS_FEAT_RATE_INCREASE_DECREASE_ALERTS_SUPPORTED         = (uint32)BV(4),  // Rate of Increase/Decrease Alerts supported
  CGMS_FEAT_DEVICE_SPECIFIC_ALERT_SUPPORTED                 = (uint32)BV(5),  // Device Specific Alert supported
  CGMS_FEAT_SENSOR_MALFUNCTION_DETECTION_SUPPORTED          = (uint32)BV(6),  // Sensor Malfunction Detection supported
  CGMS_FEAT_SENSOR_TEMPERATURE_HIGH_LOW_DETECTION_SUPPORTED = (uint32)BV(7),  // Sensor Temperature High-Low Detection supported
  CGMS_FEAT_SENSOR_RESULT_HIGH_LOW_DETECTION_SUPPORTED      = (uint32)BV(8),  // Sensor Result High-Low Detection supported
  CGMS_FEAT_LOW_BATTERY_DETECTION_SUPPORTED                 = (uint32)BV(9),  // Low Battery Detection supported
  CGMS_FEAT_SENSOR_TYPE_ERROR_DETECTION_SUPPORTED           = (uint32)BV(10), // Sensor Type Error Detection supported
  CGMS_FEAT_GENERAL_DEVICE_FAULT_SUPPORTED                  = (uint32)BV(11), // General Device Fault supported
  CGMS_FEAT_E2E_CRC_SUPPORTED                               = (uint32)BV(12), // E2E-CRC supported
  CGMS_FEAT_MULTIPLE_BOND_SUPPORTED                         = (uint32)BV(13), // Multiple Bond supported
  CGMS_FEAT_MULTIPLE_SESSION_SUPPORTED_SUPPORTED            = (uint32)BV(14), // Multiple Sessions supported
  CGMS_FEAT_CGM_TREND_INFORMATION_SUPPORTED                 = (uint32)BV(15), // CGM Trend Information supported
  CGMS_FEAT_CGM_QUALITY_SUPPORTED                           = (uint32)BV(16)  // CGM Quality supported
} CGMS_featSupported_e;

// CGM Feature Structure
typedef struct
{
  uint32 cgmFeat;  // The supported features of the Server
  uint8  typeLoc;  // This field is the combination of the Type field and the Sample Location field
  uint16 e2eCrc;   // If the device doesn’t support E2E-safety, the value of the field shall be set to 0xFFFF
} CGMS_feat_t;

/************************************
 ********** CGM Status  *************
 ************************************/

// CGM Status Structure
typedef struct
{
  uint16 timeOffset;          // The Time Offset field specifies the time since the Session Start Time
  struct
  {
    uint8  status;
    uint8  calibTemp;
    uint8  warning;
  } status;                   // CGM Sensor Status Annunciation
} CGMS_status_t;

/************************************
 **** CGM Session Start Time  *******
 ************************************/

// CGM Session Start Time Structure
typedef struct
{
  uint16 year;       // Valid range  to 9999. A value of 0 means that the year is not known.
  uint8 month;       // Valid range 1 (January) to 12 (December). A value of 0 means that the month is not known.
  uint8 day;         // Valid range 1 to 31. A value of 0 means that the day of the month is not known.
  uint8 hours;       // Number of hours past midnight. Valid range 0 to 23.
  uint8 minutes;     // Number of minutes since the start of the hour. Valid range 0 to 59.
  uint8 seconds;     // Number of seconds since the start of the minute. Valid range 0 to 59.
  int8 timeZone;     // The Sensor Status Annunciation field is composed of three octets: one for Status, one for Cal/Temp, and one for Warning
  uint8 dstOffset;   // Represents daylight saving time information associated with time.
} CGMS_sst_t;

/************************************
  Record Access Control Point (RACP)
 ************************************/

// RACP procedure status
typedef enum
{
  RACP_PROC_NON         = 0,             // There are no ongoing RACP procedure
  RACP_PROC_INPROGRESS  = (uint8)BV(0),  // RACP procedure is in progress
  RACP_PROC_ABORT       = (uint8)BV(1),  // Incoming request to abort an ongoing RACP procedure
} RACP_proc_e;


// Op codes for RACP request/response
typedef enum
{
  RACP_OPCODE_REPORT_STORED_RECORDS        = 0x01,  // Request to send all CGM stored records
  RACP_OPCODE_ABORT_OPERATION              = 0x03,  // Request to abort ongoing operation
  RACP_OPCODE_REPORT_NUM_OF_STORED_RECORDS = 0x04,  // Request to send the amount of CGM stored records
  RACP_OPCODE_NUM_OF_STORED_RECORDS_RSP    = 0x05,  // Response with the amount of CGM stored records
  RACP_OPCODE_RSP_CODE                     = 0x06,  // Response with known codes for RACP request
} RACP_opCode_e;

// Response Code Values associated with Op Code 0x06 are defined below
typedef enum
{
  RACP_RSP_SUCCESS                  = 0x01,  // Normal response for successful operation
  RACP_RSP_OP_CODE_NOT_SUPPORTED    = 0x02,  // Normal response if unsupported Op Code is received
  RACP_RSP_INVALID_OPERATOR         = 0x03,  // Normal response if Operator received does not meet the requirements of the service
  RACP_RSP_OPERATOR_NOT_SUPPORTED   = 0x04,  // Normal response if unsupported Operator is received
  RACP_RSP_INVALID_OPERAND          = 0x05,  // Normal response if Operand received does not meet the requirements of the service
  RACP_RSP_NO_RECORDS_FOUND         = 0x06,  // Normal response if request for records resulted in no records meeting criteria
  RACP_RSP_ABORT_UNSUCCESSFUL       = 0x07,  // Normal response if request for Abort is unsuccessful
  RACP_RSP_PROCEDURE_NOT_COMPLETED  = 0x08,  // Normal response if unable to complete a procedure for any reason
  RACP_RSP_OPERAND_NOT_SUPPORTED    = 0x09,  // Normal response if unsupported Operand is received
} RACP_rspCode_e;

// Operator for RACP request/response
typedef enum
{
  RACP_OPERATOR_NULL                      = 0x00,  // Used for RACP response
  RACP_OPERATOR_ALL_RECORDS               = 0x01,  // Request to send all stored records
  RACP_OPERATOR_GREATER_THAN_OR_EQUAL_TO  = 0x03,  // Request to send all records that are greater or equal to specific time offset
  RACP_OPERATOR_INVALID                   = 0x07,  // Operators 0x07-0xFF are reserved for future use
} RACP_operator_e;

// RACP request structure
PACKED_ALIGNED_TYPEDEF_STRUCT
{
  uint8 opCode;
  uint8 operator;
  uint8 FilterType;        // In the context of the CGM service, only Filter Type 0x01 (Time Offset) is allowed to be used.
  uint16 minimum;          // minimum filter value
  uint16 maximum;          // maximum filter value
} CGMS_racpReq_t;

// RACP response structure
typedef struct
{
  uint8 opCode;
  uint8 operator;
  union
  {
    uint16 numOfRecords;     // Number of records
    struct
    {
      uint8 reqOpCode;       // Request Op Code
      uint8 rspCodeValue;    // Response Code Value
    } operandCodes;
  }  operandRsp;
} CGMS_racpRsp_t;

/************************************
 ** CGM Specific Ops Control Point **
 ************************************/

// Op codes for CGMCP request/response
typedef enum
{
  CGMCP_OPCODE_SET_COMMUNICATION_INTERVAL  = 0x1,    // Once all data records for a given request have been notified
  CGMCP_OPCODE_GET_COMMUNICATION_INTERVAL  = 0x2,    // Calibration not allowed
  CGMCP_OPCODE_COMMUNICATION_INTERVAL_RSP  = 0x3,    // Calibration not allowed
  CGMCP_OPCODE_RSP_CODE                    = 0x1C,  // Calibration recommended
} CGMCP_opCode_e;

// Response Code Values associated with Op Code 0x1C are defined below
typedef enum
{
  CGMCP_RSP_SUCCESS                  = 0x01,  // Normal response for successful operation
  CGMCP_RSP_OP_CODE_NOT_SUPPORTED    = 0x02,  // Normal response if unsupported Op Code is received
  CGMCP_RSP_INVALID_OPERAND          = 0x03,  // Normal response if Operand received does not meet the requirements of the service
  CGMCP_RSP_PROCEDURE_NOT_COMPLETED  = 0x04,  // Normal response if unable to complete a procedure for any reason
  CGMCP_RSP_PARAM_OUT_OF_RANGE       = 0x05,  // Normal response if Operand received does not meet the range requirements
} CGMCP_rspCode_e;

// Response Code Values associated with Op Code 0x1C are defined below
typedef enum
{
  CGMCP_COMM_INTERVAL_NON            = 0x00,  // Normal response for successful operation
  CGMCP_COMM_INTERVAL_FASTEST        = 0xFF,  // Normal response if unsupported Op Code is received
} CGMCP_CommInterval_e;

// CGMCP response structure
PACKED_ALIGNED_TYPEDEF_STRUCT
{
  uint8 opCode;
  uint8 reqOpCode;
  uint8 rspCodeValue;
} CGMS_cgmcpRsp_t;

// CGMCP request structure
typedef struct
{
  uint8 opCode;
  uint8 operand;
} CGMS_cgmcpReq_t;

/************************************
 *********** General ****************
 ************************************/

// Data structure used to store CCC update
typedef struct
{
  uint16 connHandle;
  uint8 value;
} CGMS_cccUpdate_t;

/*********************************************************************
 * Profile Callbacks
 */

// Callback to indicate client characteristic configuration has been updated on Measurement characteristic
typedef void ( *CGMS_measOnCccUpdate_t )( char *cccUpdate );

// Callback to indicate client characteristic configuration has been updated on RACP characteristic
typedef void ( *CGMS_racpOnCccUpdate_t )( char *cccUpdate );

// Callback to indicate client characteristic configuration has been updated on CGMCP characteristic
typedef void ( *CGMS_cgmcpOnCccUpdate_t )( char *cccUpdate );

// Callback when RACP request is received
typedef void ( *CGMS_racpReqCB_t )( char *racpReq );

// Callback when CGMCP request is received
typedef void ( *CGMS_cgmcpReqCB_t )( char *cgmcpReq );

// Callback when SST value has been updated
typedef void ( *CGMS_sstUpdateCB_t )( char *pValue );

typedef struct
{
  CGMS_measOnCccUpdate_t   pfnMeasOnCccUpdateCB;   // Called when client characteristic configuration has been updated on Measurement characteristic
  CGMS_racpOnCccUpdate_t   pfnRACPOnCccUpdateCB;   // Called when client characteristic configuration has been updated on RACP characteristic
  CGMS_cgmcpOnCccUpdate_t  pfnCGMCPOnCccUpdateCB;  // Called when client characteristic configuration has been updated on CGMCP characteristic
  CGMS_racpReqCB_t         pfnRACPReqCB;           // Called when RACP request is received
  CGMS_cgmcpReqCB_t        pfnCGMCPReqCB;          // Called when CGMCP request is received
  CGMS_sstUpdateCB_t       pfnSSTUpdateCB;          // Called when CGMCP request is received
} CGMS_cb_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

extern uint8 cgmp_racpProc;
extern uint16 cgm_curTimeOffset;

/*********************************************************************
 * API FUNCTIONS
 */

/*********************************************************************
 * @fn      CGMS_addService
 *
 * @brief   This function initializes the Data Stream Server service
 *          by registering GATT attributes with the GATT server.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGMS_addService( void );

/*
 * @fn      CGMS_registerProfileCBs
 *
 * @brief   Registers the profile callback function. Only call
 *          this function once.
 *
 * @param   profileCallback - pointer to profile callback.
 *
 * @return  SUCCESS or INVALID PARAMETER
 */
bStatus_t CGMS_registerProfileCBs( CGMS_cb_t *profileCallback );

/*
 * @fn      CGMS_setParameter
 *
 * @brief   Set a CGM Service parameter.
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
bStatus_t CGMS_setParameter( uint8 param, void *pValue, uint16 len );

/*
 * @fn      CGMS_getParameter
 *
 * @brief   Get a CGM Service parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGMS_getParameter( uint8 param, void *pValue );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* CGMSERVER_H */
