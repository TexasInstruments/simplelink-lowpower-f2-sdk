/**
 * \addtogroup uip6
 * @{
 */
/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
/**
 * \file
 *         ICMP6 I/O for RPL control messages.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 * Contributors: Niclas Finne <nfi@sics.se>, Joel Hoglund <joel@sics.se>,
 *               Mathieu Pouillot <m.pouillot@watteco.com>
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

#include "net/tcpip.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-nd6.h"
#include "net/uip-icmp6.h"
#include "rpl/rpl-private.h"
#include "net/packetbuf.h"
#include "rpl/rpl-plusplus.h"

#ifdef LINUX_GATEWAY
#include "nm.h"
#endif

#include <limits.h>
#include <string.h>
#include "uip_rpl_process.h"

#ifdef DEBUG  
#undef DEBUG
#endif
#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#if UIP_CONF_IPV6
/*---------------------------------------------------------------------------*/
#define RPL_DIO_GROUNDED                 0x80
#define RPL_DIO_MOP_SHIFT                3
#define RPL_DIO_MOP_MASK                 0x3c
#define RPL_DIO_PREFERENCE_MASK          0x07

#define UIP_IP_BUF       ((struct uip_ip_hdr *)&rpl_ip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF     ((struct uip_icmp_hdr *)&rpl_ip_buf[uip_l2_l3_hdr_len])
#define UIP_ICMP_PAYLOAD ((unsigned char *)&rpl_ip_buf[uip_l2_l3_icmp_hdr_len])
/*---------------------------------------------------------------------------*/
#if !RPL_SUPPRESS_DIS_CONF
static void dis_input(void);
#endif
static void dio_input(void);
#if ((RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST) || NON_STORING_MODE_ROOT)
static void dao_input(void);
#endif
#if RPL_CONF_DAO_ACK
static void dao_ack_input(void);
extern void rpl_set_dao_ack_timer(rpl_instance_t *instance, rpl_parent_t *parent, uip_ipaddr_t *prefix, uint16_t lifetime);
extern void rpl_stop_dao_ack_timer(rpl_instance_t *instance);
#endif

void candidate_add(uip_ipaddr_t *from, rpl_dio_t *dio);

uip_ipaddr_t candidate_addr[DAO_MAX_TARGET - 1];
uint16_t candidate_rank[DAO_MAX_TARGET - 1] = { 0, };

void set_ip_from_prefix(uip_ipaddr_t *ipaddr, rpl_prefix_t *prefix);

/* some debug callbacks useful when debugging RPL networks */
#ifdef RPL_DEBUG_DIO_INPUT
void RPL_DEBUG_DIO_INPUT(uip_ipaddr_t *, rpl_dio_t *);
#endif

#ifdef RPL_DEBUG_DAO_OUTPUT
void RPL_DEBUG_DAO_OUTPUT(rpl_parent_t *);
#endif

/* some debug callbacks useful when debugging RPL networks */
#ifdef RPL_DEBUG_DIO_INPUT
void RPL_DEBUG_DIO_INPUT(uip_ipaddr_t *, rpl_dio_t *);
#endif

#ifdef RPL_DEBUG_DAO_OUTPUT
void RPL_DEBUG_DAO_OUTPUT(rpl_parent_t *);
#endif

extern rpl_of_t RPL_OF;

/*---------------------------------------------------------------------------*/
static inline uint32_t get32(unsigned char *buffer, int pos)
{
    return (uint32_t) pltfrm_byte_get(buffer, pos) << 24
            | (uint32_t) pltfrm_byte_get(buffer, pos + 1) << 16
            | (uint32_t) pltfrm_byte_get(buffer, pos + 2) << 8
            | pltfrm_byte_get(buffer, pos + 3);
}
/*---------------------------------------------------------------------------*/
static inline void set32(unsigned char *buffer, int pos, uint32_t value)
{
    pltfrm_byte_set(buffer, pos, value >> 24);
    ++pos;
    pltfrm_byte_set(buffer, pos, (value >> 16) & 0xff);
    ++pos;
    pltfrm_byte_set(buffer, pos, (value >> 8) & 0xff);
    ++pos;
    pltfrm_byte_set(buffer, pos, value & 0xff);
}
/*---------------------------------------------------------------------------*/
static inline uint16_t get16(unsigned char *buffer, int pos)
{
    return (uint16_t) pltfrm_byte_get(buffer, pos) << 8
            | pltfrm_byte_get(buffer, pos + 1);
}
/*---------------------------------------------------------------------------*/
static inline void set16(unsigned char *buffer, int pos, uint16_t value)
{
    pltfrm_byte_set(buffer, pos, value >> 8);
    ++pos;
    pltfrm_byte_set(buffer, pos, value & 0xff);
}

#define get8(my_buf,my_pos)           pltfrm_byte_get(my_buf,my_pos)
#define set8(my_buf,my_pos,my_val)    pltfrm_byte_set(my_buf,my_pos,my_val)

#if !RPL_SUPPRESS_DIS_CONF
static void
dis_input(void)
{
    rpl_instance_t *instance;
#if !RPL_SINGLE_INSTANCE
    rpl_instance_t *end;
#endif

    /* DAG Information Solicitation */
    PRINTF("RPL: Received a DIS from ");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("\n");

#if RPL_SINGLE_INSTANCE
    instance = &single_instance;
#else
    for(instance = &instance_table[0], end = instance + RPL_MAX_INSTANCES; instance < end; ++instance)
#endif
    {
        if(instance->used == 1)
        {
#if RPL_LEAF_ONLY
            if(!uip_is_addr_mcast(&UIP_IP_BUF->destipaddr))
            {
                PRINTF("RPL: LEAF ONLY Multicast DIS will NOT reset DIO timer\n");
#else /* !RPL_LEAF_ONLY */
                if(uip_is_addr_mcast(&UIP_IP_BUF->destipaddr))
                {
                    PRINTF("RPL: Multicast DIS => reset DIO timer\n");
                    rpl_reset_dio_timer(instance);
                }
                else
                {
#endif /* !RPL_LEAF_ONLY */
                    PRINTF("RPL: Unicast DIS, reply to sender\n");
                    dio_output(instance, &UIP_IP_BUF->srcipaddr);
                }
            }
        }
    }
    /*---------------------------------------------------------------------------*/
    void
    dis_output(uip_ipaddr_t *addr)
    {
        unsigned char *buffer;
        uip_ipaddr_t tmpaddr;

        /* DAG Information Solicitation  - 2 bytes reserved      */
        /*      0                   1                   2        */
        /*      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3  */
        /*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
        /*     |     Flags     |   Reserved    |   Option(s)...  */
        /*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

        buffer = UIP_ICMP_PAYLOAD;
        //buffer[0] = buffer[1] = 0;
        set8(buffer, 0, 0);
        set8(buffer, 1, 0);

        if(addr == NULL)
        {
            uip_create_linklocal_rplnodes_mcast(&tmpaddr);
            addr = &tmpaddr;
        }

        PRINTF("RPL: Sending a DIS to ");
        PRINT6ADDR(addr);
        PRINTF("\n");

        UIP_STAT_TX_DIS_NUM_INC;

        RPL_ICMP6_SEND(addr, RPL_CODE_DIS, 2);
    }
#endif
/*---------------------------------------------------------------------------*/
static void dio_input(void)
{
    unsigned char *buffer;
    uint16_t buffer_length;
    rpl_dio_t dio;
    uint16_t subopt_type;
    int i;
    int len;
    uip_ipaddr_t from;
    uip_ds6_nbr_t *nbr;

    memset(&dio, 0, sizeof(dio));

    /* Set default values in case the DIO configuration option is missing. */
    dio.dag_intdoubl = RPL_DIO_INTERVAL_DOUBLINGS;
    dio.dag_intmin = RPL_DIO_INTERVAL_MIN;
    dio.dag_redund = RPL_DIO_REDUNDANCY;
    dio.dag_min_hoprankinc = RPL_MIN_HOPRANKINC;
    dio.dag_max_rankinc = RPL_MAX_RANKINC;
    dio.ocp = RPL_OF.ocp;
    dio.default_lifetime = RPL_DEFAULT_LIFETIME;
    dio.lifetime_unit = RPL_DEFAULT_LIFETIME_UNIT;

    uip_ipaddr_copy(&from, &UIP_IP_BUF->srcipaddr);

    /* DAG Information Object */
    PRINTF("RPL: Received a DIO from ");PRINT6ADDR(&from);PRINTF("\n");

#if 0
    if (!RPL_ACCEPT_DIO_FROM((rimeaddr_t*)&rpl_ip_src_lladdr))
    {
        LOG_INFO("rpl: drop DIO\n");
        return;
    }
#endif

    if ((nbr = uip_ds6_nbr_lookup(&from)) == NULL)
    {
        if ((nbr = uip_ds6_nbr_add(&from, &rpl_ip_src_lladdr, 0, NBR_REACHABLE)) != NULL)
        {
            /* set reachable timer */
            stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
            PRINTF("RPL: Neighbor added to neighbor cache ");
            PRINT6ADDR(&from);
            PRINTF(", ");
            PRINTLLADDR(&rpl_ip_src_lladdr);
            PRINTF("");
        }
        else
        {
            PRINTF("RPL: Out of Memory, dropping DIO from ");
            PRINT6ADDR(&from);
            PRINTF(", ");
            PRINTLLADDR(&rpl_ip_src_lladdr);
            PRINTF("");
            return;
        }

    }
    else
    {
        PRINTF("RPL: Neighbor already in neighbor cache\n");
    }

    //buffer_length in BYTE units
    buffer_length = rpl_ip_len- BYTE_UNITS(uip_l3_icmp_hdr_len);

    /* Process the DIO base option. */
    i = 0;
    buffer = UIP_ICMP_PAYLOAD;

    dio.instance_id = get8(buffer, i++); //buffer[i++];
    dio.version = get8(buffer, i++); //buffer[i++];
    dio.rank = get16(buffer, i);
    i += 2;

    PRINTF("RPL: Incoming DIO (id, ver, rank) = (%u,%u,%u)\n",
            (unsigned)dio.instance_id,
            (unsigned)dio.version,
            (unsigned)dio.rank);

    dio.grounded = get8(buffer, i)& RPL_DIO_GROUNDED; //buffer[i] & RPL_DIO_GROUNDED;
    dio.mop = (get8(buffer, i)& RPL_DIO_MOP_MASK) >> RPL_DIO_MOP_SHIFT; //(buffer[i]& RPL_DIO_MOP_MASK) >> RPL_DIO_MOP_SHIFT;
    dio.preference = get8(buffer, i++)& RPL_DIO_PREFERENCE_MASK; //buffer[i++] & RPL_DIO_PREFERENCE_MASK;

    dio.dtsn = get8(buffer, i++); //buffer[i++];
    /* two reserved bytes */
    i += 2;

    //memcpy(&dio.dag_id, buffer + i, sizeof(dio.dag_id));
    pltfrm_byte_memcpy(&dio.dag_id, 0, buffer, i,
                       BYTE_UNITS(sizeof(dio.dag_id)));
    i += BYTE_UNITS(sizeof(dio.dag_id));

    PRINTF("RPL: Incoming DIO (dag_id, pref) = (");PRINT6ADDR(&dio.dag_id);PRINTF(", %u)", dio.preference);

    /* Check if there are any DIO suboptions. */
    for (; i < buffer_length; i += len)
    {
        subopt_type = get8(buffer, i); //buffer[i];
        if (subopt_type == RPL_OPTION_PAD1)
        {
            len = 1;
        }
        else
        {
            /* Suboption with a two-byte header + payload */
            len = 2 + get8(buffer, i + 1); //buffer[i + 1];
        }

        if (len + i > buffer_length)
        {
            PRINTF("RPL: Invalid DIO packet");RPL_STAT(rpl_stats.malformed_msgs++);UIP_STAT_RX_RPL_DROP_DIO_MALFORMED_NUM_INC;
            return;
        }

        PRINTF("RPL: DIO option Xu, length: Xu");PRINTF_HEX(subopt_type);PRINTF_HEX(len - 2);

        switch (subopt_type)
        {
        case RPL_OPTION_DAG_METRIC_CONTAINER:
            if (len < 6)
            {
                PRINTF("RPL: Invalid DAG MC, len = Xd"); //, len);
                RPL_STAT(rpl_stats.malformed_msgs++);UIP_STAT_RX_RPL_DROP_DIO_MALFORMED_NUM_INC;
                return;
            }
            dio.mc.type = get8(buffer, i + 2); //buffer[i + 2];
            dio.mc.flags = get8(buffer, i+3)<< 1; //buffer[i + 3] << 1;
            dio.mc.flags |= get8(buffer, i+4) >> 7;//buffer[i + 4] >> 7;
            dio.mc.aggr = (get8(buffer, i+4) >> 4) & 0x3;//(buffer[i + 4] >> 4) & 0x3;
            dio.mc.prec = get8(buffer, i+4) & 0xf;//buffer[i + 4] & 0xf;
            dio.mc.length = get8(buffer, i+5);//buffer[i + 5];

            if(dio.mc.type == RPL_DAG_MC_NONE)
            {
                /* No metric container: do nothing */
            }
            else if(dio.mc.type == RPL_DAG_MC_ETX)
            {
                dio.mc.obj.etx = get16(buffer, i + 6);

                PRINTF("RPL: DAG MC: type Xu, flags Xu, aggr Xu, prec Xu, length Xu, ETX Xu");
                PRINTF_HEX((unsigned)dio.mc.type);
                PRINTF_HEX((unsigned)dio.mc.flags);
                PRINTF_HEX((unsigned)dio.mc.aggr);
                PRINTF_HEX((unsigned)dio.mc.prec);
                PRINTF_HEX((unsigned)dio.mc.length);
                PRINTF_HEX((unsigned)dio.mc.obj.etx);
            }
            else if(dio.mc.type == RPL_DAG_MC_ENERGY)
            {
                dio.mc.obj.energy.flags = get8(buffer, i+6); //buffer[i + 6];
                dio.mc.obj.energy.energy_est = get8(buffer, i+7);//buffer[i + 7];
            }
            else
            {
                PRINTF("RPL: Unhandled DAG MC type: Xu");
                PRINTF_HEX((unsigned)dio.mc.type);
                UIP_STAT_RX_RPL_DROP_DIO_UNSUPPORTED_MC_NUM_INC;
                return;
            }
            break;
            case RPL_OPTION_ROUTE_INFO:
            if(len < 9)
            {
                PRINTF("RPL: Invalid destination prefix option, len = Xd");
                PRINTF_HEX(len);
                RPL_STAT(rpl_stats.malformed_msgs++);
                UIP_STAT_RX_RPL_DROP_DIO_MALFORMED_NUM_INC;
                return;
            }

            /* The flags field includes the preference value. */
            dio.destination_prefix.length = get8(buffer, i+2); //buffer[i + 2];
            dio.destination_prefix.flags = get8(buffer, i+3);//buffer[i + 3];
            dio.destination_prefix.lifetime = get32(buffer, i + 4);

            if(((dio.destination_prefix.length + 7) / 8) + 8 <= len &&
                    dio.destination_prefix.length <= 128)
            {
                PRINTF("RPL: Copying destination prefix");
                //memcpy(&dio.destination_prefix.prefix, &buffer[i + 8], (dio.destination_prefix.length + 7) / 8);
                pltfrm_byte_memcpy(&dio.destination_prefix.prefix, 0, buffer, i+8, (dio.destination_prefix.length + 7) / 8);
            }
            else
            {
                LOG_INFO1("RPL: Invalid route info option, len = %d\n", len);
                RPL_STAT(rpl_stats.malformed_msgs++);
                UIP_STAT_RX_RPL_DROP_DIO_MALFORMED_NUM_INC;
                return;
            }

            break;
            case RPL_OPTION_DAG_CONF:
            if(len != 16)
            {
                LOG_INFO1("RPL: Invalid DAG configuration option, len = %d\n", len);
                RPL_STAT(rpl_stats.malformed_msgs++);
                UIP_STAT_RX_RPL_DROP_DIO_MALFORMED_NUM_INC;
                return;
            }

            /* Path control field not yet implemented - at i + 2 */
            dio.dag_intdoubl = get8(buffer, i+3); //buffer[i + 3];
            dio.dag_intmin = get8(buffer, i+4);//buffer[i + 4];
            dio.dag_redund = get8(buffer, i+5);//buffer[i + 5];
            dio.dag_max_rankinc = get16(buffer, i + 6);
            dio.dag_min_hoprankinc = get16(buffer, i + 8);
            dio.ocp = get16(buffer, i + 10);
            /* buffer + 12 is reserved */
            dio.default_lifetime = get8(buffer, i+13); //buffer[i + 13];
            dio.lifetime_unit = get16(buffer, i + 14);
            PRINTF("RPL: DIO Conf:dbl=Xd, min=Xd red=Xd maxinc=Xd mininc=Xd ocp=Xd d_l=Xu l_u=Xu");
            PRINTF_HEX(dio.dag_intdoubl);
            PRINTF_HEX(dio.dag_intmin);
            PRINTF_HEX(dio.dag_redund);
            PRINTF_HEX(dio.dag_max_rankinc);
            PRINTF_HEX(dio.dag_min_hoprankinc);
            PRINTF_HEX(dio.ocp);
            PRINTF_HEX(dio.default_lifetime);
            PRINTF_HEX(dio.lifetime_unit);
            break;
            case RPL_OPTION_PREFIX_INFO:
            if(len != 32)
            {
                PRINTF("RPL: DAG Prefix info not ok, len != 32");
                RPL_STAT(rpl_stats.malformed_msgs++);
                UIP_STAT_RX_RPL_DROP_DIO_MALFORMED_NUM_INC;
                return;
            }
            dio.prefix_info.length = get8(buffer, i+2); //buffer[i + 2];
            dio.prefix_info.flags = get8(buffer, i+3);//buffer[i + 3];
            /* valid lifetime is ingnored for now - at i + 4 */
            /* preferred lifetime stored in lifetime */
            dio.prefix_info.lifetime = get32(buffer, i + 8);
            /* 32-bit reserved at i + 12 */
            PRINTF("RPL: Copying prefix information");
            //memcpy(&dio.prefix_info.prefix, &buffer[i + 16], 16);
            pltfrm_byte_memcpy(&dio.prefix_info.prefix, 0, buffer, i+16, 16);
            break;
            default:
            PRINTF("RPL: Unsupported suboption type in DIO: Xu");
            PRINTF_HEX((unsigned)subopt_type);
        }
    }

#ifdef RPL_DEBUG_DIO_INPUT
    RPL_DEBUG_DIO_INPUT(&from, &dio);
#endif

    if (!RPL_ACCEPT_DIO_FROM((rimeaddr_t*)&rpl_ip_src_lladdr))
    {
        LOG_INFO("rpl: drop DIO but record\n");
        candidate_add(&from, &dio);
    }
    else
    {
        rpl_process_dio(&from, &dio);
    }
}
/*---------------------------------------------------------------------------*/
void dio_output(rpl_instance_t *instance, uip_ipaddr_t *uc_addr)
{
#ifdef LINUX_GATEWAY
    NM_report_dio();
#endif
    unsigned char *buffer;
    int pos;
    rpl_dag_t *dag = instance->current_dag;
#if !RPL_LEAF_ONLY
    uip_ipaddr_t addr;
#endif /* !RPL_LEAF_ONLY */

#if RPL_LEAF_ONLY
    /* In leaf mode, we send DIO message only as unicasts in response to
     unicast DIS messages. */
    if(uc_addr == NULL)
    {
        PRINTF("RPL: LEAF ONLY have multicast addr: skip dio_output\n");
        return;
    }
#endif /* RPL_LEAF_ONLY */

    /* DAG Information Object */
    pos = 0;

    buffer = UIP_ICMP_PAYLOAD;
    set8(buffer, pos++, instance->instance_id); //buffer[pos++] = instance->instance_id;
    set8(buffer, pos++, dag->version); //buffer[pos++] = dag->version;

#if RPL_LEAF_ONLY
    PRINTF("RPL: LEAF ONLY DIO rank set to INFINITE_RANK\n");
    set16(buffer, pos, INFINITE_RANK);
#else /* RPL_LEAF_ONLY */
    set16(buffer, pos, dag->rank);
#endif /* RPL_LEAF_ONLY */
    pos += 2;

    set8(buffer, pos, 0); //buffer[pos] = 0;
    if (dag->grounded)
    {
        set8(buffer, pos, (get8(buffer,pos) | RPL_DIO_GROUNDED)); //buffer[pos] |= RPL_DIO_GROUNDED;
    }

    set8(buffer, pos, get8(buffer,pos) | (instance->mop << RPL_DIO_MOP_SHIFT)); //buffer[pos] |= instance->mop << RPL_DIO_MOP_SHIFT;
    set8(buffer, pos,
         get8(buffer,pos) | (dag->preference & RPL_DIO_PREFERENCE_MASK)); //buffer[pos] |= dag->preference & RPL_DIO_PREFERENCE_MASK;
    pos++;

    set8(buffer, pos++, instance->dtsn_out); //buffer[pos++] = instance->dtsn_out;

#if !RPL_PLUSPLUS_DTSN_STRATEGY //The strategy is not to increment the DTSN value for each DIO sent
#if ((RPL_MOP_DEFAULT != RPL_MOP_NON_STORING) || NON_STORING_MODE_ROOT)
    /*
     * RFC 6550, Section 9.6: Typically, in a Non-Storing mode of operation, all non-root
     nodes would increment their DTSN only when their parent(s) are
     observed to do so.
     */
    RPL_LOLLIPOP_INCREMENT(instance->dtsn_out);
#endif
#endif // !RPL_PLUSPLUS_DTSN_STRATEGY

    /* reserved 2 bytes */
    set8(buffer, pos++, 0); //buffer[pos++] = 0; /* flags */
    set8(buffer, pos++, 0); //buffer[pos++] = 0; /* reserved */

    //memcpy(buffer + pos, &dag->dag_id, sizeof(dag->dag_id));
    pltfrm_byte_memcpy(buffer, pos, &dag->dag_id, 0,
                       BYTE_UNITS(sizeof(dag->dag_id)));
    pos += BYTE_UNITS(sizeof(dag->dag_id)); //16;

#if !RPL_LEAF_ONLY
    if (instance->mc.type != RPL_DAG_MC_NONE)
    {
        instance->of->update_metric_container(instance);

        set8(buffer, pos++, RPL_OPTION_DAG_METRIC_CONTAINER); //buffer[pos++] = RPL_OPTION_DAG_METRIC_CONTAINER;
        set8(buffer, pos++, 6); //buffer[pos++] = 6;
        set8(buffer, pos++, instance->mc.type); //buffer[pos++] = instance->mc.type;
        set8(buffer, pos++, instance->mc.flags >> 1); //buffer[pos++] = instance->mc.flags >> 1;
        set8(buffer, pos, (instance->mc.flags & 1) << 7); //buffer[pos] = (instance->mc.flags & 1) << 7;
        set8(buffer,
             pos,
             get8(buffer, pos) | ((instance->mc.aggr << 4) | instance->mc.prec)); //buffer[pos++] |= (instance->mc.aggr << 4) | instance->mc.prec;
        pos++;
        if (instance->mc.type == RPL_DAG_MC_ETX)
        {
            set8(buffer, pos++, 2); //buffer[pos++] = 2;
            set16(buffer, pos, instance->mc.obj.etx);
            pos += 2;
        }
        else if (instance->mc.type == RPL_DAG_MC_ENERGY)
        {
            set8(buffer, pos++, 2); //buffer[pos++] = 2;
            set8(buffer, pos++, instance->mc.obj.energy.flags); //buffer[pos++] = instance->mc.obj.energy.flags;
            set8(buffer, pos++, instance->mc.obj.energy.energy_est); //buffer[pos++] = instance->mc.obj.energy.energy_est;
        }
        else
        {
            PRINTF("RPL: Unable to send DIO because of unhandled DAG MC type %u\n",
                    (unsigned)instance->mc.type);
            return;
        }
    }
#endif /* !RPL_LEAF_ONLY */

    /* Always add a DAG configuration option. */
    set8(buffer, pos++, RPL_OPTION_DAG_CONF); //buffer[pos++] = RPL_OPTION_DAG_CONF;
    set8(buffer, pos++, 14); //buffer[pos++] = 14;
    set8(buffer, pos++, 0); //buffer[pos++] = 0; /* No Auth, PCS = 0 */
    set8(buffer, pos++, instance->dio_intdoubl); //buffer[pos++] = instance->dio_intdoubl;
    set8(buffer, pos++, instance->dio_intmin); //buffer[pos++] = instance->dio_intmin;
    set8(buffer, pos++, instance->dio_redundancy); //buffer[pos++] = instance->dio_redundancy;
    set16(buffer, pos, instance->max_rankinc);
    pos += 2;
    set16(buffer, pos, instance->min_hoprankinc);
    pos += 2;
    /* OCP is in the DAG_CONF option */
    set16(buffer, pos, instance->of->ocp);
    pos += 2;
    set8(buffer, pos++, 0); //buffer[pos++] = 0; /* reserved */
    set8(buffer, pos++, instance->default_lifetime); //buffer[pos++] = instance->default_lifetime;
    set16(buffer, pos, instance->lifetime_unit);
    pos += 2;

    /* Check if we have a prefix to send also. */
    if (dag->prefix_info.length > 0)
    {
        set8(buffer, pos++, RPL_OPTION_PREFIX_INFO); //buffer[pos++] = RPL_OPTION_PREFIX_INFO;
        set8(buffer, pos++, 30); //buffer[pos++] = 30; /* always 30 bytes + 2 long */
        set8(buffer, pos++, dag->prefix_info.length); //buffer[pos++] = dag->prefix_info.length;
        set8(buffer, pos++, dag->prefix_info.flags); //buffer[pos++] = dag->prefix_info.flags;
//#if RPL_MOP_DEFAULT == RPL_MOP_NON_STORING
             //we need to activate the R flag
        /*
         * RFC 6550, Section 6.7.10: R: 1-bit router address flag.  When set, it indicates that the
         Prefix field contains a complete IPv6 address assigned to the
         sending router that can be used as parent in a target option.
         The indicated prefix is the first prefix length bits of the
         Prefix field.  The router IPv6 address has the same scope and
         conforms to the same lifetime values as the advertised prefix.
         This use of the Prefix field is compatible with its use in
         advertising the prefix itself, since Prefix Advertisement uses
         only the leading bits.
         */
        //Always active the R flag
        set8(buffer, pos - 1, get8(buffer,pos-1) | RPL_PIO_ROUTER_ADDRESS_FLAG); //buffer[pos-1] |= RPL_PIO_ROUTER_ADDRESS_FLAG;
//#endif
        set32(buffer, pos, dag->prefix_info.lifetime);
        pos += 4;
        set32(buffer, pos, dag->prefix_info.lifetime);
        pos += 4;
        //memset(&buffer[pos], 0, 4);
        pltfrm_byte_memset(buffer, pos, 0, 4);
        pos += 4;
        //memcpy(&buffer[pos], &dag->prefix_info.prefix, 16);
        pltfrm_byte_memcpy(buffer, pos, &dag->prefix_info.prefix, 0, 16);
//#if RPL_MOP_DEFAULT == RPL_MOP_NON_STORING
        //we need to put a global address, rewrite the prefix position if NON-STORING MODE
        /*
         * RFC 6550, Section 6.7.10:    Prefix: An IPv6 address or a prefix of an IPv6 address.  The Prefix
         Length field contains the number of valid leading bits in the
         prefix.  The bits in the prefix after the prefix length are
         reserved and MUST be initialized to zero by the sender and
         ignored by the receiver.  A router SHOULD NOT send a prefix
         option for the link-local prefix, and a host SHOULD ignore such
         a prefix option.  A non-storing node SHOULD refrain from
         advertising a prefix till it owns an address of that prefix,
         and then it SHOULD advertise its full address in this field,
         with the 'R' flag set.  The children of a node that so
         advertises a full address with the 'R' flag set may then use
         that address to determine the content of the DODAG Parent
         Address subfield of the Transit Information option.
         */
        //Always send address
        set_ip_from_prefix((uip_ipaddr_t*) (buffer + CHAR_BIT_UNITS(pos)),
                           &dag->prefix_info);
//#endif
        pos += 16;
        PRINTF("RPL: Sending prefix info in DIO for ");PRINT6ADDR(&dag->prefix_info.prefix);PRINTF("");
    }
    else
    {
        PRINTF("RPL: No prefix to announce (len %d)",
                dag->prefix_info.length);
    }

#if RPL_LEAF_ONLY
#if (DEBUG) & DEBUG_PRINT
    if(uc_addr == NULL)
    {
        PRINTF("RPL: LEAF ONLY sending unicast-DIO from multicast-DIO\n");
    }
#endif /* DEBUG_PRINT */
    PRINTF("RPL: Sending unicast-DIO with rank %u to ",
            (unsigned)dag->rank);
    LOG_IPV6ADDR(uc_addr);
    LOG_INFO("\n");
    RPL_ICMP6_SEND(uc_addr, RPL_CODE_DIO, pos);
#else /* RPL_LEAF_ONLY */
    /* Unicast requests get unicast replies! */
    if (uc_addr == NULL)
    {
        LOG_INFO1("RPL: Sending a multicast-DIO with rank %u\n",
                (unsigned)instance->current_dag->rank);
        uip_create_linklocal_rplnodes_mcast(&addr);UIP_STAT_TX_DIO_NUM_INC;
        RPL_ICMP6_SEND(&addr, RPL_CODE_DIO, pos);
    }
    else
    {
        PRINTF("RPL: Sending unicast-DIO with rank %u to ",
                (unsigned)instance->current_dag->rank);PRINT6ADDR(uc_addr);PRINTF("\n");

        UIP_STAT_TX_DIO_NUM_INC;

        RPL_ICMP6_SEND(uc_addr, RPL_CODE_DIO, pos);
    }
#endif /* RPL_LEAF_ONLY */
}
/*---------------------------------------------------------------------------*/
#if ((RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST) || NON_STORING_MODE_ROOT)
static void dao_input(void)
{
    uip_ipaddr_t dao_sender_addr;
    rpl_dag_t *dag;
    rpl_instance_t *instance;
    unsigned char *buffer;
#if !RPL_SUPPRESS_DAO_ACK_CONF
    uint16_t sequence;
#endif
    uint16_t instance_id;
    uint16_t lifetime;
    uint16_t prefixlen;
    uint16_t flags;
    uint16_t subopt_type;
#if (RPL_MOP_DEFAULT == NON_STORING_MODE_ROOT)
    uip_ipaddr_t target_parent;
#endif

    /*
     uint8_t pathcontrol;
     uint8_t pathsequence;
     */
    uip_ipaddr_t prefix;
#if !NON_STORING_MODE_ROOT
    uip_ds6_route_t *rep;
#else
    uip_ds6_route_sourceRouting_t *rep;
#endif
    uint16_t buffer_length;
    int pos;
    int len;
    int i;
    int learned_from;
#if !NON_STORING_MODE_ROOT
    rpl_parent_t *p;
#endif
    uip_ds6_nbr_t *nbr;

#ifdef LINUX_GATEWAY
    uip_ipaddr_t transit_addr;
    uint16_t transit_size;
    uip_ipaddr_t target_addr[DAO_MAX_TARGET]=
    {};
    uint8_t target_flag[DAO_MAX_TARGET]=
    {};
    uint16_t target_count=0;
#endif

    uint8_t flag;

    prefixlen = 0;

    uip_ipaddr_copy(&dao_sender_addr, &UIP_IP_BUF->srcipaddr);

    /* Destination Advertisement Object */
    LOG_INFO("RPL: Received a DAO from ");LOG_IPV6ADDR(&dao_sender_addr);LOG_INFO("\n");

    buffer = UIP_ICMP_PAYLOAD;
    buffer_length = rpl_ip_len- BYTE_UNITS(uip_l3_icmp_hdr_len);

    pos = 0;
    instance_id = get8(buffer, pos++);    //buffer[pos++];

    instance = rpl_get_instance(instance_id);
    if (instance == NULL)
    {
        LOG_INFO1("RPL: Ignoring a DAO for an unknown RPL instance(%u)\n",
                instance_id);UIP_STAT_RX_RPL_DROP_DAO_UNKNOWN_INSTANCE_NUM_INC;
        return;
    }

    lifetime = instance->default_lifetime;

    flags = get8(buffer, pos++);    //buffer[pos++];
    /* reserved */
    pos++;
#if !RPL_SUPPRESS_DAO_ACK_CONF
    sequence = get8(buffer, pos++);    //buffer[pos++];
#else
    pos++;
#endif

    dag = instance->current_dag;
    /* Is the DAGID present? */
    if (flags & RPL_DAO_D_FLAG)
    {
        //if(memcmp(&dag->dag_id, &buffer[pos], sizeof(dag->dag_id))) {
        if (pltfrm_byte_memcmp(&dag->dag_id, 0, buffer, pos,
                               BYTE_UNITS(sizeof(dag->dag_id))))
        {
            PRINTF("RPL: Ignoring a DAO for a DAG different from ours\n");UIP_STAT_RX_RPL_DROP_DAO_DIFFERENT_DAG_NUM_INC;
            return;
        }
        pos += 16;
    }
    else
    {
        /* Perhaps, there are verification to do but ... */
    }

    /* Check if there are any RPL options present. */
#ifdef LINUX_GATEWAY
    memset(target_addr, 0, sizeof(target_addr));
    memset(target_flag, 0, sizeof(target_flag));
    target_count=0;
#endif

    //In this implementation we always send only one prefix in the target and only one transit ip address (parent) if NON-STORING mode
    for (i = pos; i < buffer_length; i += len)
    {
        subopt_type = get8(buffer, i);    //buffer[i];
        if (subopt_type == RPL_OPTION_PAD1)
        {
            len = 1;
        }
        else
        {
            /* The option consists of a two-byte header and a payload. */
            len = 2 + get8(buffer, i + 1);    //buffer[i + 1];
        }

        switch (subopt_type)
        {
        case RPL_OPTION_TARGET:
            /* Handle the target option. */
#ifdef LINUX_GATEWAY
            target_flag[target_count] = get8(buffer, i+2);
            memcpy(&target_addr[target_count], &buffer[i+4], sizeof(uip_ipaddr_t));
            target_count++;
#endif
            flag = get8(buffer, i + 2);
            if (flag == RPL_RESERVED_SENDER) //only handle the regular "sender" prefix
            {
                prefixlen = get8(buffer, i + 3);       //buffer[i + 3];
                memset(&prefix, 0, sizeof(prefix));
                //memcpy(&prefix, buffer + i + 4, (prefixlen + 7) / CHAR_BIT);
                pltfrm_byte_memcpy(&prefix, 0, buffer, i + 4,
                                   (prefixlen + 7) / 8);
            }
            break;
        case RPL_OPTION_TRANSIT:
            /* The path sequence and control are ignored. */
            // pathcontrol = buffer[i + 3];
            // pathsequence = buffer[i + 4];
            lifetime = get8(buffer, i + 5);       //buffer[i + 5];
#if (RPL_MOP_DEFAULT == NON_STORING_MODE_ROOT)
            uint8_t transit_size = get8(buffer, i + 1); //buffer[i + 1];
            if (transit_size == (4 + BYTE_UNITS(sizeof(target_parent)))){
                //get the ip address of the parent of the target
                //memcpy(&target_parent, buffer + i + 6, sizeof(target_parent));
                pltfrm_byte_memcpy(&target_parent, 0, buffer, i + 6, BYTE_UNITS(sizeof(target_parent)));
             }
             else{
               //Inconsistent DAO
               PRINTF("RPL: DAO has no transit information ip address. It MUST for NON-STORING MODE\n");
             }
#endif
            break;
        }
    }
#ifdef LINUX_GATEWAY
    NM_report_dao(&transit_addr,target_addr,target_flag,lifetime);
#endif

    PRINTF("RPL: DAO lifetime: %u, prefix length: %u prefix: ",
            (unsigned)lifetime, (unsigned)prefixlen);PRINT6ADDR(&prefix);PRINTF("\n");

#if NON_STORING_MODE_ROOT
    if (instance->mop == RPL_MOP_NON_STORING)
    {
        if(lifetime == RPL_ZERO_LIFETIME)
        {
            PRINTF("RPL:dropping DAO no path. Not available in non-storing mode\n");
            UIP_STAT_RX_RPL_DROP_DAO_NO_PATH_NON_STORING_NUM_INC;
            return;
        }
        learned_from = uip_is_addr_mcast(&dao_sender_addr) ?
        RPL_ROUTE_FROM_MULTICAST_DAO : RPL_ROUTE_FROM_UNICAST_DAO;
        if(learned_from == RPL_ROUTE_FROM_MULTICAST_DAO)
        {
            PRINTF("RPL:dropping DAO learned from multicast address, not possible in non-storing mode\n");
            UIP_STAT_RX_RPL_DROP_DAO_MULTICAST_NON_STORING_NUM_INC;
            return;
        }

        //New route needs to be added
        PRINTF("RPL: adding DAO route\n");

        //Is sender in the same link? To know, we will check if the announced parent is this node. If we are its parent, it is in this link
        if (uip_ds6_is_my_addr(&target_parent))
        {
            if((nbr = uip_ds6_nbr_lookup(&dao_sender_addr)) == NULL)
            {
                if((nbr = uip_ds6_nbr_add(&dao_sender_addr, &rpl_ip_src_lladdr, 0, NBR_REACHABLE)) != NULL)
                {
                    /* set reachable timer */
                    stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
                    PRINTF("RPL: Neighbor added to neighbor cache ");
                    PRINT6ADDR(&dao_sender_addr);
                    PRINTF(", ");
                    PRINTLLADDR((uip_lladdr_t *)&rpl_ip_src_lladdr);
                    PRINTF("\n");
                }
                else
                {
                    PRINTF("RPL: Out of Memory, dropping DAO from ");
                    PRINT6ADDR(&dao_sender_addr);
                    PRINTF(", ");
                    PRINTLLADDR((uip_lladdr_t *)&rpl_ip_src_lladdr);
                    PRINTF("\n");
                    return;
                }
            }
            else
            {
                PRINTF("RPL: Neighbor already in neighbor cache\n");
            }

        }

        //We need to add a route differently. Target-Parent (no next hop). We always add or update the routes as every DAO is NEW
        /*
         * RFC 6550, Section 9.2.2: In Non-Storing mode, every DAO message a node receives is "new".
         */
        rep = rpl_add_route_nonstoring(dag, &prefix, prefixlen, &target_parent);
        //rep = rpl_add_route(dag, &prefix, prefixlen, &dao_sender_addr);
        if(rep == NULL)
        {
            RPL_STAT(rpl_stats.mem_overflows++);
            PRINTF("RPL: Could not add a route after receiving a DAO\n");
            return;
        }

#if !RPL_ROUTE_INFINITE_LIFETIME
        rep->state.lifetime = RPL_LIFETIME(instance, lifetime);
#endif
        rep->state.learned_from = learned_from;

#if !RPL_SUPPRESS_DAO_ACK_CONF
        //As a dodag root we never forward any DAO. The only thing left to do is to answer DAO ACK if requested
        if(flags & RPL_DAO_K_FLAG)
        {
            dao_ack_output(instance, &dao_sender_addr, sequence);
        }
#endif
    }
#else //!NON_STORING_MODE_ROOT

    rep = uip_ds6_route_lookup(&prefix);

    if (lifetime == RPL_ZERO_LIFETIME)
    {
        PRINTF("RPL: No-Path DAO received\n");
        /* No-Path DAO received; invoke the route purging routine. */
        if (rep != NULL && rep->state.nopath_received == 0&&
        rep->length == prefixlen &&
        uip_ds6_route_nexthop(rep) != NULL &&
        uip_ipaddr_cmp(uip_ds6_route_nexthop(rep), &dao_sender_addr))
        {
            PRINTF("RPL: Setting expiration timer for prefix ");PRINT6ADDR(&prefix);PRINTF("\n");
#if RPL_ROUTE_INFINITE_LIFETIME
            uip_ds6_route_rm(rep);
#else
            rep->state.nopath_received = 1;
            rep->state.lifetime = DAO_EXPIRATION_TIMEOUT;
#endif

            /* We forward the incoming no-path DAO to our parent, if we have
             one. */
            if (dag->preferred_parent != NULL
                    && rpl_get_parent_ipaddr(dag->preferred_parent) != NULL)
            {
                PRINTF("RPL: Forwarding no-path DAO to parent ");PRINT6ADDR(rpl_get_parent_ipaddr(dag->preferred_parent));PRINTF("\n");
                RPL_ICMP6_SEND(rpl_get_parent_ipaddr(dag->preferred_parent),
                               RPL_CODE_DAO, buffer_length);
            }

#if !RPL_SUPPRESS_DAO_ACK_CONF
            if(flags & RPL_DAO_K_FLAG)
            {
                dao_ack_output(instance, &dao_sender_addr, sequence);
            }
#endif
        }
        return;
    }

    learned_from =
    uip_is_addr_mcast(&dao_sender_addr) ?
    RPL_ROUTE_FROM_MULTICAST_DAO :
                                          RPL_ROUTE_FROM_UNICAST_DAO;

    PRINTF("RPL: DAO from %s\n",
            learned_from == RPL_ROUTE_FROM_UNICAST_DAO? "unicast": "multicast");
    if (learned_from == RPL_ROUTE_FROM_UNICAST_DAO)
    {
        /* Check whether this is a DAO forwarding loop. */
        p = rpl_find_parent(dag, &dao_sender_addr);
        /* check if this is a new DAO registration with an "illegal" rank */
        /* if we already route to this node it is likely */
        if (p != NULL &&
        DAG_RANK(p->rank, instance) < DAG_RANK(dag->rank, instance))
        {
            PRINTF("RPL: Loop detected when receiving a unicast DAO from a node with a lower rank! (%u < %u)\n",
                    DAG_RANK(p->rank, instance), DAG_RANK(dag->rank, instance));
            p->rank = INFINITE_RANK;
            p->updated = 1;
            UIP_STAT_RPL_DAO_LOWER_RANK_NUM_INC;
            return;
        }

        /* If we get the DAO from our parent, we also have a loop. */
        if (p != NULL && p == dag->preferred_parent)
        {
            PRINTF("RPL: Loop detected when receiving a unicast DAO from our parent\n");
            p->rank = INFINITE_RANK;
            p->updated = 1;
            UIP_STAT_RPL_DAO_FROM_PARENT_NUM_INC;
            return;
        }
    }

    PRINTF("RPL: adding DAO route\n");

    if ((nbr = uip_ds6_nbr_lookup(&dao_sender_addr)) == NULL)
    {
        if ((nbr = uip_ds6_nbr_add(&dao_sender_addr, &rpl_ip_src_lladdr, 0, NBR_REACHABLE)) != NULL)
        {
            /* set reachable timer */
            stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
            PRINTF("RPL: Neighbor added to neighbor cache ");
            PRINT6ADDR(&dao_sender_addr);
            PRINTF(", ");
            PRINTLLADDR((uip_lladdr_t *)&rpl_ip_src_lladdr);
            PRINTF("\n");
        }
        else
        {
            PRINTF("RPL: Out of Memory, dropping DAO from ");
            PRINT6ADDR(&dao_sender_addr);
            PRINTF(", ");
            PRINTLLADDR((uip_lladdr_t *)&rpl_ip_src_lladdr);
            PRINTF("\n");
            return;
        }
    }
    else
    {
        PRINTF("RPL: Neighbor already in neighbor cache\n");
    }

    rpl_lock_parent(p);

    rep = rpl_add_route(dag, &prefix, prefixlen, &dao_sender_addr);
    if (rep == NULL)
    {
        RPL_STAT(rpl_stats.mem_overflows++);LOG_INFO("RPL: Could not add a route after receiving a DAO\n");
        return;
    }
#if !RPL_ROUTE_INFINITE_LIFETIME
    rep->state.lifetime = RPL_LIFETIME(instance, lifetime);
#endif
    rep->state.learned_from = learned_from;

    if (learned_from == RPL_ROUTE_FROM_UNICAST_DAO)
    {
        if (dag->preferred_parent != NULL
                && rpl_get_parent_ipaddr(dag->preferred_parent) != NULL)
        {
            PRINTF("RPL: Forwarding DAO to parent ");PRINT6ADDR(rpl_get_parent_ipaddr(dag->preferred_parent));PRINTF("\n");
            RPL_ICMP6_SEND(rpl_get_parent_ipaddr(dag->preferred_parent),
                           RPL_CODE_DAO, buffer_length);
        }

#if !RPL_SUPPRESS_DAO_ACK_CONF
        if(flags & RPL_DAO_K_FLAG)
        {
            dao_ack_output(instance, &dao_sender_addr, sequence);
        }
#endif
    }

#endif

}
#endif //((RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST) || NON_STORING_MODE_ROOT)
/*---------------------------------------------------------------------------*/
void dao_output(rpl_parent_t *parent, uint16_t lifetime)
{
    /* Destination Advertisement Object */
    uip_ds6_addr_t * addr = uip_ds6_get_global(ADDR_PREFERRED);
    if (addr == NULL)
    {
        addr = uip_ds6_get_global(ADDR_TENTATIVE);
        if (addr == NULL)
        {
            PRINTF("RPL: No global address set for this node - suppressing DAO\n");
            return;
        }
    }
    /* Sending a DAO with own prefix as target */
    dao_output_target(parent, &addr->ipaddr, lifetime);

#ifndef LINUX_GATEWAY
#if FEATURE_DTLS
  {
     extern uint8_t macSecurityKeyExchangeWithParent();
     extern void macSecurityStartKeyExchangeWithParent(uip_ipaddr_t *destipaddr_p);
     /* comment out to disable link-level DTLS */ //FIXME
     if (macSecurityKeyExchangeWithParent() && parent != NULL)
     {
        macSecurityStartKeyExchangeWithParent(rpl_get_parent_ipaddr(parent));
     }
  }
#endif //FEATURE_DTLS
#endif //LINUX_GATEWAY
}
/*---------------------------------------------------------------------------*/
void dao_output_target(rpl_parent_t *parent, uip_ipaddr_t *prefix,
                       uint16_t lifetime)
{
    rpl_dag_t *dag;
    rpl_instance_t *instance;
    unsigned char *buffer;
    uint16_t prefixlen;
    int pos;

    /* Destination Advertisement Object */
    if (parent == NULL)
    {
        PRINTF("RPL dao_output_target error parent NULL\n");
        return;
    }

    dag = parent->dag;
    if (dag == NULL)
    {
        PRINTF("RPL dao_output_target error dag NULL\n");
        return;
    }

    instance = dag->instance;

    if (instance == NULL)
    {
        PRINTF("RPL dao_output_target error instance NULL\n");
        return;
    }
    if (prefix == NULL)
    {
        PRINTF("RPL dao_output_target error prefix NULL\n");
        return;
    }
#ifdef RPL_DEBUG_DAO_OUTPUT
    RPL_DEBUG_DAO_OUTPUT(parent);
#endif

    buffer = UIP_ICMP_PAYLOAD;

#if RPL_CONF_DAO_ACK
    if (!instance->dao_ack_is_retry)
    {
        //Only increment the DAO Sequence counter if this is a new DAO (not a retry waiting for DAO ACK)
        RPL_LOLLIPOP_INCREMENT(instance->dao_sequence);
    }
#else
    RPL_LOLLIPOP_INCREMENT(instance->dao_sequence);
#endif

    pos = 0;

    set8(buffer, pos++, instance->instance_id); //buffer[pos++] = instance->instance_id;
    set8(buffer, pos, 0);              //buffer[pos] = 0;
#if RPL_DAO_SPECIFY_DAG
         set8(buffer, pos, get8(buffer, pos) | RPL_DAO_D_FLAG); //buffer[pos] |= RPL_DAO_D_FLAG;
#endif /* RPL_DAO_SPECIFY_DAG */
#if RPL_CONF_DAO_ACK
    set8(buffer, pos, get8(buffer, pos) | RPL_DAO_K_FLAG); //buffer[pos] |= RPL_DAO_K_FLAG;
    rpl_set_dao_ack_timer(instance, parent, prefix, lifetime);
#endif /* RPL_CONF_DAO_ACK */
    ++pos;
    set8(buffer, pos++, 0);              //buffer[pos++] = 0; /* reserved */
    set8(buffer, pos++, instance->dao_sequence); //buffer[pos++] = dao_sequence;
#if RPL_DAO_SPECIFY_DAG
         //memcpy(buffer + pos, &dag->dag_id, sizeof(dag->dag_id));
         pltfrm_byte_memcpy(buffer, pos, &dag->dag_id, 0, BYTE_UNITS(sizeof(dag->dag_id)));
         pos+=BYTE_UNITS(sizeof(dag->dag_id));
#endif /* RPL_DAO_SPECIFY_DAG */

    /* create target subopt */
    prefixlen = sizeof(*prefix) * CHAR_BIT;
    set8(buffer, pos++, RPL_OPTION_TARGET); //buffer[pos++] = RPL_OPTION_TARGET;
    set8(buffer, pos++, 2 + ((prefixlen + 7) / 8)); //buffer[pos++] = 2 + ((prefixlen + 7) / CHAR_BIT);
    set8(buffer, pos++, RPL_RESERVED_SENDER); //  set8(buffer, pos++, 0);//buffer[pos++] = 0; /* reserved */  set8(buffer, pos++, prefixlen);//buffer[pos++] = prefixlen;
    set8(buffer, pos++, prefixlen);              //buffer[pos++] = prefixlen;
    //memcpy(buffer + pos, prefix, (prefixlen + 7) / CHAR_BIT);
    pltfrm_byte_memcpy(buffer, pos, prefix, 0, (prefixlen + 7) / 8);
    //pos += ((prefixlen + 7) / CHAR_BIT);
    pos += ((prefixlen + 7) / 8);

    /* Create a transit information sub-option. */
    uint8_t i;
    for (i = 0; i < (DAO_MAX_TARGET - 1); ++i)
    {
        if (candidate_rank[i] != 0)
        {
            set8(buffer, pos++, RPL_OPTION_TARGET); //buffer[pos++] = RPL_OPTION_TARGET;
            set8(buffer, pos++, 2 + ((prefixlen + 7) / 8)); //buffer[pos++] = 2 + ((prefixlen + 7) / CHAR_BIT);
            set8(buffer, pos++, RPL_RESERVED_CANDIDATE); //  set8(buffer, pos++, 0);//buffer[pos++] = 0; /* reserved */
            set8(buffer, pos++, prefixlen);  //buffer[pos++] = prefixlen;
            pltfrm_byte_memcpy(buffer, pos, &candidate_addr[i], 0,
                               (prefixlen + 7) / 8);
            //pos += ((prefixlen + 7) / CHAR_BIT);
            pos += ((prefixlen + 7) / 8);
        }
    }

    /* Create a transit information sub-option. */
    set8(buffer, pos++, RPL_OPTION_TRANSIT); //buffer[pos++] = RPL_OPTION_TRANSIT;

    //We now always send parent's ip
    set8(buffer, pos++, 4 + BYTE_UNITS(sizeof(uip_ipaddr_t))); //buffer[pos++] = 4 + sizeof(uip_ipaddr_t);

    set8(buffer, pos++, 0);  //buffer[pos++] = 0; /* flags - ignored */
    set8(buffer, pos++, 0);  //buffer[pos++] = 0; /* path control - ignored */
    set8(buffer, pos++, 0);  //buffer[pos++] = 0; /* path seq - ignored */
    set8(buffer, pos++, lifetime);  //buffer[pos++] = lifetime;

    if (parent->router_address_valid)
    {
        /*
         * RFC 6550, Section 9.7: The DODAG Parent Address subfield of a Transit Information option
         MUST contain one or more addresses.  All of these addresses MUST
         be addresses of DAO parents of the sender
         */
        //memcpy(buffer + pos, &parent->router_address, sizeof(uip_ipaddr_t));
        pltfrm_byte_memcpy(buffer, pos, &parent->router_address, 0,
                           BYTE_UNITS(sizeof(uip_ipaddr_t)));
        //pos+=sizeof(uip_ipaddr_t);
        pos += BYTE_UNITS(sizeof(uip_ipaddr_t));
        if (instance->mop == RPL_MOP_NON_STORING)
        {
            PRINTF("RPL: Sending DAO with prefix ");PRINT6ADDR(prefix);PRINTF(" to ");PRINT6ADDR(&dag->dag_id);PRINTF("\n");

            //DAOs must be sent to dodag root
            /*
             * RFC 6550. Section 6.4:  In Non-Storing mode, the DAO message is unicast to the DODAG root
             */
            UIP_STAT_TX_DAO_NUM_INC;
            RPL_ICMP6_SEND(&dag->dag_id, RPL_CODE_DAO, pos);
        }
        else
        {
            PRINTF("RPL: Sending DAO with prefix ");PRINT6ADDR(prefix);PRINTF(" to ");PRINT6ADDR(rpl_get_parent_ipaddr(parent));PRINTF("\n");
            if (rpl_get_parent_ipaddr(parent) != NULL)
            {
                UIP_STAT_TX_DAO_NUM_INC;
                RPL_ICMP6_SEND(rpl_get_parent_ipaddr(parent), RPL_CODE_DAO,
                               pos);
            }
        }
    }
    else
    {
        PRINTF("RPL: DIO received from parent (router) had no router address flag, thus no valid router's global address");
    }
}
/*---------------------------------------------------------------------------*/
#if !RPL_SUPPRESS_DAO_ACK_CONF

#if RPL_CONF_DAO_ACK //Will always enable the DAO ACK flag for DAOs. Need the code for DAO ACK input
static void
dao_ack_input(void)
{
//#if DEBUG
    unsigned char *buffer;
    //uint16_t buffer_length;
    uint16_t instance_id;
    uint16_t sequence;
    //uint16_t status;
    rpl_instance_t *instance;

    buffer = UIP_ICMP_PAYLOAD;
    //buffer_length = rpl_ip_len - BYTE_UNITS(uip_l3_icmp_hdr_len);

    instance_id = get8(buffer,0);//buffer[0];
    sequence = get8(buffer,2);//buffer[2];
    //status = get8(buffer,3);//buffer[3];

    PRINTF("RPL: Received a DAO ACK with sequence number %d from ",
            sequence);
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("\n");

    instance = rpl_get_instance(instance_id);
    if (instance != NULL)
    {
        if (instance->dao_sequence == sequence)
        {
            //We can stop the dao ack timer
            rpl_stop_dao_ack_timer(instance);
            instance->dao_ack_tries = 0;
        }
    }
//#endif /* DEBUG */
}
#endif //RPL_CONF_DAO_ACK
/*---------------------------------------------------------------------------*/
void
dao_ack_output(rpl_instance_t *instance, uip_ipaddr_t *dest, uint16_t sequence)
{
    unsigned char *buffer;

    PRINTF("RPL: Sending a DAO ACK with sequence number %d to ", sequence);
    PRINT6ADDR(dest);
    PRINTF("\n");

    buffer = UIP_ICMP_PAYLOAD;

    set8(buffer, 0, instance->instance_id); //buffer[0] = instance->instance_id;
    set8(buffer, 1, 0);//buffer[1] = 0;
    set8(buffer, 2, sequence);//buffer[2] = sequence;
    set8(buffer, 3, 0);//buffer[3] = 0;

    UIP_STAT_TX_DAO_ACK_NUM_INC;

    RPL_ICMP6_SEND(dest, RPL_CODE_DAO_ACK, 4);
}
#endif //RPL_SUPPRESS_DAO_ACK_CONF
/*---------------------------------------------------------------------------*/
void uip_rpl_input(void)
{
    //PRINTF("Received an RPL control message\n");
    switch (UIP_ICMP_BUF->icode)
    {
    case RPL_CODE_DIO:
        dio_input();
        UIP_STAT_RX_DIO_NUM_INC;
        break;
#if !RPL_SUPPRESS_DIS_CONF
        case RPL_CODE_DIS:
        dis_input();
        UIP_STAT_RX_DIS_NUM_INC;
        break;
#endif
#if ((RPL_MOP_DEFAULT == RPL_MOP_STORING_NO_MULTICAST) || NON_STORING_MODE_ROOT)
        //We only input DAOs if we are Regular nodes or Root for Storing mode OR the Root for Non storing mode
    case RPL_CODE_DAO:
        dao_input();
        UIP_STAT_RX_DAO_NUM_INC;
        break;
#endif
#if RPL_CONF_DAO_ACK
        case RPL_CODE_DAO_ACK:
        dao_ack_input();
        UIP_STAT_RX_DAO_ACK_NUM_INC;
        break;
#endif
    default:
        UIP_STAT_RX_ICMP_DROP_NUM_INC;
        PRINTF("RPL: received an unknown ICMP6 code (%u)\n", UIP_ICMP_BUF->icode);
        break;
    }

    rpl_ip_len= 0;
}

/*---------------------------------------------------------------------------*/
#if NON_STORING_MODE_ROOT
#define UIP_ICMP6_ERROR_BUF  ((struct uip_icmp6_error *)&rpl_ip_buf[uip_l2_l3_icmp_hdr_len])

void rpl_icmp6_destination_unreachable_input()
{
    //TODO: Study and develop the most appropriate behavior when receiving this types of errors
    PRINTF("ICMPv6: Destination unreachable received\n");
    if (UIP_ICMP_BUF->icode == ICMP6_DST_UNREACH_ERRORSHR)
    {
        PRINTF("ICMPv6: Code 7. Node not reachable\n");
        memmove((void *)UIP_IP_BUF, (char *)UIP_ICMP6_ERROR_BUF + rpl_ip_ext_len + UIP_ICMP6_ERROR_LEN, CHAR_BIT_UNITS(rpl_ip_len) - UIP_IPICMPH_LEN - rpl_ip_ext_len - UIP_ICMP6_ERROR_LEN);

        if (UIP_IP_BUF->proto == UIP_PROTO_ROUTING)
        {
            uip_ds6_route_sourceRouting_t* route = uip_ds6_sourceRouting_route_lookup(&UIP_IP_BUF->destipaddr);
            if (route)
            {
                PRINTF("ICMPv6: Erasing node in routing table, with address: ");
                PRINT6ADDR(&UIP_IP_BUF->destipaddr);
                PRINTF("\n");
                uip_ds6_route_sourceRouting_rm(route);
            }
        }
    }
}
#endif

int rpl_ipv6_icmp6_input(uint16_t type)
{
    if (type == ICMP6_RPL)
    {
        uip_rpl_input();
    }
#if NON_STORING_MODE_ROOT
    else if (type == ICMP6_DST_UNREACH)
    {
        rpl_icmp6_destination_unreachable_input();
    }
#endif
    else
    {
        return 0;
    }
    return 1;
}

void candidate_add(uip_ipaddr_t *from, rpl_dio_t *dio)
{
    //clean up the table first
    uint8_t i;

    for (i = 0; i < DAO_MAX_TARGET - 1; ++i)
    {
        uip_ds6_nbr_t *nbr = uip_ds6_nbr_lookup(&candidate_addr[i]);
        if (nbr == NULL || nbr->state != NBR_REACHABLE)
        {
            memset(&candidate_addr[i], 0, sizeof(uip_ipaddr_t));
            candidate_rank[i] = 0;
        }
    }

    int8_t selected = -1;
    for (i = 0; i < (DAO_MAX_TARGET - 1); ++i)
    {
        if (candidate_rank[i] == 0)
        { //empty item
            selected = i;
        }
        else if (memcmp(&candidate_addr[i], from, sizeof(uip_ipaddr_t)) == 0)
        { //duplicate
            selected = i;
        }
    }

    if (selected == -1)
    {
        //no empty slots and is not a duplicate. find one with largest rank
        selected = 0;
        for (i = 1; i < (DAO_MAX_TARGET - 1); ++i)
        {
            if (candidate_rank[i] > candidate_rank[selected])
                selected = i;
        }
    }
    memcpy(&candidate_addr[selected], from, sizeof(uip_ipaddr_t));
    candidate_rank[selected] = dio->rank;
}

#endif /* UIP_CONF_IPV6 */
