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
 *  @file       TCAN455X.h
 *
 *  @brief      TCAN455X Driver Interface
 *
 *  @anchor ti_drivers_TCAN455X_Overview
 *
 *  # Overview
 *  The TCAN455X driver performs the hardware access to the TCAN455x CAN
 *  controller with integrated transceiver via SPI.
 *
 *  ## Message RAM Size
 *  The TCAN455X device has 2KB of message RAM.
 *******************************************************************************
 */

#ifndef ti_drivers_can_tcan455x__include
#define ti_drivers_can_tcan455x__include

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TCAN455X_MRAM_SIZE 2048U

/*!
 *  @brief      TCAN455X global configuration
 */
typedef struct
{
    uint32_t clkFreqMHz;        /*!< TCAN455X input clock frequency in MHz */
    uint32_t spiBitRate;        /*!< SPI bit rate to communicate with TCAN455X */
    void *taskStack;            /*!< Pointer to TCAN455X interrupt handler task stack */
    uint16_t taskStackSize;     /*!< TCAN455X interrupt handler task stack size */
    uint8_t taskPri;            /*!< TCAN455X interrupt handler task priority */
    uint_least8_t spiIndex;     /*!< SPI instance index from Board file */
    uint_least8_t spiCSPin;     /*!< SPI Chip Select pin */
    uint_least8_t interruptPin; /*!< TCAN455X interrupt pin */
} TCAN455X_Config;

/* Externs from ti_drivers_config.c */
extern const TCAN455X_Config TCAN455X_config;

/*!
 *  @brief  Reads the TCAN455X device status flags
 *
 *  @return Device status flags.
 *
 *  @sa     #TCAN455X_clearStatus
 */
uint32_t TCAN455X_getStatus(void);

/*!
 *  @brief  Clears all TCAN455X device status flags
 *
 *  @sa     #TCAN455X_getStatus
 */
void TCAN455X_clearStatus(void);

/*!
 *  @brief  Sets the TCAN455X operational mode
 *
 *  @param  mode  Operational mode to set:
 *                  TCAN455X_MODE_OPMODE_SLEEP,
 *                  TCAN455X_MODE_OPMODE_STANDBY,
 *                  TCAN455X_MODE_OPMODE_NORMAL
 *
 *  @sa     #TCAN455X_getMode
 */
void TCAN455X_setMode(uint32_t mode);

/*!
 *  @brief  Reads the TCAN455X operational mode
 *
 *  @return Operational mode:
 *            TCAN455X_MODE_OPMODE_SLEEP,
 *            TCAN455X_MODE_OPMODE_STANDBY,
 *            TCAN455X_MODE_OPMODE_NORMAL
 *
 *  @sa     #TCAN455X_setMode
 */
uint32_t TCAN455X_getMode(void);

/*!
 *  @brief  Disables the sleep wake error timeout
 *
 *  The sleep wake error timeout is enabled by default and powers down the
 *  TCAN455X device within four minutes after power-on, reset, or coming out of
 *  sleep if the device is not configured by the host. This function can be used
 *  to disable the sleep wake error timeout and prevent the device from powering
 *  down.
 *
 */
void TCAN455X_disableSleepWakeErrorTimeout(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_can_tcan455x__include */
