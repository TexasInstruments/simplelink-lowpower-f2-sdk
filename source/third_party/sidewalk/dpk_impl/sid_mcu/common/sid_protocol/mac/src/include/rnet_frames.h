/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef RNET_FRAMES_H
#define RNET_FRAMES_H

#include <sid_protocol_defs.h>
#include <sid_protocol_opts.h>
#include <sid_asl_parser.h>
#include "rnet_time.h"
#include "rnet_mac_util.h"
#include "rnet_dev_addr_def.h"
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*NW layer header support macros*/
#define MAX_AUTH_CODE_SZ                        16
#define SEC_REF_COUNTER_VAL_SZ                  sizeof(uint32_t)
#define MAX_AUTH_CODE_SZ_WHILE_COUNTER_AS_REF   (MAX_AUTH_CODE_SZ - SEC_REF_COUNTER_VAL_SZ)

#define RNET_NWL_HDR_IS_FRAGMENTED_OFFSET       0x7
#define RNET_NWL_HDR_IS_FRAGMENTED_MASK         0x1
#define RNET_NWL_HDR_FRAME_PLD_TYPE_OFFSET      0x6
#define RNET_NWL_HDR_FRAME_PLD_TYPE_MASK        0x1
#define RNET_NWL_HDR_ENCR_ENA_OFFSET            0x5
#define RNET_NWL_HDR_ENCR_ENA_MASK              0x1
#define RNET_NWL_HDR_AUTH_CODE_SIZE_OFFSET      0x2
#define RNET_NWL_HDR_AUTH_CODE_SIZE_MASK        0x7
#define RNET_NWL_HDR_SEC_TOPOLOGY_OFFSET        0x1
#define RNET_NWL_HDR_SEC_TOPOLOGY_MASK          0x1
#define RNET_NWL_HDR_EXTENSION_BIT_FLAG_OFFSET  0x0
#define RNET_NWL_HDR_EXTENSION_BIT_FLAG_MASK    0x1
/*NW Layer extension header macros*/
#define RNET_EXT_HDR_SEC_REF_OFFSET             0x7
#define RNET_EXT_HDR_SEC_REF_MASK               0x1
#define RNET_EXT_HDR_SEC_PROCESS_TYPE_OFFSET 0x6
#define RNET_EXT_HDR_SEC_PROCESS_MASK 0x1
#define SEC_PROCESS_TYPE_FSK_WAN_JOIN_KEY_SIZE 16

typedef enum {
    SUB_MOD_NO_PREFERENCE = 0,
    SUB_MOD_LDR = 1,
    SUB_MOD_HDR = 2,
    SUB_MOD_HDR_LORA = 3,
    SUB_MOD_SDB = 4
} tx_sub_mod_t;

/*
 * Key type enumeration
 */
enum rnet_sec_topology {
    PAN_SEC_TOPOLOGY = 0,
    WAN_SEC_TOPOLOGY
};

// structure to pass additional pkt context
typedef struct {
    uint8_t is_pass_through_data : 1;
    uint8_t is_ffs_frame : 1;
} rnet_pkt_additional_data_t;

/*Node DB structure definition*/
typedef struct rnet_node_t {
    U8 dev_id[RNET_DEV_ID_SZ];
    U8 proto_ver :4;
    U8 tx_active:1;
#if defined(GW_SUPPORT) && GW_SUPPORT
    U8 epoch_time_updated :1;
    U8 fsk_wan_allocated_query_sent :1;
    U8 fsk_wan_allocated_query_received :1;
    U8 fsk_wan_last_join_resp_code;
    U16 last_seqno;
#else
    U8 reserve :1;
#endif
    tx_sub_mod_t data_rate;
    enum rnet_sec_topology sec_topology;
    time_def window_interval;
    time_def last_used_tm; //check for its usage
    void *next_node;                    //next node in nodeDB
    U8 tx_count;                        //number of packets queued for this node
    S8 txq_head_idx;                    //tr_store index of the first packet for this node
    S8 txq_tail_idx;                    //tr_store index of the last packet for this node
    struct {
        bool hdr_unicast_sch_enable;
        U8 hdr_unicast_offset;
        U8 hdr_unicast_period;
    }hdr_attributes;
    time_def dev_epoch_time;
} rnet_node_t;

#define IS_UNDEFINED_HDR_UNICAST_SCH(X) ((X) == NULL || (X)->hdr_attributes.hdr_unicast_sch_enable == false || \
                                        ((X)->hdr_attributes.hdr_unicast_offset == 0 && (X)->hdr_attributes.hdr_unicast_period == 0))

/* Look up table for Group info*/
typedef struct {
    U8 group_num;
    U8 group_offset;
    U8 group_period;
    void* next;
} rnet_pan_group_t;

#if !defined(X86_UNIT_TEST) || !X86_UNIT_TEST
/*
 * If size changed please update wiki and edit nrf_malloc configurations for
 * products
 * https://wiki.labcollab.net/confluence/x/C0BoQg
 */
_Static_assert(sizeof(rnet_node_t) < 50, "SIZEOF RNET_NODE_T CHANGED");
#endif

// Packet/frame structures

// Link layer extended header
typedef struct {
    U8 rsvd1 :1;
    U8 rsvd2 :1;
    U8 rsvd_ext_ll_hdr_2 :1;
    U8 hdr_1_value :5;
} ll_hdr_ext_1_t;

// Link layer frame structure
typedef struct {
    U8 frame_type :3;
    U8 epoch_bit :1;
    U8 proto_ver :4;
    U8 src_frmt :2;
    U8 dst_frmt :2;
    U8 nw_frame_ctrl_inc :1;
    U8 ack_req :1;
    U8 suppress_broadcast_ack :1;
    U8 ext_ll_hdr_1_ena :1;
    U8 src_sz :3;
    U8 dst_sz :3;
    U8 seqn_sz :2;
    U8 src[RNET_DEV_ID_SZ];
    U8 dst[RNET_DEV_ID_SZ];
    U32 seqn;
    // Extended header if ext_ll_hdr_1_ena is set
    ll_hdr_ext_1_t ext_ll_hdr_1;
    U16 sleepy_device;
} ll_frame_t;
//Initializes LL frame with its defaults
void rnet_create_default_ll_frame_header(ll_frame_t* ll_frame_header);

/*Security reference enumeration*/
enum{
    SEC_REF_GCS = 0,
    SEC_REF_COUNTER
};

typedef struct {
    uint32_t counter_val;
    uint8_t auth_code[MAX_AUTH_CODE_SZ_WHILE_COUNTER_AS_REF];
}nw_sec_ref_counter_support_t;

typedef struct{
    uint8_t auth_code[MAX_AUTH_CODE_SZ];
}nw_sec_ref_gcs_support_t;

typedef union {
    nw_sec_ref_counter_support_t nw_sec_ref_counter_support;
    nw_sec_ref_gcs_support_t nw_sec_ref_gcs_support;
}nw_auth_support_t;

enum {
    SEC_PROCESS_TYPE_DEFAULT  = 0x0, //Default must be zero for backward compatibility
    SEC_PROCESS_TYPE_FSK_WAN_JOIN = 0x1, //MSB of the non default process types must be 1
};

/*
 * @brief Network layer frame structure
 *      if <ext_hdr> is set, nw header is extended 1B
 *      if in the ext_hdr Bit_0 is set, security reference to be used is COunter. The counter value is 4B and passed with the auth_code (12B Max).
 *      for further detail, please visit: https://doorbot.atlassian.net/wiki/spaces/RL/pages/634749741/WAN+Support+with+HDR+LDR+Rate+Switching
 */
typedef struct {
    U8 is_fragment :1;
    U8 frame_pld_type :1;
    U8 encr_ena :1;
    U8 auth_code_sz :3;
    U8 sec_topology :1; // 0- PAN key; 1-WAN
    U8 ext_hdr :1;      //if ext_hdr bit (previously rsvd) is set, nw header is extended 1 byte.

    /*Extended Header (1B)--if ext_hdr bit is set */
    U8 ext_hdr_sec_ref:1;       // security reference-> 1 = COUNTER; security reference-> 0 or absence implies GCS;
    U8 ext_hdr_sec_process_type:1; // MSB 0 indicates SEC_PROCESS_TYPE_DEFAULT.
    U8 ext_hdr_rsvd:6;

    /*fragment info: if <is_fragment> bit is set*/
    U8 fragment_info;

    nw_auth_support_t nw_auth_support;
    // rnet_pkt_t* wrapped_frm;
} nwl_frame_t;

typedef struct {
    U8 encr_ena :1;
    U8 auth_code_sz :3;
    U8 ext_hdr :1;
    U8 sec_ref :1;
    U8 is_pass_through_auth_tag:1;
    U8 sec_topology :1;
    U8 sec_process_type :1;
    U32 counter_val;
    U8 auth_code[MAX_AUTH_CODE_SZ];
} nwl_opts_t;


// App support layer frame structure
typedef struct {
    U8 opc :2;
    U8 seqn_inc :1;
    U8 rsp_req :1;
    U8 addr_enc :4;
    U8 seqn_sz :2;
    U32 seqn;
    U8 sec_ena;
    U8 pld_sz;
    U8 pld_hdr;
    U8 *p_pld;
    U32 addr;
} asl_frame_t;

// "cmd" translation layer
typedef struct {
    U16 class_id;
    U16 id;
    U8 data_hdr;
    U8 data_sz;
    U8 expl_data_sz;
    U8 status_code;
    U8* data;
} cmd_frame_t;

typedef struct{
    uint8_t encr_pld_sz_byte;
    uint8_t *encr_pld;
} encr_data_t;


#if UNIFIED_ASL_SUPPORT
typedef struct {
   enum nw_data_format_id id;
   union nw_data format;
   size_t nw_data_format_size;
   uint64_t random_pad_size;
   bool is_frame_asl_encoded;
   void (*notify_piggyback_status)(void *ctx, sid_error_t err);
   void *ctx;
} __attribute__((packed)) rnet_unified_asl_nw_data_t;

typedef struct {
    asl_frame_t *asl_frame;
    cmd_frame_t *cmd;
    U8 *buf;
    U16 buf_max;
    uint32_t *sec_ref;
    bool cmd_frame_has_payload;
    bool is_nw_data_inc;
    size_t link_app_mtu;
    size_t link_max_asl_payload;
    rnet_unified_asl_nw_data_t *nw_data;
} rnet_unified_encode_asl_t;

typedef struct {
    asl_frame_t *asl_frame;
    bool is_nw_data_inc;
    enum nw_data_format_id id;
    union nw_data format;
    size_t pad_buffer_length;
} rnet_unified_decode_asl_t;
#endif

// Packet structure
typedef enum {
    RNET_PKT_PLD_TYPE_NONE = 0,
    RNET_PKT_PLD_TYPE_ASL_FRAME,
    RNET_PKT_PLD_TYPE_ENCR_DATA
} rnet_pkt_pld_type_t;

typedef struct {
    void* ctx;
    ll_frame_t ll_frame;
    nwl_frame_t nwl_frame;
    union {
        encr_data_t encr_data;
        asl_frame_t asl_frame;
    };
    cmd_frame_t cmd_frame;
#if UNIFIED_ASL_SUPPORT
    bool is_nw_data_inc;
    rnet_unified_asl_nw_data_t nw_data;
#endif
    rnet_pkt_pld_type_t pld_type;
    bool unified_asl;
    tx_sub_mod_t data_rate;
    /*
     * Reference to put additional
     * supporting parameters which
     * are not part of the orthodox
     * frame format
     * */
} rnet_pkt_t;

/* frame encode interface structs
* @brief: The members of this struct are used as
* input params
*/
typedef struct {
    rnet_pkt_t* rnet_pkt;
    U8* buf;
    U16 max_pkt_sz;
    U8 trsmt_on_dflt_chnl;
    bool is_pass_through;
    bool skip_encr;
    // size of packet input to encryption engine
    U8 enc_ip_sz;
} rnet_pkt_encode_input_t;

// Link Layer Frame Defines
#define RNET_LL_FT_BCN 0x0
#define RNET_LL_FT_DATA 0x1
#define RNET_LL_FT_NWDATA 0x2
#define RNET_LL_FT_ACK 0x3
#define RNET_LL_FT_PROBE_REQ 0x4
#define RNET_LL_FT_PROBE_RESP 0x5
#define RNET_LL_FT_RESERVED 0x6

#define RNET_LL_SRC_FRMT_CLD 0x0
#define RNET_LL_SRC_FRMT_DEVID 0x1
#define RNET_LL_SRC_FRMT_HNDL 0x2
#define RNET_LL_DST_FRMT_CLD 0x0
#define RNET_LL_DST_FRMT_DEVID 0x1
#define RNET_LL_DST_FRMT_HNDL 0x2
#define RNET_LL_DST_FRMT_GROUPID 0x3
#define RNET_LL_FRAME_CTRL_NOT_INCL 0x0
#define RNET_LL_FRAME_CTRL_INCL 0x1
#define RNET_LL_ACK_NOT_REQ 0x0
#define RNET_LL_ACK_REQ 0x1
#define RNET_LL_EXT_LL_HDR_1_NOT_ENA 0x0
#define RNET_LL_EXT_LL_HDR_1_ENA 0x1

// Network Layer Frame defines
#define RNET_NWL_ENCR_DISABLED 0x0
#define RNET_NWL_ENCR_ENABLED 0x1

#define RNET_NWL_AUTH_NOT_INCL 0x0
#define RNET_NWL_AUTH_SIZE_2B  0x1
#define RNET_NWL_AUTH_SIZE_4B  0x2
#define RNET_NWL_AUTH_SIZE_8B  0x3
#define RNET_NWL_AUTH_SIZE_12B 0x4
#define RNET_NWL_AUTH_SIZE_16B 0x5
#define RNET_NWL_FRAME_PLD_TYPE_WRAPPED 0x0
#define RNET_NWL_FRAME_PLD_TYPE_SUPPORT 0x1

// App Support Layer Frame defines
#define RNET_ASL_OPC_RD 0x0
#define RNET_ASL_OPC_WR 0x1
#define RNET_ASL_OPC_NTFY 0x2
#define RNET_ASL_OPC_RSP 0x3
#define RNET_ASL_SEQN_NOT_INCL 0x0
#define RNET_ASL_SEQN_INCL 0x1
#define RNET_ASL_RSP_NOT_REQ 0x0
#define RNET_ASL_RSP_REQ 0x1

#define RNET_FRAME_ERROR_SECURITY                     (-1)
#define RNET_FRAME_ERROR_SECURITY_BUILD_DISABLED      (-2)
#define RNET_FRAME_ERROR_INVALID_PARAM                (-3)
#define RNET_FRAME_ERROR_NOT_FOR_US                   (-6)
#define RNET_FRAME_ERROR_NULL_PRT                     (-7)
#define RNET_FRAME_ERROR_DUPLICATE_PKT                (-15)
#define RNET_NODE_ERROR_NOT_ABLE_TO_ADD_ENTRY         (-16)
#define RNET_NODE_ERROR_OTHER                         (-17)
#define RNET_NODE_NO_MEM                              (-18)
#define RNET_UNUSED                                   (-19)
#define RNET_FRAME_ERROR_INVALID_LENGTH               (-20)
#define RNET_FRAME_ERROR_NO_MEM                       (-21)

/* Additional source formats to support flex key FLEX_KEY_INNER_SRC
 * Used to classify roaming and local traffic */
#define ID_FORMAT_NOT_HASHED                           0x1     /*Usually SID or plain text cloud id*/
#define ID_FORMAT_HASHED                               0x2     /*Usually TX-UUID or encrypted cloud id*/

/*
 * @brief: function to determine if address is for beam service
 * @param[in]: address array
 * @return: true if beam server, otherwise returns false.
 */
#define IS_BEAM_CLD_SERVICE(addr_byte_0) (!(addr_byte_0 & 0x1F))

#define RNET_FRAME_CHECK_BUF_SIZE(buf_idx, max_sz)              \
    do {                                                        \
        if (max_sz <= buf_idx)  {                               \
            return RNET_FRAME_ERROR_INVALID_LENGTH;             \
        }                                                       \
    } while(0)

#define RNET_FRAME_MEM_CHECK(buf_idx, max_sz)                   \
    do {                                                        \
        if (max_sz <= buf_idx) {                                \
            return RNET_NODE_NO_MEM;                            \
        }                                                       \
    } while(0)

// Encode/Decode and accessor functions
S16 rnet_encode_pkt(rnet_pkt_encode_input_t *pkt_enc_input);
S16 rnet_decode_pkt(rnet_pkt_t* rnet_pkt, U8* buf, U16 pkt_sz, rnet_node_t** node);
S16 rnet_decode_ll_pkt(rnet_pkt_t *rnet_pkt, U8 *buf, U16 pkt_sz, rnet_node_t **node);
S16 rnet_encode_ll(ll_frame_t* ll_frame, U8* buf, U16 buf_max);
S16 rnet_decode_ll(ll_frame_t* ll_frame, U8* buf, U16 buf_max);
S16 rnet_encode_nwl(nwl_frame_t* nwl_frame, U8* buf, U16 buf_max);
S16 rnet_decode_nwl(nwl_frame_t* nwl_frame, U8* buf, U16 buf_max);
S16 rnet_encode_asl(asl_frame_t* asl_frame, cmd_frame_t* cmd_frame, U8* buf, U16 buf_max, uint32_t *sec_ref);
S16 rnet_decode_asl(asl_frame_t* asl_frame, U8* buf, U8 buf_len, uint32_t sec_ref);
#if UNIFIED_ASL_SUPPORT
S16 rnet_encode_unified_asl(rnet_unified_encode_asl_t *u_asl_frame, bool add_padding);
S16 rnet_decode_unified_asl(rnet_unified_decode_asl_t *u_asl_frame, U8 *buf, U8 buf_len, uint32_t sec_ref, uint8_t *src, bool copy_pld);
#endif

/*!
 * @brief Link layer header size calculation
 * @param[in] ll_frame Pointer to link layer frame structure
 * @return Link layer header size of the frame
 */
uint8_t sid_calculate_ll_header_size(ll_frame_t *ll_frame);

S8 rnet_asl_from_cmd(asl_frame_t* asl_frame, cmd_frame_t* cmd_frame);
#if !UNIFIED_ASL_SUPPORT
S8 rnet_cmd_from_asl(cmd_frame_t* cmd_frame, asl_frame_t* asl_frame);
S16 rnet_decode_cmd_frame(cmd_frame_t* cmd_frame, asl_frame_t* asl_frame);
#else
S8 rnet_cmd_from_asl(cmd_frame_t* cmd_frame, asl_frame_t* asl_frame, bool unified_asl);
S16 rnet_decode_cmd_frame(cmd_frame_t* cmd_frame, asl_frame_t* asl_frame, bool unified_asl);
/*!
 * @brief Add network data blob format 3 into pkt of transaction entry
 * @param pkt: pointer to pkt of transaction entry
 * @param tag_id: network data blob format 3 TLV tag id
 * @param byte_len: length of whole TLV bytes including tag, length and value
 * return: SID_ERROR_NONE if success
 */
sid_error_t sid_add_uasl_nw_data_format_3(rnet_pkt_t *pkt,
                                            enum format_3_tag_id tag_id,
                                            enum format_3_tlv_byte_len byte_len);
#endif

S16 rnet_check_duplicate_frame(ll_frame_t* ll_frame, rnet_node_t* node);
U8 rnet_parse_auth_code_size(U8 auth_sz);

#ifdef __cplusplus
}
#endif

#endif
