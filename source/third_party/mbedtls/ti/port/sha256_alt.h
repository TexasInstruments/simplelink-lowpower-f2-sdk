/******************************************************************************
 Copyright (c) 2019-2023, Texas Instruments Incorporated
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

 ******************************************************************************/

/*!
 *  @file       sha256_alt.h
 *
 *  @brief      Hardware accelerated mbedtls sha256 implementation
 *
 *  # General
 *  This alternate sha256 implementation for mbedtls is intended to provide
 *  hardware acceleration on any SimpleLink platform that has implemented the
 *  SHA2 driver APIs.
 *
 *  The implementation is based on the SHA2 driver and thus
 *  also requires any link-time dependencies it has.
 *
 *  # Limitations
 *  The replaced functions may only be called from Task context when using an
 *  operating system. The driver instances are set up to block until they
 *  acquire the mutex protecting the hardware from concurrent accesses. Blocking
 *  is only permitted in Task context. The upside of this is that none of the
 *  calls will return an error because they were unable to access the hardware
 *  immediately. The downside is of course that the functions may not be called
 *  from Hwi or Swi context.
 */

#ifndef MBEDTLS_SHA256_ALT_H
#define MBEDTLS_SHA256_ALT_H

#include "mbedtls/build_info.h"

#if defined(MBEDTLS_SHA256_ALT)
#ifdef __cplusplus
extern "C" {
#endif

#include <ti/devices/DeviceFamily.h>
#include <ti/drivers/SHA2.h>

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
#include <ti/drivers/sha2/SHA2CC26X2.h>
   typedef SHA2CC26X2_Object SHA2_Object;
   typedef SHA2CC26X2_HWAttrs SHA2_HWAttrs;
#else
   #error "No valid DeviceFamily found for the SHA2 alternate implementation!"
#endif

/**
 * \brief          SHA-256 context structure
 */
typedef struct
{
    SHA2_Handle handle; /*!< A handle that is returned by the SHA driver  */
    SHA2_Config config; /*!< structure containing SHA2 driver specific implementation  */
    SHA2_Object object; /*!< Pointer to a driver specific data object */
} mbedtls_sha256_context;

#endif /* MBEDTLS_SHA256_ALT */

#endif /* MBEDTLS_SHA256_ALT_H */
