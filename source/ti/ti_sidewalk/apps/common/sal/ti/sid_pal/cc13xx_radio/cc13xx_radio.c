/******************************************************************************

 @file  cc13xx_radio.c

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


#include "cc13xx_radio.h"

#include <sid_pal_delay_ifc.h>
#include <sid_pal_radio_ifc.h>

#include <assert.h>
#include <stddef.h>

static halo_drv_cc13xx_ctx_t drv_ctx = {0};

const halo_drv_cc13xx_ctx_t* cc13xx_get_drv_ctx(void)
{
    return &drv_ctx;
}

void set_radio_cc13xx_device_config(const radio_cc13xx_device_config_t *cfg)
{
    assert(cfg);
    drv_ctx.config = cfg;
}

uint8_t sid_pal_radio_get_status(void)
{
    return drv_ctx.radio_state;
}

sid_pal_radio_modem_mode_t sid_pal_radio_get_modem_mode(void)
{
    return SID_PAL_RADIO_MODEM_MODE_FSK;
}

int32_t sid_pal_radio_set_modem_mode(sid_pal_radio_modem_mode_t mode)
{
    drv_ctx.modem = SID_PAL_RADIO_MODEM_MODE_FSK;
    return RADIO_ERROR_NONE;
}

/*
 * The TI radio driver informs of the protocol by calling the event callback directly and it
 * handles bottom half processing by itself. The protocol calls radio_irq_process after the
 * irq_callback it registers as part of radio_init is called for TI, the irq_callback is not
 * called and it directly reports the radio event
 *
 * Update - this flow is not working from protocol perspective and so the callback from radio
 * is treated as the HW IRQ from semtech radio. This must be optimized later.
 */
int32_t sid_pal_radio_irq_process(void)
{
    cc13xx_radio_process();
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_frequency(uint32_t freq)
{
    int32_t err = RADIO_ERROR_NONE;

    if (cc13xx_set_rf_frequency(freq) != CC13XX_STATUS_OK) {
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
    }

    drv_ctx.radio_freq_hz = freq;

ret:
    return err;
}

// TODO: Revisit this when protocol is implemented for TI
int32_t sid_pal_radio_get_max_tx_power(sid_pal_radio_data_rate_t data_rate, int8_t *tx_power)
{
    if (data_rate <= SID_PAL_RADIO_DATA_RATE_22KBPS || data_rate > SID_PAL_RADIO_DATA_RATE_MAX_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    *tx_power = drv_ctx.regional_radio_param.max_tx_power;

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_tx_power(int8_t power)
{
    int32_t err = RADIO_ERROR_NONE;
    int8_t pwr = power;

    if (drv_ctx.regional_radio_param.max_tx_power < pwr) {
        pwr = drv_ctx.regional_radio_param.max_tx_power;
    }

    if (drv_ctx.config->id == TI_ID_CC13x2P) {
        if (pwr > TX_PWR_PLUS_20) {
            pwr = TX_PWR_PLUS_20;
        }
    } else if ((drv_ctx.config->id == TI_ID_CC13x2R) || (drv_ctx.config->id == TI_ID_CC13x0)) {
        if (pwr > TX_PWR_PLUS_14) {
            pwr = TX_PWR_PLUS_14;
        }
    } else {
        err = RADIO_ERROR_NOT_SUPPORTED;
        goto ret;
    }

    if (cc13xx_set_tx_power(pwr) != CC13XX_STATUS_OK) {
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
    }

ret:
    return err;
}

int32_t sid_pal_radio_sleep(uint32_t sleep_ms)
{
    int32_t err = RADIO_ERROR_NONE;

    if (cc13xx_set_sleep() != CC13XX_STATUS_OK) {
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
    }

    drv_ctx.radio_state = SID_PAL_RADIO_SLEEP;

ret:
    return err;
}

int32_t sid_pal_radio_standby(void)
{
    int32_t err = RADIO_ERROR_NONE;

    if (cc13xx_set_standby() != CC13XX_STATUS_OK) {
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
    }

    drv_ctx.radio_state = SID_PAL_RADIO_STANDBY;

ret:
    return err;
}

int32_t sid_pal_radio_start_carrier_sense(uint32_t timeout, sid_pal_radio_cad_param_exit_mode_t exit_mode)
{
    int32_t err = RADIO_ERROR_NONE;

    if (cc13xx_radio_start_carrier_sense(timeout, exit_mode) != CC13XX_STATUS_OK) {
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
    }

    drv_ctx.radio_state = SID_PAL_RADIO_CAD;

ret:
    return err;
}

int32_t sid_pal_radio_set_tx_payload(const uint8_t *buffer, uint8_t size)
{
    int32_t err = RADIO_ERROR_NONE;

    if (buffer == NULL || size == 0) {
        err = RADIO_ERROR_INVALID_PARAMS;
        goto ret;
    }

    if (cc13xx_set_tx_payload(buffer, size) != CC13XX_STATUS_OK) {
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
    }

ret:
    return err;
}

int32_t sid_pal_radio_start_tx(uint32_t timeout)
{
    int32_t err = RADIO_ERROR_NONE;

    if (cc13xx_radio_start_tx(timeout) != CC13XX_STATUS_OK) {
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
    }

    drv_ctx.radio_state = SID_PAL_RADIO_TX;

ret:
    return err;
}

int32_t sid_pal_radio_set_tx_continuous_wave(uint32_t freq, int8_t power)
{
    int32_t err = RADIO_ERROR_NONE;

    if ((err = sid_pal_radio_set_frequency(freq)) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if ((err = sid_pal_radio_set_tx_power(power)) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if (cc13xx_set_tx_cw() != CC13XX_STATUS_OK) {
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
    }

    drv_ctx.radio_state = SID_PAL_RADIO_TX;

ret:
    return err;
}

int32_t sid_pal_radio_start_rx(uint32_t timeout)
{
    int32_t err = RADIO_ERROR_NONE;

    if (cc13xx_radio_start_rx(timeout, false) != CC13XX_STATUS_OK) {
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
    }

    drv_ctx.radio_state = SID_PAL_RADIO_RX;

ret:
    return err;
}

int32_t sid_pal_radio_start_continuous_rx(void)
{
    int32_t err = RADIO_ERROR_NONE;

    if (cc13xx_radio_start_rx(0, true) != CC13XX_STATUS_OK) {
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
    }

    drv_ctx.radio_state = SID_PAL_RADIO_RX;

    ret:
        return err;
}

int32_t sid_pal_radio_set_rx_duty_cycle(uint32_t rx_time, uint32_t sleep_time)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

int16_t sid_pal_radio_rssi(void)
{
    int16_t rssi = 0;
    if (cc13xx_get_rssi_inst(&rssi) != CC13XX_STATUS_OK) {
        return INT16_MAX;
    }
    return rssi;
}

int32_t sid_pal_radio_is_channel_free(uint32_t freq, int16_t threshold, uint32_t delay_us,
                              bool *is_channel_free)
{
    int32_t err = RADIO_ERROR_NONE;
    uint32_t t_cur = 0;
    int16_t rssi = 0;

    *is_channel_free = true;

    if ((err = sid_pal_radio_set_frequency(freq)) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if ((err = sid_pal_radio_start_continuous_rx()) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if (delay_us < MIN_CHANNEL_FREE_DELAY_US) {
        delay_us = MIN_CHANNEL_FREE_DELAY_US;
    }

    do {
        sid_pal_delay_us(MIN_CHANNEL_FREE_DELAY_US);
        rssi = sid_pal_radio_rssi();

        t_cur += MIN_CHANNEL_FREE_DELAY_US;

        if (rssi > threshold) {
            *is_channel_free = false;
            break;
        }
    } while(delay_us > t_cur);

ret:
    return err;
}

int32_t sid_pal_radio_random(uint32_t *random)
{
    int32_t err = RADIO_ERROR_NONE;

    *random = UINT32_MAX;

    if ((err = sid_pal_radio_start_continuous_rx()) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if (cc13xx_get_random_numbers(random, 1) != CC13XX_STATUS_OK) {
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
    }

ret:
    return err;
}

int16_t sid_pal_radio_get_ant_dbi(void)
{
    return drv_ctx.regional_radio_param.ant_dbi;
}

int32_t sid_pal_radio_get_cca_level_adjust(sid_pal_radio_data_rate_t data_rate, int8_t *adj_level)
{
    if (data_rate <= SID_PAL_RADIO_DATA_RATE_INVALID || data_rate > SID_PAL_RADIO_DATA_RATE_MAX_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    *adj_level = drv_ctx.regional_radio_param.cca_level_adjust;
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_get_chan_noise(uint32_t freq, int16_t *noise)
{
    int32_t err = RADIO_ERROR_NONE;

    *noise = 0;

    if ((err = sid_pal_radio_set_frequency(freq)) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if ((err = sid_pal_radio_start_continuous_rx()) != RADIO_ERROR_NONE) {
        goto ret;
    }

    for (uint8_t i = 0; i < NOISE_SAMPLE_SIZE; i++) {
        *noise += sid_pal_radio_rssi();
        sid_pal_delay_us(MIN_CHANNEL_FREE_DELAY_US);
    }
    *noise /= NOISE_SAMPLE_SIZE;

ret:
    return err;
}

uint32_t sid_pal_radio_get_lora_rx_done_delay(const sid_pal_radio_lora_modulation_params_t *mod_params,
                                            const sid_pal_radio_lora_packet_params_t *pkt_params)
{
    return 0;
}

uint32_t sid_pal_radio_get_lora_tx_process_delay(void)
{
    return 0;
}

uint32_t sid_pal_radio_get_lora_rx_process_delay(void)
{
    return 0;
}

int32_t sid_pal_radio_set_region(sid_pal_radio_region_code_t region)
{
    int32_t err = RADIO_ERROR_NOT_SUPPORTED;

    if (region <= SID_PAL_RADIO_RC_NONE || region >= SID_PAL_RADIO_RC_MAX) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    for (uint8_t i = 0; i < drv_ctx.config->regional_config.reg_param_table_size; i++) {
        if (region == drv_ctx.config->regional_config.reg_param_table[i].param_region) {
            drv_ctx.regional_radio_param = drv_ctx.config->regional_config.reg_param_table[i];
            err = RADIO_ERROR_NONE;
            break;
        }
    }

    return err;
}

int32_t sid_pal_radio_get_radio_state_transition_delays(sid_pal_radio_state_transition_timings_t *state_delay)
{
    *state_delay = drv_ctx.config->state_timings;
    return RADIO_ERROR_NONE;
}

/*
 * Before using the APIs provided by radio hal, radio_init function must be called by protocol.
 * In radio_init, the first parameter notify callback function is provided by protocol to
 * receive the notify event, such as SID_PAL_RADIO_EVENT_RX_DONE, SID_PAL_RADIO_EVENT_TX_DONE events;
 * The seond parameter dio_irq_handler is also provided by protocol to handle RX in SWI context,
 * but TI ignore this function registration, because SDK has already provided RX callback function
 * which is already in SWI context. Here is only stub registered handler to be compatible with all
 * kinds of vendor architecture. The third parameter rx_packet is used to store receiving packet
 * for protocol to retrieve.
 *
 * Update - this flow is not working from protocol perspective and so the callback from radio
 * is treated as the HW IRQ from semtech radio. This must be optimized later.
 *
 * This radio_init function will call cc13xx_platform_init to init TI SDK radio drvier actually.
 * In TI Radio Hal implementaion, cc13xx_platform_init will use TI API RF_open to open the radio
 * instance, and define RX queue for incoming packet.
 *
 * For RX path, as the packets come to TI driver layer, the registered rx_callback function will be
 * called, and the incoming packets are de-queued in this rx_callback function, stored to
 * drv_ctx->radio_rx_packet, and then the event SID_PAL_RADIO_EVENT_RX_DONE will be sent to protocol
 * via drv_ctx->report_radio_event call. When protocol receives the notify event, it can retrieve
 * packet from drv_ctx->radio_rx_packet via sid_phy_get_rx_packet function call.
 * If the RX timeout is configured and it does happen, SID_PAL_RADIO_EVENT_RX_TIMEOUT will be
 * sent to upper layer via drv_ctx->report_radio_event.
 *
 * For continuous RX mode, after setup RX path with callback function, TI drvier can continuous
 * receive packets and rx_callback will be called when packets come. At anytime, a TX need to
 * be performed, because TX handler and RX handler cannot run together, the RX handler need to be
 * canceled first. After TX is done, protocol need to setup continuous RX again if it is required.
 *
 * For TX path, cc13xx_radio_start_tx will be called to actually run packet transmission. It will
 * cancel RX handler to avoid two different operations at the same time cause TI driver hang up,
 * and then post a command to TI drvier to send the packet. After packet is sent out, it will
 * notify upper layer with SID_PAL_RADIO_EVENT_TX_DONE event via drv_ctx->report_radio_event.
 */
int32_t sid_pal_radio_init(sid_pal_radio_event_notify_t notify, sid_pal_radio_irq_handler_t dio_irq_handler,
                   sid_pal_radio_rx_packet_t *rx_packet)
{
    int32_t err = RADIO_ERROR_NONE;

    if (notify == NULL || rx_packet == NULL) {
        err = RADIO_ERROR_INVALID_PARAMS;
        goto ret;
    }

    drv_ctx.radio_rx_packet = rx_packet;
    drv_ctx.report_radio_event = notify;
    drv_ctx.hw_irq_handler = dio_irq_handler;

    sid_pal_radio_set_region(drv_ctx.config->regional_config.radio_region);

    if (cc13xx_platform_init() != CC13XX_STATUS_OK) {
        err = RADIO_ERROR_HARDWARE_ERROR;
        goto ret;
    }

    if ((err = sid_pal_radio_standby()) != RADIO_ERROR_NONE) {
        goto ret;
    }

ret:
    if (err != RADIO_ERROR_NONE) {
        drv_ctx.radio_rx_packet = NULL;
        drv_ctx.report_radio_event = NULL;
    }

    return err;
}
