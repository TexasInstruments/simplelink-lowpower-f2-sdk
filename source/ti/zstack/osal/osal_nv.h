/******************************************************************************

 @file  osal_nv.h

 @brief This module defines the OSAL non-volatile memory functions.

 Group: CMCU, LPRF
 Target Device: CC2652

 ******************************************************************************

 Copyright (c) 2004-2018, Texas Instruments Incorporated
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
 Release Name: simplelink_cc26x2_sdk_1_60_00_43_s
 Release Date: 2018-01-12 15:26:58
 *****************************************************************************/

#ifndef OSAL_NV_H
#define OSAL_NV_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "hal_types.h"

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Initialize NV service
 */
extern void osal_nv_init( void *p );

/*
 * Initialize an item in NV
 */
extern uint8_t osal_nv_item_init( uint16_t id, uint16_t len, void *buf );

/*
 * Read an NV attribute
 */
extern uint8_t osal_nv_read( uint16_t id, uint16_t offset, uint16_t len, void *buf );

/*
 * Write an NV attribute
 */
extern uint8_t osal_nv_write( uint16_t id, uint16_t len, void *buf );

/*
 * Get the length of an NV item.
 */
extern uint16_t osal_nv_item_len( uint16_t id );

/*
 * Delete an NV item.
 */
extern uint8_t osal_nv_delete( uint16_t id, uint16_t len );

/*
 * Initialize an item in NV (extended format)
 */
extern uint8_t osal_nv_item_init_ex( uint16_t id, uint16_t subId, uint16_t len, void *buf );

/*
 * Read an NV attribute (extended format)
 */
extern uint8_t osal_nv_read_ex( uint16_t id, uint16_t subId, uint16_t offset, uint16_t len, void *buf );

/*
 * Search for a specific NV subId
 */
extern uint8_t osal_nv_read_match_entry( uint16_t id, uint16_t *subId, uint16_t offset, uint16_t len, void *buf, uint16_t clen, uint16_t coff, void *cBuf );

/*
 * Write an NV attribute (extended format)
 */
extern uint8_t osal_nv_write_ex( uint16_t id, uint16_t subId, uint16_t len, void *buf );

/*
 * Get the length of an NV item (extended format).
 */
extern uint16_t osal_nv_item_len_ex( uint16_t id, uint16_t subId );

/*
 * Delete an NV item (extended format).
 */
extern uint8_t osal_nv_delete_ex( uint16_t id, uint16_t subId, uint16_t len );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OSAL_NV.H */
