/******************************************************************************
*  Filename:       hw_cpu_nvic_h
*
* Copyright (c) 2015 - 2017, Texas Instruments Incorporated
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1) Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2) Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* 3) Neither the name of the ORGANIZATION nor the names of its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#ifndef __HW_CPU_NVIC_H__
#define __HW_CPU_NVIC_H__

//*****************************************************************************
//
// This section defines the register offsets of
// CPU_NVIC component
//
//*****************************************************************************
// Enables or reads the enabled state of each group of 32 interrupts
#define CPU_NVIC_O_ISER0                                            0x00000000

// Enables or reads the enabled state of each group of 32 interrupts
#define CPU_NVIC_O_ISER1                                            0x00000004

// Clears or reads the enabled state of each group of 32 interrupts
#define CPU_NVIC_O_ICER0                                            0x00000080

// Clears or reads the enabled state of each group of 32 interrupts
#define CPU_NVIC_O_ICER1                                            0x00000084

// Enables or reads the pending state of each group of 32 interrupts
#define CPU_NVIC_O_ISPR0                                            0x00000100

// Enables or reads the pending state of each group of 32 interrupts
#define CPU_NVIC_O_ISPR1                                            0x00000104

// Clears or reads the pending state of each group of 32 interrupts
#define CPU_NVIC_O_ICPR0                                            0x00000180

// Clears or reads the pending state of each group of 32 interrupts
#define CPU_NVIC_O_ICPR1                                            0x00000184

// For each group of 32 interrupts, shows the active state of each interrupt
#define CPU_NVIC_O_IABR0                                            0x00000200

// For each group of 32 interrupts, shows the active state of each interrupt
#define CPU_NVIC_O_IABR1                                            0x00000204

// For each group of 32 interrupts, determines whether each interrupt targets
// Non-secure or Secure state
#define CPU_NVIC_O_ITNS0                                            0x00000280

// For each group of 32 interrupts, determines whether each interrupt targets
// Non-secure or Secure state
#define CPU_NVIC_O_ITNS1                                            0x00000284

// Sets or reads interrupt priorities
#define CPU_NVIC_O_IPR0                                             0x00000300

// Sets or reads interrupt priorities
#define CPU_NVIC_O_IPR1                                             0x00000304

// Sets or reads interrupt priorities
#define CPU_NVIC_O_IPR2                                             0x00000308

// Sets or reads interrupt priorities
#define CPU_NVIC_O_IPR3                                             0x0000030C

// Sets or reads interrupt priorities
#define CPU_NVIC_O_IPR4                                             0x00000310

// Sets or reads interrupt priorities
#define CPU_NVIC_O_IPR5                                             0x00000314

// Sets or reads interrupt priorities
#define CPU_NVIC_O_IPR6                                             0x00000318

// Sets or reads interrupt priorities
#define CPU_NVIC_O_IPR7                                             0x0000031C

// Sets or reads interrupt priorities
#define CPU_NVIC_O_IPR8                                             0x00000320

// Sets or reads interrupt priorities
#define CPU_NVIC_O_IPR9                                             0x00000324

// Sets or reads interrupt priorities
#define CPU_NVIC_O_IPR10                                            0x00000328

// Sets or reads interrupt priorities
#define CPU_NVIC_O_IPR11                                            0x0000032C

//*****************************************************************************
//
// Register: CPU_NVIC_O_ISER0
//
//*****************************************************************************
// Field:  [31:0] SETENA
//
// For SETENA[m] in NVIC_ISER*n, indicates whether interrupt 32*n + m is
// enabled
#define CPU_NVIC_ISER0_SETENA_W                                             32
#define CPU_NVIC_ISER0_SETENA_M                                     0xFFFFFFFF
#define CPU_NVIC_ISER0_SETENA_S                                              0

//*****************************************************************************
//
// Register: CPU_NVIC_O_ISER1
//
//*****************************************************************************
// Field:  [15:0] SETENA
//
// For SETENA[m] in NVIC_ISER*n, indicates whether interrupt 32*n + m is
// enabled
#define CPU_NVIC_ISER1_SETENA_W                                             16
#define CPU_NVIC_ISER1_SETENA_M                                     0x0000FFFF
#define CPU_NVIC_ISER1_SETENA_S                                              0

//*****************************************************************************
//
// Register: CPU_NVIC_O_ICER0
//
//*****************************************************************************
// Field:  [31:0] CLRENA
//
// For CLRENA[m] in NVIC_ICER*n, indicates whether interrupt 32*n + m is
// enabled
#define CPU_NVIC_ICER0_CLRENA_W                                             32
#define CPU_NVIC_ICER0_CLRENA_M                                     0xFFFFFFFF
#define CPU_NVIC_ICER0_CLRENA_S                                              0

//*****************************************************************************
//
// Register: CPU_NVIC_O_ICER1
//
//*****************************************************************************
// Field:  [15:0] CLRENA
//
// For CLRENA[m] in NVIC_ICER*n, indicates whether interrupt 32*n + m is
// enabled
#define CPU_NVIC_ICER1_CLRENA_W                                             16
#define CPU_NVIC_ICER1_CLRENA_M                                     0x0000FFFF
#define CPU_NVIC_ICER1_CLRENA_S                                              0

//*****************************************************************************
//
// Register: CPU_NVIC_O_ISPR0
//
//*****************************************************************************
// Field:  [31:0] SETPEND
//
// For SETPEND[m] in NVIC_ISPR*n, indicates whether interrupt 32*n + m is
// pending
#define CPU_NVIC_ISPR0_SETPEND_W                                            32
#define CPU_NVIC_ISPR0_SETPEND_M                                    0xFFFFFFFF
#define CPU_NVIC_ISPR0_SETPEND_S                                             0

//*****************************************************************************
//
// Register: CPU_NVIC_O_ISPR1
//
//*****************************************************************************
// Field:  [15:0] SETPEND
//
// For SETPEND[m] in NVIC_ISPR*n, indicates whether interrupt 32*n + m is
// pending
#define CPU_NVIC_ISPR1_SETPEND_W                                            16
#define CPU_NVIC_ISPR1_SETPEND_M                                    0x0000FFFF
#define CPU_NVIC_ISPR1_SETPEND_S                                             0

//*****************************************************************************
//
// Register: CPU_NVIC_O_ICPR0
//
//*****************************************************************************
// Field:  [31:0] CLRPEND
//
// For CLRPEND[m] in NVIC_ICPR*n, indicates whether interrupt 32*n + m is
// pending
#define CPU_NVIC_ICPR0_CLRPEND_W                                            32
#define CPU_NVIC_ICPR0_CLRPEND_M                                    0xFFFFFFFF
#define CPU_NVIC_ICPR0_CLRPEND_S                                             0

//*****************************************************************************
//
// Register: CPU_NVIC_O_ICPR1
//
//*****************************************************************************
// Field:  [15:0] CLRPEND
//
// For CLRPEND[m] in NVIC_ICPR*n, indicates whether interrupt 32*n + m is
// pending
#define CPU_NVIC_ICPR1_CLRPEND_W                                            16
#define CPU_NVIC_ICPR1_CLRPEND_M                                    0x0000FFFF
#define CPU_NVIC_ICPR1_CLRPEND_S                                             0

//*****************************************************************************
//
// Register: CPU_NVIC_O_IABR0
//
//*****************************************************************************
// Field:  [31:0] ACTIVE
//
// For ACTIVE[m] in NVIC_IABR*n, indicates the active state for interrupt
// 32*n+m
#define CPU_NVIC_IABR0_ACTIVE_W                                             32
#define CPU_NVIC_IABR0_ACTIVE_M                                     0xFFFFFFFF
#define CPU_NVIC_IABR0_ACTIVE_S                                              0

//*****************************************************************************
//
// Register: CPU_NVIC_O_IABR1
//
//*****************************************************************************
// Field:  [15:0] ACTIVE
//
// For ACTIVE[m] in NVIC_IABR*n, indicates the active state for interrupt
// 32*n+m
#define CPU_NVIC_IABR1_ACTIVE_W                                             16
#define CPU_NVIC_IABR1_ACTIVE_M                                     0x0000FFFF
#define CPU_NVIC_IABR1_ACTIVE_S                                              0

//*****************************************************************************
//
// Register: CPU_NVIC_O_ITNS0
//
//*****************************************************************************
// Field:  [31:0] ITNS
//
// For ITNS[m] in NVIC_ITNS*n, the target Security state for interrupt 32*n+m
#define CPU_NVIC_ITNS0_ITNS_W                                               32
#define CPU_NVIC_ITNS0_ITNS_M                                       0xFFFFFFFF
#define CPU_NVIC_ITNS0_ITNS_S                                                0

//*****************************************************************************
//
// Register: CPU_NVIC_O_ITNS1
//
//*****************************************************************************
// Field:  [15:0] ITNS
//
// For ITNS[m] in NVIC_ITNS*n, the target Security state for interrupt 32*n+m
#define CPU_NVIC_ITNS1_ITNS_W                                               16
#define CPU_NVIC_ITNS1_ITNS_M                                       0x0000FFFF
#define CPU_NVIC_ITNS1_ITNS_S                                                0

//*****************************************************************************
//
// Register: CPU_NVIC_O_IPR0
//
//*****************************************************************************
// Field: [31:24] PRI_N3
//
// For register NVIC_IPR*0, the priority of interrupt number 4*0+3, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR0_PRI_N3_W                                               8
#define CPU_NVIC_IPR0_PRI_N3_M                                      0xFF000000
#define CPU_NVIC_IPR0_PRI_N3_S                                              24

// Field: [23:16] PRI_N2
//
// For register NVIC_IPR*0, the priority of interrupt number 4*0+2, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR0_PRI_N2_W                                               8
#define CPU_NVIC_IPR0_PRI_N2_M                                      0x00FF0000
#define CPU_NVIC_IPR0_PRI_N2_S                                              16

// Field:  [15:8] PRI_N1
//
// For register NVIC_IPR*0, the priority of interrupt number 4*0+1, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR0_PRI_N1_W                                               8
#define CPU_NVIC_IPR0_PRI_N1_M                                      0x0000FF00
#define CPU_NVIC_IPR0_PRI_N1_S                                               8

// Field:   [7:0] PRI_N0
//
// For register NVIC_IPR*0, the priority of interrupt number 4*0+0, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR0_PRI_N0_W                                               8
#define CPU_NVIC_IPR0_PRI_N0_M                                      0x000000FF
#define CPU_NVIC_IPR0_PRI_N0_S                                               0

//*****************************************************************************
//
// Register: CPU_NVIC_O_IPR1
//
//*****************************************************************************
// Field: [31:24] PRI_N3
//
// For register NVIC_IPR*1, the priority of interrupt number 4*1+3, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR1_PRI_N3_W                                               8
#define CPU_NVIC_IPR1_PRI_N3_M                                      0xFF000000
#define CPU_NVIC_IPR1_PRI_N3_S                                              24

// Field: [23:16] PRI_N2
//
// For register NVIC_IPR*1, the priority of interrupt number 4*1+2, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR1_PRI_N2_W                                               8
#define CPU_NVIC_IPR1_PRI_N2_M                                      0x00FF0000
#define CPU_NVIC_IPR1_PRI_N2_S                                              16

// Field:  [15:8] PRI_N1
//
// For register NVIC_IPR*1, the priority of interrupt number 4*1+1, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR1_PRI_N1_W                                               8
#define CPU_NVIC_IPR1_PRI_N1_M                                      0x0000FF00
#define CPU_NVIC_IPR1_PRI_N1_S                                               8

// Field:   [7:0] PRI_N0
//
// For register NVIC_IPR*1, the priority of interrupt number 4*1+0, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR1_PRI_N0_W                                               8
#define CPU_NVIC_IPR1_PRI_N0_M                                      0x000000FF
#define CPU_NVIC_IPR1_PRI_N0_S                                               0

//*****************************************************************************
//
// Register: CPU_NVIC_O_IPR2
//
//*****************************************************************************
// Field: [31:24] PRI_N3
//
// For register NVIC_IPR*2, the priority of interrupt number 4*2+3, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR2_PRI_N3_W                                               8
#define CPU_NVIC_IPR2_PRI_N3_M                                      0xFF000000
#define CPU_NVIC_IPR2_PRI_N3_S                                              24

// Field: [23:16] PRI_N2
//
// For register NVIC_IPR*2, the priority of interrupt number 4*2+2, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR2_PRI_N2_W                                               8
#define CPU_NVIC_IPR2_PRI_N2_M                                      0x00FF0000
#define CPU_NVIC_IPR2_PRI_N2_S                                              16

// Field:  [15:8] PRI_N1
//
// For register NVIC_IPR*2, the priority of interrupt number 4*2+1, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR2_PRI_N1_W                                               8
#define CPU_NVIC_IPR2_PRI_N1_M                                      0x0000FF00
#define CPU_NVIC_IPR2_PRI_N1_S                                               8

// Field:   [7:0] PRI_N0
//
// For register NVIC_IPR*2, the priority of interrupt number 4*2+0, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR2_PRI_N0_W                                               8
#define CPU_NVIC_IPR2_PRI_N0_M                                      0x000000FF
#define CPU_NVIC_IPR2_PRI_N0_S                                               0

//*****************************************************************************
//
// Register: CPU_NVIC_O_IPR3
//
//*****************************************************************************
// Field: [31:24] PRI_N3
//
// For register NVIC_IPR*3, the priority of interrupt number 4*3+3, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR3_PRI_N3_W                                               8
#define CPU_NVIC_IPR3_PRI_N3_M                                      0xFF000000
#define CPU_NVIC_IPR3_PRI_N3_S                                              24

// Field: [23:16] PRI_N2
//
// For register NVIC_IPR*3, the priority of interrupt number 4*3+2, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR3_PRI_N2_W                                               8
#define CPU_NVIC_IPR3_PRI_N2_M                                      0x00FF0000
#define CPU_NVIC_IPR3_PRI_N2_S                                              16

// Field:  [15:8] PRI_N1
//
// For register NVIC_IPR*3, the priority of interrupt number 4*3+1, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR3_PRI_N1_W                                               8
#define CPU_NVIC_IPR3_PRI_N1_M                                      0x0000FF00
#define CPU_NVIC_IPR3_PRI_N1_S                                               8

// Field:   [7:0] PRI_N0
//
// For register NVIC_IPR*3, the priority of interrupt number 4*3+0, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR3_PRI_N0_W                                               8
#define CPU_NVIC_IPR3_PRI_N0_M                                      0x000000FF
#define CPU_NVIC_IPR3_PRI_N0_S                                               0

//*****************************************************************************
//
// Register: CPU_NVIC_O_IPR4
//
//*****************************************************************************
// Field: [31:24] PRI_N3
//
// For register NVIC_IPR*4, the priority of interrupt number 4*4+3, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR4_PRI_N3_W                                               8
#define CPU_NVIC_IPR4_PRI_N3_M                                      0xFF000000
#define CPU_NVIC_IPR4_PRI_N3_S                                              24

// Field: [23:16] PRI_N2
//
// For register NVIC_IPR*4, the priority of interrupt number 4*4+2, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR4_PRI_N2_W                                               8
#define CPU_NVIC_IPR4_PRI_N2_M                                      0x00FF0000
#define CPU_NVIC_IPR4_PRI_N2_S                                              16

// Field:  [15:8] PRI_N1
//
// For register NVIC_IPR*4, the priority of interrupt number 4*4+1, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR4_PRI_N1_W                                               8
#define CPU_NVIC_IPR4_PRI_N1_M                                      0x0000FF00
#define CPU_NVIC_IPR4_PRI_N1_S                                               8

// Field:   [7:0] PRI_N0
//
// For register NVIC_IPR*4, the priority of interrupt number 4*4+0, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR4_PRI_N0_W                                               8
#define CPU_NVIC_IPR4_PRI_N0_M                                      0x000000FF
#define CPU_NVIC_IPR4_PRI_N0_S                                               0

//*****************************************************************************
//
// Register: CPU_NVIC_O_IPR5
//
//*****************************************************************************
// Field: [31:24] PRI_N3
//
// For register NVIC_IPR*5, the priority of interrupt number 4*5+3, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR5_PRI_N3_W                                               8
#define CPU_NVIC_IPR5_PRI_N3_M                                      0xFF000000
#define CPU_NVIC_IPR5_PRI_N3_S                                              24

// Field: [23:16] PRI_N2
//
// For register NVIC_IPR*5, the priority of interrupt number 4*5+2, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR5_PRI_N2_W                                               8
#define CPU_NVIC_IPR5_PRI_N2_M                                      0x00FF0000
#define CPU_NVIC_IPR5_PRI_N2_S                                              16

// Field:  [15:8] PRI_N1
//
// For register NVIC_IPR*5, the priority of interrupt number 4*5+1, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR5_PRI_N1_W                                               8
#define CPU_NVIC_IPR5_PRI_N1_M                                      0x0000FF00
#define CPU_NVIC_IPR5_PRI_N1_S                                               8

// Field:   [7:0] PRI_N0
//
// For register NVIC_IPR*5, the priority of interrupt number 4*5+0, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR5_PRI_N0_W                                               8
#define CPU_NVIC_IPR5_PRI_N0_M                                      0x000000FF
#define CPU_NVIC_IPR5_PRI_N0_S                                               0

//*****************************************************************************
//
// Register: CPU_NVIC_O_IPR6
//
//*****************************************************************************
// Field: [31:24] PRI_N3
//
// For register NVIC_IPR*6, the priority of interrupt number 4*6+3, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR6_PRI_N3_W                                               8
#define CPU_NVIC_IPR6_PRI_N3_M                                      0xFF000000
#define CPU_NVIC_IPR6_PRI_N3_S                                              24

// Field: [23:16] PRI_N2
//
// For register NVIC_IPR*6, the priority of interrupt number 4*6+2, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR6_PRI_N2_W                                               8
#define CPU_NVIC_IPR6_PRI_N2_M                                      0x00FF0000
#define CPU_NVIC_IPR6_PRI_N2_S                                              16

// Field:  [15:8] PRI_N1
//
// For register NVIC_IPR*6, the priority of interrupt number 4*6+1, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR6_PRI_N1_W                                               8
#define CPU_NVIC_IPR6_PRI_N1_M                                      0x0000FF00
#define CPU_NVIC_IPR6_PRI_N1_S                                               8

// Field:   [7:0] PRI_N0
//
// For register NVIC_IPR*6, the priority of interrupt number 4*6+0, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR6_PRI_N0_W                                               8
#define CPU_NVIC_IPR6_PRI_N0_M                                      0x000000FF
#define CPU_NVIC_IPR6_PRI_N0_S                                               0

//*****************************************************************************
//
// Register: CPU_NVIC_O_IPR7
//
//*****************************************************************************
// Field: [31:24] PRI_N3
//
// For register NVIC_IPR*7, the priority of interrupt number 4*7+3, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR7_PRI_N3_W                                               8
#define CPU_NVIC_IPR7_PRI_N3_M                                      0xFF000000
#define CPU_NVIC_IPR7_PRI_N3_S                                              24

// Field: [23:16] PRI_N2
//
// For register NVIC_IPR*7, the priority of interrupt number 4*7+2, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR7_PRI_N2_W                                               8
#define CPU_NVIC_IPR7_PRI_N2_M                                      0x00FF0000
#define CPU_NVIC_IPR7_PRI_N2_S                                              16

// Field:  [15:8] PRI_N1
//
// For register NVIC_IPR*7, the priority of interrupt number 4*7+1, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR7_PRI_N1_W                                               8
#define CPU_NVIC_IPR7_PRI_N1_M                                      0x0000FF00
#define CPU_NVIC_IPR7_PRI_N1_S                                               8

// Field:   [7:0] PRI_N0
//
// For register NVIC_IPR*7, the priority of interrupt number 4*7+0, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR7_PRI_N0_W                                               8
#define CPU_NVIC_IPR7_PRI_N0_M                                      0x000000FF
#define CPU_NVIC_IPR7_PRI_N0_S                                               0

//*****************************************************************************
//
// Register: CPU_NVIC_O_IPR8
//
//*****************************************************************************
// Field: [31:24] PRI_N3
//
// For register NVIC_IPR*8, the priority of interrupt number 4*8+3, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR8_PRI_N3_W                                               8
#define CPU_NVIC_IPR8_PRI_N3_M                                      0xFF000000
#define CPU_NVIC_IPR8_PRI_N3_S                                              24

// Field: [23:16] PRI_N2
//
// For register NVIC_IPR*8, the priority of interrupt number 4*8+2, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR8_PRI_N2_W                                               8
#define CPU_NVIC_IPR8_PRI_N2_M                                      0x00FF0000
#define CPU_NVIC_IPR8_PRI_N2_S                                              16

// Field:  [15:8] PRI_N1
//
// For register NVIC_IPR*8, the priority of interrupt number 4*8+1, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR8_PRI_N1_W                                               8
#define CPU_NVIC_IPR8_PRI_N1_M                                      0x0000FF00
#define CPU_NVIC_IPR8_PRI_N1_S                                               8

// Field:   [7:0] PRI_N0
//
// For register NVIC_IPR*8, the priority of interrupt number 4*8+0, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR8_PRI_N0_W                                               8
#define CPU_NVIC_IPR8_PRI_N0_M                                      0x000000FF
#define CPU_NVIC_IPR8_PRI_N0_S                                               0

//*****************************************************************************
//
// Register: CPU_NVIC_O_IPR9
//
//*****************************************************************************
// Field: [31:24] PRI_N3
//
// For register NVIC_IPR*9, the priority of interrupt number 4*9+3, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR9_PRI_N3_W                                               8
#define CPU_NVIC_IPR9_PRI_N3_M                                      0xFF000000
#define CPU_NVIC_IPR9_PRI_N3_S                                              24

// Field: [23:16] PRI_N2
//
// For register NVIC_IPR*9, the priority of interrupt number 4*9+2, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR9_PRI_N2_W                                               8
#define CPU_NVIC_IPR9_PRI_N2_M                                      0x00FF0000
#define CPU_NVIC_IPR9_PRI_N2_S                                              16

// Field:  [15:8] PRI_N1
//
// For register NVIC_IPR*9, the priority of interrupt number 4*9+1, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR9_PRI_N1_W                                               8
#define CPU_NVIC_IPR9_PRI_N1_M                                      0x0000FF00
#define CPU_NVIC_IPR9_PRI_N1_S                                               8

// Field:   [7:0] PRI_N0
//
// For register NVIC_IPR*9, the priority of interrupt number 4*9+0, or is RES0
// if the PE does not implement this interrupt
#define CPU_NVIC_IPR9_PRI_N0_W                                               8
#define CPU_NVIC_IPR9_PRI_N0_M                                      0x000000FF
#define CPU_NVIC_IPR9_PRI_N0_S                                               0

//*****************************************************************************
//
// Register: CPU_NVIC_O_IPR10
//
//*****************************************************************************
// Field: [31:24] PRI_N3
//
// For register NVIC_IPR*10, the priority of interrupt number 4*10+3, or is
// RES0 if the PE does not implement this interrupt
#define CPU_NVIC_IPR10_PRI_N3_W                                              8
#define CPU_NVIC_IPR10_PRI_N3_M                                     0xFF000000
#define CPU_NVIC_IPR10_PRI_N3_S                                             24

// Field: [23:16] PRI_N2
//
// For register NVIC_IPR*10, the priority of interrupt number 4*10+2, or is
// RES0 if the PE does not implement this interrupt
#define CPU_NVIC_IPR10_PRI_N2_W                                              8
#define CPU_NVIC_IPR10_PRI_N2_M                                     0x00FF0000
#define CPU_NVIC_IPR10_PRI_N2_S                                             16

// Field:  [15:8] PRI_N1
//
// For register NVIC_IPR*10, the priority of interrupt number 4*10+1, or is
// RES0 if the PE does not implement this interrupt
#define CPU_NVIC_IPR10_PRI_N1_W                                              8
#define CPU_NVIC_IPR10_PRI_N1_M                                     0x0000FF00
#define CPU_NVIC_IPR10_PRI_N1_S                                              8

// Field:   [7:0] PRI_N0
//
// For register NVIC_IPR*10, the priority of interrupt number 4*10+0, or is
// RES0 if the PE does not implement this interrupt
#define CPU_NVIC_IPR10_PRI_N0_W                                              8
#define CPU_NVIC_IPR10_PRI_N0_M                                     0x000000FF
#define CPU_NVIC_IPR10_PRI_N0_S                                              0

//*****************************************************************************
//
// Register: CPU_NVIC_O_IPR11
//
//*****************************************************************************
// Field: [31:24] PRI_N3
//
// For register NVIC_IPR*11, the priority of interrupt number 4*11+3, or is
// RES0 if the PE does not implement this interrupt
#define CPU_NVIC_IPR11_PRI_N3_W                                              8
#define CPU_NVIC_IPR11_PRI_N3_M                                     0xFF000000
#define CPU_NVIC_IPR11_PRI_N3_S                                             24

// Field: [23:16] PRI_N2
//
// For register NVIC_IPR*11, the priority of interrupt number 4*11+2, or is
// RES0 if the PE does not implement this interrupt
#define CPU_NVIC_IPR11_PRI_N2_W                                              8
#define CPU_NVIC_IPR11_PRI_N2_M                                     0x00FF0000
#define CPU_NVIC_IPR11_PRI_N2_S                                             16

// Field:  [15:8] PRI_N1
//
// For register NVIC_IPR*11, the priority of interrupt number 4*11+1, or is
// RES0 if the PE does not implement this interrupt
#define CPU_NVIC_IPR11_PRI_N1_W                                              8
#define CPU_NVIC_IPR11_PRI_N1_M                                     0x0000FF00
#define CPU_NVIC_IPR11_PRI_N1_S                                              8

// Field:   [7:0] PRI_N0
//
// For register NVIC_IPR*11, the priority of interrupt number 4*11+0, or is
// RES0 if the PE does not implement this interrupt
#define CPU_NVIC_IPR11_PRI_N0_W                                              8
#define CPU_NVIC_IPR11_PRI_N0_M                                     0x000000FF
#define CPU_NVIC_IPR11_PRI_N0_S                                              0


#endif // __CPU_NVIC__
