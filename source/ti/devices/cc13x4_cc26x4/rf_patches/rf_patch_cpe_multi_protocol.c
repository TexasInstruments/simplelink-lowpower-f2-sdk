/******************************************************************************
*  Filename:       rf_patch_cpe_multi_protocol.c
*
*  Description:    RF Core patch for all protocols in CC26x4
*
*  Copyright (c) 2015-2023, Texas Instruments Incorporated
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
//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <string.h>

#include "rf_patch_cpe_multi_protocol.h"

#ifndef CPE_PATCH_TYPE
#define CPE_PATCH_TYPE static const uint32_t
#endif

#ifndef SYS_PATCH_TYPE
#define SYS_PATCH_TYPE static const uint32_t
#endif

#ifndef PATCH_FUN_SPEC
#define PATCH_FUN_SPEC static
#endif

#ifndef _APPLY_PATCH_TAB
#define _APPLY_PATCH_TAB
#endif


CPE_PATCH_TYPE patchImageMultiProtocol[] = {
   0x210042fb,
   0x2100451f,
   0x210040e5,
   0x210045b9,
   0x210045c9,
   0x21004605,
   0x00009fbd,
   0x0000a5b5,
   0x00009eff,
   0x00009f37,
   0x0000a351,
   0x0000a4c9,
   0x0000a53b,
   0x0000a57d,
   0x0000a289,
   0x0000a2a1,
   0x0000a44d,
   0x21004119,
   0x4d06b570,
   0x21804806,
   0x6802b672,
   0x60014311,
   0x681c4b04,
   0xb6626002,
   0x47004803,
   0x21000294,
   0x40040000,
   0x40043004,
   0x00000385,
   0x4803b5f0,
   0x6800b08d,
   0x49023020,
   0x00004708,
   0x21000108,
   0x0002a3ad,
   0x460cb570,
   0x47084900,
   0x0002a6a5,
   0x4671b510,
   0x295c0a09,
   0x2100d101,
   0x2101e000,
   0xfa30f000,
   0x0000bd10,
   0x4607b5f8,
   0x47084900,
   0x00005895,
   0x4606b5f3,
   0x4a01b081,
   0x00004710,
   0x00005c61,
   0x4885b5f7,
   0x6840b084,
   0x25004616,
   0x2a009002,
   0x4c82d001,
   0x4c82e000,
   0x62622200,
   0x2e004881,
   0x4b7fd003,
   0x60036a5b,
   0x4b7ce004,
   0x2b006a5b,
   0x6002d100,
   0x7d584b7c,
   0xd00407c7,
   0x0fc00780,
   0xd10042b0,
   0x4a74755a,
   0x32402000,
   0x20016010,
   0x03004a76,
   0x29026010,
   0x2900d03b,
   0x2001d001,
   0x2000e000,
   0x7b526822,
   0x0f1b0713,
   0xfa68f000,
   0x06310406,
   0x200d0b08,
   0xe00d2701,
   0x40502201,
   0xd1262800,
   0x2301e7f8,
   0x28004058,
   0x0610d0f4,
   0xd0100f07,
   0xdd272f00,
   0x29036820,
   0x6845d00d,
   0xf0004628,
   0x2800fa3b,
   0x4861d009,
   0xe0109004,
   0x1e80485f,
   0x6825e7fa,
   0x6905e016,
   0x2001e7f0,
   0x6868e00f,
   0xf0009001,
   0x2800fa29,
   0x4858d004,
   0x27008068,
   0xe00743ff,
   0x80682003,
   0x9d019800,
   0x90001c40,
   0xdbec42b8,
   0x68219804,
   0x80482e00,
   0xd0014850,
   0xe0002104,
   0x60812101,
   0xda0e2f00,
   0xd0012e00,
   0xe0002108,
   0x60812102,
   0x46202100,
   0x72c13020,
   0x21fc6021,
   0xb0077281,
   0x8828bdf0,
   0x05002700,
   0x28030f80,
   0x2802d002,
   0xe02bd002,
   0xe000483a,
   0x9000483a,
   0xd00442a0,
   0x28006800,
   0x4f3bd012,
   0x98001cbf,
   0x7ac13020,
   0x40112283,
   0x493972c1,
   0x47884628,
   0xd0152882,
   0x42884937,
   0x4f33d115,
   0xe0361cbf,
   0x4621222c,
   0xf0009800,
   0x2100f9db,
   0x30204620,
   0x602172c1,
   0x728121fc,
   0x40462001,
   0xe7de9c00,
   0x1c7f4f29,
   0x2f00e023,
   0x68a0d121,
   0x980260e0,
   0x60256160,
   0x7b406820,
   0x0f000700,
   0xd9022805,
   0x1ebf4f21,
   0xb672e013,
   0x49232080,
   0x46054788,
   0x6820b662,
   0x22004b21,
   0x47984631,
   0xb6724607,
   0x491d4628,
   0x47883912,
   0x2f00b662,
   0x4632d0a1,
   0x46382102,
   0xff10f7ff,
   0x4b11e79b,
   0x7d1a2800,
   0x00c8d004,
   0x751a4302,
   0xe002480a,
   0x480a430a,
   0x22fe751a,
   0x540a212a,
   0x21026800,
   0x49078041,
   0x6008480f,
   0x20804907,
   0x60083180,
   0x00004770,
   0x40043000,
   0x21000134,
   0x21000108,
   0x210002a8,
   0x21000280,
   0xe000e180,
   0x00000804,
   0x40041100,
   0x00000657,
   0x0000ffff,
   0x0000426d,
   0x000004bf,
   0x21004095,
   0x2401b5f0,
   0x2000b087,
   0x48719000,
   0x4d6f9402,
   0x7f406800,
   0xd0002800,
   0x90042001,
   0x496b2000,
   0x31209001,
   0x91059804,
   0xd07e2800,
   0x3760462f,
   0x4a684629,
   0x31507ff8,
   0x06004790,
   0x77f80e00,
   0x8ce8d174,
   0x1a407fb9,
   0x84e81c80,
   0x7d309e05,
   0xd16c2800,
   0x6800485e,
   0x8f84495f,
   0x60082004,
   0x0fc00760,
   0x46287570,
   0x46013040,
   0x88009003,
   0x43902210,
   0x0fd206e2,
   0x43100112,
   0x43902220,
   0x0fd20662,
   0x43100152,
   0x43902240,
   0x0fd205e2,
   0x43100192,
   0x48518008,
   0x90014780,
   0x7d30494e,
   0x28003980,
   0x0720d029,
   0x75700fc0,
   0x5e38201c,
   0xda012800,
   0x60082004,
   0x21109803,
   0x43888800,
   0x0fc906a1,
   0x43080109,
   0x43882120,
   0x0fc90621,
   0x43080149,
   0x43882140,
   0x0fc905a1,
   0x43080189,
   0x80089903,
   0x75302000,
   0xfe38f7ff,
   0x29007d31,
   0x2101d142,
   0x90019100,
   0x201ce005,
   0x28005e38,
   0x2004da01,
   0x98006008,
   0xd0132800,
   0x04a10520,
   0x0fc90fc0,
   0x0fd207a2,
   0x462a9202,
   0x78533280,
   0x2b012401,
   0x2b03d00d,
   0xe031e002,
   0xe023e02f,
   0xe00cd007,
   0x04e10560,
   0x0fc90fc0,
   0x0fd207e2,
   0x7893e7ea,
   0xd1022b00,
   0xd1002800,
   0x78507534,
   0xd1112802,
   0xd0052900,
   0x8a809803,
   0x0f800480,
   0xd2092802,
   0x2124481e,
   0x5e697c40,
   0x08c01dc0,
   0x28031a08,
   0x7534d000,
   0x98014629,
   0xfdfaf7ff,
   0x28014604,
   0x9805d016,
   0x28007d00,
   0xe011d003,
   0x47804814,
   0x9804e7f4,
   0x04019a02,
   0x43019800,
   0xf000207e,
   0x9802f8a1,
   0xd0032800,
   0x490e2001,
   0x60880500,
   0xb0074620,
   0x2807bdf0,
   0x4a0bd001,
   0x4a0b4710,
   0x72c8604a,
   0x00004770,
   0x21000160,
   0x21000108,
   0x0002a22b,
   0xe000e180,
   0x0002a3a5,
   0x21000028,
   0x0002a679,
   0x40041100,
   0x000291a9,
   0x21004365,
   0x480b0742,
   0x60020f52,
   0x4a0a4b0b,
   0x7b12785b,
   0xd10b2b02,
   0xd00909d2,
   0xd0032900,
   0x60012105,
   0xe0022107,
   0x60012104,
   0x60012106,
   0x00004770,
   0x400862c0,
   0x210000a8,
   0x21000380,
   0x794a4916,
   0xd40c0752,
   0x2a00790a,
   0x2800d009,
   0x6988d100,
   0x03804912,
   0x18400f80,
   0x21014a11,
   0x47704710,
   0x4604b510,
   0xfd9ef7ff,
   0xf7ff4620,
   0xbd10ffe7,
   0x4604b570,
   0x20014a0b,
   0x6050460d,
   0x2000490a,
   0x46296248,
   0xf7ff4620,
   0x2c00fd93,
   0x4628d002,
   0xffd4f7ff,
   0x0000bd70,
   0x21000380,
   0x00002a30,
   0x00004eaf,
   0x40045000,
   0x40045380,
   0x88c04806,
   0xd0012801,
   0x47004805,
   0x20014905,
   0x49056048,
   0x62482000,
   0xe7f64804,
   0x21000380,
   0x0000a019,
   0x40045000,
   0x40045380,
   0x00005baf,
   0x4801b403,
   0xbd019001,
   0x00003fbd,
   0x4801b403,
   0xbd019001,
   0x000002f9,
   0x4801b403,
   0xbd019001,
   0x00003c87,
   0x4674b430,
   0x78251e64,
   0x42ab1c64,
   0x461dd200,
   0x005b5d63,
   0xbc3018e3,
   0x00004718,
};
#define _NWORD_PATCHIMAGE_MULTI_PROTOCOL 394

#define _NWORD_PATCHCPEHD_MULTI_PROTOCOL 0

#define _NWORD_PATCHSYS_MULTI_PROTOCOL 0

#define _IRQ_PATCH_0 0x0000a2bd


#ifndef _MULTI_PROTOCOL_SYSRAM_START
#define _MULTI_PROTOCOL_SYSRAM_START 0x20000000
#endif

#ifndef _MULTI_PROTOCOL_CPERAM_START
#define _MULTI_PROTOCOL_CPERAM_START 0x21000000
#endif

#define _MULTI_PROTOCOL_SYS_PATCH_FIXED_ADDR 0x20000000

#define _MULTI_PROTOCOL_PATCH_VEC_ADDR_OFFSET 0x03D0
#define _MULTI_PROTOCOL_PATCH_TAB_OFFSET 0x03D4
#define _MULTI_PROTOCOL_IRQPATCH_OFFSET 0x0480
#define _MULTI_PROTOCOL_PATCH_VEC_OFFSET 0x404C

#define _MULTI_PROTOCOL_PATCH_CPEHD_OFFSET 0x04E0

#ifndef _MULTI_PROTOCOL_NO_PROG_STATE_VAR
static uint8_t bMultiProtocolPatchEntered = 0;
#endif

PATCH_FUN_SPEC void enterMultiProtocolCpePatch(void)
{
#if (_NWORD_PATCHIMAGE_MULTI_PROTOCOL > 0)
   uint32_t *pPatchVec = (uint32_t *) (_MULTI_PROTOCOL_CPERAM_START + _MULTI_PROTOCOL_PATCH_VEC_OFFSET);

   memcpy(pPatchVec, patchImageMultiProtocol, sizeof(patchImageMultiProtocol));
#endif
}

PATCH_FUN_SPEC void enterMultiProtocolCpeHdPatch(void)
{
#if (_NWORD_PATCHCPEHD_MULTI_PROTOCOL > 0)
   uint32_t *pPatchCpeHd = (uint32_t *) (_MULTI_PROTOCOL_CPERAM_START + _MULTI_PROTOCOL_PATCH_CPEHD_OFFSET);

   memcpy(pPatchCpeHd, patchCpeHd, sizeof(patchCpeHd));
#endif
}

PATCH_FUN_SPEC void enterMultiProtocolSysPatch(void)
{
}

PATCH_FUN_SPEC void configureMultiProtocolPatch(void)
{
   uint8_t *pPatchTab = (uint8_t *) (_MULTI_PROTOCOL_CPERAM_START + _MULTI_PROTOCOL_PATCH_TAB_OFFSET);
   uint32_t *pIrqPatch = (uint32_t *) (_MULTI_PROTOCOL_CPERAM_START + _MULTI_PROTOCOL_IRQPATCH_OFFSET);


   pPatchTab[45] = 0;
   pPatchTab[7] = 1;
   pPatchTab[114] = 2;
   pPatchTab[75] = 3;
   pPatchTab[82] = 4;
   pPatchTab[80] = 5;
   pPatchTab[92] = 6;
   pPatchTab[13] = 7;
   pPatchTab[1] = 8;
   pPatchTab[162] = 9;
   pPatchTab[77] = 10;
   pPatchTab[151] = 11;
   pPatchTab[150] = 12;
   pPatchTab[12] = 13;
   pPatchTab[65] = 14;
   pPatchTab[63] = 15;
   pPatchTab[163] = 16;
   pPatchTab[41] = 17;

   pIrqPatch[1] = _IRQ_PATCH_0;
}

PATCH_FUN_SPEC void applyMultiProtocolPatch(void)
{
#ifdef _MULTI_PROTOCOL_NO_PROG_STATE_VAR
   enterMultiProtocolSysPatch();
   enterMultiProtocolCpePatch();
#else
   if (!bMultiProtocolPatchEntered)
   {
      enterMultiProtocolSysPatch();
      enterMultiProtocolCpePatch();
      bMultiProtocolPatchEntered = 1;
   }
#endif
   enterMultiProtocolCpeHdPatch();
   configureMultiProtocolPatch();
}

void refreshMultiProtocolPatch(void)
{
   enterMultiProtocolCpeHdPatch();
   configureMultiProtocolPatch();
}

void cleanMultiProtocolPatch(void)
{
#ifndef _MULTI_PROTOCOL_NO_PROG_STATE_VAR
   bMultiProtocolPatchEntered = 0;
#endif
}

void rf_patch_cpe_multi_protocol(void)
{
   applyMultiProtocolPatch();
}

#undef _IRQ_PATCH_0

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif


