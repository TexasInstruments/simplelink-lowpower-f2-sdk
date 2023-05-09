/******************************************************************************

 @file  sid_pal_spi_client.c

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
 * conditions set forth in the accompanying LICENSE.TXT file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sid_pal_serial_client_ifc.h>
#include <sid_pal_spi_client.h>

#include <sid_utils.h>
#include <sid_error.h>
#include <sid_pal_log_ifc.h>
#include <sid_pal_timer_ifc.h>
#include <sid_time_ops.h>
#include <sid_pal_uptime_ifc.h>
#include <sid_pal_assert_ifc.h>

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti_drivers_config.h>

#include "include/sid_pal_spi_client.h"

#define NEXT_BUFFER_INDEX(current_index) ((current_index) ^ 1)

struct double_buffer {
    uint8_t *double_buffer[2];
    uint8_t buffer_index;
};

struct received_packet {
    size_t size;
    uint8_t buffer_index;
};

struct impl {
    sid_pal_serial_ifc_t ifc;                       /**< External interface. */
    const sid_pal_serial_callbacks_t *callbacks;    /**< Externally defined callbacks. */
    void *cb_ctx;                                   /**< Context to pass in callbacks. */
    const struct sid_pal_spi_config *config;        /**< This module configuration */
    bool frame_transmission_in_progress;            /**< Means that new buffer for transmission
                                                       * was set and interrupt was asserted. */
    struct double_buffer tx;                        /**< Double buffer's data for transmission. Will be usefull after HALO-25926 */
    struct double_buffer rx;                        /**< Double buffer's data for reception.  Will be usefull after HALO-25926 */
    struct received_packet received_frame;          /**< Specific data for received frame. */
    sid_pal_timer_t interrupt_timer;                /**< Timer instance for handling timeout
                                                    * on asserted interrupt. */
    SPI_Handle spis;                                /**< A #SPI_Handle returned from SPI_open()*/
    SPI_Transaction transaction;                    /**< Pointer to a #SPI_Transaction */
};

static struct impl impl = { 0 };


/* ---------------- Interrupt handling functions ------------------- */
static void interrupt_assertion_timeout_cb(void *ctx, sid_pal_timer_t *originator)
{
    (void)originator;

    struct impl *impl = ctx;

    GPIO_write(impl->config->interrupt2host_pin, 1);
}

static void deassert_interrupt_to_host(struct impl *impl)
{
    sid_pal_timer_cancel(&impl->interrupt_timer);
    GPIO_write(impl->config->interrupt2host_pin, 1);
}

static void assert_interrupt_to_host(struct impl *impl)
{
    struct sid_timespec now = {};
    struct sid_timespec assertion_timout = impl->config->interrupt_assertion_timeout;

    if (sid_pal_timer_is_armed(&impl->interrupt_timer) == true) {
       return;
    }

    sid_pal_uptime_now(&now);
    sid_time_add(&assertion_timout, &now);
    if (sid_time_gt(&now, &assertion_timout) == false) {
        /* Timeout of assertion has value bigger than current time so it will happen in future. */
        sid_pal_timer_arm(&impl->interrupt_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE, &assertion_timout, NULL);
        GPIO_write(impl->config->interrupt2host_pin, 0);
    }
}
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* -------------------------- SPIS driver callback ------------------------- */
static void spis_transfer_complete(SPI_Handle handle, SPI_Transaction *transaction)
{
    if (transaction->status == SPI_TRANSFER_COMPLETED) {
        if (impl.frame_transmission_in_progress == true) {
            deassert_interrupt_to_host(&impl);
            impl.frame_transmission_in_progress = false;
            impl.callbacks->tx_done_cb(impl.cb_ctx);
        }

        if (transaction->count != 0) {
            if (transaction->count <= impl.config->mtu) {
                impl.received_frame.size = transaction->count;
                impl.received_frame.buffer_index = impl.rx.buffer_index;
                impl.rx.buffer_index = NEXT_BUFFER_INDEX(impl.rx.buffer_index);
                impl.callbacks->new_rx_done_cb(impl.cb_ctx);
            } else {
                SID_PAL_LOG_ERROR("SPI received packet of size %d exceeds MTU = %d", transaction->count, impl.config->mtu);
            }
        }
        transaction->count = impl.config->mtu;
        transaction->txBuf = NULL;
        transaction->rxBuf = (void *) impl.rx.double_buffer[impl.rx.buffer_index];
        bool transfer_ok = SPI_transfer(handle, transaction);
        SID_PAL_ASSERT(transfer_ok);
    }
}
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/* --------------- Serial Client Interface implementation  -------------- */
static sid_error_t send(sid_pal_serial_ifc_t const *_this, const uint8_t *frame, size_t size)
{
    if (_this == NULL || frame == NULL) {
        return SID_ERROR_NULL_POINTER;
    }

    struct impl *impl = containerof(_this, struct impl, ifc);

    if (size == 0 || size > impl->config->mtu) {
        return SID_ERROR_INVALID_ARGS;
    }

    if (impl->frame_transmission_in_progress == true) {
        return SID_ERROR_BUSY;
    }

    impl->tx.buffer_index = NEXT_BUFFER_INDEX(impl->tx.buffer_index);
    impl->frame_transmission_in_progress = true;
    memset(impl->tx.double_buffer[impl->tx.buffer_index], impl->config->default_tx_char, impl->config->mtu);
    memcpy(impl->tx.double_buffer[impl->tx.buffer_index], frame, size);
    SPI_transferCancel(impl->spis);

    impl->transaction.count = impl->config->mtu;
    impl->transaction.txBuf = (void *)impl->tx.double_buffer[impl->tx.buffer_index];
    impl->transaction.rxBuf = (void *)impl->rx.double_buffer[impl->rx.buffer_index];

    bool transfer_ok = SPI_transfer(impl->spis, &impl->transaction);
    SID_PAL_ASSERT(transfer_ok);

    assert_interrupt_to_host(impl);
    return SID_ERROR_NONE;
}

static sid_error_t get_frame(sid_pal_serial_ifc_t const *_this, uint8_t **frame, size_t *size)
{
    if (_this == NULL || frame == NULL || size == NULL) {
        return SID_ERROR_NULL_POINTER;
    }

    struct impl *impl = containerof(_this, struct impl, ifc);

    if (impl->received_frame.size == 0) {
        /* Nothing to fetch. No frame has been received. */
        return SID_ERROR_GENERIC;
    }

    *frame = impl->rx.double_buffer[impl->received_frame.buffer_index];
    *size = impl->received_frame.size;

    impl->received_frame.size = 0;

    return SID_ERROR_NONE;
}

static sid_error_t get_mtu(sid_pal_serial_ifc_t const *_this, uint16_t *mtu)
{
    if (_this == NULL || mtu == NULL) {
        return SID_ERROR_NULL_POINTER;
    }

    struct impl *impl = containerof(_this, struct impl, ifc);

    *mtu = impl->config->mtu;
    return SID_ERROR_NONE;
}

static void destroy_impl(struct impl *impl)
{
    for (uint8_t buffer_index = 0; buffer_index < 2; buffer_index++) {
        if (impl->tx.double_buffer[buffer_index] != NULL) {
            free(impl->tx.double_buffer[buffer_index]);
            impl->tx.double_buffer[buffer_index] = NULL;
        }
        if (impl->rx.double_buffer[buffer_index] != NULL) {
            free(impl->rx.double_buffer[buffer_index]);
            impl->rx.double_buffer[buffer_index] = NULL;
        }
    }
}

static void destroy(sid_pal_serial_ifc_t const *_this)
{
    if (_this == NULL) {
        return;
    }

    SPI_transferCancel(impl.spis);
    destroy_impl(containerof(_this, struct impl, ifc));
}

const static struct sid_pal_serial_ifc_s methods_table = {
    .send = send,
    .get_frame = get_frame,
    .get_mtu = get_mtu,
    .destroy = destroy,
};
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

sid_error_t sid_pal_spi_client_create(sid_pal_serial_ifc_t const **_this, const void *config, sid_pal_serial_params_t const *params)
{
    if (_this == NULL || config == NULL || params == NULL) {
        return SID_ERROR_NULL_POINTER;
    }
    const struct sid_pal_spi_config *spi_cfg = config;

    sid_error_t rc = SID_ERROR_NONE;

    for (uint8_t buffer_index = 0; buffer_index < 2; buffer_index++) {
        if ((impl.tx.double_buffer[buffer_index] = calloc(1, spi_cfg->mtu)) == NULL ||
            (impl.rx.double_buffer[buffer_index] = calloc(1, spi_cfg->mtu)) == NULL ) {
            rc = SID_ERROR_OOM;
            goto error;
        }
    }

    impl.rx.buffer_index = 0;
    impl.tx.buffer_index = 0;

    impl.ifc = &methods_table;
    impl.callbacks = params->callbacks;
    impl.cb_ctx = params->user_ctx;
    impl.config = spi_cfg;

    SPI_Params      spi_params;
    bool            transfer_ok;

    SPI_init();
    SPI_Params_init(&spi_params);

    spi_params.frameFormat =  spi_cfg->frame_format;
    spi_params.mode = spi_cfg->mode;
    spi_params.transferCallbackFxn = spi_cfg->transfer_mode == SPI_MODE_CALLBACK ? spis_transfer_complete : NULL;
    spi_params.bitRate = spi_cfg->bitrate;
    spi_params.transferMode = spi_cfg->transfer_mode;

    impl.spis = SPI_open(CONFIG_SPI_0, &spi_params);
    GPIO_setConfig(spi_cfg->interrupt2host_pin, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH);
    GPIO_write(spi_cfg->interrupt2host_pin, 1);
    sid_pal_timer_init(&impl.interrupt_timer, interrupt_assertion_timeout_cb, &impl);

    *_this = &impl.ifc;
exit:
    return rc;
error:
    destroy_impl(&impl);
    goto exit;
}
