/******************************************************************************

 @file  cc13xx_radio_fsk.c

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

/*
 * Modulation parameters are plan to remove in HALO-5123.
 * Packet parameters are under discussion to remove or support in the future.
 * So part of functions are place holders here, and will be well handled after discussion.
 */
#include "cc13xx_radio.h"

#include <stddef.h>
#include <string.h>

/*
 * TODO: HALO-5123
 * to remove modulation parameters support in radio hal
 */
static void radio_mp_to_cc13xx_mp(cc13xx_mod_params_gfsk_t *fsk_mp,
                                  const sid_pal_radio_fsk_modulation_params_t *mod_params)
{
    fsk_mp->br_in_bps    = mod_params->bit_rate;
    fsk_mp->fdev_in_hz   = mod_params->freq_dev;
    fsk_mp->bw_dsb_param = (cc13xx_gfsk_bw_t)mod_params->bandwidth;
    fsk_mp->mod_shape    = (cc13xx_gfsk_mod_shapes_t)mod_params->mod_shaping;
}

/*
 * This function is for time on the air computation and for sync word length and preamble
 * length modification.
 */
static void radio_pp_to_cc13xx_pp(cc13xx_pkt_params_gfsk_t *fsk_pp,
                                  const sid_pal_radio_fsk_packet_params_t *packet_params)
{
    fsk_pp->pbl_len_in_bits = (packet_params->preamble_length > 1) ?
        ((packet_params->preamble_length - 1) << 3) : 0;
    fsk_pp->pbl_min_det           = (cc13xx_gfsk_pbl_det_t)packet_params->preamble_min_detect;
    fsk_pp->sync_word_len_in_bits = packet_params->sync_word_length << 3;
    fsk_pp->addr_cmp              = (cc13xx_gfsk_addr_cmp_t)packet_params->addr_comp;
    fsk_pp->hdr_type              = (cc13xx_gfsk_pkt_len_modes_t)packet_params->header_type;
    fsk_pp->pld_len_in_bytes      = packet_params->payload_length;
    fsk_pp->crc_type              = (cc13xx_gfsk_crc_types_t)packet_params->crc_type;
    fsk_pp->dc_free               = (cc13xx_gfsk_dc_free_t)packet_params->radio_whitening_mode;
}

int32_t radio_fsk_process_rx_done(halo_drv_cc13xx_ctx_t *drv_ctx)
{
    sid_pal_radio_rx_packet_t       *radio_rx_packet      = drv_ctx->radio_rx_packet;
    sid_pal_radio_fsk_rx_packet_status_t  *fsk_rx_packet_status = &radio_rx_packet->fsk_rx_packet_status;
    sid_pal_radio_fsk_phy_hdr_t          phy_hdr               = {0};
    int32_t                      err                   = RADIO_ERROR_NONE;
    uint8_t                      *buffer               = radio_rx_packet->rcv_payload;
    uint8_t                      phr[PHR_LEN_IN_BYTES] = {0};
    int8_t                       rssi                  = 0;
    static int8_t                rssi_avg              = 0;

    radio_rx_packet->payload_len = cc13xx_get_rx_entry(phr, buffer, &rssi);

    if (radio_rx_packet->payload_len <= 0) {
        radio_rx_packet->payload_len = 0;
        err = RADIO_ERROR_GENERIC;
        goto ret;
    }

    phy_hdr.fcs_type = phr[CC13XX_PHR_TYPE_OFFSET] >> PHR_FIELD_FCS_TYPE_SHIFT;
    phy_hdr.is_data_whitening_enabled = (phr[CC13XX_PHR_TYPE_OFFSET] &
                                         (1 << PHR_FIELD_WHITEN_SHIFT)) ? true : false;

    rssi_avg = (rssi_avg == 0) ? rssi : (rssi_avg + rssi) / 2;
    fsk_rx_packet_status->rx_status = rssi - (CC13XX_RF_NOISE_FLOOR);
    fsk_rx_packet_status->rssi_sync = rssi;
    fsk_rx_packet_status->rssi_avg  = rssi_avg;

ret:
    return err;
}

sid_pal_radio_data_rate_t sid_pal_radio_fsk_mod_params_to_data_rate(const sid_pal_radio_fsk_modulation_params_t *mp)
{
    uint8_t data_rate = SID_PAL_RADIO_DATA_RATE_INVALID;

    /* The modulation params are not yet finalized and is tracked in JIRA-5059.
     * These changes here will unblock the testing for GEN2 lights.
     */
    if (mp->bit_rate == RADIO_FSK_BR_50KBPS && mp->freq_dev == RADIO_FSK_FDEV_25KHZ &&
        mp->bandwidth == (uint8_t)CC13XX_GFSK_BW_156200) {
        data_rate = SID_PAL_RADIO_DATA_RATE_50KBPS;
    } else if (mp->bit_rate == RADIO_FSK_BR_150KBPS && mp->freq_dev == RADIO_FSK_FDEV_37_5KHZ &&
               mp->bandwidth == (uint8_t)CC13XX_GFSK_BW_312000) {
        data_rate = SID_PAL_RADIO_DATA_RATE_150KBPS;
    } else if (mp->bit_rate == RADIO_FSK_BR_250KBPS && mp->freq_dev == RADIO_FSK_FDEV_62_5KHZ &&
               mp->bandwidth == (uint8_t)CC13XX_GFSK_BW_467000) {
        data_rate = SID_PAL_RADIO_DATA_RATE_250KBPS;
    }

    return data_rate;
}

int32_t sid_pal_radio_fsk_data_rate_to_mod_params(sid_pal_radio_fsk_modulation_params_t *mod_params,
                                          sid_pal_radio_data_rate_t data_rate)
{
    if (mod_params == NULL) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    switch (data_rate) {
    case SID_PAL_RADIO_DATA_RATE_50KBPS:
        mod_params->bit_rate    = RADIO_FSK_BR_50KBPS;
        mod_params->freq_dev    = RADIO_FSK_FDEV_25KHZ;
        mod_params->bandwidth   = (uint8_t)CC13XX_GFSK_BW_156200;
        mod_params->mod_shaping = (uint8_t)CC13XX_GFSK_MOD_SHAPE_BT_1;
        break;
    case SID_PAL_RADIO_DATA_RATE_150KBPS:
        mod_params->bit_rate    = RADIO_FSK_BR_150KBPS;
        mod_params->freq_dev    = RADIO_FSK_FDEV_37_5KHZ;
        mod_params->bandwidth   = (uint8_t)CC13XX_GFSK_BW_312000;
        mod_params->mod_shaping = (uint8_t)CC13XX_GFSK_MOD_SHAPE_BT_05;
        break;
    case SID_PAL_RADIO_DATA_RATE_250KBPS:
        mod_params->bit_rate    = RADIO_FSK_BR_250KBPS;
        mod_params->freq_dev    = RADIO_FSK_FDEV_62_5KHZ;
        mod_params->bandwidth   = (uint8_t)CC13XX_GFSK_BW_467000;
        mod_params->mod_shaping = (uint8_t)CC13XX_GFSK_MOD_SHAPE_BT_05;
        break;
    default:
        return RADIO_ERROR_INVALID_PARAMS;
    }

    return RADIO_ERROR_NONE;
}

/*
 * ringnet-phy will call this function to give default value to packet_params,
 * and then call radio_set_fsk_packet_params and radio_set_fsk_sync_word in this
 * file. If the parameters are fixed, we can decide to remove them.
 * This API lists here is just a place holder for future.
 */
int32_t sid_pal_radio_prepare_fsk_for_rx(sid_pal_radio_fsk_pkt_cfg_t *rx_pkt_cfg)
{
    int32_t err = RADIO_ERROR_INVALID_PARAMS;

    if (rx_pkt_cfg == NULL) {
        goto ret;
    }

    if (rx_pkt_cfg->phy_hdr == NULL || rx_pkt_cfg->packet_params == NULL ||
        rx_pkt_cfg->sync_word == NULL) {
        goto ret;
    }

    sid_pal_radio_fsk_packet_params_t *f_pp = rx_pkt_cfg->packet_params;
    sid_pal_radio_fsk_phy_hdr_t *phr       = rx_pkt_cfg->phy_hdr;
    uint8_t *sync_word             = rx_pkt_cfg->sync_word;
    uint8_t sync_word_length_in_byte = 0;

    sync_word[sync_word_length_in_byte++] = 0x55;
    sync_word[sync_word_length_in_byte++] = phr->is_fec_enabled ? 0x6F : 0x90;
    sync_word[sync_word_length_in_byte++] = 0x4E;

    f_pp->preamble_min_detect      = (uint8_t)CC13XX_GFSK_PBL_DET_16_BITS;
    f_pp->sync_word_length         = sync_word_length_in_byte;
    f_pp->addr_comp                = (uint8_t)CC13XX_GFSK_ADDR_CMP_FILT_OFF;
    f_pp->header_type              = (uint8_t)CC13XX_GFSK_PKT_FIX_LEN;
    f_pp->payload_length           = CC13XX_FSK_MAX_PAYLOAD_LEN;
    f_pp->crc_type                 = (uint8_t)CC13XX_GFSK_CRC_OFF;
    f_pp->radio_whitening_mode     = (uint8_t)CC13XX_GFSK_DC_FREE_OFF;
    err = RADIO_ERROR_NONE;

ret:
    return err;
}

int32_t sid_pal_radio_prepare_fsk_for_tx(sid_pal_radio_fsk_pkt_cfg_t *tx_pkt_cfg)
{
    int32_t err = RADIO_ERROR_INVALID_PARAMS;

    if (tx_pkt_cfg == NULL) {
        goto ret;
    }

    if (tx_pkt_cfg->phy_hdr == NULL || tx_pkt_cfg->packet_params == NULL ||
        tx_pkt_cfg->sync_word == NULL || tx_pkt_cfg->payload == NULL) {
        goto ret;
    }

    sid_pal_radio_fsk_packet_params_t *f_pp = tx_pkt_cfg->packet_params;
    if (f_pp->payload_length == 0 || f_pp->preamble_length == 0) {
        goto ret;
    }

    sid_pal_radio_fsk_phy_hdr_t *phr = tx_pkt_cfg->phy_hdr;
    if (((phr->fcs_type == RADIO_FSK_FCS_TYPE_0) &&
         (f_pp->payload_length > MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_0)) ||
        ((phr->fcs_type == RADIO_FSK_FCS_TYPE_1) &&
         (f_pp->payload_length > MAX_PAYLOAD_LENGTH_WITH_FCS_TYPE_1))) {
        goto ret;
    }

    uint8_t  psdu_length                = f_pp->payload_length;
    uint8_t  *buffer_ptr                = tx_pkt_cfg->payload;
    uint8_t  *sync_word                 = tx_pkt_cfg->sync_word;
    uint8_t  sync_word_length_in_byte   = 0;

    sync_word[sync_word_length_in_byte++] = 0x55;
    sync_word[sync_word_length_in_byte++] = (phr->is_fec_enabled == true) ? 0x6F : 0x90;
    sync_word[sync_word_length_in_byte++] = 0x4E;

    f_pp->preamble_min_detect  = (uint8_t)CC13XX_GFSK_PBL_DET_08_BITS;
    f_pp->sync_word_length     = sync_word_length_in_byte;
    f_pp->addr_comp            = (uint8_t)CC13XX_GFSK_ADDR_CMP_FILT_OFF;
    f_pp->header_type          = (uint8_t)CC13XX_GFSK_PKT_FIX_LEN;
    f_pp->payload_length       = PHR_LEN_IN_BYTES + psdu_length;
    f_pp->crc_type             = (uint8_t)CC13XX_GFSK_CRC_OFF;
    f_pp->radio_whitening_mode = (uint8_t)CC13XX_GFSK_DC_FREE_OFF;

    buffer_ptr[CC13XX_PHR_TYPE_OFFSET] = (phr->fcs_type << PHR_FIELD_FCS_TYPE_SHIFT);
    buffer_ptr[CC13XX_PHR_TYPE_OFFSET] += ((phr->is_data_whitening_enabled == true) ?
                                          1 : 0) << PHR_FIELD_WHITEN_SHIFT;

    if (phr->fcs_type == RADIO_FSK_FCS_TYPE_0) {
        psdu_length += PHR_FCS_LEN_4_BYTES;
    } else {
        psdu_length += PHR_FCS_LEN_2_BYTES;
    }

    buffer_ptr[CC13XX_PHR_TYPE_OFFSET] += PHR_LENGTH_HBYTE(psdu_length);
    buffer_ptr[CC13XX_PHR_LENGTH_OFFSET] = PHR_LENGTH_LBYTE(psdu_length);

    memcpy(buffer_ptr + PHR_LEN_IN_BYTES, f_pp->payload, f_pp->payload_length);

    err = RADIO_ERROR_NONE;

ret:
    return err;
}

int32_t sid_pal_radio_set_fsk_sync_word(const uint8_t *sync_word, uint8_t sync_word_length)
{
    if (cc13xx_set_gfsk_sync_word(sync_word, sync_word_length) != CC13XX_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }
    return RADIO_ERROR_NONE;
}

/*
 * This API lists here is just a place holder for compilation now.
 */
int32_t sid_pal_radio_set_fsk_whitening_seed(uint16_t seed)
{
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_fsk_modulation_params(const sid_pal_radio_fsk_modulation_params_t *mod_params)
{
    if (mod_params == NULL) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    cc13xx_mod_params_gfsk_t fsk_mp;
    radio_mp_to_cc13xx_mp(&fsk_mp, mod_params);
    if (cc13xx_set_gfsk_mod_params(&fsk_mp) != CC13XX_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_fsk_packet_params(const sid_pal_radio_fsk_packet_params_t *packet_params)
{
    if (packet_params == NULL) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    cc13xx_pkt_params_gfsk_t fsk_pp;
    radio_pp_to_cc13xx_pp(&fsk_pp, packet_params);
    if (cc13xx_set_gfsk_pkt_params(&fsk_pp) != CC13XX_STATUS_OK) {
        return RADIO_ERROR_HARDWARE_ERROR;
    }

    return RADIO_ERROR_NONE;
}

uint32_t sid_pal_radio_fsk_time_on_air(const sid_pal_radio_fsk_modulation_params_t *mod_params,
                              const sid_pal_radio_fsk_packet_params_t *packet_params, uint8_t packetLen)
{
    if (mod_params == NULL || packet_params == NULL) {
        return 0;
    }

    cc13xx_pkt_params_gfsk_t fsk_pp;
    cc13xx_mod_params_gfsk_t fsk_mp;

    radio_mp_to_cc13xx_mp(&fsk_mp, mod_params);

    fsk_pp.pld_len_in_bytes = packetLen;
    radio_pp_to_cc13xx_pp(&fsk_pp, packet_params);

    return cc13xx_get_gfsk_time_on_air_in_ms(&fsk_pp, &fsk_mp);
}

uint32_t sid_pal_radio_fsk_get_fsk_number_of_symbols(const sid_pal_radio_fsk_modulation_params_t *mod_params,
                                             uint32_t delay_micro_secs)
{
    uint32_t num_symb = CC13XX_US_TO_SYMBOLS(delay_micro_secs, mod_params->bit_rate);
    return num_symb;
}
