/******************************************************************************

 @file turbo_oad.h

 @brief This file contains the interface of the Turbo OAD delta image
        decoding API

 Group: LPRF SW RND
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2019-2025, Texas Instruments Incorporated
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
#if defined(FEATURE_TOAD)
#ifndef TURBO_OAD_H
#define TURBO_OAD_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ti_154stack_toad_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
@headerfile  turbo_oad.h

@mainpage TOAD API Documentation

Overview
============================

    This file contains the interface for the Turbo Over-the-Air Download (OAD) API.

    The Turbo OAD enables the support of using delta software updates in the OAD
    protocol. Delta updates are software updates that contain only the changed or
    updated content in the new software image. This results on average around
    5x to 10x faster software updates for typical small or medium sized \
    software updates.

    This API is a standalone module that can be integrated into existing OAD
    protocol and applications. A example implementation is provided in the
    15.4 off-chip OAD example application.

Usage
==========================

    This documentation provides a basic usage summary and a set of examples in
    the form of commented code fragments. Detailed descriptions of the APIs are
    provided in subsequent sections.

    Initialization is the first step. This action should be done before the OAD
    block transfer starts.

    Below is a snippet of the 15.4 oad_client.c use of initializing Turbo OAD.

    @code
    TOAD_params_t toadParams;
    TOAD_paramsInit(&toadParams);

    toadParams.oadBlockSize = OAD_BLOCK_SIZE;
    toadParams.pfnWriteDataCb = toadWriteDeltaBlock;
    toadParams.newImgLen = ((OADProtocol_imgIdentifyPld_t*)oadImgIdPld)->newImgLen;

    TOAD_status_t toadStatus = TOAD_Status_initializationError;
    toadStatus = TOAD_init(&toadParams);
    @endcode

    Note that there are at least parameters that should be initialized by the
    application:
        1. OAD block size
        2. Function pointer to a callback function for writing decoded image
           data to flash
        3. New image length

    After initialization, the OAD block transfer can begin. When blocks are
    received by the device, these blocks must be stored in the Turbo OAD
    block cache for future decoding. This is done by calling the
    TOAD_addBlockToCache() function. Depending on the configuration settings
    of the OAD protocol, blocks might be downloaded faster than the device can
    finish decoding. When calling this function, you should check the status
    to ensure that the received block was added successfully.

    The next step is to decode stored OAD blocks. This is handled by the
    TOAD_decode() function. This function will attempt to start decoding, or
    continue decoding a delta block. A delta block contains set of instructions
    and a data payload (a delta block may consist of multiple OAD blocks).

    After calling the TOAD_decode() the state of the Turbo OAD decoding can be
    reterived from the TOAD_getState() function. For more information about
    the Turbo OAD module states, see the TOAD_state_t definition.

    For more information about the Turbo OAD API and its usage, see the TI 15.4
    Stack user guide and the off-chip OAD example project.

 *****************************************************************************/

/******************************************************************************
 Typedefs
 *****************************************************************************/

/* @brief Turbo OAD callback function for writing decoded data to flash */
typedef void (*TOAD_writeDataCb_t)(uint8_t* pData, uint32_t len);

/* @brief Turbo OAD module configuration struct */
typedef struct {
    uint8_t* pAppImg;
    uint32_t oadBlockSize;
    uint32_t oadBlockCacheSize;
    uint32_t writeBufferSize;
    uint32_t newImgLen;

    TOAD_writeDataCb_t pfnWriteDataCb;
} TOAD_params_t;

/* @brief Turbo OAD module states:
 * - TOAD_State_uninitialized:   Starting state before the Turbo OAD module has
 *                               been initialized
 * - TOAD_State_idle:            The module is waiting for new data to be
 *                               received in the block cache
 * - TOAD_State_decoding:        The module is currently decoding a delta block
 * - TOAD_State_decodingWaiting: The module is either currently decoding a
 *                               delta block, or has it about to begin decoding
 *                               the next block. However, not enough data is
 *                               available in the block cache to continue
 *                               decoding.
 * - TOAD_State_decodingDone:    Completion of decoding the delta image
 */
typedef enum {
    TOAD_State_uninitialized,
    TOAD_State_idle,
    TOAD_State_decoding,
    TOAD_State_decodingWaiting,
    TOAD_State_decodingDone,
} TOAD_state_t;

/* @brief Turbo OAD module error statuses */
typedef enum {
    TOAD_Status_success,
    TOAD_Status_initializationError,
    TOAD_Status_noMemory,
    TOAD_Status_noMagic,
    TOAD_Status_decodingError,
} TOAD_status_t;

/******************************************************************************
 Function Prototypes
 *****************************************************************************/

/*********************************************************************
 * @fn          TOAD_init
 *
 * @brief       Initialize the TOAD module. This function must be called
 *                  before any other TOAD functions.
 *
 * @param       params - Pointer to a TOAD_params_t struct
 *
 * @return      TOAD_Status_success if the module was initialized successfully.
 *              Alternatively, this can return TOAD_Status_noMemory error
 *              if memory is available on the heap to initialize the module or
 *              TOAD_Status_initializationError if the parameter structure
 *              was improperly initialized.
 */
TOAD_status_t TOAD_init(TOAD_params_t* params);

/*********************************************************************
 * @fn          TOAD_paramsInit
 *
 * @brief       Initialize a TOAD_params_t struct to its default state
 *
 * @param       params - Pointer to a TOAD_params_t struct
 */
void TOAD_paramsInit(TOAD_params_t* params);

/*********************************************************************
 * @fn          TOAD_close
 *
 * @brief       Close the TOAD module and release all resources and memory.
 */
void TOAD_close();

/*********************************************************************
 * @fn          TOAD_addBlockToCache
 *
 * @brief       Stores an oad block to the internal block cache for decoding
 *
 * @param       pData - Block data to store in the cache
 *
 * @return      TOAD_Status_success if block was successfully
 *              added, or TOAD_Status_noMemory if no memory is
 *              available in the cache
 */
TOAD_status_t TOAD_addBlockToCache(uint8_t* pData);

/*********************************************************************
 * @fn          TOAD_isBlockCacheFull
 *
 * @brief       Checks if the internal OAD block cache is full and
 *              cannot add another OAD block
 *
 * @return      True if there is no room to fit an additional full
 *              block in the cache. False otherwise.
 */
bool TOAD_isBlockCacheFull();

/*********************************************************************
 * @fn          TOAD_isBlockCacheEmpty
 *
 * @brief       Checks if the internal OAD block cache is empty
 *
 * @return      True if the internal OAD block cache is empty.
 *              False otherwise.
 */
bool TOAD_isBlockCacheEmpty();

/*********************************************************************
 * @fn          TOAD_getState
 *
 * @brief       Returns the current state of the TOAD module
 *
 * @return      TOAD_state_t representing the current state of the
 *              TOAD module.
 */
TOAD_state_t TOAD_getState();

/*********************************************************************
 * @fn          TOAD_decode
 *
 * @brief       Attempts to start or continue decoding the next delta block
 *              in the OAD block cache and write decoded data to flash
 *
 * @return      TOAD_Status_success if there were no issues in trying
 *              to decode the next block. Otherwise TOAD_Status_decodingError.
 */
TOAD_status_t TOAD_decode();

#ifdef __cplusplus
}
#endif

#endif /* TURBO_OAD_H */
#endif /* FEATURE_TOAD */
