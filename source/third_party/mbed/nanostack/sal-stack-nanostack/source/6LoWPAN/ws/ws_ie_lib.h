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

#ifndef WS_IE_LIB_H_
#define WS_IE_LIB_H_

struct ws_pan_information_s;
struct ws_utt_ie;
struct ws_bt_ie;
struct ws_us_ie;
struct ws_hopping_schedule_s;
struct ws_fc_ie;


#ifdef WISUN_FAN_CORE_1_1
struct ws_pom_ie_s;
struct ws_jm_ie_s;

struct ws_wh_pan_wide_ie_s;
struct ws_wh_ffn_wide_ie_s;

struct ws_wp_short_pan_wide_ie_s;
struct ws_wp_short_ffn_wide_ie_s;
struct ws_wp_long_pan_wide_ie_s;
struct ws_wp_long_ffn_wide_ie_s;
#endif //WISUN_FAN_CORE_1_1
/**
 * @brief ws_wp_network_name_t WS nested payload network name
 */
typedef struct ws_wp_network_name {
    uint8_t network_name_length;
    uint8_t *network_name;
} ws_wp_network_name_t;

/* WS_WH HEADER IE */
uint8_t *ws_wh_utt_write(uint8_t *ptr, uint8_t message_type);
uint8_t *ws_wh_bt_write(uint8_t *ptr);
uint8_t *ws_wh_fc_write(uint8_t *ptr, struct ws_fc_ie *fc_ie);
uint8_t *ws_wh_rsl_write(uint8_t *ptr, uint8_t rsl);
uint8_t *ws_wh_vh_write(uint8_t *ptr, uint8_t *vendor_header, uint8_t vendor_header_length);
uint8_t *ws_wh_ea_write(uint8_t *ptr, uint8_t *eui64);
#ifdef WISUN_FAN_CORE_1_1
uint8_t *ws_res_wh_pan_wide_write(uint8_t *ptr, struct ws_pan_information_s *pan_cfg);
uint8_t *ws_res_wh_ffn_wide_write(uint8_t *ptr, struct ws_pan_information_s *pan_cfg);
#endif //WISUN_FAN_CORE_1_1

bool ws_wh_utt_read(uint8_t *data, uint16_t length, struct ws_utt_ie *utt_ie);
bool ws_wh_bt_read(uint8_t *data, uint16_t length, struct ws_bt_ie *bt_ie);
bool ws_wh_fc_read(uint8_t *data, uint16_t length, struct ws_fc_ie *fc_ie);
bool ws_wh_rsl_read(uint8_t *data, uint16_t length, int8_t *rsl);
bool ws_wh_ea_read(uint8_t *data, uint16_t length, uint8_t *eui64);
#ifdef WISUN_FAN_CORE_1_1
bool ws_res_wh_pan_ffn_wide_ies_read(struct ws_pan_information_s* ws_pan_info, uint8_t *headerIeList, uint16_t headerIeListLength);
#endif //WISUN_FAN_CORE_1_1

/* WS_WP_NESTED PAYLOD IE */
uint8_t *ws_wp_base_write(uint8_t *ptr, uint16_t length);
uint8_t *ws_wp_nested_hopping_schedule_write(uint8_t *ptr, struct ws_hopping_schedule_s *hopping_schedule, bool unicast_schedule);
uint8_t *ws_wp_nested_vp_write(uint8_t *ptr, uint8_t *vendor_payload, uint16_t vendor_payload_length);
uint8_t *ws_wp_nested_pan_info_write(uint8_t *ptr, struct ws_pan_information_s *pan_congiguration);
uint8_t *ws_wp_nested_netname_write(uint8_t *ptr, uint8_t *network_name, uint8_t network_name_length);
uint8_t *ws_wp_nested_pan_ver_write(uint8_t *ptr, struct ws_pan_information_s *pan_congiguration);
uint8_t *ws_wp_nested_gtkhash_write(uint8_t *ptr, uint8_t *gtkhash, uint8_t gtkhash_length);
uint16_t ws_wp_nested_hopping_schedule_length(struct ws_hopping_schedule_s *hopping_schedule, bool unicast_schedule);

#ifdef WISUN_FAN_CORE_1_1
uint8_t *ws_res_wp_short_pan_wide_write(uint8_t *ptr, struct ws_pan_information_s *pan_cfg);
uint8_t *ws_res_wp_short_ffn_wide_write(uint8_t *ptr, struct ws_pan_information_s *pan_cfg);
uint8_t *ws_res_wp_long_pan_wide_write(uint8_t *ptr, struct ws_pan_information_s *pan_cfg);
uint8_t *ws_res_wp_long_ffn_wide_write(uint8_t *ptr, struct ws_pan_information_s *pan_cfg);
#endif //WISUN_FAN_CORE_1_1

#ifdef WISUN_FAN_CORE_1_1
uint8_t *ws_wp_nested_pom_write(uint8_t *ptr, struct ws_pom_ie_s *ptr_pom_ie);
// Wi-SUN FAN 1.1v09-d10 Figure 6-62 Capability IE
#define WS_WPIE_POM_PHY_OP_MODE_NUMBER_MASK 0b00001111
#define WS_WPIE_POM_MDR_CAPABLE_MASK        0b00010000
#define WS_WPIE_POM_PHY_OP_MODE_NUMBER_SHIFT 0
#define WS_WPIE_POM_MDR_CAPABLE_SHIFT        4

uint8_t *ws_wp_nested_jm_write(uint8_t *ptr, struct ws_pan_information_s *pan_congiguration);
// 20210201-FANWG-FANTPS-1.1v09-d10
// Figure 68b JM-IE
// Wi-SUN FAN 1.1v06 Figure 68c JM-IE Metric

#define WS_WPIE_JM_METRIC_ID_MASK  0x3F
#define WS_WPIE_JM_METRIC_LEN_MASK 0xC0
#define WS_WPIE_JM_METRIC_ID_SHIFT      0
#define WS_WPIE_JM_METRIC_LEN_SHIFT     6

#define WS_WPIE_JM_IE_CONTENT_VERSION_LENGTH        1
#define WS_WPIE_JM_IE_PLF_LENGTH        3
#define WS_WPIE_JM_IE_PLF_METRIC_LENGTH       1

// Wi-SUN Assigned Value Registry 0v25
//   8. Join Metric IDs
enum {
    WS_JM_PLF = 1, // PAN Load Factor
};

#define WS_JM_TEST_METRIC_ID_1 0x30
#define WS_JM_TEST_METRIC_ID_1_LEN 4 //indicates 4 bytes of data

#define WS_JM_TEST_METRIC_ID_2 0x31
#define WS_JM_TEST_METRIC_ID_2_LEN 2 //indicates 2 bytes of data

// simulate the JMIE only has ID, but no data
#define WS_JM_TEST_METRIC_ID_3 0x32
#define WS_JM_TEST_METRIC_ID_3_LEN 0 //indicates zero bytes of data

#endif //WISUN_FAN_CORE_1_1

bool ws_wp_nested_us_read(uint8_t *data, uint16_t length, struct ws_us_ie *us_ie);
bool ws_wp_nested_bs_read(uint8_t *data, uint16_t length, struct ws_bs_ie *bs_ie);
bool ws_wp_nested_pan_read(uint8_t *data, uint16_t length, struct ws_pan_information_s *pan_congiguration);
bool ws_wp_nested_pan_version_read(uint8_t *data, uint16_t length, uint16_t *pan_version);
bool ws_wp_nested_network_name_read(uint8_t *data, uint16_t length, ws_wp_network_name_t *network_name);
uint8_t *ws_wp_nested_gtkhash_read(uint8_t *data, uint16_t length);

#ifdef WISUN_FAN_CORE_1_1
bool ws_wp_nested_pom_read(uint8_t *data, uint16_t length, struct ws_pom_ie_s *ptr_pom_ie);
bool ws_wp_nested_jm_read(uint8_t *data, uint16_t length, struct ws_jm_ie_s *ptr_jm_ie);
bool ws_res_wp_pan_ffn_wide_ies_read(struct ws_pan_information_s* ws_pan_info, uint8_t *payloadIeList, uint16_t payloadIeListLength);
#endif //WISUN_FAN_CORE_1_1

bool ws_wp_nested_vp_read(uint8_t *data, uint16_t length, struct ws_vp_ie *vp_ie);
bool ws_wp_nested_vp_get(uint8_t *data, uint16_t length, struct ws_vp_ie *vp_ie);

#endif /* WS_IE_LIB_H_ */
