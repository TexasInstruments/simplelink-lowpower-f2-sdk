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
 *  ====================== config-uip-private.h =============================================
 *  Private configuration for uIP.
 */

#ifndef CONFIG_UIP_H
#define CONFIG_UIP_H

#include "config-uip-public.h"

#ifdef LINUX_GATEWAY
#include "mac-dep.h"
#endif

/*----------------------------------------------------------------------------*/
//6LoWPAN configuration
#ifdef MAX_PACKET_TEST
#define QUEUEBUF_CONF_NUM                       2
#define SICSLOWPAN_CONF_REASS_BUF_SIZE          2
#else
#define QUEUEBUF_CONF_NUM                       6   //FIXME used for 6LoWPAN fragmentation process

#if IS_ROOT
#define SICSLOWPAN_CONF_REASS_BUF_SIZE          12
#elif IS_INTERMEDIATE
#define SICSLOWPAN_CONF_REASS_BUF_SIZE          12   //FIXME: re-assembly buffer
#else
#define SICSLOWPAN_CONF_REASS_BUF_SIZE          12
#endif
#endif

/* Specify a minimum packet size for 6lowpan compression to be
 enabled. This is needed for ContikiMAC, which needs packets to be
 larger than a specified size, if no ContikiMAC header should be
 used. */
#define SICSLOWPAN_CONF_COMPRESSION_THRESHOLD   63

#define SICSLOWPAN_CONF_COMPRESSION_IPV6        0
#define SICSLOWPAN_CONF_COMPRESSION_HC1         1
#define SICSLOWPAN_CONF_COMPRESSION_HC01        2
#define SICSLOWPAN_CONF_COMPRESSION             SICSLOWPAN_COMPRESSION_HC06

#if SICSLOWPAN_CONF_FRAG

#define SICSLOWPAN_CONF_MAXAGE                  30
#endif /* SICSLOWPAN_CONF_FRAG */

#define SICSLOWPAN_CONF_CONVENTIONAL_MAC	    1
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS       2

#define SICSLOWPAN_TX_REQUEST_FUNC              sicslowpan_tx_request

/*----------------------------------------------------------------------------*/

#ifdef TI_MAC_USE_MAC64
#define RIMEADDR_CONF_SIZE      8
#else
#define RIMEADDR_CONF_SIZE      2
#endif

#if BORDER_ROUTER
/* Packet forward interface when no route is found. Root node only*/
#ifdef LINUX_GATEWAY
#define UIP_FALLBACK_INTERFACE tun_interface
#else
#define UIP_FALLBACK_INTERFACE slip_interface
#endif
#endif

#define UIP_CONF_IPV6_QUEUE_PKT         0
#define UIP_CONF_IPV6_CHECKS            1
#define UIP_CONF_DS6_LL_NUD             1 //Use Mac layer ACKs for NUD
#define UIP_CONF_ICMP_DEST_UNREACH      1

#define UIP_CONF_DS6_PREFIX_NBU         1 //Number of unicast prefixes
#define UIP_CONF_DS6_ADDR_NBU           1 //Number of unicast addresses
#define UIP_CONF_DS6_DEFRT_NBU          1 //Number of default routers

#ifndef  UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW  200
#endif
#ifndef  UIP_CONF_TCP_MSS
#define UIP_CONF_TCP_MSS         200
#endif
#define UIP_CONF_MAX_CONNECTIONS 3
#define UIP_CONF_MAX_LISTENPORTS 4
#define UIP_CONF_UDP_CONNS       3

#define UIP_CONF_TCP_SPLIT       0

#define TCP_CONF_PERIODIC_TIMER         (4*500)   //number of milliseconds before retransmitting a NS for probing a neighbor

//Save code space! If we use TCP, we do not compile UDP code
#if UIP_CONF_TCP
#define UIP_CONF_UDP    0
#endif

#define CONF_WITH_PLTFRM_TIMERS         1 //Always use platform timers (sysbios clocks)

#define UIP_DS6_NOTIFICATION_FUNC       uip_ds6_usch_notification   //This one will call rpl_route_callback

#endif /* CONTIKI_CONF_H */
