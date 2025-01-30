/******************************************************************************

 @file  cgm_db.c

 @brief This file contains the continues glucose measurement profile data base
        for use with the BLE sample application.

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

#include <ti/bleapp/profiles/continuous_glucose_monitoring/cgm_db.h>

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// CGM Measurements Data Base Array Of All Records
static CGMS_measRecord_t               cgm_db_measRecordsArray[CGM_DB_MAX_NUM_OF_RECORDS];

// CGM Current Number Of Measurements
static CGMS_DB_recordsInfo_entry_t     cgm_db_recordsInfo;

/*********************************************************************
 * GLOBAL VARIABLES
 */

// CGM Current Measurements Interval
uint16 cgm_samplingInterval = 1;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      CGM_DB_getCurrentNumOfRecords
 *
 * @brief   This function gets the current number of measurements records
 *          in the data base.
 *
 * @param   None.
 *
 * @return  cgm_DB_NumOfRecords Value
 */
uint16 CGM_DB_getCurrentNumOfRecords(void)
{
    return cgm_db_recordsInfo.measDBMaxNumRecords;
}// End of CGM_DB_getCurrentNumOfRecords().

/*********************************************************************
 * @fn      CGM_DB_getCurrentNumOfRecords
 *
 * @brief   This function gets the current number of measurements records
 *          in the data base.
 *
 * @param   None.
 *
 * @return  cgm_DB_NumOfRecords ValueGREATER
 */
uint16 CGM_DB_getNumOfRecordsGreaterOrEqual( uint16 timeOffset)
{
    uint16 numOfRecords = 0;
    // The index of the input timeOffset.
    uint16 timeOffsetRecArrayIndex = ((timeOffset / cgm_samplingInterval) % CGM_DB_MAX_NUM_OF_RECORDS);

    if( (timeOffsetRecArrayIndex > cgm_db_recordsInfo.measDBTail) ||
        (timeOffsetRecArrayIndex < cgm_db_recordsInfo.measDBHead) ||
        (CGM_DB_getCurrentNumOfRecords() == 0) )
    {
        return  numOfRecords;
    }
    // Check valid input.
    // In case the measRecordNum is NOT in between the head index and tail index record number or the array is empty,
    // return ERROR
    if (cgm_db_measRecordsArray[timeOffsetRecArrayIndex].timeOffset < timeOffset)
    {
       timeOffsetRecArrayIndex++;
    }

    for (uint16 i = timeOffsetRecArrayIndex; i < cgm_db_recordsInfo.measDBTail; i++)
    {
        numOfRecords++;
    }
    return numOfRecords;
}// End of CGM_DB_getCurrentNumOfRecords().

/*********************************************************************
 * @fn      CGM_DB_getMinTimeOffset
 *
 * @brief   This function gets the current index of the minimum record - Head
 *
 *
 * @param   None.
 *
 * @return  measDBHead time offset Value
 */
uint16 CGM_DB_getMinTimeOffset(void)
{
    return cgm_db_measRecordsArray[cgm_db_recordsInfo.measDBHead].timeOffset;
}// End of CGM_DB_getMinTimeOffset().

/*********************************************************************
 * @fn      CGM_DB_initDataBase
 *
 * @brief   This function initializes the CGM's measurements records
 *          data base variables and array.
 *
 * @param   samplingInterval - Current Measurements Interval.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGM_DB_initDataBase( uint16 samplingInterval )
{
    uint32 i = 0;
    bStatus_t status = SUCCESS;

    // Go over all records in the data base.
    for (i = 0; i < CGM_DB_MAX_NUM_OF_RECORDS; i++)
    {
        // Update the record to be 0
        cgm_db_measRecordsArray[i].flags                = 0;
        cgm_db_measRecordsArray[i].glucoseConcen        = 0;
        cgm_db_measRecordsArray[i].size                 = 0;
        cgm_db_measRecordsArray[i].status.calibTemp     = 0;
        cgm_db_measRecordsArray[i].status.status        = 0;
        cgm_db_measRecordsArray[i].status.warning       = 0;
        cgm_db_measRecordsArray[i].timeOffset           = 0;
    }

    // Reset the records info variables.
    cgm_db_recordsInfo.measDBMaxNumRecords = 0;
    cgm_db_recordsInfo.measDBHead          = 0;
    cgm_db_recordsInfo.measDBTail          = 0;

    // Update measurements Interval
    cgm_samplingInterval = samplingInterval;

    return status;
}// End of CGM_DB_initDataBase().

/*********************************************************************
 * @fn      CGM_DB_isFull
 *
 * @brief   This function returns if the data base is full or not.
 *
 * @param   None.
 *
 * @return  TRUE in case of DB full, FALSE in case of DB not full.
 */
static bStatus_t CGM_DB_isFull(void)
{
    return (cgm_db_recordsInfo.measDBMaxNumRecords == CGM_DB_MAX_NUM_OF_RECORDS);
}// END of CGM_DB_isFull().

/*********************************************************************
 * @fn      CGM_DB_advanceIndexes
 *
 * @brief   This function advances the head and tail of the DB array.
 *
 * @param   None.
 *
 * @return  SUCCESS or stack call status.
 */
static bStatus_t CGM_DB_advanceIndexes(void)
{
    bStatus_t status = SUCCESS;

    // Check if after the increment of the tail index we have reached the max number of records.
    if (++(cgm_db_recordsInfo.measDBTail) == CGM_DB_MAX_NUM_OF_RECORDS)
    {
        // Wraparound.
        cgm_db_recordsInfo.measDBTail = 0;
        cgm_db_recordsInfo.measDBHead = 1;
    }

    // Increment the head only in case we are full.
    if (CGM_DB_isFull())
    {
        if (++(cgm_db_recordsInfo.measDBHead) == CGM_DB_MAX_NUM_OF_RECORDS)
        {
            cgm_db_recordsInfo.measDBHead = 0;
        }
    }
    // Increment the number of records in case we are not full
    else
    {
        cgm_db_recordsInfo.measDBMaxNumRecords++;
    }

    return (status);

}// End of CGM_DB_AdvanceIndexes().

/*********************************************************************
 * @fn      CGM_DB_addMeasRecord
 *
 * @brief   This function add a measurement record to the data base.
 *
 *
 * @param   pMeasRecord - pointer to the record's structure info.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGM_DB_addMeasRecord( CGMS_measRecord_t *pMeasRecord )
{
    bStatus_t status = SUCCESS;

    // Check valid pointer input.
    if (pMeasRecord == NULL)
    {
        status = INVALIDPARAMETER;
    }
    // Check we have enough space to insert the record in the data base.
    // If not, We need to perform an insert in a cyclic manner.
    else
    {
        // Insert the record in the incremented tail.
        memcpy( cgm_db_measRecordsArray + cgm_db_recordsInfo.measDBTail, pMeasRecord, CGMS_MEAS_MAX_RECORD_LEN);
        //cgm_DB_MeasRecordsArray[cgm_DB_RecordsInfo.measDBTail] = *pMeasRecord;

        // Advance the tail and head of the DB Array.
        // In case the DB is full both head and tail are incremented.
        // In case the DB is not full only the tail is incremented and the max
        // number of records.
        CGM_DB_advanceIndexes();

       }// End of else.

    return status;
}// End of CGM_DB_AddMeasRecord().

/*********************************************************************
 * @fn      CGM_DB_getMeasRecord
 *
 * @brief   This function gets a measurement record (single) from the data base.
 *          Notice that the records number needs to be in the current records range.
 *
 * @param   timeOffset    - the time offset of the wanted record to extract.
 * @param   pMeasRecord   - pointer to the record's structure info that
 *                          would be filled.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t CGM_DB_getMeasRecord(uint16 timeOffset, CGMS_measRecord_t **pMeasRecord)
{
    bStatus_t status = SUCCESS;
    // The index of the input timeOffset.
    uint16 timeOffsetRecArrayIndex = ((timeOffset / cgm_samplingInterval) % CGM_DB_MAX_NUM_OF_RECORDS);
    uint16 HeadOffsetNum;
    uint16 TailOffsetNum;
    // if database is in cyclic event
    if(CGM_DB_isFull())
    {
      HeadOffsetNum = cgm_db_measRecordsArray[(cgm_db_recordsInfo.measDBHead-1) % CGM_DB_MAX_NUM_OF_RECORDS].timeOffset;
      TailOffsetNum = cgm_db_measRecordsArray[(cgm_db_recordsInfo.measDBTail-1) % CGM_DB_MAX_NUM_OF_RECORDS].timeOffset;
    }
    else
    {
      HeadOffsetNum = cgm_db_measRecordsArray[cgm_db_recordsInfo.measDBHead].timeOffset;
      TailOffsetNum = cgm_db_measRecordsArray[(cgm_db_recordsInfo.measDBTail-1) % CGM_DB_MAX_NUM_OF_RECORDS].timeOffset;
    }

    // Check valid input.
    // In case the measRecordNum is NOT in between the head index and tail index record number or the array is empty,
    // return ERROR

    if ((timeOffset > TailOffsetNum) || (timeOffset < HeadOffsetNum) ||
        (CGM_DB_getCurrentNumOfRecords() == 0) ||
        (cgm_db_measRecordsArray[timeOffsetRecArrayIndex].timeOffset != timeOffset))
    {
        status = INVALIDPARAMETER;
    }
    else
    {
        // Copy the record's info from the measurements array
        *pMeasRecord = cgm_db_measRecordsArray + timeOffsetRecArrayIndex;
    }

    return status;
}// End of CGM_DB_GetMeasRecord().

/*********************************************************************
*********************************************************************/
