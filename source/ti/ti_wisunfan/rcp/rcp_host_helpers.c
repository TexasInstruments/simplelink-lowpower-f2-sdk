
#include "ns_trace.h"
#include "eventOS_event.h"
#include "eventOS_event_timer.h"
#include "ti_wisunfan_features.h"

/* api_mac includes */
#include "mbed_config_app.h"
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "advanced_config.h"
#include "nsconfig.h"
#include "net_interface.h"
#include "ns_types.h"
#include "mac_common_defines.h"

#include "6LoWPAN/MAC/mpx_api.h"
#include "6LoWPAN/ws/ws_llc.h"
#include "6LoWPAN/ws/ws_ie_lib.h"
#include "NWK_INTERFACE/Include/protocol_timer.h"
#include "NWK_INTERFACE/Include/protocol.h"
#include "MAC/rf_driver_storage.h"
// #include "NanostackTiRfPhy.h"

#include "mac_assert.h"

#ifndef LINUX_NANOSTACK
#include <inc/hw_ccfg.h>
#include <inc/hw_ccfg_simple_struct.h>
#include <inc/hw_memmap.h>
#include <inc/hw_fcfg1.h>
#include <inc/hw_types.h>

#include "mac_hl_patch.h"
#include "mac_settings.h"

#include "osal_port.h"
#include "timac_ns_interface.h"
#include "rcp_mac_common.h"
#endif

#include "nsdynmemLIB.h"
#include "application.h"

/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>

#include "nanostack/platform/arm_hal_phy.h"
#include "platform/arm_hal_interrupt.h"

#include "rcp_host.h"

#define RF_MTU 2047

/* api_mac includes */
#define TRACE_GROUP "hostHelp"

/* Stub defines */
#define SIZE_OF_EUI_LIST    75
#define MAC_FILTER_MODE_DISABLED  0
#define MAC_FILTER_MODE_ALLOW     1
#define MAC_FILTER_MODE_DENY      2

/* mac filter list implementation */
/* if mac_filter_list_mode is 0, both are disabled
 * if mac_filter_list_mode is 1, allowlist is enabled, if mac_filter_list_mode is 2, denylist is enabled. */
uint8_t mac_filter_list_mode = MAC_FILTER_MODE_DENY;
sAddrExt_t mac_eui_filter_list[SIZE_OF_EUI_LIST] = {0};

/* Stub global vars */
uint8_t enableVPIE;
uint8_t detectVPIE;
bool extAddrFlag = false; // Set to true after deviceExtAddr is set
uint8_t deviceExtAddr[8]  = {0};
extern RCP_HOST_DBG_s rcp_host_dbg;
extern configurable_props_t cfg_props;

uint8_t timac_get_mpl_test(void) { return 0; }
void timac_set_mpl_test (uint8_t st) {}
void timac_BootstrapCallback(uint8_t state) {}
void timacExtaddressRegister() {}
static int8_t rf_interface_state_control(phy_interface_state_e new_state, uint8_t rf_channel) { return 0; }
static int8_t rf_start_cca(uint8_t *data_ptr, uint16_t data_length, uint8_t tx_handle, data_protocol_e data_protocol) { return 0; }

/* Entry index = phyID - MAC_MRFSK_STD_PHY_ID_BEGIN */
phyIDTableEntry_t macPhyIdStdTable[MAC_STANDARD_PHY_DESCRIPTOR_ENTRIES] = 
{
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_50_K,
      MAC_MODULATION_INDEX_2FSK_50K_0_5,
      MAC_CCA_TYPE_CSMA_CA },
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_50_K,
      MAC_MODULATION_INDEX_2FSK_50K_1,
      MAC_CCA_TYPE_CSMA_CA },
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_100_K,
      MAC_MODULATION_INDEX_2FSK_100K_0_5,
      MAC_CCA_TYPE_CSMA_CA },
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_100_K,
      MAC_MODULATION_INDEX_2FSK_100K_1,
      MAC_CCA_TYPE_CSMA_CA },
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_150_K,
      MAC_MODULATION_INDEX_2FSK_150K_0_5,
      MAC_CCA_TYPE_CSMA_CA },
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_150_K,
      MAC_MODULATION_INDEX_2FSK_150K_1,
      MAC_CCA_TYPE_CSMA_CA },
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_200_K,
      MAC_MODULATION_INDEX_2FSK_200K_0_5,
      MAC_CCA_TYPE_CSMA_CA },
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_200_K,
      MAC_MODULATION_INDEX_2FSK_200K_1,
      MAC_CCA_TYPE_CSMA_CA },
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_300_K,
      MAC_MODULATION_INDEX_2FSK_300K_0_5,
      MAC_CCA_TYPE_CSMA_CA },
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_300_K,
      MAC_MODULATION_INDEX_2FSK_300K_1,
      MAC_CCA_TYPE_CSMA_CA }
};

/* Entry index = phyID - MAC_MRFSK_GENERIC_PHY_ID_BEGIN */
phyIDTableEntry_t macPhyIdGenericTable[MAC_GENERIC_PHY_DESCRIPTOR_ENTRIES] = 
{
    // Currently unused
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_50_K,
      MAC_MODULATION_INDEX_2FSK_50K_1,
      MAC_CCA_TYPE_CSMA_CA },
    // Currently unused
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_20_K,
      MAC_MODULATION_INDEX_2FSK_5K,
      MAC_CCA_TYPE_CSMA_CA },
    // Currently unused
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_20_K,
      MAC_MODULATION_INDEX_2FSK_5K,
      MAC_CCA_TYPE_CSMA_CA },
    // Currently unused
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_20_K,
      MAC_MODULATION_INDEX_2FSK_5K,
      MAC_CCA_TYPE_LBT },
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_200_K,
      MAC_MODULATION_INDEX_2FSK_200K_0_5,
      MAC_CCA_TYPE_CSMA_CA },
    { MAC_2_FSK_MODULATION, MAC_MRFSK_SYMBOL_RATE_200_K,
      MAC_MODULATION_INDEX_2FSK_200K_1,
      MAC_CCA_TYPE_LBT },
};

bool mac_filter_list_add_addr(uint8_t addr[8])
{
    uint8_t zero_addr[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t index = 0;
    bool ret = false;

    for(index = 0; index < SIZE_OF_EUI_LIST; index++) {
        if(memcmp(mac_eui_filter_list[index], zero_addr, 8) == 0) {
            memcpy(mac_eui_filter_list[index], addr, 8);
            ret = true;
            break;
        }
    }
    return ret;
}

bool mac_filter_list_rm_addr(uint8_t addr[8])
{
    uint8_t zero_addr[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t index = 0;
    bool ret = false;

    for(index = 0; index < SIZE_OF_EUI_LIST; index++) {
        if(memcmp(mac_eui_filter_list[index], addr, 8) == 0) {
            memcpy(mac_eui_filter_list[index], zero_addr, 8);
            ret = true;
            break;
        }
    }
    return ret;
}

bool mac_filter_list_allow_packet(uint8_t addr[8])
{
    bool match = false;
    uint16_t index = 0;

    if (mac_filter_list_mode == MAC_FILTER_MODE_DISABLED)
    {
        return true;
    }

    for(index = 0; index < SIZE_OF_EUI_LIST; index++) {
        if(memcmp(mac_eui_filter_list[index], addr, 8) == 0) {
            match = true;
        }
    }

    if (mac_filter_list_mode == MAC_FILTER_MODE_ALLOW)
    {
        return match;
    } else if (mac_filter_list_mode == MAC_FILTER_MODE_DENY) {
        return !match;
    }
    // Invalid filter mode, do not filter packet
    return true;
}

void read_mac_addr(uint8_t *mac_addr) {
    memcpy(mac_addr, deviceExtAddr, sizeof(deviceExtAddr));
}

/*!
 This direct execute function sets a frequency hopping attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */

extern rcp_host_internal_t rcp_host_store;
void timacSetPanId(uint16_t panId) {
    tr_info("Set MAC PAN-ID 0x%x:",panId);
    // ApiMac_mlmeSetReqUint16(ApiMac_attribute_panId, panId);
    rcp_host_set_config(RCP_CONFIG_PAN_ID, &panId, sizeof(panId));
    rcp_host_store.setup->pan_id = panId;
}

uint8_t check_phy_mode(uint8_t phyID)
{
    /*Generic Phy Mode */
    if ((phyID >= MAC_MRFSK_STD_PHY_ID_BEGIN)
     && (phyID <= MAC_MRFSK_STD_PHY_ID_END))
    {
        return RCP_PHY_MODE_STD;
    }

    /* Standard Phy Mode */
    if ((phyID >= MAC_MRFSK_GENERIC_PHY_ID_BEGIN)
     && (phyID <= MAC_MRFSK_GENERIC_PHY_ID_END))
    {
        return RCP_PHY_MODE_GEN;
    }

    return RCP_PHY_MODE_UNDEF;
}


void timac_initialize(struct mac_api_s *api)
{
    uint8_t excludeChannels[MAC_154G_CHANNEL_BITMAP_SIZ];
    uint8_t idx, sizeOfChannelMask;
    uint32_t configVal;
    uint8_t  configBool;
    rcp_mac_config_phy_init phy_init_config = {0};

    /* Initialize PHY settings and start the MAC */
    phyIDTableEntry_t *phyEntry = NULL;

    if (check_phy_mode(cfg_props.config_phy_id) == RCP_PHY_MODE_STD)
    {
        phyEntry = (phyIDTableEntry_t*) &macPhyIdStdTable[cfg_props.config_phy_id - MAC_MRFSK_STD_PHY_ID_BEGIN];
    }
    else if (check_phy_mode(cfg_props.config_phy_id) == RCP_PHY_MODE_GEN)
    {
        phyEntry = (phyIDTableEntry_t*) &macPhyIdGenericTable[cfg_props.config_phy_id - MAC_MRFSK_GENERIC_PHY_ID_BEGIN];
    }
    else
    {
        tr_error("Invalid PHY ID %d", cfg_props.config_phy_id);
        MAC_ASSERT(0);
    }

    phy_init_config.phy_id = cfg_props.config_phy_id;
    phy_init_config.ffd = cfg_props.ffd;
    phy_init_config.ch0_center_frequency = cfg_props.ch0_center_frequency;
    phy_init_config.config_channel_spacing = cfg_props.config_channel_spacing;
    phy_init_config.config_number_of_channels = cfg_props.config_number_of_channels;
    phy_init_config.mod_scheme = phyEntry->fskModScheme;
    phy_init_config.symbol_rate = phyEntry->symbolRate;
    phy_init_config.mod_index = phyEntry->fskModIndex;
    phy_init_config.cca_type = phyEntry->ccaType;

    /* Note that in the Linux host + RCP configuration, the below PHY_INIT config must be received
     * by RCP before rcp_host_mac_init is called is received. This is because PHY_INIT unblocks
     * the mac task. Sleep is added here to make this consistent. In the PHY_INIT fails, MAC_INIT will
     * fail and reset must be used to recover.
     * A flag cannot be used to block here (or in any other tasklet) as tasklets cannot preempt other
     * tasklets, thus the PHY_INIT_CNF tasklet cannot unblock this tasklet. In order guarantee robustness,
     * the interface up tasklet (kicked off by NCP UART RX) must be broken up into multiple tasklets
     * to allow for a flag to update.
     * Note that this kind of config cnf checking should be applied not just to PHY_INIT but to all
     * SET_CNF messages.*/
    rcp_host_set_config(RCP_CONFIG_PHY_INIT, &phy_init_config, sizeof(rcp_mac_config_phy_init));
    usleep(10000); // Wait for MAC task to start

    /* Initialize FH */
    rcp_host_fh_init();
    rcp_host_mac_reset();

    // MAC_MlmeSetReq(macExtendedAddress, (void *)deviceExtAddr);
    rcp_host_set_config(RCP_CONFIG_EXT_ADDR, deviceExtAddr, sizeof(deviceExtAddr));

    // ApiMac_mlmeSetReqUint8(ApiMac_attribute_phyCurrentDescriptorId, cfg_props.config_phy_id);
    rcp_host_set_config(RCP_CONFIG_PHY_DESCRIPTOR_ID, &cfg_props.config_phy_id, sizeof(cfg_props.config_phy_id));

    // ApiMac_mlmeSetReqUint8(ApiMac_attribute_regDomain, cfg_props.config_reg_domain);
    rcp_host_set_config(RCP_CONFIG_REG_DOMAIN, &cfg_props.config_reg_domain, sizeof(cfg_props.config_reg_domain));

    // ApiMac_mlmeSetReqUint8(ApiMac_attribute_channelPage, (uint8_t)CONFIG_CHANNEL_PAGE);
    rcp_host_set_config(RCP_CONFIG_CHANNEL_PAGE, &cfg_props.channel_page, sizeof(cfg_props.channel_page));

    /* Set the transmit power */
    // ApiMac_mlmeSetReqUint8(ApiMac_attribute_phyTransmitPowerSigned, (uint8_t)cfg_props.phyTxPower);
    rcp_host_set_config(RCP_CONFIG_TX_POWER, &cfg_props.phyTxPower, sizeof(cfg_props.phyTxPower));

    // ApiMac_mlmeSetReqBool(ApiMac_attribute_RxOnWhenIdle, true);
    rcp_host_set_config(RCP_CONFIG_RX_ON_WHEN_IDLE, &cfg_props.rx_on_when_idle, sizeof(cfg_props.rx_on_when_idle));

    // ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_numNonSleepDevice, FH_NUM_NON_SLEEPY_HOPPING_NEIGHBORS);
    // Not called in RCP LMAC, NT size is always 1

    // New async channel mask set function
    rcp_host_set_config(RCP_CONFIG_ASYNC_CHANNEL_MASK, &cfg_props.async_channel_list, sizeof(cfg_props.async_channel_list));

    // ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_unicastDwellInterval, cfg_props.uc_dwell_interval);
    rcp_host_set_config(RCP_CONFIG_UC_DWELL_INTERVAL, &cfg_props.uc_dwell_interval, sizeof(cfg_props.uc_dwell_interval));
    // rcp_host_set_config(RCP_CONFIG_UC_CHANNEL_FUNC, &cfg_props.uc_channel_function, sizeof(cfg_props.uc_channel_function));
    rcp_host_set_config(RCP_CONFIG_UC_CHANNEL_FUNC, &cfg_props.uc_channel_function, sizeof(cfg_props.uc_channel_function));

    if(cfg_props.uc_channel_function == 0) //fixed channel
    {
        // ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_unicastFixedChannel, cfg_props.uc_fixed_channel);
        rcp_host_set_config(RCP_CONFIG_UC_FIXED_CHANNEL, &cfg_props.uc_fixed_channel, sizeof(cfg_props.uc_fixed_channel));
    }
    else //DH1CF
    {
        sizeOfChannelMask = sizeof(cfg_props.uc_channel_list)/sizeof(uint8_t);

        if(sizeOfChannelMask > MAC_154G_CHANNEL_BITMAP_SIZ)
        {
           sizeOfChannelMask = MAC_154G_CHANNEL_BITMAP_SIZ;
        }

        memset(excludeChannels, 0, MAC_154G_CHANNEL_BITMAP_SIZ);

        for(idx = 0; idx < sizeOfChannelMask; idx++)
        {
           excludeChannels[idx] = ~cfg_props.uc_channel_list[idx];
        }
        // ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_unicastExcludedChannels, excludeChannels);
        rcp_host_set_config(RCP_CONFIG_UC_EXCLUDED_CHANNELS, excludeChannels, sizeof(excludeChannels));
    }


    if (cfg_props.ffd) {
        // ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_broadcastDwellInterval, cfg_props.bc_dwell_interval);
        // ApiMac_mlmeSetFhReqUint32(ApiMac_FHAttribute_BCInterval, cfg_props.bc_interval);
        // ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_broadcastChannelFunction, cfg_props.bc_channel_function);
        rcp_host_set_config(RCP_CONFIG_BC_DWELL_INTERVAL, &cfg_props.bc_dwell_interval, sizeof(cfg_props.bc_dwell_interval));
        rcp_host_set_config(RCP_CONFIG_BC_INTERVAL, &cfg_props.bc_interval, sizeof(cfg_props.bc_interval));
        rcp_host_set_config(RCP_CONFIG_BC_CHANNEL_FUNC, &cfg_props.bc_channel_function, sizeof(cfg_props.bc_channel_function));
        if(cfg_props.bc_channel_function == 0) //fixed channel
        {
            // ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_broadcastFixedChannel, cfg_props.bc_fixed_channel);
            rcp_host_set_config(RCP_CONFIG_BC_FIXED_CHANNEL, &cfg_props.bc_fixed_channel, sizeof(cfg_props.bc_fixed_channel));
        }
        else //DH1CF
        {
            sizeOfChannelMask = sizeof(cfg_props.bc_channel_list)/sizeof(uint8_t);
            if(sizeOfChannelMask > MAC_154G_CHANNEL_BITMAP_SIZ)
            {
            sizeOfChannelMask = MAC_154G_CHANNEL_BITMAP_SIZ;
            }

            memset(excludeChannels, 0, MAC_154G_CHANNEL_BITMAP_SIZ);

            for(idx = 0; idx < sizeOfChannelMask; idx++)
            {
            excludeChannels[idx] = ~cfg_props.bc_channel_list[idx];
            }
            // ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_broadcastExcludedChannels, excludeChannels);
            rcp_host_set_config(RCP_CONFIG_BC_EXCLUDED_CHANNELS, excludeChannels, sizeof(excludeChannels));
        }
    } else {
        // ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_broadcastDwellInterval, 0);
        // ApiMac_mlmeSetFhReqUint32(ApiMac_FHAttribute_BCInterval, 0);
        // ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_broadcastChannelFunction, 0);
        // ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_broadcastFixedChannel, 0);
        configVal = 0;
        rcp_host_set_config(RCP_CONFIG_BC_DWELL_INTERVAL, &configVal, sizeof(uint8_t));
        rcp_host_set_config(RCP_CONFIG_BC_INTERVAL, &configVal, sizeof(uint32_t));
        rcp_host_set_config(RCP_CONFIG_BC_CHANNEL_FUNC, &configVal, sizeof(uint8_t));
        rcp_host_set_config(RCP_CONFIG_BC_FIXED_CHANNEL, &configVal, sizeof(uint16_t));
        memset(excludeChannels, 0, MAC_154G_CHANNEL_BITMAP_SIZ);
        // ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_broadcastExcludedChannels, excludeChannels);
        rcp_host_set_config(RCP_CONFIG_BC_EXCLUDED_CHANNELS, excludeChannels, sizeof(excludeChannels));
    }

#ifdef MAC_DUTY_CYCLE_CHECKING
    // ApiMac_mlmeSetReqBool(ApiMac_attribute_dutyCycleEnabled, true);
    configBool = 1;
    rcp_host_set_config(RCP_CONFIG_DUTY_CYCLE_ENABLE, &configBool, sizeof(uint8_t));


    // ApiMac_mlmeSetReqUint32(ApiMac_attribute_dutyCycleRegulated,
    //                         DUTY_CYCLE_MEAS_PERIOD*MAC_DUTY_CYCLE_THRESHOLD/100);
    configVal = DUTY_CYCLE_MEAS_PERIOD*MAC_DUTY_CYCLE_THRESHOLD/100;
    rcp_host_set_config(RCP_CONFIG_DUTY_CYCLE_REGULATED, &configVal, sizeof(uint32_t));
#endif

#ifdef MAC_OVERRIDE_TX_DELAY
    // ApiMac_mlmeSetReqBool(ApiMac_attribute_customMinTxOffEnabled, true);
    configBool = 1;
    rcp_host_set_config(RCP_CONFIG_CUSTOM_MIN_TX_OFF_ENABLE, &configBool, sizeof(uint8_t));

    // ApiMac_mlmeSetReqUint32(ApiMac_attribute_minTxOffTime,
    //                         (uint32_t)MAC_CONFIG_MIN_TX_OFF);
    configVal = MAC_CONFIG_MIN_TX_OFF;
    rcp_host_set_config(RCP_CONFIG_CUSTOM_MIN_TX_OFF_TIME, &configVal, sizeof(uint32_t));
#endif
}

/** NanostackRfPhy **/
/* Channel configurations for sub-GHz */
static phy_rf_channel_configuration_s phy_subghz = {
    .channel_0_center_frequency = 902200000U,
    .channel_spacing = 200000U,
    .datarate = 50000U,
    .number_of_channels = 129,
    .modulation = M_2FSK,
    .modulation_index = MODULATION_INDEX_1_0
};

static const phy_device_channel_page_s phy_channel_pages[] = {
    { CHANNEL_PAGE_2, &phy_subghz},
    { CHANNEL_PAGE_0, NULL}
};

bool rf_initialized = false;

static phy_device_driver_s device_driver;
static int8_t rf_radio_driver_id = -1;
static uint8_t rf_new_channel;

static uint8_t ns_mac_panid[2] = {0xff, 0xff};
static uint8_t ns_mac_short_addr[2];

static void rf_lock(void)
{
    platform_enter_critical();
}

static void rf_unlock(void)
{
    platform_exit_critical();
}

static int8_t rf_address_write(phy_address_type_e address_type, uint8_t *address_ptr)
{
    switch (address_type) {
        /*Set 48-bit address*/
        case PHY_MAC_48BIT:
            break;
        /*Set 64-bit address*/
        case PHY_MAC_64BIT:
            memcpy(deviceExtAddr, address_ptr, 8);
            break;
        /*Set 16-bit address*/
        case PHY_MAC_16BIT:
            memcpy(ns_mac_short_addr, address_ptr, 2);
            break;
        /*Set PAN Id*/
        case PHY_MAC_PANID:
            memcpy(ns_mac_panid, address_ptr, 2);
            break;
    }
    return 0;
}


static int8_t rf_extension(phy_extension_type_e extension_type, uint8_t *data_ptr) {
    int8_t retval = 0;
    phy_csma_params_t *csma_params;
    phy_rf_channel_configuration_s *channel_params;
    uint32_t *timer_value;
    switch (extension_type) {
        case PHY_EXTENSION_SET_CHANNEL:
            // if (rf_state == RF_IDLE || rf_state == RF_CSMA_STARTED) {
            //     rf_receive(*data_ptr);
            //     // TODO: Set new channel
            // } else {
            //     // Store the new channel if couldn't change it yet.
            //     rf_new_channel = *data_ptr;
            //     retval = -1;
            // }
            break;
        case PHY_EXTENSION_CTRL_PENDING_BIT:
            break;
        /*Return frame pending status*/
        case PHY_EXTENSION_READ_LAST_ACK_PENDING_STATUS:
            break;
        case PHY_EXTENSION_ACCEPT_ANY_BEACON:
            break;
        case PHY_EXTENSION_SET_TX_TIME:
            // tx_time = common_read_32_bit(data_ptr);
            // TODO: TX timestamp
            break;
        case PHY_EXTENSION_READ_RX_TIME:
            // common_write_32_bit(rx_time, data_ptr);
            // TODO: RX timestamp
            break;
        case PHY_EXTENSION_DYNAMIC_RF_SUPPORTED:
            *data_ptr = true; // TODO: Check this
            break;
        case PHY_EXTENSION_GET_TIMESTAMP:
            // timer_value = (uint32_t *)data_ptr;
            // *timer_value = rf_get_timestamp();
            // TODO: Implement get timestamp
            break;
        case PHY_EXTENSION_SET_CSMA_PARAMETERS:
            // csma_params = (phy_csma_params_t *)data_ptr;
            // if (csma_params->backoff_time == 0) {
            //     rf_cca_timer_stop();
            //     if (rf_read_register(TX_FIFO_STATUS)) {
            //         rf_send_command(S2LP_CMD_SABORT);
            //         rf_poll_state_change(S2LP_STATE_READY);
            //         rf_send_command(S2LP_CMD_FLUSHTXFIFO);
            //         rf_poll_state_change(S2LP_STATE_READY);
            //     }
            //     if (rf_state == RF_TX_STARTED) {
            //         rf_state = RF_IDLE;
            //         rf_receive(rf_rx_channel);
            //     }
            //     tx_time = 0;
            // } else {
            //     tx_time = csma_params->backoff_time;
            //     cca_enabled = csma_params->cca_enabled;
            // }
            // TODO: Populate CSMA params
            break;
        case PHY_EXTENSION_READ_TX_FINNISH_TIME:
            // timer_value = (uint32_t *)data_ptr;
            // *timer_value = tx_finnish_time;
            // TODO: Implement finish time
            break;

        case PHY_EXTENSION_GET_SYMBOLS_PER_SECOND:
            // timer_value = (uint32_t *)data_ptr;
            // *timer_value = rf_symbol_rate;
            // TODO: Implement symbol rate
            break;
        case PHY_EXTENSION_SET_RF_CONFIGURATION:
            // channel_params = (phy_rf_channel_configuration_s *)data_ptr;
            // phy_subghz.datarate = channel_params->datarate;
            // phy_subghz.channel_spacing = channel_params->channel_spacing;
            // phy_subghz.channel_0_center_frequency = channel_params->channel_0_center_frequency;
            // phy_subghz.number_of_channels = channel_params->number_of_channels;
            // phy_subghz.modulation = channel_params->modulation;
            // phy_subghz.modulation_index = channel_params->modulation_index;
            // rf_calculate_symbol_rate(phy_subghz.datarate, phy_subghz.modulation);
            // rf_update_config = true;
            // if (rf_state == RF_IDLE) {
            //     rf_receive(rf_rx_channel);
            // }
            // TODO: Populate RF configuration
            break;
        case PHY_EXTENSION_SET_TX_POWER:
            // TODO: Set TX output power
            break;
        case PHY_EXTENSION_SET_CCA_THRESHOLD:
            // rssi_threshold = rf_conf_cca_threshold_percent_to_rssi(*data_ptr);
            // rf_update_config = true;
            // if (rf_state == RF_IDLE) {
            //     rf_receive(rf_rx_channel);
            // }
            // break;
            // TODO: CCA threshold
        default:
            break;
    }

    return retval;
}

static int8_t rf_device_register(const uint8_t *mac_addr)
{
    /*Set pointer to MAC address*/
    device_driver.PHY_MAC = (uint8_t *)mac_addr;
    device_driver.driver_description = (char *)"TI_RFDriver";
    device_driver.link_type = PHY_LINK_15_4_SUBGHZ_TYPE;
#ifndef WISUN_RCP_ENABLE
    device_driver.phy_channel_pages = phy_channel_pages;
#endif
    device_driver.phy_MTU = RF_MTU;
    /*No header in PHY*/
    device_driver.phy_header_length = 0;
    /*No tail in PHY*/
    device_driver.phy_tail_length = 0;
    /*Set address write function*/
     device_driver.address_write = &rf_address_write;
    /*Set RF extension function*/
    device_driver.extension = &rf_extension;
    /*Set RF state control function*/
    device_driver.state_control = &rf_interface_state_control;
    /*Set transmit function*/
    device_driver.tx = &rf_start_cca;
#ifndef WISUN_RCP_ENABLE
    /*NULLIFY rx and tx_done callbacks*/
    device_driver.phy_rx_cb = NULL;
    device_driver.phy_tx_done_cb = NULL;
#endif
    /*Register device driver*/
    rf_radio_driver_id = arm_net_phy_register(&device_driver);
    return rf_radio_driver_id;
}

int8_t NanostackTiRfPhy_rf_register()
{
    rf_lock();
    if (rf_initialized) {
        /* Multiple registrations of NanostackRfPhyAtmel not supported */
        rf_unlock();
        return -1;
    }

    rf_initialized = true;
    int8_t radio_id = rf_device_register(deviceExtAddr);
    if (radio_id < 0) {
        rf_initialized = false;
    }
    rf_unlock();
    return radio_id;
}

void mcps_data_confirm_handler(const mac_api_t *api, const mcps_data_conf_t *data)
{
    (void) api;
    (void) data;
    MAC_ASSERT(0);
    return;
}

void mcps_data_indication_handler(const mac_api_t *api, const mcps_data_ind_t *data_ind)
{
    (void) api;
    (void) data_ind;
    MAC_ASSERT(0);
    return;
}

void mcps_purge_confirm_handler(const mac_api_t *api, mcps_purge_conf_t *data)
{
    (void) api;
    (void) data;
    MAC_ASSERT(0);
    return;
}

static void stop_bootstrap_timer(protocol_interface_info_entry_t *info_entry)
{
    if (info_entry->bootsrap_mode == ARM_NWK_BOOTSRAP_MODE_6LoWPAN_BORDER_ROUTER) {
        protocol_timer_stop(PROTOCOL_TIMER_BOOTSTRAP_TIM);
    }
}

void mlme_confirm_handler(const mac_api_t *api, mlme_primitive id, const void *data)
{
    protocol_interface_info_entry_t *info_entry = protocol_stack_interface_info_get_by_id(api->parent_id);
    if (!info_entry) {
        return;
    }
    switch (id) {
        case MLME_START: {
            stop_bootstrap_timer(info_entry);
            break;
        }
        case MLME_GET:
        case MLME_SET:
        case MLME_RESET:
        case MLME_SCAN:
        case MLME_ASSOCIATE:
        case MLME_DISASSOCIATE:
        case MLME_SYNC:
        case MLME_POLL:
        case MLME_RX_ENABLE:
        case MLME_GTS:
        case MLME_BEACON_NOTIFY:
        case MLME_ORPHAN:
        case MLME_COMM_STATUS:
        case MLME_SYNC_LOSS:
        default:
            break;
    }
}

void mlme_indication_handler(const mac_api_t *api, mlme_primitive id, const void *data)
{
    (void) api;
    (void) id;
    (void) data;
    MAC_ASSERT(0);
    return;
}