/******************************************************************************

 @file  glucose_profile.c

 @brief This file contains the Glucose profile sample for use with the Basic BLE
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

#include <ti/bleapp/profiles/glucose/glucose_profile.h>
#include <ti/bleapp/services/glucose/glucose_server.h>
#include <ti/bleapp/profiles/glucose/glucose_db.h>

/*********************************************************************
 * DEFINES
 */

/*********************************************************************
* MACROS
*/

#define GLP_SENSOR_STATUS_IS_SUPPORTED(x)   x & ~ BV(9)

/*********************************************************************/

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

uint16 glp_seqNum = 0;
static GLP_cb_t *glp_appCB = NULL;
uint8 glp_racpProc = RACP_PROC_NON;
GLP_timeBase glp_timeBase = {0};

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void GLP_measOnCccUpdateCB( char *pValue );
static void GLP_racpOnCccUpdateCB( char *pValue );
static void GLP_racpReqCB( char *pValue);
static void GLP_sendRacpRsp( uint8 opCode, uint8 operator, uint8 reqOpCode,
                              uint8 rspCodeValue, uint16 numOfRecords );
static void GLP_sendRecords( uint16 minTimeOffset, uint16 maxTimeOffset );
static void GLP_SendMeasRecord(GLS_measRecord_t *measRecord);
static bStatus_t GLP_validateTimebase(GLP_timeBase timeBase);
/*********************************************************************
 * SERVER CALLBACKS
 */

// GL profile callback functions
static GLS_cb_t glp_serverCB =
{
  GLP_measOnCccUpdateCB,
  GLP_racpOnCccUpdateCB,
  GLP_racpReqCB,
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      GLP_start
 *
 * @brief   This function adds the GL Server service,
 *          registers the service callback function and initializes
 *          the service characteristics
 *
 * @param   appCallbacks - pointer to application callback
 * @param   feat - GL Feature parameters
 * @param   timeOffset - GL initial time offset parameter
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t GLP_start( GLP_cb_t *appCallbacks, uint16 feat, GLP_timeBase* gl_timebase)
{
  uint8 status = SUCCESS;
  status = GLP_validateTimebase(*gl_timebase);
  if(status != SUCCESS)
  {
      return status;
  }
  memcpy(&glp_timeBase, gl_timebase, GLP_TIME_BASE_LEN);

  // Add Glucose service
  status = GLS_addService();
  if ( status != SUCCESS )
  {
    // Return status value
    return ( status );
  }

  // Register to service callback function
  status = GLS_registerProfileCBs( &glp_serverCB );
  if ( status != SUCCESS )
  {
    // Return status value
    return ( status );
  }

  // Registers the application callback function
  if ( appCallbacks )
  {
    glp_appCB = appCallbacks;
  }
  else
  {
    return ( INVALIDPARAMETER );
  }

  // Init sequence number
  glp_seqNum = 0;
  // Set GL characteristics
  // GL Feature
  status |= GLS_setParameter( GLS_FEAT_ID, &feat, GLS_FEAT_LEN );

  // Init GL DB
  status |= GL_DB_initDataBase();
  // Return status value
  return ( status );
}


/*********************************************************************
 * @fn      GLP_addMeaserment
 *
 * @brief   Create GLP measurement record, add it to the data base
 *
 * @param   glucoseConcen - a Glucose concentration in mg/dL units
 * @param   timeOffset - Minutes since the Session Start Time
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t GLP_addMeaserment( uint16  glucoseConcen, uint16  timeOffset, uint8 type, uint8 location, uint16 sensorStatus)
{
  uint8 status = SUCCESS;
  GLS_measRecord_t meas = {0};
  uint16 glp_feat = 0;
  //  Update GL measurement parameters
  meas.SequenceNum = glp_seqNum;
  // Add timebase
  memcpy(&meas.timeBase, &glp_timeBase, GLP_TIME_BASE_LEN);
  //  Verify that glucose concentration is included, it's not mandatory
  if(glucoseConcen > 0)
  {
    // verify that type and location is included. if glucose concentration is included, type and locatino is mandatory.
    if((type < GLP_TYPE_MIN_VALUE || type > GLP_TYPE_MAX_VALUE) ||
       ((location < GLP_LOCATION_MIN_VALUE || location > GLP_LOCATION_MAX_VALUE) && (location != GLP_LOCATION_NOT_AVAILABLE)))
    {
      status = INVALIDPARAMETER;
      return status;
    }
    meas.glucoseConcen = glucoseConcen;
    meas.Type_SampleLocation.Type = type;
    meas.Type_SampleLocation.SampleLocation = location;
    //  Set flags according to the included fields.
    meas.flags |= GLS_MEAS_FLAGS_CONCENTRATION_TYPE_PRESENT;
  }
  // Verify that timeOffset is included, it's not mandatory.
  if(timeOffset > 0)
  {
    meas.flags |= GLS_MEAS_FLAGS_TIME_OFFSET_PRESENT;
    meas.TimeOffset = timeOffset;
  }
  GLS_getParameter(GLS_FEAT_ID, &glp_feat);
  if(GLP_SENSOR_STATUS_IS_SUPPORTED(glp_feat))
  {
    meas.SensorStatus = sensorStatus;
    meas.flags |= GLS_MEAS_FLAGS_SENSOR_STATUS_PRESENT;
  }
  //  Increment sequence number
  glp_seqNum++;
  //  Add GL record to data base
  status = GL_DB_addMeasRecord( &meas );
  //  Send measurement to registered devices.
  GLP_SendMeasRecord(&meas);
  //  Return status value
  return ( status );
}

/*********************************************************************
 * @fn      GLP_SendMeasRecord
 *
 * @brief   Send GL measurement record using GATT notifications
 *
 * @param   measRecord - measurment record to send
 *
 * @return  void
 */
static void GLP_SendMeasRecord( GLS_measRecord_t *measRecord )
{
  uint8 pBuf[GLS_MEAS_MAX_RECORD_LEN] = {0};
  uint8 len = GLS_MEAS_MIN_RECORD_LEN;
  // Allocate buffer with the actual size of the GL measurement record
  VOID memcpy( pBuf, &measRecord->flags, sizeof( measRecord->flags ));
  VOID memcpy( pBuf+1, &measRecord->SequenceNum, sizeof( measRecord->SequenceNum ));
  VOID memcpy( pBuf+3, &measRecord->timeBase, sizeof( measRecord->timeBase ));

  // Send time offset only if the timeOffset flag is set to 1.
  if ( measRecord->flags & GLS_MEAS_FLAGS_TIME_OFFSET_PRESENT)
  {
    VOID memcpy( pBuf + len, &measRecord->TimeOffset, sizeof( measRecord->TimeOffset ));
    len += sizeof( measRecord->TimeOffset );
  }
  // Send glucose concentration, type and location only if glucose concentration flag is set to 1.
  if ( measRecord->flags & GLS_MEAS_FLAGS_CONCENTRATION_TYPE_PRESENT)
  {
    VOID memcpy( pBuf + len, &measRecord->glucoseConcen, sizeof( measRecord->glucoseConcen ));
    len += sizeof( measRecord->glucoseConcen );
    VOID memcpy( pBuf + len, &measRecord->Type_SampleLocation, sizeof( measRecord->Type_SampleLocation ));
    len += sizeof( measRecord->Type_SampleLocation );
  }
  // Send SensorStatus only if sensor status flag is set to 1.
  if ( measRecord->flags & GLS_MEAS_FLAGS_SENSOR_STATUS_PRESENT )
  {
    VOID memcpy( pBuf + len, &measRecord->SensorStatus, sizeof( measRecord->SensorStatus ));
    len += sizeof( measRecord->SensorStatus );
  }

  GLS_setParameter( GLS_MEAS_ID, pBuf, len);
}


/*********************************************************************
 * @fn      GLP_measOnCccUpdateCB
 *
 * @brief   Callback from GL Server indicating CCC has been update
 *          for Measurement characteristic
 *
 * @param   pValue - pointer to data
 *
 * @return  None
 */
static void GLP_measOnCccUpdateCB( char *pValue )
{
  GLS_cccUpdate_t *cccUpdate = (GLS_cccUpdate_t *)pValue;

  // Verify input parameters
  if ( pValue == NULL )
  {
    return;
  }

  // If the callback function is not NULL, notify the application
  if ( glp_appCB && glp_appCB->pfnMeasOnCccUpdateCB )
  {
    glp_appCB->pfnMeasOnCccUpdateCB( cccUpdate->connHandle, cccUpdate->value );
  }
}

/*********************************************************************
 * @fn      GLP_racpOnUpdateCccCB
 *
 * @brief   Callback from GL Server indicating CCC has been update
 *          for Record Access Control Point characteristic
 *
 * @param   pValue - pointer to data
 *
 * @return  None
 */
static void GLP_racpOnCccUpdateCB( char *pValue )
{
  GLS_cccUpdate_t *cccUpdate = (GLS_cccUpdate_t *)pValue;

  // Verify input parameters
  if ( pValue == NULL )
  {
    return;
  }

  // If the callback function is not NULL, notify the application
  if ( glp_appCB && glp_appCB->pfnRACPOnCccUpdateCB )
  {
    glp_appCB->pfnRACPOnCccUpdateCB( cccUpdate->connHandle, cccUpdate->value );
  }
}


/*********************************************************************
 * @fn      GLP_racpReqCB
 *
 * @brief   Callback from GL Server indicating RACP request has been update
 *          for Record Access Control Point characteristic
 *
 * @param   pValue - pointer to RACP request
 *
 * @return  None
 */
static void GLP_racpReqCB( char *pValue)
{
  GLS_racpReq_t *racpReq = (GLS_racpReq_t *)pValue;

  // Verify input parameters
  if ( pValue == NULL )
  {
    return;
  }

  // Find out the request opCode
  switch ( racpReq->opCode )
  {
    /******************************************************/
    /******** Request to send stored recodes **************/
    /******************************************************/

    case RACP_OPCODE_REPORT_STORED_RECORDS:
    {
      // Request for all GLC records
      if (racpReq->operator == RACP_OPERATOR_ALL_RECORDS )
      {
        if ( racpReq->FilterType != GLS_RACP_OPERAND_NULL )
        {
          // Invalid operand
          GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                            RACP_RSP_INVALID_OPERAND, 0);
        }
        else if ( GL_DB_getCurrentNumOfRecords() > 0 )
        {
          // Send all stored GLC record over BLE notifications
          GLP_sendRecords( GL_DB_getFirstSequenceNum(), GL_DB_getLastSequenceNum() );

          // only send success if the RACP was not aborted
          if ( !(glp_racpProc & RACP_PROC_ABORT) )
          {
              // The RACP procedure complete successfully
              GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                                RACP_RSP_SUCCESS, 0 );
          }
          glp_racpProc = RACP_PROC_NON;
          return;
        }
        else
        {
          // There are no stored recodes, send response over BLE indication
          GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                            RACP_RSP_NO_RECORDS_FOUND, 0 );
        }
      }

      // Request for all GLC records that are greater than or equal to requested time offset
      else if ( racpReq->operator == RACP_OPERATOR_GREATER_THAN_OR_EQUAL_TO )
      {
          // only Filter Type 0x01 (Time Offset) is allowed to be used.
          if ( racpReq->FilterType == GLS_RACP_OPERAND_SEQUENCE_NUM )
          {
             if ( GL_DB_getNumOfRecordsGreaterOrEqual( racpReq->minimum ) > 0 )
            {
              // Send all stored GLC records over BLE notifications
              GLP_sendRecords( racpReq->minimum, GL_DB_getLastSequenceNum() );

              // There are stored records for that given request, send response over BLE indication
              GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                                RACP_RSP_SUCCESS, 0 );

              glp_racpProc = RACP_PROC_NON;
            }
            else
            {
              // There are no stored recodes, send response over BLE indication
              GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                                RACP_RSP_NO_RECORDS_FOUND, 0 );
            }
          }
          else
          {
            // Send response over BLE indication
            GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                              RACP_RSP_OPERAND_NOT_SUPPORTED, 0 );

          }
      }

      else if ( racpReq->operator == RACP_OPERATOR_LAST_RECORD )
      {
          if ( GL_DB_getNumOfRecordsGreaterOrEqual( GL_DB_getLastSequenceNum() ) > 0 )
         {
           // Send all stored GLC records over BLE notifications
           GLP_sendRecords( GL_DB_getLastSequenceNum(), GL_DB_getLastSequenceNum() );

           // There are stored records for that given request, send response over BLE indication
           GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                             RACP_RSP_SUCCESS, 0 );

           glp_racpProc = RACP_PROC_NON;
         }
         else
         {
           // There are no stored recodes, send response over BLE indication
           GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                             RACP_RSP_NO_RECORDS_FOUND, 0 );
         }
      }
      // The requested operator is invalid or not supported
      else if ( racpReq->operator >= RACP_OPERATOR_INVALID )
      {
        // The requested operator is not supported, send response over BLE indication
        GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                          RACP_RSP_OPERATOR_NOT_SUPPORTED, 0 );
      }
      else
      {
        // The requested operator is invalid, send response over BLE indication
        GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                          RACP_RSP_INVALID_OPERATOR, 0 );
      }
    }
    break;

    /******************************************************/
    /*********** Request to abort operation ***************/
    /******************************************************/

    case RACP_OPCODE_ABORT_OPERATION:
    {
      if ( racpReq->operator != RACP_OPERATOR_NULL )
      {
        // The requested operator is invalid, send response over BLE indication
       GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                         RACP_RSP_INVALID_OPERATOR, 0 );
      }
      else
      {
      // Send response over BLE indication
        GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                        RACP_RSP_SUCCESS, 0 );
      }
    }
    break;

    /******************************************************/
    /***** Request to send number of stored recodes *******/
    /******************************************************/

    case RACP_OPCODE_REPORT_NUM_OF_STORED_RECORDS:
    {
      // Request for all GLC records
      if ( racpReq->operator == RACP_OPERATOR_ALL_RECORDS )
      {
        // Send response over BLE indication
       GLP_sendRacpRsp( RACP_OPCODE_NUM_OF_STORED_RECORDS_RSP, RACP_OPERATOR_NULL, 0,
                         0, GL_DB_getCurrentNumOfRecords() );
      }

      // Request for all GLC records that are greater than or equal to requested time offset
      else if (racpReq->operator == RACP_OPERATOR_GREATER_THAN_OR_EQUAL_TO )
      {
        // only Filter Type 0x01 (Sequence Number) is allowed to be used.
        if ( racpReq->FilterType != GLS_RACP_OPERAND_SEQUENCE_NUM )
        {
            // Send error response over BLE indication
           GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                             RACP_RSP_OPERAND_NOT_SUPPORTED, 0 );
        }
        else
        {
          // Send response over BLE indication
          GLP_sendRacpRsp( RACP_OPCODE_NUM_OF_STORED_RECORDS_RSP, RACP_OPERATOR_NULL, 0,
                            0, GL_DB_getNumOfRecordsGreaterOrEqual( racpReq->minimum ));
        }
      }

      // The requested operator is invalid or not supported
      else if ( racpReq->operator >= RACP_OPERATOR_INVALID )
      {
        // The requested operator is not supported, send response over BLE indication
        GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                          RACP_RSP_OP_CODE_NOT_SUPPORTED, 0 );
      }
      else
      {
        // The requested operator is invalid, send response over BLE indication
        GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                          RACP_RSP_INVALID_OPERATOR, 0 );
      }
    }
    break;

    // The requested OpCode is not supported
    default:
    {
      // The requested operator is not supported, send response over BLE indication
      GLP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                        RACP_RSP_OP_CODE_NOT_SUPPORTED, 0 );
    }
    break;
  }
}

/*********************************************************************
 * @fn      GLP_sendRacpRsp
 *
 * @brief   Send RACP response over BLE indications
 *
 * @param   opCode - RACP response opcode
 * @param   operator - RACP response operator
 * @param   reqOpCode - RACP request opcode
 * @param   rspCodeValue - RACP response code value
 * @param   numOfRecords - Number of record reported
 *
 * @return  none
 */
static void GLP_sendRacpRsp( uint8 opCode, uint8 operator, uint8 reqOpCode,
                              uint8 rspCodeValue, uint16 numOfRecords )
{
  GLS_racpRsp_t racpRsp;
  // both type of messages include opCode and opeator.
  racpRsp.opCode = opCode;
  racpRsp.operator = operator;
  // if it's response RACP message, send the right correct opcode and the right code value
  // else it's a numOfrecords RACP message, send number of records.
  if ( opCode == RACP_OPCODE_RSP_CODE )
  {
    racpRsp.operandRsp.operandCodes.reqOpCode = reqOpCode;
    racpRsp.operandRsp.operandCodes.rspCodeValue = rspCodeValue;
  }
  else
  {
    racpRsp.operandRsp.numOfRecords = numOfRecords;
  }

  // Send RACP response over BLE indication
  GLS_setParameter( GLS_RACP_ID, &racpRsp, GLS_RACP_RSP_LEN );
}

/*********************************************************************
 * @fn      GLP_sendRecords
 *
 * @brief   Send GLC measurement records with corresponding time offset
 *          using GATT notifications
 *
 * @param   minSequenceNum - minimum vale of time offset field
 * @param   maxSequenceNum - maximum vale of time offset field
 *
 * @return  none
 */
static void GLP_sendRecords( uint16 minSequenceNum, uint16 maxSequenceNum )
{
  GLS_measRecord_t *measRecord = NULL;
  uint16 i = 0;

  // Mark RACP procedure as in progress
  glp_racpProc |= RACP_PROC_INPROGRESS;

  // Find all GLC measurement records with corresponding time offset
  for ( i = minSequenceNum; i <= maxSequenceNum; i++ )
  {
    if ( glp_racpProc & RACP_PROC_ABORT )
    {
      return;
    }
    // Check if the record with sequence number i exist in the data base
    if ( GL_DB_getMeasRecord( i, &measRecord ) == SUCCESS )
    {
      GLP_SendMeasRecord(measRecord);
    }
  }
}


/*********************************************************************
 * @fn      GLP_validateTimebase
 *
 * @brief   Validate TimeBase values, return invalid parameter in case of a validation
 *
 * @param   timeBase - timeBase given by the App.
 *
 * @return  none
 */
bStatus_t GLP_validateTimebase(GLP_timeBase timeBase)
{
  uint8 status = SUCCESS;
  if(timeBase.day < GLS_BT_DAY_MIN || timeBase.day > GLS_BT_DAY_MAX || timeBase.hours > GLS_BT_HOUR_MAX ||
          timeBase.minutes > GLS_BT_MINUTE_MAX || timeBase.month < GLS_BT_MONTH_MIN || timeBase.month > GLS_BT_MONTH_MAX ||
          timeBase.seconds > GLS_BT_SECOND_MAX || timeBase.year > GLS_BT_YEAR_MAX ||
          ((timeBase.year < GLS_BT_YEAR_MIN) & (GLS_BT_YEAR_UNKNOWN != GLS_BT_YEAR_UNKNOWN)))
  {
    return ( INVALIDPARAMETER );
  }
  return status;
}
