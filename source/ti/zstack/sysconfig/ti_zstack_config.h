/*
 * Copyright (c) 2018, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 *                               ti_ti_zstack_config.h
 *
 *  Macros used to define a TI Z-Stack ZigBee device. New user defined
 *  options may be added to this file, as necessary.
 *
 */

#ifndef TI_ZSTACK_CONFIG_H
#define TI_ZSTACK_CONFIG_H

/* Default (primary) and secondary channel masks. Secondary mask is used when the
 * default channels fail.
 * Channels are defined in the following:
 *        11 - 26 : 2.4 GHz     0x07FFF800
 */
/* #define MAX_CHANNELS_24GHZ      0x07FFF800 */   /* All 2.4GHz Channels */
/* #define DEFAULT_CHANLIST        0x04000000 */   /* Channel 26 - 0x1A */
/* #define DEFAULT_CHANLIST        0x02000000 */   /* Channel 25 - 0x19 */
/* #define DEFAULT_CHANLIST        0x01000000 */   /* Channel 24 - 0x18 */
/* #define DEFAULT_CHANLIST        0x00800000 */   /* Channel 23 - 0x17 */
/* #define DEFAULT_CHANLIST        0x00400000 */   /* Channel 22 - 0x16 */
/* #define DEFAULT_CHANLIST        0x00200000 */   /* Channel 21 - 0x15 */
/* #define DEFAULT_CHANLIST        0x00100000 */   /* Channel 20 - 0x14 */
/* #define DEFAULT_CHANLIST        0x00080000 */   /* Channel 19 - 0x13 */
/* #define DEFAULT_CHANLIST        0x00040000 */   /* Channel 18 - 0x12 */
/* #define DEFAULT_CHANLIST        0x00020000 */   /* Channel 17 - 0x11 */
/* #define DEFAULT_CHANLIST        0x00010000 */   /* Channel 16 - 0x10 */
/* #define DEFAULT_CHANLIST        0x00008000 */   /* Channel 15 - 0x0F */
/* #define DEFAULT_CHANLIST        0x00004000 */   /* Channel 14 - 0x0E */
/* #define DEFAULT_CHANLIST        0x00002000 */   /* Channel 13 - 0x0D */
/* #define DEFAULT_CHANLIST        0x00001000 */   /* Channel 12 - 0x0C */
/* #define DEFAULT_CHANLIST        0x00000800 */   /* Channel 11 - 0x0B */

#ifndef DEFAULT_CHANLIST
#define DEFAULT_CHANLIST           0x00000800      /* Channel 11 - 0x0B */
#endif

#ifndef SECONDARY_CHANLIST
#define SECONDARY_CHANLIST         0x00000000
#endif

/* Define the default PAN ID.
 *
 * Setting this to a value other than 0xFFFF causes
 * ZDO_COORD to use this value as its PAN ID and
 * Routers and end devices to join PAN with this ID
 */
#ifndef ZDAPP_CONFIG_PAN_ID
#define ZDAPP_CONFIG_PAN_ID           0xFFFF
#endif

/* Specifiy whether to set the EPID to the Extended Address, or to use the
 * custom Extended Pan ID (EPID) defined by ZDAPP_CONFIG_EPID
 */
#ifndef SET_EPID_AS_EXT_ADDR
#define SET_EPID_AS_EXT_ADDR          TRUE
#endif

/* Define the default Extended Pan ID (EPID)
 * ZDAPP_CONFIG_EPID is only used if SET_EPID_AS_EXT_ADDR is FALSE. Otherwise,
 * the EPID is set to the Extended Address.
 */
#ifndef ZDAPP_CONFIG_EPID
#define ZDAPP_CONFIG_EPID             {0xEF,0xCD,0xAB,0x89,0x67,0x45,0x23,0x01}
#endif

/* Minimum number of milliseconds to hold off the start of the device
 * in the network and the minimum delay between joining cycles.
 */
#ifndef NWK_START_DELAY
#define NWK_START_DELAY               100
#endif

/* Mask for the random joining delay. This value is masked with
 * the return from osal_rand() to get a random delay time for
 * each joining cycle.  This random value is added to NWK_START_DELAY.
 * For example, a value of 0x007F will be a joining delay of 0 to 127
 * milliseconds.
 */
#ifndef EXTENDED_JOINING_RANDOM_MASK
#define EXTENDED_JOINING_RANDOM_MASK  0x007F
#endif

/* Minimum number of milliseconds to delay between each beacon request
 * in a joining cycle.
 */
#ifndef BEACON_REQUEST_DELAY
#define BEACON_REQUEST_DELAY          100
#endif

/* Mask for the random beacon request delay. This value is masked with the
 * return from osal_rand() to get a random delay time for each joining cycle.
 * This random value is added to DBEACON_REQUEST_DELAY. For example, a value
 * of 0x00FF will be a beacon request delay of 0 to 255 milliseconds.
 */
#ifndef BEACON_REQ_DELAY_MASK
#define BEACON_REQ_DELAY_MASK         0x00FF
#endif

/* Jitter mask for the link status report timer. This value is masked with the
 * return from osal_rand() to add a random delay to _NIB.nwkLinkStatusPeriod.
 * For example, a value of 0x007F allows a jitter between 0-127 milliseconds.
 */
#ifndef LINK_STATUS_JITTER_MASK
#define LINK_STATUS_JITTER_MASK       0x007F
#endif

/* in seconds; set to 255 to turn off route expiry */
#ifndef ROUTE_EXPIRY_TIME
#define ROUTE_EXPIRY_TIME             30
#endif

/* This number is used by polled devices, since the spec'd formula
 * doesn't work for sleeping end devices.  For non-polled devices,
 * a formula is used. Value is in 2 milliseconds periods
 */
#ifndef APSC_ACK_WAIT_DURATION_POLLED
#define APSC_ACK_WAIT_DURATION_POLLED 3000
#endif

/*  Default indirect message holding timeout value:
 *  1-65535 (0 -> 65536) X CNT_RTG_TIMER X RTG_TIMER_INTERVAL
 */
#ifndef NWK_INDIRECT_MSG_TIMEOUT
#define NWK_INDIRECT_MSG_TIMEOUT      7
#endif

/* The number of simultaneous route discoveries in network */
#ifndef MAX_RREQ_ENTRIES
#define MAX_RREQ_ENTRIES              8
#endif

/* The maximum number of retries allowed after a transmission failure */
#ifndef APSC_MAX_FRAME_RETRIES
#define APSC_MAX_FRAME_RETRIES        3
#endif

/* Max number of times retry looking for the next hop address of a message */
#ifndef NWK_MAX_DATA_RETRIES
#define NWK_MAX_DATA_RETRIES          2
#endif

/* The number of items in the broadcast table */
#ifndef MAX_BCAST
#define MAX_BCAST                     9
#endif

/* The maximum number of groups in the groups table */
#ifndef APS_MAX_GROUPS
#define APS_MAX_GROUPS                16
#endif

/* Number of entries in the regular routing table plus additional
 * entries for route repair
 */
#ifndef MAX_RTG_ENTRIES
#define MAX_RTG_ENTRIES               40
#endif

/* Maximum number of entries in the Binding table. */
#ifndef NWK_MAX_BINDING_ENTRIES
#define NWK_MAX_BINDING_ENTRIES       4
#endif

/* Default security key. */
#ifndef DEFAULT_KEY
#define DEFAULT_KEY                   {0}
#endif

/* Minimum transmissions attempted for Channel Interference detection,
 * Frequency Agility can be disabled by setting this parameter to zero.
 */
#ifndef ZDNWKMGR_MIN_TRANSMISSIONS
#define ZDNWKMGR_MIN_TRANSMISSIONS    20
#endif



/****************************************
 * The following are for End Devices only
 ***************************************/

/* Determine whether the radio on an End Device is always on */
#ifndef RFD_RCVC_ALWAYS_ON
#define RFD_RCVC_ALWAYS_ON            FALSE
#endif

/* The number of milliseconds to wait between data request polls to the coordinator. */
#ifndef POLL_RATE
#define POLL_RATE                     1000
#endif

/* This is used after receiving a data indication or data confirmation to poll immediately
 * for queued messages...in milliseconds.
 */
#ifndef QUEUED_POLL_RATE
#define QUEUED_POLL_RATE              100
#endif

/* This is used after receiving a data confirmation to poll immediately
 * for response messages...in milliseconds
 */
#ifndef RESPONSE_POLL_RATE
#define RESPONSE_POLL_RATE            100
#endif

/* This is used as an alternate response poll rate only for rejoin request.
 * This rate is determined by the response time of the parent that the device
 * is trying to join.
 */
#ifndef REJOIN_POLL_RATE
#define REJOIN_POLL_RATE              440
#endif

#endif /* ZSTACK_CONFIG_H */
