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
* include this software, other than combinations with devices manufactured by or for TI (TI Devices).
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
* THIS SOFTWARE IS PROVIDED BY TI AND TIS LICENSORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
* BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TIS LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
/*
 *  ====================== uip_rpl_attributes.h =============================================
 */


#ifndef __UIP_RPL_ATTRIBUTES_H__
#define __UIP_RPL_ATTRIBUTES_H__

//Default values
#define UIP_RPL_ATTR_DIO_INTERVAL_MIN		12
#define UIP_RPL_ATTR_DIO_INTERVAL_DOUBLINGS	8
#define UIP_RPL_ATTR_DAO_LATENCY        	4
#define UIP_RPL_ATTR_PREFIX                     0x20,0x01,0x0d,0xb8,0x12,0x34,0xff,0xff

#define UIP_RPL_ATTR_PREFIX_LEN                 (8)
#define UIP_RPL_ATTR_ROUTING_TABLE_ENTRY_LEN    (38)

#ifndef TI_MAC_USE_MAC64 //Using short MAC addresses
#define UIP_RPL_ATTR_NBR_TABLE_ENTRY_LEN       	(21)
#else //Using long MAC addresses
#define UIP_RPL_ATTR_NBR_TABLE_ENTRY_LEN       	(27)
#endif

typedef enum {
  UIP_RPL_ATTR_OK = 0,
  UIP_RPL_ATTR_ERROR_INVALID_INDEX,
  UIP_RPL_ATTR_ERROR_STACK_STARTED,
  UIP_RPL_ATTR_ERROR_STACK_NOT_STARTED,
  UIP_RPL_ATTR_ERROR
}uip_rpl_attr_status;


typedef struct{
#ifndef RPL_DISABLE
  uint8_t       dio_interval_min;
  uint8_t       dio_interval_doublings;
  uint32_t      dao_latency;
#endif
  uint8_t       prefix[UIP_RPL_ATTR_PREFIX_LEN];
}uip_rpl_attr_t;

extern uip_rpl_attr_t uip_rpl_attributes;

//The UIP RPL attributes must be init before the Stack is initiated
void uip_rpl_attr_init();

#ifndef RPL_DISABLE

//This value is the current DIO transmission delay. Cannot be obtained if the stack was not started.
uip_rpl_attr_status uip_rpl_attr_get_dio_next_delay(uint32_t *delay);

//This is the RPL DIO interval min parameter.
uip_rpl_attr_status uip_rpl_attr_get_dio_interval_min(uint8_t *int_min);

//This is the RPL DIO interval min parameter. Cannot be set of the stack was already started
uip_rpl_attr_status uip_rpl_attr_set_dio_interval_min(uint8_t int_min);

//This is the RPL DIO interval doublings parameter.
uip_rpl_attr_status uip_rpl_attr_get_dio_interval_doublings(uint8_t *int_doublings);

//This is the RPL DIO interval doublings parameter. Cannot be set of the stack was already started
uip_rpl_attr_status uip_rpl_attr_set_dio_interval_doublings(uint8_t int_doublings);

//This is the RPL DAO latency. The DAO messages will be scheduled to be sent within a random timer between dao_latency and dao_latency/2.
uip_rpl_attr_status uip_rpl_attr_get_dao_latency(uint8_t *dao_latency);

//This is the RPL DAO latency.
uip_rpl_attr_status uip_rpl_attr_set_dao_latency(uint8_t dao_latency);

#endif //!RPL_DISABLE

//This is the IPv6 prefix used for RPL Root. For every node it will be set as 6LoWPAN Context 0 for header compression
uip_rpl_attr_status uip_rpl_attr_get_ip_global_prefix(void* prefix);

//This is the IPv6 prefix used for RPL Root.
uip_rpl_attr_status uip_rpl_attr_set_ip_global_prefix(void* prefix);

//This is the IPv6 routing table. Cannot be obtained if the stack was not started.
uip_rpl_attr_status uip_rpl_attr_get_ip_routing_table_entry(uint16_t index, char* entry);

//This is the IPv6 neighbor table. Cannot be obtained if the stack was not started.
uip_rpl_attr_status uip_rpl_attr_get_neighbor_table_entry(uint16_t index, char* entry);

#endif
