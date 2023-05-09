/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef LIB_AMAZON_RINGNET_SRC_INCLUDE_RNET_CONFIG_H_
#define LIB_AMAZON_RINGNET_SRC_INCLUDE_RNET_CONFIG_H_

#include <sid_protocol_defs.h>
#include "rnet_nw_layer.h"
#include <storage_kv_keys.h>
#include "rnet_mac_util.h"
#include <sid_protocol_regulatory_config.h>
#include <sid_protocol_dcr_config.h>
#include "rnet_mac_submodule_common.h"
#include <sid_protocol_opts.h>
#include <sid_900_cfg.h>
#include "rnet_dev_addr_def.h"
/*!
 * @brief: Protocol configuration flag to control
 * the use PAN security topology only
 */
#define HALO_PROTCL_USE_PAN_SECURITY_ONLY       PAN_SUPPORT

#define MAX_DFU_PARAMS_LENGTH                   3
#define NUM_HDR_PRITY                           3
#define DFLT_LDR_CHANNEL                        26
#define DFLT_WAN_DEV_LI_MODE                    RNET_LI_MODE_ENABLE

/*Macros for Device profiles*/
#define RNET_RF_CONFIG_VERSION                  4
#define RNET_RF_LI_MODE_VERSION                 2
#define RNET_RF_LDR_CHNL_VERSION                4
#define DEFAULT_GW_MODE_WITH_LDR                MR_GW
#define DEFAULT_GW_MODE_WITHOUT_LDR             HDR_GW
#define DEFAULT_PAN_END_DEV_MODE                HDR_ONLY_DV
#define DEFAULT_WAN_END_DEV_MODE                LDR_ONLY_DV
#define PRODUCT_GATEWAY                         0
#define PRODUCT_PAN_END_DEV                     1
#define PRODUCT_WAN_END_DEV                     2

/* End device macros*/
#define PRTCL_ON                                1
#define HDR_ON                                  1<<1
#define LDR_ON                                  1<<2
#define BCN_ON                                  1<<3
#define DFU_ON                                  1<<4
#define PROBE_ON                                1<<5
#define HDR_LORA_ON                             1<<6
#define SDB_GW_ON                               1<<7

#if GW_SUPPORT
/* Gateway macros*/
#define MR_GW_CFG_VAL                           (PRTCL_ON|HDR_ON|LDR_ON|BCN_ON|DFU_ON|PROBE_ON|SDB_GW_ON)
#define HDR_GW_CFG_VAL                          (PRTCL_ON|HDR_ON|BCN_ON|DFU_ON|PROBE_ON|SDB_GW_ON)
#define LDR_GW_CFG_VAL                          (PRTCL_ON|LDR_ON|PROBE_ON)
#define LDR_DISTRESS_GW_WITH_BCN_CFG_VAL        (PRTCL_ON|LDR_ON|BCN_ON)
#define LDR_DISTRESS_GW_WITHOUT_BCN_CFG_VAL     (PRTCL_ON|LDR_ON)
#define HDR_HALO1_GW_CFG_VAL                    (PRTCL_ON|HDR_ON|BCN_ON|DFU_ON|PROBE_ON|HDR_LORA_ON)
#define PRTCL_OFF_GW_CFG_VAL                    0
#else
/* End Device (light & buddy) macros*/
#define HDR_ONLY_DV_CFG_VAL                     (PRTCL_ON|HDR_ON|BCN_ON|PROBE_ON)
#define LDR_ONLY_DV_CFG_VAL                     (PRTCL_ON|LDR_ON)
#define HDR_AND_LDR_DV_CFG_VAL                  (PRTCL_ON|HDR_ON|LDR_ON|BCN_ON|PROBE_ON)
#define PRTCL_OFF_DV_CFG_VAL                    0
#endif

//version settings word size
//size of parameters in flash record used for bit manipulation
#define VER_BASE_SZ                             0
#define DEV_MODE_SZ                             3
#define DEV_TYPE_SZ                             3
#define VER_RSVD_SZ                             2
#define CFG_VER_SZ                              4
#define PRTCL_VER_SZ                            4
#define PRTCL_FLAG_SZ                           1
#define GRP_PRD_SZ                              8
#define UNI_PRD_SZ                              8
#define GRP_OFF_SZ                              8
#define UNI_OFF_SZ                              8
#define LDR_CHNL_SZ                             8
#define LI_MODE_SZ                              1

//Define protocol sync behavior settings
#if RNET_MAC_MODE_FSK
#define RNET_MAC_CHANNEL_HOPPING                1
#else
#define RNET_MAC_CHANNEL_HOPPING                0
#endif

// Number of HALO 1.0 P2P channels
#define RNET_NUM_OF_P2P_CHNLS                   8

// Number of WAN LDR channels
#define RNET_NUM_OF_LDR_CHNLS                   SID_REG_NA_NUM_OF_CHANNELS_500KHZ

//Macros for PAN wake up schedule
#define RNET_WAKEUP_TX_ENABLED                  (0x1)
#define RNET_WAKEUP_RX_ENABLED                  (0x2)
#define RNET_DFLT_GRP_PRDCITY                   RNET_MAC_PHASE_0_GROUP_PERIODICITY
#define RNET_DFLT_UNI_PRDCITY                   RNET_MAC_PHASE_0_UNICAST_PERIODICITY

typedef enum {
    DEV_IN_REG_OPERATION = 0,
    CLIENT_DEV = 1,
    ASSISTING_DEV = 2
} rnet_dev_role_t;

/* struct for gateway dedicated link params */
typedef struct {
    /* Note: tells, what the device role should be in this dedicated session FTP_CLIENT or ASSITING DEV */
    rnet_dev_role_t dev_role;
    /* Note: peer device power class */
    enum sid_device_profile_id peer_device_class;
    /* Note: Peer id of the other device: if the device role is Assisting GW, the ID will represent the FTP client
     * or vice versa. */
    uint8_t peer_dev_id[RNET_DEV_ID_SZ];
    /* Note: Tells the channel for the dedicated link */
    uint8_t channel;
    uint8_t state;
} rnet_dedicated_link_init_param_t;

//TODO: hide struct from public .h file
typedef struct {

    //LL parameters - Used by MAC
   struct {
        U32                   mfg_version; // Only to indicate record # in flash
        U8                    p2p_chnl;
        U8                    pan_id[RNET_PAN_ID_SZ];
        U8                    dev_id[RNET_DEV_ID_SZ];
        U8                    grp_num;
        int8_t                rf_power[SID_PAL_RADIO_DATA_RATE_MAX_NUM];
#if defined(GW_SUPPORT) && GW_SUPPORT
        U8                    sidewalk_nw_flag; //This flag is used to enable/disable sidewalk network
#endif
    }ll_params;

    //MAC submodule switches
        U16                     ptcl_swtch;

    struct {
#if !defined(GW_SUPPORT) || !GW_SUPPORT
        U8                      max_bcn_miss;
#endif
        U8                      bcn_interval_factor;
    }bcn_params;

    struct {
        U8                      grp_periodicity;
        U8                      uni_periodicity;
        U8                      grp_offset;
        U8                      uni_offset;
        U8                      gw_periodicity;
        U8                      gw_offset;
#if !defined(GW_SUPPORT) || !GW_SUPPORT
        U8                      grp_wakeup_flag;
        U8                      uni_wakeup_flag;
#endif
    }hdr_params;

    struct {
        U8                      ldr_chnl;
        U16                     ka_intvl;
        U8                      li_mode; //0-short interleaver mode; 1-long interleaver mode
    }ldr_params;

    struct {
        U8                      dflogic_enable:1;
    }schdlr_params;//dflogic

    struct {
       U8                       auth_enable:1;
       U16                      homepan_scan_intvl; //interval used to re-start GWD after scan timeout in assisted mode
    }probe_params;

    struct {
        U8                      dev_profile_id;
        U8                      rx_count;
    } dev_cls_fsk_params;                                // Note: support on GW side not required for device classes
    struct {
        U8                      dev_profile_id;
        U8                      rx_count;
        U16                     ldr_prdcty;
    } dev_cls_ldr_params;                                // Note: support on GW side not required for device classes
#if defined(GW_SUPPORT) && GW_SUPPORT
    rnet_dedicated_link_init_param_t dedicated_link_param;
#endif
} rnet_config_settings_t;

typedef struct {
    U8                         ll_params:1;
    U8                         prtcl_swtch:1;
    U8                         bcn_params:1;
    U8                         hdr_params:1;
    U8                         ldr_params:1;
    U8                         probe_params:1;
    U8                         schdlr_params:1;
}rnet_config_sections_t;

typedef union {
    rnet_config_sections_t cfg_ctx_sec;
    U16 cfg_ctx_word;
}rnet_config_context_t;

typedef enum {
    BCN = 0,
    HDR,
    LDR,
    PROBE,
    SCHDLR,
    DFU
}submod_name_t;

typedef rnet_error_t (*ntfy_fc_ptr)(rnet_config_context_t);

typedef struct {
    submod_name_t              ID;
    U8                         is_enabled:1;
    rnet_config_context_t      ctx_mask;
    ntfy_fc_ptr notify_func;
}rnet_cfg_ntfy_node_t;

typedef enum {
    LL_PARAMS = 0,
    PTCL_SWTCH = 1,
    BCN_PARAMS = 2,
    HDR_PARAMS = 3,
    LDR_PARAMS = 4,
    PROBE_PARAMS = 5,
    SCHDLR_PARAMS = 6
}config_param_key_t;

typedef enum {
    MFG_VERSION = 0,
    P2P_CHANNEL = 1,
    GRP_NUM = 2,
    PAN_ID = 3,
    DEV_ID = 4,
    RF_POWER = 5,
    SIDEWALK_NW_FLAG = 7
}ll_params_key_t;

typedef enum {
    MAC_ENABLE = 0,
    HDR_ENABLE = 1,
    LDR_ENABLE = 2,
    BCN_ENABLE = 3,
    DFU_ENABLE = 4,
    PROBE_ENABLE = 5,
    HALO1_ENABLE = 6,
    SDB_ENABLE = 7
}protocol_submodule_key_t;

typedef enum {
    MAX_BCN_MISS = 0,
    BCN_INTERVAL_FACTOR = 1
}bcn_params_key_t;

typedef enum {
    GRP_PERIODICITY = 0,
    UNI_PERIODICITY = 1,
    GRP_OFFSET = 2,
    UNI_OFFSET = 3
}hdr_params_key_t;

typedef enum {
    LDR_CHANNEL = 0,
    KA_INTERVAL = 1,
    LI_MODE = 2
}ldr_params_key_t;

typedef enum {
    DF_LOGIC_ENABLE = 1
}schdlr_params_key_t;

typedef enum {
    AUTH_ENABLE = 0,
    HOMEPAN_SCAN_INTVL = 1
}probe_params_key_t;

typedef enum {
    DEFICIT_LOGIC_ENABLE = 0,
    PROBE_AUTH_ENABLE = 1
}interal_mac_params_key_t;

typedef enum {
    DV_DEV_MODE = 0,
    DV_DEV_TYPE = 1,
    DV_CFG_VER = 2,
    DV_PRTCL_VER = 3,
    DV_PRTCL_FLAG = 4
}dev_versn_params_key_t;

typedef struct rnet_cfg_set_node {
    RNET_LIST_ENTRY(rnet_cfg_set_node); /*< Token used by sortlist. */
    config_param_key_t config_key;
    int sub_key;
}rnet_cfg_set_node;

/*
 * If size changed please update wiki and edit nrf_malloc configurations for
 * products
 * https://wiki.labcollab.net/confluence/x/C0BoQg
 */
#if !defined(X86_UNIT_TEST) || !X86_UNIT_TEST
_Static_assert(sizeof(rnet_cfg_set_node) == 12, "SIZEOF RNET_CFG_SET_NODE CHANGED");
#endif

typedef enum {
    MR_GW = 0,
    HDR_GW = 1,
    LDR_GW = 2,
    LDR_DISTRESS_GW = 3,
    HALO1_MIGR_GW = 4
}gw_modes_t;

typedef enum {
    HDR_AND_LDR_DV = 0,
    HDR_ONLY_DV = 1,
    LDR_ONLY_DV = 2
}end_dev_modes_t;

typedef enum {
    READ,
    WRITE
}cfg_opt_t;

typedef struct {
    rnet_error_t error_code;
    U8 dev_type;
    U8 prtcl_status;
    U8 dev_mode;
    U8 mac_enable;
    U8 hdr_enable;
    U8 ldr_enable;
    U8 bcn_enable;
    U8 dfu_enable;
    U8 probe_enable;
}rnet_cloud_ntfy_pld_t;

/* struct for distress gwd params */
typedef struct {
    uint8_t format_id:4;
    uint8_t ctrl_flag:4;
    uint8_t max_rand_del_off;           //seconds
    uint8_t inter_msg_del_pre_ack;      //minutes
    uint8_t inter_msg_del_post_ack;     //minutes
}rnet_gw_distress_config_t;

/* enum for tags for temporary protocol record */
typedef enum {
    LORA_LI_MODE = 0
}rnet_tmp_prtcl_exp_tag_t;

/* tlv record struct for cfg use */
typedef struct {
    rnet_tmp_prtcl_exp_tag_t tag;
    uint8_t len;
    uint8_t val;
}rnet_config_tlv_frmt_t;

/**
 * @brief Initialize config structure with values imported from flash
 * @param[in] rnet_config_settings_t Structure to store config params
 */
uint8_t rnet_config_init(rnet_config_settings_t *);

/**
 * @brief Import from initialized structure to config and factory_config
 * @param[in] rnet_config_settings_t initialized_structure
 */
void rnet_store_config_params(rnet_config_settings_t *);

/**
 * @brief Getter function for config structure
 * @param[out] rnet_config_settings_t* ptr to the struct
 */
rnet_config_settings_t* rnet_get_config(void);

/**
 * @brief Facilitator for reset cfg paramters command
 */
void rnet_restore_config_params();

/**
 * @brief This function forms the payload for get/set device
 * profile cmds from cloud
 * @param[in] uint8_t* buffer ptr
 * @ret size of payload
 */
uint8_t rnet_create_dev_cfg_pld(uint8_t *buf);

/**
 * @brief This function forms the payload for get a copy of
 * config from cloud. Entire config structure along with dev profile
 * is sent.
 * @param[in] uint8_t* buffer ptr
 * @ret size of payload
 */
uint8_t rnet_create_cfg_pld(uint8_t *buf);

/**
 * @brief Getter function to supply config param value to MAC
 * submodules when they query it
 * @param[in] config_param_key_t queried config param
 * @param[in] void* memory location to store value
 * @param[in] uint16_t value size
 */
rnet_error_t rnet_get_config_param(config_param_key_t config_key, void* sub_key, U8* value, uint16_t value_size);

/**
 * @brief Setter function to change config param value when
 * indicated by MAC submodules
 * @param[in] config_param_key_t config_param to be changed
 * @param[in] void* new value
 * @param[in] uint16_t size of new value
 */
rnet_error_t rnet_set_config_param(config_param_key_t config_key, void* sub_key, U8* value, uint16_t value_size);

/**
 * @brief Function to continuously check for updates in config
 * params and if update is indicated, call appropriate function
 * to write into flash. (Happens out of interrupt context)
 */
void rnet_process_config_change();

/**
 * @brief Function to record submodule's info and notifier function
 * @param[in] submod_name_t submodule name
 * @param[in] rnet_config_context_t context mask of the sbmodule
 * @param[in] submodule's notify function
 */
void rnet_cfg_notifier_register(submod_name_t sb_ID,rnet_config_context_t sb_ctx_mask, ntfy_fc_ptr sb_ntfy_fc_ptr);

/**
 * @brief Function to disable sbmodule's notifier
 * @param[in] submod_name_t submodule name
 */
void rnet_cfg_notifier_deregister(submod_name_t sb_ID);

/**
 * @brief Function to iterate through the notifier array
 * and call submodule notifier is submodule is enabled
 * @ret rnet_error_t error info if any
 */
rnet_error_t rnet_cfg_notify();

/**
 * @brief Function for getting pointer on configuration context
 * @param[out] pointer to place for pointer on the context
 */
void rnet_config_context_get(rnet_config_context_t ** ctx);

/**
 * @brief Function to manage device profiles and carry out the
 * required enabling/disabling of submodules
 * @param[in] cfg_opt_t read/write operation
 * @param[in] int32_t desired mode of operation/-1 if reading
 * @param[in] bool protocol enable/disable flag - checked to
 * see if mode should be applied or just written to flash
 * @ret rnet_error_t error info
 */
rnet_error_t rnet_cfg_mgr(cfg_opt_t opt, int32_t new_mode, bool prtcl_status);

/**
 * @brief Function to get protocol status from RAM - enabled/disabled
 * @ret bool status flag
 */
bool rnet_get_prtcl_status();

/**
 * @brief Function to enable/disable protocol
 * @param[in] bool desired status
 * @ret rnet_error_t error info
 */
rnet_error_t rnet_change_prtcl_status(bool new_status);

/*!
 * @brief: Function to enable/disable mac submodules
 * @param[in] bitmap submodule config flags
 * @ret rnet_error_t error info
 */
rnet_error_t rnet_prtcl(uint16_t bitmap);

/********************************** Configuration Abstraction function declarations ******************************************/

/**
 * @brief Get/Set LDR Channel value from flash while checking the cfg and protocol versions
 * If there's a version mismatch, default LDR channel will be loaded
 * @param[in] uint8_t* (get)ldr_channel buffer/ (set)new ldr_channel value
 * @param[in] cfg_opt_t READ/WRITE operation
 * @ret rnet_error_t error code
 */
rnet_error_t rnet_ldr_chnl_ver_chckr(uint8_t *ldr_channel, cfg_opt_t opt);

#if defined(WAN_SUPPORT) && WAN_SUPPORT
/**
 * @brief Retrieve Home PAN scan Interval value from flash
 * @param[in] uint8_t location to store read value
 * @ret rnet_error_t error code
 */
rnet_error_t get_homepan_scan_intvl(U16 *scan_int);

/**
 * @brief Set Home PAN scan Interval value in flash
 * @param[in] uint8_t new value
 * @ret rnet_error_t error code
 */
rnet_error_t set_homepan_scan_intvl(U16 scan_int);

#endif

#if !defined(GW_SUPPORT) || !GW_SUPPORT
/**
 * @brief Retrieve Max BCN miss value from flash
 * @param[in] uint8_t location to store read value
 */
rnet_error_t get_max_bcn_miss(U8 *bcn_miss);

/**
 * @brief Set Max BCN miss value in flash
 * @param[in] uint8_t new value
 */
rnet_error_t set_max_bcn_miss(U8 bcn_miss);
#endif // !defined(GW_SUPPORT) || !GW_SUPPORT
/**
 * @brief Retrieve BCN interval value from flash
 * @param[in] uint8_t location to store read value
 */
rnet_error_t get_bcn_interval(U8 *bcn_intrvl_fact);

/**
 * @brief Set BCN interval value in flash
 * @param[in] uint8_t new value
 */
rnet_error_t set_bcn_interval(U8 bcn_intrvl_fact);

/**
 * @brief Retrieve MAC settings word from flash
 * @param[in] uint8_t location to store read value
 * @param[in] storage_kv_key_t key to distinguish mac words
 */
rnet_error_t get_mac_settings(U8 *mac_settings_word, storage_kv_key_t config_key);

/**
 * @brief Retrieve individual submodule settings from flash
 * @param[in] bool* location to store read value
 * @param[in] protocol_submodule_key_t submodule key
 */
rnet_error_t get_submodule_settings(protocol_submodule_key_t sb_key, bool *submodule_enable);

/**
 * @brief Set individual submodule settings in flash
 * @param[in] protocol_submodule_key_t submodule key
 * @param[in] bool new value
 */
rnet_error_t set_submodule_settings(protocol_submodule_key_t sb_key, bool submodule_enable);

/**
 * @brief Update individual submodule settings in config structure
 * @param[in] rnet_config_settings_t config structure
 * @param[in] protocol_submodule_key_t submodule key
 * @param[in] bool new value
 */
void set_submodule_settings_helper(rnet_config_settings_t *config_settings, protocol_submodule_key_t sb_key, bool submodule_enable);

/**
 * @brief Retrieve individual mac settings from flash
 * @param[in] bool* location to store read value
 * @param[in] interal_mac_params_key_t param key
 */
rnet_error_t get_mac_param_settings(interal_mac_params_key_t mp_key, bool *mac_param_enable);

/**
 * @brief Set mac params in flash
 * @param[in] protocol_submodule_key_t submodule key
 * @param[in] bool new value
 */
rnet_error_t set_mac_param_settings(protocol_submodule_key_t sb_key, bool submodule_enable);

/**
 * @brief Update individual submodule settings in config structure
 * @param[in] rnet_config_settings_t* config structure
 * @param[in] interal_mac_params_key_t param key
 * @param[in] bool new value
 */
void set_mac_param_settings_helper(rnet_config_settings_t *config_settings, interal_mac_params_key_t mp_key, bool mac_param_enable);

/**
 * @brief Retrieve device version settings word from flash
 * @param[in] uint16_t* location to store read value
 */
rnet_error_t get_dv_settings(U16 *ver_set);

/**
 * @brief Retrieve individual dev version settings from flash
 * @param[in] uint8_t* device mode
 * @param[in] uint8_t* device type
 * @param[in] uint8_t* configuration version
 * @param[in] uint8_t* protocol version
 */
rnet_error_t get_dv_param_settings(uint8_t *dev_mode, uint8_t *dev_type, uint8_t *cfg_ver, uint8_t *prtcl_ver);

/**
 * @brief Set device version settings in flash
 * @param[in] dev_versn_params_key_t dv value to be updated
 * @param[in] uint8_t new value
 */
rnet_error_t set_dv_settings(dev_versn_params_key_t param_name, uint8_t param_value);

/**
 * @brief Function to get default config
 * @param[in] uint8_t* device mode
 * @param[in] uint8_t* device type
 */
void rnet_get_dflt_dev_cfg(uint8_t *curr_dev_mode, uint8_t *curr_dev_type);

/**
 * @brief Function verify version such that correct device
 * profile is loaded on boot up (on READ). If there is a version mismatch, load default values.
 * On WRITE, the input dev_mode is stored in flash.
 * @param[in] uint8_t* device mode
 * @param[in] uint8_t* device type
 * @param[in] cfg_opt_t read/write operation
 */
rnet_error_t rnet_cfg_ver_chckr (uint8_t *dev_mode, uint8_t *dev_type, cfg_opt_t opt);

/**
 * @brief Retrieve prtcl version settings word from flash
 * @param[in] uint16_t* location to store read value
 */
rnet_error_t get_prtcl_settings(U16 *ver_set);

/**
 * @brief Retrieve individual prtcl version settings from flash
 * i.e. status, cfg version and prtcl version
 * @param[in] uint8_t* prtcl status
 * @param[in] uint8_t* configuration version
 * @param[in] uint8_t* protocol version
 */
rnet_error_t get_prtcl_param_settings(uint8_t *prtcl_status, uint8_t *cfg_ver, uint8_t *prtcl_ver);

/**
 * @brief Set individual prtcl version settings in flash
 * @param[in] dev_versn_params_key_t value to be updated
 * @param[in] uint8_t new value
 */
rnet_error_t set_prtcl_settings(dev_versn_params_key_t param_name, uint8_t param_value);

/**
 * @brief Function to get default prtcl config depending upon device type
 * @param[in] uint8_t* prtcl status
 */
void rnet_get_dflt_prtcl_cfg(uint8_t *curr_prtcl_status);

/**
 * @brief Function verify prtcl version word such that correct protocol
 * status is loaded on boot up (on READ). If there is a version mismatch, load default values.
 * On WRITE, the input prtcl_status is stored in flash.
 * @param[in] uint8_t* prtcl status
 * @param[in] cfg_opt_t Read/Write
 */
rnet_error_t rnet_prtcl_ver_chckr (uint8_t *prtcl_status, cfg_opt_t opt);

#if defined(GW_SUPPORT) && GW_SUPPORT
/*
 * @brief: Flash function to get sidewalk network flag (opt in/opt out)
 * @param U8* flag
 * @ret rnet_error_t
 */
rnet_error_t get_sidewalk_nw_flg(U8 *gw_flag);

/*
 * @brief: Flash function to set sidewalk network flag (opt in/opt out)
 * @param U8 flag
 * @ret rnet_error_t
 */
rnet_error_t set_sidewalk_nw_flg(U8 gw_flag);
#endif

/**
 * @brief Retrieve hdr periodicity settings word from flash
 * @param[in] uint16_t* location to store read value
 */
rnet_error_t get_hdr_prdcty_settings(U16 *hdr_prd_param);

/**
 * @brief Retrieve individual hdr periodicity settings from flash
 * @param[in] uint8_t* grp_prdcty
 * @param[in] uint8_t* unicast_prdcty
 */
rnet_error_t get_hdr_prdcty_param_settings(uint8_t *grp_prdcty, uint8_t *unicast_prdcty);

/**
 * @brief Set individual hdr periodicity settings in flash
 * @param[in] hdr_params_key_t value to be updated
 * @param[in] uint8_t new value
 */
rnet_error_t set_hdr_prdcty_settings(hdr_params_key_t param_name, uint8_t param_value);

/**
 * @brief Retrieve hdr offset settings word from flash
 * @param[in] uint8_t* location to store read value
 */
rnet_error_t get_hdr_offset_settings(U16 *hdr_prd_param);

/**
 * @brief Retrieve individual hdr offset settings from flash
 * @param[in] uint8_t* grp_offset
 * @param[in] uint8_t* unicast_offset
 */
rnet_error_t get_hdr_offset_param_settings(uint8_t *grp_offset, uint8_t *unicast_offset);

/**
 * @brief Set individual hdr offset settings in flash
 * @param[in] hdr_params_key_t value to be updated
 * @param[in] uint8_t new value
 */
rnet_error_t set_hdr_offset_settings(hdr_params_key_t param_name, uint8_t param_value);

/**
 * @brief Retrieve individual distress gwd params from flash
 * @param[in] rnet_gw_distress_config_t* gwd_params
 */
rnet_error_t get_distress_gw_params(rnet_gw_distress_config_t* gwd_params);

/*
 * @brief: Flash function to set distress gwd params
 * @param uint8_t *gwd_params
 * @ret rnet_error_t
 */
rnet_error_t set_distress_gw_params(uint8_t *gwd_params);

/**
 * @brief Function verify long interleaver mode version word such that correct li mode
 * is loaded on boot up (on READ). If there is a version mismatch, load default values.
 * On WRITE, the input li_mode is stored in flash.
 * @param[in/out] uint8_t* li mode
 * @param[in] cfg_opt_t Read/Write
 */
ret_code_t rnet_li_mode_ver_chckr(uint8_t *li_mode, cfg_opt_t opt);

/**
 * @brief Get tx power for data rate
 *
 * @param data rate
 * @param tx power
 * @ret RNET_SUCCESS on success or an apporpriate error number is returned.
 */
rnet_error_t rnet_get_tx_power_for_data_rate(uint8_t data_rate, int8_t *power);

/**
 * @brief Set tx power for data rate
 *
 * @param data rate
 * @param tx power
 * @ret RNET_SUCCESS on success or an apporpriate error number is returned.
 */
rnet_error_t rnet_set_tx_power_for_data_rate(uint8_t data_rate, int8_t power);
#endif /* LIB_AMAZON_RINGNET_SRC_INCLUDE_RNET_CONFIG_H_ */
