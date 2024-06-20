/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *         ICMPv6 echo request and error messages (RFC 4443)
 * \author Julien Abeille <jabeille@cisco.com>
 * \author Mathilde Durvy <mdurvy@cisco.com>
 */

/*
 * Copyright (c) 2001-2003, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 */
/******************************************************************************
 *
 * Copyright (c) 2014 Texas Instruments Inc.  All rights reserved.
 *
 * DESCRIPTION:
 *
 * HISTORY:
 *
 *
 ******************************************************************************/

#include <string.h>
#include "net/uip-ds6.h"
#include "net/uip-icmp6.h"

#include "uip_rpl_process.h"

#ifdef DEBUG  
#undef DEBUG
#endif
#define DEBUG DEBUG_NONE
#include "uip-debug.h"

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_ICMP6_ERROR_BUF  ((struct uip_icmp6_error *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_EXT_BUF              ((struct uip_ext_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_FIRST_EXT_BUF        ((struct uip_ext_hdr *)&uip_buf[UIP_LLIPH_LEN])

/** \brief temporary IP address */
static uip_ipaddr_t tmp_ipaddr;

#if UIP_CONF_IPV6
/*---------------------------------------------------------------------------*/
void uip_icmp6_echo_request_output(uip_ipaddr_t *dest, uint16_t payload_byte,
                                   uint16_t payload_length)
{
    UIP_IP_BUF->vtc = 0x60;
    UIP_IP_BUF->tcflow = 0;
    UIP_IP_BUF->flow = 0;
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
    UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, dest);
    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
    UIP_ICMP_BUF->type = ICMP6_ECHO_REQUEST;
    UIP_ICMP_BUF->icode = 0;
    /* set identifier and sequence number to 0 */
    pltfrm_byte_memset(UIP_ICMP_BUF, BYTE_UNITS(UIP_ICMPH_LEN), 0, 4);
    /* put the data: repeat the ping counter the Payload number of times */
    pltfrm_byte_memset(UIP_ICMP_BUF,
                       BYTE_UNITS(UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN),
                       payload_byte, payload_length);
    uip_len= BYTE_UNITS(UIP_ICMPH_LEN) + BYTE_UNITS(UIP_ICMP6_ECHO_REQUEST_LEN) + BYTE_UNITS(UIP_IPH_LEN) + payload_length;
    UIP_IP_BUF->len0 = ((uip_len- BYTE_UNITS(UIP_IPH_LEN)) >> 8);
    UIP_IP_BUF->len1 = ((uip_len- BYTE_UNITS(UIP_IPH_LEN)) & 0x00FF);
    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();
    //Debugging messages
    PRINTF("Sending Echo Request to"); PRINT6ADDR(&UIP_IP_BUF->destipaddr); PRINTF("from"); PRINT6ADDR(&UIP_IP_BUF->srcipaddr); PRINTF("\n");
    //Try to schedule
    tcpip_ipv6_output();
}
void uip_icmp6_echo_request_input(void)
{
    u16_t temp_ext_len;
    /*
     * we send an echo reply. It is trivial if there was no extension
     * headers in the request otherwise we need to remove the extension
     * headers and change a few fields
     */
    PRINTF("Received Echo Request from"); PRINT6ADDR(&UIP_IP_BUF->srcipaddr); PRINTF("to"); PRINT6ADDR(&UIP_IP_BUF->destipaddr); PRINTF("");

    /* IP header */
    UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;

    if (uip_is_addr_mcast(&UIP_IP_BUF->destipaddr))
    {
        uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &UIP_IP_BUF->srcipaddr);
        uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
    }
    else
    {
        uip_ipaddr_copy(&tmp_ipaddr, &UIP_IP_BUF->srcipaddr);
        uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
        uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &tmp_ipaddr);
    }

    if (uip_ext_len > 0)
    {
        temp_ext_len = UIP_HBHO_HDR_EXTENSION(UIP_HBHO_HDR_INVERT, NULL);
        if (temp_ext_len != 0)
        {
            /* If there were other extension headers*/
            UIP_FIRST_EXT_BUF->next = UIP_PROTO_ICMP6;
            if (uip_ext_len != temp_ext_len)
            {
                uip_len-= BYTE_UNITS(uip_ext_len - temp_ext_len);
                UIP_IP_BUF->len0 = ((uip_len - BYTE_UNITS(UIP_IPH_LEN)) >> 8);
                UIP_IP_BUF->len1 = ((uip_len - BYTE_UNITS(UIP_IPH_LEN)) & 0xff);
                /* move the echo request payload (starting after the icmp header)
                 * to the new location in the reply.
                 * The shift is equal to the length of the remaining extension headers present
                 * Note: UIP_ICMP_BUF still points to the echo request at this stage
                 */
                memmove((char *)UIP_ICMP_BUF + UIP_ICMPH_LEN - (uip_ext_len - temp_ext_len),
                        (char *)UIP_ICMP_BUF + UIP_ICMPH_LEN,
                        (CHAR_BIT_UNITS(uip_len) - UIP_IPH_LEN - temp_ext_len - UIP_ICMPH_LEN));
            }
            uip_ext_len = temp_ext_len;
        }
        else
        {
            /* If there were extension headers*/
            UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
            uip_len-= BYTE_UNITS(uip_ext_len);
            UIP_IP_BUF->len0 = ((uip_len- BYTE_UNITS(UIP_IPH_LEN)) >> 8);
            UIP_IP_BUF->len1 = ((uip_len- BYTE_UNITS(UIP_IPH_LEN)) & 0xff);
            /* move the echo request payload (starting after the icmp header)
             * to the new location in the reply.
             * The shift is equal to the length of the extension headers present
             * Note: UIP_ICMP_BUF still points to the echo request at this stage
             */
            memmove((char *) UIP_ICMP_BUF + UIP_ICMPH_LEN - uip_ext_len,
                    (char *) UIP_ICMP_BUF + UIP_ICMPH_LEN,
                    (CHAR_BIT_UNITS(uip_len) - UIP_IPH_LEN - UIP_ICMPH_LEN));
            uip_ext_len = 0;
        }
    }
    /* Below is important for the correctness of UIP_ICMP_BUF and the
     * checksum
     */

    /* Note: now UIP_ICMP_BUF points to the beginning of the echo reply */
    UIP_ICMP_BUF->type = ICMP6_ECHO_REPLY;
    UIP_ICMP_BUF->icode = 0;
    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

    PRINTF("Sending Echo Reply to"); PRINT6ADDR(&UIP_IP_BUF->destipaddr); PRINTF("from"); PRINT6ADDR(&UIP_IP_BUF->srcipaddr); PRINTF(""); UIP_STAT(++uip_stat.icmp.sent); UIP_STAT_TX_ECHO_REP_NUM_INC;
    UIP_STAT_TX_ICMP_NUM_INC;
    return;
}
/*---------------------------------------------------------------------------*/
void uip_icmp6_error_output(u16_t type, u16_t code, u32_t param)
{
    uip_ipaddr_t destination;

    //The destination for the error message must be the source of the packet
    uip_ipaddr_copy(&destination, &UIP_IP_BUF->srcipaddr);
    uip_icmp6_error_output_destination(type, code, param, &destination);
}

void uip_icmp6_error_output_destination(u16_t type, u16_t code, u32_t param,
                                        uip_ipaddr_t* destination)
{

    /* check if originating packet is not an ICMP error*/
    if (uip_ext_len)
    {
        if (UIP_EXT_BUF->next == UIP_PROTO_ICMP6 && UIP_ICMP_BUF->type < 128)
        {
            uip_len= 0;
            return;
        }
    }
    else
    {
        if(UIP_IP_BUF->proto == UIP_PROTO_ICMP6 && UIP_ICMP_BUF->type < 128)
        {
            uip_len = 0;
            return;
        }
    }

    uip_ext_len = UIP_HBHO_HDR_EXTENSION(UIP_HBHO_HDR_INVERT, NULL);
    /* remember data of original packet before shifting */
    uip_ipaddr_copy(&tmp_ipaddr, &UIP_IP_BUF->destipaddr);

    uip_len += BYTE_UNITS(UIP_IPICMPH_LEN + UIP_ICMP6_ERROR_LEN);

    if(uip_len > UIP_LINK_MTU)
    uip_len = UIP_LINK_MTU;

    memmove((char *)UIP_ICMP6_ERROR_BUF + uip_ext_len + UIP_ICMP6_ERROR_LEN,
            (void *)UIP_IP_BUF, CHAR_BIT_UNITS(uip_len) - UIP_IPICMPH_LEN - uip_ext_len - UIP_ICMP6_ERROR_LEN);

    UIP_IP_BUF->vtc = 0x60;
    UIP_IP_BUF->tcflow = 0;
    UIP_IP_BUF->flow = 0;
    if (uip_ext_len)
    {
        UIP_FIRST_EXT_BUF->next = UIP_PROTO_ICMP6;
    }
    else
    {
        UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
    }
    UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;

    /* the source should not be unspecified nor multicast, the check for
     multicast is done in uip_process */
    if(uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr))
    {
        uip_len = 0;
        return;
    }

    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, destination);

    if(uip_is_addr_mcast(&tmp_ipaddr))
    {
        if(type == ICMP6_PARAM_PROB && code == ICMP6_PARAMPROB_OPTION)
        {
            uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &tmp_ipaddr);
        }
        else
        {
            uip_len = 0;
            return;
        }
    }
    else
    {
#if UIP_CONF_ROUTER
        /* need to pick a source that corresponds to this node */
        uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &tmp_ipaddr);
#else
        uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &tmp_ipaddr);
#endif
    }

    UIP_ICMP_BUF->type = type;
    UIP_ICMP_BUF->icode = code;
    UIP_ICMP6_ERROR_BUF->param1 = uip_htonl(param) >> 16;
    UIP_ICMP6_ERROR_BUF->param2 = uip_htonl(param) & 0xffff;
    UIP_IP_BUF->len0 = ((uip_len - BYTE_UNITS(UIP_IPH_LEN)) >> 8);
    UIP_IP_BUF->len1 = ((uip_len - BYTE_UNITS(UIP_IPH_LEN)) & 0xff);
    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

    UIP_STAT(++uip_stat.icmp.sent);
    UIP_STAT_TX_ICMP_NUM_INC;

    PRINTF("Sending ICMPv6 ERROR message to");
    PRINT6ADDR(&UIP_IP_BUF->destipaddr);
    PRINTF("from");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("");
    return;
}

/*---------------------------------------------------------------------------*/
void uip_icmp6_send(uip_ipaddr_t *dest, int type, int code, int payload_len) //payload_len is the length in BYTE units
{

    UIP_IP_BUF->vtc = 0x60;
    UIP_IP_BUF->tcflow = 0;
    UIP_IP_BUF->flow = 0;
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
    UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;
    UIP_IP_BUF->len0 = (BYTE_UNITS(UIP_ICMPH_LEN) + payload_len) >> 8;
    UIP_IP_BUF->len1 = (BYTE_UNITS(UIP_ICMPH_LEN) + payload_len) & 0xff;

    memcpy(&UIP_IP_BUF->destipaddr, dest, sizeof(*dest));
    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);

    UIP_ICMP_BUF->type = type;
    UIP_ICMP_BUF->icode = code;

    UIP_ICMP_BUF->icmpchksum = 0;
    UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

    uip_len= BYTE_UNITS(UIP_IPH_LEN + UIP_ICMPH_LEN) + payload_len;

    UIP_STAT_TX_ICMP_NUM_INC;

    tcpip_ipv6_output();
}

/** @} */
#endif /* UIP_CONF_IPV6 */
