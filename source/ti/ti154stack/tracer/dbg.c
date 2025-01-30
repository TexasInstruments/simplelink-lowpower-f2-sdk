/******************************************************************************

 @file  dbg.c

 @brief Handles debug trace operations

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2011-2025, Texas Instruments Incorporated

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

/// \addtogroup module_dbg
//@{
#include "dbg.h"
#include "rfctrc_regs.h"
//#include "interrupt.h"
#include <stdint.h>
#include "cpu.h"
#include <ti/drivers/rf/RF.h>
#ifndef DBG_GLOBAL_DISABLE

typedef char cs_t;

#define ENTER_CRITICAL_SECTION(x)                                              \
  do { (x) = !CPUcpsid(); } while (0)

#define EXIT_CRITICAL_SECTION(x)                                               \
  do { if (x) { (void) CPUcpsie(); } } while (0)

#define __enable_irq()    asm("cpsie i");
#define __disable_irq()   asm("cpsid i");
/** \brief Function used to implement DBG_PRINTn
 *
 * Waits for the LVDS channel to be ready before storing the printf() parameters and transmitting the
 * LVDS packet.
 *
 * \param[in]  x
 *     Contains the following values:
 *     - [17:16] The channel number to use (1, 2 or 3)
 *     - [15:8] Trace packet header byte
 *     - [2:0] Number of 16-bit printf() arguments, 0-4
 * \param[in]  y
 *     Contains printf() argument 0 in LSW and argument 1 in MSW
 * \param[in]  z
 *     Contains printf() argument 2 in LSW and argument 3 in MSW
 */
// Disable Optimization to prevent GCC builds from failing to link in debug data

bool getRFState()
{
  RF_InfoVal info;

  RF_getInfo(NULL, RF_GET_RADIO_STATE, &info);
  return (info.bRadioState);
}

#if defined(__GNUC__)
#pragma GCC push_options
#pragma GCC optimize ("O0")
#endif //__GNUC__

void dbgPrintf(uint32_t x, uint32_t y, uint32_t z)
{
   int  ch = (x >> 16) - 1;
   cs_t cs;
   if (getRFState())
   {
       // Wait for previous packet to be transmitted
       while (1) {
          //__disable_irq();
          ENTER_CRITICAL_SECTION(cs);
          if ((SP_TRCCH1CMD[ch] & TRCCH1CMD_CH1PKTHDR_BM) == 0) {
             // Channel ready, transmit packet
             SP_TRCCH1PAR01[ch] = y;
             SP_TRCCH1PAR23[ch] = z;
             SP_TRCCH1CMD[ch] = x;
             //__enable_irq();
             EXIT_CRITICAL_SECTION(cs);
             break;
          }
          else {
             //__enable_irq();
             EXIT_CRITICAL_SECTION(cs);
          }
       }
   }
} // dbgPrintf



/** \brief Function used to implement DBG_PRINTn, n=1,2
 *
 * Waits for the LVDS channel to be ready before storing the printf() parameters and transmitting the
 * LVDS packet.
 *
 * \param[in]  x
 *     Contains the following values:
 *     - [17:16] The channel number to use (1, 2 or 3)
 *     - [15:8] Trace packet header byte
 *     - [2:0] Number of 16-bit printf() arguments, 0-2
 * \param[in]  y
 *     Contains printf() argument 0 in LSW and argument 1 in MSW
 */
void dbgPrintf2(uint32_t x, uint32_t y)
{
   int  ch = (x >> 16) - 1;
   cs_t cs;
   if (getRFState())
   {
       // Wait for previous packet to be transmitted
       while (1) {
          //__disable_irq();
          ENTER_CRITICAL_SECTION(cs);
          if ((SP_TRCCH1CMD[ch] & TRCCH1CMD_CH1PKTHDR_BM) == 0) {
             // Channel ready, transmit packet
             SP_TRCCH1PAR01[ch] = y;
             SP_TRCCH1CMD[ch] = x;
             //__enable_irq();
             EXIT_CRITICAL_SECTION(cs);
             break;
          }
          else {
             //__enable_irq();
             EXIT_CRITICAL_SECTION(cs);
          }
       }
   }
} // dbgPrintf2



/** \brief Function used to implement DBG_PRINT0
 *
 * Waits for the LVDS channel to be ready before storing the printf() parameters and transmitting the
 * LVDS packet.
 *
 * \param[in]  x
 *     Contains the following values:
 *     - [17:16] The channel number to use (1, 2 or 3)
 *     - [15:8] Trace packet header byte
 *     - [2:0] Number of 16-bit printf() arguments, 0
 */
void dbgPrintf0(uint32_t x)
{
   int  ch = (x >> 16) - 1;
   cs_t cs;
   if (getRFState())
   {
       // Wait for previous packet to be transmitted
       while (1) {
          //__disable_irq();
          ENTER_CRITICAL_SECTION(cs);
          if ((SP_TRCCH1CMD[ch] & TRCCH1CMD_CH1PKTHDR_BM) == 0) {
             // Channel ready, transmit packet
             SP_TRCCH1CMD[ch] = x;
             //__enable_irq();
             EXIT_CRITICAL_SECTION(cs);
             break;
          }
          else {
             //__enable_irq();
             EXIT_CRITICAL_SECTION(cs);
          }
       }
   }
} // dbgPrintf0
#if defined(__GNUC__)
#pragma GCC pop_options
#endif //__GNUC__
#endif // !DBG_GLOBAL_DISABLE


//@}

