/******************************************************************************
*  Filename:       rf_patch_mce_ieee_cohr.c
*
*  Description: RF core patch for IEEE 802.15.4 support ("IEEE" API command set) in CC13x4 and CC26x4
*
*  Copyright (c) 2021-2022, Texas Instruments Incorporated
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*
*  1) Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2) Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*
*  3) Neither the name of the ORGANIZATION nor the names of its contributors may
*     be used to endorse or promote products derived from this software without
*     specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
*  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
*  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
*  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
*  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
*  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
*  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#include <stdint.h>
#include "rf_patch_mce_ieee_cohr.h"
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_types.h)

#ifndef MCE_PATCH_TYPE
#define MCE_PATCH_TYPE static const uint32_t
#endif

#ifndef PATCH_FUN_SPEC
#define PATCH_FUN_SPEC
#endif

#ifndef RFC_MCERAM_BASE
#define RFC_MCERAM_BASE 0x21008000
#endif

MCE_PATCH_TYPE patchIeeeCohrMce[339] = { 
   0xf70360c8,
   0xc39b9b3a,
   0x2fca744a,
   0x079d0fcf,
   0x7f7f01e0,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x000000a7,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000003,
   0x0000001f,
   0x80000000,
   0x0004000c,
   0x000114c4,
   0x00000009,
   0x018a8000,
   0x0f900000,
   0x121d002e,
   0x00000a11,
   0x00000b60,
   0x00404010,
   0x001c0000,
   0x041e1e1e,
   0x00008004,
   0x00003c00,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x40200209,
   0x0000001f,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x10000000,
   0x000000a7,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000003,
   0x0000001f,
   0x80000000,
   0x0004000c,
   0x00010000,
   0x00000009,
   0x018a8000,
   0x0fb00000,
   0x111e002e,
   0x00000a11,
   0x00000b60,
   0x00404010,
   0x001c0000,
   0x041e1e1e,
   0x00008004,
   0x00003200,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x40200209,
   0x0000001f,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x10000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x720d7223,
   0x720f720e,
   0x72687210,
   0x7203a35d,
   0x73057204,
   0x73967306,
   0xc7c07296,
   0xb0d09010,
   0x669fc030,
   0xa0d07100,
   0x721bb110,
   0x10208162,
   0x06703952,
   0x16300020,
   0x14011101,
   0x60f76c01,
   0x61236108,
   0x6192615a,
   0x60f760f7,
   0x60f760f7,
   0x61236103,
   0x6192615a,
   0x60f760f7,
   0x60fa60f7,
   0x60fb1220,
   0x73111210,
   0x73137312,
   0x001081b1,
   0xb07091b0,
   0xc14160d7,
   0xc680c192,
   0x610c1820,
   0xc192c641,
   0x1820c680,
   0x6e236f13,
   0x16121611,
   0xc682690c,
   0x1820ca10,
   0x12034119,
   0x16126e23,
   0x78806916,
   0xc1409cd0,
   0x83309ce0,
   0x12209d00,
   0xb68c99e0,
   0x729660fa,
   0x91f0c130,
   0x1213b113,
   0xb0e8b128,
   0xb1287100,
   0x9233a0e8,
   0x1e008cd0,
   0x99804136,
   0xb111b960,
   0x7100b0d1,
   0xb002b012,
   0xb013b683,
   0xb111b003,
   0x7296a0d1,
   0x7100b0d3,
   0xa0d3b113,
   0xc0001000,
   0xc0209960,
   0xb0d19980,
   0x9960c030,
   0xb1117100,
   0x7296a0d1,
   0xa0037268,
   0x7268a002,
   0x73067305,
   0x72967396,
   0xc7c0a230,
   0x60fa9010,
   0x94c07810,
   0x960095e0,
   0x96409620,
   0x94d07820,
   0x961095f0,
   0x96509630,
   0x95c07830,
   0x95d07840,
   0x8240b68b,
   0x45732230,
   0x7100b0d5,
   0xa0d5b115,
   0x7296616b,
   0xc030a3a7,
   0xb00291e0,
   0xb006b004,
   0xb121b00d,
   0x7100b0e1,
   0xb00ca0e1,
   0xa00cb01c,
   0x22808160,
   0x78604187,
   0x78506188,
   0x90309050,
   0x90607870,
   0xb1219040,
   0x7100b0e1,
   0xa0e1b072,
   0xb01ca00c,
   0x8d00b08e,
   0x396213f2,
   0x83310420,
   0x315213f2,
   0x04213152,
   0x93300010,
   0xa2301001,
   0x31808460,
   0x31818461,
   0x00103981,
   0x81959450,
   0x120e06f5,
   0x12061217,
   0x944012f0,
   0x9440c1c0,
   0xb011b472,
   0xae91ae90,
   0x12041203,
   0x120a1208,
   0xa3a5a3a7,
   0x69bdc480,
   0xc300b3a5,
   0xb3a769c0,
   0xc900a3a5,
   0xb3a569c4,
   0x69c7c600,
   0x22508230,
   0xb23545ce,
   0x69cdc0f0,
   0xb0d5b115,
   0xb14cb10c,
   0xb0737100,
   0x22508090,
   0xa0d545ba,
   0xa472b230,
   0xb14cb441,
   0xc1a0a10c,
   0xb121b0e1,
   0x69df7100,
   0xb00ca0e1,
   0xbe91bea1,
   0x669fc040,
   0xb07e7100,
   0xbe92ae91,
   0x7100bea2,
   0x8e60bbce,
   0x31203180,
   0x3d203d80,
   0x31401001,
   0x3d201810,
   0x396213f2,
   0x83310420,
   0x14100421,
   0x0bf20420,
   0x04218331,
   0x93301410,
   0xbe91bea1,
   0x7100ae92,
   0x7100bea1,
   0xbea1b07f,
   0x8e5fbbce,
   0x42291e36,
   0x42341e0f,
   0x421a1e16,
   0x06f08190,
   0x45921c0f,
   0x62341216,
   0x39408190,
   0x1c0f06f0,
   0x12054592,
   0x12361202,
   0x9070c300,
   0x669fc050,
   0x94507890,
   0x16476234,
   0x818091cf,
   0x42341e00,
   0x4a421c70,
   0x1c701a40,
   0xa2354e34,
   0xbea17100,
   0x8090b07f,
   0x44dc2200,
   0xb0fcb13c,
   0x7100ae91,
   0xa0fcbe91,
   0x620c6654,
   0x669fc060,
   0x7203a235,
   0x73057204,
   0xa0027306,
   0xa006a004,
   0x73057268,
   0x73967306,
   0xc7c07296,
   0x60fa9010,
   0x8e5fbbce,
   0x1e8b10fb,
   0x1a8b4a5a,
   0xc080c0cc,
   0x163018b0,
   0x14011101,
   0x908c6c01,
   0x908c908c,
   0x908c908c,
   0x908c908c,
   0x0000908c,
   0xba8e0000,
   0x86791000,
   0x1000b082,
   0x8673ba8e,
   0xb083b083,
   0xba8e1000,
   0x1e8f8674,
   0x1c394a80,
   0x1c494e87,
   0x31184e8a,
   0x7000311a,
   0x4a871c39,
   0x4a8a1c49,
   0x311a3118,
   0x33187000,
   0x628c311a,
   0x331a3118,
   0x89de99c8,
   0x4e9a1e8e,
   0x89de99ca,
   0x4e951e8e,
   0xc0087000,
   0xb085c00a,
   0x7000b080,
   0xc00ac008,
   0xb071b084,
   0x9a507000,
   0x22008a40,
   0xba3046a0,
   0x00007000
};

PATCH_FUN_SPEC void rf_patch_mce_ieee_cohr(void)
{
#ifdef __PATCH_NO_UNROLLING
   uint32_t i;
   for (i = 0; i < 339; i++) {
      HWREG(RFC_MCERAM_BASE + 4 * i) = patchIeeeCohrMce[i];
   }
#else
   const uint32_t *pS = patchIeeeCohrMce;
   volatile unsigned long *pD = &HWREG(RFC_MCERAM_BASE);
   uint32_t t1, t2, t3, t4, t5, t6, t7, t8;
   uint32_t nIterations = 42;

   do {
      t1 = *pS++;
      t2 = *pS++;
      t3 = *pS++;
      t4 = *pS++;
      t5 = *pS++;
      t6 = *pS++;
      t7 = *pS++;
      t8 = *pS++;
      *pD++ = t1;
      *pD++ = t2;
      *pD++ = t3;
      *pD++ = t4;
      *pD++ = t5;
      *pD++ = t6;
      *pD++ = t7;
      *pD++ = t8;
   } while (--nIterations);

   t1 = *pS++;
   t2 = *pS++;
   t3 = *pS++;
   *pD++ = t1;
   *pD++ = t2;
   *pD++ = t3;
#endif
}