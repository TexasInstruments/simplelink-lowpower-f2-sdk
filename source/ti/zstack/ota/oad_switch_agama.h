/******************************************************************************

 @file oad_switch_agama.h

 @brief Over the Air Download API

 Group: WCS LPC
 Target Device: CC2652

 ******************************************************************************

 Copyright (c) 2016-2017, Texas Instruments Incorporated
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
 Release Name: simplelink_cc26x2_sdk_1_60_00_32_s
 Release Date: 2017-12-19 20:06:40
 *****************************************************************************/
#ifndef OAD_SWITCH_AGAMA_H
#define OAD_SWITCH_AGAMA_H

/******************************************************************************
 Includes
 *****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Typedefs
 *****************************************************************************/

/******************************************************************************
 Public Variables
 *****************************************************************************/

/******************************************************************************
 Function Prototypes
 *****************************************************************************/

/*!
 * @brief marks the external image for switch
 *
 * Sets the metadata for the factory image stored in external flash as ready to
 * run. Then resets the device to initiate the switch.
 *
 * @warn This function will not return if it succeeds
 */
void OAD_markSwitch(void);


#ifdef __cplusplus
}
#endif

#endif /* OAD_SWITCH_AGAMA_H */

