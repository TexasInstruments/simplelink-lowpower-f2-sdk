/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** ============================================================================
 *  @file       ANSIX936KDFXX.h
 *
 *  @brief      ANSI-X9.63 Key Derivation Function driver implementation
 *              for the CC13X1/CC26X1, CC13X2/CC26X2, CC13X4/CC26X4, and CC23X0
 *              families
 *
 *  This file should only be included in the board file to fill the
 *  ANSIX936KDF_config struct.
 *
 *  # Supported Hash Type #
 *  SHA-256 is in the only hash type supported.
 *
 *  # Limitations #
 *  - Callback return behavior is not supported.
 *  - For devices which utilize SW-backed SHA2 implementations:
 *    - Blocking return behavior does not yield and operates exactly the same as
 *      polling mode.
 */

#ifndef ti_drivers_ansix936kdf_ANSIX936KDFXX__include
#define ti_drivers_ansix936kdf_ANSIX936KDFXX__include

#include <stdbool.h>

#include <ti/drivers/SHA2.h>

#include <ti/devices/DeviceFamily.h>

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X1_CC26X1)
    #include <ti/drivers/sha2/SHA2CC26X1.h>
#elif ((DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X2_CC26X2) || \
       (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4))
    #include <ti/drivers/sha2/SHA2CC26X2.h>
#elif (DeviceFamily_PARENT == DeviceFamily_PARENT_CC23X0)
    #include <ti/drivers/sha2/SHA2LPF3SW.h>
#else
    #error "ANSIX936KDFXX device family not supported"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @brief  Hardware-specific configuration attributes
 *
 *  ANSIX936KDFXX hardware attributes are used in the board file by the
 *  #ANSIX936KDF_Config struct.
 */
typedef struct
{
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X1_CC26X1)
    SHA2CC26X1_HWAttrs sha2HWAttrs;
#elif ((DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X2_CC26X2) || \
       (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4))
    SHA2CC26X2_HWAttrs sha2HWAttrs;
#elif (DeviceFamily_PARENT == DeviceFamily_PARENT_CC23X0)
    SHA2LPF3SW_HWAttrs sha2HWAttrs;
#endif
} ANSIX936KDFXX_HWAttrs;

/*!
 *  @brief  ANSIX936KDFXX Object
 *
 *  The application must not access any member variables of this structure!
 */
typedef struct
{
    SHA2_Config sha2Config;
    SHA2_Handle sha2Handle;
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X1_CC26X1)
    SHA2CC26X1_Object sha2Object;
#elif ((DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X2_CC26X2) || \
       (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4))
    SHA2CC26X2_Object sha2Object;
#elif (DeviceFamily_PARENT == DeviceFamily_PARENT_CC23X0)
    SHA2LPF3SW_Object sha2Object;
#endif
    bool isOpen;
} ANSIX936KDFXX_Object;

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_ansix936kdf_ANSIX936KDFXX__include */
