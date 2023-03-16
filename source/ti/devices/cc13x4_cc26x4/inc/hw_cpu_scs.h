/******************************************************************************
*  Filename:       hw_cpu_scs_h
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

#ifndef __HW_CPU_SCS_H__
#define __HW_CPU_SCS_H__

//*****************************************************************************
//
// This section defines the register offsets of
// CPU_SCS component
//
//*****************************************************************************
// CPUID Base
#define CPU_SCS_O_CPUID                                             0x00000000

// Interrupt Control State
#define CPU_SCS_O_ICSR                                              0x00000004

// Vector Table Offset
#define CPU_SCS_O_VTOR                                              0x00000008

// Application Interrupt/Reset Control
#define CPU_SCS_O_AIRCR                                             0x0000000C

// System Control
#define CPU_SCS_O_SCR                                               0x00000010

// Configuration Control
#define CPU_SCS_O_CCR                                               0x00000014

// System Handlers 4-7 Priority
#define CPU_SCS_O_SHPR1                                             0x00000018

// System Handlers 8-11 Priority
#define CPU_SCS_O_SHPR2                                             0x0000001C

// System Handlers 12-15 Priority
#define CPU_SCS_O_SHPR3                                             0x00000020

// System Handler Control and State
#define CPU_SCS_O_SHCSR                                             0x00000024

// Configurable Fault Status
#define CPU_SCS_O_CFSR                                              0x00000028

// Hard Fault Status
#define CPU_SCS_O_HFSR                                              0x0000002C

// Debug Fault Status
#define CPU_SCS_O_DFSR                                              0x00000030

// Mem Manage Fault Address
#define CPU_SCS_O_MMFAR                                             0x00000034

// Bus Fault Address
#define CPU_SCS_O_BFAR                                              0x00000038

// Auxiliary Fault Status
#define CPU_SCS_O_AFSR                                              0x0000003C

// Processor Feature 0
#define CPU_SCS_O_ID_PFR0                                           0x00000040

// Processor Feature 1
#define CPU_SCS_O_ID_PFR1                                           0x00000044

// Debug Feature 0
#define CPU_SCS_O_ID_DFR0                                           0x00000048

// Auxiliary Feature 0
#define CPU_SCS_O_ID_AFR0                                           0x0000004C

// Memory Model Feature 0
#define CPU_SCS_O_ID_MMFR0                                          0x00000050

// Memory Model Feature 1
#define CPU_SCS_O_ID_MMFR1                                          0x00000054

// Memory Model Feature 2
#define CPU_SCS_O_ID_MMFR2                                          0x00000058

// Memory Model Feature 3
#define CPU_SCS_O_ID_MMFR3                                          0x0000005C

// ISA Feature 0
#define CPU_SCS_O_ID_ISAR0                                          0x00000060

// ISA Feature 1
#define CPU_SCS_O_ID_ISAR1                                          0x00000064

// ISA Feature 2
#define CPU_SCS_O_ID_ISAR2                                          0x00000068

// ISA Feature 3
#define CPU_SCS_O_ID_ISAR3                                          0x0000006C

// ISA Feature 4
#define CPU_SCS_O_ID_ISAR4                                          0x00000070

//*****************************************************************************
//
// Register: CPU_SCS_O_CPUID
//
//*****************************************************************************
// Field: [31:24] IMPLEMENTER
//
// Implementor code.
#define CPU_SCS_CPUID_IMPLEMENTER_W                                          8
#define CPU_SCS_CPUID_IMPLEMENTER_M                                 0xFF000000
#define CPU_SCS_CPUID_IMPLEMENTER_S                                         24

// Field: [23:20] VARIANT
//
// Implementation defined variant number.
#define CPU_SCS_CPUID_VARIANT_W                                              4
#define CPU_SCS_CPUID_VARIANT_M                                     0x00F00000
#define CPU_SCS_CPUID_VARIANT_S                                             20

// Field: [19:16] CONSTANT
//
// Reads as 0xF
#define CPU_SCS_CPUID_CONSTANT_W                                             4
#define CPU_SCS_CPUID_CONSTANT_M                                    0x000F0000
#define CPU_SCS_CPUID_CONSTANT_S                                            16

// Field:  [15:4] PARTNO
//
// Number of processor within family.
#define CPU_SCS_CPUID_PARTNO_W                                              12
#define CPU_SCS_CPUID_PARTNO_M                                      0x0000FFF0
#define CPU_SCS_CPUID_PARTNO_S                                               4

// Field:   [3:0] REVISION
//
// Implementation defined revision number.
#define CPU_SCS_CPUID_REVISION_W                                             4
#define CPU_SCS_CPUID_REVISION_M                                    0x0000000F
#define CPU_SCS_CPUID_REVISION_S                                             0

//*****************************************************************************
//
// Register: CPU_SCS_O_ICSR
//
//*****************************************************************************
// Field:    [31] NMIPENDSET
//
// Set pending NMI bit. Setting this bit pends and activates an NMI. Because
// NMI is the highest-priority interrupt, it takes effect as soon as it
// registers.
//
// 0: No action
// 1: Set pending NMI
#define CPU_SCS_ICSR_NMIPENDSET                                     0x80000000
#define CPU_SCS_ICSR_NMIPENDSET_BITN                                        31
#define CPU_SCS_ICSR_NMIPENDSET_M                                   0x80000000
#define CPU_SCS_ICSR_NMIPENDSET_S                                           31

// Field:    [30] PENDNMICLR
//
// Pend NMI clear. Allows the NMI exception pending state to be cleared.
// 0x0 No effect.
// 0x1 Clear pending status.
#define CPU_SCS_ICSR_PENDNMICLR                                     0x40000000
#define CPU_SCS_ICSR_PENDNMICLR_BITN                                        30
#define CPU_SCS_ICSR_PENDNMICLR_M                                   0x40000000
#define CPU_SCS_ICSR_PENDNMICLR_S                                           30

// Field:    [28] PENDSVSET
//
// Set pending pendSV bit.
//
// 0: No action
// 1: Set pending PendSV
#define CPU_SCS_ICSR_PENDSVSET                                      0x10000000
#define CPU_SCS_ICSR_PENDSVSET_BITN                                         28
#define CPU_SCS_ICSR_PENDSVSET_M                                    0x10000000
#define CPU_SCS_ICSR_PENDSVSET_S                                            28

// Field:    [27] PENDSVCLR
//
// Clear pending pendSV bit
//
// 0: No action
// 1: Clear pending pendSV
#define CPU_SCS_ICSR_PENDSVCLR                                      0x08000000
#define CPU_SCS_ICSR_PENDSVCLR_BITN                                         27
#define CPU_SCS_ICSR_PENDSVCLR_M                                    0x08000000
#define CPU_SCS_ICSR_PENDSVCLR_S                                            27

// Field:    [26] PENDSTSET
//
// Set a pending SysTick bit.
//
// 0: No action
// 1: Set pending SysTick
#define CPU_SCS_ICSR_PENDSTSET                                      0x04000000
#define CPU_SCS_ICSR_PENDSTSET_BITN                                         26
#define CPU_SCS_ICSR_PENDSTSET_M                                    0x04000000
#define CPU_SCS_ICSR_PENDSTSET_S                                            26

// Field:    [25] PENDSTCLR
//
// Clear pending SysTick bit
//
// 0: No action
// 1: Clear pending SysTick
#define CPU_SCS_ICSR_PENDSTCLR                                      0x02000000
#define CPU_SCS_ICSR_PENDSTCLR_BITN                                         25
#define CPU_SCS_ICSR_PENDSTCLR_M                                    0x02000000
#define CPU_SCS_ICSR_PENDSTCLR_S                                            25

// Field:    [24] STTNS
//
// SysTick Targets Non-secure. Controls whether in a single SysTick
// implementation, the SysTick is Secure or
// Non-secure.
//
// 0x0 SysTick is Secure.
// 0x1 SysTick is Non-secure.
#define CPU_SCS_ICSR_STTNS                                          0x01000000
#define CPU_SCS_ICSR_STTNS_BITN                                             24
#define CPU_SCS_ICSR_STTNS_M                                        0x01000000
#define CPU_SCS_ICSR_STTNS_S                                                24

// Field:    [23] ISRPREEMPT
//
// This field can only be used at debug time. It indicates that a pending
// interrupt is to be taken in the next running cycle. If DHCSR.C_MASKINTS= 0,
// the interrupt is serviced.
//
// 0: A pending exception is not serviced.
// 1: A pending exception is serviced on exit from the debug halt state
#define CPU_SCS_ICSR_ISRPREEMPT                                     0x00800000
#define CPU_SCS_ICSR_ISRPREEMPT_BITN                                        23
#define CPU_SCS_ICSR_ISRPREEMPT_M                                   0x00800000
#define CPU_SCS_ICSR_ISRPREEMPT_S                                           23

// Field:    [22] ISRPENDING
//
// Interrupt pending flag. Excludes NMI and faults.
//
// 0x0: Interrupt not pending
// 0x1: Interrupt pending
#define CPU_SCS_ICSR_ISRPENDING                                     0x00400000
#define CPU_SCS_ICSR_ISRPENDING_BITN                                        22
#define CPU_SCS_ICSR_ISRPENDING_M                                   0x00400000
#define CPU_SCS_ICSR_ISRPENDING_S                                           22

// Field: [20:12] VECTPENDING
//
// Pending ISR number field. This field contains the interrupt number of the
// highest priority pending ISR.
#define CPU_SCS_ICSR_VECTPENDING_W                                           9
#define CPU_SCS_ICSR_VECTPENDING_M                                  0x001FF000
#define CPU_SCS_ICSR_VECTPENDING_S                                          12

// Field:    [11] RETTOBASE
//
// Indicates whether there are preempted active exceptions:
//
// 0: There are preempted active exceptions to execute
// 1: There are no active exceptions, or the currently-executing exception is
// the only active exception.
#define CPU_SCS_ICSR_RETTOBASE                                      0x00000800
#define CPU_SCS_ICSR_RETTOBASE_BITN                                         11
#define CPU_SCS_ICSR_RETTOBASE_M                                    0x00000800
#define CPU_SCS_ICSR_RETTOBASE_S                                            11

// Field:   [8:0] VECTACTIVE
//
// Active ISR number field. Reset clears this field.
#define CPU_SCS_ICSR_VECTACTIVE_W                                            9
#define CPU_SCS_ICSR_VECTACTIVE_M                                   0x000001FF
#define CPU_SCS_ICSR_VECTACTIVE_S                                            0

//*****************************************************************************
//
// Register: CPU_SCS_O_VTOR
//
//*****************************************************************************
// Field:  [31:7] TBLOFF
//
// Bits 31 down to 7 of the vector table base offset.
#define CPU_SCS_VTOR_TBLOFF_W                                               25
#define CPU_SCS_VTOR_TBLOFF_M                                       0xFFFFFF80
#define CPU_SCS_VTOR_TBLOFF_S                                                7

//*****************************************************************************
//
// Register: CPU_SCS_O_AIRCR
//
//*****************************************************************************
// Field: [31:16] VECTKEY
//
// Register key. Writing to this register (AIRCR) requires 0x05FA in VECTKEY.
// Otherwise the write value is ignored. Read always returns 0xFA05.
#define CPU_SCS_AIRCR_VECTKEY_W                                             16
#define CPU_SCS_AIRCR_VECTKEY_M                                     0xFFFF0000
#define CPU_SCS_AIRCR_VECTKEY_S                                             16

// Field:    [15] ENDIANESS
//
// Data endianness bit
// ENUMs:
// BIG                      Big endian
// LITTLE                   Little endian
#define CPU_SCS_AIRCR_ENDIANESS                                     0x00008000
#define CPU_SCS_AIRCR_ENDIANESS_BITN                                        15
#define CPU_SCS_AIRCR_ENDIANESS_M                                   0x00008000
#define CPU_SCS_AIRCR_ENDIANESS_S                                           15
#define CPU_SCS_AIRCR_ENDIANESS_BIG                                 0x00008000
#define CPU_SCS_AIRCR_ENDIANESS_LITTLE                              0x00000000

// Field:    [14] PRIS
//
// Prioritize Secure exceptions. The value of this bit defines whether Secure
// exception priority boosting is
// enabled.
#define CPU_SCS_AIRCR_PRIS                                          0x00004000
#define CPU_SCS_AIRCR_PRIS_BITN                                             14
#define CPU_SCS_AIRCR_PRIS_M                                        0x00004000
#define CPU_SCS_AIRCR_PRIS_S                                                14

// Field:    [13] BFHFNMINS
//
// BusFault, HardFault, and NMI Non-secure enable. The value of this bit
// defines whether BusFault and NMI
// exceptions are Non-secure, and whether exceptions target the Non-secure
// HardFault exception
// 0x0 BusFault, HardFault, and NMI are Secure.
// 0x1 BusFault and NMI are Non-secure and exceptions can target Non-secure
// HardFault.
#define CPU_SCS_AIRCR_BFHFNMINS                                     0x00002000
#define CPU_SCS_AIRCR_BFHFNMINS_BITN                                        13
#define CPU_SCS_AIRCR_BFHFNMINS_M                                   0x00002000
#define CPU_SCS_AIRCR_BFHFNMINS_S                                           13

// Field:  [10:8] PRIGROUP
//
// Interrupt priority grouping field. This field is a binary point position
// indicator for creating subpriorities for exceptions that share the same
// pre-emption level. It divides the PRI_n field in the Interrupt Priority
// Registers (NVIC_IPR0, NVIC_IPR1,..., and  NVIC_IPR8) into a pre-emption
// level and a subpriority level. The binary point is a left-of value. This
// means that the PRIGROUP value represents a point starting at the left of the
// Least Significant Bit (LSB). The lowest value might not be 0 depending on
// the number of bits allocated for priorities, and implementation choices.
#define CPU_SCS_AIRCR_PRIGROUP_W                                             3
#define CPU_SCS_AIRCR_PRIGROUP_M                                    0x00000700
#define CPU_SCS_AIRCR_PRIGROUP_S                                             8

// Field:     [3] SYSRESETREQS
//
// System reset request Secure only. The value of this bit defines whether the
// SYSRESETREQ bit is functional
// for Non-secure use
#define CPU_SCS_AIRCR_SYSRESETREQS                                  0x00000008
#define CPU_SCS_AIRCR_SYSRESETREQS_BITN                                      3
#define CPU_SCS_AIRCR_SYSRESETREQS_M                                0x00000008
#define CPU_SCS_AIRCR_SYSRESETREQS_S                                         3

// Field:     [2] SYSRESETREQ
//
// Requests a warm reset. Setting this bit does not prevent Halting Debug from
// running.
#define CPU_SCS_AIRCR_SYSRESETREQ                                   0x00000004
#define CPU_SCS_AIRCR_SYSRESETREQ_BITN                                       2
#define CPU_SCS_AIRCR_SYSRESETREQ_M                                 0x00000004
#define CPU_SCS_AIRCR_SYSRESETREQ_S                                          2

// Field:     [1] VECTCLRACTIVE
//
// Clears all active state information for active NMI, fault, and interrupts.
// It is the responsibility of the application to reinitialize the stack. This
// bit is for returning to a known state during debug. The bit self-clears.
// IPSR is not cleared by this operation. So, if used by an application, it
// must only be used at the base level of activation, or within a system
// handler whose active bit can be set.
#define CPU_SCS_AIRCR_VECTCLRACTIVE                                 0x00000002
#define CPU_SCS_AIRCR_VECTCLRACTIVE_BITN                                     1
#define CPU_SCS_AIRCR_VECTCLRACTIVE_M                               0x00000002
#define CPU_SCS_AIRCR_VECTCLRACTIVE_S                                        1

//*****************************************************************************
//
// Register: CPU_SCS_O_SCR
//
//*****************************************************************************
// Field:     [4] SEVONPEND
//
// Send Event on Pending bit:
//
// 0: Only enabled interrupts or events can wakeup the processor, disabled
// interrupts are excluded
// 1: Enabled events and all interrupts, including disabled interrupts, can
// wakeup the processor.
//
// When an event or interrupt enters pending state, the event signal wakes up
// the processor from WFE. If
// the processor is not waiting for an event, the event is registered and
// affects the next WFE.
// The processor also wakes up on execution of an SEV instruction.
#define CPU_SCS_SCR_SEVONPEND                                       0x00000010
#define CPU_SCS_SCR_SEVONPEND_BITN                                           4
#define CPU_SCS_SCR_SEVONPEND_M                                     0x00000010
#define CPU_SCS_SCR_SEVONPEND_S                                              4

// Field:     [3] SLEEPDEEPS
//
// Sleep deep secure. This field controls whether the SLEEPDEEP bit is only
// accessible from the Secure state
#define CPU_SCS_SCR_SLEEPDEEPS                                      0x00000008
#define CPU_SCS_SCR_SLEEPDEEPS_BITN                                          3
#define CPU_SCS_SCR_SLEEPDEEPS_M                                    0x00000008
#define CPU_SCS_SCR_SLEEPDEEPS_S                                             3

// Field:     [2] SLEEPDEEP
//
// Controls whether the processor uses sleep or deep sleep as its low power
// mode
// ENUMs:
// DEEPSLEEP                Deep sleep
// SLEEP                    Sleep
#define CPU_SCS_SCR_SLEEPDEEP                                       0x00000004
#define CPU_SCS_SCR_SLEEPDEEP_BITN                                           2
#define CPU_SCS_SCR_SLEEPDEEP_M                                     0x00000004
#define CPU_SCS_SCR_SLEEPDEEP_S                                              2
#define CPU_SCS_SCR_SLEEPDEEP_DEEPSLEEP                             0x00000004
#define CPU_SCS_SCR_SLEEPDEEP_SLEEP                                 0x00000000

// Field:     [1] SLEEPONEXIT
//
// Sleep on exit when returning from Handler mode to Thread mode. Enables
// interrupt driven applications to avoid returning to empty main application.
//
// 0: Do not sleep when returning to thread mode
// 1: Sleep on ISR exit
#define CPU_SCS_SCR_SLEEPONEXIT                                     0x00000002
#define CPU_SCS_SCR_SLEEPONEXIT_BITN                                         1
#define CPU_SCS_SCR_SLEEPONEXIT_M                                   0x00000002
#define CPU_SCS_SCR_SLEEPONEXIT_S                                            1

//*****************************************************************************
//
// Register: CPU_SCS_O_CCR
//
//*****************************************************************************
// Field:     [8] BFHFNMIGN
//
// Enables handlers with priority -1 or -2 to ignore data BusFaults caused by
// load and store instructions. This applies to the HardFault, NMI, and
// FAULTMASK escalated handlers:
//
// 0: Data BusFaults caused by load and store instructions cause a lock-up
// 1: Data BusFaults caused by load and store instructions are ignored.
//
// Set this bit to 1 only when the handler and its data are in absolutely safe
// memory. The normal use
// of this bit is to probe system devices and bridges to detect problems.
#define CPU_SCS_CCR_BFHFNMIGN                                       0x00000100
#define CPU_SCS_CCR_BFHFNMIGN_BITN                                           8
#define CPU_SCS_CCR_BFHFNMIGN_M                                     0x00000100
#define CPU_SCS_CCR_BFHFNMIGN_S                                              8

// Field:     [4] DIV_0_TRP
//
// Enables faulting or halting when the processor executes an SDIV or UDIV
// instruction with a divisor of 0:
//
// 0: Do not trap divide by 0. In this mode, a divide by zero returns a
// quotient of 0.
// 1: Trap divide by 0. The relevant Usage Fault Status Register bit is
// CFSR.DIVBYZERO.
#define CPU_SCS_CCR_DIV_0_TRP                                       0x00000010
#define CPU_SCS_CCR_DIV_0_TRP_BITN                                           4
#define CPU_SCS_CCR_DIV_0_TRP_M                                     0x00000010
#define CPU_SCS_CCR_DIV_0_TRP_S                                              4

// Field:     [3] UNALIGN_TRP
//
// Enables unaligned access traps:
//
// 0: Do not trap unaligned halfword and word accesses
// 1: Trap unaligned halfword and word accesses. The relevant Usage Fault
// Status Register bit is CFSR.UNALIGNED.
//
// If this bit is set to 1, an unaligned access generates a UsageFault.
// Unaligned LDM, STM, LDRD, and STRD instructions always fault regardless of
// the value in UNALIGN_TRP.
#define CPU_SCS_CCR_UNALIGN_TRP                                     0x00000008
#define CPU_SCS_CCR_UNALIGN_TRP_BITN                                         3
#define CPU_SCS_CCR_UNALIGN_TRP_M                                   0x00000008
#define CPU_SCS_CCR_UNALIGN_TRP_S                                            3

// Field:     [1] USERSETMPEND
//
// Enables unprivileged software access to STIR:
//
// 0: User code is not allowed to write to the Software Trigger Interrupt
// register (STIR).
// 1: User code can write the Software Trigger Interrupt register (STIR) to
// trigger (pend) a Main exception, which is associated with the Main stack
// pointer.
#define CPU_SCS_CCR_USERSETMPEND                                    0x00000002
#define CPU_SCS_CCR_USERSETMPEND_BITN                                        1
#define CPU_SCS_CCR_USERSETMPEND_M                                  0x00000002
#define CPU_SCS_CCR_USERSETMPEND_S                                           1

//*****************************************************************************
//
// Register: CPU_SCS_O_SHPR1
//
//*****************************************************************************
// Field: [23:16] PRI_6
//
// Priority of system handler 6. UsageFault
#define CPU_SCS_SHPR1_PRI_6_W                                                8
#define CPU_SCS_SHPR1_PRI_6_M                                       0x00FF0000
#define CPU_SCS_SHPR1_PRI_6_S                                               16

// Field:  [15:8] PRI_5
//
// Priority of system handler 5: BusFault
#define CPU_SCS_SHPR1_PRI_5_W                                                8
#define CPU_SCS_SHPR1_PRI_5_M                                       0x0000FF00
#define CPU_SCS_SHPR1_PRI_5_S                                                8

// Field:   [7:0] PRI_4
//
// Priority of system handler 4: MemManage
#define CPU_SCS_SHPR1_PRI_4_W                                                8
#define CPU_SCS_SHPR1_PRI_4_M                                       0x000000FF
#define CPU_SCS_SHPR1_PRI_4_S                                                0

//*****************************************************************************
//
// Register: CPU_SCS_O_SHPR2
//
//*****************************************************************************
// Field: [31:24] PRI_11
//
// Priority of system handler 11. SVCall
#define CPU_SCS_SHPR2_PRI_11_W                                               8
#define CPU_SCS_SHPR2_PRI_11_M                                      0xFF000000
#define CPU_SCS_SHPR2_PRI_11_S                                              24

//*****************************************************************************
//
// Register: CPU_SCS_O_SHPR3
//
//*****************************************************************************
// Field: [31:24] PRI_15
//
// Priority of system handler 15. SysTick exception
#define CPU_SCS_SHPR3_PRI_15_W                                               8
#define CPU_SCS_SHPR3_PRI_15_M                                      0xFF000000
#define CPU_SCS_SHPR3_PRI_15_S                                              24

// Field: [23:16] PRI_14
//
// Priority of system handler 14. Pend SV
#define CPU_SCS_SHPR3_PRI_14_W                                               8
#define CPU_SCS_SHPR3_PRI_14_M                                      0x00FF0000
#define CPU_SCS_SHPR3_PRI_14_S                                              16

// Field:   [7:0] PRI_12
//
// Priority of system handler 12. Debug Monitor
#define CPU_SCS_SHPR3_PRI_12_W                                               8
#define CPU_SCS_SHPR3_PRI_12_M                                      0x000000FF
#define CPU_SCS_SHPR3_PRI_12_S                                               0

//*****************************************************************************
//
// Register: CPU_SCS_O_SHCSR
//
//*****************************************************************************
// Field:    [21] HARDFAULTPENDED
//
// SecureFault exception pended state
// ENUMs:
// PENDING                  Exception is pending.
// NOTPENDING               Exception is not active
#define CPU_SCS_SHCSR_HARDFAULTPENDED                               0x00200000
#define CPU_SCS_SHCSR_HARDFAULTPENDED_BITN                                  21
#define CPU_SCS_SHCSR_HARDFAULTPENDED_M                             0x00200000
#define CPU_SCS_SHCSR_HARDFAULTPENDED_S                                     21
#define CPU_SCS_SHCSR_HARDFAULTPENDED_PENDING                       0x00200000
#define CPU_SCS_SHCSR_HARDFAULTPENDED_NOTPENDING                    0x00000000

// Field:    [20] SECUREFAULTPENDED
//
// SecureFault exception pended state
// ENUMs:
// PENDING                  Exception is pending.
// NOTPENDING               Exception is not active
#define CPU_SCS_SHCSR_SECUREFAULTPENDED                             0x00100000
#define CPU_SCS_SHCSR_SECUREFAULTPENDED_BITN                                20
#define CPU_SCS_SHCSR_SECUREFAULTPENDED_M                           0x00100000
#define CPU_SCS_SHCSR_SECUREFAULTPENDED_S                                   20
#define CPU_SCS_SHCSR_SECUREFAULTPENDED_PENDING                     0x00100000
#define CPU_SCS_SHCSR_SECUREFAULTPENDED_NOTPENDING                  0x00000000

// Field:    [19] SECUREFAULTENA
//
// SecureFault exception enable.
// ENUMs:
// EN                       Exception enabled
// DIS                      Exception disabled
#define CPU_SCS_SHCSR_SECUREFAULTENA                                0x00080000
#define CPU_SCS_SHCSR_SECUREFAULTENA_BITN                                   19
#define CPU_SCS_SHCSR_SECUREFAULTENA_M                              0x00080000
#define CPU_SCS_SHCSR_SECUREFAULTENA_S                                      19
#define CPU_SCS_SHCSR_SECUREFAULTENA_EN                             0x00080000
#define CPU_SCS_SHCSR_SECUREFAULTENA_DIS                            0x00000000

// Field:    [18] USGFAULTENA
//
// Usage fault system handler enable
// ENUMs:
// EN                       Exception enabled
// DIS                      Exception disabled
#define CPU_SCS_SHCSR_USGFAULTENA                                   0x00040000
#define CPU_SCS_SHCSR_USGFAULTENA_BITN                                      18
#define CPU_SCS_SHCSR_USGFAULTENA_M                                 0x00040000
#define CPU_SCS_SHCSR_USGFAULTENA_S                                         18
#define CPU_SCS_SHCSR_USGFAULTENA_EN                                0x00040000
#define CPU_SCS_SHCSR_USGFAULTENA_DIS                               0x00000000

// Field:    [17] BUSFAULTENA
//
// Bus fault system handler enable
// ENUMs:
// EN                       Exception enabled
// DIS                      Exception disabled
#define CPU_SCS_SHCSR_BUSFAULTENA                                   0x00020000
#define CPU_SCS_SHCSR_BUSFAULTENA_BITN                                      17
#define CPU_SCS_SHCSR_BUSFAULTENA_M                                 0x00020000
#define CPU_SCS_SHCSR_BUSFAULTENA_S                                         17
#define CPU_SCS_SHCSR_BUSFAULTENA_EN                                0x00020000
#define CPU_SCS_SHCSR_BUSFAULTENA_DIS                               0x00000000

// Field:    [16] MEMFAULTENA
//
// MemManage fault system handler enable
// ENUMs:
// EN                       Exception enabled
// DIS                      Exception disabled
#define CPU_SCS_SHCSR_MEMFAULTENA                                   0x00010000
#define CPU_SCS_SHCSR_MEMFAULTENA_BITN                                      16
#define CPU_SCS_SHCSR_MEMFAULTENA_M                                 0x00010000
#define CPU_SCS_SHCSR_MEMFAULTENA_S                                         16
#define CPU_SCS_SHCSR_MEMFAULTENA_EN                                0x00010000
#define CPU_SCS_SHCSR_MEMFAULTENA_DIS                               0x00000000

// Field:    [15] SVCALLPENDED
//
// SVCall pending
// ENUMs:
// PENDING                  Exception is pending.
// NOTPENDING               Exception is not active
#define CPU_SCS_SHCSR_SVCALLPENDED                                  0x00008000
#define CPU_SCS_SHCSR_SVCALLPENDED_BITN                                     15
#define CPU_SCS_SHCSR_SVCALLPENDED_M                                0x00008000
#define CPU_SCS_SHCSR_SVCALLPENDED_S                                        15
#define CPU_SCS_SHCSR_SVCALLPENDED_PENDING                          0x00008000
#define CPU_SCS_SHCSR_SVCALLPENDED_NOTPENDING                       0x00000000

// Field:    [14] BUSFAULTPENDED
//
// BusFault pending
// ENUMs:
// PENDING                  Exception is pending.
// NOTPENDING               Exception is not active
#define CPU_SCS_SHCSR_BUSFAULTPENDED                                0x00004000
#define CPU_SCS_SHCSR_BUSFAULTPENDED_BITN                                   14
#define CPU_SCS_SHCSR_BUSFAULTPENDED_M                              0x00004000
#define CPU_SCS_SHCSR_BUSFAULTPENDED_S                                      14
#define CPU_SCS_SHCSR_BUSFAULTPENDED_PENDING                        0x00004000
#define CPU_SCS_SHCSR_BUSFAULTPENDED_NOTPENDING                     0x00000000

// Field:    [13] MEMFAULTPENDED
//
// MemManage exception pending
// ENUMs:
// PENDING                  Exception is pending.
// NOTPENDING               Exception is not active
#define CPU_SCS_SHCSR_MEMFAULTPENDED                                0x00002000
#define CPU_SCS_SHCSR_MEMFAULTPENDED_BITN                                   13
#define CPU_SCS_SHCSR_MEMFAULTPENDED_M                              0x00002000
#define CPU_SCS_SHCSR_MEMFAULTPENDED_S                                      13
#define CPU_SCS_SHCSR_MEMFAULTPENDED_PENDING                        0x00002000
#define CPU_SCS_SHCSR_MEMFAULTPENDED_NOTPENDING                     0x00000000

// Field:    [12] USGFAULTPENDED
//
// Usage fault pending
// ENUMs:
// PENDING                  Exception is pending.
// NOTPENDING               Exception is not active
#define CPU_SCS_SHCSR_USGFAULTPENDED                                0x00001000
#define CPU_SCS_SHCSR_USGFAULTPENDED_BITN                                   12
#define CPU_SCS_SHCSR_USGFAULTPENDED_M                              0x00001000
#define CPU_SCS_SHCSR_USGFAULTPENDED_S                                      12
#define CPU_SCS_SHCSR_USGFAULTPENDED_PENDING                        0x00001000
#define CPU_SCS_SHCSR_USGFAULTPENDED_NOTPENDING                     0x00000000

// Field:    [11] SYSTICKACT
//
// SysTick active flag.
//
// 0x0: Not active
// 0x1: Active
// ENUMs:
// ACTIVE                   Exception is active
// NOTACTIVE                Exception is not active
#define CPU_SCS_SHCSR_SYSTICKACT                                    0x00000800
#define CPU_SCS_SHCSR_SYSTICKACT_BITN                                       11
#define CPU_SCS_SHCSR_SYSTICKACT_M                                  0x00000800
#define CPU_SCS_SHCSR_SYSTICKACT_S                                          11
#define CPU_SCS_SHCSR_SYSTICKACT_ACTIVE                             0x00000800
#define CPU_SCS_SHCSR_SYSTICKACT_NOTACTIVE                          0x00000000

// Field:    [10] PENDSVACT
//
// PendSV active
//
// 0x0: Not active
// 0x1: Active
#define CPU_SCS_SHCSR_PENDSVACT                                     0x00000400
#define CPU_SCS_SHCSR_PENDSVACT_BITN                                        10
#define CPU_SCS_SHCSR_PENDSVACT_M                                   0x00000400
#define CPU_SCS_SHCSR_PENDSVACT_S                                           10

// Field:     [8] MONITORACT
//
// Debug monitor active
// ENUMs:
// ACTIVE                   Exception is active
// NOTACTIVE                Exception is not active
#define CPU_SCS_SHCSR_MONITORACT                                    0x00000100
#define CPU_SCS_SHCSR_MONITORACT_BITN                                        8
#define CPU_SCS_SHCSR_MONITORACT_M                                  0x00000100
#define CPU_SCS_SHCSR_MONITORACT_S                                           8
#define CPU_SCS_SHCSR_MONITORACT_ACTIVE                             0x00000100
#define CPU_SCS_SHCSR_MONITORACT_NOTACTIVE                          0x00000000

// Field:     [7] SVCALLACT
//
// SVCall active
// ENUMs:
// ACTIVE                   Exception is active
// NOTACTIVE                Exception is not active
#define CPU_SCS_SHCSR_SVCALLACT                                     0x00000080
#define CPU_SCS_SHCSR_SVCALLACT_BITN                                         7
#define CPU_SCS_SHCSR_SVCALLACT_M                                   0x00000080
#define CPU_SCS_SHCSR_SVCALLACT_S                                            7
#define CPU_SCS_SHCSR_SVCALLACT_ACTIVE                              0x00000080
#define CPU_SCS_SHCSR_SVCALLACT_NOTACTIVE                           0x00000000

// Field:     [5] NMIACT
//
// NMI exception active state
// ENUMs:
// ACTIVE                   Exception is active
// NOTACTIVE                Exception is not active
#define CPU_SCS_SHCSR_NMIACT                                        0x00000020
#define CPU_SCS_SHCSR_NMIACT_BITN                                            5
#define CPU_SCS_SHCSR_NMIACT_M                                      0x00000020
#define CPU_SCS_SHCSR_NMIACT_S                                               5
#define CPU_SCS_SHCSR_NMIACT_ACTIVE                                 0x00000020
#define CPU_SCS_SHCSR_NMIACT_NOTACTIVE                              0x00000000

// Field:     [4] SECUREFAULTACT
//
// SecureFault exception active state
// ENUMs:
// ACTIVE                   Exception is active
// NOTACTIVE                Exception is not active
#define CPU_SCS_SHCSR_SECUREFAULTACT                                0x00000010
#define CPU_SCS_SHCSR_SECUREFAULTACT_BITN                                    4
#define CPU_SCS_SHCSR_SECUREFAULTACT_M                              0x00000010
#define CPU_SCS_SHCSR_SECUREFAULTACT_S                                       4
#define CPU_SCS_SHCSR_SECUREFAULTACT_ACTIVE                         0x00000010
#define CPU_SCS_SHCSR_SECUREFAULTACT_NOTACTIVE                      0x00000000

// Field:     [3] USGFAULTACT
//
// UsageFault exception active
// ENUMs:
// ACTIVE                   Exception is active
// NOTACTIVE                Exception is not active
#define CPU_SCS_SHCSR_USGFAULTACT                                   0x00000008
#define CPU_SCS_SHCSR_USGFAULTACT_BITN                                       3
#define CPU_SCS_SHCSR_USGFAULTACT_M                                 0x00000008
#define CPU_SCS_SHCSR_USGFAULTACT_S                                          3
#define CPU_SCS_SHCSR_USGFAULTACT_ACTIVE                            0x00000008
#define CPU_SCS_SHCSR_USGFAULTACT_NOTACTIVE                         0x00000000

// Field:     [2] HARDFAULTACT
//
// HardFault exception active state. Indicates and allows limited modification
// of the active state of the HardFault exception for the selected Security
// state
// ENUMs:
// ACTIVE                   Exception is active
// NOTACTIVE                Exception is not active
#define CPU_SCS_SHCSR_HARDFAULTACT                                  0x00000004
#define CPU_SCS_SHCSR_HARDFAULTACT_BITN                                      2
#define CPU_SCS_SHCSR_HARDFAULTACT_M                                0x00000004
#define CPU_SCS_SHCSR_HARDFAULTACT_S                                         2
#define CPU_SCS_SHCSR_HARDFAULTACT_ACTIVE                           0x00000004
#define CPU_SCS_SHCSR_HARDFAULTACT_NOTACTIVE                        0x00000000

// Field:     [1] BUSFAULTACT
//
// BusFault exception active
// ENUMs:
// ACTIVE                   Exception is active
// NOTACTIVE                Exception is not active
#define CPU_SCS_SHCSR_BUSFAULTACT                                   0x00000002
#define CPU_SCS_SHCSR_BUSFAULTACT_BITN                                       1
#define CPU_SCS_SHCSR_BUSFAULTACT_M                                 0x00000002
#define CPU_SCS_SHCSR_BUSFAULTACT_S                                          1
#define CPU_SCS_SHCSR_BUSFAULTACT_ACTIVE                            0x00000002
#define CPU_SCS_SHCSR_BUSFAULTACT_NOTACTIVE                         0x00000000

// Field:     [0] MEMFAULTACT
//
// MemManage exception active
// ENUMs:
// ACTIVE                   Exception is active
// NOTACTIVE                Exception is not active
#define CPU_SCS_SHCSR_MEMFAULTACT                                   0x00000001
#define CPU_SCS_SHCSR_MEMFAULTACT_BITN                                       0
#define CPU_SCS_SHCSR_MEMFAULTACT_M                                 0x00000001
#define CPU_SCS_SHCSR_MEMFAULTACT_S                                          0
#define CPU_SCS_SHCSR_MEMFAULTACT_ACTIVE                            0x00000001
#define CPU_SCS_SHCSR_MEMFAULTACT_NOTACTIVE                         0x00000000

//*****************************************************************************
//
// Register: CPU_SCS_O_CFSR
//
//*****************************************************************************
// Field:    [25] DIVBYZERO
//
// When CCR.DIV_0_TRP (see Configuration Control Register on page 8-26) is
// enabled and an SDIV or UDIV instruction is used with a divisor of 0, this
// fault occurs The instruction is executed and the return PC points to it. If
// CCR.DIV_0_TRP is not set, then the divide returns a quotient of 0.
#define CPU_SCS_CFSR_DIVBYZERO                                      0x02000000
#define CPU_SCS_CFSR_DIVBYZERO_BITN                                         25
#define CPU_SCS_CFSR_DIVBYZERO_M                                    0x02000000
#define CPU_SCS_CFSR_DIVBYZERO_S                                            25

// Field:    [24] UNALIGNED
//
// When CCR.UNALIGN_TRP is enabled, and there is an attempt to make an
// unaligned memory access, then this fault occurs. Unaligned LDM/STM/LDRD/STRD
// instructions always fault irrespective of the setting of CCR.UNALIGN_TRP.
#define CPU_SCS_CFSR_UNALIGNED                                      0x01000000
#define CPU_SCS_CFSR_UNALIGNED_BITN                                         24
#define CPU_SCS_CFSR_UNALIGNED_M                                    0x01000000
#define CPU_SCS_CFSR_UNALIGNED_S                                            24

// Field:    [19] NOCP
//
// Attempt to use a coprocessor instruction. The processor does not support
// coprocessor instructions.
#define CPU_SCS_CFSR_NOCP                                           0x00080000
#define CPU_SCS_CFSR_NOCP_BITN                                              19
#define CPU_SCS_CFSR_NOCP_M                                         0x00080000
#define CPU_SCS_CFSR_NOCP_S                                                 19

// Field:    [18] INVPC
//
// Attempt to load EXC_RETURN into PC illegally. Invalid instruction, invalid
// context, invalid value. The return PC points to the instruction that tried
// to set the PC.
#define CPU_SCS_CFSR_INVPC                                          0x00040000
#define CPU_SCS_CFSR_INVPC_BITN                                             18
#define CPU_SCS_CFSR_INVPC_M                                        0x00040000
#define CPU_SCS_CFSR_INVPC_S                                                18

// Field:    [17] INVSTATE
//
// Indicates an attempt to execute in an invalid EPSR state (e.g. after a BX
// type instruction has changed state). This includes state change after entry
// to or return from exception, as well as from inter-working instructions.
// Return PC points to faulting instruction, with the invalid state.
#define CPU_SCS_CFSR_INVSTATE                                       0x00020000
#define CPU_SCS_CFSR_INVSTATE_BITN                                          17
#define CPU_SCS_CFSR_INVSTATE_M                                     0x00020000
#define CPU_SCS_CFSR_INVSTATE_S                                             17

// Field:    [16] UNDEFINSTR
//
// This bit is set when the processor attempts to execute an undefined
// instruction. This is an instruction that the processor cannot decode. The
// return PC points to the undefined instruction.
#define CPU_SCS_CFSR_UNDEFINSTR                                     0x00010000
#define CPU_SCS_CFSR_UNDEFINSTR_BITN                                        16
#define CPU_SCS_CFSR_UNDEFINSTR_M                                   0x00010000
#define CPU_SCS_CFSR_UNDEFINSTR_S                                           16

// Field:    [15] BFARVALID
//
// This bit is set if the Bus Fault Address Register (BFAR) contains a valid
// address. This is true after a bus fault where the address is known. Other
// faults can clear this bit, such as a Mem Manage fault occurring later. If a
// Bus fault occurs that is escalated to a Hard Fault because of priority, the
// Hard Fault handler must clear this bit. This prevents problems if returning
// to a stacked active Bus fault handler whose BFAR value has been overwritten.
#define CPU_SCS_CFSR_BFARVALID                                      0x00008000
#define CPU_SCS_CFSR_BFARVALID_BITN                                         15
#define CPU_SCS_CFSR_BFARVALID_M                                    0x00008000
#define CPU_SCS_CFSR_BFARVALID_S                                            15

// Field:    [12] STKERR
//
// Stacking from exception has caused one or more bus faults. The SP is still
// adjusted and the values in the context area on the stack might be incorrect.
// BFAR is not written.
#define CPU_SCS_CFSR_STKERR                                         0x00001000
#define CPU_SCS_CFSR_STKERR_BITN                                            12
#define CPU_SCS_CFSR_STKERR_M                                       0x00001000
#define CPU_SCS_CFSR_STKERR_S                                               12

// Field:    [11] UNSTKERR
//
// Unstack from exception return has caused one or more bus faults. This is
// chained to the handler, so that the original return stack is still present.
// SP is not adjusted from failing return and new save is not performed. BFAR
// is not written.
#define CPU_SCS_CFSR_UNSTKERR                                       0x00000800
#define CPU_SCS_CFSR_UNSTKERR_BITN                                          11
#define CPU_SCS_CFSR_UNSTKERR_M                                     0x00000800
#define CPU_SCS_CFSR_UNSTKERR_S                                             11

// Field:    [10] IMPRECISERR
//
// Imprecise data bus error. It is a BusFault, but the Return PC is not related
// to the causing instruction. This is not a synchronous fault. So, if detected
// when the priority of the current activation is higher than the Bus Fault, it
// only pends. Bus fault activates when returning to a lower priority
// activation. If a precise fault occurs before returning to a lower priority
// exception, the handler detects both IMPRECISERR set and one of the precise
// fault status bits set at the same time. BFAR is not written.
#define CPU_SCS_CFSR_IMPRECISERR                                    0x00000400
#define CPU_SCS_CFSR_IMPRECISERR_BITN                                       10
#define CPU_SCS_CFSR_IMPRECISERR_M                                  0x00000400
#define CPU_SCS_CFSR_IMPRECISERR_S                                          10

// Field:     [9] PRECISERR
//
// Precise data bus error return.
#define CPU_SCS_CFSR_PRECISERR                                      0x00000200
#define CPU_SCS_CFSR_PRECISERR_BITN                                          9
#define CPU_SCS_CFSR_PRECISERR_M                                    0x00000200
#define CPU_SCS_CFSR_PRECISERR_S                                             9

// Field:     [8] IBUSERR
//
// Instruction bus error flag. This flag is set by a prefetch error. The fault
// stops on the instruction, so if the error occurs under a branch shadow, no
// fault occurs. BFAR is not written.
#define CPU_SCS_CFSR_IBUSERR                                        0x00000100
#define CPU_SCS_CFSR_IBUSERR_BITN                                            8
#define CPU_SCS_CFSR_IBUSERR_M                                      0x00000100
#define CPU_SCS_CFSR_IBUSERR_S                                               8

// Field:     [7] MMARVALID
//
// Memory Manage Address Register (MMFAR) address valid flag. A later-arriving
// fault, such as a bus fault, can clear a memory manage fault.. If a MemManage
// fault occurs that is escalated to a Hard Fault because of priority, the Hard
// Fault handler must clear this bit. This prevents problems on return to a
// stacked active MemManage handler whose MMFAR value has been overwritten.
#define CPU_SCS_CFSR_MMARVALID                                      0x00000080
#define CPU_SCS_CFSR_MMARVALID_BITN                                          7
#define CPU_SCS_CFSR_MMARVALID_M                                    0x00000080
#define CPU_SCS_CFSR_MMARVALID_S                                             7

// Field:     [4] MSTKERR
//
// Stacking from exception has caused one or more access violations. The SP is
// still adjusted and the values in the context area on the stack might be
// incorrect. MMFAR is not written.
#define CPU_SCS_CFSR_MSTKERR                                        0x00000010
#define CPU_SCS_CFSR_MSTKERR_BITN                                            4
#define CPU_SCS_CFSR_MSTKERR_M                                      0x00000010
#define CPU_SCS_CFSR_MSTKERR_S                                               4

// Field:     [3] MUNSTKERR
//
// Unstack from exception return has caused one or more access violations. This
// is chained to the handler, so that the original return stack is still
// present. SP is not adjusted from failing return and new save is not
// performed. MMFAR is not written.
#define CPU_SCS_CFSR_MUNSTKERR                                      0x00000008
#define CPU_SCS_CFSR_MUNSTKERR_BITN                                          3
#define CPU_SCS_CFSR_MUNSTKERR_M                                    0x00000008
#define CPU_SCS_CFSR_MUNSTKERR_S                                             3

// Field:     [1] DACCVIOL
//
// Data access violation flag. Attempting to load or store at a location that
// does not permit the operation sets this flag. The return PC points to the
// faulting instruction. This error loads MMFAR with the address of the
// attempted access.
#define CPU_SCS_CFSR_DACCVIOL                                       0x00000002
#define CPU_SCS_CFSR_DACCVIOL_BITN                                           1
#define CPU_SCS_CFSR_DACCVIOL_M                                     0x00000002
#define CPU_SCS_CFSR_DACCVIOL_S                                              1

// Field:     [0] IACCVIOL
//
// Instruction access violation flag. Attempting to fetch an instruction from a
// location that does not permit execution sets this flag. This occurs on any
// access to an XN region, even when the MPU is disabled or not present. The
// return PC points to the faulting instruction. MMFAR is not written.
#define CPU_SCS_CFSR_IACCVIOL                                       0x00000001
#define CPU_SCS_CFSR_IACCVIOL_BITN                                           0
#define CPU_SCS_CFSR_IACCVIOL_M                                     0x00000001
#define CPU_SCS_CFSR_IACCVIOL_S                                              0

//*****************************************************************************
//
// Register: CPU_SCS_O_HFSR
//
//*****************************************************************************
// Field:    [31] DEBUGEVT
//
// This bit is set if there is a fault related to debug. This is only possible
// when halting debug is not enabled. For monitor enabled debug, it only
// happens for BKPT when the current priority is higher than the monitor. When
// both halting and monitor debug are disabled, it only happens for debug
// events that are not ignored (minimally, BKPT). The Debug Fault Status
// Register is updated.
#define CPU_SCS_HFSR_DEBUGEVT                                       0x80000000
#define CPU_SCS_HFSR_DEBUGEVT_BITN                                          31
#define CPU_SCS_HFSR_DEBUGEVT_M                                     0x80000000
#define CPU_SCS_HFSR_DEBUGEVT_S                                             31

// Field:    [30] FORCED
//
// Hard Fault activated because a Configurable Fault was received and cannot
// activate because of priority or because the Configurable Fault is disabled.
// The Hard Fault handler then has to read the other fault status registers to
// determine cause.
#define CPU_SCS_HFSR_FORCED                                         0x40000000
#define CPU_SCS_HFSR_FORCED_BITN                                            30
#define CPU_SCS_HFSR_FORCED_M                                       0x40000000
#define CPU_SCS_HFSR_FORCED_S                                               30

// Field:     [1] VECTTBL
//
// This bit is set if there is a fault because of vector table read on
// exception processing (Bus Fault). This case is always a Hard Fault. The
// return PC points to the pre-empted instruction.
#define CPU_SCS_HFSR_VECTTBL                                        0x00000002
#define CPU_SCS_HFSR_VECTTBL_BITN                                            1
#define CPU_SCS_HFSR_VECTTBL_M                                      0x00000002
#define CPU_SCS_HFSR_VECTTBL_S                                               1

//*****************************************************************************
//
// Register: CPU_SCS_O_DFSR
//
//*****************************************************************************
// Field:     [4] EXTERNAL
//
// External debug request flag. The processor stops on next instruction
// boundary.
//
// 0x0: External debug request signal not asserted
// 0x1: External debug request signal asserted
#define CPU_SCS_DFSR_EXTERNAL                                       0x00000010
#define CPU_SCS_DFSR_EXTERNAL_BITN                                           4
#define CPU_SCS_DFSR_EXTERNAL_M                                     0x00000010
#define CPU_SCS_DFSR_EXTERNAL_S                                              4

// Field:     [3] VCATCH
//
// Vector catch flag. When this flag is set, a flag in one of the local fault
// status registers is also set to indicate the type of fault.
//
// 0x0: No vector catch occurred
// 0x1: Vector catch occurred
#define CPU_SCS_DFSR_VCATCH                                         0x00000008
#define CPU_SCS_DFSR_VCATCH_BITN                                             3
#define CPU_SCS_DFSR_VCATCH_M                                       0x00000008
#define CPU_SCS_DFSR_VCATCH_S                                                3

// Field:     [2] DWTTRAP
//
// Data Watchpoint and Trace (DWT) flag. The processor stops at the current
// instruction or at the next instruction.
//
// 0x0: No DWT match
// 0x1: DWT match
#define CPU_SCS_DFSR_DWTTRAP                                        0x00000004
#define CPU_SCS_DFSR_DWTTRAP_BITN                                            2
#define CPU_SCS_DFSR_DWTTRAP_M                                      0x00000004
#define CPU_SCS_DFSR_DWTTRAP_S                                               2

// Field:     [1] BKPT
//
// BKPT flag. The BKPT flag is set by a BKPT instruction in flash patch code,
// and also by normal code. Return PC points to breakpoint containing
// instruction.
//
// 0x0: No BKPT instruction execution
// 0x1: BKPT instruction execution
#define CPU_SCS_DFSR_BKPT                                           0x00000002
#define CPU_SCS_DFSR_BKPT_BITN                                               1
#define CPU_SCS_DFSR_BKPT_M                                         0x00000002
#define CPU_SCS_DFSR_BKPT_S                                                  1

// Field:     [0] HALTED
//
// Halt request flag. The processor is halted on the next instruction.
//
// 0x0: No halt request
// 0x1: Halt requested by NVIC, including step
#define CPU_SCS_DFSR_HALTED                                         0x00000001
#define CPU_SCS_DFSR_HALTED_BITN                                             0
#define CPU_SCS_DFSR_HALTED_M                                       0x00000001
#define CPU_SCS_DFSR_HALTED_S                                                0

//*****************************************************************************
//
// Register: CPU_SCS_O_MMFAR
//
//*****************************************************************************
// Field:  [31:0] ADDRESS
//
// Mem Manage fault address field.
// This field is the data address of a faulted load or store attempt. When an
// unaligned access faults, the address is the actual address that faulted.
// Because an access can be split into multiple parts, each aligned, this
// address can be any offset in the range of the requested size. Flags
// CFSR.IACCVIOL, CFSR.DACCVIOL ,CFSR.MUNSTKERR and CFSR.MSTKERR in combination
// with CFSR.MMARVALIDindicate the cause of the fault.
#define CPU_SCS_MMFAR_ADDRESS_W                                             32
#define CPU_SCS_MMFAR_ADDRESS_M                                     0xFFFFFFFF
#define CPU_SCS_MMFAR_ADDRESS_S                                              0

//*****************************************************************************
//
// Register: CPU_SCS_O_BFAR
//
//*****************************************************************************
// Field:  [31:0] ADDRESS
//
// Bus fault address field. This field is the data address of a faulted load or
// store attempt. When an unaligned access faults, the address is the address
// requested by the instruction, even if that is not the address that faulted.
// Flags CFSR.IBUSERR, CFSR.PRECISERR, CFSR.IMPRECISERR, CFSR.UNSTKERR and
// CFSR.STKERR in combination with CFSR.BFARVALID indicate the cause of the
// fault.
#define CPU_SCS_BFAR_ADDRESS_W                                              32
#define CPU_SCS_BFAR_ADDRESS_M                                      0xFFFFFFFF
#define CPU_SCS_BFAR_ADDRESS_S                                               0

//*****************************************************************************
//
// Register: CPU_SCS_O_AFSR
//
//*****************************************************************************
// Field:  [31:0] IMPDEF
//
// Implementation defined. The bits map directly onto the signal assignment to
// the auxiliary fault inputs. Tied to 0
#define CPU_SCS_AFSR_IMPDEF_W                                               32
#define CPU_SCS_AFSR_IMPDEF_M                                       0xFFFFFFFF
#define CPU_SCS_AFSR_IMPDEF_S                                                0

//*****************************************************************************
//
// Register: CPU_SCS_O_ID_PFR0
//
//*****************************************************************************
// Field:   [7:4] STATE1
//
// State1 (T-bit == 1)
//
// 0x0: N/A
// 0x1: N/A
// 0x2: Thumb-2 encoding with the 16-bit basic instructions plus 32-bit
// Buncond/BL but no other 32-bit basic instructions (Note non-basic 32-bit
// instructions can be added using the appropriate instruction attribute, but
// other 32-bit basic instructions cannot.)
// 0x3: Thumb-2 encoding with all Thumb-2 basic instructions
#define CPU_SCS_ID_PFR0_STATE1_W                                             4
#define CPU_SCS_ID_PFR0_STATE1_M                                    0x000000F0
#define CPU_SCS_ID_PFR0_STATE1_S                                             4

// Field:   [3:0] STATE0
//
// State0 (T-bit == 0)
//
// 0x0: No ARM encoding
// 0x1: N/A
#define CPU_SCS_ID_PFR0_STATE0_W                                             4
#define CPU_SCS_ID_PFR0_STATE0_M                                    0x0000000F
#define CPU_SCS_ID_PFR0_STATE0_S                                             0

//*****************************************************************************
//
// Register: CPU_SCS_O_ID_PFR1
//
//*****************************************************************************
// Field:  [11:8] MICROCONTROLLER_PROGRAMMERS_MODEL
//
// Microcontroller programmer's model
//
// 0x0: Not supported
// 0x2: Two-stack support
#define CPU_SCS_ID_PFR1_MICROCONTROLLER_PROGRAMMERS_MODEL_W                  4
#define CPU_SCS_ID_PFR1_MICROCONTROLLER_PROGRAMMERS_MODEL_M         0x00000F00
#define CPU_SCS_ID_PFR1_MICROCONTROLLER_PROGRAMMERS_MODEL_S                  8

// Field:   [7:4] SECURITY
//
// Security. Identifies whether the Security Extension is implemented
#define CPU_SCS_ID_PFR1_SECURITY_W                                           4
#define CPU_SCS_ID_PFR1_SECURITY_M                                  0x000000F0
#define CPU_SCS_ID_PFR1_SECURITY_S                                           4

//*****************************************************************************
//
// Register: CPU_SCS_O_ID_DFR0
//
//*****************************************************************************
// Field: [23:20] MICROCONTROLLER_DEBUG_MODEL
//
// Microcontroller Debug Model - memory mapped
//
// 0x0: Not supported
// 0x1: Microcontroller debug v1 (ITMv1 and DWTv1)
#define CPU_SCS_ID_DFR0_MICROCONTROLLER_DEBUG_MODEL_W                        4
#define CPU_SCS_ID_DFR0_MICROCONTROLLER_DEBUG_MODEL_M               0x00F00000
#define CPU_SCS_ID_DFR0_MICROCONTROLLER_DEBUG_MODEL_S                       20

//*****************************************************************************
//
// Register: CPU_SCS_O_ID_AFR0
//
//*****************************************************************************
//*****************************************************************************
//
// Register: CPU_SCS_O_ID_MMFR0
//
//*****************************************************************************
//*****************************************************************************
//
// Register: CPU_SCS_O_ID_MMFR1
//
//*****************************************************************************
//*****************************************************************************
//
// Register: CPU_SCS_O_ID_MMFR2
//
//*****************************************************************************
// Field: [27:24] WAIT_FOR_INTERRUPT_STALLING
//
// wait for interrupt stalling
//
// 0x0: Not supported
// 0x1: Wait for interrupt supported
#define CPU_SCS_ID_MMFR2_WAIT_FOR_INTERRUPT_STALLING_W                       4
#define CPU_SCS_ID_MMFR2_WAIT_FOR_INTERRUPT_STALLING_M              0x0F000000
#define CPU_SCS_ID_MMFR2_WAIT_FOR_INTERRUPT_STALLING_S                      24

//*****************************************************************************
//
// Register: CPU_SCS_O_ID_MMFR3
//
//*****************************************************************************
//*****************************************************************************
//
// Register: CPU_SCS_O_ID_ISAR0
//
//*****************************************************************************
//*****************************************************************************
//
// Register: CPU_SCS_O_ID_ISAR1
//
//*****************************************************************************
//*****************************************************************************
//
// Register: CPU_SCS_O_ID_ISAR2
//
//*****************************************************************************
//*****************************************************************************
//
// Register: CPU_SCS_O_ID_ISAR3
//
//*****************************************************************************
//*****************************************************************************
//
// Register: CPU_SCS_O_ID_ISAR4
//
//*****************************************************************************

#endif // __CPU_SCS__
