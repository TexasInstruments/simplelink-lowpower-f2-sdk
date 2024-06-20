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
 *  ====================== rpl_api.h =============================================
 *  RPL API header file for upper layer. 
 */

#ifndef RPL_API_H
#define RPL_API_H

#include "uip-conf.h"
#include "net/uip.h"

struct rpl_instance;
struct rpl_parent;
typedef struct rpl_parent rpl_parent_t;
typedef struct rpl_instance rpl_instance_t;

/*----------------------------------------------------------------------------*/

#define RPL_EVENT_INSTANCE_JOINED       0x01
#define RPL_EVENT_LOCAL_REPAIR          0x02
#define RPL_EVENT_PARENT_SWITCH		    0x03

void rpl_event_notification_prototype(int event, uint16_t instance_id);
/*----------------------------------------------------------------------------*/
/* Public API RPL functions. */
//Reset RPL the instance DIO timer
void rpl_reset_dio_timer(rpl_instance_t *instance);
#if UIP_RPL_ROOT
//Start the RPL Instance
void rpl_start_instance(uip_ipaddr_t *prefix);
//Trigger global repair
int rpl_repair_root(uint16_t instance_id);
#endif
//Increment the RPL Instance DTSN value
void rpl_dtsn_increment(rpl_instance_t *instance);
/*----------------------------------------------------------------------------*/
//RPL functions for accessing internal structures
rpl_instance_t *rpl_get_default_instance();
rpl_instance_t *rpl_get_instance(uint16_t instance_id);
rpl_parent_t *rpl_parents_head();
rpl_parent_t *rpl_parents_next(rpl_parent_t * entry);
/*----------------------------------------------------------------------------*/
#endif /* RPL_API_H */
