/******************************************************************************
*  Filename:       rf_patch_rfe_genook.c
*
*  Description: RF core patch for General OOK support in CC13x4 and CC26x4.
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
#include "rf_patch_rfe_genook.h"
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

RFE_PATCH_TYPE patchGenookRfe[422] = {
   0x00006140,
   0x11011000,
   0x004d1203,
   0x002e24f1,
   0x0a940018,
   0x003ffffe,
   0x00ff007f,
   0x07fe03ff,
   0x000007ff,
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
   0x00000000,
   0x00000000,
   0x40300000,
   0x40014000,
   0x40074003,
   0x404f400f,
   0x41cf40cf,
   0x47cf43cf,
   0x3fcf4fcf,
   0x1fcf2fcf,
   0x00000fcf,
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
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0xc0500000,
   0x70009100,
   0x9100c070,
   0xc0507000,
   0x70009150,
   0x9150c070,
   0x31827000,
   0x91310021,
   0x7000b110,
   0x00213182,
   0xb1609181,
   0x81017000,
   0xa100b101,
   0x91323182,
   0x9101b110,
   0x81411011,
   0x407d2241,
   0x700006f1,
   0xc0501025,
   0xc3f49100,
   0x1420c2b0,
   0x10316f03,
   0xc0220441,
   0x00213182,
   0xb1109131,
   0x10313963,
   0xc0820441,
   0x00213182,
   0xb1109131,
   0x10313963,
   0x3182c0a2,
   0x91310021,
   0x1050b110,
   0x14053115,
   0x70009255,
   0xc2b26462,
   0x06311031,
   0x02c13161,
   0xc112646b,
   0x39211031,
   0x31510671,
   0x646b02e1,
   0x64627000,
   0x7100b054,
   0xb064a054,
   0x220080f0,
   0x808140b4,
   0x1e5106f1,
   0xc11140ca,
   0x646bc122,
   0x82b1645f,
   0x39813181,
   0x646bc0e2,
   0x68c9c300,
   0x12406462,
   0xb03290b0,
   0x395382a3,
   0x64a43953,
   0x68d3c360,
   0x90b01280,
   0x7000b032,
   0xc1016462,
   0x646bc122,
   0xc0a2c101,
   0xc1016470,
   0x646bc0c2,
   0x64a482a3,
   0x90b012c0,
   0x7000b032,
   0xc2726462,
   0x646bc081,
   0xc111c122,
   0xc002646b,
   0x646bc111,
   0xc331c062,
   0xc362646b,
   0x646bc111,
   0xc111c302,
   0x82a3646b,
   0x64a43953,
   0x6475c3e2,
   0x40fe2211,
   0xc881c242,
   0xc252646b,
   0x646bc111,
   0xcee1c272,
   0xc202646b,
   0x646bc881,
   0xc801c202,
   0xc170646b,
   0x70006912,
   0xc2426462,
   0x646bc801,
   0xc011c252,
   0xc272646b,
   0x646bc0e1,
   0xc101c002,
   0xc062646b,
   0x646bc301,
   0xc101c122,
   0xc362646b,
   0x646bc101,
   0xc101c302,
   0x82a3646b,
   0x700064a4,
   0x6745cff0,
   0x82b16465,
   0x39813181,
   0x6470c182,
   0xc1116468,
   0x6470c0a2,
   0x00007000,
   0x00000000,
   0x72057306,
   0x720e720b,
   0x7100b050,
   0xb0608081,
   0x8092a050,
   0x224180a2,
   0x80804561,
   0x0410c1f1,
   0x11011630,
   0x6c011401,
   0x617c617c,
   0x617c617c,
   0x617c617c,
   0x617c617c,
   0x617c617c,
   0x617c617c,
   0x809162fb,
   0x0421c0f2,
   0x80823121,
   0x14122a42,
   0x11011632,
   0x6c011421,
   0x6199618c,
   0x62fb618c,
   0x631d617d,
   0x617c617c,
   0x6199618c,
   0x62fb618c,
   0x631d617d,
   0x617c617c,
   0x64e86181,
   0x65146599,
   0x12106181,
   0x618490b0,
   0x9050c010,
   0x906078a0,
   0x1210720e,
   0x61449030,
   0x91a07850,
   0x92607860,
   0x92707870,
   0x92807880,
   0x92907890,
   0x90a0c1a0,
   0xa0bc6181,
   0xb060a0e1,
   0x80f0a054,
   0x45a42250,
   0x22008040,
   0x619b46c8,
   0x6745cfe0,
   0x393080f0,
   0x22100630,
   0x784141ad,
   0x220061b2,
   0x783141b1,
   0x782161b2,
   0x827d91e1,
   0x39408280,
   0x0410c0f1,
   0xc0121007,
   0x82693072,
   0x0419c0f1,
   0xc0f1826a,
   0x041a394a,
   0xc0f1826e,
   0x041e398e,
   0x10bc10ab,
   0x648210c2,
   0x78e7c00f,
   0xb003b013,
   0xb0536669,
   0xb013b050,
   0xc082663f,
   0x665066a5,
   0xc020c00f,
   0xb0637100,
   0x710069d8,
   0x8041b063,
   0x46c82201,
   0x225080f0,
   0x221041db,
   0x22f045db,
   0xc08246e6,
   0x10f066a5,
   0x92103920,
   0x666f8223,
   0x18d31030,
   0x0bf34df9,
   0x1ce31613,
   0x92134a0d,
   0x143b8233,
   0x1ce361fe,
   0x92134a0d,
   0x183b8233,
   0x4e4c1cab,
   0x4a4e1c9b,
   0x420d1cbc,
   0x826010b2,
   0x420b22d0,
   0x221080f0,
   0x6482460d,
   0x61db660e,
   0x18b010c0,
   0x10034a20,
   0x1e003910,
   0x380f4217,
   0x22033807,
   0x10f04230,
   0x180f3920,
   0x39201070,
   0x62301807,
   0xc0011003,
   0x39111801,
   0x42281e01,
   0x3017301f,
   0x42302203,
   0x392010f0,
   0x1070140f,
   0x14073920,
   0x828066bf,
   0x311006f0,
   0x393180f1,
   0x1e010631,
   0x3810423a,
   0x7100b063,
   0x10bc6a3a,
   0x82807000,
   0x311006f0,
   0x393180f1,
   0x1e010631,
   0x38104248,
   0x7100b063,
   0x70006a48,
   0x620210ab,
   0x6202109b,
   0x392210f2,
   0x82229212,
   0x18128251,
   0x181281a1,
   0x31818291,
   0x1c123d81,
   0xb0e14a61,
   0xcfd0b032,
   0xc7f16745,
   0x4e651421,
   0x91b2c812,
   0xb03191c2,
   0x12087000,
   0xc800c006,
   0x91c091b0,
   0x82517000,
   0x18101030,
   0x181081a1,
   0x80e11406,
   0x31828292,
   0x1c203d82,
   0x22114a81,
   0xb0e1468b,
   0xcfc0b032,
   0x22116745,
   0x1a32428b,
   0x4e8b1c20,
   0xb032a0e1,
   0x9342dfb0,
   0x82816745,
   0x06f13941,
   0x42961e01,
   0xc0101618,
   0x1c083010,
   0x3c1646a4,
   0x1461c7f1,
   0xc8164e9a,
   0x81c191b6,
   0x3d813181,
   0x4aa11c16,
   0xb03191c6,
   0x1208c006,
   0x82037000,
   0x10f13123,
   0x3c211871,
   0x4ead1e11,
   0x1005c011,
   0x4eb11c37,
   0x103762b3,
   0x141762b4,
   0x4ab71c3f,
   0x103f62b9,
   0x1e0162bf,
   0x121146bc,
   0x42bf1e0f,
   0x10f1181f,
   0x39311471,
   0x063080e0,
   0x14103121,
   0x700090e0,
   0x81b28251,
   0x3d823182,
   0x9341efa0,
   0x67459352,
   0x6514a003,
   0x81b16181,
   0x3d813181,
   0x39808290,
   0x1cf11801,
   0x14014ae5,
   0x22c080b0,
   0xb0bc46e5,
   0xef90b033,
   0x935f9341,
   0x70006745,
   0xb063a003,
   0xb054b064,
   0x6514b0e0,
   0x80407100,
   0x46c82200,
   0x64e8b064,
   0x7100a0e0,
   0x22008040,
   0xb06446c8,
   0xb003a054,
   0xcf8061db,
   0x64b36745,
   0xb054b0e1,
   0x6462b050,
   0x7100b064,
   0x80f0b064,
   0x47402240,
   0x1e118081,
   0x22104340,
   0x6319430e,
   0xc0c2c201,
   0x80a0646b,
   0x39403180,
   0xc1016b14,
   0x646bc0c2,
   0xc3316303,
   0x646bc0c2,
   0xcf706303,
   0x65306745,
   0xb0e164b3,
   0xb050b054,
   0xb0646468,
   0xb0647100,
   0x224080f0,
   0x80814740,
   0x43401e11,
   0x43312210,
   0xc401633c,
   0x6470c162,
   0x318080a0,
   0x6b373940,
   0xc162c301,
   0x63266470,
   0xc162c771,
   0x63266470,
   0xcf60a054,
   0x64d86745,
   0x93306181,
   0x22008320,
   0xb3104746,
   0x00007000
};

PATCH_FUN_SPEC void rf_patch_rfe_genook(void)
{
#ifdef __PATCH_NO_UNROLLING
   uint32_t i;
   for (i = 0; i < 422; i++) {
      HWREG(RFC_RFERAM_BASE + 4 * i) = patchGenookRfe[i];
   }
#else
   const uint32_t *pS = patchGenookRfe;
   volatile unsigned long *pD = &HWREG(RFC_RFERAM_BASE);
   uint32_t t1, t2, t3, t4, t5, t6, t7, t8;
   uint32_t nIterations = 52;

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
   t6 = *pS++;
   *pD++ = t1;
   *pD++ = t2;
   *pD++ = t3;
   *pD++ = t4;
   *pD++ = t5;
   *pD++ = t6;
#endif
}
