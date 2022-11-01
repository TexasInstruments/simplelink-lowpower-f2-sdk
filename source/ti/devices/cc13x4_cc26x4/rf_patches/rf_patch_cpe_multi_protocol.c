/******************************************************************************
*  Filename:       rf_patch_cpe_multi_protocol.c
*
*  Description:    RF Core patch for all protocols in CC26x4
*
*  Copyright (c) 2015-2022, Texas Instruments Incorporated
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
   0x21004237,
   0x00009fbd,
   0x0000a019,
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
   0x4803b5f0,
   0x6800b08d,
   0x49023020,
   0x00004708,
   0x21000108,
   0x0002a3ad,
   0x460cb570,
   0x47084900,
   0x0002a6a5,
   0x2401b5f0,
   0x2000b087,
   0x48679000,
   0x4d659402,
   0x7f406800,
   0xd0002800,
   0x90042001,
   0x49612000,
   0x31209001,
   0x91059804,
   0xd0722800,
   0x3760462f,
   0x4a5e4629,
   0x31507ff8,
   0x06004790,
   0x77f80e00,
   0x8ce8d168,
   0x1a407fb9,
   0x84e81c80,
   0x7d309e05,
   0xd17d2800,
   0x68004854,
   0x8f844955,
   0x60082004,
   0x0fc00760,
   0x46287570,
   0x46013040,
   0x88009003,
   0x43902210,
   0x0fd206e2,
   0x43100112,
   0x43902240,
   0x0fd20662,
   0x43100192,
   0x484a8008,
   0x90014780,
   0x7d304947,
   0x28003980,
   0x0720d023,
   0x75700fc0,
   0x5e38201c,
   0xda012800,
   0x60082004,
   0x21109803,
   0x43888800,
   0x0fc906a1,
   0x43080109,
   0x43882140,
   0x0fc90621,
   0x43080189,
   0x80089903,
   0x75302000,
   0xff84f7ff,
   0x29007d31,
   0x2101d13a,
   0x90019100,
   0x201ce005,
   0x28005e38,
   0x2004da01,
   0x98006008,
   0xd0102800,
   0x052105a0,
   0x0fc90fc0,
   0x0fd207a2,
   0x462a9202,
   0x78933280,
   0x20014303,
   0xd0092b00,
   0xe029e009,
   0x05e0e027,
   0x0fc00561,
   0x07e20fc9,
   0xe7ed0fd2,
   0x78527530,
   0xd1112a02,
   0xd0052900,
   0x8a899903,
   0x0f890489,
   0xd2092902,
   0x2224491e,
   0x5eaa7c49,
   0x08c91dc9,
   0x29031a51,
   0x7530d000,
   0x98014629,
   0xff4ef7ff,
   0x28014604,
   0x9805d016,
   0x28007d00,
   0xe011d003,
   0x47804814,
   0x9804e7f4,
   0x04019a02,
   0x43019800,
   0xf000207e,
   0x9802f827,
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
   0x210040a5,
   0x4801b403,
   0xbd019001,
   0x00003c87,
};
#define _NWORD_PATCHIMAGE_MULTI_PROTOCOL 140

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


   pPatchTab[7] = 0;
   pPatchTab[92] = 1;
   pPatchTab[80] = 2;
   pPatchTab[13] = 3;
   pPatchTab[1] = 4;
   pPatchTab[162] = 5;
   pPatchTab[77] = 6;
   pPatchTab[151] = 7;
   pPatchTab[150] = 8;
   pPatchTab[12] = 9;
   pPatchTab[65] = 10;
   pPatchTab[63] = 11;
   pPatchTab[163] = 12;

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


