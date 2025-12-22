/*
 * Copyright (c) 2018-2019, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "nsconfig.h"
#include <string.h>
#include "ns_types.h"
#include "ns_list.h"
#include "ns_trace.h"
#include "common_functions.h"
#include "mac_common_defines.h"
#include "6LoWPAN/MAC/mac_ie_lib.h"
#include "6LoWPAN/ws/ws_common_defines.h"
#include "6LoWPAN/ws/ws_ie_lib.h"
#include "nsdynmemLIB.h"

#define TRACE_GROUP "wsie"

#ifdef WISUN_FAN_CORE_1_1
extern uint32_t g_num_jm_metrics_len ;
#endif //WISUN_FAN_CORE_1_1

static uint8_t *ws_wh_header_base_write(uint8_t *ptr, uint16_t length, uint8_t type)
{
    ptr = mac_ie_header_base_write(ptr, MAC_HEADER_ASSIGNED_EXTERNAL_ORG_IE_ID, length + 1);
    *ptr++ = type;
    return ptr;
}

static uint16_t ws_channel_plan_length(uint8_t channel_plan)
{
    switch (channel_plan) {
        case 0:
            //Regulator domain and operationg class inline
            return 2;
        case 1:
            //CH0, Channel spacing and number of channel's inline
            //CH0 (24 bit) + channel spacing (4 bit) + number of channels (16 bit)
            return 6;
#ifdef WISUN_RCP_ENABLE//WISUN_FAN_CORE_1_1
        case 2:
            // Regulatory Domain and CHannel plan ID
            return 2;
#endif //WISUN_FAN_CORE_1_1
        default:
            return 0;
    }
}

static uint16_t ws_channel_function_length(uint8_t channel_function, uint16_t hop_channel_count)
{
    switch (channel_function) {
        case 0:
            //Fixed channel inline
            return 2;
        case 1:
        case 2:
            return 0;
        case 3:
            //Hop count + channel hop list
            return (1 + hop_channel_count);
        default:
            return 0;

    }
}

uint16_t ws_wp_nested_hopping_schedule_length(struct ws_hopping_schedule_s *hopping_schedule, bool unicast_schedule)
{
    uint16_t length;
    uint8_t channel_function;
    if (unicast_schedule) {
        length = 4;
        channel_function = hopping_schedule->uc_channel_function;
    } else {
        length = 10;
        channel_function = hopping_schedule->bc_channel_function;
    }

    length += ws_channel_plan_length(hopping_schedule->channel_plan);

    length += ws_channel_function_length(channel_function, 1);

    if (unicast_schedule && hopping_schedule->excluded_channels.excuded_channel_ctrl) {
        if (hopping_schedule->excluded_channels.excuded_channel_ctrl == WS_EXC_CHAN_CTRL_RANGE) {
            length += (hopping_schedule->excluded_channels.excluded_range_length * 4) + 1;
        } else {
            length += hopping_schedule->excluded_channels.channel_mask_bytes_inline;
        }
    }
#ifdef WISUN_RCP_ENABLE
    // update the BS-IE exclude channel
    if ((!unicast_schedule) && hopping_schedule->bc_excluded_channels.excuded_channel_ctrl) {
        if (hopping_schedule->bc_excluded_channels.excuded_channel_ctrl == WS_EXC_CHAN_CTRL_RANGE) {
            length += (hopping_schedule->bc_excluded_channels.excluded_range_length * 4) + 1;
        } else {
            length += hopping_schedule->bc_excluded_channels.channel_mask_bytes_inline;
        }
    }
#endif
    return length;
}

uint8_t *ws_wh_utt_write(uint8_t *ptr, uint8_t message_type)
{
    ptr = ws_wh_header_base_write(ptr, 4, WH_IE_UTT_TYPE);
    *ptr++ = message_type;
    memset(ptr, 0, 3);
    ptr += 3;
    return ptr;
}

uint8_t *ws_wh_bt_write(uint8_t *ptr)
{
    ptr = ws_wh_header_base_write(ptr, 5, WH_IE_BT_TYPE);
    memset(ptr, 0, 5);
    ptr += 5;
    return ptr;
}


uint8_t *ws_wh_fc_write(uint8_t *ptr, ws_fc_ie_t *fc_ie)
{
    ptr = ws_wh_header_base_write(ptr, 2, WH_IE_FC_TYPE);
    *ptr++ = fc_ie->tx_flow_ctrl;
    *ptr++ = fc_ie->rx_flow_ctrl;
    return ptr;
}

uint8_t *ws_wh_rsl_write(uint8_t *ptr, uint8_t rsl)
{
    ptr = ws_wh_header_base_write(ptr, 1, WH_IE_RSL_TYPE);
    *ptr++ = rsl;
    return ptr;
}

uint8_t *ws_wh_ea_write(uint8_t *ptr, uint8_t *eui64)
{
    ptr = ws_wh_header_base_write(ptr, 8, WH_IE_EA_TYPE);
    memcpy(ptr, eui64, 8);
    ptr += 8;
    return ptr;
}

uint8_t *ws_wh_vh_write(uint8_t *ptr, uint8_t *vendor_header, uint8_t vendor_header_length)
{
    ptr = ws_wh_header_base_write(ptr, vendor_header_length, WH_IE_VH_TYPE);
    if (vendor_header_length) {
        memcpy(ptr, vendor_header, vendor_header_length);
        ptr += vendor_header_length;
    }
    return ptr;
}

uint8_t *ws_wp_base_write(uint8_t *ptr, uint16_t length)
{
    return mac_ie_payload_base_write(ptr, WS_WP_NESTED_IE, length);
}

uint8_t *ws_wp_nested_hopping_schedule_write(uint8_t *ptr, struct ws_hopping_schedule_s *hopping_schedule, bool unicast_schedule)
{
    //Calculate length
    uint16_t length = ws_wp_nested_hopping_schedule_length(hopping_schedule, unicast_schedule);
    if (!unicast_schedule) {
        ptr = mac_ie_nested_ie_long_base_write(ptr, WP_PAYLOAD_IE_BS_TYPE, length);
        ptr = common_write_32_bit_inverse(hopping_schedule->fhss_broadcast_interval, ptr);
        ptr = common_write_16_bit_inverse(hopping_schedule->fhss_bsi, ptr);
        *ptr++ = hopping_schedule->fhss_bc_dwell_interval;
    } else {
        ptr = mac_ie_nested_ie_long_base_write(ptr, WP_PAYLOAD_IE_US_TYPE, length);
        *ptr++ =  hopping_schedule->fhss_uc_dwell_interval;
    }

    *ptr++ =  hopping_schedule->clock_drift;
    *ptr++ =  hopping_schedule->timing_accurancy;
    uint8_t channel_info_base = 0;
    channel_info_base = (hopping_schedule->channel_plan);
    if (unicast_schedule) {
        channel_info_base |= (hopping_schedule->uc_channel_function << 3);
        //Set Excluded Channel control part
        channel_info_base |= (hopping_schedule->excluded_channels.excuded_channel_ctrl << 6);
    } else {
        channel_info_base |= (hopping_schedule->bc_channel_function << 3);
        //Set Excluded Channel control part
        channel_info_base |= (hopping_schedule->bc_excluded_channels.excuded_channel_ctrl << 6);
    }

    *ptr++ = channel_info_base;

    switch (hopping_schedule->channel_plan) {
        case 0:
            //Regulator domain and operationg class inline
            *ptr++ = hopping_schedule->regulatory_domain;
            *ptr++ = hopping_schedule->operating_class;
            break;
        case 1:
            //CH0, Channel spasing and number of channel's inline
            //CH0: th eunit is Khz (from sysconfig and in cfg_prop.)
            ptr = common_write_24_bit_inverse(hopping_schedule->ch0_freq , ptr);
            *ptr++ = hopping_schedule->channel_spacing;
            ptr = common_write_16_bit_inverse(hopping_schedule->number_of_channels, ptr);
            break;

#ifdef WISUN_RCP_ENABLE //WISUN_FAN_CORE_1_1
        case 2:
             *ptr++ = hopping_schedule->regulatory_domain;
             *ptr++ = hopping_schedule->channel_plan_id;
              break;
#endif //WISUN_FAN_CORE_1_1

        default:
            break;
    }

    uint8_t cf = hopping_schedule->uc_channel_function;
    uint16_t fixed_channel = hopping_schedule->uc_fixed_channel;
    if (!unicast_schedule) {
        cf = hopping_schedule->bc_channel_function;
    }
    switch (cf) {
        case 0:
            //Fixed channel inline
            if (!unicast_schedule) {
                fixed_channel = hopping_schedule->bc_fixed_channel;
            }
            ptr = common_write_16_bit_inverse(fixed_channel, ptr);
            break;
        case 1:
        case 2:
            //No Inline
            break;
        case 3:
            //Hop count + channel hop list
            *ptr++ = 1;
            *ptr++ = 0;
            break;
        default:
            break;

    }

#ifdef WISUN_RCP_ENABLE
    // need to support both UC and BC exclude channel list
    ws_excluded_channel_data_t *p_excluded_channels;
    uint8_t *p_excluded_channels_start;

    if (unicast_schedule)
    {   // UNICAST case
        p_excluded_channels = &(hopping_schedule->excluded_channels);
    }
    else
    {   //Broadcast case
        p_excluded_channels = &(hopping_schedule->bc_excluded_channels);
    }
    if (p_excluded_channels->excuded_channel_ctrl) {
        if (p_excluded_channels->excuded_channel_ctrl == WS_EXC_CHAN_CTRL_RANGE) {
            uint8_t range_length = p_excluded_channels->excluded_range_length;
            ws_excluded_channel_range_data_t *range_ptr = p_excluded_channels->exluded_range;
            *ptr++ = range_length;
            while (range_length) {
                ptr = common_write_16_bit_inverse(range_ptr->range_start, ptr);
                ptr = common_write_16_bit_inverse(range_ptr->range_end, ptr);
                range_length--;
                range_ptr++;
            }
        } else if (p_excluded_channels->excuded_channel_ctrl == WS_EXC_CHAN_CTRL_BITMASK) {
            //Set Mask
            uint16_t channel_mask_length = p_excluded_channels->channel_mask_bytes_inline * 8;

            for (uint8_t i = 0; i < p_excluded_channels->channel_mask_bytes_inline; i++) {
                uint8_t mask_value =p_excluded_channels->channel_mask4[i];
                /* if channel is not defined in channel plan, we need to use the zero bit */
                mask_value &= hopping_schedule->regulation_channel_mask[i];

                /* make our range channels to set 0x01*/
                // disable this featire for Certification test
#ifndef WISUN_CERT_CONFIG
                mask_value |= hopping_schedule->out_range_channel_mask[i];
#endif
                *ptr++ = mask_value;
                channel_mask_length -= 8;
                if (channel_mask_length == 0) {
                    break;
                }
            }
        }
    }
#else
    if (unicast_schedule && hopping_schedule->excluded_channels.excuded_channel_ctrl) {
        if (hopping_schedule->excluded_channels.excuded_channel_ctrl == WS_EXC_CHAN_CTRL_RANGE) {
            uint8_t range_length = hopping_schedule->excluded_channels.excluded_range_length;
            ws_excluded_channel_range_data_t *range_ptr = hopping_schedule->excluded_channels.exluded_range;
            *ptr++ = range_length;
            while (range_length) {
                ptr = common_write_16_bit_inverse(range_ptr->range_start, ptr);
                ptr = common_write_16_bit_inverse(range_ptr->range_end, ptr);
                range_length--;
                range_ptr++;
            }
        } else if (hopping_schedule->excluded_channels.excuded_channel_ctrl == WS_EXC_CHAN_CTRL_BITMASK) {
            //Set Mask
            uint16_t channel_mask_length = hopping_schedule->excluded_channels.channel_mask_bytes_inline * 8;

            for (uint8_t i = 0; i < 8; i++) {
                uint32_t mask_value = hopping_schedule->excluded_channels.channel_mask[i];
                if (channel_mask_length >= 32) {
                    ptr = common_write_32_bit(mask_value, ptr);
                    channel_mask_length -= 32;
                } else {
                    //Write MSB Bits from mask 24-8 top bits
                    uint8_t move_mask = 0;
                    while (channel_mask_length) {
                        *ptr++ = (uint8_t)(mask_value >> (24 - move_mask));
                        channel_mask_length -= 8;
                        move_mask += 8;
                    }
                }

                if (channel_mask_length == 0) {
                    break;
                }
            }
        }
    }
#endif //WISUN_RCP_ENABLE

    return ptr;
}

uint8_t *ws_wp_nested_vp_write(uint8_t *ptr, uint8_t *vendor_payload, uint16_t vendor_payload_length)
{
    if (vendor_payload_length) {
        ptr = mac_ie_nested_ie_long_base_write(ptr, WP_PAYLOAD_IE_VP_TYPE, vendor_payload_length);
        memcpy(ptr, vendor_payload, vendor_payload_length);
        ptr += vendor_payload_length;
    }
    return ptr;
}

uint8_t *ws_wp_nested_pan_info_write(uint8_t *ptr, struct ws_pan_information_s *pan_configuration)
{
    if (!pan_configuration) {
        return mac_ie_nested_ie_short_base_write(ptr, WP_PAYLOAD_IE_PAN_TYPE, 0);
    }
    ptr = mac_ie_nested_ie_short_base_write(ptr, WP_PAYLOAD_IE_PAN_TYPE, 5);
    ptr = common_write_16_bit_inverse(pan_configuration->pan_size, ptr);
    ptr = common_write_16_bit_inverse(pan_configuration->routing_cost, ptr);
    uint8_t temp8 = 0;
    temp8 |= (pan_configuration->use_parent_bs << 0);
    temp8 |= (pan_configuration->rpl_routing_method << 1);
    /* 20210201-FANWG-FANTPS-1.1v10-d3 : PAN Information Element (PAN-IE)
     * FAN 1.0 certified nodes MUST set FAN TPS Version field to 1.
     * FAN 1.1 certified nodes MUST set FAN TPS Version field to 2
    */
#ifdef WISUN_FAN_CORE_1_1
    temp8 |= (WS_FAN_VERSION_1_1) << 5;
#else
    temp8 |= (WS_FAN_VERSION_1_0) << 5;
#endif
    *ptr++ = temp8;
    return ptr;
}


uint8_t *ws_wp_nested_netname_write(uint8_t *ptr, uint8_t *network_name, uint8_t network_name_length)
{
    ptr = mac_ie_nested_ie_short_base_write(ptr, WP_PAYLOAD_IE_NETNAME_TYPE, network_name_length);
    if (network_name_length) {
        memcpy(ptr, network_name, network_name_length);
        ptr += network_name_length;
    }
    return ptr;
}

uint8_t *ws_wp_nested_pan_ver_write(uint8_t *ptr, struct ws_pan_information_s *pan_configuration)
{
    if (!pan_configuration) {
        return ptr;
    }
    ptr = mac_ie_nested_ie_short_base_write(ptr, WP_PAYLOAD_IE_PAN_VER_TYPE, 2);
    return common_write_16_bit_inverse(pan_configuration->pan_version, ptr);
}

uint8_t *ws_wp_nested_gtkhash_write(uint8_t *ptr, uint8_t *gtkhash, uint8_t gtkhash_length)
{
    ptr = mac_ie_nested_ie_short_base_write(ptr, WP_PAYLOAD_IE_GTKHASH_TYPE, gtkhash_length);
    if (gtkhash_length) {
        memcpy(ptr, gtkhash, 32);
        ptr += 32;
    }
    return ptr;
}

#ifdef WISUN_FAN_CORE_1_1
uint8_t *ws_res_wh_pan_wide_write(uint8_t *ptr, struct ws_pan_information_s *pan_cfg)
{
    uint8_t idx = 0, len = 0;
    uint8_t num_res_ies = pan_cfg->num_res_wh_pan_wide_ies;

    for(idx = 0; idx < num_res_ies; idx++ )
    {
        ptr = ws_wh_header_base_write(ptr, pan_cfg->res_wh_pan_wide_ie_lens[idx], pan_cfg->res_wh_pan_wide_ie_ids[idx]);
        memcpy(ptr, pan_cfg->res_wh_pan_wide_ies[idx].val, pan_cfg->res_wh_pan_wide_ies[idx].len);
        ptr += pan_cfg->res_wh_pan_wide_ies[idx].len;
    }

    return ptr;
}

uint8_t *ws_res_wh_ffn_wide_write(uint8_t *ptr, struct ws_pan_information_s *pan_cfg)
{
    uint8_t idx = 0, len = 0;
    uint8_t num_res_ies = pan_cfg->num_res_wh_ffn_wide_ies;

    for(idx = 0; idx < num_res_ies; idx++)
    {
        ptr = ws_wh_header_base_write(ptr, pan_cfg->res_wh_ffn_wide_ies[idx].len, pan_cfg->res_wh_ffn_wide_ie_ids[idx]);
        memcpy(ptr, pan_cfg->res_wh_ffn_wide_ies[idx].val, pan_cfg->res_wh_ffn_wide_ies[idx].len);
        ptr += pan_cfg->res_wh_ffn_wide_ies[idx].len;
    }

    return ptr;
}

uint8_t *ws_wp_nested_pom_write(uint8_t *ptr, struct ws_pom_ie_s *ptr_pom_ie)
{
    uint8_t len, i;
    uint8_t capacity_ie;
    if (ptr_pom_ie == NULL)
    {
        return ptr;
    }
    len = ptr_pom_ie->phy_op_mode_number + 1;
    ptr = mac_ie_nested_ie_short_base_write(ptr, WP_PAYLOAD_IE_POM, len);

    // capacity IE
    capacity_ie = ptr_pom_ie->phy_op_mode_number  |
                  ptr_pom_ie->mdr_command_capable << WS_WPIE_POM_PHY_OP_MODE_NUMBER_SHIFT;
    *ptr++ = capacity_ie;

    // write the PHY Operating mode
    for (i=0; i<len ; i++)
    {
        *ptr++ = ptr_pom_ie->phy_op_mode_id[i];
    }
    return ptr;
}

uint8_t *ws_wp_nested_jm_write(uint8_t *ptr, struct ws_pan_information_s *pan_congiguration)
{

    uint8_t temp, totallen, numMetrics = 0, metricLen, metricLenBits, i, j;
    if (pan_congiguration == NULL)
    {
        return ptr;
    }

    //code for writing JM-IE
    //write sub-IE length| ID| type
    //len = 1 + 1 + 4; //<content version> | <metric ID, length> | <data>
    //calc total len
    totallen = 1; //<content version>
    for(i = 0; i < WS_WPIE_JM_IE_MAX_NUM_METRICS; i++)
    {
        if(pan_congiguration->jm_metric_ids[i] !=0)
        {
            numMetrics++;
            totallen = totallen + 1 + pan_congiguration->jm_metric_lens[i]; //<metric ID, length> | <data
        }
    }

    //write 1 byte:  sub-IE length| ID| type
    ptr = mac_ie_nested_ie_short_base_write(ptr, WP_PAYLOAD_IE_JM, totallen);

    //write JM-IE content

    //1 byte: content version
    *ptr++ = pan_congiguration->jm_version;

    //write JM-IE metrics
    for(i = 0; i < WS_WPIE_JM_IE_MAX_NUM_METRICS; i++)
    {
        if(pan_congiguration->jm_metric_ids[i] !=0)
        {
            //1 byte: metric id+ metric length
            // 6.3.2.3.2.12Join Metrics Information Element (JM-IE)
            // mertric ID (6 bits) in LSB, length in MSB
            temp = (pan_congiguration->jm_metric_ids[i] << WS_WPIE_JM_METRIC_ID_SHIFT);

            metricLen = pan_congiguration->jm_metric_lens[i];
            //len field encode
            // 0 indicates the Metric Data field is 0 octets in length
            // 1 indicates the Metric Data field is 1 octets in length
            // 2 indicates the Metric Data field is 2 octets in length
            // 3 indicates the Metric Data field is 4 octets in length (special handling)
            metricLen == WS_WPIE_JM_IE_METRIC_LEN_MAX ? (metricLenBits = 0x3):(metricLenBits = metricLen);
            // LEN is in MSB
            temp = temp | ( (metricLenBits  << WS_WPIE_JM_METRIC_LEN_SHIFT ) & WS_WPIE_JM_METRIC_LEN_MASK);

            *ptr++ = temp;

            //4 bytes: actual content of metric 1
            for (j = 0; j < metricLen; j++)
            {
                *ptr++ = pan_congiguration->jm_metric_values[i][j];
            }

        }
    }

    return ptr;
}
uint8_t *ws_res_wp_short_pan_wide_write(uint8_t *ptr, struct ws_pan_information_s *pan_cfg)
{
    uint8_t idx = 0, len = 0;
    uint8_t num_res_ies = pan_cfg->num_res_wp_short_pan_wide_ies;

    for(idx = 0; idx < num_res_ies; idx++ )
    {
        //write 2 bytes:  sub-IE length| ID| type
        ptr = mac_ie_nested_ie_short_base_write(ptr, pan_cfg->res_wp_short_pan_wide_ie_ids[idx], pan_cfg->res_wp_short_pan_wide_ie_lens[idx]);
        memcpy(ptr, pan_cfg->res_wp_short_pan_wide_ies[idx].val, pan_cfg->res_wp_short_pan_wide_ies[idx].len);
        ptr += pan_cfg->res_wp_short_pan_wide_ies[idx].len;
    }

    return ptr;
}
uint8_t *ws_res_wp_short_ffn_wide_write(uint8_t *ptr, struct ws_pan_information_s *pan_cfg)
{
    uint8_t idx = 0, len = 0;
    uint8_t num_res_ies = pan_cfg->num_res_wp_short_ffn_wide_ies;

    for(idx = 0; idx < num_res_ies; idx++ )
    {
        //write 2 bytes:  sub-IE length| ID| type
        ptr = mac_ie_nested_ie_short_base_write(ptr, pan_cfg->res_wp_short_ffn_wide_ie_ids[idx], pan_cfg->res_wp_short_ffn_wide_ie_lens[idx]);
        memcpy(ptr, pan_cfg->res_wp_short_ffn_wide_ies[idx].val, pan_cfg->res_wp_short_ffn_wide_ies[idx].len);
        ptr += pan_cfg->res_wp_short_ffn_wide_ies[idx].len;
    }

    return ptr;
}

uint8_t *ws_res_wp_long_pan_wide_write(uint8_t *ptr, struct ws_pan_information_s *pan_cfg)
{
    uint8_t idx = 0, len = 0;
    uint8_t num_res_ies = pan_cfg->num_res_wp_long_pan_wide_ies;

    for(idx = 0; idx < num_res_ies; idx++ )
    {
        //write 2 bytes:  sub-IE length| ID| type
        ptr = mac_ie_nested_ie_long_base_write(ptr, pan_cfg->res_wp_long_pan_wide_ie_ids[idx], pan_cfg->res_wp_long_pan_wide_ie_lens[idx]);
        memcpy(ptr, pan_cfg->res_wp_long_pan_wide_ies[idx].val, pan_cfg->res_wp_long_pan_wide_ies[idx].len);
        ptr += pan_cfg->res_wp_long_pan_wide_ies[idx].len;
    }

    return ptr;
}
uint8_t *ws_res_wp_long_ffn_wide_write(uint8_t *ptr, struct ws_pan_information_s *pan_cfg)
{
    uint8_t idx = 0, len = 0;
    uint8_t num_res_ies = pan_cfg->num_res_wp_long_ffn_wide_ies;

    for(idx = 0; idx < num_res_ies; idx++ )
    {
        //write 2 bytes:  sub-IE length| ID| type
        ptr = mac_ie_nested_ie_long_base_write(ptr, pan_cfg->res_wp_long_ffn_wide_ie_ids[idx], pan_cfg->res_wp_long_ffn_wide_ie_lens[idx]);
        memcpy(ptr, pan_cfg->res_wp_long_ffn_wide_ies[idx].val, pan_cfg->res_wp_long_ffn_wide_ies[idx].len);
        ptr += pan_cfg->res_wp_long_ffn_wide_ies[idx].len;
    }

    return ptr;
}

#endif //WISUN_FAN_CORE_1_1
bool ws_wh_utt_read(uint8_t *data, uint16_t length, struct ws_utt_ie *utt_ie)
{
    mac_header_IE_t utt_ie_data;
    utt_ie_data.id = MAC_HEADER_ASSIGNED_EXTERNAL_ORG_IE_ID;
    if (4 != mac_ie_header_sub_id_discover(data, length, &utt_ie_data, WH_IE_UTT_TYPE)) {
        // NO UTT header
        return false;
    }
    data = utt_ie_data.content_ptr;
    utt_ie->message_type = *data++;
    utt_ie->ufsi = common_read_24_bit_inverse(data);
    return true;
}

bool ws_wh_bt_read(uint8_t *data, uint16_t length, struct ws_bt_ie *bt_ie)
{
    mac_header_IE_t btt_ie_data;
    btt_ie_data.id = MAC_HEADER_ASSIGNED_EXTERNAL_ORG_IE_ID;
    if (5 != mac_ie_header_sub_id_discover(data, length, &btt_ie_data, WH_IE_BT_TYPE)) {
        return false;
    }
    data = btt_ie_data.content_ptr;
    bt_ie->broadcast_slot_number = common_read_16_bit_inverse(data);
    bt_ie->broadcast_interval_offset = common_read_24_bit_inverse(data + 2);
    return true;
}

bool ws_wh_fc_read(uint8_t *data, uint16_t length, struct ws_fc_ie *fc_ie)
{
    mac_header_IE_t fc_ie_data;
    fc_ie_data.id = MAC_HEADER_ASSIGNED_EXTERNAL_ORG_IE_ID;
    if (2 != mac_ie_header_sub_id_discover(data, length, &fc_ie_data, WH_IE_FC_TYPE)) {
        return false;
    }
    data = fc_ie_data.content_ptr;
    fc_ie->tx_flow_ctrl = *data++;
    fc_ie->rx_flow_ctrl = *data;
    return true;
}

bool ws_wh_rsl_read(uint8_t *data, uint16_t length, int8_t *rsl)
{
    mac_header_IE_t rsl_ie_data;
    rsl_ie_data.id = MAC_HEADER_ASSIGNED_EXTERNAL_ORG_IE_ID;
    if (1 != mac_ie_header_sub_id_discover(data, length, &rsl_ie_data, WH_IE_RSL_TYPE)) {
        return false;
    }
    *rsl = *rsl_ie_data.content_ptr;

    return true;
}

bool ws_wh_ea_read(uint8_t *data, uint16_t length, uint8_t *eui64)
{
    mac_header_IE_t rsl_ie_data;
    rsl_ie_data.id = MAC_HEADER_ASSIGNED_EXTERNAL_ORG_IE_ID;
    if (8 != mac_ie_header_sub_id_discover(data, length, &rsl_ie_data, WH_IE_EA_TYPE)) {
        return false;
    }
    memcpy(eui64, rsl_ie_data.content_ptr, 8);

    return true;
}

#ifdef WISUN_FAN_CORE_1_1
//function to populate the number of res WH IEs and their IDs and lengths in the cur->ws_info->pan_information data base
bool ws_res_wh_sub_ids_discover(struct ws_pan_information_s* ws_pan_info, uint8_t *header_ptr, uint16_t length)
{
    mac_header_IE_t ie_element;
    uint8_t sub_id, ie_len;
    uint8_t *val_ptr;

    uint8_t pan_wide_idx = 0, ffn_wide_idx = 0;

    while (length > WISUN_MIN_IE_LENGTH) { //IE should be atleast 2 bytes to be able to interpret the header of the IE

        //fetch the IE
        mac_ie_header_parse(&ie_element, header_ptr);
        //point to the sub iD
        sub_id = *ie_element.content_ptr++; //intentinally incrementing
        ie_len = (ie_element.length -1);

        if (ie_element.length && MAC_HEADER_ASSIGNED_EXTERNAL_ORG_IE_ID == ie_element.id)
        {
            if(sub_id >= WH_PAN_WIDE_RES_IE_SUBID_MIN && sub_id <= WH_PAN_WIDE_RES_IE_SUBID_MAX)
            {
                //found pan wide WH IE
                ws_pan_info->num_res_wh_pan_wide_ies++;
                ws_pan_info->res_wh_pan_wide_ie_ids[pan_wide_idx] = sub_id;
                ws_pan_info->res_wh_pan_wide_ie_lens[pan_wide_idx] = ie_len;

                // copy the iE content
                ws_pan_info->res_wh_pan_wide_ies[pan_wide_idx].len = ie_len;

                val_ptr = ns_dyn_mem_alloc(ie_len);
                if(val_ptr != NULL)
                {
                    ws_pan_info->res_wh_pan_wide_ies[pan_wide_idx].val = val_ptr;
                    memcpy(ws_pan_info->res_wh_pan_wide_ies[pan_wide_idx].val, ie_element.content_ptr, ie_len);
                    pan_wide_idx++;
                }
                else
                {
                    //intentionally setting to zero to make sure stale or half read values are not used
                    ws_pan_info->num_res_wh_pan_wide_ies = 0;
                    ws_pan_info->res_wh_pan_wide_ie_ids[pan_wide_idx] = 0;
                    ws_pan_info->res_wh_pan_wide_ie_lens[pan_wide_idx] = 0;
                    ws_pan_info->res_wh_pan_wide_ies[pan_wide_idx].len = 0;
                    tr_error("memory allocation issue while reading reserved WH Pan wide IE");
                    return(false);
                }

            }
            else if(sub_id >= WH_FFN_WIDE_RES_IE_SUBID_MIN && sub_id <= WH_FFN_WIDE_RES_IE_SUBID_MAX)
            {
                //found ffn wide WH IE
                ws_pan_info->num_res_wh_ffn_wide_ies++;
                ws_pan_info->res_wh_ffn_wide_ie_ids[ffn_wide_idx] = sub_id;
                ws_pan_info->res_wh_ffn_wide_ie_lens[ffn_wide_idx] = ie_len;

                // copy the iE content
                ws_pan_info->res_wh_ffn_wide_ies[ffn_wide_idx].len = ie_len;

                val_ptr = ns_dyn_mem_alloc(ie_len);
                if(val_ptr != NULL)
                {
                    ws_pan_info->res_wh_ffn_wide_ies[ffn_wide_idx].val = val_ptr;
                    memcpy(ws_pan_info->res_wh_ffn_wide_ies[ffn_wide_idx].val, ie_element.content_ptr, ie_len);
                    ffn_wide_idx++;
                }
                else
                {
                    //intentionally setting to zero to make sure stale or half read values are not used
                    ws_pan_info->num_res_wh_ffn_wide_ies = 0;
                    ws_pan_info->res_wh_ffn_wide_ie_ids[ffn_wide_idx] = 0;
                    ws_pan_info->res_wh_ffn_wide_ie_lens[ffn_wide_idx] = 0;
                    ws_pan_info->res_wh_ffn_wide_ies[ffn_wide_idx].len = 0;
                    tr_error("memory allocation issue while reading reserved WH FFN wide IE");
                    return(false);
                }

            }
            else
            {
                //do nothing; continue processing
            }

        }

        //move ahead to fetch next IE
        length -= ie_element.length + 2;
        header_ptr += ie_element.length + 2;
    }

    //TBD: consider putting a check on length becoming zero : else it means there are some mal formed ies
    tr_debug("Read %d PAN Wide IEs and %d FFN Wide IEs", ws_pan_info->num_res_wh_pan_wide_ies, ws_pan_info->num_res_wh_ffn_wide_ies);
    return(true);
}

bool ws_res_wh_pan_ffn_wide_ies_read(struct ws_pan_information_s* ws_pan_info, uint8_t *headerIeList, uint16_t headerIeListLength)
{
    uint8_t idx = 0;
    bool status;

    //free the memory
    for(idx = 0; idx < ws_pan_info->num_res_wh_pan_wide_ies; idx++)
    {
        ns_dyn_mem_free(ws_pan_info->res_wh_pan_wide_ies[idx].val);
        ws_pan_info->res_wh_pan_wide_ies[idx].val = NULL;
    }

    for(idx = 0; idx < ws_pan_info->num_res_wh_ffn_wide_ies; idx++)
    {
        ns_dyn_mem_free(ws_pan_info->res_wh_ffn_wide_ies[idx].val);
        ws_pan_info->res_wh_ffn_wide_ies[idx].val = NULL;
    }

    //clear out counters
    ws_pan_info->num_res_wh_pan_wide_ies = 0;
    ws_pan_info->num_res_wh_ffn_wide_ies = 0;

    //Discover all wh pan & ffn wide ies ; contents stored in ws_pan_info
    status = ws_res_wh_sub_ids_discover(ws_pan_info, headerIeList, headerIeListLength);

    return (status);
}

//function to populate the number of res WH IEs and their IDs and lengths in the cur->ws_info->pan_information data base
bool ws_res_wp_sub_ids_discover(struct ws_pan_information_s* ws_pan_info, uint8_t *payload_ptr, uint16_t length)
{
    mac_nested_payload_IE_t ie_element;
    uint8_t sub_id, ie_len;
    uint8_t *val_ptr;

    uint8_t short_pan_wide_idx = 0, short_ffn_wide_idx = 0;
    uint8_t long_pan_wide_idx = 0, long_ffn_wide_idx = 0;

    while (length >= WS_WP_SUB_IE_ELEMENT_HEADER_LENGTH) {

        //fetch the Sub-IE
        mac_ie_nested_id_parse(&ie_element, payload_ptr);
        sub_id = ie_element.id;
        ie_len = ie_element.length;

        //assuming SUB IDs will have non zero length
        if (ie_element.length)
        {
            if(0 == ie_element.type_long) //short type
            {
                if(sub_id >= WP_SHORT_PAN_WIDE_RES_IE_SUBID_MIN && sub_id <= WP_SHORT_PAN_WIDE_RES_IE_SUBID_MAX)
                {
                    //found pan wide WP IE
                    ws_pan_info->num_res_wp_short_pan_wide_ies++;
                    ws_pan_info->res_wp_short_pan_wide_ie_ids[short_pan_wide_idx] = sub_id;
                    ws_pan_info->res_wp_short_pan_wide_ie_lens[short_pan_wide_idx] = ie_len;

                    // copy the iE content
                    ws_pan_info->res_wp_short_pan_wide_ies[short_pan_wide_idx].len = ie_len;

                    val_ptr = ns_dyn_mem_alloc(ie_len);
                    if(val_ptr != NULL)
                    {
                        ws_pan_info->res_wp_short_pan_wide_ies[short_pan_wide_idx].val = val_ptr;
                        memcpy(ws_pan_info->res_wp_short_pan_wide_ies[short_pan_wide_idx].val, ie_element.content_ptr, ie_len);
                        short_pan_wide_idx++;
                    }
                    else
                    {
                        //intentionally setting to zero to make sure stale or half read values are not used
                        ws_pan_info->num_res_wp_short_pan_wide_ies = 0;
                        ws_pan_info->res_wp_short_pan_wide_ie_ids[short_pan_wide_idx] = 0;
                        ws_pan_info->res_wp_short_pan_wide_ie_lens[short_pan_wide_idx] = 0;
                        ws_pan_info->res_wp_short_pan_wide_ies[short_pan_wide_idx].len = 0;
                        tr_error("memory allocation issue while reading reserved WH Pan wide IE");
                        return(false);
                    }

                }
                else if(sub_id >= WP_SHORT_FFN_WIDE_RES_IE_SUBID_MIN && sub_id <= WP_SHORT_FFN_WIDE_RES_IE_SUBID_MAX)
                {
                    //found ffn wide WP IE
                    ws_pan_info->num_res_wp_short_ffn_wide_ies ++;
                    ws_pan_info->res_wp_short_ffn_wide_ie_ids[short_ffn_wide_idx] = sub_id;
                    ws_pan_info->res_wp_short_ffn_wide_ie_lens[short_ffn_wide_idx] = ie_len;

                    // copy the iE content
                    ws_pan_info->res_wp_short_ffn_wide_ies[short_ffn_wide_idx].len = ie_len;

                    val_ptr = ns_dyn_mem_alloc(ie_len);
                    if(val_ptr != NULL)
                    {
                        ws_pan_info->res_wp_short_ffn_wide_ies[short_ffn_wide_idx].val = val_ptr;
                        memcpy(ws_pan_info->res_wp_short_ffn_wide_ies[short_ffn_wide_idx].val, ie_element.content_ptr, ie_len);
                        short_ffn_wide_idx++;
                    }
                    else
                    {
                        //intentionally setting to zero to make sure stale or half read values are not used
                        ws_pan_info->num_res_wp_short_ffn_wide_ies = 0;
                        ws_pan_info->res_wp_short_ffn_wide_ie_ids[short_ffn_wide_idx] = 0;
                        ws_pan_info->res_wp_short_ffn_wide_ie_lens[short_ffn_wide_idx] = 0;
                        ws_pan_info->res_wp_short_ffn_wide_ies[short_ffn_wide_idx].len = 0;
                        tr_error("memory allocation issue while reading reserved WH FFN wide IE");
                        return(false);
                    }

                }
                // else:do nothing; continue processing
            }// type short
            else //type long
            {
                if(sub_id >= WP_LONG_PAN_WIDE_RES_IE_SUBID_MIN && sub_id <= WP_LONG_PAN_WIDE_RES_IE_SUBID_MAX)
                {
                    //found pan wide WP IE
                    ws_pan_info->num_res_wp_long_pan_wide_ies++;
                    ws_pan_info->res_wp_long_pan_wide_ie_ids[long_pan_wide_idx] = sub_id;
                    ws_pan_info->res_wp_long_pan_wide_ie_lens[long_pan_wide_idx] = ie_len;

                    // copy the iE content
                    ws_pan_info->res_wp_long_pan_wide_ies[long_pan_wide_idx].len = ie_len;

                    val_ptr = ns_dyn_mem_alloc(ie_len);
                    if(val_ptr != NULL)
                    {
                        ws_pan_info->res_wp_long_pan_wide_ies[long_pan_wide_idx].val = val_ptr;
                        memcpy(ws_pan_info->res_wp_long_pan_wide_ies[long_pan_wide_idx].val, ie_element.content_ptr, ie_len);
                        long_pan_wide_idx++;
                    }
                    else
                    {
                        //intentionally setting to zero to make sure stale or half read values are not used
                        ws_pan_info->num_res_wp_long_pan_wide_ies = 0;
                        ws_pan_info->res_wp_long_pan_wide_ie_ids[long_pan_wide_idx] = 0;
                        ws_pan_info->res_wp_long_pan_wide_ie_lens[long_pan_wide_idx] = 0;
                        ws_pan_info->res_wp_long_pan_wide_ies[long_pan_wide_idx].len = 0;
                        tr_error("memory allocation issue while reading reserved WH Pan wide IE");
                        return(false);
                    }

                }
                else if(sub_id >= WP_LONG_FFN_WIDE_RES_IE_SUBID_MIN && sub_id <= WP_LONG_FFN_WIDE_RES_IE_SUBID_MAX)
                {
                    //found ffn wide WP IE
                    ws_pan_info->num_res_wp_long_ffn_wide_ies ++;
                    ws_pan_info->res_wp_long_ffn_wide_ie_ids[long_ffn_wide_idx] = sub_id;
                    ws_pan_info->res_wp_long_ffn_wide_ie_lens[long_ffn_wide_idx] = ie_len;

                    // copy the iE content
                    ws_pan_info->res_wp_long_ffn_wide_ies[long_ffn_wide_idx].len = ie_len;

                    val_ptr = ns_dyn_mem_alloc(ie_len);
                    if(val_ptr != NULL)
                    {
                        ws_pan_info->res_wp_long_ffn_wide_ies[long_ffn_wide_idx].val = val_ptr;
                        memcpy(ws_pan_info->res_wp_long_ffn_wide_ies[long_ffn_wide_idx].val, ie_element.content_ptr, ie_len);
                        long_ffn_wide_idx++;
                    }
                    else
                    {
                        //intentionally setting to zero to make sure stale or half read values are not used
                        ws_pan_info->num_res_wp_long_ffn_wide_ies = 0;
                        ws_pan_info->res_wp_long_ffn_wide_ie_ids[long_ffn_wide_idx] = 0;
                        ws_pan_info->res_wp_long_ffn_wide_ie_lens[long_ffn_wide_idx] = 0;
                        ws_pan_info->res_wp_long_ffn_wide_ies[long_ffn_wide_idx].len = 0;
                        tr_error("memory allocation issue while reading reserved WH FFN wide IE");
                        return(false);
                    }

                }
                // else:do nothing; continue processing

            } //type long

        }//check for IE Length

        //move ahead to fetch next SUB IE
        length -= ie_element.length + 2;
        payload_ptr += ie_element.length + 2;
    }

    return(true);
}

bool ws_res_wp_pan_ffn_wide_ies_read(struct ws_pan_information_s* ws_pan_info, uint8_t *payloadIeList, uint16_t paylaodIeListLength)
{
    uint8_t idx = 0;
    bool status;

    //free the memory
    //short pan wide
    for(idx = 0; idx < ws_pan_info->num_res_wp_short_pan_wide_ies; idx++)
    {
        ns_dyn_mem_free(ws_pan_info->res_wp_short_pan_wide_ies[idx].val);
        ws_pan_info->res_wp_short_pan_wide_ies[idx].val = NULL;
    }
    //short ffn wide
    for(idx = 0; idx < ws_pan_info->num_res_wp_short_ffn_wide_ies; idx++)
    {
        ns_dyn_mem_free(ws_pan_info->res_wp_short_ffn_wide_ies[idx].val);
        ws_pan_info->res_wp_short_ffn_wide_ies[idx].val = NULL;
    }
    //long pan wide
    for(idx = 0; idx < ws_pan_info->num_res_wp_long_pan_wide_ies; idx++)
    {
        ns_dyn_mem_free(ws_pan_info->res_wp_long_pan_wide_ies[idx].val);
        ws_pan_info->res_wp_long_pan_wide_ies[idx].val = NULL;
    }
    //long ffn wide
    for(idx = 0; idx < ws_pan_info->num_res_wp_long_ffn_wide_ies; idx++)
    {
        ns_dyn_mem_free(ws_pan_info->res_wp_long_ffn_wide_ies[idx].val);
        ws_pan_info->res_wp_long_ffn_wide_ies[idx].val = NULL;
    }

    //clear out counters
    ws_pan_info->num_res_wp_short_pan_wide_ies = 0;
    ws_pan_info->num_res_wp_short_ffn_wide_ies = 0;
    ws_pan_info->num_res_wp_long_pan_wide_ies = 0;
    ws_pan_info->num_res_wp_long_ffn_wide_ies = 0;

    //Discover all wh pan & ffn wide ies ; contents stored in ws_pan_info
    status = ws_res_wp_sub_ids_discover(ws_pan_info, payloadIeList, paylaodIeListLength);

    return (status);
}
#endif //WISUN_FAN_CORE_1_1
static uint8_t *ws_channel_plan_zero_read(uint8_t *ptr, ws_channel_plan_zero_t *plan)
{
    plan->regulator_domain = *ptr++;
    plan->operation_class = *ptr++;
    return ptr;
}

static uint8_t *ws_channel_plan_one_read(uint8_t *ptr, ws_channel_plan_one_t *plan)
{
    /* Channel 0 frequency (KHz) */
    plan->ch0 = common_read_24_bit_inverse(ptr);
    ptr += 3;
    plan->channel_spacing = *ptr++;
    plan->number_of_channel = common_read_16_bit_inverse(ptr);
    ptr += 2;
    return ptr;
}

static uint8_t *ws_channel_plan_two_read(uint8_t *ptr, ws_channel_plan_two_t *plan)
{
    plan->regulator_domain = *ptr++;
    plan->channel_plan_id = *ptr++;
    return ptr;
}

static uint8_t *ws_channel_function_zero_read(uint8_t *ptr, ws_channel_function_zero_t *plan)
{
    plan->fixed_channel = common_read_16_bit_inverse(ptr);
    return ptr + 2;
}

static uint8_t *ws_channel_function_three_read(uint8_t *ptr, ws_channel_function_three_t *plan)
{
    plan->channel_hop_count = *ptr++;
    plan->channel_list = ptr++;
    return ptr;
}

bool ws_wp_nested_us_read(uint8_t *data, uint16_t length, struct ws_us_ie *us_ie)
{
    mac_nested_payload_IE_t nested_payload_ie;
    nested_payload_ie.id = WP_PAYLOAD_IE_US_TYPE;
    nested_payload_ie.type_long = true;
    if (mac_ie_nested_discover(data, length, &nested_payload_ie) < 4) {
        return false;
    }

    data = nested_payload_ie.content_ptr;
    us_ie->dwell_interval = *data++;
    us_ie->clock_drift = *data++;
    us_ie->timing_accurancy = *data++;
    us_ie->channel_plan = (*data & 3);
    us_ie->channel_function = (*data & 0x38) >> 3;
    us_ie->excluded_channel_ctrl = (*data & 0xc0) >> 6;
    data++;
    uint16_t info_length = 0;
    nested_payload_ie.length -= 4;
    info_length = ws_channel_plan_length(us_ie->channel_plan);
    if (nested_payload_ie.length < info_length) {
        return false;
    }

    nested_payload_ie.length -= info_length;
    switch (us_ie->channel_plan) {
        case 0:
            data = ws_channel_plan_zero_read(data, &us_ie->plan.zero);
            break;

        case 1:
            data = ws_channel_plan_one_read(data, &us_ie->plan.one);
            break;

        case 2:
            data = ws_channel_plan_two_read(data, &us_ie->plan.two);
            break;
        default:
            return false;

    }

    info_length = ws_channel_function_length(us_ie->channel_function, 0);

    if (nested_payload_ie.length < info_length) {
        return false;
    }
    nested_payload_ie.length -= info_length;


    switch (us_ie->channel_function) {
        case 0:
            data = ws_channel_function_zero_read(data, &us_ie->function.zero);
            break;

        case 1:
            /* TR51CF, we don't support */
            return false;
        case 2:
            break;

        case 3:
            /* vendor defined channel function */
            return false;
        default:
            return false;

    }

    switch (us_ie->excluded_channel_ctrl) {
        case WS_EXC_CHAN_CTRL_NONE:

            break;
        case WS_EXC_CHAN_CTRL_RANGE:
            us_ie->excluded_channels.range.number_of_range = *data;
            if (nested_payload_ie.length < (us_ie->excluded_channels.range.number_of_range * 4) + 1) {
                return false;
            }
            //Set Range start after validation
            us_ie->excluded_channels.range.range_start = data + 1;
            break;

        case WS_EXC_CHAN_CTRL_BITMASK:
            if (us_ie->channel_plan == 1) {
                us_ie->excluded_channels.mask.mask_len_inline = ((us_ie->plan.one.number_of_channel + 7) / 8);
                if (us_ie->excluded_channels.mask.mask_len_inline != nested_payload_ie.length) {
                    //Channel mask length is not correct
                    return false;
                }
            } else {
                us_ie->excluded_channels.mask.mask_len_inline = nested_payload_ie.length;
            }

            us_ie->excluded_channels.mask.channel_mask = data;
            break;
        default:
            return false;
    }

    return true;
}
bool ws_wp_nested_bs_read(uint8_t *data, uint16_t length, struct ws_bs_ie *bs_ie)
{
    mac_nested_payload_IE_t nested_payload_ie;
    nested_payload_ie.id = WP_PAYLOAD_IE_BS_TYPE;
    nested_payload_ie.type_long = true;
    if (mac_ie_nested_discover(data, length, &nested_payload_ie) < 10) {
        return false;
    }
    data = nested_payload_ie.content_ptr;
    bs_ie->broadcast_interval = common_read_32_bit_inverse(data);
    bs_ie->broadcast_schedule_identifier = common_read_16_bit_inverse(data + 4);
    data += 6;
    bs_ie->dwell_interval = *data++;
    bs_ie->clock_drift = *data++;
    bs_ie->timing_accurancy = *data++;

    bs_ie->channel_plan = (*data & 3);
    bs_ie->channel_function = (*data & 0x38) >> 3;
    bs_ie->excluded_channel_ctrl = (*data & 0xc0) >> 6;
    data++;
    nested_payload_ie.length -= 10;
    uint16_t info_length = 0;

    info_length = ws_channel_plan_length(bs_ie->channel_plan);
    if (nested_payload_ie.length < info_length) {
        return false;
    }
    nested_payload_ie.length -= info_length;
    switch (bs_ie->channel_plan) {
        case 0:
            data = ws_channel_plan_zero_read(data, &bs_ie->plan.zero);
            break;

        case 1:
            data = ws_channel_plan_one_read(data, &bs_ie->plan.one);
            break;
        case 2:
            data = ws_channel_plan_two_read(data, &bs_ie->plan.two);
            break;
        default:
            return false;

    }

    info_length = ws_channel_function_length(bs_ie->channel_function, 0);
    if (nested_payload_ie.length < info_length) {
        return false;
    }
    nested_payload_ie.length -= info_length;

    switch (bs_ie->channel_function) {
        case 0:
            data = ws_channel_function_zero_read(data, &bs_ie->function.zero);
            break;

        case 1:
            /* TR51CF, we don't support */
            return false;

        case 2:
            break;

        case 3:
            /* vendor defined channel function */
            return false;
        default:
            return false;

    }
#ifdef WISUN_RCP_ENABLE
    switch (bs_ie->excluded_channel_ctrl) {
        case WS_EXC_CHAN_CTRL_NONE:

            break;
        case WS_EXC_CHAN_CTRL_RANGE:
            bs_ie->excluded_channels.range.number_of_range = *data;
            if (nested_payload_ie.length < (bs_ie->excluded_channels.range.number_of_range * 4) + 1) {
                return false;
            }
            //Set Range start after validation
            bs_ie->excluded_channels.range.range_start = data + 1;
            break;

        case WS_EXC_CHAN_CTRL_BITMASK:
            if (bs_ie->channel_plan == 1) {
                bs_ie->excluded_channels.mask.mask_len_inline = ((bs_ie->plan.one.number_of_channel + 7) / 8);
                if (bs_ie->excluded_channels.mask.mask_len_inline != nested_payload_ie.length) {
                    //Channel mask length is not correct
                    return false;
                }
            } else {
                bs_ie->excluded_channels.mask.mask_len_inline = nested_payload_ie.length;
            }

            bs_ie->excluded_channels.mask.channel_mask = data;
            break;
        default:
            return false;
    }
#endif
    return true;
}

bool ws_wp_nested_pan_read(uint8_t *data, uint16_t length, struct ws_pan_information_s *pan_configuration)
{
    mac_nested_payload_IE_t nested_payload_ie;
    nested_payload_ie.id = WP_PAYLOAD_IE_PAN_TYPE;
    nested_payload_ie.type_long = false;
    if (mac_ie_nested_discover(data, length, &nested_payload_ie) !=  5) {
        return false;
    }

    pan_configuration->pan_size = common_read_16_bit_inverse(nested_payload_ie.content_ptr);
    pan_configuration->routing_cost = common_read_16_bit_inverse(nested_payload_ie.content_ptr + 2);
    pan_configuration->use_parent_bs = (nested_payload_ie.content_ptr[4] & 0x01) == 0x01;
    pan_configuration->rpl_routing_method = (nested_payload_ie.content_ptr[4] & 0x02) == 0x02;
    pan_configuration->version = (nested_payload_ie.content_ptr[4] & 0xe0) >> 5;

    return true;
}

bool ws_wp_nested_pan_version_read(uint8_t *data, uint16_t length, uint16_t *pan_version)
{
    mac_nested_payload_IE_t nested_payload_ie;
    nested_payload_ie.id = WP_PAYLOAD_IE_PAN_VER_TYPE;
    nested_payload_ie.type_long = false;
    if (mac_ie_nested_discover(data, length, &nested_payload_ie) !=  2) {
        return false;
    }
    *pan_version = common_read_16_bit_inverse(nested_payload_ie.content_ptr);

    return true;
}

uint8_t *ws_wp_nested_gtkhash_read(uint8_t *data, uint16_t length)
{
    mac_nested_payload_IE_t nested_payload_ie;
    nested_payload_ie.id = WP_PAYLOAD_IE_GTKHASH_TYPE;
    nested_payload_ie.type_long = false;
    if (mac_ie_nested_discover(data, length, &nested_payload_ie) !=  32) {
        return NULL;
    }

    return nested_payload_ie.content_ptr;
}


bool ws_wp_nested_network_name_read(uint8_t *data, uint16_t length, ws_wp_network_name_t *network_name)
{
    mac_nested_payload_IE_t nested_payload_ie;
    nested_payload_ie.id = WP_PAYLOAD_IE_NETNAME_TYPE;
    nested_payload_ie.type_long = false;

    if (0 == mac_ie_nested_discover(data, length, &nested_payload_ie)) {
        return false;
    } else if (nested_payload_ie.length > 32) {
        //Too long name
        return false;
    }
    network_name->network_name = nested_payload_ie.content_ptr;
    network_name->network_name_length = nested_payload_ie.length;
    return true;
}

#ifdef WISUN_FAN_CORE_1_1
bool ws_wp_nested_pom_read(uint8_t *data, uint16_t length, struct ws_pom_ie_s *ptr_pom_ie)
{
    mac_nested_payload_IE_t nested_payload_ie;
    nested_payload_ie.id = WP_PAYLOAD_IE_POM;
    nested_payload_ie.type_long = false;

    uint8_t capacity_ie,len, i, *pdata;

    if (0 == mac_ie_nested_discover(data, length, &nested_payload_ie)) {
        return false;
    }
    capacity_ie = *(nested_payload_ie.content_ptr);
    len = capacity_ie & WS_WPIE_POM_PHY_OP_MODE_NUMBER_MASK;
    if (nested_payload_ie.length > (len+1))
    {
        // too long
        return false;
    }
    ptr_pom_ie->phy_op_mode_number  = capacity_ie & WS_WPIE_POM_PHY_OP_MODE_NUMBER_MASK;
    ptr_pom_ie->mdr_command_capable = capacity_ie >> WS_WPIE_POM_MDR_CAPABLE_SHIFT;

    pdata = nested_payload_ie.content_ptr + 1;

    for (i=0; i<len; i++)
    {
        ptr_pom_ie->phy_op_mode_id[i] = pdata[i];
    }

    return true;
}

bool ws_wp_nested_jm_read(uint8_t *data, uint16_t length, struct ws_jm_ie_s *ptr_jm_ie)
{
    mac_nested_payload_IE_t nested_payload_ie;
    nested_payload_ie.id = WP_PAYLOAD_IE_JM;
    nested_payload_ie.type_long = false;

    uint8_t metric_id = 0, metric_len = 0, metric_idx = 0, *pdata = NULL, toParseLen = 0;

    //set jm_ie read to all zeros to start with
    memset(ptr_jm_ie, 0, sizeof(ws_jm_ie_t));

    if (0 == mac_ie_nested_discover(data, length, &nested_payload_ie)) {
        return false;
    }
    pdata = nested_payload_ie.content_ptr;
    toParseLen = nested_payload_ie.length;

    ptr_jm_ie->version = *pdata++;
    toParseLen--;
    g_num_jm_metrics_len = 1; //if JM-IE is present: atleast content verison will be present; minimum lenght is 1

    //parse the generic metrics
    //keeping code simple : no checks on len matching all metric ids content
    while(toParseLen > 0)
    {
        // read the metric ID and Len
        metric_id = (*pdata & WS_WPIE_JM_METRIC_ID_MASK) >> WS_WPIE_JM_METRIC_ID_SHIFT;
        metric_len = (*pdata & WS_WPIE_JM_METRIC_LEN_MASK) >> WS_WPIE_JM_METRIC_LEN_SHIFT;
        metric_len = (metric_len == 0x3? 4: metric_len);
        pdata++;
        toParseLen--;

        ptr_jm_ie->jm_metric_ids[metric_idx] = metric_id;
        ptr_jm_ie->jm_metric_lens[metric_idx] = metric_len;
        g_num_jm_metrics_len += (1 + metric_len); // 1 is for the byte containing metric and metric len
        memcpy(&ptr_jm_ie->jm_metric_values[metric_idx][0], pdata, metric_len);
        pdata += metric_len;
        toParseLen -= metric_len;

        //add check if it is JM_PLF
        if (metric_id == WS_JM_PLF)
        {
            if(metric_len != WS_WPIE_JM_IE_PLF_METRIC_LENGTH)
            {
                return false;
            }
            ptr_jm_ie->plf = ptr_jm_ie->jm_metric_values[metric_idx][0]; //just 1 byte anyway
        }

        //to the next metric ID
        metric_idx++;

        if(toParseLen == 0) //parsed all JM metrics: break out of the loop.
        {
            break;
        }

        //should not have come here if no rules are violated. There cannot be more than 4 JM metrics
        if(metric_idx >= WS_WPIE_JM_IE_MAX_NUM_METRICS)
        {
            return false;
        }
    }

    ptr_jm_ie->jm_num_of_metrics = metric_idx;
    return true;
}
#endif //WISUN_FAN_CORE_1_1

#ifdef FEATURE_WISUN_SUPPORT
#define MSG_ID_PANID     (0x01)
bool ws_wp_nested_vp_read(uint8_t *data, uint16_t length, struct ws_vp_ie *vp_ie)
{
    uint8_t msgID;
    mac_nested_payload_IE_t nested_payload_ie;
    nested_payload_ie.id = WP_PAYLOAD_IE_VP_TYPE;
    nested_payload_ie.type_long = true;
    if (mac_ie_nested_discover(data, length, &nested_payload_ie) < 3) {
        return false;
    }
    data = nested_payload_ie.content_ptr;
    vp_ie->vendor_id = *data++;
    /* messages are encoded in
     * msgID-1 + value (length is predefined)
     * msgID-2 + value
     * msgID-n : 0xFF termination
     */
    msgID = *data++;
    if (msgID == MSG_ID_PANID)
    {
        vp_ie->network_pan_id = common_read_16_bit_inverse(data);
    }
    return true;
}

/* provide this function to MAC
 * data point to WiSUN Paylaod IE header
 * */
bool ws_wp_nested_vp_get(uint8_t *data, uint16_t length, struct ws_vp_ie *vp_ie)
{
    mac_nested_payload_IE_t nested_payload_ie;
    mac_payload_IE_t ws_wp_nested;
    ws_wp_nested.id = WS_WP_NESTED_IE;

    /* make sure we have WiSUN Paylaod header IE */
    if (mac_ie_payload_discover(data, length, &ws_wp_nested) > 2)
    {   /* there is Wisun Paylaod header IE , serach VPIE */
        nested_payload_ie.id = WP_PAYLOAD_IE_VP_TYPE;
        nested_payload_ie.type_long = true;

        if (mac_ie_nested_discover(ws_wp_nested.content_ptr, ws_wp_nested.length, &nested_payload_ie) < 3)
        {
            return false;
        }

        vp_ie->ptrContent = nested_payload_ie.content_ptr;
        vp_ie->length = nested_payload_ie.length;
        return true;
    }
    return false;

}
#endif
