/******************************************************************************

 @file  simple_gatt_profile.h

 @brief This file contains the Simple GATT profile definitions and prototypes
        prototypes.

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

#ifndef SIMPLEGATTPROFILE_H
#define SIMPLEGATTPROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */
// Profile Parameters
#define SIMPLEGATTPROFILE_CHAR1                   0  // RW uint8 - Profile Characteristic 1 value
#define SIMPLEGATTPROFILE_CHAR2                   1  // RW uint8 - Profile Characteristic 2 value
#define SIMPLEGATTPROFILE_CHAR3                   2  // RW uint8 - Profile Characteristic 3 value
#define SIMPLEGATTPROFILE_CHAR4                   3  // RW uint8 - Profile Characteristic 4 value
#define SIMPLEGATTPROFILE_CHAR5                   4  // RW uint8 - Profile Characteristic 4 value

// Simple Profile Service UUID
#define SIMPLEGATTPROFILE_SERV_UUID               0xFFF0

// Key Pressed UUID
#define SIMPLEGATTPROFILE_CHAR1_UUID            0xFFF1
#define SIMPLEGATTPROFILE_CHAR2_UUID            0xFFF2
#define SIMPLEGATTPROFILE_CHAR3_UUID            0xFFF3
#define SIMPLEGATTPROFILE_CHAR4_UUID            0xFFF4
#define SIMPLEGATTPROFILE_CHAR5_UUID            0xFFF5

// Simple Keys Profile Services bit fields
#define SIMPLEGATTPROFILE_SERVICE               0x00000001

// Length of Characteristic 5 in bytes
#define SIMPLEGATTPROFILE_CHAR5_LEN           5

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */
// Callback when a characteristic value has changed
typedef void (*pfnSimpleGattProfile_Change_t)( uint8 paramID );

typedef struct
{
  pfnSimpleGattProfile_Change_t        pfnSimpleGattProfile_Change;  // Called when characteristic value changes
} SimpleGattProfile_CBs_t;

/*********************************************************************
 * API FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleGattProfile_addService
 *
 * @brief   This function initializes the Simple GATT Server service
 *          by registering GATT attributes with the GATT server.
 *
 * @return  SUCCESS or stack call status
 */
bStatus_t SimpleGattProfile_addService( void );

/*
 * @fn      SimpleGattProfile_registerAppCBs
 *
 * @brief   Registers the application callback function. Only call
 *          this function once.
 *
 * @param   appCallbacks - pointer to application callback.
 *
 * @return  SUCCESS or INVALIDPARAMETER
 */
bStatus_t SimpleGattProfile_registerAppCBs( SimpleGattProfile_CBs_t *appCallbacks );

/*
 * @fn      SimpleGattProfile_setParameter
 *
 * @brief   Set a Simple GATT Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to right
 * @param   value - pointer to data to write.  This is dependent on
 *                  the parameter ID and WILL be cast to the appropriate
 *                  data type (example: data type of uint16 will be cast to
 *                  uint16 pointer).
 *
 * @return  SUCCESS or INVALIDPARAMETER
 */
bStatus_t SimpleGattProfile_setParameter( uint8 param, uint8 len, void *value );

/*
 * @fn      SimpleGattProfile_getParameter
 *
 * @brief   Get a Simple GATT Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to write. This is dependent on
 *                  the parameter ID and WILL be cast to the appropriate
 *                  data type (example: data type of uint16 will be cast to
 *                  uint16 pointer).
 *
 * @return  SUCCESS or INVALIDPARAMETER
 */
bStatus_t SimpleGattProfile_getParameter( uint8 param, void *value );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEGATTPROFILE_H */
