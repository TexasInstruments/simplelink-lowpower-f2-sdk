/******************************************************************************

 @file oad_switch.c

 @brief Over the Air Download for use with the BLE OAD example

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
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

/******************************************************************************
 Includes
 *****************************************************************************/
#include <string.h>
#include <oad_image_header.h>
#include <oad_image_header_app.h>

/* Low level driverlib files (non-rtos) */
#include <ti/devices/DeviceFamily.h>
#include <ti/drivers/dpl/HwiP.h>
#include DeviceFamily_constructPath(driverlib/flash.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)

/*!
 Marks the running image on internal flash as invalid

 Public function defined in oad.h
 */
void OAD_markSwitch(void)
{
    /*
     * We only need to invalidate the IMAGE header
     * We can do this by writing a CRC_INVALID to the crcStat.
     */

    /* Enter critical section. */
    uint32_t key = HwiP_disable();


    /* create constant on the stack to address */
    uint8_t invalidCrc = CRC_INVALID;
    uint32_t retval = FlashProgram(&invalidCrc, (uint32_t)(&_imgHdr.fixedHdr.crcStat),
                                   sizeof(_imgHdr.fixedHdr.crcStat));

    /* press the virtual reset button */
    SysCtrlSystemReset();

    /*
     * The idea is that execution should never reach this point. If for some
     * reason it does, we re-enable Interupts
     */

    /* Exit critical section. */
    HwiP_restore(key);
}

/******************************************************************************
 Local Functions
 *****************************************************************************/
