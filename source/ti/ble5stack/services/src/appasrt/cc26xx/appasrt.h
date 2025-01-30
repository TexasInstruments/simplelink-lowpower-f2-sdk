/******************************************************************************

 @file  appasrt.h

 @brief Application assert function definitions.

 Group: WCS, LPC, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2015-2025, Texas Instruments Incorporated
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
#ifndef APPASRT_H
#define APPASRT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "appasrtintf.h"

//*****************************************************************************
// Functions
//*****************************************************************************

/**
 *
 * @brief   Loads the applicaiton assert handler to function pointer for use
 *          by the stack.
 *
 * @param[in]   pAssrtHnldr : Pointer to assert handler function on app side.
 * @param[out]  None.
 *
 * @return  none
 */
extern void APPASRT_loadAssertHndlr(APPASRTINTF_assrtHndlerFunc_t *pAssrtHnldr);

/**
 * @fn       APPASRT_callAssrtHndler
 *
 * @brief   Calls the application assert handler.
 *
 * @param[in]   None.
 * @param[out]  None.
 *
 * @return  none
 */
extern void APPASRT_callAssrtHndler(void);

//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* APPASRT_H */

