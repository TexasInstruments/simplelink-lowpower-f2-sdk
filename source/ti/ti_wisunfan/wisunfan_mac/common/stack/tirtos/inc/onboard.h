/******************************************************************************

 @file  onboard.h

 @brief Defines stuff for Evaluation boards

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2013-2025, Texas Instruments Incorporated

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

#ifndef ONBOARD_H
#define ONBOARD_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#ifdef __IAR_SYSTEMS_ICC__
#include <intrinsics.h>
#endif /* __IAR_SYSTEMS_ICC__ */

#include "hal_mcu.h"
#include "hal_uart.h"
#include "hal_sleep.h"
#include "osal.h"

/*********************************************************************
 * GLOBAL VARIABLES
 */

// 64-bit Extended Address of this device
extern uint8 aExtendedAddress[8];

/*********************************************************************
 * CONSTANTS
 */

// Timer clock and power-saving definitions
#define TICK_COUNT         1  // TIMAC requires this number to be 1

/* OSAL timer defines */
#define TICK_TIME   1000   // Timer per tick - in micro-sec

// These Key definitions are unique to this development system.
// They are used to bypass functions when starting up the device.
#define SW_BYPASS_NV    HAL_KEY_SW_2  // Bypass Network layer NV restore
#define SW_BYPASS_START HAL_KEY_SW_1  // Bypass Network initialization

// LCD Support Defintions
#ifdef LCD_SUPPORTED
  #if !defined DEBUG
    #define DEBUG  0
  #endif
  #if LCD_SUPPORTED==DEBUG
    #define SERIAL_DEBUG_SUPPORTED  // Serial-debug
  #endif
#else // No LCD support
  #undef SERIAL_DEBUG_SUPPORTED  // No serial-debug
#endif

/* Serial Port Definitions */
#if defined (ZAPP_P1)
  #define ZAPP_PORT HAL_UART_PORT_0 //SERIAL_PORT1
#elif defined (ZAPP_P2)
  #define ZAPP_PORT HAL_UART_PORT_1 //SERIAL_PORT2
#else
  #undef ZAPP_PORT
#endif
#if defined (ZTOOL_P1)
  #define ZTOOL_PORT HAL_UART_PORT_0 //SERIAL_PORT1
#elif defined (ZTOOL_P2)
  #define ZTOOL_PORT HAL_UART_PORT_1 //SERIAL_PORT2
#else
  #undef ZTOOL_PORT
#endif

/* Tx and Rx buffer size defines used by SPIMgr.c */
#define MT_UART_TX_BUFF_MAX  170
#define MT_UART_RX_BUFF_MAX  120
#define MT_UART_THRESHOLD    5
#define MT_UART_IDLE_TIMEOUT 5

// Restart system from absolute beginning
// Disables interrupts, forces WatchDog reset
#define SystemReset()  HAL_SYSTEM_RESET()

/* Reset reason for reset indication */
#define ResetReason() (0)

#define BootLoader()   // Not yet implemented

// Power conservation
#define OSAL_SET_CPU_INTO_SLEEP(timeout) halSleep(timeout);  /* Called from OSAL_PwrMgr */


// Internal (MCU) RAM addresses
#define MCU_RAM_BEG 0x1100
#define MCU_RAM_END 0x20FF
#define MCU_RAM_LEN (MCU_RAM_END - MCU_RAM_BEG + 1)


#ifdef __IAR_SYSTEMS_ICC__
// Internal (MCU) Stack addresses
#define CSTACK_BEG ((uint8 const *)(_Pragma("segment=\"CSTACK\"") __segment_begin("CSTACK")))
#define CSTACK_END ((uint8 const *)(_Pragma("segment=\"CSTACK\"") __segment_end("CSTACK"))-1)
// Stack Initialization Value
#define STACK_INIT_VALUE  0xCD
#else
#if !(defined __TI_COMPILER_VERSION__ || defined __GNUC__)
/* TODO: implement stack check for CCS tool chain */
#error Check compiler compatibility.
#endif /* ccs */
#endif


/* The following Heap sizes are setup for typical TI sample applications,
 * and should be adjusted to your systems requirements.
 */
// Internal (MCU) heap size
#if !defined( INT_HEAP_LEN )
  #define INT_HEAP_LEN  6144  // 6.0K
#endif

// Memory Allocation Heap
#define MAXMEMHEAP INT_HEAP_LEN  // Typically, 0.70-1.50K

// Initialization levels
#define OB_COLD  0
#define OB_WARM  1
#define OB_READY 2

#ifdef LCD_SUPPORTED
  #define BUZZER_OFF  0
  #define BUZZER_ON   1
  #define BUZZER_BLIP 2
#endif

#define SystemResetSoft()  SystemReset()

/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
  osal_event_hdr_t hdr;
  uint8 state; // shift
  uint8 keys;  // keys
} keyChange_t;

/*********************************************************************
 * FUNCTIONS
 */
  /*
   * Initialize the Peripherals
   *    level: 0=cold, 1=warm, 2=ready
   */
  extern void InitBoard( uint8 level );

 /*
  * Get elapsed timer clock counts
  */
  extern uint32 TimerElapsed( void );

  /*
   * Register for all key events
   */
  extern uint8 RegisterForKeys( uint8 task_id );

/* Keypad Control Functions */

  /*
   * Send "Key Pressed" message to application
   */
  extern uint8 OnBoard_SendKeys( uint8 keys, uint8 state );

/* LCD Emulation/Control Functions */
  /*
   * Convert an interger to an ascii string
   */
  extern void _itoa( uint16 num, uint8 *buf, uint8 radix );


  extern void Dimmer( uint8 lvl );

/* External I/O Processing Functions */
  /*
   * Turn on an external lamp
   */
  extern void BigLight_On( void );

  /*
   * Turn off an external lamp
   */
  extern void BigLight_Off( void );

  /*
   * Turn on/off an external buzzer
   *   on:   BUZZER_ON or BUZZER_OFF
   */
  extern void BuzzerControl( uint8 on );

  /*
   * Get setting of external dip switch
   */
  extern uint8 GetUserDipSw( void );

  /*
   * Calculate the size of used stack
   */
  extern uint16 OnBoard_stack_used( void );

  /*
   * Callback routine to handle keys
   */
  extern void OnBoard_KeyCallback ( uint8 keys, uint8 state );

  /*
   * Board specific random number generator
   */
  extern uint16 Onboard_rand( void );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif // ONBOARD_H
