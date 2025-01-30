/******************************************************************************

@file  oad_profile.h

 @brief /////

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

#ifndef OAD_PROFILE_H
#define OAD_PROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */
typedef enum OADProfile_AppCommand_e
{
    OAD_PROFILE_PROCEED,
    OAD_PROFILE_CANCEL
}OADProfile_AppCommand_e;

/*!
 * @OADProfile_App_Msg_e OAD msg from oad profile to app
 * @{
 */
typedef enum OADProfile_App_Msg_e
{
    OAD_PROFILE_MSG_REVOKE_IMG_HDR, //!< Status from profile to app revoke app header
    OAD_PROFILE_MSG_NEW_IMG_IDENDIFY, //!< For off-chip inform app about new image identify
    OAD_PROFILE_MSG_START_DOWNLOAD, //!< Start download new image
    OAD_PROFILE_MSG_FINISH_DOWNLOAD, //!< Dowanload of new image has finished
    OAD_PROFILE_MSG_RESET_REQ, //!< Status from profile to app for get permission to do reset
}OADProfile_App_Msg_e;

typedef OADProfile_AppCommand_e (*OADProfile_AppCallback_t)(OADProfile_App_Msg_e msg);

/*********************************************************************
 * API FUNCTIONS
 */
extern bStatus_t OADProfile_start(OADProfile_AppCallback_t pOADAppCB);

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OAD_PROFILE_H */
