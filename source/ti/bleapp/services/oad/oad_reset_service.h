/******************************************************************************

@file  oad_reset_service.h

 @brief This file contains the OAD profile GATT reset service, including
        service table, GATT callback and API.

Group: WCS, BTS
Target Device: cc13xx_cc26xx

******************************************************************************

 Copyright (c) 2022-2025, Texas Instruments Incorporated
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

#ifndef OAD_RESET_SERVICE_H
#define OAD_RESET_SERVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "hal_types.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*!
 * Stores information related to OAD reset write event
 */
typedef struct
{
  uint16 connHandle;
  uint8  cmd;
} oadResetWrite_t;

//CB from service to profile
typedef void (*oadServiceCB_t)(char* pData);
/*********************************************************************
 * API FUNCTIONS
 */

/*********************************************************************
 * @fn      OadReset_AddService
 *
 * @brief   Initializes the OAD reset service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   pfnOadServiceCB - Callback function from service
 *                            to profile after context switch.
 *
 * @return  SUCCESS or INVALIDPARAMETER
 */
extern bStatus_t OadReset_AddService(oadServiceCB_t pfnOadServiceCB);

/*********************************************************************
 * CONSTANTS
 */

/*!
 * 16-bit root of the OAD reset Service UUID.
 * \note the service and chars will use the 128-bit TI base UUID
 */
#define OAD_RESET_SERVICE_UUID          0xFFD0
/*!
 * 16-bit root of the OAD reset Characteristic UUID.
 */
#define OAD_RESET_CHAR_UUID             0xFFD1

/*!
 * Start OAD reset external control command
 * This command is used to tell the target
 * device that need to invalid self header
 * and start reboot
 */
#define OAD_RESET_CMD_START_OAD         0x01

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OAD_RESET_SERVICE_H */
