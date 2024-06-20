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
 *  ====================== uip-conf.h =============================================
 *  Public configuration for uIP.
 */

#ifndef __UIP_CONF_H__
#define __UIP_CONF_H__

#include "plat-conf.h"
#include "pltfrm_lib.h"

//If config-uip-private.h file does not configure the following features, we configure them here
#include "config-uip-private.h"

#ifndef RIMEADDR_CONF_SIZE
#define RIMEADDR_CONF_SIZE      2 //This is the default link layer (rime) address.
#endif

#ifndef CONF_WITH_SICSLOWPAN
#define	WITH_SICSLOWPAN	        1
#else
#define WITH_SICSLOWPAN	        CONF_WITH_SICSLOWPAN
#endif

#ifndef CONF_WITH_PLTFRM_TIMERS
#define WITH_PLTFRM_TIMERS	1
#else
#define WITH_PLTFRM_TIMERS	CONF_WITH_PLTFRM_TIMERS
#endif

#ifndef LINUX_GATEWAY
#include "6lowpan_mac.h"
#endif

/*------------------------------------------------------------*/
//CONSTANTS that we need to define for our Stack
#if (!defined(CLOCK_CONF_SECOND) && !defined(CLOCK_SECOND))
#error "This platform dependent constant needs to be defined"
#endif

#ifndef CCIF
#define CCIF
#endif

#ifndef CLIF
#define CLIF
#endif

#ifndef PROCESS_CURRENT
#define PROCESS_CURRENT() 	((void*)0)
#endif

#ifndef PROCESS_NONE
#define PROCESS_NONE		((void*)0)
#endif

#define ORIGINAL_CONTIKI        0

/*----------------------------------------------------------------------------*/
//The following features are only supported like this (at least for the moment): No Changes allowed
#define UIP_CONF_IPV6                   1       //Use IPv6
#define UIP_CONF_ICMP6                  0       //Use separate ICMPv6 process
#define UIP_CONF_SEND_RS                0       //Send Router Solicitation (not used when using RPL!)
#define UIP_CONF_ND6_SEND_RA            0       //Send Router Advertisement (not used when using RPL!)
#define UIP_CONF_ROUTER                 1       //Configure Node as a router (always)
#define UIP_CONF_IPV6_REASSEMBLY        0       //We do not support reassembly, at least for now
#define UIP_CONF_IP_FORWARD             0       //We do not support ip forwarding, at least for now
#define UIP_CONF_PINGADDRCONF           0       //This one depends on IP Forwarding
#define UIP_CONF_BROADCAST              0       //This one depends on IP Forwarding
#define UIP_CONF_FWCACHE_SIZE           0       //This one depends on IP Forwarding
#define UIP_ARCH_IPCHKSUM               0       //No separate checksum functions
#define UIP_CONF_LOGGING                0       //Logging disabled. Use debugging messages instead
#define UIP_CONF_LLH_LEN                0       //Do not need to specify a Link Layer Header length
#define UIP_CONF_LL_802154              1       //Use 802.15.4 Link Layer addresses
#define DAO_MAX_TARGET                  4       //check this
#ifdef LINUX_GATEWAY
#define UIP_CONF_STATISTICS             1       //Do not use Contiki Statistics
#else
#define UIP_CONF_STATISTICS             0       //Do not use Contiki Statistics
#endif

/*----------------------------------------------------------------------------*/
//Using sicslowpan or not defines the use of the queuebuf and packetbuf modules
//Sicslowpan module is not actually part of the uIPv6 library
#if WITH_SICSLOWPAN
#define WITH_QUEUEBUF	1
#define UIP_PACKETBUF   1
#else
#define WITH_QUEUEBUF	0
#define UIP_PACKETBUF   0
#endif

/*----------------------------------------------------------------------------*/
//The next features are CONFIGURABLE (by the developer - config-uip-private.h)
//If these parameters are not configured before, here we make sure that they are given a value
//Only 1 default router is possible
#ifndef UIP_CONF_DS6_SINGLE_DEFRT
#define UIP_CONF_DS6_SINGLE_DEFRT               0
#endif

//The default router validity is infinite
#ifndef UIP_CONF_DS6_INFINITE_DEFRT
#define UIP_CONF_DS6_INFINITE_DEFRT             0
#endif

//The unicast address validity is infinite
#ifndef UIP_CONF_DS6_INFINITE_ADDRESSES
#define UIP_CONF_DS6_INFINITE_ADDRESSES         0
#endif

//Use MAC Layer ACKs for NUD (node unreachability detection)
#ifndef UIP_CONF_DS6_LL_NUD
#define UIP_CONF_DS6_LL_NUD             1
#endif

//Perform checks such as checksums, ets
#ifndef UIP_CONF_IPV6_CHECKS
#define UIP_CONF_IPV6_CHECKS            1
#endif

//Queue packets if neighbor is in incomplete state
#ifndef UIP_CONF_IPV6_QUEUE_PKT
#define UIP_CONF_IPV6_QUEUE_PKT         0
#endif

//Respond with destination unreachable icmp6 message
#ifndef UIP_CONF_ICMP_DEST_UNREACH
#define UIP_CONF_ICMP_DEST_UNREACH      1
#endif

//Configuration of UDP parameters
#ifndef UIP_CONF_UDP
#define UIP_CONF_UDP                    1       //Allow UDP
#else
#if UIP_CONF_UDP == 0
#define UIP_CONF_UDP_CHECKSUMS        0
#endif
#endif

#ifndef UIP_CONF_UDP_CHECKSUMS
#define UIP_CONF_UDP_CHECKSUMS          1       //Perform UDP checksum
#endif

#ifndef UIP_CONF_UDP_CONNS
#define UIP_CONF_UDP_CONNS              2       //Max number of UDP connections
#endif

//Configuration of TCP parameters
#ifndef  UIP_CONF_RECEIVE_WINDOW
#define  UIP_CONF_RECEIVE_WINDOW        200     //TCP receiving window size
#endif

#ifndef  UIP_CONF_TCP_MSS
#define UIP_CONF_TCP_MSS                200     //TCP max segment size
#endif

#ifndef UIP_CONF_TCP_SPLIT
#define UIP_CONF_TCP_SPLIT              0       //Split messages for TCP
#endif

#ifndef UIP_CONF_MAX_CONNECTIONS
#define UIP_CONF_MAX_CONNECTIONS        2       //Max number of TCP connections
#endif

#ifndef UIP_CONF_MAX_LISTENPORTS
#define UIP_CONF_MAX_LISTENPORTS        4       //Max number of TCP listening ports
#endif

#ifndef TCP_CONF_PERIODIC_TIMER
#define TCP_CONF_PERIODIC_TIMER         (4*500)   //number of milliseconds before retransmitting a NS for probing a neighbor
#endif

#ifndef UIP_CONF_DS6_DEFRT_NBU
#define UIP_CONF_DS6_DEFRT_NBU          1 //Number of default routers (this value is dismissed if UIP_CONF_DS6_SINGLE_DEFRT is 1)
#endif

#ifndef UIP_CONF_DS6_PREFIX_NBU
#define UIP_CONF_DS6_PREFIX_NBU         1 //Number of unicast prefixes
#endif

#ifndef UIP_CONF_DS6_ADDR_NBU
#define UIP_CONF_DS6_ADDR_NBU           1 //Number of unicast addresses
#endif

#ifndef UIP_ROUTE_NEIGHBOR_NUM
#define UIP_ROUTE_NEIGHBOR_NUM          NBR_TABLE_CONF_MAX_NEIGHBORS
#endif

/*----------------------------------------------------------------------------*/
#ifndef UIP_CONF_ND6_REACHABLE_TIME
#define UIP_CONF_ND6_REACHABLE_TIME     60000   //number of milliseconds that we consider a neighbor reachable
#endif

#ifndef UIP_CONF_ND6_RETRANS_TIMER
#define UIP_CONF_ND6_RETRANS_TIMER      10000   //number of milliseconds before retransmitting a NS for probing a neighbor
#endif

//Configure the uIP buffer size (in bytes). Converted to Char bit units (number of bits per char)
#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE		    CHAR_BIT_UNITS(300) //FIXME
#endif

//Configure max number of NEIGHBORS
#ifndef NBR_TABLE_CONF_MAX_NEIGHBORS
#ifdef  UIP_CONF_DS6_NBR_NBU
#define NBR_TABLE_CONF_MAX_NEIGHBORS    UIP_CONF_DS6_NBR_NBU
#else
#define NBR_TABLE_CONF_MAX_NEIGHBORS     5
#endif
#endif /* NBR_TABLE_CONF_MAX_NEIGHBORS */

//Configure the max ROUTING TABLE entries
#ifndef UIP_CONF_MAX_ROUTES
#ifdef  UIP_CONF_DS6_ROUTE_NBU
#define UIP_CONF_MAX_ROUTES     UIP_CONF_DS6_ROUTE_NBU
#else
#define UIP_CONF_MAX_ROUTES             3
#endif
#endif /* UIP_CONF_MAX_ROUTES */

/*----------------------------------------------------------------------------*/
//Constants for uIPv6
//Hop By Hop Option header actions
#define UIP_HBHO_HDR_UPDATE_EMPTY        0
#define UIP_HBHO_HDR_UPDATE_NEXT_HOP     1
#define UIP_HBHO_HDR_VERIFY              2
#define UIP_HBHO_HDR_INSERT              3
#define UIP_HBHO_HDR_INVERT              4

//Source Routing header actions
#define UIP_SOURCE_ROUTING_HDR_INSERT    0
#define UIP_SOURCE_ROUTING_HDR_PROCESS   1

//Routing mode:
#define UIP_DS6_STANDARD_ROUTING        0
#define UIP_DS6_SOURCE_ROUTING          1
#define UIP_DS6_NO_ROUTING              2

//POSSIBLE RESULTS AFTER PROCESSING SOURCE ROUTING HEADER
#define UIP_SOURCE_ROUTING_HDR_DISCARD 	    0
#define UIP_SOURCE_ROUTING_HDR_ERROR_MSG 	1
#define UIP_SOURCE_ROUTING_HDR_FWD_MSG		2

/*----------------------------------------------------------------------------*/
//uIPv6 CONFIGURATION WHEN USING RPL (it is defined at the end of rpl-conf.h)
#ifndef RPL_DISABLE
#include "rpl-conf.h"
#endif

/*----------------------------------------------------------------------------*/
//uIPv6 Default Extension Configuration (if none of the following were configured separately, these are the default values)
//If no configuration was defined previously for the following, use standard settings
#ifdef UIP_DS6_ROUTING_MODE_CONF
#define UIP_DS6_ROUTING_MODE    UIP_DS6_ROUTING_MODE_CONF
#else
#define UIP_DS6_ROUTING_MODE    UIP_DS6_STANDARD_ROUTING                //Use standard routing tables
#endif

/*
 #ifndef UIP_ECHO_REPLY_CALLBACK
 #define UIP_ECHO_REPLY_CALLBACK(buf,len)        //No callback function for echo replied received
 #endif

 #ifndef UIP_TX_STATUS_CALLBACK
 #define UIP_TX_STATUS_CALLBACK(status)         //No transmission status callback function
 #endif
 */
/*----------------------------------------------------------------------------*/
//uIPv6 Statistics: Chose the ones that are interesting and define the incrementation of a suitable variable
//By default, no statistics are used
#ifdef TCPIP_FULL_DEBUG

#ifndef UIP_STAT_ROUTE_ADDED_NUM_INC
#define UIP_STAT_ROUTE_ADDED_NUM_INC                        TCPIP_Dbg.tcpip_route_added++
#endif

#ifndef UIP_STAT_ROUTE_NOT_ADDED_NUM_INC
#define UIP_STAT_ROUTE_NOT_ADDED_NUM_INC                    TCPIP_Dbg.tcpip_route_not_added++
#endif

#ifndef UIP_STAT_ROUTE_DELETED_NUM_INC
#define UIP_STAT_ROUTE_DELETED_NUM_INC                      TCPIP_Dbg.tcpip_route_deleted++
#endif

#ifndef UIP_STAT_NEIGHBOR_ADDED_NUM_INC
#define UIP_STAT_NEIGHBOR_ADDED_NUM_INC                     TCPIP_Dbg.tcpip_neighbor_added++
#endif

#ifndef UIP_STAT_NEIGHBOR_NOT_ADDED_NUM_INC
#define UIP_STAT_NEIGHBOR_NOT_ADDED_NUM_INC                 TCPIP_Dbg.tcpip_neighbor_not_added++
#endif

#ifndef UIP_STAT_NEIGHBOR_SECONDARY_ADDED_NUM_INC
#define UIP_STAT_NEIGHBOR_SECONDARY_ADDED_NUM_INC           TCPIP_Dbg.tcpip_neighbor_secondary_added++
#endif

#ifndef UIP_STAT_NEIGHBOR_SECONDARY_NOT_ADDED_NUM_INC
#define UIP_STAT_NEIGHBOR_SECONDARY_NOT_ADDED_NUM_INC       TCPIP_Dbg.tcpip_neighbor_secondary_not_added++
#endif

#ifndef UIP_STAT_NEIGHBOR_DELETED_NUM_INC
#define UIP_STAT_NEIGHBOR_DELETED_NUM_INC                   TCPIP_Dbg.tcpip_neighbor_deleted++
#endif

#ifndef UIP_STAT_RX_IP_NUM_INC
#define UIP_STAT_RX_IP_NUM_INC                              TCPIP_Dbg.tcpip_rx_ip++
#endif

#ifndef UIP_STAT_RX_IP_ROUTED_NUM_INC
#define UIP_STAT_RX_IP_ROUTED_NUM_INC                       TCPIP_Dbg.tcpip_rx_ip_routed++
#endif

#ifndef UIP_STAT_RX_IP_DROP_NUM_INC
#define UIP_STAT_RX_IP_DROP_NUM_INC                         TCPIP_Dbg.tcpip_rx_ip_drop++
#endif

#ifndef UIP_STAT_RX_UDP_NUM_INC
#define UIP_STAT_RX_UDP_NUM_INC                             TCPIP_Dbg.tcpip_rx_udp++
#endif

#ifndef UIP_STAT_RX_UDP_DROP_NUM_INC
#define UIP_STAT_RX_UDP_DROP_NUM_INC                        TCPIP_Dbg.tcpip_rx_udp_drop++
#endif

#ifndef UIP_STAT_RX_UDP_CHKERR_NUM_INC
#define UIP_STAT_RX_UDP_CHKERR_NUM_INC                      TCPIP_Dbg.tcpip_rx_udp_chkerr++
#endif

#ifndef UIP_STAT_RX_ICMP_NUM_INC
#define UIP_STAT_RX_ICMP_NUM_INC                            TCPIP_Dbg.tcpip_rx_icmp++
#endif

#ifndef UIP_STAT_RX_ICMP_DROP_NUM_INC
#define UIP_STAT_RX_ICMP_DROP_NUM_INC                       TCPIP_Dbg.tcpip_rx_icmp_drop++
#endif

#ifndef UIP_STAT_RX_ICMP_CHKERR_NUM_INC
#define UIP_STAT_RX_ICMP_CHKERR_NUM_INC                     TCPIP_Dbg.tcpip_rx_icmp_chkerr++
#endif

#ifndef UIP_STAT_RX_NS_NUM_INC
#define UIP_STAT_RX_NS_NUM_INC                              TCPIP_Dbg.tcpip_rx_ns++
#endif

#ifndef UIP_STAT_RX_NA_NUM_INC
#define UIP_STAT_RX_NA_NUM_INC                              TCPIP_Dbg.tcpip_rx_na++
#endif

#ifndef UIP_STAT_RX_ECHO_REQ_NUM_INC
#define UIP_STAT_RX_ECHO_REQ_NUM_INC
#endif

#ifndef UIP_STAT_RX_ECHO_REP_NUM_INC
#define UIP_STAT_RX_ECHO_REP_NUM_INC
#endif

#ifndef UIP_STAT_TX_IP_NUM_INC
#define UIP_STAT_TX_IP_NUM_INC                              //TCPIP_Dbg.tcpip_tx_ip++
#endif

#ifndef UIP_STAT_TX_UDP_NUM_INC
#define UIP_STAT_TX_UDP_NUM_INC                             TCPIP_Dbg.tcpip_tx_udp++
#endif

#ifndef UIP_STAT_TX_ICMP_NUM_INC
#define UIP_STAT_TX_ICMP_NUM_INC                            TCPIP_Dbg.tcpip_tx_icmp++
#endif

#ifndef UIP_STAT_TX_ECHO_REQ_NUM_INC
#define UIP_STAT_TX_ECHO_REQ_NUM_INC
#endif

#ifndef UIP_STAT_TX_ECHO_REP_NUM_INC
#define UIP_STAT_TX_ECHO_REP_NUM_INC
#endif

#ifndef UIP_STAT_TX_NS_NUM_INC
#define UIP_STAT_TX_NS_NUM_INC                              TCPIP_Dbg.tcpip_tx_ns++
#endif

#ifndef UIP_STAT_TX_NA_NUM_INC
#define UIP_STAT_TX_NA_NUM_INC                              TCPIP_Dbg.tcpip_tx_na++
#endif

#ifndef UIP_STAT_TX_ROUTE_NONE_NUM_INC
#define UIP_STAT_TX_ROUTE_NONE_NUM_INC                      TCPIP_Dbg.tcpip_tx_route_none++
#endif

#ifndef UIP_STAT_TX_ROUTE_ONLINK_NUM_INC
#define UIP_STAT_TX_ROUTE_ONLINK_NUM_INC                    TCPIP_Dbg.tcpip_tx_route_onlink++
#endif

#ifndef UIP_STAT_TX_ROUTE_DEFAULT_NUM_INC
#define UIP_STAT_TX_ROUTE_DEFAULT_NUM_INC                   TCPIP_Dbg.tcpip_tx_default_route++
#endif

#ifndef UIP_STAT_TX_ROUTE_FOUND_NUM_INC
#define UIP_STAT_TX_ROUTE_FOUND_NUM_INC                     TCPIP_Dbg.tcpip_tx_route_found++
#endif

#ifndef UIP_STAT_TX_PATH_FOUND_NUM_INC
#define UIP_STAT_TX_PATH_FOUND_NUM_INC                      TCPIP_Dbg.tcpip_tx_path_found++
#endif

#ifndef UIP_STAT_TX_PATH_INVALID_NUM_INC
#define UIP_STAT_TX_PATH_INVALID_NUM_INC                    TCPIP_Dbg.tcpip_tx_path_invalid++
#endif

#ifndef UIP_STAT_TX_NBR_NONE_FOR_ROUTE_NUM_INC
#define UIP_STAT_TX_NBR_NONE_FOR_ROUTE_NUM_INC              TCPIP_Dbg.tcpip_tx_nbr_none_for_route++
#endif

#ifndef UIP_STAT_TX_NBR_UNKNOWN_NUM_INC
#define UIP_STAT_TX_NBR_UNKNOWN_NUM_INC                     TCPIP_Dbg.tcpip_tx_nbr_unknown++
#endif

#ifndef UIP_STAT_TX_NBR_STALE_NUM_INC
#define UIP_STAT_TX_NBR_STALE_NUM_INC                       TCPIP_Dbg.tcpip_tx_nbr_stale++
#endif

#ifndef UIP_STAT_TX_NBR_INCOMPLETE_NUM_INC
#define UIP_STAT_TX_NBR_INCOMPLETE_NUM_INC                  TCPIP_Dbg.tcpip_tx_nbr_incomplete++
#endif

#ifndef UIP_STAT_TX_MCAST_NUM_INC
#define UIP_STAT_TX_MCAST_NUM_INC                           TCPIP_Dbg.tcpip_tx_mcast++
#endif

#ifndef UIP_STAT_TX_IP_DROP_TOO_BIG_INC
#define UIP_STAT_TX_IP_DROP_TOO_BIG_INC                     TCPIP_Dbg.tcpip_tx_ip_drop_too_big++
#endif

#ifndef UIP_STAT_TX_IP_DROP_DEST_UNSPEC_INC
#define UIP_STAT_TX_IP_DROP_DEST_UNSPEC_INC
#endif

#ifndef UIP_STAT_TX_IP_DROP_DEST_MY_ADDR_INC
#define UIP_STAT_TX_IP_DROP_DEST_MY_ADDR_INC
#endif

#ifndef UIP_STAT_TX_CANNOT_INSERT_SRH_INC
#define UIP_STAT_TX_CANNOT_INSERT_SRH_INC
#endif

#else

#ifndef UIP_STAT_ROUTE_ADDED_NUM_INC
#define UIP_STAT_ROUTE_ADDED_NUM_INC                        TCPIP_Dbg.tcpip_route_added++
#endif

#ifndef UIP_STAT_ROUTE_NOT_ADDED_NUM_INC
#define UIP_STAT_ROUTE_NOT_ADDED_NUM_INC                    TCPIP_Dbg.tcpip_route_not_added++
#endif

#ifndef UIP_STAT_ROUTE_DELETED_NUM_INC
#define UIP_STAT_ROUTE_DELETED_NUM_INC                      TCPIP_Dbg.tcpip_route_deleted++
#endif

#ifndef UIP_STAT_NEIGHBOR_ADDED_NUM_INC
#define UIP_STAT_NEIGHBOR_ADDED_NUM_INC                     TCPIP_Dbg.tcpip_neighbor_added++
#endif

#ifndef UIP_STAT_NEIGHBOR_NOT_ADDED_NUM_INC
#define UIP_STAT_NEIGHBOR_NOT_ADDED_NUM_INC                 TCPIP_Dbg.tcpip_neighbor_not_added++
#endif

#ifndef UIP_STAT_NEIGHBOR_SECONDARY_ADDED_NUM_INC
#define UIP_STAT_NEIGHBOR_SECONDARY_ADDED_NUM_INC           //TCPIP_Dbg.tcpip_neighbor_secondary_added++
#endif

#ifndef UIP_STAT_NEIGHBOR_SECONDARY_NOT_ADDED_NUM_INC
#define UIP_STAT_NEIGHBOR_SECONDARY_NOT_ADDED_NUM_INC       //TCPIP_Dbg.tcpip_neighbor_secondary_not_added++
#endif

#ifndef UIP_STAT_NEIGHBOR_DELETED_NUM_INC
#define UIP_STAT_NEIGHBOR_DELETED_NUM_INC                   //TCPIP_Dbg.tcpip_neighbor_deleted++
#endif

#ifndef UIP_STAT_RX_IP_NUM_INC
#define UIP_STAT_RX_IP_NUM_INC                              //TCPIP_Dbg.tcpip_rx_ip++
#endif

#ifndef UIP_STAT_RX_IP_ROUTED_NUM_INC
#define UIP_STAT_RX_IP_ROUTED_NUM_INC                       //TCPIP_Dbg.tcpip_rx_ip_routed++
#endif

#ifndef UIP_STAT_RX_IP_DROP_NUM_INC
#define UIP_STAT_RX_IP_DROP_NUM_INC                         //TCPIP_Dbg.tcpip_rx_ip_drop++
#endif

#ifndef UIP_STAT_RX_UDP_NUM_INC
#define UIP_STAT_RX_UDP_NUM_INC                             //TCPIP_Dbg.tcpip_rx_udp++
#endif

#ifndef UIP_STAT_RX_UDP_DROP_NUM_INC
#define UIP_STAT_RX_UDP_DROP_NUM_INC                        //TCPIP_Dbg.tcpip_rx_udp_drop++
#endif

#ifndef UIP_STAT_RX_UDP_CHKERR_NUM_INC
#define UIP_STAT_RX_UDP_CHKERR_NUM_INC                      //TCPIP_Dbg.tcpip_rx_udp_chkerr++
#endif

#ifndef UIP_STAT_RX_ICMP_NUM_INC
#define UIP_STAT_RX_ICMP_NUM_INC                            //TCPIP_Dbg.tcpip_rx_icmp++
#endif

#ifndef UIP_STAT_RX_ICMP_DROP_NUM_INC
#define UIP_STAT_RX_ICMP_DROP_NUM_INC                       //TCPIP_Dbg.tcpip_rx_icmp_drop++
#endif

#ifndef UIP_STAT_RX_ICMP_CHKERR_NUM_INC
#define UIP_STAT_RX_ICMP_CHKERR_NUM_INC                     //TCPIP_Dbg.tcpip_rx_icmp_chkerr++
#endif

#ifndef UIP_STAT_RX_NS_NUM_INC
#define UIP_STAT_RX_NS_NUM_INC                              //TCPIP_Dbg.tcpip_rx_ns++
#endif

#ifndef UIP_STAT_RX_NA_NUM_INC
#define UIP_STAT_RX_NA_NUM_INC                              //TCPIP_Dbg.tcpip_rx_na++
#endif

#ifndef UIP_STAT_RX_ECHO_REQ_NUM_INC
#define UIP_STAT_RX_ECHO_REQ_NUM_INC
#endif

#ifndef UIP_STAT_RX_ECHO_REP_NUM_INC
#define UIP_STAT_RX_ECHO_REP_NUM_INC
#endif

#ifndef UIP_STAT_TX_IP_NUM_INC
#define UIP_STAT_TX_IP_NUM_INC                              //TCPIP_Dbg.tcpip_tx_ip++
#endif

#ifndef UIP_STAT_TX_UDP_NUM_INC
#define UIP_STAT_TX_UDP_NUM_INC                             //TCPIP_Dbg.tcpip_tx_udp++
#endif

#ifndef UIP_STAT_TX_ICMP_NUM_INC
#define UIP_STAT_TX_ICMP_NUM_INC                            //TCPIP_Dbg.tcpip_tx_icmp++
#endif

#ifndef UIP_STAT_TX_ECHO_REQ_NUM_INC
#define UIP_STAT_TX_ECHO_REQ_NUM_INC
#endif

#ifndef UIP_STAT_TX_ECHO_REP_NUM_INC
#define UIP_STAT_TX_ECHO_REP_NUM_INC
#endif

#ifndef UIP_STAT_TX_NS_NUM_INC
#define UIP_STAT_TX_NS_NUM_INC                              //TCPIP_Dbg.tcpip_tx_ns++
#endif

#ifndef UIP_STAT_TX_NA_NUM_INC
#define UIP_STAT_TX_NA_NUM_INC                              //TCPIP_Dbg.tcpip_tx_na++
#endif

#ifndef UIP_STAT_TX_ROUTE_NONE_NUM_INC
#define UIP_STAT_TX_ROUTE_NONE_NUM_INC                      //TCPIP_Dbg.tcpip_tx_route_none++
#endif

#ifndef UIP_STAT_TX_ROUTE_ONLINK_NUM_INC
#define UIP_STAT_TX_ROUTE_ONLINK_NUM_INC                    //TCPIP_Dbg.tcpip_tx_route_onlink++
#endif

#ifndef UIP_STAT_TX_ROUTE_DEFAULT_NUM_INC
#define UIP_STAT_TX_ROUTE_DEFAULT_NUM_INC                   //TCPIP_Dbg.tcpip_tx_default_route++
#endif

#ifndef UIP_STAT_TX_ROUTE_FOUND_NUM_INC
#define UIP_STAT_TX_ROUTE_FOUND_NUM_INC                     //TCPIP_Dbg.tcpip_tx_route_found++
#endif

#ifndef UIP_STAT_TX_PATH_FOUND_NUM_INC
#define UIP_STAT_TX_PATH_FOUND_NUM_INC                      //TCPIP_Dbg.tcpip_tx_path_found++
#endif

#ifndef UIP_STAT_TX_PATH_INVALID_NUM_INC
#define UIP_STAT_TX_PATH_INVALID_NUM_INC                    //TCPIP_Dbg.tcpip_tx_path_invalid++
#endif

#ifndef UIP_STAT_TX_NBR_NONE_FOR_ROUTE_NUM_INC
#define UIP_STAT_TX_NBR_NONE_FOR_ROUTE_NUM_INC              //TCPIP_Dbg.tcpip_tx_nbr_none_for_route++
#endif

#ifndef UIP_STAT_TX_NBR_UNKNOWN_NUM_INC
#define UIP_STAT_TX_NBR_UNKNOWN_NUM_INC                    // TCPIP_Dbg.tcpip_tx_nbr_unknown++
#endif

#ifndef UIP_STAT_TX_NBR_STALE_NUM_INC
#define UIP_STAT_TX_NBR_STALE_NUM_INC                       //TCPIP_Dbg.tcpip_tx_nbr_stale++
#endif

#ifndef UIP_STAT_TX_NBR_INCOMPLETE_NUM_INC
#define UIP_STAT_TX_NBR_INCOMPLETE_NUM_INC                  //TCPIP_Dbg.tcpip_tx_nbr_incomplete++
#endif

#ifndef UIP_STAT_TX_MCAST_NUM_INC
#define UIP_STAT_TX_MCAST_NUM_INC                           //TCPIP_Dbg.tcpip_tx_mcast++
#endif

#ifndef UIP_STAT_TX_IP_DROP_TOO_BIG_INC
#define UIP_STAT_TX_IP_DROP_TOO_BIG_INC                     //TCPIP_Dbg.tcpip_tx_ip_drop_too_big++
#endif

#ifndef UIP_STAT_TX_IP_DROP_DEST_UNSPEC_INC
#define UIP_STAT_TX_IP_DROP_DEST_UNSPEC_INC
#endif

#ifndef UIP_STAT_TX_IP_DROP_DEST_MY_ADDR_INC
#define UIP_STAT_TX_IP_DROP_DEST_MY_ADDR_INC
#endif

#ifndef UIP_STAT_TX_CANNOT_INSERT_SRH_INC
#define UIP_STAT_TX_CANNOT_INSERT_SRH_INC
#endif

#endif

#ifndef UIP_STAT_SLP_TX_INC
#define UIP_STAT_SLP_TX_INC
#endif

#ifndef UIP_STAT_SLP_TX_DROP_WRONG_LEN_INC
#define UIP_STAT_SLP_TX_DROP_WRONG_LEN_INC
#endif

#ifndef UIP_STAT_SLP_REASS_TIMEOUT_INC
#define UIP_STAT_SLP_REASS_TIMEOUT_INC
#endif

#ifndef UIP_STAT_SLP_FULL_DROP_SEGMENT_INC
#define UIP_STAT_SLP_FULL_DROP_SEGMENT_INC
#endif

#ifndef UIP_STAT_SLP_FRAG_1_INC
#define UIP_STAT_SLP_FRAG_1_INC
#endif

#ifndef UIP_STAT_SLP_FRAG_N_INC
#define UIP_STAT_SLP_FRAG_N_INC
#endif

#ifndef UIP_STAT_SLP_FRAG_N_HDR_ERROR_INC
#define UIP_STAT_SLP_FRAG_N_HDR_ERROR_INC
#endif

#ifndef UIP_STAT_SLP_UNKNOWN_HDR_ERROR_INC
#define UIP_STAT_SLP_UNKNOWN_HDR_ERROR_INC
#endif

#ifndef UIP_STAT_SLP_PACKET_LEN_SMALL_INC
#define UIP_STAT_SLP_PACKET_LEN_SMALL_INC
#endif

#ifndef UIP_STAT_SLP_PACKET_LEN_BIG_INC
#define UIP_STAT_SLP_PACKET_LEN_BIG_INC
#endif

#ifndef UIP_STAT_SLP_RX_INC
#define UIP_STAT_SLP_RX_INC
#endif

#endif /* __UIP_CONF_H__ */
