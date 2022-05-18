/******************************************************************************
*  Filename:       rf_patch_rfe_ieee_cohr.c
*
*  Description: RF core patch for IEEE 802.15.4 support ("IEEE" API command set) in CC13x4 and CC26x4
*
*  Copyright (c) 2022, Texas Instruments Incorporated
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
#include "rf_patch_rfe_ieee_cohr.h"
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_types.h)

#ifndef RFE_PATCH_TYPE
#define RFE_PATCH_TYPE static const uint32_t
#endif

#ifndef PATCH_FUN_SPEC
#define PATCH_FUN_SPEC
#endif

#ifndef RFC_RFERAM_BASE
#define RFC_RFERAM_BASE 0x2100C000
#endif

RFE_PATCH_TYPE patchIeeeCohrRfe[325] = { 
   0x163f614c,
   0x07ff07f7,
   0x70301240,
   0x60007000,
   0x40005000,
   0x40034001,
   0x400f4007,
   0x40cf404f,
   0x43cf41cf,
   0x4fcf47cf,
   0x2fcf3fcf,
   0x0fcf1fcf,
   0x9100c050,
   0xc0707000,
   0x70009100,
   0x00213182,
   0xb1109131,
   0x81017000,
   0xa100b101,
   0x91323182,
   0x9101b110,
   0x81411011,
   0x402b2241,
   0x700006f1,
   0x9150c050,
   0xc0707000,
   0x70009150,
   0x00213182,
   0xb1609181,
   0x10257000,
   0x9100c050,
   0xc050c3f4,
   0x6f031420,
   0x04411031,
   0x22f082a0,
   0x26514048,
   0x3182c022,
   0x91310021,
   0x3963b110,
   0x04411031,
   0x3182c082,
   0x91310021,
   0x3963b110,
   0xc0a21031,
   0x00213182,
   0xb1109131,
   0x31151050,
   0x92551405,
   0x641b7000,
   0x1031c2b2,
   0x31610631,
   0x641e02c1,
   0x1031c112,
   0x06713921,
   0x02e13151,
   0x7000641e,
   0x82b16418,
   0x39813181,
   0x641ec0e2,
   0xc111641b,
   0x641ec122,
   0x687bc470,
   0xc0c2c111,
   0x6494641e,
   0x700064a7,
   0x82b16430,
   0x39813181,
   0x6436c182,
   0xc1116433,
   0x6436c0a2,
   0x688dc470,
   0xc162c331,
   0x64946436,
   0x700064a7,
   0xb054b050,
   0x80407100,
   0x44a12240,
   0x40942200,
   0x8081b060,
   0x44941e11,
   0xa0547000,
   0x80f0b064,
   0x40942200,
   0x12407000,
   0xb03290b0,
   0x395382a3,
   0x64613953,
   0x68afc2f0,
   0xc1f18080,
   0xc1510410,
   0x40bb1c10,
   0xc221641b,
   0x641ec0c2,
   0x643360bf,
   0xc162c441,
   0x80906436,
   0x44c33950,
   0x68c3ce00,
   0x90b01280,
   0x7000b032,
   0xc201641b,
   0x641ec0c2,
   0x318080a0,
   0x68cf3940,
   0xc0c2c101,
   0xc101641e,
   0x641ec122,
   0x646182a3,
   0x90b012c0,
   0x7000b032,
   0xc4016433,
   0x6436c162,
   0x318080a0,
   0x68e33940,
   0xc162c301,
   0xc1016436,
   0x6436c0a2,
   0x646182a3,
   0x90b012c0,
   0x7000b032,
   0xc272641b,
   0x641ec081,
   0xc111c122,
   0xc002641e,
   0x641ec111,
   0xc331c062,
   0xc362641e,
   0x641ec111,
   0xc111c302,
   0x82a3641e,
   0x64613953,
   0x6423c3e2,
   0x41062211,
   0xc881c242,
   0xc252641e,
   0x641ec111,
   0xcee1c272,
   0xc202641e,
   0x641ec881,
   0xc801c202,
   0xc170641e,
   0x7000691a,
   0xc242641b,
   0x641ec801,
   0xc011c252,
   0xc272641e,
   0x641ec0e1,
   0xc101c002,
   0xc062641e,
   0x641ec301,
   0xc101c122,
   0xc362641e,
   0x641ec101,
   0xc101c302,
   0x82a3641e,
   0x70006461,
   0x1e118081,
   0xb0504146,
   0x7100b054,
   0x22408040,
   0xa0544147,
   0x80f1b064,
   0x453b2201,
   0x22007000,
   0xb060413b,
   0x00006138,
   0x72057306,
   0x720e720b,
   0x7100b050,
   0xb0608081,
   0x8092a050,
   0x9341eff0,
   0x66849352,
   0x45702241,
   0xc1f18080,
   0x16300410,
   0x14011101,
   0x61816c01,
   0x61816181,
   0x61816181,
   0x61826181,
   0x61866184,
   0x61816188,
   0x6281627e,
   0x0402c0f0,
   0x2a413132,
   0x16321412,
   0x14211101,
   0x61a66c01,
   0x61a6618a,
   0x6190618c,
   0x61816194,
   0x61986181,
   0x61986470,
   0x619864c8,
   0x619864f0,
   0x6198651c,
   0x619865b8,
   0x65386470,
   0x619864c8,
   0x65b864f0,
   0x6198651c,
   0x65386482,
   0x619864dc,
   0xdfe08082,
   0x66849342,
   0x90b01210,
   0x122061a1,
   0x730690b0,
   0x12107205,
   0x61509030,
   0x91a0c420,
   0x3110c120,
   0x1410c121,
   0x78409250,
   0xc1809260,
   0xc3f09270,
   0xc0f092b0,
   0x781090a0,
   0x619891e0,
   0x6684cfd0,
   0x395c826c,
   0x040cc1f0,
   0x398e826e,
   0x06fe392e,
   0x643b10c2,
   0x10cb827d,
   0xb0e3120a,
   0x80f0a054,
   0x45d02250,
   0x22008040,
   0x61c74673,
   0x663ea0e3,
   0xb053b013,
   0xb050b063,
   0xb064b054,
   0x80f3b003,
   0x41b82253,
   0x80417100,
   0x46732201,
   0x41e32241,
   0xb063b064,
   0x92108200,
   0x80f3822f,
   0x46712203,
   0x45ee1e0a,
   0x62056606,
   0x45f51e1a,
   0x1e2a6606,
   0x66344605,
   0x1e2a6205,
   0x660645fe,
   0x45fb1e3a,
   0x123a6634,
   0x6205c089,
   0x42002200,
   0x1e091a19,
   0x120a4e05,
   0x627161d9,
   0x18d310f3,
   0x0bf34e15,
   0x1ce31613,
   0x70004e0e,
   0x82339213,
   0x1e2a143b,
   0x123a4614,
   0x1e0a6222,
   0x1e2a461b,
   0x121a421b,
   0x1ce37000,
   0x70004e1e,
   0x9213161a,
   0x183b8233,
   0x39508260,
   0x0410c1f1,
   0x4e3a1c0b,
   0x04108260,
   0x4a3c1c0b,
   0x462f1cbc,
   0xa0e07000,
   0x10b210bc,
   0x7000643b,
   0x7100b063,
   0x7100b063,
   0x7000b063,
   0x622c100b,
   0x622c100b,
   0x78307827,
   0x1a101870,
   0x6e71c001,
   0x6a431617,
   0x12087827,
   0xc800c006,
   0x700091b0,
   0x10001000,
   0x825110f0,
   0x6d711810,
   0x18166d71,
   0x6e701406,
   0x78311617,
   0x465b1c17,
   0x16187827,
   0x466e1e88,
   0x3d301060,
   0x181081a1,
   0x225180f1,
   0x91b0426e,
   0x318181c1,
   0x1c103d81,
   0x91c04a6c,
   0x1278b031,
   0x10007000,
   0x664e1000,
   0xa0e361d9,
   0x81b28251,
   0x3d823182,
   0x9341efc0,
   0x66849352,
   0x7000a003,
   0x643b80a2,
   0xb0506198,
   0x61987100,
   0x83209330,
   0x46852200,
   0x7000b310
};

PATCH_FUN_SPEC void rf_patch_rfe_ieee_cohr(void)
{
#ifdef __PATCH_NO_UNROLLING
   uint32_t i;
   for (i = 0; i < 325; i++) {
      HWREG(RFC_RFERAM_BASE + 4 * i) = patchIeeeCohrRfe[i];
   }
#else
   const uint32_t *pS = patchIeeeCohrRfe;
   volatile unsigned long *pD = &HWREG(RFC_RFERAM_BASE);
   uint32_t t1, t2, t3, t4, t5, t6, t7, t8;
   uint32_t nIterations = 40;

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
   t4 = *pS++;
   t5 = *pS++;
   *pD++ = t1;
   *pD++ = t2;
   *pD++ = t3;
   *pD++ = t4;
   *pD++ = t5;
#endif
}