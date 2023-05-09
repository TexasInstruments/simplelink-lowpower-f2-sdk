/******************************************************************************
*  Filename:       rf_patch_rfe_sun_oqpsk.c
*
*  Description: RF core patch for IEEE 802.15.4g SUN OQPSK support in CC13x4 and CC26x4.
*
*  Copyright (c) 2023, Texas Instruments Incorporated
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
#include "rf_patch_rfe_sun_oqpsk.h"
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

#ifndef RFE_PATCH_MODE
#define RFE_PATCH_MODE 0
#endif

RFE_PATCH_TYPE patchSunOqpskRfe[357] = { 
   0x13076153,
   0x1f18d240,
   0x0ab03f13,
   0xff07a000,
   0x7000ffc0,
   0x60107010,
   0x40105010,
   0x40134011,
   0x401f4017,
   0x40df405f,
   0x43df41df,
   0x4fdf47df,
   0x2fdf3fdf,
   0x0fdf1fdf,
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
   0x402f2241,
   0x700006f1,
   0x9150c050,
   0xc0707000,
   0x70009150,
   0x00213182,
   0xb1609181,
   0x10257000,
   0x9100c050,
   0xc090c3f4,
   0x6f031420,
   0x06f11031,
   0x22f082a0,
   0x2651404c,
   0x3182c022,
   0x91310021,
   0x1031b110,
   0x06c13921,
   0x39639241,
   0x04411031,
   0x3182c082,
   0x91310021,
   0x3963b110,
   0xc0a21031,
   0x00213182,
   0xb1109131,
   0x31151050,
   0x92551405,
   0x641f7000,
   0x1031c2b2,
   0x31610631,
   0x642202c1,
   0x1031c112,
   0x06713921,
   0x02e13151,
   0x70006422,
   0x82b1641c,
   0x39813181,
   0x6422c0e2,
   0xc111641f,
   0x6422c122,
   0x6883c470,
   0xc0c2c111,
   0x649c6422,
   0x700064af,
   0x82b16434,
   0x39813181,
   0x643ac182,
   0xc1116437,
   0x643ac0a2,
   0x6895c470,
   0xc162c331,
   0x649c643a,
   0x700064af,
   0xb054b050,
   0x80407100,
   0x44a92240,
   0x409c2200,
   0x8081b060,
   0x449c1e11,
   0xa0547000,
   0x80f0b064,
   0x409c2200,
   0x12407000,
   0xb03290b0,
   0x395382a3,
   0x64693953,
   0x68b7c2f0,
   0xc1f18080,
   0xc1510410,
   0x40c31c10,
   0xc221641f,
   0x6422c0c2,
   0x643760c7,
   0xc162c441,
   0x8090643a,
   0x44cb3950,
   0x68cbce00,
   0x90b01280,
   0x7000b032,
   0xc201641f,
   0x6422c0c2,
   0x318080a0,
   0x68d73940,
   0xc0c2c101,
   0xc1016422,
   0x6422c122,
   0x646982a3,
   0x90b012c0,
   0x7000b032,
   0xc4016437,
   0x643ac162,
   0x318080a0,
   0x68eb3940,
   0xc162c301,
   0xc101643a,
   0x643ac0a2,
   0x646982a3,
   0x90b012c0,
   0x7000b032,
   0xc272641f,
   0x6422c081,
   0xc111c122,
   0xc0026422,
   0x6422c111,
   0xc331c062,
   0xc3626422,
   0x6422c111,
   0xc111c302,
   0x82a36422,
   0x64693953,
   0x6427c3e2,
   0x410e2211,
   0xc881c242,
   0xc2526422,
   0x6422c111,
   0xcee1c272,
   0xc2026422,
   0x6422c881,
   0xc801c202,
   0xc1706422,
   0x70006922,
   0xc242641f,
   0x6422c801,
   0xc011c252,
   0xc2726422,
   0x6422c0e1,
   0xc101c002,
   0xc0626422,
   0x6422c301,
   0xc101c122,
   0xc3626422,
   0x6422c101,
   0xc101c302,
   0x82a36422,
   0x70006469,
   0x1e118081,
   0xb050414e,
   0x7100b054,
   0x22408040,
   0xa054414f,
   0x80f1b064,
   0x45432201,
   0x22007000,
   0xb0604143,
   0x73066140,
   0x720b7205,
   0xb050720e,
   0x80817100,
   0xa050b060,
   0x22418092,
   0x80804573,
   0x0410c1f1,
   0x11011630,
   0x6c011401,
   0x61856185,
   0x61856185,
   0x61856185,
   0x61856185,
   0x61856185,
   0x61856185,
   0x80916185,
   0x312106f1,
   0x2a428082,
   0x16321412,
   0x14211101,
   0x619a6c01,
   0x619a6185,
   0x618e6186,
   0x6185618a,
   0x61926185,
   0x65406478,
   0x619264d0,
   0x6540648a,
   0x619264e4,
   0x65b364f8,
   0x61926524,
   0x90b01210,
   0x7306720e,
   0x12107205,
   0x61579030,
   0x91a0c3d0,
   0x92607820,
   0x92707830,
   0x92807840,
   0x92907850,
   0x92b07860,
   0x91e07810,
   0x90a0c1f0,
   0x39508260,
   0x0410c1f1,
   0x31101001,
   0x92501410,
   0xa0bc6192,
   0xb0e3a0e2,
   0x80f0a054,
   0x45be2250,
   0x22008040,
   0x61b646c0,
   0xa040a0e3,
   0x318d827d,
   0x8260398d,
   0x0410c1f1,
   0x826a1009,
   0x041a395a,
   0x39808260,
   0x06f03920,
   0x10ab100e,
   0x10c210bc,
   0x722e643f,
   0x66c3cff0,
   0xb003b013,
   0xb0536664,
   0xb054b050,
   0xb013b064,
   0x664d6646,
   0x80417100,
   0x46c02201,
   0x221080f0,
   0x22f045f0,
   0xb06446ad,
   0x41e02231,
   0x666ab063,
   0x61e06611,
   0x81bfb064,
   0x3d8f318f,
   0x934fdfe0,
   0x710066c3,
   0x22018041,
   0xb06446c0,
   0x225080f0,
   0xb063420a,
   0x22e18261,
   0x666a4605,
   0x8261669e,
   0x45f722f1,
   0x61f76611,
   0x318181b1,
   0xdfd03d81,
   0x66c39341,
   0x820061b3,
   0x82239210,
   0xa2e082e7,
   0x4e2018d3,
   0x16130bf3,
   0x4a451ce3,
   0x82339213,
   0x6231143b,
   0x462c1cba,
   0x4a2c1e23,
   0x4e2c1ce3,
   0x2207b2e0,
   0xa2e0422c,
   0x62371a1b,
   0x4a451ce3,
   0x82339213,
   0x1cab183b,
   0x1c9b4e41,
   0x1cbc4a43,
   0x10b24245,
   0x22f08260,
   0x80f0423e,
   0x46452220,
   0x6646643f,
   0x10ab6245,
   0x109b6235,
   0x70006235,
   0x06f08280,
   0x7100b063,
   0x10bc6a48,
   0x82027000,
   0x82229212,
   0x18128251,
   0x181281a1,
   0x31818291,
   0x1c123d81,
   0xb0e24a5b,
   0xc7f1b032,
   0x4e5f1421,
   0x91b2c812,
   0xb03191c2,
   0x7000b0e1,
   0xc0061208,
   0x91b0c800,
   0x700091c0,
   0x82008251,
   0x82209210,
   0x81a11810,
   0x14061810,
   0x829280e1,
   0x3d823182,
   0x4a7e1c20,
   0x2221b0e2,
   0xb0324682,
   0x66c3cfc0,
   0x42822221,
   0xb032a0e2,
   0x39418281,
   0x1e0106f1,
   0x1618428f,
   0x3010c010,
   0x469d1c08,
   0x3c101060,
   0xc7f11006,
   0x4e931461,
   0x91b6c816,
   0x318181c1,
   0x1c163d81,
   0x91c64a9a,
   0xc006b031,
   0x70001208,
   0x318181b1,
   0x82903d81,
   0x18013980,
   0x4aac1cf1,
   0x80b01401,
   0x46ac22c0,
   0xb033b0bc,
   0xa0037000,
   0xb064b063,
   0x6524b0ef,
   0x80407100,
   0x46c02200,
   0x64f8b064,
   0x7100a0ef,
   0x22008040,
   0xb06446c0,
   0x61e0b003,
   0xa003a0e3,
   0x93307000,
   0x22008320,
   0xb31046c4,
   0x00007000
};

PATCH_FUN_SPEC void rf_patch_rfe_sun_oqpsk(void)
{
#ifdef __PATCH_NO_UNROLLING
   uint32_t i;
   for (i = 0; i < 357; i++) {
      HWREG(RFC_RFERAM_BASE + 4 * i) = patchSunOqpskRfe[i];
   }
#else
   const uint32_t *pS = patchSunOqpskRfe;
   volatile unsigned long *pD = &HWREG(RFC_RFERAM_BASE);
   uint32_t t1, t2, t3, t4, t5, t6, t7, t8;
   uint32_t nIterations = 44;

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
