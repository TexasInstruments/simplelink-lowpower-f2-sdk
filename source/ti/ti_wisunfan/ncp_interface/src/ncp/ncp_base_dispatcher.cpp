/*
 *    Copyright (c) 2018, The OpenThread Authors.
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
 *   This file implements general thread device required Spinel interface to the OpenThread stack.
 */

#include "ncp_base.hpp"

namespace ot {
namespace Ncp {

#if __cplusplus >= 201103L
constexpr bool NcpBase::AreHandlerEntriesSorted(const HandlerEntry *aHandlerEntries, size_t aSize)
{
    return aSize < 2 ? true
                     : ((aHandlerEntries[aSize - 1].mKey > aHandlerEntries[aSize - 2].mKey) &&
                        AreHandlerEntriesSorted(aHandlerEntries, aSize - 1));
}

#define OT_NCP_CONST constexpr
#else
#define OT_NCP_CONST const
#endif

NcpBase::PropertyHandler NcpBase::FindGetPropertyHandler(spinel_prop_key_t aKey)
{
#define OT_NCP_GET_HANDLER_ENTRY(aPropertyName) {aPropertyName, &NcpBase::HandlePropertyGet<aPropertyName>}

    OT_NCP_CONST static HandlerEntry sHandlerEntries[] = {
        /* core properties */
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_LAST_STATUS),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_PROTOCOL_VERSION),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_NCP_VERSION),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_INTERFACE_TYPE),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_HWADDR),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_TRX_FW_VERSION),
        /* PHY properties */
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_PHY_CCA_THRESHOLD),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_PHY_TX_POWER),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_PHY_NUM_NBRS),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_PHY_NBR_METRICS),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_PHY_METRICS),
        /* MAC properties */
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_MAC_15_4_PANID),
        /* NET properties */
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_NET_IF_UP),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_NET_STACK_UP),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_NET_ROLE),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_NET_NETWORK_NAME),
        /* Tech specific: PHY properties */
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_PHY_REGION),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_PHY_MODE_ID),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_PHY_UNICAST_CHANNEL_LIST),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_PHY_BROADCAST_CHANNEL_LIST),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_PHY_ASYNC_CHANNEL_LIST),
        /* Tech specific: MAC properties */
        /* Tech specific: NET properties */
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_NET_STATE),
//        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_PARENT_LIST),
//        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_ROUTING_COST),
//        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_LAST_DAO_RCVD_INFO), //mvtodo: revisit
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_DODAG_ROUTE_DEST),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_DODAG_ROUTE),
        /* IPv6 properties */
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_IPV6_ADDRESS_TABLE),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_MULTICAST_LIST),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_NUM_CONNECTED_DEVICES),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_CONNECTED_DEVICES),
        /* Stream Properties */
        /* Tech specific: PHY Extended properties */
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_PHY_CH_SPACING),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_PHY_CHO_CENTER_FREQ),
        /* Tech specific: MAC Extended properties */
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_MAC_UC_DWELL_INTERVAL),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_MAC_BC_DWELL_INTERVAL),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_MAC_BC_INTERVAL),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_MAC_UC_CHANNEL_FUNCTION),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_MAC_BC_CHANNEL_FUNCTION),
#ifdef TI_WISUN_FAN_DEBUG
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_MAC_MAC_FILTER_LIST),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_MAC_FILTER_MODE),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_TEST_COMMAND),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_VPIE_COMMAND),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_MACMPL_COMMAND),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_EXTERNAL_DHCP_SERVER_ENABLED),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_EXTERNAL_DHCP_SERVER_ADDRESS),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_EXTERNAL_AUTH_SERVER_ENABLED),
        OT_NCP_GET_HANDLER_ENTRY(SPINEL_PROP_EXTERNAL_AUTH_SERVER_ADDRESS),
#endif
        /* Tech specific: NET Extended properties */
    };

#undef OT_NCP_GET_HANDLER_ENTRY

#if __cplusplus >= 201103L
    static_assert(AreHandlerEntriesSorted(sHandlerEntries, OT_ARRAY_LENGTH(sHandlerEntries)),
                  "NCP property getter entries not sorted!");
#endif

    return FindPropertyHandler(sHandlerEntries, OT_ARRAY_LENGTH(sHandlerEntries), aKey);
}

NcpBase::PropertyHandler NcpBase::FindSetPropertyHandler(spinel_prop_key_t aKey)
{
#define OT_NCP_SET_HANDLER_ENTRY(aPropertyName) {aPropertyName, &NcpBase::HandlePropertySet<aPropertyName>}

    OT_NCP_CONST static HandlerEntry sHandlerEntries[] = {
#if 0
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_POWER_STATE),
#if OPENTHREAD_CONFIG_NCP_ENABLE_MCU_POWER_STATE_CONTROL
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_MCU_POWER_STATE),
#endif
#if OPENTHREAD_RADIO || OPENTHREAD_CONFIG_LINK_RAW_ENABLE
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_PHY_ENABLED),
#endif
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_PHY_CHAN),
#if OPENTHREAD_MTD || OPENTHREAD_FTD
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_PHY_CHAN_SUPPORTED),
#endif
#endif
        /* core properties */
#ifdef WRITABLE_HWADDR
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_HWADDR),
#endif
        /* PHY properties */
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_PHY_CCA_THRESHOLD),
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_PHY_TX_POWER),
        /* MAC properties */
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_MAC_15_4_PANID),
        /* NET properties */
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_NET_IF_UP),
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_NET_STACK_UP),
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_NET_NETWORK_NAME),
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_NET_UDP_START),
        /* Tech specific: PHY properties */
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_PHY_UNICAST_CHANNEL_LIST),
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_PHY_BROADCAST_CHANNEL_LIST),
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_PHY_ASYNC_CHANNEL_LIST),
        /* Tech specific: MAC properties */
        /* Tech specific: NET properties */
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_DODAG_ROUTE_DEST),
        /* IPv6 properties */
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_STREAM_NET),
        /* Tech specific: PHY Extended properties */
        /* Tech specific: MAC Extended properties */

        OT_NCP_SET_HANDLER_ENTRY(SPINEL_MAC_UC_DWELL_INTERVAL),
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_MAC_BC_DWELL_INTERVAL),
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_MAC_BC_INTERVAL),
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_MAC_UC_CHANNEL_FUNCTION),
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_MAC_BC_CHANNEL_FUNCTION),
#ifdef TI_WISUN_FAN_DEBUG
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_MAC_FILTER_MODE),
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_TEST_COMMAND),
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_VPIE_COMMAND),
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_MACMPL_COMMAND),
#endif
        /* Tech specific: NET Extended properties */
        OT_NCP_SET_HANDLER_ENTRY(SPINEL_PROP_REVOKE_GTK_HWADDR),
    };

#undef OT_NCP_SET_HANDLER_ENTRY

#if __cplusplus >= 201103L
    static_assert(AreHandlerEntriesSorted(sHandlerEntries, OT_ARRAY_LENGTH(sHandlerEntries)),
                  "NCP property setter entries not sorted!");
#endif

    return FindPropertyHandler(sHandlerEntries, OT_ARRAY_LENGTH(sHandlerEntries), aKey);
}

NcpBase::PropertyHandler NcpBase::FindInsertPropertyHandler(spinel_prop_key_t aKey)
{
#define OT_NCP_INSERT_HANDLER_ENTRY(aPropertyName) {aPropertyName, &NcpBase::HandlePropertyInsert<aPropertyName>}

    OT_NCP_CONST static HandlerEntry sHandlerEntries[] = {
#ifdef TI_WISUN_FAN_DEBUG
        OT_NCP_INSERT_HANDLER_ENTRY(SPINEL_PROP_MULTICAST_LIST),
        OT_NCP_INSERT_HANDLER_ENTRY(SPINEL_PROP_MAC_MAC_FILTER_LIST),
#endif
    };

#undef OT_NCP_INSERT_HANDLER_ENTRY

#if __cplusplus >= 201103L
    static_assert(AreHandlerEntriesSorted(sHandlerEntries, OT_ARRAY_LENGTH(sHandlerEntries)),
                  "NCP property setter entries not sorted!");
#endif

    return FindPropertyHandler(sHandlerEntries, OT_ARRAY_LENGTH(sHandlerEntries), aKey);
}

NcpBase::PropertyHandler NcpBase::FindRemovePropertyHandler(spinel_prop_key_t aKey)
{
#define OT_NCP_REMOVE_HANDLER_ENTRY(aPropertyName) {aPropertyName, &NcpBase::HandlePropertyRemove<aPropertyName>}

    OT_NCP_CONST static HandlerEntry sHandlerEntries[] = {
#ifdef TI_WISUN_FAN_DEBUG
        OT_NCP_REMOVE_HANDLER_ENTRY(SPINEL_PROP_MULTICAST_LIST),
        OT_NCP_REMOVE_HANDLER_ENTRY(SPINEL_PROP_MAC_MAC_FILTER_LIST),
#endif
    };

#undef OT_NCP_REMOVE_HANDLER_ENTRY

#if __cplusplus >= 201103L
    static_assert(AreHandlerEntriesSorted(sHandlerEntries, OT_ARRAY_LENGTH(sHandlerEntries)),
                  "NCP property setter entries not sorted!");
#endif

    return FindPropertyHandler(sHandlerEntries, OT_ARRAY_LENGTH(sHandlerEntries), aKey);
}

NcpBase::PropertyHandler NcpBase::FindPropertyHandler(const HandlerEntry *aHandlerEntries,
                                                      size_t              aSize,
                                                      spinel_prop_key_t   aKey)
{
    size_t l = 0;

    OT_ASSERT(aSize > 0);

    for (size_t r = aSize - 1; l < r;)
    {
        size_t m = (l + r) / 2;

        if (aHandlerEntries[m].mKey < aKey)
        {
            l = m + 1;
        }
        else
        {
            r = m;
        }
    }

    //return aHandlerEntries[l].mKey == aKey ? aHandlerEntries[l].mHandler : NULL;
    //temporary hack - till other properties are implemented for wisun. Return the last handler entry if property not found
    return aHandlerEntries[l].mKey == aKey ? aHandlerEntries[l].mHandler : aHandlerEntries[(aSize-1)].mHandler;
}

} // namespace Ncp
} // namespace ot
