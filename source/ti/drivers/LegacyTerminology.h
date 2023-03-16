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
/*!*****************************************************************************
 *  @file       LegacyTerminology.h
 *
 *  @brief      Provide a translation layer for legacy terminology
 *
 * # How to Enable Backwards Compatibility
 * Backwards-compatibility for legacy terminology is disabled by default. To
 * enable the use of legacy terminology, add ENABLE_LEGACY_TERMINOLOGY to your
 * application's compiler arguments.
 *
 * # Overview #
 * TI is transitioning to use more inclusive terminology. As a result some
 * language and legacy APIs may be different than what you would expect to see
 * for certain technology areas. The purpose of this file is to ease porting for
 * customers who have software on existing platforms still using prior
 * terminology while moving TI defined software to follow the latest industry
 * driven standards.
 *
 * @warning Using this header may inadvertedly redefine parts of your
 * application code if that code uses any of the legacy terms as substrings.
 */

#warning "Backwards-compatibility for legacy terminology will be removed in a \
future release. Please see the release notes for more information."

/* Settings to support SPI.h */
#define SPI_MASTER SPI_CONTROLLER
#define SPI_SLAVE  SPI_PERIPHERAL

/* Settings to support I2S.h */
#define I2S_MASTER  I2S_CONTROLLER
#define I2S_SLAVE   I2S_TARGET
#define MCLKDivider CCLKDivider

/* Settings to support I2C.h */
#define slaveAddress targetAddress

/* Settings to support PDMCC26XX_util.h. Since this driver is not
 * SysConfig-enabled, we are providing legacy names for the changes therein.
 */
#define mclkPin       cclkPin
#define mclkDiv       cclkDiv
#define enableMclkPin enableCclkPin