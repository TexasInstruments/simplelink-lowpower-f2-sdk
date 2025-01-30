/******************************************************************************
 *  Copyright (c) 2022-2024, Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1) Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2) Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3) Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/* Note: This file is copied from driverlib and modified to change App VTOR to
 * point to Secure image start.
 */

#ifndef __CCFC_C__
#define __CCFC_C__

#include "region_defs.h"
#include "../inc/hw_ccfg.h"

#define BL2_HEADER_SIZE 0x100

/*! \brief Default ccfg struct definition
 *  Defines the ccfg structure and field configuration with default values.
 *  If fields are to be overridden, the structure below should be modified in
 *  place or copied into a new C file to avoid contaminating the local SDK
 *  installation.
 */
#if defined(__IAR_SYSTEMS_ICC__)
__root const ccfg_t __ccfg @ ".ccfg" =
#elif defined(__llvm__)
    #pragma GCC diagnostic ignored "-Woverride-init"
    #pragma GCC diagnostic ignored "-Wmissing-braces"
const ccfg_t __ccfg __attribute__((section(".ccfg"), retain)) =
#elif (defined(__GNUC__))
    #pragma GCC diagnostic ignored "-Woverride-init"
    #pragma GCC diagnostic ignored "-Wmissing-braces"
const ccfg_t __ccfg __attribute__((section(".ccfg"), used)) =
#else
    #error "Unsupported compiler used. Expected one of [TI Clang, IAR, GCC]"
#endif
    {
        CCFG_DEFAULT_VALUES,
        // Manual overrides
        .bootCfg.pBldrVtor = XCFG_BC_PBLDR_UNDEF,
        .bootCfg.pAppVtor  = (void *)S_CODE_START,
    };

#endif // __CCFC_C__
