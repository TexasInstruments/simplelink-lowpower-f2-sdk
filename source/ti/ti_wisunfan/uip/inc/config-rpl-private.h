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
 *  ====================== config-rpl-private.h =============================================
 *  Private configuration for RPL.
 */

#ifndef CONFIG_RPL_H
#define CONFIG_RPL_H

#include "config-rpl-public.h"

#if IS_ROOT
#define UIP_RPL_ROOT    1
#else
#define UIP_RPL_ROOT    0
#endif

#define RPL_CONF_LEAF_ONLY      ((!IS_ROOT) && (!IS_INTERMEDIATE))

/*
 * Maximum of concurent RPL instances.
 */
#define RPL_CONF_MAX_INSTANCES          1         //One instance only

/*
 * Maximum number of DAGs within an instance.
 */
#define RPL_CONF_MAX_DAG_PER_INSTANCE    1          //One dag per instance

#define RPL_ACCEPT_DIO_FROM_FUNC     mac_has_ded_link //For TSCH we need to evaluate if we have a dedicated link with the target to accept DIOs from it

/*-------*/
#define RPL_PLUSPLUS_CONF    0       //Use some RPL++ enhancements

#define RPL_PLUSPLUS_MODIFY_DAO_INTERVAL_CONF        0 //Dynamic DAO interval (for very large networks)
#define RPL_PLUSPLUS_PROBE_IF_NO_ACK_CONF	     0 //1 //Probe neighbor if no ACK
#define RPL_PLUSPLUS_PERIODIC_PARENT_CHECK_CONF      0 //1 //Parent "keep alive"
#define RPL_PLUSPLUS_DAO_UPDATE_ROUTES_CONF          0 //Fast recovery
#define RPL_PLUSPLUS_NEXT_HOP_DEAD_CONF              0 //1 //Update route info

#define RPL_PLUSPLUS_DTSN_STRATEGY_CONF              0
/*-------*/

#define RPL_SUPPRESS_DIS_CONF           1 //Disables DIS code

#define RPL_SUPPRESS_DAO_ACK_CONF       1 //Disables DAO ACK code. RPL_CONF_DAO_ACK will be automatically 0 if this one is 1.

#if UIP_RPL_ROOT
#define RPL_CONF_DAO_ACK                0 //Root nodes do not need the code and variables for DAO ACK flag and timer activation. If we do not enable RPL_SUPPRESS_DAO_ACK_CONF, they will respond with DAO ACK anyway
#else
#define RPL_CONF_DAO_ACK                1 //Meaningless if RPL_SUPPRESS_DAO_ACK_CONF is 1. If not, being 1 means that the DAO ACK flag will be enabled for every DAO and we will expect a DAO ACK
#endif

/* The default value for the initial DAO ACK timeout. */
#define RPL_CONF_DAO_ACK_INITIAL_LATENCY        (CLOCK_SECOND * 8)

/* Maximum number of DAO Ack tries. */
#define RPL_CONF_DAO_ACK_MAX_TRIES                   3

#define RPL_DISABLE_HBHO_CONF           1 //Disables HBHO code

//This RPL implementation uses infinite route lifetime:
#if RPL_CONF_LEAF_ONLY
//#define RPL_ROUTE_INFINITE_LIFETIME             1
#endif

//This RPL implementation uses only 1 default router
#undef UIP_CONF_DS6_SINGLE_DEFRT
#define UIP_CONF_DS6_SINGLE_DEFRT               1 //Only allow 1 default router (optimize code size!)

//This RPL implementation allows always infinite default router (no lifetime)
#undef UIP_CONF_DS6_INFINITE_DEFRT
#define UIP_CONF_DS6_INFINITE_DEFRT             1 //Only no lifetime default router (optimize code size!)

//This RPL implementation allows always infinite addresses (no lifetime)
#undef UIP_CONF_DS6_INFINITE_ADDRESSES
#define UIP_CONF_DS6_INFINITE_ADDRESSES         1 //infinite lifetime unicast addresses (optimize code size!)

//Define the function to call for RPL events
#define RPL_EVENT_NOTIFICATION_FUNC     	rpl_event_notification

#endif
