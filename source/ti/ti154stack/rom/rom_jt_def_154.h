/******************************************************************************

 @file rom_jt_def_154.h

 @brief ROM jump table definition

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

#ifndef ROM_JT_DEF_H
#define ROM_JT_DEF_H

#include "hal_types.h"

/*******************************************************************************
 * EXTERNS
 */

// Common ROM's RAM table for pointers to ICall functions and flash jump tables.
// Note: This linker imported symbol is treated as a variable by the compiler.
// 0: iCall/OSCAL Dispatch Function Pointer
// 1: FH ROM Flash jump table
// 2: HMAC ROM Flash Jump table
// 3: LMAC ROM Flash Jump table 
// 

// ROM's RAM table offset to ROM Flash Jump Table pointer.
#define ROM_RAM_ICALL_OSAL_TABLE_INDEX                  0
#define ROM_RAM_FH_TABLE_INDEX                          1
#define ROM_RAM_HMAC_TABLE_INDEX                        2
#define ROM_RAM_LMAC_TABLE_INDEX                        3

#define ROM_RAM_TABLE_SIZE                              4

extern uint32_t RAM_MAC_BASE_ADDR[];

// Defines used for the flash jump table routines that are not part of build.
// Note: Any change to this table must accompany a change to ROM_Flash_JT[]!

#define ROM_ICALL_OSAL_JT_LOCATION                      (&RAM_MAC_BASE_ADDR[ROM_RAM_ICALL_OSAL_TABLE_INDEX])
#define ROM_ICALL_OSAL_JT_BASE                          (*((uint32 **)ROM_ICALL_OSAL_JT_LOCATION))
#define ROM_ICALL_OSAL_JT_OFFSET(index)                 (*(ROM_ICALL_OSAL_JT_BASE+(index)))

#define ROM_FH_JT_LOCATION                              (&RAM_MAC_BASE_ADDR[ROM_RAM_FH_TABLE_INDEX])
#define ROM_FH_JT_BASE                                  (*((uint32 **)ROM_FH_JT_LOCATION))
#define ROM_FH_JT_OFFSET(index)                         (*(ROM_FH_JT_BASE+(index)))

#define ROM_HMAC_JT_LOCATION                            (&RAM_MAC_BASE_ADDR[ROM_RAM_HMAC_TABLE_INDEX])
#define ROM_HMAC_JT_BASE                                (*((uint32 **)ROM_HMAC_JT_LOCATION))
#define ROM_HMAC_JT_OFFSET(index)                       (*(ROM_HMAC_JT_BASE+(index)))

#define ROM_LMAC_JT_LOCATION                            (&RAM_MAC_BASE_ADDR[ROM_RAM_LMAC_TABLE_INDEX])
#define ROM_LMAC_JT_BASE                                (*((uint32 **)ROM_LMAC_JT_LOCATION))
#define ROM_LMAC_JT_OFFSET(index)                       (*(ROM_LMAC_JT_BASE+(index)))

void Icall_Osal_ROM_Init(void);
void FH_ROM_Init(void);
void HMAC_ROM_Init(void);
void LMAC_ROM_Init(void);

#endif
