/*
 *    Copyright (c) 2016-2017, The OpenThread Authors.
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 *    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file implements minimal thread device required Spinel interface to the OpenThread stack.
 */

#include "openthread-core-config.h"
#include "ncp_base.hpp"
#include <arm_hal_interrupt.h>

#if OPENTHREAD_CONFIG_BORDER_ROUTER_ENABLE
#include <openthread/border_router.h>
#endif
#if OPENTHREAD_CONFIG_CHANNEL_MONITOR_ENABLE
#include <openthread/channel_monitor.h>
#endif
#if OPENTHREAD_CONFIG_CHILD_SUPERVISION_ENABLE
#include <openthread/child_supervision.h>
#endif
#include <openthread/diag.h>
#include <openthread/icmp6.h>
#if OPENTHREAD_CONFIG_JAM_DETECTION_ENABLE
#include <openthread/jam_detection.h>
#endif
#include <openthread/ncp.h>
#if OPENTHREAD_CONFIG_TIME_SYNC_ENABLE
#include <openthread/network_time.h>
#endif
#include <openthread/platform/misc.h>
#include <openthread/platform/radio.h>
#if OPENTHREAD_FTD
#include <openthread/thread_ftd.h>
#endif
#if OPENTHREAD_CONFIG_TMF_NETDATA_SERVICE_ENABLE
#include <openthread/server.h>
#endif

#include "common/code_utils.hpp"
#include "common/debug.hpp"
#include "common/instance.hpp"
//#include "net/ip6.hpp"

#include <string.h>
//#include <iostream>
#include <cstring>

#include "otstack.h"
#include <ti/drivers/GPIO.h>
//#include <ti_wisunfan_config.h>
#include "ti_drivers_config.h"
//Additional header files for integrating with nanostack
#include "nsconfig.h"
#include "Core/include/ns_buffer.h"
#include "ns_trace.h"
#include "nsdynmemLIB.h"
#include <openthread/message.h>
#include "ncp_interface/src/core/common/message.hpp"
#include "common/locator.hpp"
#include "Common_Protocols/ipv6_constants.h"

#include "net_rpl.h"
#include "RPL/rpl_protocol.h"
#include "RPL/rpl_policy.h"
#include "RPL/rpl_control.h"
#include "RPL/rpl_objective.h"
#include "RPL/rpl_upward.h"
#include "RPL/rpl_downward.h"
#include "RPL/rpl_structures.h"

#include "Service_Libs/mac_neighbor_table/mac_neighbor_table.h"
#include "net_interface.h"
#include "platform/arm_hal_phy.h"

#include "mbed-mesh-api/mesh_interface_types.h"
//#include "api_mac.h"
#include "mac_spec.h"
#include "saddr.h"
#include "application.h"
#include "socket_api.h"
#include "ip6string.h"
#include "net_interface.h"
#include "Core/include/ns_address_internal.h"
#include <ioc.h>
#include "ti_radio_config.h"
#include "6LoWPAN/ws/ws_common_defines.h"

#ifdef FEATURE_TEST_INVALID_FRAME
#include "mac_security_pib.h"
#include "timac_api.h"
#endif

#ifdef WISUN_TRX
#include <ti/trx/TRX.h>
#endif // WISUN_TRX

#define WISUN_PROTOCOL_VERSION_MAJOR 1
#define WISUN_PROTOCOL_VERSION_MINOR 0
#define WISUN_PROTOCOL_VERSION_SECOND_MINOR 1

#define INTERFACE_TYPE_WISUN 4

#define STACK_NAME "TIWISUNFAN"
#define STACK_VERSION "1.0.2"
#define BUILD_INFO
#define OTHER_INFO "RELEASE"
#define TRACE_GROUP "ncp"

#define PROTOCOL_NAME "Wi-SUNFAN"
#define PROTOCOL_VERSION "1.0"
#define CONNECTED_DEVICES_BLOCK_SIZE 2

#ifdef WISUN_FAN_DEBUG
volatile uint32_t num_drop_frame_from_host = 0;
volatile uint32_t num_ip_packet_to_host = 0;
#endif

namespace ot {
namespace Ncp {

#ifdef WISUN_NCP_ENABLE

// Used by CONNECTED_DEVICES property
uint16_t num_dao_targets = 0;
uint8_t block_index = 0;
uint8_t *connected_device_ipv6_addrs;

// Used by DODAG_ROUTE property
uint8_t dodag_route_dest_addr[16] = {0};

extern "C" configurable_props_t cfg_props;

#ifdef SWITCH_NCP_TO_TRACE
extern "C" uint32_t g_switchNcp2Trace;
#endif //SWITCH_NCP_TO_TRACE

#ifdef WISUN_TEST_MPL_EMBEDDED
extern "C" bool udpSocketSetup(void);
extern "C" void ns_trace_printf(uint8_t dlevel, const char *grp, const char *fmt, ...);
#ifdef HAVE_RPL_ROOT
extern "C" void startUDPTraffic (uint32_t numPkts, uint8_t pktInterval, uint8_t hopCount, uint16_t pktLen);
#endif
#endif

extern "C" uint8_t get_current_net_state();
extern "C" uint8_t get_network_panid();

/* Core properties */

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_LAST_STATUS>(void)
{
    return mEncoder.WriteUintPacked(mLastStatus);
}

const char *GetProtocolVersionString(void)
{
    static const char sVersion[] = PROTOCOL_NAME "/" PROTOCOL_VERSION
        ; // Trailing semicolon to end statement.

    return sVersion;
}

const char *GetVersionString(void)
{

    static const char sVersion[] = STACK_NAME "/" STACK_VERSION "; " OTHER_INFO
#if defined(__DATE__)
                                         "; " __DATE__ " " __TIME__
#endif
        ; // Trailing semicolon to end statement.

    return sVersion;
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_PROTOCOL_VERSION>(void)
{
    return mEncoder.WriteUtf8(GetProtocolVersionString());
}


template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_NCP_VERSION>(void)
{
    return mEncoder.WriteUtf8(GetVersionString());
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_INTERFACE_TYPE>(void)
{
    return mEncoder.WriteUintPacked(INTERFACE_TYPE_WISUN);
}

extern "C" void ccfg_read_mac_addr(uint8_t *mac_addr);
template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_HWADDR>(void)
{
#ifndef WRITABLE_HWADDR
    uint8_t hwAddr[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    ccfg_read_mac_addr(hwAddr);
    return mEncoder.WriteEui64(hwAddr);
#else
    otError error   = OT_ERROR_NONE;
    uint8_t i = 0;

    // if cfg_props.hwaddr is all zeros, use ccfg addr instead
    unsigned char null_hw_addr[8] = {0};
    if ((std::memcmp(&cfg_props.hwaddr, null_hw_addr, 8) == 0))
    {
        ccfg_read_mac_addr(&cfg_props.hwaddr[0]);
    }

    SuccessOrExit(error = mEncoder.WriteEui64(cfg_props.hwaddr));
    exit:
        return(error);
#endif /* WRITABLE_HWADDR */

}

#ifdef WRITABLE_HWADDR
template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_HWADDR>(void)
{
    spinel_eui64_t hwAddr = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    otError error   = OT_ERROR_NONE;
    uint8_t i = 0;

    SuccessOrExit(error = mDecoder.ReadEui64(hwAddr));
    
    for(i = 0; i < HWADDR_SIZE; i++)
    {
        cfg_props.hwaddr[i] = hwAddr.bytes[i];
    }

    exit:
        return(error);
}

#endif /* WRITABLE_HWADDR*/


#ifdef WISUN_TRX
extern "C" TRX_Request_LastStatus_Param_Version trxVersion;
#endif

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_TRX_FW_VERSION>(void)
{
    otError error   = OT_ERROR_NONE;

#ifdef WISUN_TRX
    SuccessOrExit(error = mEncoder.WriteUint8(trxVersion.major));
    SuccessOrExit(error = mEncoder.WriteUint8(trxVersion.minor));
    SuccessOrExit(error = mEncoder.WriteUint8(trxVersion.patch));
    SuccessOrExit(error = mEncoder.WriteUint8(trxVersion.build));
    SuccessOrExit(error = mEncoder.WriteUint32(trxVersion.hash));
#else
    //non TRX wisun projects - return all zeros
    SuccessOrExit(error = mEncoder.WriteUint8(0));
    SuccessOrExit(error = mEncoder.WriteUint8(0));
    SuccessOrExit(error = mEncoder.WriteUint8(0));
    SuccessOrExit(error = mEncoder.WriteUint8(0));
    SuccessOrExit(error = mEncoder.WriteUint32(0));
#endif

    exit:
        return(error);
}


/* PHY properties */


template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_PHY_CCA_THRESHOLD>(void)
{
    return mEncoder.WriteInt8(cfg_props.ccaDefaultdBm);
}

template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_PHY_CCA_THRESHOLD>(void)
{
    otError error   = OT_ERROR_NONE;

    SuccessOrExit(error = mDecoder.ReadInt8(cfg_props.ccaDefaultdBm));

    exit:
        return error;
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_PHY_TX_POWER>(void)
{
    return mEncoder.WriteInt8(cfg_props.phyTxPower);
}

template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_PHY_TX_POWER>(void)
{
    otError error   = OT_ERROR_NONE;

    SuccessOrExit(error = mDecoder.ReadInt8(cfg_props.phyTxPower));

    exit:
        return error;
}

extern "C" uint8_t cur_num_nbrs;
extern "C" uint8_t nbr_idx;
extern "C" nbr_node_metrics_t nbr_nodes_metrics[SIZE_OF_NEIGH_LIST];

extern "C" void fetch_neighbor_details();

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_PHY_NUM_NBRS>(void)
{
    otError  error = OT_ERROR_NONE;

    //collate nbr details into nbr_nodes_metrics array.
    fetch_neighbor_details();

    //reset neighbor read idx
    nbr_idx = 0;

    //Send the info to Spinel encoder
    SuccessOrExit(error = mEncoder.WriteUint8(cur_num_nbrs));

    exit:
        return error;
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_PHY_NBR_METRICS>(void)
{
    otError  error = OT_ERROR_NONE;

    SuccessOrExit(error = mEncoder.WriteEui64(nbr_nodes_metrics[nbr_idx].mac_eui));
    SuccessOrExit(error = mEncoder.WriteUint16(nbr_nodes_metrics[nbr_idx].rssi_in));
    SuccessOrExit(error = mEncoder.WriteUint16(nbr_nodes_metrics[nbr_idx].rssi_out));

    nbr_idx++;

    exit:
           return error;
}

#ifdef WISUN_TEST_METRICS
extern "C" void get_test_metrics(test_metrics_s *test_metrics);
#endif

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_PHY_METRICS>(void)
{
    otError  error = OT_ERROR_NONE;

#ifdef WISUN_TEST_METRICS
    test_metrics_s test_metrics;
    //Retrieve Test metrics data
    get_test_metrics(&test_metrics);

    //Send the info to Spinel encoder
    SuccessOrExit(error = mEncoder.WriteDataWithLen((uint8_t *)&test_metrics, sizeof(test_metrics_s)));
#else
    SuccessOrExit(error = mEncoder.WriteUint8(0));
#endif

    exit:
        return error;
}

/* MAC properties */
template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_MAC_15_4_PANID>(void)
{
    if (get_current_net_state() < 3)
    {
        return mEncoder.WriteUint16(cfg_props.pan_id);
    }
    return mEncoder.WriteUint16(get_network_panid());
}

template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_MAC_15_4_PANID>(void)
{
#ifdef HAVE_RPL_ROOT
    otError  error = OT_ERROR_NONE;

    SuccessOrExit(error = mDecoder.ReadUint16(cfg_props.pan_id));

exit:
    return error;
#else
    return OT_ERROR_NOT_IMPLEMENTED;
#endif
}

/* NET properties */
extern "C" bool is_net_if_up();
template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_NET_IF_UP>(void)
{
    return mEncoder.WriteBool((bool)is_net_if_up());
}

extern "C" otError nanostack_net_if_up();
template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_NET_IF_UP>(void)
{
    bool    enable = false;
    otError error   = OT_ERROR_NONE;

    SuccessOrExit(error = mDecoder.ReadBool(enable));

    //bring interface up if currently the interface is down
    if(true == enable)
    {
        if(!is_net_if_up())
        {
            error = nanostack_net_if_up();
        }
    }
    //to be done: enable = false case

exit:
    return error;
}

extern "C" bool is_net_stack_up();
template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_NET_STACK_UP>(void)
{
    return mEncoder.WriteBool((bool)is_net_stack_up());
}

extern "C" otError nanostack_net_stack_up(void);
#ifdef TEST_WISUN_STACK_RESTART
extern "C" int nanostack_net_stack_restart(void);
#endif //TEST_WISUN_STACK_RESTART
template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_NET_STACK_UP>(void)
{
    bool    enable = false;
    otError error   = OT_ERROR_NONE;

    SuccessOrExit(error = mDecoder.ReadBool(enable));

    if(true == enable)
    {
    	//if net stack not already brought up
        if(get_current_net_state() == 0)
		{

#ifdef SWITCH_NCP_TO_TRACE
	        //set a flag to drop pushing info from NCP onto Primary Uart
	        g_switchNcp2Trace = 1;

	        //reassign the uart pin to trace
#if defined(LP_CC1312R7) || defined(LAUNCHXL_CC1312R1)
        	IOCPortConfigureSet(IOID_3, IOC_PORT_MCU_SWV, IOC_STD_OUTPUT);
#else  // 1312
       		IOCPortConfigureSet(IOID_13, IOC_PORT_MCU_SWV, IOC_STD_OUTPUT);
#endif // 1312
#endif // SWITCH_NCP_TO_TRACE              
        
    		//bringup the wisunstack
        	error = nanostack_net_stack_up();        

#ifdef WISUN_TEST_MPL_EMBEDDED
            // Set up UDP socket when network stack is up
            if (error == OT_ERROR_NONE)
            {
                if(udpSocketSetup() == false)
                {
                    tr_debug("Socket setup failed");
                }
            }
#endif
    	}
#ifdef TEST_WISUN_STACK_RESTART
		else
		{
		    //restart wisun stack
		    if(nanostack_net_stack_restart() < 0)
            {
		        error = OT_ERROR_FAILED;
            }
		}
#endif //TEST_WISUN_STACK_RESTART
    }//(true == enabled)

exit:
    return error;
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_NET_ROLE>(void)
{
    bool netRole = 0; // 0 - Border Router; 1 - Router
    if(cfg_props.wisun_device_type == MESH_DEVICE_TYPE_WISUN_BORDER_ROUTER)
    {
        netRole = 0;
    }
    else
    {
        netRole = 1;
    }
    return mEncoder.WriteBool((bool)netRole);
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_NET_NETWORK_NAME>(void)
{
    return mEncoder.WriteUtf8((char *)cfg_props.network_name);
}

template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_NET_NETWORK_NAME>(void)
{
    const char *string = NULL;
    otError     error  = OT_ERROR_NONE;

    SuccessOrExit(mDecoder.ReadUtf8(string));

    memset(cfg_props.network_name, 0, MAX_NETWORK_NAME_SIZE);
    strcpy((char*)cfg_props.network_name, string);

    exit:
        return error;
}

template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_NET_UDP_START>(void)
{
    uint32_t udpPkts = 0;
    uint8_t udpPktInterval = 1;
    uint8_t udpHopCount = 1;
    uint8_t udpPktLen = 20;

    spinel_eui64_t udpTestParams;
    otError     error  = OT_ERROR_NONE;

    SuccessOrExit(error = mDecoder.ReadEui64(udpTestParams));
    // Number of packets to send - 4 bytes
    udpPkts = udpTestParams.bytes[7] + (udpTestParams.bytes[6] << 8) + (udpTestParams.bytes[5] << 16) + (udpTestParams.bytes[4] << 24);
    // Packet interval - 1 byte
    udpPktInterval = udpTestParams.bytes[3];
    // Packet interval is in seconds
    if (udpPktInterval == 0)
    {
        udpPktInterval = 1;
    }

    // UDP Hop Count - 1 byte
    udpHopCount = udpTestParams.bytes[2];
    if (udpHopCount == 0)
    {
        udpHopCount = 1;
    }
    // Packet Length - 1 byte
    udpPktLen = udpTestParams.bytes[1];
    if (udpPktLen < 20)
    {
        udpPktLen = 20;
    }

#ifdef WISUN_TEST_MPL_EMBEDDED
#ifdef HAVE_RPL_ROOT
    // Start UDP Traffic for given number of packets
    startUDPTraffic(udpPkts, udpPktInterval, udpHopCount, udpPktLen);
#endif
#endif

exit:
    return error;
}

/* Tech Specific: PHY properties */
template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_PHY_REGION>(void)
{
    return mEncoder.WriteUint8(cfg_props.config_reg_domain);
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_PHY_MODE_ID>(void)
{
    return mEncoder.WriteUint8(cfg_props.config_phy_id);
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_PHY_UNICAST_CHANNEL_LIST>(void)
{
    otError error  = OT_ERROR_NONE;
    uint8_t i = 0;

    for(i = 0; i < CHANNEL_BITMAP_SIZE; i++)
    {
        SuccessOrExit(error = mEncoder.WriteUint8(cfg_props.uc_channel_list[i]));
    }

exit:
    return(error);
}

extern "C" uint8_t get_first_fixed_channel(uint8_t * channel_list);
template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_PHY_UNICAST_CHANNEL_LIST>(void)
{
   otError error  = OT_ERROR_NONE;
   uint8_t i = 0;

   const uint8_t *chListPtr = NULL;
   uint16_t       chListLen = 0;

   SuccessOrExit(error = mDecoder.ReadDataWithLen(chListPtr, chListLen));

   for(i = 0; i < CHANNEL_BITMAP_SIZE; i++)
   {
       cfg_props.uc_channel_list[i] = chListPtr[i];
   }

   // Update fixed unicast channel based on new selection
   cfg_props.uc_fixed_channel = get_first_fixed_channel(cfg_props.uc_channel_list);

   exit:
       return(error);
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_PHY_BROADCAST_CHANNEL_LIST>(void)
{
    otError error  = OT_ERROR_NONE;
    uint8_t i = 0;

    for(i = 0; i < CHANNEL_BITMAP_SIZE; i++)
    {
        SuccessOrExit(error = mEncoder.WriteUint8(cfg_props.bc_channel_list[i]));
    }

exit:
    return(error);
}

template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_PHY_BROADCAST_CHANNEL_LIST>(void)
{
   otError error  = OT_ERROR_NONE;
   uint8_t i = 0;

   const uint8_t *chListPtr = NULL;
   uint16_t       chListLen = 0;

   SuccessOrExit(error = mDecoder.ReadDataWithLen(chListPtr, chListLen));

   for(i = 0; i < CHANNEL_BITMAP_SIZE; i++)
   {
       cfg_props.bc_channel_list[i] = chListPtr[i];
   }

   // Update fixed broadcast channel based on new selection
   cfg_props.bc_fixed_channel = get_first_fixed_channel(cfg_props.bc_channel_list);

   exit:
       return(error);
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_PHY_ASYNC_CHANNEL_LIST>(void)
{
    otError error  = OT_ERROR_NONE;
    uint8_t i = 0;

    for(i = 0; i < CHANNEL_BITMAP_SIZE; i++)
    {
        SuccessOrExit(error = mEncoder.WriteUint8(cfg_props.async_channel_list[i]));
    }

exit:
    return(error);

}

template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_PHY_ASYNC_CHANNEL_LIST>(void)
{
   otError error  = OT_ERROR_NONE;
   uint8_t i = 0;

   const uint8_t *chListPtr = NULL;
   uint16_t       chListLen = 0;

   SuccessOrExit(error = mDecoder.ReadDataWithLen(chListPtr, chListLen));

   for(i = 0; i < CHANNEL_BITMAP_SIZE; i++)
   {
       cfg_props.async_channel_list[i] = chListPtr[i];
   }

   exit:
       return(error);
}


template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_PHY_CH_SPACING>(void)
{
    return mEncoder.WriteUint16(cfg_props.config_channel_spacing);
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_PHY_CHO_CENTER_FREQ>(void)
{
    otError     error  = OT_ERROR_NONE;

    uint16_t Ch0freq_MHz = (uint16_t)(cfg_props.ch0_center_frequency/1000);
    uint16_t Ch0freq_kHz = (uint16_t)(cfg_props.ch0_center_frequency - Ch0freq_MHz*1000);

    SuccessOrExit(error = mEncoder.WriteUint16(Ch0freq_MHz));
    SuccessOrExit(error = mEncoder.WriteUint16(Ch0freq_kHz));

    exit:
        return error;
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_MAC_BC_INTERVAL>(void)
{
    return mEncoder.WriteUint32(cfg_props.bc_interval);
}

template <> otError NcpBase::HandlePropertySet<SPINEL_MAC_BC_INTERVAL>(void)
{
    otError error   = OT_ERROR_NONE;

    SuccessOrExit(error = mDecoder.ReadUint32(cfg_props.bc_interval));

exit:
    return error;
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_MAC_BC_DWELL_INTERVAL>(void)
{
    return mEncoder.WriteUint8(cfg_props.bc_dwell_interval);
}

template <> otError NcpBase::HandlePropertySet<SPINEL_MAC_BC_DWELL_INTERVAL>(void)
{
    otError error   = OT_ERROR_NONE;

    SuccessOrExit(error = mDecoder.ReadUint8(cfg_props.bc_dwell_interval));

exit:
    return error;
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_MAC_BC_CHANNEL_FUNCTION>(void)
{
    return mEncoder.WriteUint8(cfg_props.bc_channel_function);
}

template <> otError NcpBase::HandlePropertySet<SPINEL_MAC_BC_CHANNEL_FUNCTION>(void)
{
    otError error   = OT_ERROR_NONE;

    SuccessOrExit(error = mDecoder.ReadUint8(cfg_props.bc_channel_function));

exit:
    return error;
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_MAC_UC_DWELL_INTERVAL>(void)
{
    return mEncoder.WriteUint8(cfg_props.uc_dwell_interval);
}

template <> otError NcpBase::HandlePropertySet<SPINEL_MAC_UC_DWELL_INTERVAL>(void)
{
    otError error   = OT_ERROR_NONE;

    SuccessOrExit(error = mDecoder.ReadUint8(cfg_props.uc_dwell_interval));

exit:
    return error;
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_MAC_UC_CHANNEL_FUNCTION>(void)
{
    return mEncoder.WriteUint8(cfg_props.uc_channel_function);
}

template <> otError NcpBase::HandlePropertySet<SPINEL_MAC_UC_CHANNEL_FUNCTION>(void)
{
    otError error   = OT_ERROR_NONE;

    SuccessOrExit(error = mDecoder.ReadUint8(cfg_props.uc_channel_function));

exit:
    return error;
}

#ifdef TI_WISUN_FAN_DEBUG
extern "C" uint8_t filterMode;
template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_MAC_FILTER_MODE>(void)
{
    return mEncoder.WriteUint8(filterMode);
}

template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_MAC_FILTER_MODE>(void)
{
    otError error   = OT_ERROR_NONE;

    SuccessOrExit(error = mDecoder.ReadUint8(filterMode));

exit:
    return error;
}

#ifdef FEATURE_TEST_EAPOL_ACTIVE_MAX
extern "C" uint8_t test_eapol_active_max;
extern "C" uint16_t num_eapol_active_max_rejections;
#endif
#ifdef FEATURE_EDFE_TEST_MODE
extern "C" uint8_t switchToEDFE;
#endif
#ifdef FEATURE_TEST_INVALID_FRAME
extern "C" macSecurityPib_t macSecurityPib;
uint8_t testInvalidFrame = false;
uint8_t prevKey[MAC_KEY_MAX_LEN] = {0};
uint32_t prevFrameCount = 0;
#endif

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_TEST_COMMAND>(void)
{
    otError error = OT_ERROR_NONE;
    uint8_t value = 0;

#ifdef FEATURE_TEST_EAPOL_ACTIVE_MAX
    value = num_eapol_active_max_rejections;
#endif
#ifdef FEATURE_EDFE_TEST_MODE
    value = switchToEDFE;
#endif
#ifdef FEATURE_TEST_INVALID_FRAME
    value = testInvalidFrame;
#endif
    SuccessOrExit(error = mEncoder.WriteUint8(value));
exit:
    return error;
}

template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_TEST_COMMAND>(void)
{
    otError error = OT_ERROR_NONE;
    uint8_t value = 0;

    SuccessOrExit(error = mDecoder.ReadUint8(value));
#ifdef FEATURE_TEST_EAPOL_ACTIVE_MAX
    test_eapol_active_max = value;
#endif
#ifdef FEATURE_EDFE_TEST_MODE
    switchToEDFE = value;
#endif
#ifdef FEATURE_TEST_INVALID_FRAME
    if (value == true && testInvalidFrame == false)
    {
        // Store old key/frame count and change current key/frame count to test values
        memcpy(prevKey, macSecurityPib.macKeyTable[0].key, MAC_KEY_MAX_LEN);
        prevFrameCount = macSecurityPib.macKeyTable[0].frameCounter;

        memset(macSecurityPib.macKeyTable[0].key, 0xAB, MAC_KEY_MAX_LEN);
        macSecurityPib.macKeyTable[0].frameCounter = 0;
    }
    else if (value == false && testInvalidFrame == true)
    {
        // Restore old key/frame count
        memcpy(macSecurityPib.macKeyTable[0].key, prevKey, MAC_KEY_MAX_LEN);
        macSecurityPib.macKeyTable[0].frameCounter = prevFrameCount;
    }
    testInvalidFrame = value;
#endif
exit:
    return error;
}

extern "C" uint8_t enableVPIE;
template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_VPIE_COMMAND>(void)
{
    otError error = OT_ERROR_NONE;
    bool enable = false;

    enable = enableVPIE;

    SuccessOrExit(error = mEncoder.WriteBool(enable));
exit:
    return error;
}


template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_VPIE_COMMAND>(void)
{
    otError error = OT_ERROR_NONE;
    bool enable = false;

    SuccessOrExit(error = mDecoder.ReadBool(enable));

    enableVPIE = enable;

exit:
    return error;
}

extern "C" void timac_set_mpl_test (uint8_t st);
extern "C" uint8_t timac_get_mpl_test(void);

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_MACMPL_COMMAND>(void)
{
    otError error = OT_ERROR_NONE;
    bool enable = false;
    uint8_t st;
    st = timac_get_mpl_test();
    enable = st;

    SuccessOrExit(error = mEncoder.WriteBool(enable));
exit:
    return error;
}

template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_MACMPL_COMMAND>(void)
{
    otError error = OT_ERROR_NONE;
    bool enable = false;

    SuccessOrExit(error = mDecoder.ReadBool(enable));

    timac_set_mpl_test(enable);

exit:
    return error;
}

#endif //TI_WISUN_FAN_DEBUG

extern "C" uint8_t *bitcopy(uint8_t *restrict dst, const uint8_t *restrict src, uint_fast8_t bits);
extern "C" void nanostack_process_routing_table_update_from_stack(uint8_t changed_info, uint8_t* prefix, uint8_t len_prefix, uint8_t* addr_nexthop, uint32_t lifetime)
{
    //make 16 byte IPv6 address from prefix
    uint8_t addr_self[16] = { 0 };
    bitcopy(addr_self, prefix, len_prefix);

    ot::Ncp::NcpBase *ncp = ot::Ncp::NcpBase::GetNcpInstance();
    ncp->SendRouteTableUpdate(changed_info, addr_self, len_prefix, addr_nexthop, lifetime);

    //what if an error occurs? when does the ncp try to send the failed update? How is the malformed spinel frame removed
}

otError NcpBase::SendRouteTableUpdate(uint8_t changed_info, uint8_t* addr_self, uint8_t len_prefix, uint8_t* addr_nexthop, uint32_t lifetime)
{
    otError error   = OT_ERROR_NONE;
    uint8_t           header   = SPINEL_HEADER_FLAG | SPINEL_HEADER_IID_0;
    spinel_prop_key_t propKey  = SPINEL_PROP_ROUTING_TABLE_UPDATE;

    // begin spinel encoding
    SuccessOrExit(error = mEncoder.BeginFrame(header, SPINEL_CMD_PROP_VALUE_IS, propKey));
    SuccessOrExit(error = mEncoder.OpenStruct());
    SuccessOrExit(error = mEncoder.WriteUint8(changed_info));
    SuccessOrExit(error = mEncoder.WriteIp6Address(addr_self));
    SuccessOrExit(error = mEncoder.WriteUint8(len_prefix));
    SuccessOrExit(error = mEncoder.WriteIp6Address(addr_nexthop));
    SuccessOrExit(error = mEncoder.WriteUint32(lifetime));
    SuccessOrExit(error = mEncoder.CloseStruct());
    SuccessOrExit(error = mEncoder.EndFrame());

exit:
    return error;
}


extern "C" otError nanostack_process_stream_net_from_host(uint8_t* framePtr, uint16_t length);

template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_STREAM_NET>(void)
{
    const uint8_t *framePtr = NULL;
    uint16_t       frameLen = 0;
    const uint8_t *metaPtr  = NULL;
    uint16_t       metaLen  = 0;
    otMessage *    message  = NULL;
    otError        error    = OT_ERROR_NONE;

    SuccessOrExit(error = mDecoder.ReadDataWithLen(framePtr, frameLen));
    SuccessOrExit(error = mDecoder.ReadData(metaPtr, metaLen));

    nanostack_process_stream_net_from_host((uint8_t*) framePtr, frameLen);

exit:
    mDroppedInboundIpFrameCounter++;
#ifdef WISUN_FAN_DEBUG
    num_drop_frame_from_host++;
#endif

    return error;
}

void NcpBase::HandleDatagramFromStack(otMessage *aMessage)
{
    VerifyOrExit(aMessage != NULL, OT_NOOP);

    SuccessOrExit(otMessageQueueEnqueue(&mMessageQueue, aMessage));

    // If there is no queued spinel command response, try to write/send
    // the datagram message immediately. If there is a queued response
    // or if currently out of buffer space, the IPv6 datagram message
    // will be sent from `HandleFrameRemovedFromNcpBuffer()` when buffer
    //  space becomes available and after any pending spinel command
    // response.

    if (IsResponseQueueEmpty())
    {
        IgnoreReturnValue(SendQueuedDatagramMessages());
    }

exit:
    return;
}


extern "C" void nanostack_process_stream_net_from_stack(buffer_t *buf)
{
    ot::Ncp::NcpBase *ncp = ot::Ncp::NcpBase::GetNcpInstance();
    otMessageSettings settings = {true, OT_MESSAGE_PRIORITY_NORMAL};
    Message* message  = NULL;
    uint8_t* aData = buffer_ipv6_pointer(buf);
    uint16_t aDataLength = buffer_ipv6_length(buf);

#ifdef WISUN_FAN_DEBUG
    num_ip_packet_to_host++;
#endif
    
    if (ncp == NULL || buf->ipv6_buf_ptr == 0xFFFF)
    {
       // tr_debug("\n NCP Instance is empty or IPv6 Buf Ptr is invalid!!!");
        goto exit;
    }

    VerifyOrExit((message = ncp->GetOtInstance()->mMessagePool.New(Message::kTypeIp6, 0, &settings)) != NULL, OT_NOOP);

    if (message->Append(aData, aDataLength) != OT_ERROR_NONE)
    {
        message->Free();
        message = NULL;
    }

    ncp->HandleDatagramFromStack((otMessage*) message);

exit:
    //buffer_free(buf);
    /* Received Packet gets dropped due to mismatch priority or non availability
     * of message buffers
     */
   // tr_debug("\n Received Packet is dropped due to non-availability of buffers");
    return;
}


otError NcpBase::SendQueuedDatagramMessages(void)
{
    otError    error = OT_ERROR_NONE;
    otMessage *message;

    while ((message = otMessageQueueGetHead(&mMessageQueue)) != NULL)
    {
        // Since an `otMessage` instance can be in one queue at a time,
        // it is first dequeued from `mMessageQueue` before attempting
        // to include it in a spinel frame by calling `SendDatagramMessage()`
        // If forming of the spinel frame fails, the message is enqueued
        // back at the front of `mMessageQueue`.

        otMessageQueueDequeue(&mMessageQueue, message);

        error = SendDatagramMessage(message);

        if (error != OT_ERROR_NONE)
        {
            otMessageQueueEnqueueAtHead(&mMessageQueue, message);
        }

        SuccessOrExit(error);
    }

    exit:
        return error;
}

otError NcpBase::SendDatagramMessage(otMessage *aMessage)
{
    otError           error    = OT_ERROR_NONE;
    uint8_t           header   = SPINEL_HEADER_FLAG | SPINEL_HEADER_IID_0;
    bool              isSecure = otMessageIsLinkSecurityEnabled(aMessage);
    spinel_prop_key_t propKey  = SPINEL_PROP_STREAM_NET;

    if (!isSecure)
    {
      //  tr_debug("\n Error: Attempting to send an un secure IPv6 frame to host");
    }

    SuccessOrExit(error = mEncoder.BeginFrame(header, SPINEL_CMD_PROP_VALUE_IS, propKey));
    SuccessOrExit(error = mEncoder.WriteUint16(otMessageGetLength(aMessage)));
    SuccessOrExit(error = mEncoder.WriteMessage(aMessage));

    // Append any metadata (rssi, lqi, channel, etc) here!

    SuccessOrExit(error = mEncoder.EndFrame());
    mOutboundSecureIpFrameCounter++;

exit:
    return error;
}

#ifdef TI_WISUN_FAN_DEBUG
extern "C" sAddrExt_t mac_eui_filter_list[SIZE_OF_EUI_LIST];
#ifdef HAVE_RPL_ROOT
extern "C" sAddrExt_t eapol_eui_allow_list[EAPOL_EUI_LIST_SIZE];
#endif
#define SIZE_EUI_LIST_PRINT 25

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_MAC_MAC_FILTER_LIST>(void)
{
    otError             error    = OT_ERROR_NONE;
    uint16_t index = 0;
    uint16_t print_index = 0;
    sAddrExt_t empty_addr = {0};

#ifdef HAVE_RPL_ROOT
    if (filterMode == 3)
    {
        while (index < EAPOL_EUI_LIST_SIZE && print_index < SIZE_EUI_LIST_PRINT)
        {
            if (std::memcmp(eapol_eui_allow_list[index], empty_addr, SADDR_EXT_LEN) != 0)
            {
                SuccessOrExit(error = mEncoder.WriteEui64(eapol_eui_allow_list[index]));
                print_index++;
            }
            index++;
        }

    }
    else
#endif
    {
        while (index < SIZE_OF_EUI_LIST && print_index < SIZE_EUI_LIST_PRINT)
        {
            if (std::memcmp(mac_eui_filter_list[index], empty_addr, SADDR_EXT_LEN) != 0)
            {
                SuccessOrExit(error = mEncoder.WriteEui64(mac_eui_filter_list[index]));
                print_index++;
            }
            index++;
        }
    }

exit:
    return error;
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_EXTERNAL_DHCP_SERVER_ENABLED>(void)
{
    otError error = OT_ERROR_NONE;
    SuccessOrExit(error = mEncoder.WriteUint8(ti_br_config.use_external_dhcp_server));
exit:
    return error;
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_EXTERNAL_DHCP_SERVER_ADDRESS>(void)
{
    otError error = OT_ERROR_NONE;
    SuccessOrExit(error = mEncoder.WriteIp6Address(ti_br_config.external_dhcp_server_addr));
exit:
    return error;
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_EXTERNAL_AUTH_SERVER_ENABLED>(void)
{
    otError error = OT_ERROR_NONE;
    SuccessOrExit(error = mEncoder.WriteUint8(ti_br_config.use_external_radius_server));
exit:
    return error;
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_EXTERNAL_AUTH_SERVER_ADDRESS>(void)
{
    otError error = OT_ERROR_NONE;
    SuccessOrExit(error = mEncoder.WriteIp6Address(ti_br_config.external_radius_server_addr));
exit:
    return error;
}

extern "C" bool macRx_insertAddrIntoList(uint8_t* euiAddress);
extern "C" bool macRx_removeAddrFromList(uint8_t* euiAddress);
#ifdef HAVE_RPL_ROOT
extern "C" bool insert_eapol_eui_allow_list(uint8_t* euiAddress);
extern "C" bool remove_eapol_eui_allow_list(uint8_t* euiAddress);
#endif
template <> otError NcpBase::HandlePropertyInsert<SPINEL_PROP_MAC_MAC_FILTER_LIST>(void)
{
    otError error = OT_ERROR_NONE;
    bool retVal = false;
    spinel_eui64_t extAddress;

    // read the eui address to be inserted
    SuccessOrExit(error = mDecoder.ReadEui64(extAddress));

    // insert to address list
#ifdef HAVE_RPL_ROOT
    if (filterMode == 3)
    {
        retVal = insert_eapol_eui_allow_list(&extAddress.bytes[0]);
    }
    else
#endif
    {
        retVal = macRx_insertAddrIntoList(&extAddress.bytes[0]);
    }
    if(false == retVal)
    {
        error = OT_ERROR_FAILED;
    }

exit:
    return error;
}

template <> otError NcpBase::HandlePropertyRemove<SPINEL_PROP_MAC_MAC_FILTER_LIST>(void)
{
    otError error = OT_ERROR_NONE;
    bool retVal = false;
    spinel_eui64_t extAddress;

    // read the eui address to be removed
    SuccessOrExit(error = mDecoder.ReadEui64(extAddress));

    // remove from address list
#ifdef HAVE_RPL_ROOT
    if (filterMode == 3)
    {
        retVal = remove_eapol_eui_allow_list(&extAddress.bytes[0]);
    }
    else
#endif
    {
        retVal = macRx_removeAddrFromList(&extAddress.bytes[0]);
    }
    if(false == retVal)
    {
        error = OT_ERROR_NO_ADDRESS;
    }

exit:
    return error;
}
#endif //TI_WISUN_FAN_DEBUG

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_NET_STATE>(void)
{
    otError error = OT_ERROR_NONE;
    SuccessOrExit(error = mEncoder.WriteUint8(get_current_net_state()));

exit:
    return error;
}

extern "C" if_address_entry_t *get_linkLocal_address();
extern "C" if_address_entry_t *get_globalUnicast_address();
template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_IPV6_ADDRESS_TABLE>(void)
{
    otError error = OT_ERROR_NONE;

    if_address_entry_t *entry;

    entry = get_globalUnicast_address();

    if(entry == NULL)
    {
        error = OT_ERROR_FAILED;
    }
    SuccessOrExit(error);

    SuccessOrExit(error = mEncoder.OpenStruct());
    SuccessOrExit(error = mEncoder.WriteIp6Address(entry->address));
    SuccessOrExit(error = mEncoder.WriteUint8(entry->prefix_len));
    SuccessOrExit(error = mEncoder.WriteUint32(entry->valid_lifetime));
    SuccessOrExit(error = mEncoder.WriteUint32(entry->preferred_lifetime));
    SuccessOrExit(error = mEncoder.CloseStruct());

    entry = get_linkLocal_address();

    if(entry == NULL)
    {
        error = OT_ERROR_FAILED;
    }
    SuccessOrExit(error);

    SuccessOrExit(error = mEncoder.OpenStruct());
    SuccessOrExit(error = mEncoder.WriteIp6Address(entry->address));
    SuccessOrExit(error = mEncoder.WriteUint8(entry->prefix_len));
    SuccessOrExit(error = mEncoder.WriteUint32(entry->valid_lifetime));
    SuccessOrExit(error = mEncoder.WriteUint32(entry->preferred_lifetime));
    SuccessOrExit(error = mEncoder.CloseStruct());

exit:

    return error;
}

extern "C" if_group_list_t *get_multicast_ip_groups();
template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_MULTICAST_LIST>(void)
{
    otError error = OT_ERROR_NONE;
    if_group_list_t *multicast_list;

    if (get_current_net_state() != 5)
    {
        error = OT_ERROR_INVALID_STATE;
    }
    SuccessOrExit(error);

    multicast_list = get_multicast_ip_groups();

    SuccessOrExit(error = mEncoder.OpenStruct());
    ns_list_foreach(if_group_entry_t, entry, multicast_list)
    {
        // Only print multicast addresses above realm scope
        if (addr_ipv6_multicast_scope(&entry->group[0]) > IPV6_SCOPE_REALM_LOCAL)
        {
            SuccessOrExit(error = mEncoder.WriteIp6Address(&entry->group[0]));
        }
    }
    SuccessOrExit(error = mEncoder.CloseStruct());

exit:
    return error;
}

extern "C" int8_t remove_multicast_addr(const uint8_t *address_ptr);
template <> otError NcpBase::HandlePropertyRemove<SPINEL_PROP_MULTICAST_LIST>(void)
{
    otError error = OT_ERROR_NONE;
    spinel_ipv6addr_t multicast_addr;

    // read the ipv6 address to be inserted
    SuccessOrExit(error = mDecoder.ReadIp6Address(multicast_addr));

    // Return value 0 indicates success
    if(remove_multicast_addr(&multicast_addr.bytes[0]) != 0)
    {
        error = OT_ERROR_FAILED;
    }

exit:
    return error;
}

extern "C" int8_t add_multicast_addr(const uint8_t *address_ptr);
template <> otError NcpBase::HandlePropertyInsert<SPINEL_PROP_MULTICAST_LIST>(void)
{
    otError error = OT_ERROR_NONE;
    spinel_ipv6addr_t multicast_addr;

    // read the ipv6 address to be inserted
    SuccessOrExit(error = mDecoder.ReadIp6Address(multicast_addr));

    // Return value 0 indicates success
    if(add_multicast_addr(&multicast_addr.bytes[0]) != 0)
    {
        error = OT_ERROR_FAILED;
    }

exit:
    return error;
}


extern "C" rpl_instance_t *get_rpl_instance();
template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_NUM_CONNECTED_DEVICES>(void)
{
#ifdef HAVE_RPL_ROOT
    otError error = OT_ERROR_NONE;
    rpl_instance_t *instance;
    rpl_dao_target_list_t *dao_targets;
    uint16_t local_num_dao_targets = 0;

    if (get_current_net_state() != 5 ||
        cfg_props.wisun_device_type != MESH_DEVICE_TYPE_WISUN_BORDER_ROUTER)
    {
        error = OT_ERROR_INVALID_STATE;
    }
    SuccessOrExit(error);

    instance = get_rpl_instance();
    if (instance == NULL)
    {
        SuccessOrExit(error = mEncoder.WriteUint16(0));
        return error;
    }

    dao_targets = &instance->dao_targets;
    if (dao_targets == NULL)
    {
        SuccessOrExit(error = mEncoder.WriteUint16(0));
        return error;
    }

    local_num_dao_targets = ns_list_count(dao_targets); // Max possible number of dao targets
    if (local_num_dao_targets == 0)
    {
        SuccessOrExit(error = mEncoder.WriteUint16(0));
        return error;
    }

    local_num_dao_targets = 0; // Reuse for calculating actual number of dao targets stored
    ns_list_foreach(rpl_dao_target_t, target, dao_targets) {
        if (target->root)
        {
            local_num_dao_targets++;
        }
    }
    SuccessOrExit(error = mEncoder.WriteUint16(local_num_dao_targets));
exit:
    return error;
#else
    return OT_ERROR_NOT_IMPLEMENTED;
#endif
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_CONNECTED_DEVICES>(void)
{
#ifdef HAVE_RPL_ROOT
    otError error = OT_ERROR_NONE;
    rpl_instance_t *instance;
    rpl_dao_target_list_t *dao_targets;
    int i;

    // New call to property get, retrieve all connected devices and set num_dao_targets
    if (num_dao_targets == 0)
    {
        /* Not expected to have a valid pointer here. Added as a safety check to prevent memory leak */
        if(connected_device_ipv6_addrs)
        {
            ns_dyn_mem_free(connected_device_ipv6_addrs);
            connected_device_ipv6_addrs = NULL;
        }

        if (get_current_net_state() != 5 ||
            cfg_props.wisun_device_type != MESH_DEVICE_TYPE_WISUN_BORDER_ROUTER)
        {
            error = OT_ERROR_INVALID_STATE;
        }
        SuccessOrExit(error);

        instance = get_rpl_instance();
        if (instance == NULL)
        {
            SuccessOrExit(error = mEncoder.WriteUint8(1<<7));
            return error;
        }

        dao_targets = &instance->dao_targets;
        if (dao_targets == NULL)
        {
            SuccessOrExit(error = mEncoder.WriteUint8(1<<7));
            return error;
        }

        platform_enter_critical();
        num_dao_targets = ns_list_count(dao_targets); // Max possible number of dao targets
        platform_exit_critical();
        if (num_dao_targets == 0)
        {
            SuccessOrExit(error = mEncoder.WriteUint8(1<<7));
            return error;
        }

        connected_device_ipv6_addrs = (uint8_t *) ns_dyn_mem_alloc(sizeof(uint8_t) * 16 * num_dao_targets);
        if (!connected_device_ipv6_addrs)
        {
            error = OT_ERROR_FAILED;
        }
        SuccessOrExit(error);

        num_dao_targets = 0; // Reuse for calculating actual number of dao targets stored
        block_index = 0;
        platform_enter_critical();
        ns_list_foreach(rpl_dao_target_t, target, dao_targets) {
            if (target->root)
            {
                memcpy(&connected_device_ipv6_addrs[16*num_dao_targets], target->prefix, 16);
                num_dao_targets++;
            }
        }
        platform_exit_critical();

        if (num_dao_targets == 0)
        {
            SuccessOrExit(error = mEncoder.WriteUint8(1<<7));
            return error;
        }
    }

    // Send block of ipv6 addresses if addresses remain
    if (num_dao_targets > 0)
    {
        if (connected_device_ipv6_addrs)
        {
            uint16_t devices_in_block;
            if (num_dao_targets >= CONNECTED_DEVICES_BLOCK_SIZE)
            {
                devices_in_block = CONNECTED_DEVICES_BLOCK_SIZE;
                num_dao_targets -= CONNECTED_DEVICES_BLOCK_SIZE;
            }
            else
            {
                devices_in_block = num_dao_targets;
                num_dao_targets = 0;
            }

            // Write connected devices block header (1 byte):
            // 1 for last block, 0 for blocks remaining
            if (num_dao_targets == 0)
            {
                SuccessOrExit(error = mEncoder.WriteUint8((1<<7) + block_index));
            }
            else
            {
                SuccessOrExit(error = mEncoder.WriteUint8(block_index));
            }

            // Write all IPv6 addresses in block
            for (i = 0; i < devices_in_block; i++)
            {
                SuccessOrExit(error = mEncoder.WriteIp6Address(
                        &connected_device_ipv6_addrs[16 * (num_dao_targets + i)]));
            }
            block_index++;
        }
        else
        {
            error = OT_ERROR_FAILED;
        }
        SuccessOrExit(error);

        if (connected_device_ipv6_addrs && num_dao_targets == 0)
        {
            ns_dyn_mem_free(connected_device_ipv6_addrs);
            connected_device_ipv6_addrs = NULL;
        }
    }
exit:
    if (error != OT_ERROR_NONE)
    {
        num_dao_targets = 0;
        if (connected_device_ipv6_addrs)
        {
            ns_dyn_mem_free(connected_device_ipv6_addrs);
            connected_device_ipv6_addrs = NULL;
        }
    }
    return error;
#else
    return OT_ERROR_NOT_IMPLEMENTED;
#endif
}

template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_DODAG_ROUTE_DEST>(void)
{
#ifdef HAVE_RPL_ROOT
    otError error = OT_ERROR_NONE;
    SuccessOrExit(error = mEncoder.WriteIp6Address(dodag_route_dest_addr));
exit:
    return error;
#else
    return OT_ERROR_NOT_IMPLEMENTED;
#endif
}

template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_DODAG_ROUTE_DEST>(void)
{
#ifdef HAVE_RPL_ROOT
    otError error = OT_ERROR_NONE;
    spinel_ipv6addr_t addr;

    SuccessOrExit(error = mDecoder.ReadIp6Address(addr));

    // Return value 0 indicates success
    memcpy(dodag_route_dest_addr, addr.bytes, 16);
exit:
    return error;
#else
    return OT_ERROR_NOT_IMPLEMENTED;
#endif
}

extern "C" int revoke_gtk_hwaddr(uint8_t* eui64);
template <> otError NcpBase::HandlePropertySet<SPINEL_PROP_REVOKE_GTK_HWADDR>(void)
{
#ifdef HAVE_RPL_ROOT
    spinel_eui64_t extAddress;

    // read the eui address
    mDecoder.ReadEui64(extAddress);

    if(revoke_gtk_hwaddr(extAddress.bytes) < 0)
    {
        return OT_ERROR_FAILED;
    }
    return OT_ERROR_NONE;

#else
    return OT_ERROR_NOT_IMPLEMENTED;
#endif
}

extern "C" rpl_dao_target_t *get_dao_target_from_addr(rpl_instance_t *instance, const uint8_t *addr);
// Destination IPv6 address used by GET DODAG_ROUTE is determined by SET DODAG_ROUTE_DEST
template <> otError NcpBase::HandlePropertyGet<SPINEL_PROP_DODAG_ROUTE>(void)
{
#ifdef HAVE_RPL_ROOT
    otError error = OT_ERROR_NONE;
    struct rpl_instance *instance = NULL;
    bool connected = false;
    uint8_t *route_list = NULL;
    rpl_dao_target_t *dao_target = NULL;
    uint16_t max_targets = 0;
    uint8_t path_cost = 0;
    uint16_t curr_target_index = 0;

    if (get_current_net_state() != 5 ||
        cfg_props.wisun_device_type != MESH_DEVICE_TYPE_WISUN_BORDER_ROUTER)
    {
        error = OT_ERROR_INVALID_STATE;
    }
    SuccessOrExit(error);

    instance = get_rpl_instance();
    if (instance == NULL)
    {
        error = OT_ERROR_NO_ROUTE;
    }
    SuccessOrExit(error);

    dao_target = get_dao_target_from_addr(instance, dodag_route_dest_addr);
    if (dao_target == NULL)
    {
        error = OT_ERROR_NO_ROUTE;
    }
    SuccessOrExit(error);

    max_targets = ns_list_count(&instance->dao_targets); // Max possible number of dao targets
    if (max_targets == 0)
    {
        error = OT_ERROR_NO_ROUTE;
    }
    SuccessOrExit(error);

    route_list = (uint8_t *) ns_dyn_mem_alloc(sizeof(uint8_t) * 16 * max_targets);
    if (!route_list)
    {
        error = OT_ERROR_FAILED;
    }
    SuccessOrExit(error);

    while (1)
    {
        rpl_dao_root_transit_t *transit = ns_list_get_first(&dao_target->info.root.transits);
        rpl_dao_target_t *parent = transit->parent;
        memcpy(&route_list[16*curr_target_index], transit->transit, 16);
        curr_target_index++;
        path_cost += transit->cost;

        // Finished if we hit NULL - ourselves
        if (parent == NULL) {
            connected = true;
            break;
        }
        if (!parent->connected) {
            connected = false;
            break;
        }
        dao_target = parent;
    }

    if (!connected)
    {
        error = OT_ERROR_NO_ROUTE;
    }
    SuccessOrExit(error);

    SuccessOrExit(error = mEncoder.WriteUint8(path_cost)); // Path cost
    for (int i = curr_target_index - 1; i >= 0; i--)
    {
        SuccessOrExit(error = mEncoder.WriteIp6Address(&route_list[16 * i]));
    }
    SuccessOrExit(error = mEncoder.WriteIp6Address(dodag_route_dest_addr));
exit:
    if (route_list)
    {
        ns_dyn_mem_free(route_list);
    }
    if (error == OT_ERROR_NO_ROUTE)
    {
        mEncoder.WriteUint8(0); // Path cost 0
        mEncoder.WriteIp6Address(dodag_route_dest_addr);
        error = OT_ERROR_NONE;
    }
    return error;
#else
    return OT_ERROR_NOT_IMPLEMENTED;
#endif
}

#else // WISUN_NCP_ENABLE
extern "C" void nanostack_process_routing_table_update_from_stack(uint8_t changed_info, uint8_t* prefix, uint8_t len_prefix, uint8_t* addr_nexthop, uint32_t lifetime)
{
    // Stub function
    return;
}

extern "C" void nanostack_process_stream_net_from_stack(buffer_t *buf)
{
    // Stub function
    return;
}


#endif //WISUN_NCP_ENABLE

} // name space Ncp
} // name space ot

