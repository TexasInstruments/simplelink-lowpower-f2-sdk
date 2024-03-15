/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*!*****************************************************************************
 *  @file       MCAN_reg.h
 *  @brief      M_CAN controller v3.2.1 register definitions
 *******************************************************************************
 */

#ifndef __MCAN_REG_H__
#define __MCAN_REG_H__

//*****************************************************************************
//
// This section defines the register offsets for the MCAN controller
//
//*****************************************************************************
// MCAN Core Release Register
#define MCAN_CREL 0x00000000U

// MCAN Endian Register
#define MCAN_ENDN 0x00000004U

// MCAN Data Bit Timing and Prescaler Register
#define MCAN_DBTP 0x0000000CU

// MCAN Test Register
#define MCAN_TEST 0x00000010U

// MCAN RAM Watchdog
#define MCAN_RWD 0x00000014U

// MCAN CC Control Register
#define MCAN_CCCR 0x00000018U

// MCAN Nominal Bit Timing and Prescaler Register
#define MCAN_NBTP 0x0000001CU

// MCAN Timestamp Counter Configuration
#define MCAN_TSCC 0x00000020U

// MCAN Timestamp Counter Value
#define MCAN_TSCV 0x00000024U

// MCAN Timeout Counter Configuration
#define MCAN_TOCC 0x00000028U

// MCAN Timeout Counter Value
#define MCAN_TOCV 0x0000002CU

// MCAN Error Counter Register
#define MCAN_ECR 0x00000040U

// MCAN Protocol Status Register
#define MCAN_PSR 0x00000044U

// MCAN Transmitter Delay Compensation Register
#define MCAN_TDCR 0x00000048U

// MCAN Interrupt Register
#define MCAN_IR 0x00000050U

// MCAN Interrupt Enable
#define MCAN_IE 0x00000054U

// MCAN Interrupt Line Select
#define MCAN_ILS 0x00000058U

// MCAN Interrupt Line Enable
#define MCAN_ILE 0x0000005CU

// MCAN Global Filter Configuration
#define MCAN_GFC 0x00000080U

// MCAN Standard ID Filter Configuration
#define MCAN_SIDFC 0x00000084U

// MCAN Extended ID Filter Configuration
#define MCAN_XIDFC 0x00000088U

// MCAN Extended ID and Mask
#define MCAN_XIDAM 0x00000090U

// MCAN High Priority Message Status
#define MCAN_HPMS 0x00000094U

// MCAN New Data 1
#define MCAN_NDAT1 0x00000098U

// MCAN New Data 2
#define MCAN_NDAT2 0x0000009CU

// MCAN Rx FIFO 0 Configuration
#define MCAN_RXF0C 0x000000A0U

// MCAN Rx FIFO 0 Status
#define MCAN_RXF0S 0x000000A4U

// MCAN Rx FIFO 0 Acknowledge
#define MCAN_RXF0A 0x000000A8U

// MCAN Rx Buffer Configuration
#define MCAN_RXBC 0x000000ACU

// MCAN Rx FIFO 1 Configuration
#define MCAN_RXF1C 0x000000B0U

// MCAN Rx FIFO 1 Status
#define MCAN_RXF1S 0x000000B4U

// MCAN Rx FIFO 1 Acknowledge
#define MCAN_RXF1A 0x000000B8U

// MCAN Rx Buffer / FIFO Element Size Configuration
#define MCAN_RXESC 0x000000BCU

// MCAN Tx Buffer Configuration
#define MCAN_TXBC 0x000000C0U

// MCAN Tx FIFO / Queue Status
#define MCAN_TXFQS 0x000000C4U

// MCAN Tx Buffer Element Size Configuration
#define MCAN_TXESC 0x000000C8U

// MCAN Tx Buffer Request Pending
#define MCAN_TXBRP 0x000000CCU

// MCAN Tx Buffer Add Request
#define MCAN_TXBAR 0x000000D0U

// MCAN Tx Buffer Cancellation Request
#define MCAN_TXBCR 0x000000D4U

// MCAN Tx Buffer Transmission Occurred
#define MCAN_TXBTO 0x000000D8U

// MCAN Tx Buffer Cancellation Finished
#define MCAN_TXBCF 0x000000DCU

// MCAN Tx Buffer Transmission Interrupt Enable
#define MCAN_TXBTIE 0x000000E0U

// MCAN Tx Buffer Cancellation Finished Interrupt Enable
#define MCAN_TXBCIE 0x000000E4U

// MCAN Tx Event FIFO Configuration
#define MCAN_TXEFC 0x000000F0U

// MCAN Tx Event FIFO Status
#define MCAN_TXEFS 0x000000F4U

// MCAN Tx Event FIFO Acknowledge
#define MCAN_TXEFA 0x000000F8U

//*****************************************************************************
//
// Register: MCAN_CREL
//
//*****************************************************************************
// Field: [31:28] REL
//
// Core Release. One digit, BCD-coded.
#define MCAN_CREL_REL_WIDTH 4U
#define MCAN_CREL_REL_MASK  0xF0000000U
#define MCAN_CREL_REL_SHIFT 28U

// Field: [27:24] STEP
//
// Step of Core Release. One digit, BCD-coded.
#define MCAN_CREL_STEP_WIDTH 4U
#define MCAN_CREL_STEP_MASK  0x0F000000U
#define MCAN_CREL_STEP_SHIFT 24U

// Field: [23:20] SUBSTEP
//
// Sub-Step of Core Release. One digit, BCD-coded.
#define MCAN_CREL_SUBSTEP_WIDTH 4U
#define MCAN_CREL_SUBSTEP_MASK  0x00F00000U
#define MCAN_CREL_SUBSTEP_SHIFT 20U

// Field: [19:16] YEAR
//
// Time Stamp Year. One digit, BCD-coded.
#define MCAN_CREL_YEAR_WIDTH 4U
#define MCAN_CREL_YEAR_MASK  0x000F0000U
#define MCAN_CREL_YEAR_SHIFT 16U

// Field:  [15:8] MON
//
// Time Stamp Month. Two digits, BCD-coded.
#define MCAN_CREL_MON_WIDTH 8U
#define MCAN_CREL_MON_MASK  0x0000FF00U
#define MCAN_CREL_MON_SHIFT 8U

// Field:   [7:0] DAY
//
// Time Stamp Day. Two digits, BCD-coded.
#define MCAN_CREL_DAY_WIDTH 8U
#define MCAN_CREL_DAY_MASK  0x000000FFU
#define MCAN_CREL_DAY_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_ENDN
//
//*****************************************************************************
// Field:  [31:0] ETV
//
// Endianess Test Value. Reading the constant value maintained in this register
// allows software to determine the endianess of the host CPU.
#define MCAN_ENDN_ETV_WIDTH 32U
#define MCAN_ENDN_ETV_MASK  0xFFFFFFFFU
#define MCAN_ENDN_ETV_SHIFT 0U

#define MCAN_ENDN_ETV_VALUE 0x87654321U

//*****************************************************************************
//
// Register: MCAN_DBTP
//
//*****************************************************************************
// Field:    [23] TDC
//
// Transmitter Delay Compensation
//   0  Transmitter Delay Compensation disabled
//   1  Transmitter Delay Compensation enabled
//
// +I107
#define MCAN_DBTP_TDC       0x00800000U
#define MCAN_DBTP_TDC_MASK  0x00800000U
#define MCAN_DBTP_TDC_SHIFT 23U

// Field: [20:16] DBRP
//
// Data Bit Rate Prescaler. The value by which the oscillator frequency is
// divided for generating the bit time quanta. The bit time is built up from a
// multiple of this quanta. Valid values for the Bit Rate Prescaler are 0 to
// 31. The actual interpretation by the hardware of this value is such that one
// more than the value programmed here is used.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_DBTP_DBRP_WIDTH 5U
#define MCAN_DBTP_DBRP_MASK  0x001F0000U
#define MCAN_DBTP_DBRP_SHIFT 16U

// Field:  [12:8] DTSEG1
//
// Data Time Segment Before Sample Point. Valid values are 0 to 31. The actual
// interpretation by the hardware of this value is such that one more than the
// programmed value is used.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_DBTP_DTSEG1_WIDTH 5U
#define MCAN_DBTP_DTSEG1_MASK  0x00001F00U
#define MCAN_DBTP_DTSEG1_SHIFT 8U

// Field:   [7:4] DTSEG2
//
// Data Time Segment After Sample Point. Valid values are 0 to 15. The actual
// interpretation by the hardware of this value is such that one more than the
// programmed value is used.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_DBTP_DTSEG2_WIDTH 4U
#define MCAN_DBTP_DTSEG2_MASK  0x000000F0U
#define MCAN_DBTP_DTSEG2_SHIFT 4U

// Field:   [3:0] DSJW
//
// Data Resynchronization Jump Width. Valid values are 0 to 15. The actual
// interpretation by the hardware of this value is such that one more than the
// value programmed here is used.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_DBTP_DSJW_WIDTH 4U
#define MCAN_DBTP_DSJW_MASK  0x0000000FU
#define MCAN_DBTP_DSJW_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TEST
//
//*****************************************************************************
// Field:     [7] RX
//
// Receive Pin. Monitors the actual value of the CAN receive pin.
//   0  The CAN bus is dominant (CAN RX pin = '0')
//   1  The CAN bus is recessive (CAN RX pin = '1')
#define MCAN_TEST_RX       0x00000080U
#define MCAN_TEST_RX_MASK  0x00000080U
#define MCAN_TEST_RX_SHIFT 7U

// Field:   [6:5] TX
//
// Control of Transmit Pin
//   00  CAN TX pin controlled by the CAN Core, updated at the end of the CAN
// bit time
//   01  Sample Point can be monitored at CAN TX pin
//   10  Dominant ('0') level at CAN TX pin
//   11  Recessive ('1') at CAN TX pin
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_TEST_TX_WIDTH 2U
#define MCAN_TEST_TX_MASK  0x00000060U
#define MCAN_TEST_TX_SHIFT 5U

// Field:     [4] LBCK
//
// Loop Back Mode
//   0  Reset value, Loop Back Mode is disabled
//   1  Loop Back Mode is enabled
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_TEST_LBCK       0x00000010U
#define MCAN_TEST_LBCK_MASK  0x00000010U
#define MCAN_TEST_LBCK_SHIFT 4U

//*****************************************************************************
//
// Register: MCAN_RWD
//
//*****************************************************************************
// Field:  [15:8] WDV
//
// Watchdog Value. Acutal Message RAM Watchdog Counter Value.
//
// The RAM Watchdog monitors the READY output of the Message RAM. A Message RAM
// access via the MCAN's Generic Master Interface starts the Message RAM
// Watchdog Counter with the value configured by the WDC field. The counter is
// reloaded with WDC when the Message RAM signals successful completion by
// activating its READY output. In case there is no response from the Message
// RAM until the counter has counted down to zero, the counter stops and
// interrupt flag MCAN_IR.WDI is set. The RAM Watchdog Counter is clocked by
// the host (system) clock.
#define MCAN_RWD_WDV_WIDTH 8U
#define MCAN_RWD_WDV_MASK  0x0000FF00U
#define MCAN_RWD_WDV_SHIFT 8U

// Field:   [7:0] WDC
//
// Watchdog Configuration. Start value of the Message RAM Watchdog Counter.
// With the reset value of "00" the counter is disabled.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_RWD_WDC_WIDTH 8U
#define MCAN_RWD_WDC_MASK  0x000000FFU
#define MCAN_RWD_WDC_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_CCCR
//
//*****************************************************************************
// Field:    [15] NISO
//
// Non ISO Operation. If this bit is set, the MCAN uses the CAN FD frame format
// as specified by the Bosch CAN FD Specification V1.0.
//   0  CAN FD frame format according to ISO 11898-1:2015
//   1  CAN FD frame format according to Bosch CAN FD Specification V1.0
#define MCAN_CCCR_NISO       0x00008000U
#define MCAN_CCCR_NISO_MASK  0x00008000U
#define MCAN_CCCR_NISO_SHIFT 15U

// Field:    [14] TXP
//
// Transmit Pause. If this bit is set, the MCAN pauses for two CAN bit times
// before starting the next transmission after itself has successfully
// transmitted a frame.
//   0  Transmit pause disabled
//   1  Transmit pause enabled
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_CCCR_TXP       0x00004000U
#define MCAN_CCCR_TXP_MASK  0x00004000U
#define MCAN_CCCR_TXP_SHIFT 14U

// Field:    [13] EFBI
//
// Edge Filtering during Bus Integration
//   0  Edge filtering disabled
//   1  Two consecutive dominant tq required to detect an edge for hard
// synchronization
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_CCCR_EFBI       0x00002000U
#define MCAN_CCCR_EFBI_MASK  0x00002000U
#define MCAN_CCCR_EFBI_SHIFT 13U

// Field:    [12] PXHD
//
// Protocol Exception Handling Disable
//   0  Protocol exception handling enabled
//   1  Protocol exception handling disabled
// Note: When protocol exception handling is disabled, the MCAN will transmit
// an error frame when it detects a protocol exception condition.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_CCCR_PXHD       0x00001000U
#define MCAN_CCCR_PXHD_MASK  0x00001000U
#define MCAN_CCCR_PXHD_SHIFT 12U

// Field:     [9] BRSE
//
// Bit Rate Switch Enable
//   0  Bit rate switching for transmissions disabled
//   1  Bit rate switching for transmissions enabled
// Note: When CAN FD operation is disabled FDOE = '0', BRSE is not evaluated.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_CCCR_BRSE       0x00000200U
#define MCAN_CCCR_BRSE_MASK  0x00000200U
#define MCAN_CCCR_BRSE_SHIFT 9U

// Field:     [8] FDOE
//
// Flexible Datarate Operation Enable
//   0  FD operation disabled
//   1  FD operation enabled
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_CCCR_FDOE       0x00000100U
#define MCAN_CCCR_FDOE_MASK  0x00000100U
#define MCAN_CCCR_FDOE_SHIFT 8U

// Field:     [7] TEST
//
// Test Mode Enable
//   0  Normal operation, register TEST holds reset values
//   1  Test Mode, write access to register TEST enabled
//
// Qualified Write 1 to Set is possible only with CCCR.CCE='1' and
// CCCR.INIT='1'.
#define MCAN_CCCR_TEST       0x00000080U
#define MCAN_CCCR_TEST_MASK  0x00000080U
#define MCAN_CCCR_TEST_SHIFT 7U

// Field:     [6] DAR
//
// Disable Automatic Retransmission
//   0  Automatic retransmission of messages not transmitted successfully
// enabled
//   1  Automatic retransmission disabled
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_CCCR_DAR       0x00000040U
#define MCAN_CCCR_DAR_MASK  0x00000040U
#define MCAN_CCCR_DAR_SHIFT 6U

// Field:     [5] MON
//
// Bus Monitoring Mode. Bit MON can only be set by SW when both CCE and INIT
// are set to '1'. The bit can be reset by SW at any time.
//   0  Bus Monitoring Mode is disabled
//   1  Bus Monitoring Mode is enabled
//
// Qualified Write 1 to Set is possible only with CCCR.CCE='1' and
// CCCR.INIT='1'.
#define MCAN_CCCR_MON       0x00000020U
#define MCAN_CCCR_MON_MASK  0x00000020U
#define MCAN_CCCR_MON_SHIFT 5U

// Field:     [4] CSR
//
// Clock Stop Request
//   0  No clock stop is requested
//   1  Clock stop requested. When clock stop is requested, first INIT and then
// CSA will be set after all pending transfer requests have been completed and
// the CAN bus reached idle.
#define MCAN_CCCR_CSR       0x00000010U
#define MCAN_CCCR_CSR_MASK  0x00000010U
#define MCAN_CCCR_CSR_SHIFT 4U

// Field:     [3] CSA
//
// Clock Stop Acknowledge
//   0  No clock stop acknowledged
//   1  MCAN may be set in power down by stopping the Host and CAN clocks
#define MCAN_CCCR_CSA       0x00000008U
#define MCAN_CCCR_CSA_MASK  0x00000008U
#define MCAN_CCCR_CSA_SHIFT 3U

// Field:     [2] ASM
//
// Restricted Operation Mode. Bit ASM can only be set by SW when both CCE and
// INIT are set to '1'. The bit can be reset by SW at any time.
//   0  Normal CAN operation
//   1  Restricted Operation Mode active
//
// Qualified Write 1 to Set is possible only with CCCR.CCE='1' and
// CCCR.INIT='1'.
#define MCAN_CCCR_ASM       0x00000004U
#define MCAN_CCCR_ASM_MASK  0x00000004U
#define MCAN_CCCR_ASM_SHIFT 2U

// Field:     [1] CCE
//
// Configuration Change Enable
//   0  The CPU has no write access to the protected configuration registers
//   1  The CPU has write access to the protected configuration registers
// (while CCCR.INIT = '1')
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_CCCR_CCE       0x00000002U
#define MCAN_CCCR_CCE_MASK  0x00000002U
#define MCAN_CCCR_CCE_SHIFT 1U

// Field:     [0] INIT
//
// Initialization
//   0  Normal Operation
//   1  Initialization is started
// Note: Due to the synchronization mechanism between the two clock domains,
// there may be a delay until the value written to INIT can be read back.
// Therefore the programmer has to assure that the previous value written to
// INIT has been accepted by reading INIT before setting INIT to a new value.
#define MCAN_CCCR_INIT       0x00000001U
#define MCAN_CCCR_INIT_MASK  0x00000001U
#define MCAN_CCCR_INIT_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_NBTP
//
//*****************************************************************************
// Field: [31:25] NSJW
//
// Nominal (Re)Synchronization Jump Width. Valid values are 0 to 127. The
// actual interpretation by the hardware of this value is such that one more
// than the value programmed here is used.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_NBTP_NSJW_WIDTH 7U
#define MCAN_NBTP_NSJW_MASK  0xFE000000U
#define MCAN_NBTP_NSJW_SHIFT 25U

// Field: [24:16] NBRP
//
// Nominal Bit Rate Prescaler. The value by which the oscillator frequency is
// divided for generating the bit time quanta. The bit time is built up from a
// multiple of this quanta. Valid values for the Bit Rate Prescaler are 0 to
// 511. The actual interpretation by the hardware of this value is such that
// one more than the value programmed here is used.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_NBTP_NBRP_WIDTH 9U
#define MCAN_NBTP_NBRP_MASK  0x01FF0000U
#define MCAN_NBTP_NBRP_SHIFT 16U

// Field:  [15:8] NTSEG1
//
// Nominal Time Segment Before Sample Point. Valid values are 1 to 255. The
// actual interpretation by the hardware of this value is such that one more
// than the programmed value is used.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_NBTP_NTSEG1_WIDTH 8U
#define MCAN_NBTP_NTSEG1_MASK  0x0000FF00U
#define MCAN_NBTP_NTSEG1_SHIFT 8U

// Field:   [6:0] NTSEG2
//
// Nominal Time Segment After Sample Point. Valid values are 1 to 127. The
// actual interpretation by the hardware of this value is such that one more
// than the programmed value is used.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_NBTP_NTSEG2_WIDTH 7U
#define MCAN_NBTP_NTSEG2_MASK  0x0000007FU
#define MCAN_NBTP_NTSEG2_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TSCC
//
//*****************************************************************************
// Field: [19:16] TCP
//
// Timestamp Counter Prescaler. Configures the timestamp and timeout counters
// time unit in multiples of CAN bit times. Valid values are 0 to 15. The
// actual interpretation by the hardware of this value is such that one more
// than the value programmed here is used.
//
// Note: With CAN FD an external counter is required for timestamp generation
// (TSS = "10").
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_TSCC_TCP_WIDTH 4U
#define MCAN_TSCC_TCP_MASK  0x000F0000U
#define MCAN_TSCC_TCP_SHIFT 16U

// Field:   [1:0] TSS
//
// Timestamp Select
//   00  Timestamp counter value always 0x0000
//   01  Timestamp counter value incremented according to TCP
//   10  External timestamp counter value used
//   11  Same as "00"
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_TSCC_TSS_WIDTH 2U
#define MCAN_TSCC_TSS_MASK  0x00000003U
#define MCAN_TSCC_TSS_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TSCV
//
//*****************************************************************************
// Field:  [15:0] TSC
//
// Timestamp Counter. The internal/external Timestamp Counter value is captured
// on start of frame (both Rx and Tx). When TSCC.TSS = "01", the Timestamp
// Counter is incremented in multiples of CAN bit times, (1...16), depending on
// the configuration of TSCC.TCP. A wrap around sets interrupt flag IR.TSW.
// Write access resets the counter to zero. When TSCC.TSS = "10", TSC reflects
// the External Timestamp Counter value, and a write access has no impact.
//
// Note: A "wrap around" is a change of the Timestamp Counter value from
// non-zero to zero not
// caused by write access to MCAN_TSCV.
#define MCAN_TSCV_TSC_WIDTH 16U
#define MCAN_TSCV_TSC_MASK  0x0000FFFFU
#define MCAN_TSCV_TSC_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TOCC
//
//*****************************************************************************
// Field: [31:16] TOP
//
// Timeout Period. Start value of the Timeout Counter (down-counter).
// Configures the Timeout Period.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_TOCC_TOP_WIDTH 16U
#define MCAN_TOCC_TOP_MASK  0xFFFF0000U
#define MCAN_TOCC_TOP_SHIFT 16U

// Field:   [2:1] TOS
//
// Timeout Select. When operating in Continuous mode, a write to TOCV presets
// the counter to the value configured by TOCC.TOP and continues down-counting.
// When the Timeout Counter is controlled by one of the FIFOs, an empty FIFO
// presets the counter to the value configured by TOCC.TOP. Down-counting is
// started when the first FIFO element is stored.
//   00  Continuous operation
//   01  Timeout controlled by Tx Event FIFO
//   10  Timeout controlled by Rx FIFO 0
//   11  Timeout controlled by Rx FIFO 1
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_TOCC_TOS_WIDTH 2U
#define MCAN_TOCC_TOS_MASK  0x00000006U
#define MCAN_TOCC_TOS_SHIFT 1U

// Field:     [0] ETOC
//
// Enable Timeout Counter
//   0  Timeout Counter disabled
//   1  Timeout Counter enabled
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_TOCC_ETOC       0x00000001U
#define MCAN_TOCC_ETOC_MASK  0x00000001U
#define MCAN_TOCC_ETOC_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TOCV
//
//*****************************************************************************
// Field:  [15:0] TOC
//
// Timeout Counter. The Timeout Counter is decremented in multiples of CAN bit
// times, (1...16), depending on the configuration of TSCC.TCP. When
// decremented to zero, interrupt flag IR.TOO is set and the Timeout Counter is
// stopped. Start and reset/restart conditions are configured via TOCC.TOS.
#define MCAN_TOCV_TOC_WIDTH 16U
#define MCAN_TOCV_TOC_MASK  0x0000FFFFU
#define MCAN_TOCV_TOC_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_ECR
//
//*****************************************************************************
// Field: [23:16] CEL
//
// CAN Error Logging. The counter is incremented each time when a CAN protocol
// error causes the Transmit Error Counter or the Receive Error Counter to be
// incremented. It is reset by read access to CEL. The counter stops at 0xFF;
// the next increment of TEC or REC sets interrupt flag IR.ELO.
//
// Note: When CCCR.ASM is set, the CAN protocol controller does not increment
// TEC and REC when a CAN protocol error is detected, but CEL is still
// incremented.
#define MCAN_ECR_CEL_WIDTH 8U
#define MCAN_ECR_CEL_MASK  0x00FF0000U
#define MCAN_ECR_CEL_SHIFT 16U

// Field:    [15] RP
//
// Receive Error Passive
//   0  The Receive Error Counter is below the error passive level of 128
//   1  The Receive Error Counter has reached the error passive level of 128
#define MCAN_ECR_RP       0x00008000U
#define MCAN_ECR_RP_MASK  0x00008000U
#define MCAN_ECR_RP_SHIFT 15U

// Field:  [14:8] REC
//
// Receive Error Counter. Actual state of the Receive Error Counter, values
// between 0 and 127.
//
// Note: When CCCR.ASM is set, the CAN protocol controller does not increment
// TEC and REC when a CAN protocol error is detected, but CEL is still
// incremented.
#define MCAN_ECR_REC_WIDTH 7U
#define MCAN_ECR_REC_MASK  0x00007F00U
#define MCAN_ECR_REC_SHIFT 8U

// Field:   [7:0] TEC
//
// Transmit Error Counter. Actual state of the Transmit Error Counter, values
// between 0 and 255.
//
// Note: When CCCR.ASM is set, the CAN protocol controller does not increment
// TEC and REC when a CAN protocol error is detected, but CEL is still
// incremented.
#define MCAN_ECR_TEC_WIDTH 8U
#define MCAN_ECR_TEC_MASK  0x000000FFU
#define MCAN_ECR_TEC_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_PSR
//
//*****************************************************************************
// Field: [22:16] TDCV
//
// Transmitter Delay Compensation Value. Position of the secondary sample
// point, defined by the sum of the measured delay from the internal CAN TX
// signal to the internal CAN RX signal and TDCR.TDCO. The SSP position is, in
// the data phase, the number of mtq between the start of the transmitted bit
// and the secondary sample point. Valid values are 0 to 127 mtq.
#define MCAN_PSR_TDCV_WIDTH 7U
#define MCAN_PSR_TDCV_MASK  0x007F0000U
#define MCAN_PSR_TDCV_SHIFT 16U

// Field:    [14] PXE
//
// Protocol Exception Event
//   0  No protocol exception event occurred since last read access
//   1  Protocol exception event occurred
#define MCAN_PSR_PXE       0x00004000U
#define MCAN_PSR_PXE_MASK  0x00004000U
#define MCAN_PSR_PXE_SHIFT 14U

// Field:    [13] RFDF
//
// Received a CAN FD Message.  This bit is set independent of acceptance
// filtering.
//   0  Since this bit was reset by the CPU, no CAN FD message has been
// received
//   1  Message in CAN FD format with FDF flag set has been received
#define MCAN_PSR_RFDF       0x00002000U
#define MCAN_PSR_RFDF_MASK  0x00002000U
#define MCAN_PSR_RFDF_SHIFT 13U

// Field:    [12] RBRS
//
// BRS Flag of Last Received CAN FD Message. This bit is set together with
// RFDF, independent of acceptance filtering.
//   0  Last received CAN FD message did not have its BRS flag set
//   1  Last received CAN FD message had its BRS flag set
#define MCAN_PSR_RBRS       0x00001000U
#define MCAN_PSR_RBRS_MASK  0x00001000U
#define MCAN_PSR_RBRS_SHIFT 12U

// Field:    [11] RESI
//
// ESI Flag of Last Received CAN FD Message. This bit is set together with
// RFDF, independent of acceptance filtering.
//   0  Last received CAN FD message did not have its ESI flag set
//   1  Last received CAN FD message had its ESI flag set
#define MCAN_PSR_RESI       0x00000800U
#define MCAN_PSR_RESI_MASK  0x00000800U
#define MCAN_PSR_RESI_SHIFT 11U

// Field:  [10:8] DLEC
//
// Data Phase Last Error Code. Type of last error that occurred in the data
// phase of a CAN FD format frame with its BRS flag set. Coding is the same as
// for LEC. This field will be cleared to zero when a CAN FD format frame with
// its BRS flag set has been transferred (reception or transmission) without
// error.
#define MCAN_PSR_DLEC_WIDTH 3U
#define MCAN_PSR_DLEC_MASK  0x00000700U
#define MCAN_PSR_DLEC_SHIFT 8U

// Field:     [7] BO
//
// Bus_Off Status
//   0  The M_CAN is not Bus_Off
//   1  The M_CAN is in Bus_Off state
#define MCAN_PSR_BO       0x00000080U
#define MCAN_PSR_BO_MASK  0x00000080U
#define MCAN_PSR_BO_SHIFT 7U

// Field:     [6] EW
//
// Warning Status
//   0  Both error counters are below the Error_Warning limit of 96
//   1  At least one of error counter has reached the Error_Warning limit of 96
#define MCAN_PSR_EW       0x00000040U
#define MCAN_PSR_EW_MASK  0x00000040U
#define MCAN_PSR_EW_SHIFT 6U

// Field:     [5] EP
//
// Error Passive
//   0  The M_CAN is in the Error_Active state. It normally takes part in bus
// communication and sends an active error flag when an error has been detected
//   1  The M_CAN is in the Error_Passive state
#define MCAN_PSR_EP       0x00000020U
#define MCAN_PSR_EP_MASK  0x00000020U
#define MCAN_PSR_EP_SHIFT 5U

// Field:   [4:3] ACT
//
// Node Activity.  Monitors the module's CAN communication state.
//   00  Synchronizing - node is synchronizing on CAN communication
//   01  Idle - node is neither receiver nor transmitter
//   10  Receiver - node is operating as receiver
//   11  Transmitter - node is operating as transmitter
//
// Note: ACT is set to "00" by a Protocol Exception Event.
#define MCAN_PSR_ACT_WIDTH 2U
#define MCAN_PSR_ACT_MASK  0x00000018U
#define MCAN_PSR_ACT_SHIFT 3U

// Field:   [2:0] LEC
//
// Last Error Code. The LEC indicates the type of the last error to occur on
// the CAN bus. This field will be cleared to '0' when a message has been
// transferred (reception or transmission) without error.
//   0  No Error: No error occurred since LEC has been reset by successful
// reception or transmission.
//   1  Stuff Error: More than 5 equal bits in a sequence have occurred in a
// part of a received message where this is not allowed.
//   2  Form Error: A fixed format part of a received frame has the wrong
// format.
//   3  AckError: The message transmitted by the MCAN was not acknowledged by
// another node.
//   4  Bit1Error: During the transmission of a message (with the exception of
// the arbitration field), the device wanted to send a recessive level (bit of
// logical value '1'), but the monitored bus value was dominant.
//   5  Bit0Error: During the transmission of a message (or acknowledge bit, or
// active error flag, or overload flag), the device wanted to send a dominant
// level (data or identifier bit logical value '0'), but the monitored bus
// value was recessive. During Bus_Off recovery this status is set each time a
// sequence of 11 recessive bits has been monitored. This enables the CPU to
// monitor the proceeding of the Bus_Off recovery sequence (indicating the bus
// is not stuck at dominant or continuously disturbed).
//   6  CRCError: The CRC check sum of a received message was incorrect. The
// CRC of an incoming message does not match with the CRC calculated from the
// received data.
//   7  NoChange: Any read access to the Protocol Status Register
// re-initializes the LEC to '7'. When the LEC shows the value '7', no CAN bus
// event was detected since the last CPU read access to the Protocol Status
// Register.
//
// Note: When a frame in CAN FD format has reached the data phase with BRS flag
// set, the next CAN event (error or valid frame) will be shown in DLEC instead
// of LEC. An error in a fixed stuff bit of a CAN FD CRC sequence will be shown
// as a Form Error, not Stuff Error. Note: The Bus_Off recovery sequence (see
// ISO 11898-1:2015) cannot be shortened by setting or resetting CCCR.INIT. If
// the device goes Bus_Off, it will set CCCR.INIT of its own accord, stopping
// all bus activities. Once CCCR.INIT has been cleared by the CPU, the device
// will then wait for 129 occurrences of Bus Idle (129 * 11 consecutive
// recessive bits) before resuming normal operation. At the end of the Bus_Off
// recovery sequence, the Error Management Counters will be reset. During the
// waiting time after the resetting of CCCR.INIT, each time a sequence of 11
// recessive bits has been monitored, a Bit0Error code is written to PSR.LEC,
// enabling the CPU to readily check up whether the CAN bus is stuck at
// dominant or continuously disturbed and to monitor the Bus_Off recovery
// sequence. ECR.REC is used to count these sequences.
#define MCAN_PSR_LEC_WIDTH 3U
#define MCAN_PSR_LEC_MASK  0x00000007U
#define MCAN_PSR_LEC_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TDCR
//
//*****************************************************************************
// Field:  [14:8] TDCO
//
// Transmitter Delay Compensation Offset. Offset value defining the distance
// between the measured delay from the internal CAN TX signal to the internal
// CAN RX signal and the secondary sample point. Valid values are 0 to 127 mtq.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_TDCR_TDCO_WIDTH 7U
#define MCAN_TDCR_TDCO_MASK  0x00007F00U
#define MCAN_TDCR_TDCO_SHIFT 8U

// Field:   [6:0] TDCF
//
// Transmitter Delay Compensation Filter Window Length. Defines the minimum
// value for the SSP position, dominant edges on the internal CAN RX signal
// that would result in an earlier SSP position are ignored for transmitter
// delay measurement. The feature is enabled when TDCF is configured to a value
// greater than TDCO. Valid values are 0 to 127 mtq.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_TDCR_TDCF_WIDTH 7U
#define MCAN_TDCR_TDCF_MASK  0x0000007FU
#define MCAN_TDCR_TDCF_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_IR
//
//*****************************************************************************
// Field:    [29] ARA
//
// Access to Reserved Address
//   0  No access to reserved address occurred
//   1  Access to reserved address occurred
#define MCAN_IR_ARA       0x20000000U
#define MCAN_IR_ARA_MASK  0x20000000U
#define MCAN_IR_ARA_SHIFT 29U

// Field:    [28] PED
//
// Protocol Error in Data Phase (Data Bit Time is used)
//   0  No protocol error in data phase
//   1  Protocol error in data phase detected (PSR.DLEC ? 0,7)
#define MCAN_IR_PED       0x10000000U
#define MCAN_IR_PED_MASK  0x10000000U
#define MCAN_IR_PED_SHIFT 28U

// Field:    [27] PEA
//
// Protocol Error in Arbitration Phase (Nominal Bit Time is used)
//   0  No protocol error in arbitration phase
//   1  Protocol error in arbitration phase detected (PSR.LEC ? 0,7)
#define MCAN_IR_PEA       0x08000000U
#define MCAN_IR_PEA_MASK  0x08000000U
#define MCAN_IR_PEA_SHIFT 27U

// Field:    [26] WDI
//
// Watchdog Interrupt
//   0  No Message RAM Watchdog event occurred
//   1  Message RAM Watchdog event due to missing READY
#define MCAN_IR_WDI       0x04000000U
#define MCAN_IR_WDI_MASK  0x04000000U
#define MCAN_IR_WDI_SHIFT 26U

// Field:    [25] BO
//
// Bus_Off Status
//   0  Bus_Off status unchanged
//   1  Bus_Off status changed
#define MCAN_IR_BO       0x02000000U
#define MCAN_IR_BO_MASK  0x02000000U
#define MCAN_IR_BO_SHIFT 25U

// Field:    [24] EW
//
// Warning Status
//   0  Error_Warning status unchanged
//   1  Error_Warning status changed
#define MCAN_IR_EW       0x01000000U
#define MCAN_IR_EW_MASK  0x01000000U
#define MCAN_IR_EW_SHIFT 24U

// Field:    [23] EP
//
// Error Passive
//   0  Error_Passive status unchanged
//   1  Error_Passive status changed
#define MCAN_IR_EP       0x00800000U
#define MCAN_IR_EP_MASK  0x00800000U
#define MCAN_IR_EP_SHIFT 23U

// Field:    [22] ELO
//
// Error Logging Overflow
//   0  CAN Error Logging Counter did not overflow
//   1  Overflow of CAN Error Logging Counter occurred
#define MCAN_IR_ELO       0x00400000U
#define MCAN_IR_ELO_MASK  0x00400000U
#define MCAN_IR_ELO_SHIFT 22U

// Field:    [21] BEU
//
// Bit Error Uncorrected. Message RAM bit error detected, uncorrected. This bit
// is set when a double bit error is detected by the ECC aggregator attached to
// the Message RAM. An uncorrected Message RAM bit error sets CCCR.INIT to '1'.
// This is done to avoid transmission of corrupted data.
//   0  No bit error detected when reading from Message RAM
//   1  Bit error detected, uncorrected (e.g. parity logic)
#define MCAN_IR_BEU       0x00200000U
#define MCAN_IR_BEU_MASK  0x00200000U
#define MCAN_IR_BEU_SHIFT 21U

// Field:    [20] BEC
//
// Warning: This bit is reserved on CC27x0 and must not be written.
//
// Bit Error Corrected. Message RAM bit error detected and corrected.
//   0 No bit error detected when reading from Message RAM
//   1 Bit error detected and corrected (e.g. parity logic)
#define MCAN_IR_BEC       0x00100000U
#define MCAN_IR_BEC_MASK  0x00100000U
#define MCAN_IR_BEC_SHIFT 20U

// Field:    [19] DRX
//
// Message Stored to Dedicated Rx Buffer. The flag is set whenever a received
// message has been stored into a dedicated Rx Buffer.
//   0  No Rx Buffer updated
//   1  At least one received message stored into an Rx Buffer
#define MCAN_IR_DRX       0x00080000U
#define MCAN_IR_DRX_MASK  0x00080000U
#define MCAN_IR_DRX_SHIFT 19U

// Field:    [18] TOO
//
// Timeout Occurred
//   0  No timeout
//   1  Timeout reached
#define MCAN_IR_TOO       0x00040000U
#define MCAN_IR_TOO_MASK  0x00040000U
#define MCAN_IR_TOO_SHIFT 18U

// Field:    [17] MRAF
//
// Message RAM Access Failure.  The flag is set, when the Rx Handler:
//   - has not completed acceptance filtering or storage of an accepted message
// until the arbitration field of the following message has been received. In
// this case acceptance filtering or message storage is aborted and the Rx
// Handler starts processing of the following message.
//   - was not able to write a message to the Message RAM. In this case message
// storage is aborted.
//
// In both cases the FIFO put index is not updated resp. the New Data flag for
// a dedicated Rx Buffer is not set, a partly stored message is overwritten
// when the next message is stored to this location.
//
// The flag is also set when the Tx Handler was not able to read a message from
// the Message RAM in time. In this case message transmission is aborted. In
// case of a Tx Handler access failure the MCAN is switched into Restricted
// Operation Mode. To leave Restricted Operation Mode, the Host CPU has to
// reset CCCR.ASM.
//   0  No Message RAM access failure occurred
//   1  Message RAM access failure occurred
#define MCAN_IR_MRAF       0x00020000U
#define MCAN_IR_MRAF_MASK  0x00020000U
#define MCAN_IR_MRAF_SHIFT 17U

// Field:    [16] TSW
//
// Timestamp Wraparound
//   0  No timestamp counter wrap-around
//   1  Timestamp counter wrapped around
#define MCAN_IR_TSW       0x00010000U
#define MCAN_IR_TSW_MASK  0x00010000U
#define MCAN_IR_TSW_SHIFT 16U

// Field:    [15] TEFL
//
// Tx Event FIFO Element Lost
//   0  No Tx Event FIFO element lost
//   1  Tx Event FIFO element lost, also set after write attempt to Tx Event
// FIFO of size zero
#define MCAN_IR_TEFL       0x00008000U
#define MCAN_IR_TEFL_MASK  0x00008000U
#define MCAN_IR_TEFL_SHIFT 15U

// Field:    [14] TEFF
//
// Tx Event FIFO Full
//   0  Tx Event FIFO not full
//   1  Tx Event FIFO full
#define MCAN_IR_TEFF       0x00004000U
#define MCAN_IR_TEFF_MASK  0x00004000U
#define MCAN_IR_TEFF_SHIFT 14U

// Field:    [13] TEFW
//
// Tx Event FIFO Watermark Reached
//   0  Tx Event FIFO fill level below watermark
//   1  Tx Event FIFO fill level reached watermark
#define MCAN_IR_TEFW       0x00002000U
#define MCAN_IR_TEFW_MASK  0x00002000U
#define MCAN_IR_TEFW_SHIFT 13U

// Field:    [12] TEFN
//
// Tx Event FIFO New Entry
//   0  Tx Event FIFO unchanged
//   1  Tx Handler wrote Tx Event FIFO element
#define MCAN_IR_TEFN       0x00001000U
#define MCAN_IR_TEFN_MASK  0x00001000U
#define MCAN_IR_TEFN_SHIFT 12U

// Field:    [11] TFE
//
// Tx FIFO Empty
//   0  Tx FIFO non-empty
//   1  Tx FIFO empty
#define MCAN_IR_TFE       0x00000800U
#define MCAN_IR_TFE_MASK  0x00000800U
#define MCAN_IR_TFE_SHIFT 11U

// Field:    [10] TCF
//
// Transmission Cancellation Finished
//   0  No transmission cancellation finished
//   1  Transmission cancellation finished
#define MCAN_IR_TCF       0x00000400U
#define MCAN_IR_TCF_MASK  0x00000400U
#define MCAN_IR_TCF_SHIFT 10U

// Field:     [9] TC
//
// Transmission Completed
//   0  No transmission completed
//   1  Transmission completed
#define MCAN_IR_TC       0x00000200U
#define MCAN_IR_TC_MASK  0x00000200U
#define MCAN_IR_TC_SHIFT 9U

// Field:     [8] HPM
//
// High Priority Message
//   0  No high priority message received
//   1  High priority message received
#define MCAN_IR_HPM       0x00000100U
#define MCAN_IR_HPM_MASK  0x00000100U
#define MCAN_IR_HPM_SHIFT 8U

// Field:     [7] RF1L
//
// Rx FIFO 1 Message Lost
//   0  No Rx FIFO 1 message lost
//   1  Rx FIFO 1 message lost, also set after write attempt to Rx FIFO 1 of
// size zero
#define MCAN_IR_RF1L       0x00000080U
#define MCAN_IR_RF1L_MASK  0x00000080U
#define MCAN_IR_RF1L_SHIFT 7U

// Field:     [6] RF1F
//
// Rx FIFO 1 Full
//   0  Rx FIFO 1 not full
//   1  Rx FIFO 1 full
#define MCAN_IR_RF1F       0x00000040U
#define MCAN_IR_RF1F_MASK  0x00000040U
#define MCAN_IR_RF1F_SHIFT 6U

// Field:     [5] RF1W
//
// Rx FIFO 1 Watermark Reached
//   0  Rx FIFO 1 fill level below watermark
//   1  Rx FIFO 1 fill level reached watermark
#define MCAN_IR_RF1W       0x00000020U
#define MCAN_IR_RF1W_MASK  0x00000020U
#define MCAN_IR_RF1W_SHIFT 5U

// Field:     [4] RF1N
//
// Rx FIFO 1 New Message
//   0  No new message written to Rx FIFO 1
//   1  New message written to Rx FIFO 1
#define MCAN_IR_RF1N       0x00000010U
#define MCAN_IR_RF1N_MASK  0x00000010U
#define MCAN_IR_RF1N_SHIFT 4U

// Field:     [3] RF0L
//
// Rx FIFO 0 Message Lost
//   0  No Rx FIFO 0 message lost
//   1  Rx FIFO 0 message lost, also set after write attempt to Rx FIFO 0 of
// size zero
#define MCAN_IR_RF0L       0x00000008U
#define MCAN_IR_RF0L_MASK  0x00000008U
#define MCAN_IR_RF0L_SHIFT 3U

// Field:     [2] RF0F
//
// Rx FIFO 0 Full
//   0  Rx FIFO 0 not full
//   1  Rx FIFO 0 full
#define MCAN_IR_RF0F       0x00000004U
#define MCAN_IR_RF0F_MASK  0x00000004U
#define MCAN_IR_RF0F_SHIFT 2U

// Field:     [1] RF0W
//
// Rx FIFO 0 Watermark Reached
//   0  Rx FIFO 0 fill level below watermark
//   1  Rx FIFO 0 fill level reached watermark
#define MCAN_IR_RF0W       0x00000002U
#define MCAN_IR_RF0W_MASK  0x00000002U
#define MCAN_IR_RF0W_SHIFT 1U

// Field:     [0] RF0N
//
// Rx FIFO 0 New Message
//   0  No new message written to Rx FIFO 0
//   1  New message written to Rx FIFO 0
#define MCAN_IR_RF0N       0x00000001U
#define MCAN_IR_RF0N_MASK  0x00000001U
#define MCAN_IR_RF0N_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_IE
//
//*****************************************************************************
// Field:    [29] ARAE
//
// Access to Reserved Address Enable
#define MCAN_IE_ARAE       0x20000000U
#define MCAN_IE_ARAE_MASK  0x20000000U
#define MCAN_IE_ARAE_SHIFT 29U

// Field:    [28] PEDE
//
// Protocol Error in Data Phase Enable
#define MCAN_IE_PEDE       0x10000000U
#define MCAN_IE_PEDE_MASK  0x10000000U
#define MCAN_IE_PEDE_SHIFT 28U

// Field:    [27] PEAE
//
// Protocol Error in Arbitration Phase Enable
#define MCAN_IE_PEAE       0x08000000U
#define MCAN_IE_PEAE_MASK  0x08000000U
#define MCAN_IE_PEAE_SHIFT 27U

// Field:    [26] WDIE
//
// Watchdog Interrupt Enable
#define MCAN_IE_WDIE       0x04000000U
#define MCAN_IE_WDIE_MASK  0x04000000U
#define MCAN_IE_WDIE_SHIFT 26U

// Field:    [25] BOE
//
// Bus_Off Status Enable
#define MCAN_IE_BOE       0x02000000U
#define MCAN_IE_BOE_MASK  0x02000000U
#define MCAN_IE_BOE_SHIFT 25U

// Field:    [24] EWE
//
// Warning Status Enable
#define MCAN_IE_EWE       0x01000000U
#define MCAN_IE_EWE_MASK  0x01000000U
#define MCAN_IE_EWE_SHIFT 24U

// Field:    [23] EPE
//
// Error Passive Enable
#define MCAN_IE_EPE       0x00800000U
#define MCAN_IE_EPE_MASK  0x00800000U
#define MCAN_IE_EPE_SHIFT 23U

// Field:    [22] ELOE
//
// Error Logging Overflow Enable
#define MCAN_IE_ELOE       0x00400000U
#define MCAN_IE_ELOE_MASK  0x00400000U
#define MCAN_IE_ELOE_SHIFT 22U

// Field:    [21] BEUE
//
// Bit Error Uncorrected Enable
#define MCAN_IE_BEUE       0x00200000U
#define MCAN_IE_BEUE_MASK  0x00200000U
#define MCAN_IE_BEUE_SHIFT 21U

// Field:    [20] BECE
//
// Bit Error Corrected Enable
//
// A separate interrupt line reserved for corrected bit errors is provided via
// the MCAN_ERROR_REGS. It advised for the user to use these registers and
// leave this bit cleared to '0'.
#define MCAN_IE_BECE       0x00100000U
#define MCAN_IE_BECE_MASK  0x00100000U
#define MCAN_IE_BECE_SHIFT 20U

// Field:    [19] DRXE
//
// Message Stored to Dedicated Rx Buffer Enable
#define MCAN_IE_DRXE       0x00080000U
#define MCAN_IE_DRXE_MASK  0x00080000U
#define MCAN_IE_DRXE_SHIFT 19U

// Field:    [18] TOOE
//
// Timeout Occurred Enable
#define MCAN_IE_TOOE       0x00040000U
#define MCAN_IE_TOOE_MASK  0x00040000U
#define MCAN_IE_TOOE_SHIFT 18U

// Field:    [17] MRAFE
//
// Message RAM Access Failure Enable
#define MCAN_IE_MRAFE       0x00020000U
#define MCAN_IE_MRAFE_MASK  0x00020000U
#define MCAN_IE_MRAFE_SHIFT 17U

// Field:    [16] TSWE
//
// Timestamp Wraparound Enable
#define MCAN_IE_TSWE       0x00010000U
#define MCAN_IE_TSWE_MASK  0x00010000U
#define MCAN_IE_TSWE_SHIFT 16U

// Field:    [15] TEFLE
//
// Tx Event FIFO Element Lost Enable
#define MCAN_IE_TEFLE       0x00008000U
#define MCAN_IE_TEFLE_MASK  0x00008000U
#define MCAN_IE_TEFLE_SHIFT 15U

// Field:    [14] TEFFE
//
// Tx Event FIFO Full Enable
#define MCAN_IE_TEFFE       0x00004000U
#define MCAN_IE_TEFFE_MASK  0x00004000U
#define MCAN_IE_TEFFE_SHIFT 14U

// Field:    [13] TEFWE
//
// Tx Event FIFO Watermark Reached Enable
#define MCAN_IE_TEFWE       0x00002000U
#define MCAN_IE_TEFWE_MASK  0x00002000U
#define MCAN_IE_TEFWE_SHIFT 13U

// Field:    [12] TEFNE
//
// Tx Event FIFO New Entry Enable
#define MCAN_IE_TEFNE       0x00001000U
#define MCAN_IE_TEFNE_MASK  0x00001000U
#define MCAN_IE_TEFNE_SHIFT 12U

// Field:    [11] TFEE
//
// Tx FIFO Empty Enable
#define MCAN_IE_TFEE       0x00000800U
#define MCAN_IE_TFEE_MASK  0x00000800U
#define MCAN_IE_TFEE_SHIFT 11U

// Field:    [10] TCFE
//
// Transmission Cancellation Finished Enable
#define MCAN_IE_TCFE       0x00000400U
#define MCAN_IE_TCFE_MASK  0x00000400U
#define MCAN_IE_TCFE_SHIFT 10U

// Field:     [9] TCE
//
// Transmission Completed Enable
#define MCAN_IE_TCE       0x00000200U
#define MCAN_IE_TCE_MASK  0x00000200U
#define MCAN_IE_TCE_SHIFT 9U

// Field:     [8] HPME
//
// High Priority Message Enable
#define MCAN_IE_HPME       0x00000100U
#define MCAN_IE_HPME_MASK  0x00000100U
#define MCAN_IE_HPME_SHIFT 8U

// Field:     [7] RF1LE
//
// Rx FIFO 1 Message Lost Enable
#define MCAN_IE_RF1LE       0x00000080U
#define MCAN_IE_RF1LE_MASK  0x00000080U
#define MCAN_IE_RF1LE_SHIFT 7U

// Field:     [6] RF1FE
//
// Rx FIFO 1 Full Enable
#define MCAN_IE_RF1FE       0x00000040U
#define MCAN_IE_RF1FE_MASK  0x00000040U
#define MCAN_IE_RF1FE_SHIFT 6U

// Field:     [5] RF1WE
//
// Rx FIFO 1 Watermark Reached Enable
#define MCAN_IE_RF1WE       0x00000020U
#define MCAN_IE_RF1WE_MASK  0x00000020U
#define MCAN_IE_RF1WE_SHIFT 5U

// Field:     [4] RF1NE
//
// Rx FIFO 1 New Message Enable
#define MCAN_IE_RF1NE       0x00000010U
#define MCAN_IE_RF1NE_MASK  0x00000010U
#define MCAN_IE_RF1NE_SHIFT 4U

// Field:     [3] RF0LE
//
// Rx FIFO 0 Message Lost Enable
#define MCAN_IE_RF0LE       0x00000008U
#define MCAN_IE_RF0LE_MASK  0x00000008U
#define MCAN_IE_RF0LE_SHIFT 3U

// Field:     [2] RF0FE
//
// Rx FIFO 0 Full Enable
#define MCAN_IE_RF0FE       0x00000004U
#define MCAN_IE_RF0FE_MASK  0x00000004U
#define MCAN_IE_RF0FE_SHIFT 2U

// Field:     [1] RF0WE
//
// Rx FIFO 0 Watermark Reached Enable
#define MCAN_IE_RF0WE       0x00000002U
#define MCAN_IE_RF0WE_MASK  0x00000002U
#define MCAN_IE_RF0WE_SHIFT 1U

// Field:     [0] RF0NE
//
// Rx FIFO 0 New Message Enable
#define MCAN_IE_RF0NE       0x00000001U
#define MCAN_IE_RF0NE_MASK  0x00000001U
#define MCAN_IE_RF0NE_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_ILS
//
//*****************************************************************************
// Field:    [29] ARAL
//
// Access to Reserved Address Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_ARAL       0x20000000U
#define MCAN_ILS_ARAL_MASK  0x20000000U
#define MCAN_ILS_ARAL_SHIFT 29U

// Field:    [28] PEDL
//
// Protocol Error in Data Phase Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_PEDL       0x10000000U
#define MCAN_ILS_PEDL_MASK  0x10000000U
#define MCAN_ILS_PEDL_SHIFT 28U

// Field:    [27] PEAL
//
// Protocol Error in Arbitration Phase Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_PEAL       0x08000000U
#define MCAN_ILS_PEAL_MASK  0x08000000U
#define MCAN_ILS_PEAL_SHIFT 27U

// Field:    [26] WDIL
//
// Watchdog Interrupt Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_WDIL       0x04000000U
#define MCAN_ILS_WDIL_MASK  0x04000000U
#define MCAN_ILS_WDIL_SHIFT 26U

// Field:    [25] BOL
//
// Bus_Off Status Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_BOL       0x02000000U
#define MCAN_ILS_BOL_MASK  0x02000000U
#define MCAN_ILS_BOL_SHIFT 25U

// Field:    [24] EWL
//
// Warning Status Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_EWL       0x01000000U
#define MCAN_ILS_EWL_MASK  0x01000000U
#define MCAN_ILS_EWL_SHIFT 24U

// Field:    [23] EPL
//
// Error Passive Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_EPL       0x00800000U
#define MCAN_ILS_EPL_MASK  0x00800000U
#define MCAN_ILS_EPL_SHIFT 23U

// Field:    [22] ELOL
//
// Error Logging Overflow Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_ELOL       0x00400000U
#define MCAN_ILS_ELOL_MASK  0x00400000U
#define MCAN_ILS_ELOL_SHIFT 22U

// Field:    [21] BEUL
//
// Bit Error Uncorrected Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_BEUL       0x00200000U
#define MCAN_ILS_BEUL_MASK  0x00200000U
#define MCAN_ILS_BEUL_SHIFT 21U

// Field:    [20] BECL
//
// Bit Error Corrected Line
//
// A separate interrupt line reserved for corrected bit errors is provided via
// the MCAN_ERROR_REGS. It advised for the user to use these registers and
// leave the MCAN_IE.BECE bit cleared to '0' (disabled), thereby relegating
// this bit to not applicable.
#define MCAN_ILS_BECL       0x00100000U
#define MCAN_ILS_BECL_MASK  0x00100000U
#define MCAN_ILS_BECL_SHIFT 20U

// Field:    [19] DRXL
//
// Message Stored to Dedicated Rx Buffer Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_DRXL       0x00080000U
#define MCAN_ILS_DRXL_MASK  0x00080000U
#define MCAN_ILS_DRXL_SHIFT 19U

// Field:    [18] TOOL
//
// Timeout Occurred Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_TOOL       0x00040000U
#define MCAN_ILS_TOOL_MASK  0x00040000U
#define MCAN_ILS_TOOL_SHIFT 18U

// Field:    [17] MRAFL
//
// Message RAM Access Failure Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_MRAFL       0x00020000U
#define MCAN_ILS_MRAFL_MASK  0x00020000U
#define MCAN_ILS_MRAFL_SHIFT 17U

// Field:    [16] TSWL
//
// Timestamp Wraparound Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_TSWL       0x00010000U
#define MCAN_ILS_TSWL_MASK  0x00010000U
#define MCAN_ILS_TSWL_SHIFT 16U

// Field:    [15] TEFLL
//
// Tx Event FIFO Element Lost Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_TEFLL       0x00008000U
#define MCAN_ILS_TEFLL_MASK  0x00008000U
#define MCAN_ILS_TEFLL_SHIFT 15U

// Field:    [14] TEFFL
//
// Tx Event FIFO Full Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_TEFFL       0x00004000U
#define MCAN_ILS_TEFFL_MASK  0x00004000U
#define MCAN_ILS_TEFFL_SHIFT 14U

// Field:    [13] TEFWL
//
// Tx Event FIFO Watermark Reached Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_TEFWL       0x00002000U
#define MCAN_ILS_TEFWL_MASK  0x00002000U
#define MCAN_ILS_TEFWL_SHIFT 13U

// Field:    [12] TEFNL
//
// Tx Event FIFO New Entry Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_TEFNL       0x00001000U
#define MCAN_ILS_TEFNL_MASK  0x00001000U
#define MCAN_ILS_TEFNL_SHIFT 12U

// Field:    [11] TFEL
//
// Tx FIFO Empty Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_TFEL       0x00000800U
#define MCAN_ILS_TFEL_MASK  0x00000800U
#define MCAN_ILS_TFEL_SHIFT 11U

// Field:    [10] TCFL
//
// Transmission Cancellation Finished Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_TCFL       0x00000400U
#define MCAN_ILS_TCFL_MASK  0x00000400U
#define MCAN_ILS_TCFL_SHIFT 10U

// Field:     [9] TCL
//
// Transmission Completed Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_TCL       0x00000200U
#define MCAN_ILS_TCL_MASK  0x00000200U
#define MCAN_ILS_TCL_SHIFT 9U

// Field:     [8] HPML
//
// High Priority Message Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_HPML       0x00000100U
#define MCAN_ILS_HPML_MASK  0x00000100U
#define MCAN_ILS_HPML_SHIFT 8U

// Field:     [7] RF1LL
//
// Rx FIFO 1 Message Lost Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_RF1LL       0x00000080U
#define MCAN_ILS_RF1LL_MASK  0x00000080U
#define MCAN_ILS_RF1LL_SHIFT 7U

// Field:     [6] RF1FL
//
// Rx FIFO 1 Full Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_RF1FL       0x00000040U
#define MCAN_ILS_RF1FL_MASK  0x00000040U
#define MCAN_ILS_RF1FL_SHIFT 6U

// Field:     [5] RF1WL
//
// Rx FIFO 1 Watermark Reached Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_RF1WL       0x00000020U
#define MCAN_ILS_RF1WL_MASK  0x00000020U
#define MCAN_ILS_RF1WL_SHIFT 5U

// Field:     [4] RF1NL
//
// Rx FIFO 1 New Message Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_RF1NL       0x00000010U
#define MCAN_ILS_RF1NL_MASK  0x00000010U
#define MCAN_ILS_RF1NL_SHIFT 4U

// Field:     [3] RF0LL
//
// Rx FIFO 0 Message Lost Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_RF0LL       0x00000008U
#define MCAN_ILS_RF0LL_MASK  0x00000008U
#define MCAN_ILS_RF0LL_SHIFT 3U

// Field:     [2] RF0FL
//
// Rx FIFO 0 Full Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_RF0FL       0x00000004U
#define MCAN_ILS_RF0FL_MASK  0x00000004U
#define MCAN_ILS_RF0FL_SHIFT 2U

// Field:     [1] RF0WL
//
// Rx FIFO 0 Watermark Reached Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_RF0WL       0x00000002U
#define MCAN_ILS_RF0WL_MASK  0x00000002U
#define MCAN_ILS_RF0WL_SHIFT 1U

// Field:     [0] RF0NL
//
// Rx FIFO 0 New Message Line
//   0  Interrupt source is assigned to Interrupt Line 0
//   1  Interrupt source is assigned to Interrupt Line 1
#define MCAN_ILS_RF0NL       0x00000001U
#define MCAN_ILS_RF0NL_MASK  0x00000001U
#define MCAN_ILS_RF0NL_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_ILE
//
//*****************************************************************************
// Field:     [1] EINT1
//
// Enable Interrupt Line 1
//   0  Interrupt Line 1 is disabled
//   1  Interrupt Line 1 is enabled
#define MCAN_ILE_EINT1       0x00000002U
#define MCAN_ILE_EINT1_MASK  0x00000002U
#define MCAN_ILE_EINT1_SHIFT 1U

// Field:     [0] EINT0
//
// Enable Interrupt Line 0
//   0  Interrupt Line 0 is disabled
//   1  Interrupt Line 0 is enabled
#define MCAN_ILE_EINT0       0x00000001U
#define MCAN_ILE_EINT0_MASK  0x00000001U
#define MCAN_ILE_EINT0_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_GFC
//
//*****************************************************************************
// Field:   [5:4] ANFS
//
// Accept Non-matching Frames Standard. Defines how received messages with
// 11-bit IDs that do not match any element of the filter list are treated.
//   00  Accept in Rx FIFO 0
//   01  Accept in Rx FIFO 1
//   10  Reject
//   11  Reject
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_GFC_ANFS_WIDTH 2U
#define MCAN_GFC_ANFS_MASK  0x00000030U
#define MCAN_GFC_ANFS_SHIFT 4U

// Field:   [3:2] ANFE
//
// Accept Non-matching Frames Extended. Defines how received messages with
// 29-bit IDs that do not match any element of the filter list are treated.
//   00  Accept in Rx FIFO 0
//   01  Accept in Rx FIFO 1
//   10  Reject
//   11  Reject
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_GFC_ANFE_WIDTH 2U
#define MCAN_GFC_ANFE_MASK  0x0000000CU
#define MCAN_GFC_ANFE_SHIFT 2U

// Field:     [1] RRFS
//
// Reject Remote Frames Standard
//   0  Filter remote frames with 11-bit standard IDs
//   1  Reject all remote frames with 11-bit standard IDs
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_GFC_RRFS       0x00000002U
#define MCAN_GFC_RRFS_MASK  0x00000002U
#define MCAN_GFC_RRFS_SHIFT 1U

// Field:     [0] RRFE
//
// Reject Remote Frames Extended
//   0  Filter remote frames with 29-bit extended IDs
//   1  Reject all remote frames with 29-bit extended IDs
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_GFC_RRFE       0x00000001U
#define MCAN_GFC_RRFE_MASK  0x00000001U
#define MCAN_GFC_RRFE_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_SIDFC
//
//*****************************************************************************
// Field: [23:16] LSS
//
// List Size Standard
//   0        No standard Message ID filter
//   1-128  Number of standard Message ID filter elements
//   >128   Values greater than 128 are interpreted as 128
#define MCAN_SIDFC_LSS_WIDTH 8U
#define MCAN_SIDFC_LSS_MASK  0x00FF0000U
#define MCAN_SIDFC_LSS_SHIFT 16U

// Field:  [15:2] FLSSA
//
// Filter List Standard Start Address. Start address of standard Message ID
// filter list (32-bit word address).
#define MCAN_SIDFC_FLSSA_WIDTH 14U
#define MCAN_SIDFC_FLSSA_MASK  0x0000FFFCU
#define MCAN_SIDFC_FLSSA_SHIFT 2U

//*****************************************************************************
//
// Register: MCAN_XIDFC
//
//*****************************************************************************
// Field: [22:16] LSE
//
// Filter List Extended Start Address. Start address of extended Message ID
// filter list (32-bit word address).
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_XIDFC_LSE_WIDTH 7U
#define MCAN_XIDFC_LSE_MASK  0x007F0000U
#define MCAN_XIDFC_LSE_SHIFT 16U

// Field:  [15:2] FLESA
//
// List Size Extended
//   0     No extended Message ID filter
//   1-64 Number of extended Message ID filter elements
//   >64  Values greater than 64 are interpreted as 64
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_XIDFC_FLESA_WIDTH 14U
#define MCAN_XIDFC_FLESA_MASK  0x0000FFFCU
#define MCAN_XIDFC_FLESA_SHIFT 2U

//*****************************************************************************
//
// Register: MCAN_XIDAM
//
//*****************************************************************************
// Field:  [28:0] EIDM
//
// Extended ID Mask. For acceptance filtering of extended frames the Extended
// ID AND Mask is ANDed with the Message ID of a received frame. Intended for
// masking of 29-bit IDs in SAE J1939. With the reset value of all bits set to
// one the mask is not active.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_XIDAM_EIDM_WIDTH 29U
#define MCAN_XIDAM_EIDM_MASK  0x1FFFFFFFU
#define MCAN_XIDAM_EIDM_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_HPMS
//
//*****************************************************************************
// Field:    [15] FLST
//
// Filter List. Indicates the filter list of the matching filter element.
//   0  Standard Filter List
//   1  Extended Filter List
#define MCAN_HPMS_FLST       0x00008000U
#define MCAN_HPMS_FLST_MASK  0x00008000U
#define MCAN_HPMS_FLST_SHIFT 15U

// Field:  [14:8] FIDX
//
// Filter Index. Index of matching filter element. Range is 0 to SIDFC.LSS - 1
// resp. XIDFC.LSE - 1.
#define MCAN_HPMS_FIDX_WIDTH 7U
#define MCAN_HPMS_FIDX_MASK  0x00007F00U
#define MCAN_HPMS_FIDX_SHIFT 8U

// Field:   [7:6] MSI
//
// Message Storage Indicator
//   00  No FIFO selected
//   01  FIFO message lost
//   10  Message stored in FIFO 0
//   11  Message stored in FIFO 1
#define MCAN_HPMS_MSI_WIDTH 2U
#define MCAN_HPMS_MSI_MASK  0x000000C0U
#define MCAN_HPMS_MSI_SHIFT 6U

// Field:   [5:0] BIDX
//
// Buffer Index. Index of Rx FIFO element to which the message was stored. Only
// valid when MSI(1) = '1'.
#define MCAN_HPMS_BIDX_WIDTH 6U
#define MCAN_HPMS_BIDX_MASK  0x0000003FU
#define MCAN_HPMS_BIDX_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_NDAT1
//
//*****************************************************************************
// Field:    [31] ND31
//
// New Data RX Buffer 31
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND31       0x80000000U
#define MCAN_NDAT1_ND31_MASK  0x80000000U
#define MCAN_NDAT1_ND31_SHIFT 31U

// Field:    [30] ND30
//
// New Data RX Buffer 30
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND30       0x40000000U
#define MCAN_NDAT1_ND30_MASK  0x40000000U
#define MCAN_NDAT1_ND30_SHIFT 30U

// Field:    [29] ND29
//
// New Data RX Buffer 29
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND29       0x20000000U
#define MCAN_NDAT1_ND29_MASK  0x20000000U
#define MCAN_NDAT1_ND29_SHIFT 29U

// Field:    [28] ND28
//
// New Data RX Buffer 28
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND28       0x10000000U
#define MCAN_NDAT1_ND28_MASK  0x10000000U
#define MCAN_NDAT1_ND28_SHIFT 28U

// Field:    [27] ND27
//
// New Data RX Buffer 27
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND27       0x08000000U
#define MCAN_NDAT1_ND27_MASK  0x08000000U
#define MCAN_NDAT1_ND27_SHIFT 27U

// Field:    [26] ND26
//
// New Data RX Buffer 26
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND26       0x04000000U
#define MCAN_NDAT1_ND26_MASK  0x04000000U
#define MCAN_NDAT1_ND26_SHIFT 26U

// Field:    [25] ND25
//
// New Data RX Buffer 25
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND25       0x02000000U
#define MCAN_NDAT1_ND25_MASK  0x02000000U
#define MCAN_NDAT1_ND25_SHIFT 25U

// Field:    [24] ND24
//
// New Data RX Buffer 24
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND24       0x01000000U
#define MCAN_NDAT1_ND24_MASK  0x01000000U
#define MCAN_NDAT1_ND24_SHIFT 24U

// Field:    [23] ND23
//
// New Data RX Buffer 23
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND23       0x00800000U
#define MCAN_NDAT1_ND23_MASK  0x00800000U
#define MCAN_NDAT1_ND23_SHIFT 23U

// Field:    [22] ND22
//
// New Data RX Buffer 22
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND22       0x00400000U
#define MCAN_NDAT1_ND22_MASK  0x00400000U
#define MCAN_NDAT1_ND22_SHIFT 22U

// Field:    [21] ND21
//
// New Data RX Buffer 21
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND21       0x00200000U
#define MCAN_NDAT1_ND21_MASK  0x00200000U
#define MCAN_NDAT1_ND21_SHIFT 21U

// Field:    [20] ND20
//
// New Data RX Buffer 20
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND20       0x00100000U
#define MCAN_NDAT1_ND20_MASK  0x00100000U
#define MCAN_NDAT1_ND20_SHIFT 20U

// Field:    [19] ND19
//
// New Data RX Buffer 19
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND19       0x00080000U
#define MCAN_NDAT1_ND19_MASK  0x00080000U
#define MCAN_NDAT1_ND19_SHIFT 19U

// Field:    [18] ND18
//
// New Data RX Buffer 18
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND18       0x00040000U
#define MCAN_NDAT1_ND18_MASK  0x00040000U
#define MCAN_NDAT1_ND18_SHIFT 18U

// Field:    [17] ND17
//
// New Data RX Buffer 17
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND17       0x00020000U
#define MCAN_NDAT1_ND17_MASK  0x00020000U
#define MCAN_NDAT1_ND17_SHIFT 17U

// Field:    [16] ND16
//
// New Data RX Buffer 16
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND16       0x00010000U
#define MCAN_NDAT1_ND16_MASK  0x00010000U
#define MCAN_NDAT1_ND16_SHIFT 16U

// Field:    [15] ND15
//
// New Data RX Buffer 15
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND15       0x00008000U
#define MCAN_NDAT1_ND15_MASK  0x00008000U
#define MCAN_NDAT1_ND15_SHIFT 15U

// Field:    [14] ND14
//
// New Data RX Buffer 14
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND14       0x00004000U
#define MCAN_NDAT1_ND14_MASK  0x00004000U
#define MCAN_NDAT1_ND14_SHIFT 14U

// Field:    [13] ND13
//
// New Data RX Buffer 13
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND13       0x00002000U
#define MCAN_NDAT1_ND13_MASK  0x00002000U
#define MCAN_NDAT1_ND13_SHIFT 13U

// Field:    [12] ND12
//
// New Data RX Buffer 12
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND12       0x00001000U
#define MCAN_NDAT1_ND12_MASK  0x00001000U
#define MCAN_NDAT1_ND12_SHIFT 12U

// Field:    [11] ND11
//
// New Data RX Buffer 11
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND11       0x00000800U
#define MCAN_NDAT1_ND11_MASK  0x00000800U
#define MCAN_NDAT1_ND11_SHIFT 11U

// Field:    [10] ND10
//
// New Data RX Buffer 10
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND10       0x00000400U
#define MCAN_NDAT1_ND10_MASK  0x00000400U
#define MCAN_NDAT1_ND10_SHIFT 10U

// Field:     [9] ND9
//
// New Data RX Buffer 9
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND9       0x00000200U
#define MCAN_NDAT1_ND9_MASK  0x00000200U
#define MCAN_NDAT1_ND9_SHIFT 9U

// Field:     [8] ND8
//
// New Data RX Buffer 8
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND8       0x00000100U
#define MCAN_NDAT1_ND8_MASK  0x00000100U
#define MCAN_NDAT1_ND8_SHIFT 8U

// Field:     [7] ND7
//
// New Data RX Buffer 7
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND7       0x00000080U
#define MCAN_NDAT1_ND7_MASK  0x00000080U
#define MCAN_NDAT1_ND7_SHIFT 7U

// Field:     [6] ND6
//
// New Data RX Buffer 6
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND6       0x00000040U
#define MCAN_NDAT1_ND6_MASK  0x00000040U
#define MCAN_NDAT1_ND6_SHIFT 6U

// Field:     [5] ND5
//
// New Data RX Buffer 5
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND5       0x00000020U
#define MCAN_NDAT1_ND5_MASK  0x00000020U
#define MCAN_NDAT1_ND5_SHIFT 5U

// Field:     [4] ND4
//
// New Data RX Buffer 4
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND4       0x00000010U
#define MCAN_NDAT1_ND4_MASK  0x00000010U
#define MCAN_NDAT1_ND4_SHIFT 4U

// Field:     [3] ND3
//
// New Data RX Buffer 3
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND3       0x00000008U
#define MCAN_NDAT1_ND3_MASK  0x00000008U
#define MCAN_NDAT1_ND3_SHIFT 3U

// Field:     [2] ND2
//
// New Data RX Buffer 2
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND2       0x00000004U
#define MCAN_NDAT1_ND2_MASK  0x00000004U
#define MCAN_NDAT1_ND2_SHIFT 2U

// Field:     [1] ND1
//
// New Data RX Buffer 1
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND1       0x00000002U
#define MCAN_NDAT1_ND1_MASK  0x00000002U
#define MCAN_NDAT1_ND1_SHIFT 1U

// Field:     [0] ND0
//
// New Data RX Buffer 0
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT1_ND0       0x00000001U
#define MCAN_NDAT1_ND0_MASK  0x00000001U
#define MCAN_NDAT1_ND0_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_NDAT2
//
//*****************************************************************************
// Field:    [31] ND63
//
// New Data RX Buffer 63
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND63       0x80000000U
#define MCAN_NDAT2_ND63_MASK  0x80000000U
#define MCAN_NDAT2_ND63_SHIFT 31U

// Field:    [30] ND62
//
// New Data RX Buffer 62
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND62       0x40000000U
#define MCAN_NDAT2_ND62_MASK  0x40000000U
#define MCAN_NDAT2_ND62_SHIFT 30U

// Field:    [29] ND61
//
// New Data RX Buffer 61
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND61       0x20000000U
#define MCAN_NDAT2_ND61_MASK  0x20000000U
#define MCAN_NDAT2_ND61_SHIFT 29U

// Field:    [28] ND60
//
// New Data RX Buffer 60
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND60       0x10000000U
#define MCAN_NDAT2_ND60_MASK  0x10000000U
#define MCAN_NDAT2_ND60_SHIFT 28U

// Field:    [27] ND59
//
// New Data RX Buffer 59
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND59       0x08000000U
#define MCAN_NDAT2_ND59_MASK  0x08000000U
#define MCAN_NDAT2_ND59_SHIFT 27U

// Field:    [26] ND58
//
// New Data RX Buffer 58
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND58       0x04000000U
#define MCAN_NDAT2_ND58_MASK  0x04000000U
#define MCAN_NDAT2_ND58_SHIFT 26U

// Field:    [25] ND57
//
// New Data RX Buffer 57
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND57       0x02000000U
#define MCAN_NDAT2_ND57_MASK  0x02000000U
#define MCAN_NDAT2_ND57_SHIFT 25U

// Field:    [24] ND56
//
// New Data RX Buffer 56
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND56       0x01000000U
#define MCAN_NDAT2_ND56_MASK  0x01000000U
#define MCAN_NDAT2_ND56_SHIFT 24U

// Field:    [23] ND55
//
// New Data RX Buffer 55
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND55       0x00800000U
#define MCAN_NDAT2_ND55_MASK  0x00800000U
#define MCAN_NDAT2_ND55_SHIFT 23U

// Field:    [22] ND54
//
// New Data RX Buffer 54
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND54       0x00400000U
#define MCAN_NDAT2_ND54_MASK  0x00400000U
#define MCAN_NDAT2_ND54_SHIFT 22U

// Field:    [21] ND53
//
// New Data RX Buffer 53
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND53       0x00200000U
#define MCAN_NDAT2_ND53_MASK  0x00200000U
#define MCAN_NDAT2_ND53_SHIFT 21U

// Field:    [20] ND52
//
// New Data RX Buffer 52
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND52       0x00100000U
#define MCAN_NDAT2_ND52_MASK  0x00100000U
#define MCAN_NDAT2_ND52_SHIFT 20U

// Field:    [19] ND51
//
// New Data RX Buffer 51
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND51       0x00080000U
#define MCAN_NDAT2_ND51_MASK  0x00080000U
#define MCAN_NDAT2_ND51_SHIFT 19U

// Field:    [18] ND50
//
// New Data RX Buffer 50
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND50       0x00040000U
#define MCAN_NDAT2_ND50_MASK  0x00040000U
#define MCAN_NDAT2_ND50_SHIFT 18U

// Field:    [17] ND49
//
// New Data RX Buffer 49
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND49       0x00020000U
#define MCAN_NDAT2_ND49_MASK  0x00020000U
#define MCAN_NDAT2_ND49_SHIFT 17U

// Field:    [16] ND48
//
// New Data RX Buffer 48
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND48       0x00010000U
#define MCAN_NDAT2_ND48_MASK  0x00010000U
#define MCAN_NDAT2_ND48_SHIFT 16U

// Field:    [15] ND47
//
// New Data RX Buffer 47
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND47       0x00008000U
#define MCAN_NDAT2_ND47_MASK  0x00008000U
#define MCAN_NDAT2_ND47_SHIFT 15U

// Field:    [14] ND46
//
// New Data RX Buffer 46
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND46       0x00004000U
#define MCAN_NDAT2_ND46_MASK  0x00004000U
#define MCAN_NDAT2_ND46_SHIFT 14U

// Field:    [13] ND45
//
// New Data RX Buffer 45
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND45       0x00002000U
#define MCAN_NDAT2_ND45_MASK  0x00002000U
#define MCAN_NDAT2_ND45_SHIFT 13U

// Field:    [12] ND44
//
// New Data RX Buffer 44
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND44       0x00001000U
#define MCAN_NDAT2_ND44_MASK  0x00001000U
#define MCAN_NDAT2_ND44_SHIFT 12U

// Field:    [11] ND43
//
// New Data RX Buffer 43
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND43       0x00000800U
#define MCAN_NDAT2_ND43_MASK  0x00000800U
#define MCAN_NDAT2_ND43_SHIFT 11U

// Field:    [10] ND42
//
// New Data RX Buffer 42
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND42       0x00000400U
#define MCAN_NDAT2_ND42_MASK  0x00000400U
#define MCAN_NDAT2_ND42_SHIFT 10U

// Field:     [9] ND41
//
// New Data RX Buffer 41
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND41       0x00000200U
#define MCAN_NDAT2_ND41_MASK  0x00000200U
#define MCAN_NDAT2_ND41_SHIFT 9U

// Field:     [8] ND40
//
// New Data RX Buffer 40
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND40       0x00000100U
#define MCAN_NDAT2_ND40_MASK  0x00000100U
#define MCAN_NDAT2_ND40_SHIFT 8U

// Field:     [7] ND39
//
// New Data RX Buffer 39
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND39       0x00000080U
#define MCAN_NDAT2_ND39_MASK  0x00000080U
#define MCAN_NDAT2_ND39_SHIFT 7U

// Field:     [6] ND38
//
// New Data RX Buffer 38
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND38       0x00000040U
#define MCAN_NDAT2_ND38_MASK  0x00000040U
#define MCAN_NDAT2_ND38_SHIFT 6U

// Field:     [5] ND37
//
// New Data RX Buffer 37
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND37       0x00000020U
#define MCAN_NDAT2_ND37_MASK  0x00000020U
#define MCAN_NDAT2_ND37_SHIFT 5U

// Field:     [4] ND36
//
// New Data RX Buffer 36
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND36       0x00000010U
#define MCAN_NDAT2_ND36_MASK  0x00000010U
#define MCAN_NDAT2_ND36_SHIFT 4U

// Field:     [3] ND35
//
// New Data RX Buffer 35
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND35       0x00000008U
#define MCAN_NDAT2_ND35_MASK  0x00000008U
#define MCAN_NDAT2_ND35_SHIFT 3U

// Field:     [2] ND34
//
// New Data RX Buffer 34
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND34       0x00000004U
#define MCAN_NDAT2_ND34_MASK  0x00000004U
#define MCAN_NDAT2_ND34_SHIFT 2U

// Field:     [1] ND33
//
// New Data RX Buffer 33
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND33       0x00000002U
#define MCAN_NDAT2_ND33_MASK  0x00000002U
#define MCAN_NDAT2_ND33_SHIFT 1U

// Field:     [0] ND32
//
// New Data RX Buffer 32
//   0  Rx Buffer not updated
//   1  Rx Buffer updated from new message
#define MCAN_NDAT2_ND32       0x00000001U
#define MCAN_NDAT2_ND32_MASK  0x00000001U
#define MCAN_NDAT2_ND32_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_RXF0C
//
//*****************************************************************************
// Field:    [31] F0OM
//
// FIFO 0 Operation Mode. FIFO 0 can be operated in blocking or in overwrite
// mode.
//   0  FIFO 0 blocking mode
//   1  FIFO 0 overwrite mode
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_RXF0C_F0OM       0x80000000U
#define MCAN_RXF0C_F0OM_MASK  0x80000000U
#define MCAN_RXF0C_F0OM_SHIFT 31U

// Field: [30:24] F0WM
//
// Rx FIFO 0 Watermark
//   0      Watermark interrupt disabled
//   1-64  Level for Rx FIFO 0 watermark interrupt (IR.RF0W)
//   >64   Watermark interrupt disabled
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_RXF0C_F0WM_WIDTH 7U
#define MCAN_RXF0C_F0WM_MASK  0x7F000000U
#define MCAN_RXF0C_F0WM_SHIFT 24U

// Field: [22:16] F0S
//
// Rx FIFO 0 Size. The Rx FIFO 0 elements are indexed from 0 to F0S-1.
//   0      No Rx FIFO 0
//   1-64  Number of Rx FIFO 0 elements
//   >64   Values greater than 64 are interpreted as 64
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_RXF0C_F0S_WIDTH 7U
#define MCAN_RXF0C_F0S_MASK  0x007F0000U
#define MCAN_RXF0C_F0S_SHIFT 16U

// Field:  [15:2] F0SA
//
// Rx FIFO 0 Start Address. Start address of Rx FIFO 0 in Message RAM (32-bit
// word address).
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_RXF0C_F0SA_WIDTH 14U
#define MCAN_RXF0C_F0SA_MASK  0x0000FFFCU
#define MCAN_RXF0C_F0SA_SHIFT 2U

//*****************************************************************************
//
// Register: MCAN_RXF0S
//
//*****************************************************************************
// Field:    [25] RF0L
//
// Rx FIFO 0 Message Lost. This bit is a copy of interrupt flag IR.RF0L. When
// IR.RF0L is reset, this bit is also reset.
//   0  No Rx FIFO 0 message lost
//   1  Rx FIFO 0 message lost, also set after write attempt to Rx FIFO 0 of
// size zero
//
// Note: Overwriting the oldest message when RXF0C.F0OM = '1' will not set this
// flag.
#define MCAN_RXF0S_RF0L       0x02000000U
#define MCAN_RXF0S_RF0L_MASK  0x02000000U
#define MCAN_RXF0S_RF0L_SHIFT 25U

// Field:    [24] F0F
//
// Rx FIFO 0 Full
//   0  Rx FIFO 0 not full
//   1  Rx FIFO 0 full
#define MCAN_RXF0S_F0F       0x01000000U
#define MCAN_RXF0S_F0F_MASK  0x01000000U
#define MCAN_RXF0S_F0F_SHIFT 24U

// Field: [21:16] F0PI
//
// Rx FIFO 0 Put Index. Rx FIFO 0 write index pointer, range 0 to 63.
#define MCAN_RXF0S_F0PI_WIDTH 6U
#define MCAN_RXF0S_F0PI_MASK  0x003F0000U
#define MCAN_RXF0S_F0PI_SHIFT 16U

// Field:  [13:8] F0GI
//
// Rx FIFO 0 Get Index. Rx FIFO 0 read index pointer, range 0 to 63.
#define MCAN_RXF0S_F0GI_WIDTH 6U
#define MCAN_RXF0S_F0GI_MASK  0x00003F00U
#define MCAN_RXF0S_F0GI_SHIFT 8U

// Field:   [6:0] F0FL
//
// Rx FIFO 0 Fill Level. Number of elements stored in Rx FIFO 0, range 0 to 64.
#define MCAN_RXF0S_F0FL_WIDTH 7U
#define MCAN_RXF0S_F0FL_MASK  0x0000007FU
#define MCAN_RXF0S_F0FL_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_RXF0A
//
//*****************************************************************************
// Field:   [5:0] F0AI
//
// Rx FIFO 0 Acknowledge Index. After the Host has read a message or a sequence
// of messages from Rx FIFO 0 it has to write the buffer index of the last
// element read from Rx FIFO 0 to F0AI. This will set the Rx FIFO 0 Get Index
// RXF0S.F0GI to F0AI + 1 and update the FIFO 0 Fill Level RXF0S.F0FL.
#define MCAN_RXF0A_F0AI_WIDTH 6U
#define MCAN_RXF0A_F0AI_MASK  0x0000003FU
#define MCAN_RXF0A_F0AI_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_RXBC
//
//*****************************************************************************
// Field:  [15:2] RBSA
//
// Rx Buffer Start Address. Configures the start address of the Rx Buffers
// section in the Message RAM (32-bit word address).
#define MCAN_RXBC_RBSA_WIDTH 14U
#define MCAN_RXBC_RBSA_MASK  0x0000FFFCU
#define MCAN_RXBC_RBSA_SHIFT 2U

//*****************************************************************************
//
// Register: MCAN_RXF1C
//
//*****************************************************************************
// Field:    [31] F1OM
//
// FIFO 1 Operation Mode. FIFO 1 can be operated in blocking or in overwrite
// mode.
//   0  FIFO 1 blocking mode
//   1  FIFO 1 overwrite mode
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_RXF1C_F1OM       0x80000000U
#define MCAN_RXF1C_F1OM_MASK  0x80000000U
#define MCAN_RXF1C_F1OM_SHIFT 31U

// Field: [30:24] F1WM
//
// Rx FIFO 1 Watermark
//   0      Watermark interrupt disabled
//   1-64  Level for Rx FIFO 1 watermark interrupt (IR.RF1W)
//   >64   Watermark interrupt disabled
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_RXF1C_F1WM_WIDTH 7U
#define MCAN_RXF1C_F1WM_MASK  0x7F000000U
#define MCAN_RXF1C_F1WM_SHIFT 24U

// Field: [22:16] F1S
//
// Rx FIFO 1 Size. The Rx FIFO 1 elements are indexed from 0 to F1S - 1.
//   0      No Rx FIFO 1
//   1-64  Number of Rx FIFO 1 elements
//   >64   Values greater than 64 are interpreted as 64
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_RXF1C_F1S_WIDTH 7U
#define MCAN_RXF1C_F1S_MASK  0x007F0000U
#define MCAN_RXF1C_F1S_SHIFT 16U

// Field:  [15:2] F1SA
//
// Rx FIFO 1 Start Address Start address of Rx FIFO 1 in Message RAM (32-bit
// word address).
#define MCAN_RXF1C_F1SA_WIDTH 14U
#define MCAN_RXF1C_F1SA_MASK  0x0000FFFCU
#define MCAN_RXF1C_F1SA_SHIFT 2U

//*****************************************************************************
//
// Register: MCAN_RXF1S
//
//*****************************************************************************
// Field: [31:30] DMS
//
// Debug Message Status
//   00  Idle state, wait for reception of debug messages, DMA request is
// cleared
//   01  Debug message A received
//   10  Debug messages A, B received
//   11  Debug messages A, B, C received, DMA request is set
#define MCAN_RXF1S_DMS_WIDTH 2U
#define MCAN_RXF1S_DMS_MASK  0xC0000000U
#define MCAN_RXF1S_DMS_SHIFT 30U

// Field:    [25] RF1L
//
// Rx FIFO 1 Message Lost. This bit is a copy of interrupt flag IR.RF1L. When
// IR.RF1L is reset, this bit is also reset.
//   0  No Rx FIFO 1 message lost
//   1  Rx FIFO 1 message lost, also set after write attempt to Rx FIFO 1 of
// size zero
//
// Note: Overwriting the oldest message when RXF1C.F1OM = '1' will not set this
// flag.
#define MCAN_RXF1S_RF1L       0x02000000U
#define MCAN_RXF1S_RF1L_MASK  0x02000000U
#define MCAN_RXF1S_RF1L_SHIFT 25U

// Field:    [24] F1F
//
// Rx FIFO 1 Full
//   0  Rx FIFO 1 not full
//   1  Rx FIFO 1 full
#define MCAN_RXF1S_F1F       0x01000000U
#define MCAN_RXF1S_F1F_MASK  0x01000000U
#define MCAN_RXF1S_F1F_SHIFT 24U

// Field: [21:16] F1PI
//
// Rx FIFO 1 Put Index. Rx FIFO 1 write index pointer, range 0 to 63.
#define MCAN_RXF1S_F1PI_WIDTH 6U
#define MCAN_RXF1S_F1PI_MASK  0x003F0000U
#define MCAN_RXF1S_F1PI_SHIFT 16U

// Field:  [13:8] F1GI
//
// Rx FIFO 1 Get Index. Rx FIFO 1 read index pointer, range 0 to 63.
#define MCAN_RXF1S_F1GI_WIDTH 6U
#define MCAN_RXF1S_F1GI_MASK  0x00003F00U
#define MCAN_RXF1S_F1GI_SHIFT 8U

// Field:   [6:0] F1FL
//
// Rx FIFO 1 Fill Level. Number of elements stored in Rx FIFO 1, range 0 to 64.
#define MCAN_RXF1S_F1FL_WIDTH 7U
#define MCAN_RXF1S_F1FL_MASK  0x0000007FU
#define MCAN_RXF1S_F1FL_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_RXF1A
//
//*****************************************************************************
// Field:   [5:0] F1AI
//
// Rx FIFO 1 Acknowledge Index. After the Host has read a message or a sequence
// of messages from Rx FIFO 1 it has to write the buffer index of the last
// element read from Rx FIFO 1 to F1AI. This will set the Rx FIFO 1 Get Index
// RXF1S.F1GI to F1AI + 1 and update the FIFO 1 Fill Level RXF1S.F1FL.
#define MCAN_RXF1A_F1AI_WIDTH 6U
#define MCAN_RXF1A_F1AI_MASK  0x0000003FU
#define MCAN_RXF1A_F1AI_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_RXESC
//
//*****************************************************************************
// Field:  [10:8] RBDS
//
// Rx Buffer Data Field Size
//   000  8 byte data field
//   001  12 byte data field
//   010  16 byte data field
//   011  20 byte data field
//   100  24 byte data field
//   101  32 byte data field
//   110  48 byte data field
//   111  64 byte data field
//
// Note: In case the data field size of an accepted CAN frame exceeds the data
// field size configured for the matching Rx Buffer or Rx FIFO, only the number
// of bytes as configured by RXESC are stored to the Rx Buffer resp. Rx FIFO
// element. The rest of the frame's data field is ignored.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_RXESC_RBDS_WIDTH 3U
#define MCAN_RXESC_RBDS_MASK  0x00000700U
#define MCAN_RXESC_RBDS_SHIFT 8U

// Field:   [6:4] F1DS
//
// Rx FIFO 1 Data Field Size
//   000  8 byte data field
//   001  12 byte data field
//   010  16 byte data field
//   011  20 byte data field
//   100  24 byte data field
//   101  32 byte data field
//   110  48 byte data field
//   111  64 byte data field
//
// Note: In case the data field size of an accepted CAN frame exceeds the data
// field size configured for the matching Rx Buffer or Rx FIFO, only the number
// of bytes as configured by RXESC are stored to the Rx Buffer resp. Rx FIFO
// element. The rest of the frame's data field is ignored.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_RXESC_F1DS_WIDTH 3U
#define MCAN_RXESC_F1DS_MASK  0x00000070U
#define MCAN_RXESC_F1DS_SHIFT 4U

// Field:   [2:0] F0DS
//
// Rx FIFO 0 Data Field Size
//   000  8 byte data field
//   001  12 byte data field
//   010  16 byte data field
//   011  20 byte data field
//   100  24 byte data field
//   101  32 byte data field
//   110  48 byte data field
//   111  64 byte data field
//
// Note: In case the data field size of an accepted CAN frame exceeds the data
// field size configured for the matching Rx Buffer or Rx FIFO, only the number
// of bytes as configured by RXESC are stored to the Rx Buffer resp. Rx FIFO
// element. The rest of the frame's data field is ignored.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_RXESC_F0DS_WIDTH 3U
#define MCAN_RXESC_F0DS_MASK  0x00000007U
#define MCAN_RXESC_F0DS_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TXBC
//
//*****************************************************************************
// Field:    [30] TFQM
//
// Tx FIFO/Queue Mode
//   0  Tx FIFO operation
//   1  Tx Queue operation
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_TXBC_TFQM       0x40000000U
#define MCAN_TXBC_TFQM_MASK  0x40000000U
#define MCAN_TXBC_TFQM_SHIFT 30U

// Field: [29:24] TFQS
//
// Transmit FIFO/Queue Size
//   0      No Tx FIFO/Queue
//   1-32  Number of Tx Buffers used for Tx FIFO/Queue
//   >32   Values greater than 32 are interpreted as 32
//
// Note: Be aware that the sum of TFQS and NDTB may be not greater than 32.
// There is no check
// for erroneous configurations. The Tx Buffers section in the Message RAM
// starts with the
// dedicated Tx Buffers.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_TXBC_TFQS_WIDTH 6U
#define MCAN_TXBC_TFQS_MASK  0x3F000000U
#define MCAN_TXBC_TFQS_SHIFT 24U

// Field: [21:16] NDTB
//
// Number of Dedicated Transmit Buffers
//   0      No Dedicated Tx Buffers
//   1-32  Number of Dedicated Tx Buffers
//   >32   Values greater than 32 are interpreted as 32
//
// Note: Be aware that the sum of TFQS and NDTB may be not greater than 32.
// There is no check
// for erroneous configurations. The Tx Buffers section in the Message RAM
// starts with the
// dedicated Tx Buffers.
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_TXBC_NDTB_WIDTH 6U
#define MCAN_TXBC_NDTB_MASK  0x003F0000U
#define MCAN_TXBC_NDTB_SHIFT 16U

// Field:  [15:2] TBSA
//
// Tx Buffers Start Address. Start address of Tx Buffers section in Message RAM
// (32-bit word address).
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_TXBC_TBSA_WIDTH 14U
#define MCAN_TXBC_TBSA_MASK  0x0000FFFCU
#define MCAN_TXBC_TBSA_SHIFT 2U

//*****************************************************************************
//
// Register: MCAN_TXFQS
//
//*****************************************************************************
// Field:    [21] TFQF
//
// Tx FIFO/Queue Full
//   0  Tx FIFO/Queue not full
//   1  Tx FIFO/Queue full
#define MCAN_TXFQS_TFQF       0x00200000U
#define MCAN_TXFQS_TFQF_MASK  0x00200000U
#define MCAN_TXFQS_TFQF_SHIFT 21U

// Field: [20:16] TFQPI
//
// Tx FIFO/Queue Put Index. Tx FIFO/Queue write index pointer, range 0 to 31.
//
// Note: In case of mixed configurations where dedicated Tx Buffers are
// combined with a Tx FIFO or a Tx Queue, the Put and Get Indices indicate the
// number of the Tx Buffer starting with the first dedicated Tx Buffers.
// Example: For a configuration of 12 dedicated Tx Buffers and a Tx FIFO of 20
// Buffers a Put Index of 15 points to the fourth buffer of the Tx FIFO.
#define MCAN_TXFQS_TFQPI_WIDTH 5U
#define MCAN_TXFQS_TFQPI_MASK  0x001F0000U
#define MCAN_TXFQS_TFQPI_SHIFT 16U

// Field:  [12:8] TFGI
//
// Tx FIFO Get Index. Tx FIFO read index pointer, range 0 to 31. Read as zero
// when Tx Queue operation is configured (TXBC.TFQM = '1').
//
// Note: In case of mixed configurations where dedicated Tx Buffers are
// combined with a Tx FIFO or a Tx Queue, the Put and Get Indices indicate the
// number of the Tx Buffer starting with the first dedicated Tx Buffers.
// Example: For a configuration of 12 dedicated Tx Buffers and a Tx FIFO of 20
// Buffers a Put Index of 15 points to the fourth buffer of the Tx FIFO.
#define MCAN_TXFQS_TFGI_WIDTH 5U
#define MCAN_TXFQS_TFGI_MASK  0x00001F00U
#define MCAN_TXFQS_TFGI_SHIFT 8U

// Field:   [5:0] TFFL
//
// Tx FIFO Free Level.  Number of consecutive free Tx FIFO elements starting
// from TFGI, range 0 to 32. Read as zero when Tx Queue operation is configured
// (TXBC.TFQM = '1').
#define MCAN_TXFQS_TFFL_WIDTH 6U
#define MCAN_TXFQS_TFFL_MASK  0x0000003FU
#define MCAN_TXFQS_TFFL_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TXESC
//
//*****************************************************************************
// Field:   [2:0] TBDS
//
// Tx Buffer Data Field Size
//   000  8 byte data field
//   001  12 byte data field
//   010  16 byte data field
//   011  20 byte data field
//   100  24 byte data field
//   101  32 byte data field
//   110  48 byte data field
//   111  64 byte data field
//
// Note: In case the data length code DLC of a Tx Buffer element is configured
// to a value higher than the Tx Buffer data field size TXESC.TBDS, the bytes
// not defined by the Tx Buffer are transmitted as "0xCC" (padding bytes).
//
// Qualified Write is possible only with CCCR.CCE='1' and CCCR.INIT='1'.
#define MCAN_TXESC_TBDS_WIDTH 3U
#define MCAN_TXESC_TBDS_MASK  0x00000007U
#define MCAN_TXESC_TBDS_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TXBRP
//
//*****************************************************************************
// Field:    [31] TRP31
//
// Transmission Request Pending 31. See description for bit 0.
#define MCAN_TXBRP_TRP31       0x80000000U
#define MCAN_TXBRP_TRP31_MASK  0x80000000U
#define MCAN_TXBRP_TRP31_SHIFT 31U

// Field:    [30] TRP30
//
// Transmission Request Pending 30. See description for bit 0.
#define MCAN_TXBRP_TRP30       0x40000000U
#define MCAN_TXBRP_TRP30_MASK  0x40000000U
#define MCAN_TXBRP_TRP30_SHIFT 30U

// Field:    [29] TRP29
//
// Transmission Request Pending 29. See description for bit 0.
#define MCAN_TXBRP_TRP29       0x20000000U
#define MCAN_TXBRP_TRP29_MASK  0x20000000U
#define MCAN_TXBRP_TRP29_SHIFT 29U

// Field:    [28] TRP28
//
// Transmission Request Pending 28. See description for bit 0.
#define MCAN_TXBRP_TRP28       0x10000000U
#define MCAN_TXBRP_TRP28_MASK  0x10000000U
#define MCAN_TXBRP_TRP28_SHIFT 28U

// Field:    [27] TRP27
//
// Transmission Request Pending 27. See description for bit 0.
#define MCAN_TXBRP_TRP27       0x08000000U
#define MCAN_TXBRP_TRP27_MASK  0x08000000U
#define MCAN_TXBRP_TRP27_SHIFT 27U

// Field:    [26] TRP26
//
// Transmission Request Pending 26. See description for bit 0.
#define MCAN_TXBRP_TRP26       0x04000000U
#define MCAN_TXBRP_TRP26_MASK  0x04000000U
#define MCAN_TXBRP_TRP26_SHIFT 26U

// Field:    [25] TRP25
//
// Transmission Request Pending 25. See description for bit 0.
#define MCAN_TXBRP_TRP25       0x02000000U
#define MCAN_TXBRP_TRP25_MASK  0x02000000U
#define MCAN_TXBRP_TRP25_SHIFT 25U

// Field:    [24] TRP24
//
// Transmission Request Pending 24. See description for bit 0.
#define MCAN_TXBRP_TRP24       0x01000000U
#define MCAN_TXBRP_TRP24_MASK  0x01000000U
#define MCAN_TXBRP_TRP24_SHIFT 24U

// Field:    [23] TRP23
//
// Transmission Request Pending 23. See description for bit 0.
#define MCAN_TXBRP_TRP23       0x00800000U
#define MCAN_TXBRP_TRP23_MASK  0x00800000U
#define MCAN_TXBRP_TRP23_SHIFT 23U

// Field:    [22] TRP22
//
// Transmission Request Pending 22. See description for bit 0.
#define MCAN_TXBRP_TRP22       0x00400000U
#define MCAN_TXBRP_TRP22_MASK  0x00400000U
#define MCAN_TXBRP_TRP22_SHIFT 22U

// Field:    [21] TRP21
//
// Transmission Request Pending 21. See description for bit 0.
#define MCAN_TXBRP_TRP21       0x00200000U
#define MCAN_TXBRP_TRP21_MASK  0x00200000U
#define MCAN_TXBRP_TRP21_SHIFT 21U

// Field:    [20] TRP20
//
// Transmission Request Pending 20. See description for bit 0.
#define MCAN_TXBRP_TRP20       0x00100000U
#define MCAN_TXBRP_TRP20_MASK  0x00100000U
#define MCAN_TXBRP_TRP20_SHIFT 20U

// Field:    [19] TRP19
//
// Transmission Request Pending 19. See description for bit 0.
#define MCAN_TXBRP_TRP19       0x00080000U
#define MCAN_TXBRP_TRP19_MASK  0x00080000U
#define MCAN_TXBRP_TRP19_SHIFT 19U

// Field:    [18] TRP18
//
// Transmission Request Pending 18. See description for bit 0.
#define MCAN_TXBRP_TRP18       0x00040000U
#define MCAN_TXBRP_TRP18_MASK  0x00040000U
#define MCAN_TXBRP_TRP18_SHIFT 18U

// Field:    [17] TRP17
//
// Transmission Request Pending 17. See description for bit 0.
#define MCAN_TXBRP_TRP17       0x00020000U
#define MCAN_TXBRP_TRP17_MASK  0x00020000U
#define MCAN_TXBRP_TRP17_SHIFT 17U

// Field:    [16] TRP16
//
// Transmission Request Pending 16. See description for bit 0.
#define MCAN_TXBRP_TRP16       0x00010000U
#define MCAN_TXBRP_TRP16_MASK  0x00010000U
#define MCAN_TXBRP_TRP16_SHIFT 16U

// Field:    [15] TRP15
//
// Transmission Request Pending 15. See description for bit 0.
#define MCAN_TXBRP_TRP15       0x00008000U
#define MCAN_TXBRP_TRP15_MASK  0x00008000U
#define MCAN_TXBRP_TRP15_SHIFT 15U

// Field:    [14] TRP14
//
// Transmission Request Pending 14. See description for bit 0.
#define MCAN_TXBRP_TRP14       0x00004000U
#define MCAN_TXBRP_TRP14_MASK  0x00004000U
#define MCAN_TXBRP_TRP14_SHIFT 14U

// Field:    [13] TRP13
//
// Transmission Request Pending 13. See description for bit 0.
#define MCAN_TXBRP_TRP13       0x00002000U
#define MCAN_TXBRP_TRP13_MASK  0x00002000U
#define MCAN_TXBRP_TRP13_SHIFT 13U

// Field:    [12] TRP12
//
// Transmission Request Pending 12. See description for bit 0.
#define MCAN_TXBRP_TRP12       0x00001000U
#define MCAN_TXBRP_TRP12_MASK  0x00001000U
#define MCAN_TXBRP_TRP12_SHIFT 12U

// Field:    [11] TRP11
//
// Transmission Request Pending 11. See description for bit 0.
#define MCAN_TXBRP_TRP11       0x00000800U
#define MCAN_TXBRP_TRP11_MASK  0x00000800U
#define MCAN_TXBRP_TRP11_SHIFT 11U

// Field:    [10] TRP10
//
// Transmission Request Pending 10. See description for bit 0.
#define MCAN_TXBRP_TRP10       0x00000400U
#define MCAN_TXBRP_TRP10_MASK  0x00000400U
#define MCAN_TXBRP_TRP10_SHIFT 10U

// Field:     [9] TRP9
//
// Transmission Request Pending 9. See description for bit 0.
#define MCAN_TXBRP_TRP9       0x00000200U
#define MCAN_TXBRP_TRP9_MASK  0x00000200U
#define MCAN_TXBRP_TRP9_SHIFT 9U

// Field:     [8] TRP8
//
// Transmission Request Pending 8. See description for bit 0.
#define MCAN_TXBRP_TRP8       0x00000100U
#define MCAN_TXBRP_TRP8_MASK  0x00000100U
#define MCAN_TXBRP_TRP8_SHIFT 8U

// Field:     [7] TRP7
//
// Transmission Request Pending 7. See description for bit 0.
#define MCAN_TXBRP_TRP7       0x00000080U
#define MCAN_TXBRP_TRP7_MASK  0x00000080U
#define MCAN_TXBRP_TRP7_SHIFT 7U

// Field:     [6] TRP6
//
// Transmission Request Pending 6. See description for bit 0.
#define MCAN_TXBRP_TRP6       0x00000040U
#define MCAN_TXBRP_TRP6_MASK  0x00000040U
#define MCAN_TXBRP_TRP6_SHIFT 6U

// Field:     [5] TRP5
//
// Transmission Request Pending 5. See description for bit 0.
#define MCAN_TXBRP_TRP5       0x00000020U
#define MCAN_TXBRP_TRP5_MASK  0x00000020U
#define MCAN_TXBRP_TRP5_SHIFT 5U

// Field:     [4] TRP4
//
// Transmission Request Pending 4. See description for bit 0.
#define MCAN_TXBRP_TRP4       0x00000010U
#define MCAN_TXBRP_TRP4_MASK  0x00000010U
#define MCAN_TXBRP_TRP4_SHIFT 4U

// Field:     [3] TRP3
//
// Transmission Request Pending 3. See description for bit 0.
#define MCAN_TXBRP_TRP3       0x00000008U
#define MCAN_TXBRP_TRP3_MASK  0x00000008U
#define MCAN_TXBRP_TRP3_SHIFT 3U

// Field:     [2] TRP2
//
// Transmission Request Pending 2. See description for bit 0.
#define MCAN_TXBRP_TRP2       0x00000004U
#define MCAN_TXBRP_TRP2_MASK  0x00000004U
#define MCAN_TXBRP_TRP2_SHIFT 2U

// Field:     [1] TRP1
//
// Transmission Request Pending 1. See description for bit 0.
#define MCAN_TXBRP_TRP1       0x00000002U
#define MCAN_TXBRP_TRP1_MASK  0x00000002U
#define MCAN_TXBRP_TRP1_SHIFT 1U

// Field:     [0] TRP0
//
// Transmission Request Pending 0.
//
// Each Tx Buffer has its own Transmission Request Pending bit. The bits are
// set via register TXBAR. The bits are reset after a requested transmission
// has completed or has been cancelled via register TXBCR.
//
// TXBRP bits are set only for those Tx Buffers configured via TXBC. After a
// TXBRP bit has been set, a Tx scan is started to check for the pending Tx
// request with the highest priority (Tx Buffer with lowest Message ID).
//
// A cancellation request resets the corresponding transmission request pending
// bit of register TXBRP. In case a transmission has already been started when
// a cancellation is requested, this is done at the end of the transmission,
// regardless whether the transmission was successful or not. The cancellation
// request bits are reset directly after the corresponding TXBRP bit has been
// reset.
//
// After a cancellation has been requested, a finished cancellation is
// signalled via TXBCF
// - after successful transmission together with the corresponding TXBTO bit
// - when the transmission has not yet been started at the point of
// cancellation
// - when the transmission has been aborted due to lost arbitration
// - when an error occurred during frame transmission
//
// In DAR mode all transmissions are automatically cancelled if they are not
// successful. The corresponding TXBCF bit is set for all unsuccessful
// transmissions.
//   0  No transmission request pending
//   1  Transmission request pending
//
// Note: TXBRP bits which are set while a Tx scan is in progress are not
// considered during this particular Tx scan. In case a cancellation is
// requested for such a Tx Buffer, this Add Request is cancelled immediately,
// the corresponding TXBRP bit is reset.
#define MCAN_TXBRP_TRP0       0x00000001U
#define MCAN_TXBRP_TRP0_MASK  0x00000001U
#define MCAN_TXBRP_TRP0_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TXBAR
//
//*****************************************************************************
// Field:    [31] AR31
//
// Add Request 31. See description for bit 0.
#define MCAN_TXBAR_AR31       0x80000000U
#define MCAN_TXBAR_AR31_MASK  0x80000000U
#define MCAN_TXBAR_AR31_SHIFT 31U

// Field:    [30] AR30
//
// Add Request 30. See description for bit 0.
#define MCAN_TXBAR_AR30       0x40000000U
#define MCAN_TXBAR_AR30_MASK  0x40000000U
#define MCAN_TXBAR_AR30_SHIFT 30U

// Field:    [29] AR29
//
// Add Request 29. See description for bit 0.
#define MCAN_TXBAR_AR29       0x20000000U
#define MCAN_TXBAR_AR29_MASK  0x20000000U
#define MCAN_TXBAR_AR29_SHIFT 29U

// Field:    [28] AR28
//
// Add Request 28. See description for bit 0.
#define MCAN_TXBAR_AR28       0x10000000U
#define MCAN_TXBAR_AR28_MASK  0x10000000U
#define MCAN_TXBAR_AR28_SHIFT 28U

// Field:    [27] AR27
//
// Add Request 27. See description for bit 0.
#define MCAN_TXBAR_AR27       0x08000000U
#define MCAN_TXBAR_AR27_MASK  0x08000000U
#define MCAN_TXBAR_AR27_SHIFT 27U

// Field:    [26] AR26
//
// Add Request 26. See description for bit 0.
#define MCAN_TXBAR_AR26       0x04000000U
#define MCAN_TXBAR_AR26_MASK  0x04000000U
#define MCAN_TXBAR_AR26_SHIFT 26U

// Field:    [25] AR25
//
// Add Request 25. See description for bit 0.
#define MCAN_TXBAR_AR25       0x02000000U
#define MCAN_TXBAR_AR25_MASK  0x02000000U
#define MCAN_TXBAR_AR25_SHIFT 25U

// Field:    [24] AR24
//
// Add Request 24. See description for bit 0.
#define MCAN_TXBAR_AR24       0x01000000U
#define MCAN_TXBAR_AR24_MASK  0x01000000U
#define MCAN_TXBAR_AR24_SHIFT 24U

// Field:    [23] AR23
//
// Add Request 23. See description for bit 0.
#define MCAN_TXBAR_AR23       0x00800000U
#define MCAN_TXBAR_AR23_MASK  0x00800000U
#define MCAN_TXBAR_AR23_SHIFT 23U

// Field:    [22] AR22
//
// Add Request 22. See description for bit 0.
#define MCAN_TXBAR_AR22       0x00400000U
#define MCAN_TXBAR_AR22_MASK  0x00400000U
#define MCAN_TXBAR_AR22_SHIFT 22U

// Field:    [21] AR21
//
// Add Request 21. See description for bit 0.
#define MCAN_TXBAR_AR21       0x00200000U
#define MCAN_TXBAR_AR21_MASK  0x00200000U
#define MCAN_TXBAR_AR21_SHIFT 21U

// Field:    [20] AR20
//
// Add Request 20. See description for bit 0.
#define MCAN_TXBAR_AR20       0x00100000U
#define MCAN_TXBAR_AR20_MASK  0x00100000U
#define MCAN_TXBAR_AR20_SHIFT 20U

// Field:    [19] AR19
//
// Add Request 19. See description for bit 0.
#define MCAN_TXBAR_AR19       0x00080000U
#define MCAN_TXBAR_AR19_MASK  0x00080000U
#define MCAN_TXBAR_AR19_SHIFT 19U

// Field:    [18] AR18
//
// Add Request 18. See description for bit 0.
#define MCAN_TXBAR_AR18       0x00040000U
#define MCAN_TXBAR_AR18_MASK  0x00040000U
#define MCAN_TXBAR_AR18_SHIFT 18U

// Field:    [17] AR17
//
// Add Request 17. See description for bit 0.
#define MCAN_TXBAR_AR17       0x00020000U
#define MCAN_TXBAR_AR17_MASK  0x00020000U
#define MCAN_TXBAR_AR17_SHIFT 17U

// Field:    [16] AR16
//
// Add Request 16. See description for bit 0.
#define MCAN_TXBAR_AR16       0x00010000U
#define MCAN_TXBAR_AR16_MASK  0x00010000U
#define MCAN_TXBAR_AR16_SHIFT 16U

// Field:    [15] AR15
//
// Add Request 15. See description for bit 0.
#define MCAN_TXBAR_AR15       0x00008000U
#define MCAN_TXBAR_AR15_MASK  0x00008000U
#define MCAN_TXBAR_AR15_SHIFT 15U

// Field:    [14] AR14
//
// Add Request 14. See description for bit 0.
#define MCAN_TXBAR_AR14       0x00004000U
#define MCAN_TXBAR_AR14_MASK  0x00004000U
#define MCAN_TXBAR_AR14_SHIFT 14U

// Field:    [13] AR13
//
// Add Request 13. See description for bit 0.
#define MCAN_TXBAR_AR13       0x00002000U
#define MCAN_TXBAR_AR13_MASK  0x00002000U
#define MCAN_TXBAR_AR13_SHIFT 13U

// Field:    [12] AR12
//
// Add Request 12. See description for bit 0.
#define MCAN_TXBAR_AR12       0x00001000U
#define MCAN_TXBAR_AR12_MASK  0x00001000U
#define MCAN_TXBAR_AR12_SHIFT 12U

// Field:    [11] AR11
//
// Add Request 11. See description for bit 0.
#define MCAN_TXBAR_AR11       0x00000800U
#define MCAN_TXBAR_AR11_MASK  0x00000800U
#define MCAN_TXBAR_AR11_SHIFT 11U

// Field:    [10] AR10
//
// Add Request 10. See description for bit 0.
#define MCAN_TXBAR_AR10       0x00000400U
#define MCAN_TXBAR_AR10_MASK  0x00000400U
#define MCAN_TXBAR_AR10_SHIFT 10U

// Field:     [9] AR9
//
// Add Request 9. See description for bit 0.
#define MCAN_TXBAR_AR9       0x00000200U
#define MCAN_TXBAR_AR9_MASK  0x00000200U
#define MCAN_TXBAR_AR9_SHIFT 9U

// Field:     [8] AR8
//
// Add Request 8. See description for bit 0.
#define MCAN_TXBAR_AR8       0x00000100U
#define MCAN_TXBAR_AR8_MASK  0x00000100U
#define MCAN_TXBAR_AR8_SHIFT 8U

// Field:     [7] AR7
//
// Add Request 7. See description for bit 0.
#define MCAN_TXBAR_AR7       0x00000080U
#define MCAN_TXBAR_AR7_MASK  0x00000080U
#define MCAN_TXBAR_AR7_SHIFT 7U

// Field:     [6] AR6
//
// Add Request 6. See description for bit 0.
#define MCAN_TXBAR_AR6       0x00000040U
#define MCAN_TXBAR_AR6_MASK  0x00000040U
#define MCAN_TXBAR_AR6_SHIFT 6U

// Field:     [5] AR5
//
// Add Request 5. See description for bit 0.
#define MCAN_TXBAR_AR5       0x00000020U
#define MCAN_TXBAR_AR5_MASK  0x00000020U
#define MCAN_TXBAR_AR5_SHIFT 5U

// Field:     [4] AR4
//
// Add Request 4. See description for bit 0.
#define MCAN_TXBAR_AR4       0x00000010U
#define MCAN_TXBAR_AR4_MASK  0x00000010U
#define MCAN_TXBAR_AR4_SHIFT 4U

// Field:     [3] AR3
//
// Add Request 3. See description for bit 0.
#define MCAN_TXBAR_AR3       0x00000008U
#define MCAN_TXBAR_AR3_MASK  0x00000008U
#define MCAN_TXBAR_AR3_SHIFT 3U

// Field:     [2] AR2
//
// Add Request 2. See description for bit 0.
#define MCAN_TXBAR_AR2       0x00000004U
#define MCAN_TXBAR_AR2_MASK  0x00000004U
#define MCAN_TXBAR_AR2_SHIFT 2U

// Field:     [1] AR1
//
// Add Request 1. See description for bit 0.
#define MCAN_TXBAR_AR1       0x00000002U
#define MCAN_TXBAR_AR1_MASK  0x00000002U
#define MCAN_TXBAR_AR1_SHIFT 1U

// Field:     [0] AR0
//
// Add Request 0.
//
// Each Tx Buffer has its own Add Request bit. Writing a '1' will set the
// corresponding Add Request bit; writing a '0' has no impact. This enables the
// Host to set transmission requests for multiple Tx Buffers with one write to
// TXBAR. TXBAR bits are set only for those Tx Buffers configured via TXBC.
// When no Tx scan is running, the bits are reset immediately, else the bits
// remain set until the Tx scan process has completed.
//   0  No transmission request added
//   1  Transmission requested added
//
// Note: If an add request is applied for a Tx Buffer with pending transmission
// request (corresponding TXBRP bit already set), this add request is ignored.
//
// Qualified Write is possible only with CCCR.CCE='0'
#define MCAN_TXBAR_AR0       0x00000001U
#define MCAN_TXBAR_AR0_MASK  0x00000001U
#define MCAN_TXBAR_AR0_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TXBCR
//
//*****************************************************************************
// Field:    [31] CR31
//
// Cancellation Request 31. See description for bit 0.
#define MCAN_TXBCR_CR31       0x80000000U
#define MCAN_TXBCR_CR31_MASK  0x80000000U
#define MCAN_TXBCR_CR31_SHIFT 31U

// Field:    [30] CR30
//
// Cancellation Request 30. See description for bit 0.
#define MCAN_TXBCR_CR30       0x40000000U
#define MCAN_TXBCR_CR30_MASK  0x40000000U
#define MCAN_TXBCR_CR30_SHIFT 30U

// Field:    [29] CR29
//
// Cancellation Request 29. See description for bit 0.
#define MCAN_TXBCR_CR29       0x20000000U
#define MCAN_TXBCR_CR29_MASK  0x20000000U
#define MCAN_TXBCR_CR29_SHIFT 29U

// Field:    [28] CR28
//
// Cancellation Request 28. See description for bit 0.
#define MCAN_TXBCR_CR28       0x10000000U
#define MCAN_TXBCR_CR28_MASK  0x10000000U
#define MCAN_TXBCR_CR28_SHIFT 28U

// Field:    [27] CR27
//
// Cancellation Request 27. See description for bit 0.
#define MCAN_TXBCR_CR27       0x08000000U
#define MCAN_TXBCR_CR27_MASK  0x08000000U
#define MCAN_TXBCR_CR27_SHIFT 27U

// Field:    [26] CR26
//
// Cancellation Request 26. See description for bit 0.
#define MCAN_TXBCR_CR26       0x04000000U
#define MCAN_TXBCR_CR26_MASK  0x04000000U
#define MCAN_TXBCR_CR26_SHIFT 26U

// Field:    [25] CR25
//
// Cancellation Request 25. See description for bit 0.
#define MCAN_TXBCR_CR25       0x02000000U
#define MCAN_TXBCR_CR25_MASK  0x02000000U
#define MCAN_TXBCR_CR25_SHIFT 25U

// Field:    [24] CR24
//
// Cancellation Request 24. See description for bit 0.
#define MCAN_TXBCR_CR24       0x01000000U
#define MCAN_TXBCR_CR24_MASK  0x01000000U
#define MCAN_TXBCR_CR24_SHIFT 24U

// Field:    [23] CR23
//
// Cancellation Request 23. See description for bit 0.
#define MCAN_TXBCR_CR23       0x00800000U
#define MCAN_TXBCR_CR23_MASK  0x00800000U
#define MCAN_TXBCR_CR23_SHIFT 23U

// Field:    [22] CR22
//
// Cancellation Request 22. See description for bit 0.
#define MCAN_TXBCR_CR22       0x00400000U
#define MCAN_TXBCR_CR22_MASK  0x00400000U
#define MCAN_TXBCR_CR22_SHIFT 22U

// Field:    [21] CR21
//
// Cancellation Request 21. See description for bit 0.
#define MCAN_TXBCR_CR21       0x00200000U
#define MCAN_TXBCR_CR21_MASK  0x00200000U
#define MCAN_TXBCR_CR21_SHIFT 21U

// Field:    [20] CR20
//
// Cancellation Request 20. See description for bit 0.
#define MCAN_TXBCR_CR20       0x00100000U
#define MCAN_TXBCR_CR20_MASK  0x00100000U
#define MCAN_TXBCR_CR20_SHIFT 20U

// Field:    [19] CR19
//
// Cancellation Request 19. See description for bit 0.
#define MCAN_TXBCR_CR19       0x00080000U
#define MCAN_TXBCR_CR19_MASK  0x00080000U
#define MCAN_TXBCR_CR19_SHIFT 19U

// Field:    [18] CR18
//
// Cancellation Request 18. See description for bit 0.
#define MCAN_TXBCR_CR18       0x00040000U
#define MCAN_TXBCR_CR18_MASK  0x00040000U
#define MCAN_TXBCR_CR18_SHIFT 18U

// Field:    [17] CR17
//
// Cancellation Request 17. See description for bit 0.
#define MCAN_TXBCR_CR17       0x00020000U
#define MCAN_TXBCR_CR17_MASK  0x00020000U
#define MCAN_TXBCR_CR17_SHIFT 17U

// Field:    [16] CR16
//
// Cancellation Request 16. See description for bit 0.
#define MCAN_TXBCR_CR16       0x00010000U
#define MCAN_TXBCR_CR16_MASK  0x00010000U
#define MCAN_TXBCR_CR16_SHIFT 16U

// Field:    [15] CR15
//
// Cancellation Request 15. See description for bit 0.
#define MCAN_TXBCR_CR15       0x00008000U
#define MCAN_TXBCR_CR15_MASK  0x00008000U
#define MCAN_TXBCR_CR15_SHIFT 15U

// Field:    [14] CR14
//
// Cancellation Request 14. See description for bit 0.
#define MCAN_TXBCR_CR14       0x00004000U
#define MCAN_TXBCR_CR14_MASK  0x00004000U
#define MCAN_TXBCR_CR14_SHIFT 14U

// Field:    [13] CR13
//
// Cancellation Request 13. See description for bit 0.
#define MCAN_TXBCR_CR13       0x00002000U
#define MCAN_TXBCR_CR13_MASK  0x00002000U
#define MCAN_TXBCR_CR13_SHIFT 13U

// Field:    [12] CR12
//
// Cancellation Request 12. See description for bit 0.
#define MCAN_TXBCR_CR12       0x00001000U
#define MCAN_TXBCR_CR12_MASK  0x00001000U
#define MCAN_TXBCR_CR12_SHIFT 12U

// Field:    [11] CR11
//
// Cancellation Request 11. See description for bit 0.
#define MCAN_TXBCR_CR11       0x00000800U
#define MCAN_TXBCR_CR11_MASK  0x00000800U
#define MCAN_TXBCR_CR11_SHIFT 11U

// Field:    [10] CR10
//
// Cancellation Request 10. See description for bit 0.
#define MCAN_TXBCR_CR10       0x00000400U
#define MCAN_TXBCR_CR10_MASK  0x00000400U
#define MCAN_TXBCR_CR10_SHIFT 10U

// Field:     [9] CR9
//
// Cancellation Request 9. See description for bit 0.
#define MCAN_TXBCR_CR9       0x00000200U
#define MCAN_TXBCR_CR9_MASK  0x00000200U
#define MCAN_TXBCR_CR9_SHIFT 9U

// Field:     [8] CR8
//
// Cancellation Request 8. See description for bit 0.
#define MCAN_TXBCR_CR8       0x00000100U
#define MCAN_TXBCR_CR8_MASK  0x00000100U
#define MCAN_TXBCR_CR8_SHIFT 8U

// Field:     [7] CR7
//
// Cancellation Request 7. See description for bit 0.
#define MCAN_TXBCR_CR7       0x00000080U
#define MCAN_TXBCR_CR7_MASK  0x00000080U
#define MCAN_TXBCR_CR7_SHIFT 7U

// Field:     [6] CR6
//
// Cancellation Request 6. See description for bit 0.
#define MCAN_TXBCR_CR6       0x00000040U
#define MCAN_TXBCR_CR6_MASK  0x00000040U
#define MCAN_TXBCR_CR6_SHIFT 6U

// Field:     [5] CR5
//
// Cancellation Request 5. See description for bit 0.
#define MCAN_TXBCR_CR5       0x00000020U
#define MCAN_TXBCR_CR5_MASK  0x00000020U
#define MCAN_TXBCR_CR5_SHIFT 5U

// Field:     [4] CR4
//
// Cancellation Request 4. See description for bit 0.
#define MCAN_TXBCR_CR4       0x00000010U
#define MCAN_TXBCR_CR4_MASK  0x00000010U
#define MCAN_TXBCR_CR4_SHIFT 4U

// Field:     [3] CR3
//
// Cancellation Request 3. See description for bit 0.
#define MCAN_TXBCR_CR3       0x00000008U
#define MCAN_TXBCR_CR3_MASK  0x00000008U
#define MCAN_TXBCR_CR3_SHIFT 3U

// Field:     [2] CR2
//
// Cancellation Request 2. See description for bit 0.
#define MCAN_TXBCR_CR2       0x00000004U
#define MCAN_TXBCR_CR2_MASK  0x00000004U
#define MCAN_TXBCR_CR2_SHIFT 2U

// Field:     [1] CR1
//
// Cancellation Request 1. See description for bit 0.
#define MCAN_TXBCR_CR1       0x00000002U
#define MCAN_TXBCR_CR1_MASK  0x00000002U
#define MCAN_TXBCR_CR1_SHIFT 1U

// Field:     [0] CR0
//
// Cancellation Request 0.
//
// Each Tx Buffer has its own Cancellation Request bit. Writing a '1' will set
// the corresponding Cancellation Request bit; writing a '0' has no impact.
// This enables the Host to set cancellation requests for multiple Tx Buffers
// with one write to TXBCR. TXBCR bits are set only for those Tx Buffers
// configured via TXBC. The bits remain set until the corresponding bit of
// TXBRP is reset.
//   0  No cancellation pending
//   1  Cancellation pending
//
// Qualified Write is possible only with CCCR.CCE='0'
#define MCAN_TXBCR_CR0       0x00000001U
#define MCAN_TXBCR_CR0_MASK  0x00000001U
#define MCAN_TXBCR_CR0_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TXBTO
//
//*****************************************************************************
// Field:    [31] TO31
//
// Transmission Occurred 31. See description for bit 0.
#define MCAN_TXBTO_TO31       0x80000000U
#define MCAN_TXBTO_TO31_MASK  0x80000000U
#define MCAN_TXBTO_TO31_SHIFT 31U

// Field:    [30] TO30
//
// Transmission Occurred 30. See description for bit 0.
#define MCAN_TXBTO_TO30       0x40000000U
#define MCAN_TXBTO_TO30_MASK  0x40000000U
#define MCAN_TXBTO_TO30_SHIFT 30U

// Field:    [29] TO29
//
// Transmission Occurred 29. See description for bit 0.
#define MCAN_TXBTO_TO29       0x20000000U
#define MCAN_TXBTO_TO29_MASK  0x20000000U
#define MCAN_TXBTO_TO29_SHIFT 29U

// Field:    [28] TO28
//
// Transmission Occurred 28. See description for bit 0.
#define MCAN_TXBTO_TO28       0x10000000U
#define MCAN_TXBTO_TO28_MASK  0x10000000U
#define MCAN_TXBTO_TO28_SHIFT 28U

// Field:    [27] TO27
//
// Transmission Occurred 27. See description for bit 0.
#define MCAN_TXBTO_TO27       0x08000000U
#define MCAN_TXBTO_TO27_MASK  0x08000000U
#define MCAN_TXBTO_TO27_SHIFT 27U

// Field:    [26] TO26
//
// Transmission Occurred 26. See description for bit 0.
#define MCAN_TXBTO_TO26       0x04000000U
#define MCAN_TXBTO_TO26_MASK  0x04000000U
#define MCAN_TXBTO_TO26_SHIFT 26U

// Field:    [25] TO25
//
// Transmission Occurred 25. See description for bit 0.
#define MCAN_TXBTO_TO25       0x02000000U
#define MCAN_TXBTO_TO25_MASK  0x02000000U
#define MCAN_TXBTO_TO25_SHIFT 25U

// Field:    [24] TO24
//
// Transmission Occurred 24. See description for bit 0.
#define MCAN_TXBTO_TO24       0x01000000U
#define MCAN_TXBTO_TO24_MASK  0x01000000U
#define MCAN_TXBTO_TO24_SHIFT 24U

// Field:    [23] TO23
//
// Transmission Occurred 23. See description for bit 0.
#define MCAN_TXBTO_TO23       0x00800000U
#define MCAN_TXBTO_TO23_MASK  0x00800000U
#define MCAN_TXBTO_TO23_SHIFT 23U

// Field:    [22] TO22
//
// Transmission Occurred 22. See description for bit 0.
#define MCAN_TXBTO_TO22       0x00400000U
#define MCAN_TXBTO_TO22_MASK  0x00400000U
#define MCAN_TXBTO_TO22_SHIFT 22U

// Field:    [21] TO21
//
// Transmission Occurred 21. See description for bit 0.
#define MCAN_TXBTO_TO21       0x00200000U
#define MCAN_TXBTO_TO21_MASK  0x00200000U
#define MCAN_TXBTO_TO21_SHIFT 21U

// Field:    [20] TO20
//
// Transmission Occurred 20. See description for bit 0.
#define MCAN_TXBTO_TO20       0x00100000U
#define MCAN_TXBTO_TO20_MASK  0x00100000U
#define MCAN_TXBTO_TO20_SHIFT 20U

// Field:    [19] TO19
//
// Transmission Occurred 19. See description for bit 0.
#define MCAN_TXBTO_TO19       0x00080000U
#define MCAN_TXBTO_TO19_MASK  0x00080000U
#define MCAN_TXBTO_TO19_SHIFT 19U

// Field:    [18] TO18
//
// Transmission Occurred 18. See description for bit 0.
#define MCAN_TXBTO_TO18       0x00040000U
#define MCAN_TXBTO_TO18_MASK  0x00040000U
#define MCAN_TXBTO_TO18_SHIFT 18U

// Field:    [17] TO17
//
// Transmission Occurred 17. See description for bit 0.
#define MCAN_TXBTO_TO17       0x00020000U
#define MCAN_TXBTO_TO17_MASK  0x00020000U
#define MCAN_TXBTO_TO17_SHIFT 17U

// Field:    [16] TO16
//
// Transmission Occurred 16. See description for bit 0.
#define MCAN_TXBTO_TO16       0x00010000U
#define MCAN_TXBTO_TO16_MASK  0x00010000U
#define MCAN_TXBTO_TO16_SHIFT 16U

// Field:    [15] TO15
//
// Transmission Occurred 15. See description for bit 0.
#define MCAN_TXBTO_TO15       0x00008000U
#define MCAN_TXBTO_TO15_MASK  0x00008000U
#define MCAN_TXBTO_TO15_SHIFT 15U

// Field:    [14] TO14
//
// Transmission Occurred 14. See description for bit 0.
#define MCAN_TXBTO_TO14       0x00004000U
#define MCAN_TXBTO_TO14_MASK  0x00004000U
#define MCAN_TXBTO_TO14_SHIFT 14U

// Field:    [13] TO13
//
// Transmission Occurred 13. See description for bit 0.
#define MCAN_TXBTO_TO13       0x00002000U
#define MCAN_TXBTO_TO13_MASK  0x00002000U
#define MCAN_TXBTO_TO13_SHIFT 13U

// Field:    [12] TO12
//
// Transmission Occurred 12. See description for bit 0.
#define MCAN_TXBTO_TO12       0x00001000U
#define MCAN_TXBTO_TO12_MASK  0x00001000U
#define MCAN_TXBTO_TO12_SHIFT 12U

// Field:    [11] TO11
//
// Transmission Occurred 11. See description for bit 0.
#define MCAN_TXBTO_TO11       0x00000800U
#define MCAN_TXBTO_TO11_MASK  0x00000800U
#define MCAN_TXBTO_TO11_SHIFT 11U

// Field:    [10] TO10
//
// Transmission Occurred 10. See description for bit 0.
#define MCAN_TXBTO_TO10       0x00000400U
#define MCAN_TXBTO_TO10_MASK  0x00000400U
#define MCAN_TXBTO_TO10_SHIFT 10U

// Field:     [9] TO9
//
// Transmission Occurred 9. See description for bit 0.
#define MCAN_TXBTO_TO9       0x00000200U
#define MCAN_TXBTO_TO9_MASK  0x00000200U
#define MCAN_TXBTO_TO9_SHIFT 9U

// Field:     [8] TO8
//
// Transmission Occurred 8. See description for bit 0.
#define MCAN_TXBTO_TO8       0x00000100U
#define MCAN_TXBTO_TO8_MASK  0x00000100U
#define MCAN_TXBTO_TO8_SHIFT 8U

// Field:     [7] TO7
//
// Transmission Occurred 7. See description for bit 0.
#define MCAN_TXBTO_TO7       0x00000080U
#define MCAN_TXBTO_TO7_MASK  0x00000080U
#define MCAN_TXBTO_TO7_SHIFT 7U

// Field:     [6] TO6
//
// Transmission Occurred 6. See description for bit 0.
#define MCAN_TXBTO_TO6       0x00000040U
#define MCAN_TXBTO_TO6_MASK  0x00000040U
#define MCAN_TXBTO_TO6_SHIFT 6U

// Field:     [5] TO5
//
// Transmission Occurred 5. See description for bit 0.
#define MCAN_TXBTO_TO5       0x00000020U
#define MCAN_TXBTO_TO5_MASK  0x00000020U
#define MCAN_TXBTO_TO5_SHIFT 5U

// Field:     [4] TO4
//
// Transmission Occurred 4. See description for bit 0.
#define MCAN_TXBTO_TO4       0x00000010U
#define MCAN_TXBTO_TO4_MASK  0x00000010U
#define MCAN_TXBTO_TO4_SHIFT 4U

// Field:     [3] TO3
//
// Transmission Occurred 3. See description for bit 0.
#define MCAN_TXBTO_TO3       0x00000008U
#define MCAN_TXBTO_TO3_MASK  0x00000008U
#define MCAN_TXBTO_TO3_SHIFT 3U

// Field:     [2] TO2
//
// Transmission Occurred 2. See description for bit 0.
#define MCAN_TXBTO_TO2       0x00000004U
#define MCAN_TXBTO_TO2_MASK  0x00000004U
#define MCAN_TXBTO_TO2_SHIFT 2U

// Field:     [1] TO1
//
// Transmission Occurred 1. See description for bit 0.
#define MCAN_TXBTO_TO1       0x00000002U
#define MCAN_TXBTO_TO1_MASK  0x00000002U
#define MCAN_TXBTO_TO1_SHIFT 1U

// Field:     [0] TO0
//
// Transmission Occurred 0.
//
// Each Tx Buffer has its own Transmission Occurred bit. The bits are set when
// the corresponding TXBRP bit is cleared after a successful transmission. The
// bits are reset when a new transmission is requested by writing a '1' to the
// corresponding bit of register TXBAR.
//   0  No transmission occurred
//   1  Transmission occurred
#define MCAN_TXBTO_TO0       0x00000001U
#define MCAN_TXBTO_TO0_MASK  0x00000001U
#define MCAN_TXBTO_TO0_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TXBCF
//
//*****************************************************************************
// Field:    [31] CF31
//
// Cancellation Finished 31. See description for bit 0.
#define MCAN_TXBCF_CF31       0x80000000U
#define MCAN_TXBCF_CF31_MASK  0x80000000U
#define MCAN_TXBCF_CF31_SHIFT 31U

// Field:    [30] CF30
//
// Cancellation Finished 30. See description for bit 0.
#define MCAN_TXBCF_CF30       0x40000000U
#define MCAN_TXBCF_CF30_MASK  0x40000000U
#define MCAN_TXBCF_CF30_SHIFT 30U

// Field:    [29] CF29
//
// Cancellation Finished 29. See description for bit 0.
#define MCAN_TXBCF_CF29       0x20000000U
#define MCAN_TXBCF_CF29_MASK  0x20000000U
#define MCAN_TXBCF_CF29_SHIFT 29U

// Field:    [28] CF28
//
// Cancellation Finished 28. See description for bit 0.
#define MCAN_TXBCF_CF28       0x10000000U
#define MCAN_TXBCF_CF28_MASK  0x10000000U
#define MCAN_TXBCF_CF28_SHIFT 28U

// Field:    [27] CF27
//
// Cancellation Finished 27. See description for bit 0.
#define MCAN_TXBCF_CF27       0x08000000U
#define MCAN_TXBCF_CF27_MASK  0x08000000U
#define MCAN_TXBCF_CF27_SHIFT 27U

// Field:    [26] CF26
//
// Cancellation Finished 26. See description for bit 0.
#define MCAN_TXBCF_CF26       0x04000000U
#define MCAN_TXBCF_CF26_MASK  0x04000000U
#define MCAN_TXBCF_CF26_SHIFT 26U

// Field:    [25] CF25
//
// Cancellation Finished 25. See description for bit 0.
#define MCAN_TXBCF_CF25       0x02000000U
#define MCAN_TXBCF_CF25_MASK  0x02000000U
#define MCAN_TXBCF_CF25_SHIFT 25U

// Field:    [24] CF24
//
// Cancellation Finished 24. See description for bit 0.
#define MCAN_TXBCF_CF24       0x01000000U
#define MCAN_TXBCF_CF24_MASK  0x01000000U
#define MCAN_TXBCF_CF24_SHIFT 24U

// Field:    [23] CF23
//
// Cancellation Finished 23. See description for bit 0.
#define MCAN_TXBCF_CF23       0x00800000U
#define MCAN_TXBCF_CF23_MASK  0x00800000U
#define MCAN_TXBCF_CF23_SHIFT 23U

// Field:    [22] CF22
//
// Cancellation Finished 22. See description for bit 0.
#define MCAN_TXBCF_CF22       0x00400000U
#define MCAN_TXBCF_CF22_MASK  0x00400000U
#define MCAN_TXBCF_CF22_SHIFT 22U

// Field:    [21] CF21
//
// Cancellation Finished 21. See description for bit 0.
#define MCAN_TXBCF_CF21       0x00200000U
#define MCAN_TXBCF_CF21_MASK  0x00200000U
#define MCAN_TXBCF_CF21_SHIFT 21U

// Field:    [20] CF20
//
// Cancellation Finished 20. See description for bit 0.
#define MCAN_TXBCF_CF20       0x00100000U
#define MCAN_TXBCF_CF20_MASK  0x00100000U
#define MCAN_TXBCF_CF20_SHIFT 20U

// Field:    [19] CF19
//
// Cancellation Finished 19. See description for bit 0.
#define MCAN_TXBCF_CF19       0x00080000U
#define MCAN_TXBCF_CF19_MASK  0x00080000U
#define MCAN_TXBCF_CF19_SHIFT 19U

// Field:    [18] CF18
//
// Cancellation Finished 18. See description for bit 0.
#define MCAN_TXBCF_CF18       0x00040000U
#define MCAN_TXBCF_CF18_MASK  0x00040000U
#define MCAN_TXBCF_CF18_SHIFT 18U

// Field:    [17] CF17
//
// Cancellation Finished 17. See description for bit 0.
#define MCAN_TXBCF_CF17       0x00020000U
#define MCAN_TXBCF_CF17_MASK  0x00020000U
#define MCAN_TXBCF_CF17_SHIFT 17U

// Field:    [16] CF16
//
// Cancellation Finished 16. See description for bit 0.
#define MCAN_TXBCF_CF16       0x00010000U
#define MCAN_TXBCF_CF16_MASK  0x00010000U
#define MCAN_TXBCF_CF16_SHIFT 16U

// Field:    [15] CF15
//
// Cancellation Finished 15. See description for bit 0.
#define MCAN_TXBCF_CF15       0x00008000U
#define MCAN_TXBCF_CF15_MASK  0x00008000U
#define MCAN_TXBCF_CF15_SHIFT 15U

// Field:    [14] CF14
//
// Cancellation Finished 14. See description for bit 0.
#define MCAN_TXBCF_CF14       0x00004000U
#define MCAN_TXBCF_CF14_MASK  0x00004000U
#define MCAN_TXBCF_CF14_SHIFT 14U

// Field:    [13] CF13
//
// Cancellation Finished 13. See description for bit 0.
#define MCAN_TXBCF_CF13       0x00002000U
#define MCAN_TXBCF_CF13_MASK  0x00002000U
#define MCAN_TXBCF_CF13_SHIFT 13U

// Field:    [12] CF12
//
// Cancellation Finished 12. See description for bit 0.
#define MCAN_TXBCF_CF12       0x00001000U
#define MCAN_TXBCF_CF12_MASK  0x00001000U
#define MCAN_TXBCF_CF12_SHIFT 12U

// Field:    [11] CF11
//
// Cancellation Finished 11. See description for bit 0.
#define MCAN_TXBCF_CF11       0x00000800U
#define MCAN_TXBCF_CF11_MASK  0x00000800U
#define MCAN_TXBCF_CF11_SHIFT 11U

// Field:    [10] CF10
//
// Cancellation Finished 10. See description for bit 0.
#define MCAN_TXBCF_CF10       0x00000400U
#define MCAN_TXBCF_CF10_MASK  0x00000400U
#define MCAN_TXBCF_CF10_SHIFT 10U

// Field:     [9] CF9
//
// Cancellation Finished 9. See description for bit 0.
#define MCAN_TXBCF_CF9       0x00000200U
#define MCAN_TXBCF_CF9_MASK  0x00000200U
#define MCAN_TXBCF_CF9_SHIFT 9U

// Field:     [8] CF8
//
// Cancellation Finished 8. See description for bit 0.
#define MCAN_TXBCF_CF8       0x00000100U
#define MCAN_TXBCF_CF8_MASK  0x00000100U
#define MCAN_TXBCF_CF8_SHIFT 8U

// Field:     [7] CF7
//
// Cancellation Finished 7. See description for bit 0.
#define MCAN_TXBCF_CF7       0x00000080U
#define MCAN_TXBCF_CF7_MASK  0x00000080U
#define MCAN_TXBCF_CF7_SHIFT 7U

// Field:     [6] CF6
//
// Cancellation Finished 6. See description for bit 0.
#define MCAN_TXBCF_CF6       0x00000040U
#define MCAN_TXBCF_CF6_MASK  0x00000040U
#define MCAN_TXBCF_CF6_SHIFT 6U

// Field:     [5] CF5
//
// Cancellation Finished 5. See description for bit 0.
#define MCAN_TXBCF_CF5       0x00000020U
#define MCAN_TXBCF_CF5_MASK  0x00000020U
#define MCAN_TXBCF_CF5_SHIFT 5U

// Field:     [4] CF4
//
// Cancellation Finished 4. See description for bit 0.
#define MCAN_TXBCF_CF4       0x00000010U
#define MCAN_TXBCF_CF4_MASK  0x00000010U
#define MCAN_TXBCF_CF4_SHIFT 4U

// Field:     [3] CF3
//
// Cancellation Finished 3. See description for bit 0.
#define MCAN_TXBCF_CF3       0x00000008U
#define MCAN_TXBCF_CF3_MASK  0x00000008U
#define MCAN_TXBCF_CF3_SHIFT 3U

// Field:     [2] CF2
//
// Cancellation Finished 2. See description for bit 0.
#define MCAN_TXBCF_CF2       0x00000004U
#define MCAN_TXBCF_CF2_MASK  0x00000004U
#define MCAN_TXBCF_CF2_SHIFT 2U

// Field:     [1] CF1
//
// Cancellation Finished 1. See description for bit 0.
#define MCAN_TXBCF_CF1       0x00000002U
#define MCAN_TXBCF_CF1_MASK  0x00000002U
#define MCAN_TXBCF_CF1_SHIFT 1U

// Field:     [0] CF0
//
// Cancellation Finished 0.
//
// Each Tx Buffer has its own Cancellation Finished bit. The bits are set when
// the corresponding TXBRP bit is cleared after a cancellation was requested
// via TXBCR. In case the corresponding TXBRP bit was not set at the point of
// cancellation, CF is set immediately. The bits are reset when a new
// transmission is requested by writing a '1' to the corresponding bit of
// register TXBAR.
//   0  No transmit buffer cancellation
//   1  Transmit buffer cancellation finished
#define MCAN_TXBCF_CF0       0x00000001U
#define MCAN_TXBCF_CF0_MASK  0x00000001U
#define MCAN_TXBCF_CF0_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TXBTIE
//
//*****************************************************************************
// Field:    [31] TIE31
//
// Transmission Interrupt Enable 31. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE31       0x80000000U
#define MCAN_TXBTIE_TIE31_MASK  0x80000000U
#define MCAN_TXBTIE_TIE31_SHIFT 31U

// Field:    [30] TIE30
//
// Transmission Interrupt Enable 30. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE30       0x40000000U
#define MCAN_TXBTIE_TIE30_MASK  0x40000000U
#define MCAN_TXBTIE_TIE30_SHIFT 30U

// Field:    [29] TIE29
//
// Transmission Interrupt Enable 29. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE29       0x20000000U
#define MCAN_TXBTIE_TIE29_MASK  0x20000000U
#define MCAN_TXBTIE_TIE29_SHIFT 29U

// Field:    [28] TIE28
//
// Transmission Interrupt Enable 28. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE28       0x10000000U
#define MCAN_TXBTIE_TIE28_MASK  0x10000000U
#define MCAN_TXBTIE_TIE28_SHIFT 28U

// Field:    [27] TIE27
//
// Transmission Interrupt Enable 27. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE27       0x08000000U
#define MCAN_TXBTIE_TIE27_MASK  0x08000000U
#define MCAN_TXBTIE_TIE27_SHIFT 27U

// Field:    [26] TIE26
//
// Transmission Interrupt Enable 26. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE26       0x04000000U
#define MCAN_TXBTIE_TIE26_MASK  0x04000000U
#define MCAN_TXBTIE_TIE26_SHIFT 26U

// Field:    [25] TIE25
//
// Transmission Interrupt Enable 25. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE25       0x02000000U
#define MCAN_TXBTIE_TIE25_MASK  0x02000000U
#define MCAN_TXBTIE_TIE25_SHIFT 25U

// Field:    [24] TIE24
//
// Transmission Interrupt Enable 24. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE24       0x01000000U
#define MCAN_TXBTIE_TIE24_MASK  0x01000000U
#define MCAN_TXBTIE_TIE24_SHIFT 24U

// Field:    [23] TIE23
//
// Transmission Interrupt Enable 23. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE23       0x00800000U
#define MCAN_TXBTIE_TIE23_MASK  0x00800000U
#define MCAN_TXBTIE_TIE23_SHIFT 23U

// Field:    [22] TIE22
//
// Transmission Interrupt Enable 22. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE22       0x00400000U
#define MCAN_TXBTIE_TIE22_MASK  0x00400000U
#define MCAN_TXBTIE_TIE22_SHIFT 22U

// Field:    [21] TIE21
//
// Transmission Interrupt Enable 21. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE21       0x00200000U
#define MCAN_TXBTIE_TIE21_MASK  0x00200000U
#define MCAN_TXBTIE_TIE21_SHIFT 21U

// Field:    [20] TIE20
//
// Transmission Interrupt Enable 20. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE20       0x00100000U
#define MCAN_TXBTIE_TIE20_MASK  0x00100000U
#define MCAN_TXBTIE_TIE20_SHIFT 20U

// Field:    [19] TIE19
//
// Transmission Interrupt Enable 19. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE19       0x00080000U
#define MCAN_TXBTIE_TIE19_MASK  0x00080000U
#define MCAN_TXBTIE_TIE19_SHIFT 19U

// Field:    [18] TIE18
//
// Transmission Interrupt Enable 18. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE18       0x00040000U
#define MCAN_TXBTIE_TIE18_MASK  0x00040000U
#define MCAN_TXBTIE_TIE18_SHIFT 18U

// Field:    [17] TIE17
//
// Transmission Interrupt Enable 17. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE17       0x00020000U
#define MCAN_TXBTIE_TIE17_MASK  0x00020000U
#define MCAN_TXBTIE_TIE17_SHIFT 17U

// Field:    [16] TIE16
//
// Transmission Interrupt Enable 16. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE16       0x00010000U
#define MCAN_TXBTIE_TIE16_MASK  0x00010000U
#define MCAN_TXBTIE_TIE16_SHIFT 16U

// Field:    [15] TIE15
//
// Transmission Interrupt Enable 15. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE15       0x00008000U
#define MCAN_TXBTIE_TIE15_MASK  0x00008000U
#define MCAN_TXBTIE_TIE15_SHIFT 15U

// Field:    [14] TIE14
//
// Transmission Interrupt Enable 14. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE14       0x00004000U
#define MCAN_TXBTIE_TIE14_MASK  0x00004000U
#define MCAN_TXBTIE_TIE14_SHIFT 14U

// Field:    [13] TIE13
//
// Transmission Interrupt Enable 13. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE13       0x00002000U
#define MCAN_TXBTIE_TIE13_MASK  0x00002000U
#define MCAN_TXBTIE_TIE13_SHIFT 13U

// Field:    [12] TIE12
//
// Transmission Interrupt Enable 12. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE12       0x00001000U
#define MCAN_TXBTIE_TIE12_MASK  0x00001000U
#define MCAN_TXBTIE_TIE12_SHIFT 12U

// Field:    [11] TIE11
//
// Transmission Interrupt Enable 11. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE11       0x00000800U
#define MCAN_TXBTIE_TIE11_MASK  0x00000800U
#define MCAN_TXBTIE_TIE11_SHIFT 11U

// Field:    [10] TIE10
//
// Transmission Interrupt Enable 10. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE10       0x00000400U
#define MCAN_TXBTIE_TIE10_MASK  0x00000400U
#define MCAN_TXBTIE_TIE10_SHIFT 10U

// Field:     [9] TIE9
//
// Transmission Interrupt Enable 9. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE9       0x00000200U
#define MCAN_TXBTIE_TIE9_MASK  0x00000200U
#define MCAN_TXBTIE_TIE9_SHIFT 9U

// Field:     [8] TIE8
//
// Transmission Interrupt Enable 8. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE8       0x00000100U
#define MCAN_TXBTIE_TIE8_MASK  0x00000100U
#define MCAN_TXBTIE_TIE8_SHIFT 8U

// Field:     [7] TIE7
//
// Transmission Interrupt Enable 7. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE7       0x00000080U
#define MCAN_TXBTIE_TIE7_MASK  0x00000080U
#define MCAN_TXBTIE_TIE7_SHIFT 7U

// Field:     [6] TIE6
//
// Transmission Interrupt Enable 6. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE6       0x00000040U
#define MCAN_TXBTIE_TIE6_MASK  0x00000040U
#define MCAN_TXBTIE_TIE6_SHIFT 6U

// Field:     [5] TIE5
//
// Transmission Interrupt Enable 5. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE5       0x00000020U
#define MCAN_TXBTIE_TIE5_MASK  0x00000020U
#define MCAN_TXBTIE_TIE5_SHIFT 5U

// Field:     [4] TIE4
//
// Transmission Interrupt Enable 4. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE4       0x00000010U
#define MCAN_TXBTIE_TIE4_MASK  0x00000010U
#define MCAN_TXBTIE_TIE4_SHIFT 4U

// Field:     [3] TIE3
//
// Transmission Interrupt Enable 3. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE3       0x00000008U
#define MCAN_TXBTIE_TIE3_MASK  0x00000008U
#define MCAN_TXBTIE_TIE3_SHIFT 3U

// Field:     [2] TIE2
//
// Transmission Interrupt Enable 2. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE2       0x00000004U
#define MCAN_TXBTIE_TIE2_MASK  0x00000004U
#define MCAN_TXBTIE_TIE2_SHIFT 2U

// Field:     [1] TIE1
//
// Transmission Interrupt Enable 1. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE1       0x00000002U
#define MCAN_TXBTIE_TIE1_MASK  0x00000002U
#define MCAN_TXBTIE_TIE1_SHIFT 1U

// Field:     [0] TIE0
//
// Transmission Interrupt Enable 0. Each Tx Buffer has its own Transmission
// Interrupt Enable bit.
//   0  Transmission interrupt disabled
//   1  Transmission interrupt enable
#define MCAN_TXBTIE_TIE0       0x00000001U
#define MCAN_TXBTIE_TIE0_MASK  0x00000001U
#define MCAN_TXBTIE_TIE0_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TXBCIE
//
//*****************************************************************************
// Field:    [31] CFIE31
//
// Cancellation Finished Interrupt Enable 31. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE31       0x80000000U
#define MCAN_TXBCIE_CFIE31_MASK  0x80000000U
#define MCAN_TXBCIE_CFIE31_SHIFT 31U

// Field:    [30] CFIE30
//
// Cancellation Finished Interrupt Enable 30. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE30       0x40000000U
#define MCAN_TXBCIE_CFIE30_MASK  0x40000000U
#define MCAN_TXBCIE_CFIE30_SHIFT 30U

// Field:    [29] CFIE29
//
// Cancellation Finished Interrupt Enable 29. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE29       0x20000000U
#define MCAN_TXBCIE_CFIE29_MASK  0x20000000U
#define MCAN_TXBCIE_CFIE29_SHIFT 29U

// Field:    [28] CFIE28
//
// Cancellation Finished Interrupt Enable 28. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE28       0x10000000U
#define MCAN_TXBCIE_CFIE28_MASK  0x10000000U
#define MCAN_TXBCIE_CFIE28_SHIFT 28U

// Field:    [27] CFIE27
//
// Cancellation Finished Interrupt Enable 27. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE27       0x08000000U
#define MCAN_TXBCIE_CFIE27_MASK  0x08000000U
#define MCAN_TXBCIE_CFIE27_SHIFT 27U

// Field:    [26] CFIE26
//
// Cancellation Finished Interrupt Enable 26. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE26       0x04000000U
#define MCAN_TXBCIE_CFIE26_MASK  0x04000000U
#define MCAN_TXBCIE_CFIE26_SHIFT 26U

// Field:    [25] CFIE25
//
// Cancellation Finished Interrupt Enable 25. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE25       0x02000000U
#define MCAN_TXBCIE_CFIE25_MASK  0x02000000U
#define MCAN_TXBCIE_CFIE25_SHIFT 25U

// Field:    [24] CFIE24
//
// Cancellation Finished Interrupt Enable 24. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE24       0x01000000U
#define MCAN_TXBCIE_CFIE24_MASK  0x01000000U
#define MCAN_TXBCIE_CFIE24_SHIFT 24U

// Field:    [23] CFIE23
//
// Cancellation Finished Interrupt Enable 23. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE23       0x00800000U
#define MCAN_TXBCIE_CFIE23_MASK  0x00800000U
#define MCAN_TXBCIE_CFIE23_SHIFT 23U

// Field:    [22] CFIE22
//
// Cancellation Finished Interrupt Enable 22. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE22       0x00400000U
#define MCAN_TXBCIE_CFIE22_MASK  0x00400000U
#define MCAN_TXBCIE_CFIE22_SHIFT 22U

// Field:    [21] CFIE21
//
// Cancellation Finished Interrupt Enable 21. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE21       0x00200000U
#define MCAN_TXBCIE_CFIE21_MASK  0x00200000U
#define MCAN_TXBCIE_CFIE21_SHIFT 21U

// Field:    [20] CFIE20
//
// Cancellation Finished Interrupt Enable 20. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE20       0x00100000U
#define MCAN_TXBCIE_CFIE20_MASK  0x00100000U
#define MCAN_TXBCIE_CFIE20_SHIFT 20U

// Field:    [19] CFIE19
//
// Cancellation Finished Interrupt Enable 19. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE19       0x00080000U
#define MCAN_TXBCIE_CFIE19_MASK  0x00080000U
#define MCAN_TXBCIE_CFIE19_SHIFT 19U

// Field:    [18] CFIE18
//
// Cancellation Finished Interrupt Enable 18. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE18       0x00040000U
#define MCAN_TXBCIE_CFIE18_MASK  0x00040000U
#define MCAN_TXBCIE_CFIE18_SHIFT 18U

// Field:    [17] CFIE17
//
// Cancellation Finished Interrupt Enable 17. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE17       0x00020000U
#define MCAN_TXBCIE_CFIE17_MASK  0x00020000U
#define MCAN_TXBCIE_CFIE17_SHIFT 17U

// Field:    [16] CFIE16
//
// Cancellation Finished Interrupt Enable 16. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE16       0x00010000U
#define MCAN_TXBCIE_CFIE16_MASK  0x00010000U
#define MCAN_TXBCIE_CFIE16_SHIFT 16U

// Field:    [15] CFIE15
//
// Cancellation Finished Interrupt Enable 15. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE15       0x00008000U
#define MCAN_TXBCIE_CFIE15_MASK  0x00008000U
#define MCAN_TXBCIE_CFIE15_SHIFT 15U

// Field:    [14] CFIE14
//
// Cancellation Finished Interrupt Enable 14. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE14       0x00004000U
#define MCAN_TXBCIE_CFIE14_MASK  0x00004000U
#define MCAN_TXBCIE_CFIE14_SHIFT 14U

// Field:    [13] CFIE13
//
// Cancellation Finished Interrupt Enable 13. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE13       0x00002000U
#define MCAN_TXBCIE_CFIE13_MASK  0x00002000U
#define MCAN_TXBCIE_CFIE13_SHIFT 13U

// Field:    [12] CFIE12
//
// Cancellation Finished Interrupt Enable 12. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE12       0x00001000U
#define MCAN_TXBCIE_CFIE12_MASK  0x00001000U
#define MCAN_TXBCIE_CFIE12_SHIFT 12U

// Field:    [11] CFIE11
//
// Cancellation Finished Interrupt Enable 11. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE11       0x00000800U
#define MCAN_TXBCIE_CFIE11_MASK  0x00000800U
#define MCAN_TXBCIE_CFIE11_SHIFT 11U

// Field:    [10] CFIE10
//
// Cancellation Finished Interrupt Enable 10. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE10       0x00000400U
#define MCAN_TXBCIE_CFIE10_MASK  0x00000400U
#define MCAN_TXBCIE_CFIE10_SHIFT 10U

// Field:     [9] CFIE9
//
// Cancellation Finished Interrupt Enable 9. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE9       0x00000200U
#define MCAN_TXBCIE_CFIE9_MASK  0x00000200U
#define MCAN_TXBCIE_CFIE9_SHIFT 9U

// Field:     [8] CFIE8
//
// Cancellation Finished Interrupt Enable 8. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE8       0x00000100U
#define MCAN_TXBCIE_CFIE8_MASK  0x00000100U
#define MCAN_TXBCIE_CFIE8_SHIFT 8U

// Field:     [7] CFIE7
//
// Cancellation Finished Interrupt Enable 7. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE7       0x00000080U
#define MCAN_TXBCIE_CFIE7_MASK  0x00000080U
#define MCAN_TXBCIE_CFIE7_SHIFT 7U

// Field:     [6] CFIE6
//
// Cancellation Finished Interrupt Enable 6. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE6       0x00000040U
#define MCAN_TXBCIE_CFIE6_MASK  0x00000040U
#define MCAN_TXBCIE_CFIE6_SHIFT 6U

// Field:     [5] CFIE5
//
// Cancellation Finished Interrupt Enable 5. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE5       0x00000020U
#define MCAN_TXBCIE_CFIE5_MASK  0x00000020U
#define MCAN_TXBCIE_CFIE5_SHIFT 5U

// Field:     [4] CFIE4
//
// Cancellation Finished Interrupt Enable 4. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE4       0x00000010U
#define MCAN_TXBCIE_CFIE4_MASK  0x00000010U
#define MCAN_TXBCIE_CFIE4_SHIFT 4U

// Field:     [3] CFIE3
//
// Cancellation Finished Interrupt Enable 3. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE3       0x00000008U
#define MCAN_TXBCIE_CFIE3_MASK  0x00000008U
#define MCAN_TXBCIE_CFIE3_SHIFT 3U

// Field:     [2] CFIE2
//
// Cancellation Finished Interrupt Enable 2. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE2       0x00000004U
#define MCAN_TXBCIE_CFIE2_MASK  0x00000004U
#define MCAN_TXBCIE_CFIE2_SHIFT 2U

// Field:     [1] CFIE1
//
// Cancellation Finished Interrupt Enable 1. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE1       0x00000002U
#define MCAN_TXBCIE_CFIE1_MASK  0x00000002U
#define MCAN_TXBCIE_CFIE1_SHIFT 1U

// Field:     [0] CFIE0
//
// Cancellation Finished Interrupt Enable 0. Each Tx Buffer has its own
// Cancellation Finished Interrupt Enable bit.
//   0  Cancellation finished interrupt disabled
//   1  Cancellation finished interrupt enabled
#define MCAN_TXBCIE_CFIE0       0x00000001U
#define MCAN_TXBCIE_CFIE0_MASK  0x00000001U
#define MCAN_TXBCIE_CFIE0_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TXEFC
//
//*****************************************************************************
// Field: [29:24] EFWM
//
// Event FIFO Watermark
//   0      Watermark interrupt disabled
//   1-32  Level for Tx Event FIFO watermark interrupt (IR.TEFW)
//   >32   Watermark interrupt disabled
#define MCAN_TXEFC_EFWM_WIDTH 6U
#define MCAN_TXEFC_EFWM_MASK  0x3F000000U
#define MCAN_TXEFC_EFWM_SHIFT 24U

// Field: [21:16] EFS
//
// Event FIFO Size. The Tx Event FIFO elements are indexed from 0 to EFS - 1.
//   0      Tx Event FIFO disabled
//   1-32  Number of Tx Event FIFO elements
//   >32   Values greater than 32 are interpreted as 32
#define MCAN_TXEFC_EFS_WIDTH 6U
#define MCAN_TXEFC_EFS_MASK  0x003F0000U
#define MCAN_TXEFC_EFS_SHIFT 16U

// Field:  [15:2] EFSA
//
// Event FIFO Start Address. Start address of Tx Event FIFO in Message RAM
// (32-bit word address).
#define MCAN_TXEFC_EFSA_WIDTH 14U
#define MCAN_TXEFC_EFSA_MASK  0x0000FFFCU
#define MCAN_TXEFC_EFSA_SHIFT 2U

//*****************************************************************************
//
// Register: MCAN_TXEFS
//
//*****************************************************************************
// Field:    [25] TEFL
//
// Tx Event FIFO Element Lost. This bit is a copy of interrupt flag IR.TEFL.
// When IR.TEFL is reset, this bit is also reset.
//   0  No Tx Event FIFO element lost
//   1  Tx Event FIFO element lost, also set after write attempt to Tx Event
// FIFO of size zero.
#define MCAN_TXEFS_TEFL       0x02000000U
#define MCAN_TXEFS_TEFL_MASK  0x02000000U
#define MCAN_TXEFS_TEFL_SHIFT 25U

// Field:    [24] EFF
//
// Event FIFO Full
//   0  Tx Event FIFO not full
//   1  Tx Event FIFO full
#define MCAN_TXEFS_EFF       0x01000000U
#define MCAN_TXEFS_EFF_MASK  0x01000000U
#define MCAN_TXEFS_EFF_SHIFT 24U

// Field: [20:16] EFPI
//
// Event FIFO Put Index.Tx Event FIFO write index pointer, range 0 to 31.
#define MCAN_TXEFS_EFPI_WIDTH 5U
#define MCAN_TXEFS_EFPI_MASK  0x001F0000U
#define MCAN_TXEFS_EFPI_SHIFT 16U

// Field:  [12:8] EFGI
//
// Event FIFO Get Index. Tx Event FIFO read index pointer, range 0 to 31.
#define MCAN_TXEFS_EFGI_WIDTH 5U
#define MCAN_TXEFS_EFGI_MASK  0x00001F00U
#define MCAN_TXEFS_EFGI_SHIFT 8U

// Field:   [5:0] EFFL
//
// Event FIFO Fill Level. Number of elements stored in Tx Event FIFO, range 0
// to 32.
#define MCAN_TXEFS_EFFL_WIDTH 6U
#define MCAN_TXEFS_EFFL_MASK  0x0000003FU
#define MCAN_TXEFS_EFFL_SHIFT 0U

//*****************************************************************************
//
// Register: MCAN_TXEFA
//
//*****************************************************************************
// Field:   [4:0] EFAI
//
// Event FIFO Acknowledge Index. After the Host has read an element or a
// sequence of elements from the Tx Event FIFO it has to write the index of the
// last element read from Tx Event FIFO to EFAI. This will set the Tx Event
// FIFO Get Index TXEFS.EFGI to EFAI + 1 and update the Event FIFO Fill Level
// TXEFS.EFFL.
#define MCAN_TXEFA_EFAI_WIDTH 5U
#define MCAN_TXEFA_EFAI_MASK  0x0000001FU
#define MCAN_TXEFA_EFAI_SHIFT 0U

#endif /* __MCAN_REG_H__ */
