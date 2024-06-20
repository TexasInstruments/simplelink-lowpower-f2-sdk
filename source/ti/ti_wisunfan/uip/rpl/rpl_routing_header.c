/*
 * Copyright (c) 2014 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free,
 * non-exclusive license under copyrights and patents it now or hereafter
 * owns or controls to make, have made, use, import, offer to sell and sell ("Utilize")
 * this software subject to the terms herein.  With respect to the foregoing patent
 *license, such license is granted  solely to the extent that any such patent is necessary
 * to Utilize the software alone.  The patent license shall not apply to any combinations which
 * include this software, other than combinations with devices manufactured by or for TI (“TI Devices”).
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license (including the
 * above copyright notice and the disclaimer and (if applicable) source code license limitations below)
 * in the documentation and/or other materials provided with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided that the following
 * conditions are met:
 *
 *       * No reverse engineering, decompilation, or disassembly of this software is permitted with respect to any
 *     software provided in binary form.
 *       * any redistribution and use are licensed by TI for use only with TI Devices.
 *       * Nothing shall obligate TI to provide you with source code for the software licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the source code are permitted
 * provided that the following conditions are met:
 *
 *   * any redistribution and use of the source code, including any resulting derivative works, are licensed by
 *     TI for use only with TI Devices.
 *   * any redistribution and use of any object code compiled from the source code and any resulting derivative
 *     works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers may be used to endorse or
 * promote products derived from this software without specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI’S LICENSORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI’S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ====================== rpl-routing-header.c =============================================
 *  This file is created to contain the functions that were added for RPL++ improvements 
 */

#include <string.h>
#include "net/uip.h"
#include "rpl/rpl-routing-header.h"
#include "net/uip-icmp6.h"
#include "net/uip-ds6.h"
#include "rpl/rpl-private.h"

#if (RPL_CONF_MOP == RPL_MOP_NON_STORING) //Do not compile if we will not use it

#define TEST_RPL_ROUTING_HDR_PROCESSING 0

#define TEST_RPL_ROUTING_HDR_DIFFERENT_CMPRI_CMPRE 0

#define TEST_RPL_ROUTING_HDR_SEG_LEFT 0

#define TEST_RPL_ROUTING_HDR_MULTICAST_ADDRESS 0

#define TEST_RPL_ROUTING_HDR_TTL 0

#ifdef DEBUG  
#undef DEBUG
#endif
#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#define UIP_IP_BUF                			((struct uip_ip_hdr *)&rpl_ip_buf[UIP_LLH_LEN])
#define UIP_EXT_BUF               			((struct uip_ext_hdr *)&rpl_ip_buf[uip_l2_l3_hdr_len])
#define UIP_ROUTING_HDR_RPL_BUF   			((struct uip_routing_hdr_rpl *)&rpl_ip_buf[uip_l2_l3_hdr_len + uip_ext_opt_offset])
#define UIP_ROUTING_HDR_RPL_NEXT_BUF         ((struct uip_ext_hdr *)&rpl_ip_buf[uip_l2_l3_hdr_len + CHAR_BIT_UNITS(uip_routing_hdr_rpl_len)])//This will work as 'uip_routing_hdr_rpl_len' will be a multiple of 8, thus an even value

#if NON_STORING_MODE_ROOT

#if USE_RPL_HBHO_HEADER
//Declaration for removing HBHO extension header in case it is there.
void rpl_remove_header(void);
#endif

/*
 * For simplifying the processing for compression and the use of buffers (less memory to use), the maximum number of octets shared by the final destination and each hop's destination will be compressed and will remain unchanged whenever a hop process the header.
 * This is to say that the values of CmprI and cmprE will always be the same. In this way, the size of the routing header is always the same, even after processing it and swapping the next hop's address with the ipv6 destination address, which allows us to avoid using a secondary buffer for recreating the header.
 */
u16_t numberOfSharedOctets(uip_ipaddr_t* addr1, uip_ipaddr_t* addr2)
{
    u16_t sharedOctets = 0;

    //while (addr1->u8[sharedOctets] == addr2->u8[sharedOctets]){
    while (pltfrm_byte_get(addr1->u16,sharedOctets) == pltfrm_byte_get(addr2->u16,sharedOctets))
    {
        ++sharedOctets;
    }
    return sharedOctets;
}

u16_t numberOfSharedOctetsPrefix(uip_ds6_sourceRouting_path_t *path)
{
    u16_t max_shared_octets = sizeof(uip_ipaddr_t); //Max possible shared octets (16 bytes for ipv6 addresses)
    u16_t shared_octets = 0;
    uip_ipaddr_t* prev_ipaddr = path->ipaddr;

    path = uip_ds6_sourceRouting_path_next(path);
    while (path)
    {
        shared_octets = numberOfSharedOctets(prev_ipaddr, path->ipaddr);
        if (max_shared_octets > shared_octets)
        {
            //if these two addresses share less octets than the max number of octets shared by any other pair, replace the max_shared_octets by this value
            max_shared_octets = shared_octets;
        }
        prev_ipaddr = path->ipaddr;
        path = uip_ds6_sourceRouting_path_next(path);
    }
    return max_shared_octets;
}

u16_t calculatePaddingBytes(u16_t uip_routing_hdr_rpl_len)
{
    return (uip_routing_hdr_rpl_len % 8)? (8 - (uip_routing_hdr_rpl_len % 8)) : 0;
}

int rpl_insert_routing_header(uip_ds6_sourceRouting_path_t *path)
{
#if TEST_RPL_ROUTING_HDR_MULTICAST_ADDRESS
    uip_ipaddr_t multicast_addr;
    uip_create_linklocal_allnodes_mcast(&multicast_addr);
    uip_ds6_sourceRouting_path_next(path)->ipaddr = &multicast_addr;
#endif
    int uip_ext_opt_offset = 0;
    int pos = 0;
    //ALL these in BYTE UNITS
    u16_t shared_octets_prefix = numberOfSharedOctetsPrefix(path);
    u16_t notShared_octets_address = BYTE_UNITS(sizeof(uip_ipaddr_t))-shared_octets_prefix;
    u16_t uip_routing_hdr_rpl_len = BYTE_UNITS(UIP_ROUTING_HDR_RPL_INITIAL_LEN) + ((uip_ds6_sourceRouting_path_num_nodes()-1)*notShared_octets_address);
    u16_t padding = calculatePaddingBytes(uip_routing_hdr_rpl_len);

    uip_routing_hdr_rpl_len += padding;//Now the hdr length is multiple of 8
    //Check if there is enough room for the header!
    if (CHAR_BIT_UNITS(uip_routing_hdr_rpl_len) + CHAR_BIT_UNITS(rpl_ip_len) > RPL_BUFSIZE)
    {
        //No room for routing header
        return 0;
    }
#if USE_RPL_HBHO_HEADER
    //Make sure there is no other RPL header. Downward traffic does not need a HBHO header
    rpl_remove_header();
#endif
    //Now add the new extension header
    if (UIP_IP_BUF->proto != UIP_PROTO_ROUTING)
    {    //Add Source Routing Header
        memmove(UIP_ROUTING_HDR_RPL_NEXT_BUF, UIP_EXT_BUF, CHAR_BIT_UNITS(rpl_ip_len) - UIP_IPH_LEN);
        memset(UIP_ROUTING_HDR_RPL_BUF, 0, CHAR_BIT_UNITS(uip_routing_hdr_rpl_len));

        UIP_ROUTING_HDR_RPL_BUF->routing_type = UIP_ROUTING_HDR_RPL_TYPE;
        UIP_ROUTING_HDR_RPL_BUF->pad_rsrv = (padding << 4);//Only 4 bits of padding
        UIP_ROUTING_HDR_RPL_BUF->rsrv0 = 0;
        UIP_ROUTING_HDR_RPL_BUF->rsrv1 = 0;
        UIP_ROUTING_HDR_RPL_BUF->next = UIP_IP_BUF->proto;
        UIP_IP_BUF->proto = UIP_PROTO_ROUTING;

#if TEST_RPL_ROUTING_HDR_SEG_LEFT
        UIP_ROUTING_HDR_RPL_BUF->seg_left = uip_ds6_sourceRouting_path_num_nodes()+1;
#else
        UIP_ROUTING_HDR_RPL_BUF->seg_left = uip_ds6_sourceRouting_path_num_nodes()-1;
#endif

#if TEST_RPL_ROUTING_HDR_DIFFERENT_CMPRI_CMPRE
        UIP_ROUTING_HDR_RPL_BUF->cmprI_cmprE = 1; //values are 0 for CmprI and 1 for CmprE
#else
        UIP_ROUTING_HDR_RPL_BUF->cmprI_cmprE = (shared_octets_prefix << 4) | (shared_octets_prefix & 0x0F); //It is the same value for both I and E
#endif
        /*
         * RFC 6554, Section 3:  Hdr Ext Len         8-bit unsigned integer.  Length of the Routing
         header in 8-octet units, not including the first
         8 octets.  Note that when Addresses[1..n] are
         compressed (i.e., value of CmprI or CmprE is not
         0), Hdr Ext Len does not equal twice the number
         of Addresses.
         *
         */
        UIP_ROUTING_HDR_RPL_BUF->len = (uip_routing_hdr_rpl_len-BYTE_UNITS(UIP_ROUTING_HDR_RPL_INITIAL_LEN))/8; //octet units. do not count the initial octet. Includes addresses and padding

        //put the addresses to the header
        //first one is the next hop, omit it
        path = uip_ds6_sourceRouting_path_next(path);
        while (path)
        {
            //memcpy((((u8_t*)UIP_ROUTING_HDR_RPL_BUF) + UIP_ROUTING_HDR_RPL_INITIAL_LEN)+pos, ((u8_t*)path->ipaddr) + shared_octets_prefix, notShared_octets_address);
            pltfrm_byte_memcpy((char*)UIP_ROUTING_HDR_RPL_BUF + UIP_ROUTING_HDR_RPL_INITIAL_LEN, pos, path->ipaddr, shared_octets_prefix, notShared_octets_address);
            pos += notShared_octets_address;
            path = uip_ds6_sourceRouting_path_next(path);
        }
        rpl_ip_len += uip_routing_hdr_rpl_len;
        UIP_IP_BUF->len0 = ((rpl_ip_len - BYTE_UNITS(UIP_IPH_LEN)) >> 8);
        UIP_IP_BUF->len1 = ((rpl_ip_len - BYTE_UNITS(UIP_IPH_LEN)) & 0xff);

#if TEST_RPL_ROUTING_HDR_TTL
        UIP_IP_BUF->ttl = 1;
#endif
    }
    return 1;
}
#endif //NON_STORING_MODE_ROOT

int computeIndex(int n, u16_t shared_octets_prefix, int uip_ext_opt_offset)
{
    //int uip_ext_opt_offset = 0;
    //return ((char*)UIP_ROUTING_HDR_RPL_BUF) + UIP_ROUTING_HDR_RPL_INITIAL_LEN + ((n - UIP_ROUTING_HDR_RPL_BUF->seg_left)*(sizeof(uip_ipaddr_t)-shared_octets_prefix));
    return BYTE_UNITS(UIP_ROUTING_HDR_RPL_INITIAL_LEN) + ((n - UIP_ROUTING_HDR_RPL_BUF->seg_left)*(BYTE_UNITS(sizeof(uip_ipaddr_t))-shared_octets_prefix));
}

void computeNextHopAddress(int i, uip_ipaddr_t *next_address, uip_ipaddr_t *dest_address, u16_t shared_octets_prefix, int uip_ext_opt_offset)
{
    uip_ipaddr_copy(next_address, dest_address);
    //memcpy(((u8_t*)next_address) + shared_octets_prefix, i, sizeof(uip_ipaddr_t)-shared_octets_prefix);
    pltfrm_byte_memcpy((char*)next_address, shared_octets_prefix, (char*)UIP_ROUTING_HDR_RPL_BUF, i, BYTE_UNITS(sizeof(uip_ipaddr_t))-shared_octets_prefix);
}

char addressLoop(u16_t shared_octets_prefix, int uip_ext_opt_offset)
{
    uip_ipaddr_t address;
    int current_index = 0;
    int following_index = 0;
    //int uip_ext_opt_offset = 0;
    int header_length = UIP_ROUTING_HDR_RPL_BUF->len*8;
    u16_t notShared_octets_address = BYTE_UNITS(sizeof(uip_ipaddr_t))-shared_octets_prefix;

    while (current_index < header_length)
    {
        //get current address
        //computeNextHopAddress(((char*)UIP_ROUTING_HDR_RPL_BUF) + UIP_ROUTING_HDR_RPL_INITIAL_LEN + current_index, &address, &UIP_IP_BUF->destipaddr, shared_octets_prefix);
        computeNextHopAddress(BYTE_UNITS(UIP_ROUTING_HDR_RPL_INITIAL_LEN) + current_index, &address, &UIP_IP_BUF->destipaddr, shared_octets_prefix, uip_ext_opt_offset);
        current_index += notShared_octets_address;
        //check if address is interface address of this node
        if (uip_ds6_is_my_addr(&address))
        {
            //omit next address and go to the following one
            following_index = current_index + notShared_octets_address;
            //check for all following addresses if they are not an interface address of this node
            while(following_index < header_length)
            {
                //get the address
                //computeNextHopAddress(((char*)UIP_ROUTING_HDR_RPL_BUF) + UIP_ROUTING_HDR_RPL_INITIAL_LEN + following_index, &address, &UIP_IP_BUF->destipaddr, shared_octets_prefix);
                computeNextHopAddress(BYTE_UNITS(UIP_ROUTING_HDR_RPL_INITIAL_LEN) + following_index, &address, &UIP_IP_BUF->destipaddr, shared_octets_prefix, uip_ext_opt_offset);
                following_index += notShared_octets_address;
                if (uip_ds6_is_my_addr(&address))
                {
                    //in this case, there are two addresses of this node separated by at least 1 address. Error message
                    return 1;
                }
            }
        }
        //continue to next address
    }
    return 0;

}

void swapDestinationAndNextAddress(int i, u16_t shared_octets_prefix, int uip_ext_opt_offset)
{
    uip_ipaddr_t temp_address;
    //memcpy(&temp_address, address_i_ptr, sizeof(uip_ipaddr_t)-shared_octets_prefix);
    pltfrm_byte_memcpy(&temp_address, 0, (char*)UIP_ROUTING_HDR_RPL_BUF, i, BYTE_UNITS(sizeof(uip_ipaddr_t))-shared_octets_prefix);
    //memcpy(address_i_ptr, ((u8_t*)&UIP_IP_BUF->destipaddr)+shared_octets_prefix, sizeof(uip_ipaddr_t)-shared_octets_prefix);
    pltfrm_byte_memcpy((char*)UIP_ROUTING_HDR_RPL_BUF, i, (char*)&UIP_IP_BUF->destipaddr, shared_octets_prefix, BYTE_UNITS(sizeof(uip_ipaddr_t))-shared_octets_prefix);
    //memcpy(((u8_t*)&UIP_IP_BUF->destipaddr)+shared_octets_prefix, &temp_address, sizeof(uip_ipaddr_t)-shared_octets_prefix);
    pltfrm_byte_memcpy((char*)&UIP_IP_BUF->destipaddr, shared_octets_prefix, &temp_address, 0, BYTE_UNITS(sizeof(uip_ipaddr_t))-shared_octets_prefix);
}

int rpl_process_routing_header()
{
    int uip_ext_opt_offset = 0;
    u16_t shared_octets_prefix = UIP_ROUTING_HDR_RPL_BUF->cmprI_cmprE & 0x0f; //both values must be the same
    int n = (((UIP_ROUTING_HDR_RPL_BUF->len * 8) - ((UIP_ROUTING_HDR_RPL_BUF->pad_rsrv & 0xf0) >> 4) - (16 - (UIP_ROUTING_HDR_RPL_BUF->cmprI_cmprE & 0x0f))) / (16 - ((UIP_ROUTING_HDR_RPL_BUF->cmprI_cmprE & 0xf0) >> 4))) + 1;
    int i = 0;
    uip_ipaddr_t next_address;
    rpl_dag_t * dag = NULL;

    if (default_instance)
    {
        dag = default_instance->current_dag;
    }

    //Our compression does not accept different values in cmprI and cmprE. Check that they are the same
    if ((UIP_ROUTING_HDR_RPL_BUF->cmprI_cmprE >> 4) != shared_octets_prefix)
    {
        //Not the same? discard packet
#if TEST_RPL_ROUTING_HDR_PROCESSING
        pltfrm_debug("RPL ROUTING HDR PROCESSING: Error. CmprI and CmprE are not equivalent\n");
#endif
        //discard the packet
        return UIP_SOURCE_ROUTING_HDR_DISCARD;
    }

    if (UIP_ROUTING_HDR_RPL_BUF->seg_left > n)
    {
#if TEST_RPL_ROUTING_HDR_PROCESSING
        pltfrm_debug("RPL ROUTING HDR PROCESSING: Error. Segments left > n\n");
#endif
        /*
         RFC 6554, Section 4.2: send an ICMP Parameter Problem, Code 0, message to the Source
         Address, pointing to the Segments Left field, and discard the
         packet
         */
        if(dag != NULL)
        {
            uip_icmp6_error_output_destination(ICMP6_PARAM_PROB, ICMP6_PARAMPROB_HEADER, UIP_ROUTING_HDR_RPL_BUF->seg_left, &dag->dag_id); //UIP_IPH_LEN + rpl_ip_ext_len + 2);
        }
        return UIP_SOURCE_ROUTING_HDR_ERROR_MSG;
    }
    else
    {
        i = computeIndex(n, shared_octets_prefix, uip_ext_opt_offset);
        UIP_ROUTING_HDR_RPL_BUF->seg_left--;
        computeNextHopAddress(i, &next_address, &UIP_IP_BUF->destipaddr, shared_octets_prefix, uip_ext_opt_offset);
#if TEST_RPL_ROUTING_HDR_PROCESSING
        pltfrm_debug("RPL ROUTING HDR PROCESSING: Next hop is: ");
        pltfrm_debug_address(&next_address);
#endif
        if (uip_is_addr_mcast(&next_address))
        {
#if TEST_RPL_ROUTING_HDR_PROCESSING
            pltfrm_debug("RPL ROUTING HDR PROCESSING: Error. Next hop is multicast. Discarding..\n");
#endif
            //discard the packet
            return UIP_SOURCE_ROUTING_HDR_DISCARD;
        }
        /*
         *     RFC 6554, Section 4.2:  if 2 or more entries in Address[1..n] are assigned to
         local interface and are separated by at least one
         address not assigned to local interface
         */
        else if (addressLoop(shared_octets_prefix, uip_ext_opt_offset))
        {
#if TEST_RPL_ROUTING_HDR_PROCESSING
            pltfrm_debug("RPL ROUTING HDR PROCESSING: Error. Address loop detected. Discarding..\n");
#endif
            //RFC 6554, Section 4.2: send an ICMP Parameter Problem (Code 0) and discard the packet
            if(dag != NULL)
            {
                uip_icmp6_error_output_destination(ICMP6_PARAM_PROB, ICMP6_PARAMPROB_HEADER, UIP_ROUTING_HDR_RPL_BUF->seg_left, &dag->dag_id); //UIP_IPH_LEN + rpl_ip_ext_len + 2);
            }
            return UIP_SOURCE_ROUTING_HDR_ERROR_MSG;
        }
        else
        {
            //RFC 6554, Section 4.2: swap the IPv6 Destination Address and Address[i]
            swapDestinationAndNextAddress(i, shared_octets_prefix, uip_ext_opt_offset);
            /* RFC 6554, Section 4.2: the IPv6 Hop Limit is less than or equal to 1 */
            if(UIP_IP_BUF->ttl <= 1)
            {
#if TEST_RPL_ROUTING_HDR_PROCESSING
                pltfrm_debug("RPL ROUTING HDR PROCESSING: Error. TTL <= 1. Discarding..\n");
#endif
                /*
                 * 	RFC 6554, Section 4.2: send an ICMP Time Exceeded -- Hop Limit Exceeded in
                 Transit message to the Source Address and discard the
                 packet
                 */
                if(dag != NULL)
                {
                    uip_icmp6_error_output_destination(ICMP6_TIME_EXCEEDED, ICMP6_TIME_EXCEED_TRANSIT, 0, &dag->dag_id);
                }
                return UIP_SOURCE_ROUTING_HDR_ERROR_MSG;
            }
            else
            {
#if TEST_RPL_ROUTING_HDR_PROCESSING
                pltfrm_debug("RPL ROUTING HDR PROCESSING: Ready to forward\n");
#endif
                //RFC 6554, Section 4.2: decrement the Hop Limit by 1
                UIP_IP_BUF->ttl = UIP_IP_BUF->ttl - 1;
                PRINTF("Forwarding packet to ");
                PRINT6ADDR(&UIP_IP_BUF->destipaddr);
                PRINTF("");
                UIP_STAT(++uip_stat.ip.forwarded);
                /*
                 * RFC 6554, Section 4.2: resubmit the packet to the IPv6 module for transmission to the new destination
                 */
                return UIP_SOURCE_ROUTING_HDR_FWD_MSG;
            }
        }
    }
}

int rpl_ipv6_routing_header_extension(int action, void *data)
{
#if NON_STORING_MODE_ROOT
    uip_ds6_sourceRouting_path_t *path;
#endif
    switch(action)
    {
#if NON_STORING_MODE_ROOT
        case RPL_SOURCE_ROUTING_HDR_INSERT:
        path = (uip_ds6_sourceRouting_path_t *)data;
        return rpl_insert_routing_header(path);
#endif
        case RPL_SOURCE_ROUTING_HDR_PROCESS:
        return rpl_process_routing_header();
    }
    return 0;
}

#endif //(RPL_CONF_MOP == RPL_MOP_NON_STORING)
