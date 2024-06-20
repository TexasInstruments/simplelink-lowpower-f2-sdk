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
 * include this software, other than combinations with devices manufactured by or for TI (â€œTI Devicesâ€�).
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
 * THIS SOFTWARE IS PROVIDED BY TI AND TIâ€™S LICENSORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TIâ€™S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ====================== config-uip-public.h =============================================
 *  Public configuration for uIP.
 */

#ifndef __CONFIG_UIP_PUBLIC__
#define __CONFIG_UIP_PUBLIC__

/**************************************************************************************************
 * IP Configuration
 */

#define UDP_CLIENT_PORT 8775
#define UDP_SERVER_PORT 5688
#define UDP_RIOT_PORT   5600

#if FEATURE_DTLS
#define UDP_DTLS_UDP_PORT      (20220)
#define UDP_MAC_SEC_UDP_PORT   (20224)
#endif

/*----------------------------------------------------------------------------*/
//6LoWPAN Configuration
#define CONF_WITH_SICSLOWPAN            1 //Always use 6LoWPAN HC and/or fragmentation

#define SICSLOWPAN_CONF_FRAG            1 //Enable fragmentation too

#define SICSLOWPAN_CONF_ADDR_CONTEXT_0          prefix_setup(addr_contexts[0].prefix) //Initialize addr_contexts[0].prefix

/*----------------------------------------------------------------------------*/
//uIPv6 Configuration
#if IS_ROOT

//Each route uses 58 bytes (Non Storing). The routing table structures use 72 bytes
//Each route uses 46 bytes (Storing). The routing table structures use 80 bytes
#define UIP_CONF_DS6_ROUTE_NBU			200
//Each neighbor uses around 80 bytes (8 of which, are for the relationship with the routing table, thus 72 bytes is Non-Storing Mode). The neighbor table structures use around 100 bytes
#if FEATURE_DTLS
#define UIP_CONF_DS6_NBR_NBU			100     //FIXME
#else
#define UIP_CONF_DS6_NBR_NBU            100
#endif

#elif IS_INTERMEDIATE

//Each route uses 46 bytes. The routing table structures use 80 bytes
#define UIP_CONF_DS6_ROUTE_NBU			100       //FIXME
//Each neighbor uses around 80 bytes (8 of which, are for the relationship with the routing table, thus 72 bytes is Non-Storing Mode). The neighbor table structures use around 100 bytes
#define UIP_CONF_DS6_NBR_NBU			100       //FIXME

#else //LEAF

#define UIP_CONF_DS6_ROUTE_NBU			2
#define UIP_CONF_DS6_NBR_NBU			5

#endif

#define UIP_CONF_ND6_REACHABLE_TIME     300000  //should be consistant with rpl timeout   //number of milliseconds that we consider a neighbor reachable

#define UIP_CONF_ND6_RETRANS_TIMER      10000   //number of milliseconds before retransmitting a NS for probing a neighbor

#ifdef MAX_PACKET_TEST
#define UIP_CONF_BUFFER_SIZE            2000
#else
#define UIP_CONF_BUFFER_SIZE		    300    //FIXME: Number of bytes for the uIP buffer (> total header+payload)
#endif

//Statistics (define the ones to use)
#define UIP_STAT_TX_IP_NUM_INC                  TCPIP_Dbg.tcpip_output_pkt++

#define UIP_STAT_SLP_TX_INC                     LOWPAN_Dbg.number_tx_pkt++
#define UIP_STAT_SLP_TX_DROP_WRONG_LEN_INC      LOWPAN_Dbg.tx_drop_len_wrong++
#define UIP_STAT_SLP_REASS_TIMEOUT_INC          LOWPAN_Dbg.reass_timeout++
#define UIP_STAT_SLP_FULL_DROP_SEGMENT_INC      LOWPAN_Dbg.buf_full_drop_segment++
#define UIP_STAT_SLP_FRAG_1_INC                 LOWPAN_Dbg.frag_1++
#define UIP_STAT_SLP_FRAG_N_INC                 LOWPAN_Dbg.frag_n++
#define UIP_STAT_SLP_FRAG_N_HDR_ERROR_INC       LOWPAN_Dbg.frag_n_header_error++
#define UIP_STAT_SLP_UNKNOWN_HDR_ERROR_INC      LOWPAN_Dbg.unknown_header_error++
#define UIP_STAT_SLP_PACKET_LEN_SMALL_INC       LOWPAN_Dbg.packet_len_small_error++
#define UIP_STAT_SLP_PACKET_LEN_BIG_INC         LOWPAN_Dbg.packet_len_big_error++
#define UIP_STAT_SLP_RX_INC                     LOWPAN_Dbg.total_rx_packet++

#endif //CONFIG_UIP_PUBLIC
