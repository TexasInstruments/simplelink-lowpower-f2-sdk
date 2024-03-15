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
 *  @file       TCAN455X_reg.h
 *  @brief      TCAN455X device-specific register definitions
 *******************************************************************************
 */

#ifndef __TCAN455X_REG_H__
#define __TCAN455X_REG_H__

/******************************************************************************/
/* Device ID and Interrupt/Diagnostic Flag Register Addresses */
/******************************************************************************/
#define TCAN455X_DEVICE_ID0 0x0000
#define TCAN455X_DEVICE_ID1 0x0004
#define TCAN455X_REVISION   0x0008
#define TCAN455X_STATUS     0x000C

/******************************************************************************/
/* Device Configuration Register Addresses */
/******************************************************************************/
#define TCAN455X_MODE         0x0800
#define TCAN455X_TS_PRESCALER 0x0804
#define TCAN455X_TEST         0x0808
#define TCAN455X_IR           0x0820
#define TCAN455X_IE           0x0830

/******************************************************************************/
/* MCAN and MRAM Register Address */
/******************************************************************************/
#define TCAN455X_MCAN 0x1000U
#define TCAN455X_MRAM 0x8000U

/******************************************************************************/
/* Device Register Bit Field Defines */
/******************************************************************************/

/* Modes of Operation and Pin Configuration Registers (0x0800) */

/*! Mode register Bit 5 is a reserved bit that must be written as a 1 */
#define TCAN455X_MODE_FORCED_SET_BITS 0x00000020

/* Wake pin */
#define TCAN455X_MODE_WAKE_PIN_MASK      0xC0000000
#define TCAN455X_MODE_WAKE_PIN_DIS       0x00000000
#define TCAN455X_MODE_WAKE_PIN_RISING    0x40000000
#define TCAN455X_MODE_WAKE_PIN_FALLING   0x80000000
#define TCAN455X_MODE_WAKE_PIN_BOTHEDGES 0xC0000000

/* GPO2 Pin Configuration */
#define TCAN455X_MODE_GPO2_MASK      0x00C00000
#define TCAN455X_MODE_GPO2_CAN_FAULT 0x00000000
#define TCAN455X_MODE_GPO2_MCAN_INT0 0x00400000
#define TCAN455X_MODE_GPO2_WDT       0x00800000
#define TCAN455X_MODE_GPO2_NINT      0x00C00000

/* Test Mode Enable Bit */
#define TCAN455X_MODE_TESTMODE_ENMASK 0x00200000
#define TCAN455X_MODE_TESTMODE_EN     0x00200000
#define TCAN455X_MODE_TESTMODE_DIS    0x00000000

/* nWKRQ Pin GPO Voltage Rail COnfiguration */
#define TCAN455X_MODE_NWKRQ_VOLT_MASK     0x00080000
#define TCAN455X_MODE_NWKRQ_VOLT_INTERNAL 0x00000000
#define TCAN455X_MODE_NWKRQ_VOLT_VIO      0x00080000

/* WD_BIT */
#define TCAN455X_MODE_WDT_RESET_BIT 0x00040000

/* WD_ACTION */
#define TCAN455X_MODE_WDT_ACTION_MASK      0x00020000
#define TCAN455X_MODE_WDT_ACTION_INT       0x00000000
#define TCAN455X_MODE_WDT_ACTION_INH_PULSE 0x00010000
#define TCAN455X_MODE_WDT_ACTION_WDT_PULSE 0x00020000

/* CLKOUT/GPO1 Pin Mode Select */
#define TCAN455X_MODE_GPO1_MODE_MASK   0x0000C000
#define TCAN455X_MODE_GPO1_MODE_GPO    0x00000000
#define TCAN455X_MODE_GPO1_MODE_CLKOUT 0x00004000
#define TCAN455X_MODE_GPO1_MODE_GPI    0x00008000

/* Fail Safe Enable */
#define TCAN455X_MODE_FAIL_SAFE_MASK 0x00002000
#define TCAN455X_MODE_FAIL_SAFE_EN   0x00002000
#define TCAN455X_MODE_FAIL_SAFE_DIS  0x00000000

/* CLKOUT Prescaler */
#define TCAN455X_MODE_CLKOUT_MASK 0x00001000
#define TCAN455X_MODE_CLKOUT_DIV1 0x00000000
#define TCAN455X_MODE_CLKOUT_DIV2 0x00001000

/* GPO1 Function Select */
#define TCAN455X_MODE_GPO1_FUNC_MASK       0x00000C00
#define TCAN455X_MODE_GPO1_FUNC_SPI_INT    0x00000000
#define TCAN455X_MODE_GPO1_FUNC_MCAN_INT1  0x00000400
#define TCAN455X_MODE_GPO1_FUNC_UVLO_THERM 0x00000800

/* INH Pin Disable */
#define TCAN455X_MODE_INH_MASK 0x00000200
#define TCAN455X_MODE_INH_DIS  0x00000200
#define TCAN455X_MODE_INH_EN   0x00000000

/* nWKRQ Pin Configuration */
#define TCAN455X_MODE_NWKRQ_CONFIG_MASK 0x00000100
#define TCAN455X_MODE_NWKRQ_CONFIG_INH  0x00000000
#define TCAN455X_MODE_NWKRQ_CONFIG_WKRQ 0x00000100

/* Mode of Operation */
#define TCAN455X_MODE_OPMODE_MASK    0x000000C0
#define TCAN455X_MODE_OPMODE_SLEEP   0x00000000
#define TCAN455X_MODE_OPMODE_STANDBY 0x00000040
#define TCAN455X_MODE_OPMODE_NORMAL  0x00000080

/* WDT_EN */
#define TCAN455X_MODE_WDT_MASK 0x00000008
#define TCAN455X_MODE_WDT_EN   0x00000008
#define TCAN455X_MODE_WDT_DIS  0x00000000

/* Dev Reset */
#define TCAN455X_MODE_RESET 0x00000004

/* SWE_DIS: Sleep Wake Error Disable */
#define TCAN455X_MODE_SWE_MASK 0x00000002
#define TCAN455X_MODE_SWE_DIS  0x00000002
#define TCAN455X_MODE_SWE_EN   0x00000000

/* Test Mode Configuration */
#define TCAN455X_MODE_TESTMODE_MASK       0x00000001
#define TCAN455X_MODE_TESTMODE_PHY        0x00000000
#define TCAN455X_MODE_TESTMODE_CONTROLLER 0x00000001

/* Device Interrupt Register values (0x0820) */
#define TCAN455X_IR_CANLGND     0x08000000
#define TCAN455X_IR_CANBUSOPEN  0x04000000
#define TCAN455X_IR_CANBUSGND   0x02000000
#define TCAN455X_IR_CANBUSBAT   0x01000000
#define TCAN455X_IR_RSVD3       0x00800000
#define TCAN455X_IR_UVSUP       0x00400000
#define TCAN455X_IR_UVIO        0x00200000
#define TCAN455X_IR_PWRON       0x00100000
#define TCAN455X_IR_TSD         0x00080000
#define TCAN455X_IR_WDTO        0x00040000
#define TCAN455X_IR_RSVD2       0x00020000
#define TCAN455X_IR_ECCERR      0x00010000
#define TCAN455X_IR_CANINT      0x00008000
#define TCAN455X_IR_LWU         0x00004000
#define TCAN455X_IR_WKERR       0x00002000
#define TCAN455X_IR_FRAME_OVF   0x00001000
#define TCAN455X_IR_RSVD1       0x00000800
#define TCAN455X_IR_CANSLNT     0x00000400
#define TCAN455X_IR_RSVD0       0x00000200
#define TCAN455X_IR_CANDOM      0x00000100
#define TCAN455X_IR_GLOBALERR   0x00000080
#define TCAN455X_IR_nWKRQ       0x00000040
#define TCAN455X_IR_CANERR      0x00000020
#define TCAN455X_IR_CANBUSFAULT 0x00000010
#define TCAN455X_IR_SPIERR      0x00000008
#define TCAN455X_IR_SWERR       0x00000004
#define TCAN455X_IR_M_CAN_INT   0x00000002
#define TCAN455X_IR_VTWD        0x00000001

/* Device Interrupt Enable Values (0x0830) */
#define TCAN455X_IE_UVCCOUT   0x00800000
#define TCAN455X_IE_UVSUP     0x00400000
#define TCAN455X_IE_UVIO      0x00200000
#define TCAN455X_IE_PWRON     0x00100000
#define TCAN455X_IE_TSD       0x00080000
#define TCAN455X_IE_WDTO      0x00040000
#define TCAN455X_IE_RSVD2     0x00020000
#define TCAN455X_IE_ECCERR    0x00010000
#define TCAN455X_IE_CANINT    0x00008000
#define TCAN455X_IE_LWU       0x00004000
#define TCAN455X_IE_WKERR     0x00002000
#define TCAN455X_IE_FRAME_OVF 0x00001000
#define TCAN455X_IE_RSVD1     0x00000800
#define TCAN455X_IE_CANSLNT   0x00000400
#define TCAN455X_IE_RSVD0     0x00000200
#define TCAN455X_IE_CANDOM    0x00000100
#define TCAN455X_IE_MASK      0xFF69D700

#endif /* __TCAN455X_REG_H__ */
