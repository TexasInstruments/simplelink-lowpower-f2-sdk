/******************************************************************************

 @file  temp_service.h

 This file contains Temperature Service header file.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2009-2025, Texas Instruments Incorporated
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

#ifndef _TEMPERATURE_H_
#define _TEMPERATURE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "sensor_common.h"

/*********************************************************************
* CONSTANTS
*/

// Service UUID
#define TEMPERATURE_SERV_UUID 0xAA00

//  Characteristic defines
#define TEMPERATURE_DATA_ID   0
#define TEMPERATURE_DATA_UUID 0xAA01
#define TEMPERATURE_DATA_LEN  4

//  Characteristic defines
#define TEMPERATURE_CONFIGURATION_ID   1
#define TEMPERATURE_CONFIGURATION_UUID 0xAA02
#define TEMPERATURE_CONFIGURATION_LEN  1

//  Characteristic defines
#define TEMPERATURE_PERIOD_ID   2
#define TEMPERATURE_PERIOD_UUID 0xAA03
#define TEMPERATURE_PERIOD_LEN  1

/*********************************************************************
 * API FUNCTIONS
 */

/**
 * Initializes the Temperature service by registering GATT attributes with
 * the GATT server
 *
 * @return bleMemAllocError
 * @return SUCCESS
 */
extern bStatus_t Temperature_AddService();

/**
 * Registers the profile callback
 *
 * For LPSTK sensor services, this should be the common sensors profile
 *
 * profileCallbacks - pointer to application callbacks.
 */
extern bStatus_t Temperature_RegisterProfileCBs( sensorServiceCBs_t *profileCallbacks );

/**
 * Set a Temperature Service parameter.
 *
 * @param   param - service parameter ID
 * @param   len   - length of data to right
 * @param   value - pointer to data to write.  This is dependent on
 *                  the parameter ID and WILL be cast to the appropriate
 *                  data type (example: data type of uint16 will be cast to
 *                  uint16 pointer).
 *
 * @return bleInvalidRange invalid length
 * @return INVALIDPARAMETER not a valid param
 * @return SUCCESS
 */
extern bStatus_t Temperature_SetParameter(uint8_t param, uint16_t len, void *value);

/**
 * Get a Temperature Service parameter.
 *
 * @param param service parameter ID
 * @param value pointer to data to write.  This is dependent on the parameter ID
 *        and WILL be cast to the appropriate data type (example: data type
 *        of uint16 will be cast to uint16 pointer)
 *
 * @return INVALIDPARAMETER invalid param
 * @return SUCCESS
 */
extern bStatus_t Temperature_GetParameter(uint8_t param, void *value);

#ifdef __cplusplus
}
#endif

#endif /* _TEMPERATURE_H_ */
