/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: mac.h,v 1.14 2010/10/03 20:37:32 adamdunkels Exp $
 */

/**
 * \file
 *         MAC driver header file
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __6LOWPAN_MAC_H__
#define __6LOWPAN_MAC_H__

typedef void (*mac_callback_t)(void *ptr, int status, int transmissions);

/**
 * The structure of a MAC protocol driver in Contiki.
 */
struct mac_driver
{
    char *name;

    /** Initialize the MAC driver */
    void (*init)(void);

    /** Send a packet from the Rime buffer  */
    void (*send)(mac_callback_t sent_callback, void *ptr);

    /** Callback for getting notified of incoming packet. */
    void (*input)(void);

    /** Turn the MAC layer on. */
    int (*on)(void);

    /** Turn the MAC layer off. */
    int (*off)(int keep_radio_on);

    /** Returns the channel check interval, expressed in clock_time_t ticks. */
    unsigned short (*channel_check_interval)(void);
};

/* Generic MAC return values. */
enum
{
    /**< The MAC layer transmission was OK. */
    MAC_TX_OK,

    /**< The MAC layer transmission could not be performed due to a
     collision. */
    MAC_TX_COLLISION,

    /**< The MAC layer did not get an acknowledgement for the packet. */
    MAC_TX_NOACK,

    /**< The MAC layer deferred the transmission for a later time. */
    MAC_TX_DEFERRED,

    /**< The MAC layer transmission could not be performed because of an
     error. The upper layer may try again later. */
    MAC_TX_ERR,

    /**< The MAC layer transmission could not be performed because of a
     fatal error. The upper layer does not need to try again, as the
     error will be fatal then as well. */
    MAC_TX_ERR_FATAL,
};

#endif /* __6LOWPAN_MAC_H__ */
