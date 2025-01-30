/******************************************************************************

 @file  glucose_db.c

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

#include <ti/bleapp/profiles/glucose/glucose_db.h>

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// GL Measurements Data Base Array Of All Records
static GLS_measRecord_t               gl_db_measRecordsArray[GL_DB_MAX_NUM_OF_RECORDS];

// gl Current Number Of Measurements
static GLS_DB_recordsInfo_entry_t     gl_db_recordsInfo;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */


/*********************************************************************
 * @fn      GL_DB_initDataBase
 *
 * @brief   This function initializes the gl's measurements records
 *          data base variables and array.
 *
 * @param   samplingInterval - Current Measurements Interval.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t GL_DB_initDataBase(void)
{
    uint32 i = 0;
    bStatus_t status = SUCCESS;

    // Go over all records in the data base.
    for (i = 0; i < GL_DB_MAX_NUM_OF_RECORDS; i++)
    {
        // Update the record to be 0
        gl_db_measRecordsArray[i].flags                               = 0;
        gl_db_measRecordsArray[i].SequenceNum                         = 0;
        gl_db_measRecordsArray[i].timeBase.year                       = 0;
        gl_db_measRecordsArray[i].timeBase.month                      = 0;
        gl_db_measRecordsArray[i].timeBase.day                        = 0;
        gl_db_measRecordsArray[i].timeBase.hours                      = 0;
        gl_db_measRecordsArray[i].timeBase.minutes                    = 0;
        gl_db_measRecordsArray[i].timeBase.seconds                    = 0;
        gl_db_measRecordsArray[i].TimeOffset                          = 0;
        gl_db_measRecordsArray[i].glucoseConcen                       = 0;
        gl_db_measRecordsArray[i].Type_SampleLocation.SampleLocation  = 0;
        gl_db_measRecordsArray[i].Type_SampleLocation.Type            = 0;
        gl_db_measRecordsArray[i].SensorStatus                        = 0;
    }

    // Reset the records info variables.
    gl_db_recordsInfo.measDBMaxNumRecords = 0;
    gl_db_recordsInfo.measDBHead          = 0;
    gl_db_recordsInfo.measDBTail          = 0;

    return status;
}

/*********************************************************************
 * @fn      GL_DB_isFull
 *
 * @brief   This function returns if the data base is full or not.
 *
 * @param   None.
 *
 * @return  TRUE in case of DB full, FALSE in case of DB not full.
 */
static bStatus_t GL_DB_isFull(void)
{
    return (gl_db_recordsInfo.measDBMaxNumRecords == GL_DB_MAX_NUM_OF_RECORDS);
}

/*********************************************************************
 * @fn      GL_DB_getCurrentNumOfRecords
 *
 * @brief   This function gets the current number of measurements records
 *          in the data base.
 *
 * @param   None.
 *
 * @return  GL_DB_NumOfRecords Value
 */
uint16 GL_DB_getCurrentNumOfRecords(void)
{
    return gl_db_recordsInfo.measDBMaxNumRecords;
}// End of GL_DB_getCurrentNumOfRecords().

/*********************************************************************
 * @fn      GL_DB_getNumOfRecordsGreaterOrEqual
 *
 * @brief   This function gets the number of measurements records
 *          in the data base that greater or equal than the sequence number.
 *
 * @param   None.
 *
 * @return  GL_DB_NumOfRecords ValueGREATER
 */

uint16 GL_DB_getNumOfRecordsGreaterOrEqual( uint16 SequenceNum)
{
    uint16 numOfRecords = 0;
    uint16 HeadSequenceNum = 0;
    uint16 TailSequenceNum = 0;

    HeadSequenceNum = GL_DB_getFirstSequenceNum();
    TailSequenceNum = GL_DB_getLastSequenceNum();

    // Check valid input.
    // In case the measRecordNum is NOT in between the head index and tail index record number or the array is empty,
    // return ERROR
    if ((SequenceNum > TailSequenceNum) || (SequenceNum < HeadSequenceNum) || (GL_DB_getCurrentNumOfRecords() == 0))
    {
        return numOfRecords;
    }
    numOfRecords = TailSequenceNum - SequenceNum + 1;

    return numOfRecords;
}// End of GL_DB_getCurrentNumOfRecords().

/*********************************************************************
 * @fn      GL_DB_advanceIndexes
 *
 * @brief   This function advances the head and tail of the DB array.
 *
 * @param   None.
 *
 * @return  SUCCESS or stack call status.
 */
static bStatus_t GL_DB_advanceIndexes(void)
{
    bStatus_t status = SUCCESS;

    // Check if after the increment of the tail index we have reached the max number of records.
    if (++(gl_db_recordsInfo.measDBTail) == GL_DB_MAX_NUM_OF_RECORDS)
    {
        // Wraparound.
        gl_db_recordsInfo.measDBTail = 0;
    }

    // Increment the head only in case we are full.
    if (GL_DB_isFull())
    {
        if (++(gl_db_recordsInfo.measDBHead) == GL_DB_MAX_NUM_OF_RECORDS)
        {
            gl_db_recordsInfo.measDBHead = 0;
        }
    }
    // Increment the number of records in case we are not full
    else
    {
        gl_db_recordsInfo.measDBMaxNumRecords++;
    }

    return (status);

}// End of GL_DB_AdvanceIndexes().

/*********************************************************************
 * @fn      GL_DB_addMeasRecord
 *
 * @brief   This function add a measurement record to the data base.
 *
 *
 * @param   pMeasRecord - pointer to the record's structure info.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t GL_DB_addMeasRecord( GLS_measRecord_t *pMeasRecord )
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
        memcpy( gl_db_measRecordsArray + gl_db_recordsInfo.measDBTail, pMeasRecord, GLS_MEAS_MAX_RECORD_LEN);
        //GL_DB_MeasRecordsArray[GL_DB_RecordsInfo.measDBTail] = *pMeasRecord;

        // Advance the tail and head of the DB Array.
        // In case the DB is full both head and tail are incremented.
        // In case the DB is not full only the tail is incremented and the max
        // number of records.
        GL_DB_advanceIndexes();

     }// End of else.

    return status;
}// End of GL_DB_AddMeasRecord().

/*********************************************************************
 * @fn      GL_DB_getMeasRecord
 *
 * @brief   This function gets a measurement record (single) from the data base.
 *          Notice that the records number needs to be in the current records range.
 *
 * @param   SequenceNum    - the time offset of the wanted record to extract.
 * @param   pMeasRecord   - pointer to the record's structure info that
 *                          would be filled.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t GL_DB_getMeasRecord(uint16 SequenceNum, GLS_measRecord_t **pMeasRecord)
{
    bStatus_t status = SUCCESS;
    uint16 SequenceIndex;
    uint16 HeadSequenceNum;
    uint16 TailSequenceNum;

    HeadSequenceNum = GL_DB_getFirstSequenceNum();
    TailSequenceNum = GL_DB_getLastSequenceNum();

    // Check valid input.
    // In case the measRecordNum is NOT in between the head index and tail index record number or the array is empty,
    // return ERROR
    if ((SequenceNum > TailSequenceNum) || (SequenceNum < HeadSequenceNum) || (GL_DB_getCurrentNumOfRecords() == 0))
    {
        status = INVALIDPARAMETER;
    }
    else
    {
      status = SUCCESS;
      //difference between head and sequenceNum
      SequenceIndex = (gl_db_recordsInfo.measDBHead + (SequenceNum - gl_db_measRecordsArray[gl_db_recordsInfo.measDBHead].SequenceNum)) % GL_DB_MAX_NUM_OF_RECORDS;
      // Copy the record's info from the measurements array
      *pMeasRecord = gl_db_measRecordsArray + SequenceIndex;
    }

    return status;
}// End of GL_DB_GetMeasRecord().


/*********************************************************************
 * @fn      GL_DB_getLastSequenceNum
 *
 * @brief   This function returns the newest sequence number in the data base
 *
 * @param   None.
 *
 * @return  GL_DB_NumOfRecords ValueGREATER
 */

uint16 GL_DB_getLastSequenceNum()
{
    uint16 seqNum = 0;

    if ( (GL_DB_isFull()) && (gl_db_recordsInfo.measDBTail == 0))
    {
        seqNum = gl_db_measRecordsArray[GL_DB_MAX_NUM_OF_RECORDS-1].SequenceNum;
    }
    else if (gl_db_recordsInfo.measDBTail == 0)
    {
        seqNum = gl_db_measRecordsArray[(gl_db_recordsInfo.measDBTail)].SequenceNum;
    }
    else
    {
        seqNum = gl_db_measRecordsArray[(gl_db_recordsInfo.measDBTail-1)].SequenceNum;
    }
    return seqNum;
}// End of GL_DB_getCurrentNumOfRecords().

/*********************************************************************
 * @fn      GL_DB_getFirstSequenceNum
 *
 * @brief   This function returns the oldest sequence number in the data base
 *
 * @param   None.
 *
 * @return  GL_DB_NumOfRecords ValueGREATER
 */

uint16 GL_DB_getFirstSequenceNum()
{
  return gl_db_measRecordsArray[(gl_db_recordsInfo.measDBHead)].SequenceNum;
}// End of GL_DB_getCurrentNumOfRecords().


/*********************************************************************
*********************************************************************/
