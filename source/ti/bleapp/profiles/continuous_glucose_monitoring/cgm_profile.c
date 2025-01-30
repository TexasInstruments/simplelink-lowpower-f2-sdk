/******************************************************************************

 @file  cgm_profile.c

 @brief This file contains the CGM profile sample for use with the Basic BLE
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

#include <ti/bleapp/profiles/continuous_glucose_monitoring/cgm_profile.h>
#include <ti/bleapp/services/continuous_glucose_monitoring/cgm_server.h>
#include <ti/bleapp/profiles/continuous_glucose_monitoring/cgm_db.h>

/*********************************************************************
 * DEFINES
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static CGMP_cb_t *cgmp_appCB = NULL;
uint8 cgmp_racpProc = RACP_PROC_NON;                           // RACP procedure status
uint8 cgmp_cgmcpCommInterval = CGMS_CGMCP_COMM_INT_VAL_NON;    // CGM communication interval value in minutes
static Clock_Struct cgmp_CommIntClk;                           // CGM communication interval clock

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void CGMP_measOnCccUpdateCB( char *pValue );
static void CGMP_racpOnCccUpdateCB( char *pValue );
static void CGMP_cgmcpOnCccUpdateCB( char *pValue );
static void CGMP_sstUpdateCB( char *pValue );
static void CGMP_racpReqCB( char *pValue);
static void CGMP_sendRacpRsp( uint8 opCode, uint8 operator, uint8 reqOpCode,
                              uint8 rspCodeValue, uint16 numOfRecords );
static void CGMP_cgmcpReqCB( char *pValue);
static void CGMP_sendCgmcpRsp( uint8 opCode, uint8 reqOpCode, uint8 rspCodeValue );
static void CGMP_sendRecords( uint16 minTimeOffset, uint16 maxTimeOffset );
static void CGMP_CommIntClkTimeout( char *pData );

/*********************************************************************
 * SERVER CALLBACKS
 */

// CGM profile callback functions
static CGMS_cb_t cgmp_serverCB =
{
  CGMP_measOnCccUpdateCB,
  CGMP_racpOnCccUpdateCB,
  CGMP_cgmcpOnCccUpdateCB,
  CGMP_racpReqCB,
  CGMP_cgmcpReqCB,
  CGMP_sstUpdateCB
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      CGMP_start
 *
 * @brief   This function adds the CGM Server service,
 *          registers the service callback function and initializes
 *          the service characteristics
 *
 * @param   appCallbacks - pointer to application callback
 * @param   feat - CGM Feature parameters
 * @param   timeOffset - CGM initial time offset parameter
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGMP_start( CGMP_cb_t *appCallbacks, CGMS_feat_t feat, uint16 timeOffset )
{
  uint8 status = SUCCESS;
  uint16 srt = CGMP_DEFAUALT_SRT;
  CGMS_status_t stat = {0};

  // Init time offset value
  stat.timeOffset = timeOffset;
  stat.status.calibTemp |= CGMS_CALIB_TIME_SYNC_REQUIRED;

  // Add data stream service
  status = CGMS_addService();
  if ( status != SUCCESS )
  {
    // Return status value
    return ( status );
  }

  // Register to service callback function
  status = CGMS_registerProfileCBs( &cgmp_serverCB );
  if ( status != SUCCESS )
  {
    // Return status value
    return ( status );
  }

  // Registers the application callback function
  if ( appCallbacks )
  {
    cgmp_appCB = appCallbacks;
  }
  else
  {
    return ( INVALIDPARAMETER );
  }

  // Set CGM characteristics
  // CGM Feature
  status |= CGMS_setParameter( CGMS_FEAT_ID, &feat, CGMS_FEAT_LEN );

  // CGM Status
  status |= CGMS_setParameter( CGMS_STAT_ID, &stat, CGMS_STAT_LEN );

  // CGM Status Session Run Time
  status |= CGMS_setParameter( CGMS_SRT_ID, &srt, CGMS_SRT_LEN );

  // Init CGM DB
  status |= CGM_DB_initDataBase( CGMS_MEAS_DEFAUALT_INTERVAL );

  // Check if the CGMCP communiction interval is not zero
  if ( cgmp_cgmcpCommInterval != CGMS_CGMCP_COMM_INT_VAL_NON )
  {
    // Create and CGMP activity timer
    Util_constructClock( &cgmp_CommIntClk, CGMP_clockCB,
                         cgmp_cgmcpCommInterval, cgmp_cgmcpCommInterval,
                         TRUE, (uint32)CGMP_CommIntClkTimeout );
  }
  else
  {
    // There is no communication interval, only construct clock
    Util_constructClock( &cgmp_CommIntClk, CGMP_clockCB,
                         CGMS_CGMCP_COMM_INT_VAL_NON, CGMS_CGMCP_COMM_INT_VAL_NON,
                         FALSE, (uint32)CGMP_CommIntClkTimeout );
  }

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      CGMP_addMeaserment
 *
 * @brief   Create CGM measurement record, add it to the data base
 *          and update CGM status characteristic
 *
 * @param   glucoseConcen - a Glucose concentration in mg/dL units
 * @param   timeOffset - Minutes since the Session Start Time
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGMP_addMeaserment( uint16  glucoseConcen, uint16  timeOffset )
{
  uint8 status = SUCCESS;
  CGMS_status_t stat = {0};
  CGMS_measRecord_t meas = {0};

  // Update CGM measurement parameters
  meas.size = CGMS_MEAS_MIN_RECORD_LEN;
  meas.glucoseConcen = glucoseConcen;
  meas.timeOffset = timeOffset;

  // Check if Sensor Status Annunciation fields should be added to the measurement
  status = CGMS_getParameter( CGMS_STAT_ID, &stat );
  if ( status == SUCCESS )
  {
    // Check if Status field should be added to the measurement record
    if ( stat.status.status != 0 )
    {
      // Add Status field, update the flags field and increase measurement record size
      meas.status.status = stat.status.status;
      meas.flags |= CGMS_MEAS_FLAGS_STATUS_PRESENT;
      meas.size++;
    }

    // Check if Cal/Temp field should be added to the measurement record
    if ( stat.status.calibTemp != 0 )
    {
      // Add Cal/Temp field, update the flags field and increase measurement record size
      meas.status.calibTemp = stat.status.calibTemp;
      meas.flags |= CGMS_MEAS_FLAGS_CALIB_TEMP_PRESENT;
      meas.size++;
    }

    // Check if Warning field should be added to the measurement record
    if ( stat.status.warning != 0 )
    {
      // Add Warning field, update the flags  and increase measurement record size
      meas.status.warning = stat.status.warning;
      meas.flags |= CGMS_MEAS_FLAGS_WARNING_PRESENT;
      meas.size++;
    }

    // Add CGM record to data base
    status = CGM_DB_addMeasRecord( &meas );

    // Update status characteristic with the updated timeOffset
    stat.timeOffset = timeOffset;
    status = CGMS_setParameter( CGMS_STAT_ID, &stat, CGMS_STAT_LEN );
  }

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      CGMP_updateStatus
 *
 * @brief   Update status characteristic on the server attributes table
 *
 * @param   stat - updated status value
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGMP_updateStatus( CGMS_status_t stat )
{
  uint8 status = SUCCESS;

  status = CGMS_setParameter( CGMS_STAT_ID, &stat, CGMS_STAT_LEN );

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      CGMP_updateSessionRunTime
 *
 * @brief   Update session run time characteristic on the server attributes table
 *
 * @param   timeToReduce - value in hours to reduce from session run time charicterstic
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGMP_updateSessionRunTime( uint16 timeToReduce )
{
  uint8 status = SUCCESS;
  uint16 cgmp_srt_val = 0;

  // Get the current SRT value
  CGMS_getParameter( CGMS_SRT_ID, &cgmp_srt_val );

  // Check if the current SRT is grater than 0
  if ( cgmp_srt_val == 0 )
  {
    return ( FAILURE );
  }

  // Reduce time from the SRT value
  cgmp_srt_val -= timeToReduce;

  // Update the SRT value
  status = CGMS_setParameter( CGMS_SRT_ID, &cgmp_srt_val, CGMS_SRT_LEN );

  // Return status value
  return ( status );
}

/*********************************************************************
 * @fn      CGMP_measOnCccUpdateCB
 *
 * @brief   Callback from CGM Server indicating CCC has been update
 *          for Measurement characteristic
 *
 * @param   pValue - pointer to data
 *
 * @return  None
 */
static void CGMP_measOnCccUpdateCB( char *pValue )
{
  CGMS_cccUpdate_t *cccUpdate = (CGMS_cccUpdate_t *)pValue;

  // Verify input parameters
  if ( pValue == NULL )
  {
    return;
  }

  // If the callback function is not NULL, notify the application
  if ( cgmp_appCB && cgmp_appCB->pfnMeasOnCccUpdateCB )
  {
      cgmp_appCB->pfnMeasOnCccUpdateCB( cccUpdate->connHandle, cccUpdate->value );
  }
}

/*********************************************************************
 * @fn      CGMP_racpOnUpdateCccCB
 *
 * @brief   Callback from CGM Server indicating CCC has been update
 *          for Record Access Control Point characteristic
 *
 * @param   pValue - pointer to data
 *
 * @return  None
 */
static void CGMP_racpOnCccUpdateCB( char *pValue )
{
  CGMS_cccUpdate_t *cccUpdate = (CGMS_cccUpdate_t *)pValue;

  // Verify input parameters
  if ( pValue == NULL )
  {
    return;
  }

  // If the callback function is not NULL, notify the application
  if ( cgmp_appCB && cgmp_appCB->pfnRACPOnCccUpdateCB )
  {
    cgmp_appCB->pfnRACPOnCccUpdateCB( cccUpdate->connHandle, cccUpdate->value );
  }
}

/*********************************************************************
 * @fn      CGMP_cgmcpOnCccUpdateCB
 *
 * @brief   Callback from CGM Server indicating CCC has been update
 *          for CGM Control Point characteristic
 *
 * @param   pValue - pointer to data
 *
 * @return  None
 */
static void CGMP_cgmcpOnCccUpdateCB( char *pValue )
{
  CGMS_cccUpdate_t *cccUpdate = (CGMS_cccUpdate_t *)pValue;

  // Verify input parameters
  if ( pValue == NULL )
  {
    return;
  }

  // If the callback function is not NULL, notify the application
  if ( cgmp_appCB && cgmp_appCB->pfnCGMCPOnCccUpdateCB )
  {
    cgmp_appCB->pfnCGMCPOnCccUpdateCB( cccUpdate->connHandle, cccUpdate->value );
  }
}

/*********************************************************************
 * @fn      CGMP_sstUpdateCB
 *
 * @brief   Callback from CGM Server indicating session start time value
 *          has been updated
 *
 * @param   pValue - pointer to session start time ID
 *
 * @return  None
 */
static void CGMP_sstUpdateCB( char *pValue )
{
  // Verify input parameters
  if ( pValue != NULL)
  {
    // If the callback function is not NULL, notify the application
    if ( cgmp_appCB && cgmp_appCB->pfnSSTUpdateCB )
    {
      cgmp_appCB->pfnSSTUpdateCB( ( CGMS_sst_t *)pValue );
    }
  }
}
/*********************************************************************
 * @fn      CGMP_racpReqCB
 *
 * @brief   Callback from CGM Server indicating RACP request has been update
 *          for Record Access Control Point characteristic
 *
 * @param   pValue - pointer to RACP request
 *
 * @return  None
 */
static void CGMP_racpReqCB( char *pValue)
{
  CGMS_racpReq_t *racpReq = (CGMS_racpReq_t *)pValue;

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
      // Request for all CGM records
      if (racpReq->operator == RACP_OPERATOR_ALL_RECORDS )
      {
        if ( racpReq->FilterType != CGMS_RACP_OPERAND_NULL )
        {
          // Invalid operand
          CGMP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                            RACP_RSP_INVALID_OPERAND, 0);
        }
        else if ( CGM_DB_getCurrentNumOfRecords() > 0 )
        {
          // Send all stored CGM recordS over BLE notifications
          CGMP_sendRecords( CGM_DB_getMinTimeOffset(), cgm_curTimeOffset );

          // Check if the RACP process was aborted
          if ( !(cgmp_racpProc & RACP_PROC_ABORT) )
          {
            // The RACP procedure complete successfully
            CGMP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                              RACP_RSP_SUCCESS, 0 );
          }

          cgmp_racpProc = RACP_PROC_NON;
          return;
        }
        else
        {
          // There are no stored recodes, send response over BLE indication
          CGMP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                            RACP_RSP_NO_RECORDS_FOUND, 0 );
        }
      }

      // Request for all CGM records that are greater than or equal to requested time offset
      else if ( racpReq->operator == RACP_OPERATOR_GREATER_THAN_OR_EQUAL_TO )
      {
          // only Filter Type 0x01 (Time Offset) is allowed to be used.
          if ( racpReq->FilterType == CGMS_RACP_OPERAND_TIME_OFFSET )
          {
             if ( CGM_DB_getNumOfRecordsGreaterOrEqual( racpReq->minimum ) > 0 )
            {
              // Send all stored CGM recordS over BLE notifications
              CGMP_sendRecords( racpReq->minimum, cgm_curTimeOffset );

              // There are stored records for that given request, send response over BLE indication
              CGMP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                                RACP_RSP_SUCCESS, 0 );

              cgmp_racpProc = RACP_PROC_NON;

            }
            else
            {
              // There are no stored recodes, send response over BLE indication
              CGMP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                                RACP_RSP_NO_RECORDS_FOUND, 0 );
            }
          }
          else
          {
            // Send response over BLE indication
            CGMP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                              RACP_RSP_OPERAND_NOT_SUPPORTED, 0 );
          }
      }

      // The requested operator is invalid or not supported
      else if ( racpReq->operator >= RACP_OPERATOR_INVALID )
      {
        // The requested operator is not supported, send response over BLE indication
        CGMP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                          RACP_RSP_OPERATOR_NOT_SUPPORTED, 0 );
      }
      else
      {
        // The requested operator is invalid, send response over BLE indication
        CGMP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
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
       CGMP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                         RACP_RSP_INVALID_OPERATOR, 0 );
      }

      // If RACP procedure is in progress
      else
      {
        // Send response over BLE indication
        CGMP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                            RACP_RSP_SUCCESS, 0 );
      }
    }
    break;

    /******************************************************/
    /***** Request to send number of stored recodes *******/
    /******************************************************/

    case RACP_OPCODE_REPORT_NUM_OF_STORED_RECORDS:
    {
      // Request for all CGM records
      if ( racpReq->operator == RACP_OPERATOR_ALL_RECORDS )
      {
        // Send response over BLE indication
       CGMP_sendRacpRsp( RACP_OPCODE_NUM_OF_STORED_RECORDS_RSP, RACP_OPERATOR_NULL, 0,
                         0, CGM_DB_getCurrentNumOfRecords() );
      }

      // Request for all CGM records that are greater than or equal to requested time offset
      else if (racpReq->operator == RACP_OPERATOR_GREATER_THAN_OR_EQUAL_TO )
      {
        // only Filter Type 0x01 (Time Offset) is allowed to be used.
        if ( racpReq->FilterType != CGMS_RACP_OPERAND_TIME_OFFSET )
        {
            // Send error response over BLE indication
           CGMP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                             RACP_RSP_OPERAND_NOT_SUPPORTED, 0 );
        }
        else
        {
          // Send response over BLE indication
          CGMP_sendRacpRsp( RACP_OPCODE_NUM_OF_STORED_RECORDS_RSP, RACP_OPERATOR_NULL, 0,
                            0, CGM_DB_getNumOfRecordsGreaterOrEqual( racpReq->minimum ));
        }
      }

      // The requested operator is invalid or not supported
      else if ( racpReq->operator >= RACP_OPERATOR_INVALID )
      {
        // The requested operator is not supported, send response over BLE indication
        CGMP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                          RACP_RSP_OP_CODE_NOT_SUPPORTED, 0 );
      }
      else
      {
        // The requested operator is invalid, send response over BLE indication
        CGMP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                          RACP_RSP_INVALID_OPERATOR, 0 );
      }
    }
    break;

    // The requested OpCode is not supported
    default:
    {
      // The requested operator is not supported, send response over BLE indication
      CGMP_sendRacpRsp( RACP_OPCODE_RSP_CODE, RACP_OPERATOR_NULL, racpReq->opCode,
                        RACP_RSP_OP_CODE_NOT_SUPPORTED, 0 );
    }
    break;
  }
}

/*********************************************************************
 * @fn      CGMP_sendRacpRsp
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
static void CGMP_sendRacpRsp( uint8 opCode, uint8 operator, uint8 reqOpCode,
                              uint8 rspCodeValue, uint16 numOfRecords )
{
  CGMS_racpRsp_t racpRsp;

  racpRsp.opCode = opCode;
  racpRsp.operator = operator;
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
  CGMS_setParameter( CGMS_RACP_ID, &racpRsp, CGMS_RACP_RSP_LEN );
}

/*********************************************************************
 * @fn      CGMP_cgmcpReqCB
 *
 * @brief   Callback from CGM Server indicating CGM Specific Ops Control Point
 *          request has been update
 *          for Record Access Control Point characteristic
 *
 * @param   pValue - pointer to RACP request
 *
 * @return  none
 */
static void CGMP_cgmcpReqCB( char *pValue)
{
  CGMS_cgmcpReq_t *cgmcpReq = ( CGMS_cgmcpReq_t * )pValue;

  // Verify input parameters
  if ( pValue == NULL )
  {
    return;
  }

  // Find out the request opCode
  switch ( cgmcpReq->opCode )
  {
    /******************************************************/
    /**** Request to Set CGM communication interval *******/
    /******************************************************/

    case CGMCP_OPCODE_SET_COMMUNICATION_INTERVAL:
    {
      // Request to disable the periodic communication
      if (cgmcpReq->opCode == CGMCP_COMM_INTERVAL_NON)
      {
        // Check if the value of communication interval changed
        if (cgmp_cgmcpCommInterval != CGMS_CGMCP_COMM_INT_VAL_NON )
        {
          // Update the value of the communication interval
          cgmp_cgmcpCommInterval = CGMS_CGMCP_COMM_INT_VAL_NON;

          // Stop periodic communication
          Util_stopClock(&cgmp_CommIntClk);
        }

        // Send response
        CGMP_sendCgmcpRsp( CGMCP_OPCODE_RSP_CODE, cgmcpReq->opCode, CGMCP_RSP_SUCCESS );
      }

      // Request to change the communication interval time to the fastest interval
      else if (cgmcpReq->opCode == CGMCP_COMM_INTERVAL_FASTEST)
      {
        // Check if the value of communication interval changed
        if (cgmp_cgmcpCommInterval != CGMS_CGMCP_COMM_INT_VAL_FASTEST )
        {
          // Update the value of the communication interval
          cgmp_cgmcpCommInterval = CGMS_CGMCP_COMM_INT_VAL_FASTEST;

          // Update periodic communication time
          Util_rescheduleClock( &cgmp_CommIntClk, cgmp_cgmcpCommInterval * CGMP_MIM_TO_MSEC );
          Util_startClock ( &cgmp_CommIntClk );
         }

        // Send response
        CGMP_sendCgmcpRsp( CGMCP_OPCODE_RSP_CODE, cgmcpReq->opCode, CGMCP_RSP_SUCCESS );
      }

      // Request to change the communication interval time
      else
      {
        // Check if the value of communication interval changed
        if (cgmp_cgmcpCommInterval != cgmcpReq->operand )
        {
          // Update the value of the communication interval
          cgmp_cgmcpCommInterval = cgmcpReq->operand;

          // Update periodic communication time
          Util_rescheduleClock( &cgmp_CommIntClk, cgmp_cgmcpCommInterval * CGMP_MIM_TO_MSEC );
          Util_startClock ( &cgmp_CommIntClk );
        }

        // Send response
        CGMP_sendCgmcpRsp( CGMCP_OPCODE_RSP_CODE, cgmcpReq->opCode, CGMCP_RSP_SUCCESS );
      }
    }
    break;

    /******************************************************/
    /**** Request to Get CGM communication interval *******/
    /******************************************************/

    case CGMCP_OPCODE_GET_COMMUNICATION_INTERVAL:
    {
      // Send the current communication interval
      uint8 cgmcpRsp[2];

      cgmcpRsp[0] = CGMCP_OPCODE_COMMUNICATION_INTERVAL_RSP;
      cgmcpRsp[1] = cgmp_cgmcpCommInterval;

      // Send CGMCP response over BLE indication
      CGMS_setParameter( CGMS_CGMCP_ID, &cgmcpRsp, CGMS_CGMCP_RSP_COMM_INT_LEN );
    }
    break;
  }
  return;
}

/*********************************************************************
 * @fn      CGMP_sendCgmcpRsp
 *
 * @brief   Send CGMCP response over BLE indications
 *
 * @param   opCode - CGMCP response opcode
 * @param   reqOpCode - RACP request opcode
 * @param   rspCodeValue - RACP response code value
 *
 * @return  none
 */
static void CGMP_sendCgmcpRsp( uint8 opCode, uint8 reqOpCode, uint8 rspCodeValue )
{
  CGMS_cgmcpRsp_t cgmcpRsp;

  cgmcpRsp.opCode = opCode;
  cgmcpRsp.reqOpCode = reqOpCode;
  cgmcpRsp.rspCodeValue = rspCodeValue;

  // Send RACP response over BLE indication
  CGMS_setParameter( CGMS_CGMCP_ID, &cgmcpRsp, CGMS_CGMCP_RSP_GENERAL_LEN );
}

/*********************************************************************
 * @fn      CGMP_sendRecords
 *
 * @brief   Send CGM measurement records with corresponding time offset
 *          using GATT notifications
 *
 * @param   minTimeOffset - minimum vale of time offset field
 * @param   maxTimeOffset - maximum vale of time offset field
 *
 * @return  none
 */
static void CGMP_sendRecords( uint16 minTimeOffset, uint16 maxTimeOffset )
{
  CGMS_measRecord_t *measRecord = NULL;
  uint8 pBuf[CGMS_MEAS_MAX_RECORD_LEN] = {0};
  uint16 i = 0;
  uint8 len = 0;

  // Mark RACP procedure as in progress
  cgmp_racpProc |= RACP_PROC_INPROGRESS;

  // Find all CGM measurement records with corresponding time offset
  for ( i = minTimeOffset; i <= maxTimeOffset; i++ )
  {
    if ( cgmp_racpProc & RACP_PROC_ABORT )
    {
      return;
    }

    // Set measurement record length to minimum length
    len = CGMS_MEAS_MIN_RECORD_LEN;

    // Check if the record with time offset i exist in the data base
    if ( CGM_DB_getMeasRecord( i, &measRecord ) == SUCCESS )
    {
      // Allocate buffer with the actual size of the CGM measurement record
      VOID memcpy( pBuf, measRecord, len );

      if ( measRecord->flags & CGMS_MEAS_FLAGS_WARNING_PRESENT)
      {
        VOID memcpy( pBuf + len, &measRecord->status.warning, sizeof( measRecord->status.warning ));
        len++;
      }

      if (measRecord->flags & CGMS_MEAS_FLAGS_CALIB_TEMP_PRESENT)
      {
        VOID memcpy( pBuf + len, &measRecord->status.calibTemp, sizeof( measRecord->status.calibTemp ));
        len++;
      }

      if (measRecord->flags & CGMS_MEAS_FLAGS_STATUS_PRESENT)
      {
        VOID memcpy( pBuf + len, &measRecord->status.status, sizeof( measRecord->status.status ));
        len++;
      }

      CGMS_setParameter( CGMS_MEAS_ID, pBuf, len);
    }
  }
}

/*********************************************************************
 * @fn      CGMP_CommIntClkTimeout
 *
 * @brief    This function is triggered when the clock expires
 *
 * @param    pData - pointer to data
 *
 * @return  SUCCESS or stack call status
 */
static void CGMP_CommIntClkTimeout( char *pData)
{
  // Currently not supported
}

/*
 * @fn      CGMP_clockCB
 *
 * @brief   Callback function for clock module
 *
 * @param   arg - argument passed to callback function.
 *
 * @return  none
 */
#ifdef FREERTOS
void CGMP_clockCB( uint32_t arg )
#else
void CGMP_clockCB( uintptr_t arg )
#endif // FREERTOS
{
  BLEAppUtil_invokeFunctionNoData((InvokeFromBLEAppUtilContext_t)arg);
}

/*********************************************************************
*********************************************************************/
