/******************************************************************************

 @file  ti_cc13x2_crypto_config.h

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************

 Copyright (c) 2023, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************


 *****************************************************************************/

/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include <ti_cc13x2_pal_crypto_config.h>

#ifndef CONFIG_CC13X2_PAL_CRYPTO_TRNG_INSTANCE_ID
#error "missing value for CONFIG_CC13X2_PAL_CRYPTO_TRNG_INSTANCE_ID"
#endif
#ifndef CONFIG_CC13X2_PAL_CRYPTO_SHA2_INSTANCE_ID
#error "missing value for CONFIG_CC13X2_PAL_CRYPTO_SHA2_INSTANCE_ID"
#endif
#ifndef CONFIG_CC13X2_PAL_CRYPTO_AESCMAC_INSTANCE_ID
#error "missing value for CONFIG_CC13X2_PAL_CRYPTO_AESCMAC_INSTANCE_ID"
#endif
#ifndef CONFIG_CC13X2_PAL_CRYPTO_AESCTR_INSTANCE_ID
#error "missing value for CONFIG_CC13X2_PAL_CRYPTO_AESCTR_INSTANCE_ID"
#endif
#ifndef CONFIG_CC13X2_PAL_CRYPTO_AESGCM_INSTANCE_ID
#error "missing value for CONFIG_CC13X2_PAL_CRYPTO_AESGCM_INSTANCE_ID"
#endif
#ifndef CONFIG_CC13X2_PAL_CRYPTO_EDDSA_INSTANCE_ID
#error "missing value for CONFIG_CC13X2_PAL_CRYPTO_EDDSA_INSTANCE_ID"
#endif
#ifndef CONFIG_CC13X2_PAL_CRYPTO_ECDSA_INSTANCE_ID
#error "missing value for CONFIG_CC13X2_PAL_CRYPTO_ECDSA_INSTANCE_ID"
#endif
#ifndef CONFIG_CC13X2_PAL_CRYPTO_ECDH_INSTANCE_ID
#error "missing value for CONFIG_CC13X2_PAL_CRYPTO_ECDH_INSTANCE_ID"
#endif