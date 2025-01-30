/*****************************************************************************

 @file  glucose_server.h

 @brief This file contains the Glucose service definitions and prototypes.

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
#ifndef GLUCOSESERVER_H
#define GLUCOSESERVER_H

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
#define GLS_SERV_UUID 0x1808

/************************************
 ***** Characteristic defines *******
 ************************************/

// GLS Measurement
#define GLS_MEAS_ID   0
#define GLS_MEAS_UUID 0x2A18

// GLS Feature
#define GLS_FEAT_ID   1
#define GLS_FEAT_UUID 0x2A51


// Record Access Control Point
#define GLS_RACP_ID   5
#define GLS_RACP_UUID 0x2A52


/************************************
 ********** GLS Measurement *********
 ************************************/

#define GLS_MEAS_MIN_RECORD_LEN    10  // Minimum length of Glicose Measurement record
#define GLS_MEAS_MAX_RECORD_LEN    sizeof(GLS_measRecord_t)  // Maximum length of Glucose Measurement record

/************************************
 ******* Glucose Base Time  *******
 ************************************/

#define GLS_BT_YEAR_MIN            1582    // Minimum value
#define GLS_BT_YEAR_MAX            9999    // Maximum value
#define GLS_BT_YEAR_UNKNOWN        0       // Year is not known

#define GLS_BT_MONTH_MIN           1       // Minimum value
#define GLS_BT_MONTH_MAX           12      // Maximum value
#define GLS_BT_MONTH_UNKNOWN       0       // Month is not known

#define GLS_BT_DAY_MIN             1       // Minimum value
#define GLS_BT_DAY_MAX             31      // Maximum value
#define GLS_BT_DAY_UNKNOWN         0       // Day is not known

#define GLS_BT_HOUR_MAX            23      // Maximum value

#define GLS_BT_MINUTE_MAX          59      // Maximum value

#define GLS_BT_SECOND_MAX          59      // Maximum value


/************************************
 ************ GLS Feature ***********
 ************************************/

#define GLS_FEAT_LEN  2

/************************************
 *** Record Access Control Point ****
 ************************************/

#define GLS_RACP_REQ_MIN_LEN           2   // Minimum length of RACP Request
#define GLS_RACP_REQ_MAX_LEN           7   // Maximum length of RACP Request
#define GLS_RACP_RSP_LEN               4   // RACP Response length
#define GLS_RACP_OPERAND_SEQUENCE_NUM  1   // For the GL service, only Sequence Number operand(0x01) is allowed to be used.
#define GLS_RACP_OPERAND_NULL          0   // Null operand

// Error codes
#define GLS_RACP_PROC_IN_PROG               0x80 // The Client Characteristic Configuration descriptor is not configured according to the requirements of the service.
#define GLS_RACP_CCCD_IMPROPERLY_CONFG      0x81 // A Record Access Control Point request cannot be serviced because a previously triggered RACP operation is still in progress

/*********************************************************************
 * TYPEDEFS
 */


// GLS Measurement Flags field
typedef enum
{
  GLS_MEAS_FLAGS_TIME_OFFSET_PRESENT          = (uint8)BV(0),  // Time Offset field present
  GLS_MEAS_FLAGS_CONCENTRATION_TYPE_PRESENT   = (uint8)BV(1),  // GLS Concentration and Type-Sample Location fields presen
  GLS_MEAS_FLAGS_UNITS_MMOL                   = (uint8)BV(2),  // GLS concentration in units of mmol/L
  GLS_MEAS_FLAGS_UNITS_MG                     = (uint8)~BV(2),  // GLS concentration in units of mg/dL
  GLS_MEAS_FLAGS_SENSOR_STATUS_PRESENT        = (uint8)BV(3),  // Sensor Status Annunciation field present
  GLS_MEAS_FLAGS_CONTEXT                      = (uint8)BV(4)   // This record includes context information
} GLS_measFlags_e;

// GLS Measurement status field in Sensor Status Annunciation
typedef enum
{
  GLS_STAT_DEV_BATTERY_LOW            = (uint8)BV(0),  // The battery was low at the time of measurement
  GLS_STAT_SENSOR_MALFUNCTION         = (uint8)BV(1),  // The sensor was malfunctioning or faulting at the time of measuremen
  GLS_STAT_SAMPLE_SIZE_INSUFFICIENT   = (uint8)BV(2),  // There was not enough blood or control solution on the strip during the measurement.
  GLS_STAT_STRIP_INSERTION_ERROR      = (uint8)BV(3),  // The strip was not inserted correctly
  GLS_STAT_STRIP_TYPE_INCORRECT       = (uint8)BV(4),  // The strip was not the right type for the device
  GLS_STAT_RESULT_TOO_HIGH            = (uint8)BV(5),   // The reading or value was higher than the device can process
  GLS_STAT_RESULT_TOO_LOW             = (uint8)BV(6),   // The reading or value was lower than the device can process
  GLS_STAT_TEMPERATURE_TOO_HIGH       = (uint8)BV(7),   // The ambient temperature was too high for a valid test/result at the time of measurement
  GLS_STAT_TEMPERATURE_TOO_LOW        = (uint8)BV(8),   // The ambient temperature was too low for a valid test/result at the time of measurement
  GLS_STAT_READ_INTERRUPTED           = (uint8)BV(9),   // The ambient temperature was too low for a valid test/result at the time of measurement
  GLS_STAT_GENERAL_DEVICE_FAULT       = (uint8)BV(10),  // A general device fault has occurred in the sensor device
  GLS_STAT_TIME_FAULT                 = (uint8)BV(10),  // A time fault has occurred in the sensor device and the time is inaccurate
} GLS_measStatus_e;

//Glucose Measurment struct
typedef struct
{
  uint8   flags;
  uint16  SequenceNum;
  struct
  {
    uint16 year;
    uint8 month;
    uint8 day;
    uint8 hours;
    uint8 minutes;
    uint8 seconds;
  } timeBase;
  int16 TimeOffset;
  uint16  glucoseConcen;
  struct{
    uint8 Type : 4;
    uint8 SampleLocation :4;

  } Type_SampleLocation;
  uint16  SensorStatus;
} GLS_measRecord_t;

/************************************
 ********** Glucose Feature  ************
 ************************************/

// Glucose Feature: indicate whice features the device support.
typedef enum GLS_featSupported_e
{
  GLS_FEAT_LOW_BATTERY_DETECTION_SUPPORTED                   = (uint32)BV(0),  // Calibration supported
  GLS_FEAT_SENSOR_MALFUNCTION_DETECTION_SUPPORTED            = (uint32)BV(1),  // Patient High/Low Alerts supported
  GLS_FEAT_SAMPLE_SIZE_SUPPORTED                             = (uint32)BV(2),  // Hypo Alerts supported
  GLS_FEAT_STRIP_INSERTION_ERROR_DETECTION_SUPPORTED         = (uint32)BV(3),  // Hyper Alerts supported
  GLS_FEAT_STRIP_TYPE_ERROR_DETECTION_SUPPORTED              = (uint32)BV(4),  // Rate of Increase/Decrease Alerts supported
  GLS_FEAT_SENSOR_RESULT_HIGH_LOW_DETECTION_SUPPORTED        = (uint32)BV(5),  // Device Specific Alert supported
  GLS_FEAT_SENSOR_TEMPERATURE_HIGH_LOW_DETECTION_SUPPORTED   = (uint32)BV(6),  // Sensor Malfunction Detection supported
  GLS_FEAT_SENSOR_READ_INTERRUPT_DETECTION_SUPPORTED         = (uint32)BV(7),  // Sensor Result High-Low Detection supported
  GLS_FEAT_GEMERAL_DEVICE_FAULT_SUPPORTED                    = (uint32)BV(8),  // Low Battery Detection supported
  GLS_FEAT_TIME_FAULT_SUPPORTED                              = (uint32)BV(9), // Sensor Type Error Detection supported
  GLS_FEAT_MULTIPLE_BOND_SUPPORTED                           = (uint32)BV(10), // General Device Fault supported
} GLS_featSupported_e;

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
  RACP_OPCODE_REPORT_STORED_RECORDS        = 0x01,  // Request to send all Glucose stored records
  RACP_OPCODE_ABORT_OPERATION              = 0x03,  // Request to abort ongoing operation
  RACP_OPCODE_REPORT_NUM_OF_STORED_RECORDS = 0x04,  // Request to send the amount of Glucose stored records
  RACP_OPCODE_NUM_OF_STORED_RECORDS_RSP    = 0x05,  // Response with the amount of Glucose stored records
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
  RACP_OPERATOR_LAST_RECORD               = 0x06,  // Request to send last record
  RACP_OPERATOR_INVALID                   = 0x07,  // Operators 0x07-0xFF are reserved for future use
} RACP_operator_e;

//RACP message Struct
PACKED_ALIGNED_TYPEDEF_STRUCT
{
  uint8 opCode;
  uint8 operator;
  uint8 FilterType;        // In the context of the GLS service, only Filter Type 0x01 (Sequence Number) is allowed to be used.
  uint16 minimum;          // minimum filter value
  uint16 maximum;          // maximum filter value
} GLS_racpReq_t;

//Number of records RACP message Struct
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
} GLS_racpRsp_t;

/************************************
 *********** General ****************
 ************************************/

// Data structure used to store CCC update
typedef struct
{
  uint16 connHandle;
  uint8 value;
} GLS_cccUpdate_t;

/*********************************************************************
 * Profile Callbacks
 */
// Callback to indicate client characteristic configuration has been updated on Measurement characteristic
typedef void ( *GLS_measOnCccUpdate_t )( char *cccUpdate );

// Callback to indicate client characteristic configuration has been updated on RACP characteristic
typedef void ( *GLS_racpOnCccUpdate_t )( char *cccUpdate );

// Callback when RACP request is received
typedef void ( *GLS_racpReqCB_t )( char *racpReq );

//Server CallBack struct
typedef struct
{
  GLS_measOnCccUpdate_t   pfnMeasOnCccUpdateCB;   // Called when client characteristic configuration has been updated on Measurement characteristic
  GLS_racpOnCccUpdate_t   pfnRACPOnCccUpdateCB;   // Called when client characteristic configuration has been updated on RACP characteristic
  GLS_racpReqCB_t         pfnRACPReqCB;           // Called when RACP request is received
} GLS_cb_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

extern uint8 glp_racpProc;
/*********************************************************************
 * API FUNCTIONS
 */

/*********************************************************************

 * @fn      GLS_registerProfileCBs
 *
 * @brief   Registers the profile callback function. Only call
 *          this function once.
 *
 * @param   profileCallback - pointer to profile callback.
 *
 * @return  SUCCESS or INVALID PARAMETER
 */
bStatus_t GLS_registerProfileCBs( GLS_cb_t *profileCallback );

/*
 * @fn      GLS_setParameter
 *
 * @brief   Set a GLS Service parameter.
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
bStatus_t GLS_setParameter( uint8 param, void *pValue, uint16 len );

/*
 * @fn      GLS_getParameter
 *
 * @brief   Get a Glucose Service parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t GLS_getParameter( uint8 param, void *pValue );

/* @fn      GLS_addService
 *
 * @brief   This function initializes the Data Stream Server service
 *          by registering GATT attributes with the GATT server.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t GLS_addService( void );
/*********************************************************************
*********************************************************************/

#endif
