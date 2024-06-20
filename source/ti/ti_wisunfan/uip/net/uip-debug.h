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
 * $Id: uip-debug.h,v 1.1 2010/04/30 13:20:57 joxe Exp $
 */
/**
 * \file
 *         A set of debugging macros.
 *
 * \author Nicolas Tsiftes <nvt@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
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
#ifndef UIP_DEBUG_H  
#define UIP_DEBUG_H

#include "net/uip.h"
#include <stdio.h>
#include <stdarg.h>

#include "pltfrm_lib.h"

void uip_debug_ipaddr_print(const uip_ipaddr_t *addr);
void uip_debug_lladdr_print(const uip_lladdr_t *addr);

#define DEBUG_NONE      0
#define DEBUG_PRINT     1
#define DEBUG_ANNOTATE  2
#define DEBUG_FULL      DEBUG_ANNOTATE | DEBUG_PRINT

/* PRINTA will always print if the debug routines are called directly */
#ifdef __AVR__
#include <avr/pgmspace.h>
#define PRINTA(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTA(format, ...) printf(format, __VA_ARGS__)
#endif

#if (DEBUG) & DEBUG_ANNOTATE
#ifdef __AVR__
#define ANNOTATE(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define ANNOTATE(...) printf(__VA_ARGS__)
#endif
#else
#define ANNOTATE(...)
#endif /* (DEBUG) & DEBUG_ANNOTATE */

#if (DEBUG) & DEBUG_PRINT
#define PRINTF(...) pltfrm_debug(__VA_ARGS__)
#define PRINTFI(...) pltfrm_debug(__VA_ARGS__)
#define PRINTFO(...) pltfrm_debug(__VA_ARGS__)
#define PRINTF_DEC(m) pltfrm_debug_dec(m)
#define PRINTF_HEX(m) pltfrm_debug_hex(m)
#define PRINT6ADDR(addr) pltfrm_debug_address(addr)
#define PRINTLLADDR(lladdr) pltfrm_debug_lladdr(lladdr)
#ifdef LOG_INFO  
#undef LOG_INFO
#endif
#define LOG_INFO(...) pltfrm_debug(__VA_ARGS__)
#define LOG_INFO0(...) pltfrm_debug(__VA_ARGS__)
#define LOG_INFO1(...) pltfrm_debug(__VA_ARGS__)
#define LOG_INFO2(...) pltfrm_debug(__VA_ARGS__)
#define LOG_INFO3(...) pltfrm_debug(__VA_ARGS__)
#define LOG_INFO4(...) pltfrm_debug(__VA_ARGS__)
#define LOG_IPV6ADDR(addr) pltfrm_debug_address(addr)
#else
#define PRINTF(...)
#define PRINTFI(...)
#define PRINTFO(...)
#define PRINTF_DEC(m)
#define PRINTF_HEX(m)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(lladdr)
#ifdef LOG_INFO  
#undef LOG_INFO
#endif
#define LOG_INFO(...) 
#define LOG_INFO0(...) 
#define LOG_INFO1(...) 
#define LOG_INFO2(...) 
#define LOG_INFO3(...) 
#define LOG_INFO4(...) 
#define LOG_IPV6ADDR(...)
#endif
#endif 
