/*
 * Copyright (c) 2023-2024, Texas Instruments Incorporated
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
 *  @file       CANMCAN.h
 *
 *  @brief      Common CAN functions for MCAN configuration
 *******************************************************************************
 */

#ifndef ti_drivers_can_common_canmcan__include
#define ti_drivers_can_common_canmcan__include

#include <stdint.h>
#include <ti/drivers/CAN.h>
#include <third_party/mcan/MCAN.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! Interrupt mask for all Rx sources (Rx FIFO 0, Rx FIFO 1, and dedicated Rx
 *  buffers).
 */
#define MCAN_INT_SRC_RX_MASK                                                             \
    ((uint32_t)MCAN_INT_SRC_RX_FIFO0_NEW_MSG | (uint32_t)MCAN_INT_SRC_RX_FIFO1_NEW_MSG | \
     (uint32_t)MCAN_INT_SRC_DEDICATED_RX_BUFF_MSG)

/*!
 *  @brief  Sets the MCAN raw bit timing.
 *
 *  @param  rawTiming    A pointer to CAN_BitRateTimingRaw.
 *
 *  @retval CAN_STATUS_SUCCESS if successful.
 *  @retval CAN_STATUS_ERROR if any timing values are invalid.
 */
int_fast16_t CANMCAN_setBitTimingRaw(const CAN_BitRateTimingRaw *rawTiming);

/*!
 *  @brief  Configures the MCAN message RAM
 *
 *  Configures the MCAN message RAM. If CAN FD is enabled, buffers are configured
 *  to support a max payload size of 64-bytes. Otherwise, the buffers are
 *  configured to support a max payload size of 8-bytes for classic CAN.
 *
 *  @param  config       A pointer to CAN_MsgRamConfig.
 *  @param  msgRamSize   Size of the message RAM in bytes.
 *  @param  enableCANFD  Set to true if CAN FD is enabled, false otherwise.
 *
 *  @retval CAN_STATUS_SUCCESS if successful.
 *  @retval CAN_STATUS_ERROR if the message RAM config is invalid.
 */
int_fast16_t CANMCAN_configMsgRam(const CAN_MsgRamConfig *config, uint32_t msgRamSize, bool enableCANFD);

/*!
 *  @brief  Returns the MCAN interrupt mask based on the CAN event mask.
 *
 *  @param  eventMask    CAN event mask.
 *
 *  @return MCAN interrupt mask
 */
uint32_t CANMCAN_getInterruptMask(uint32_t eventMask);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_can_common_canmcan__include */