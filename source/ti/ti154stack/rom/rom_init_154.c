/******************************************************************************

 @file  rom_init_154.c

 @brief This file contains the externs for FH ROM API initialization.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2017-2025, Texas Instruments Incorporated

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

/*******************************************************************************
 * INCLUDES
 */

#include "rom_jt_def_154.h"

/*******************************************************************************
 * EXTERNS
 */


/*******************************************************************************
 * PROTOTYPES
 */

/*******************************************************************************
 * MACROS
 */

/* RAM address assigned */
#if defined __TI_COMPILER_VERSION__ && defined __TI_ARM__
#pragma DATA_SECTION(RAM_MAC_BASE_ADDR ,".data:RAM_MAC_BASE_ADDR")
uint32_t RAM_MAC_BASE_ADDR[ROM_RAM_TABLE_SIZE];

#elif defined(__IAR_SYSTEMS_ICC__)
#pragma location=".data_RAM_BASE_ADDR"
uint32_t RAM_MAC_BASE_ADDR[ROM_RAM_TABLE_SIZE];
#elif defined(__GNUC__) || defined(__clang__)
uint32_t RAM_MAC_BASE_ADDR[ROM_RAM_TABLE_SIZE] __attribute__((section(".data:RAM_MAC_BASE_ADDR")));
#else
#error "Unsupported platform or compiler"
#endif

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * TYPEDEFS
 */


/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */



/* ******************************************************************************
 * @fn          FH ROM Spinlock
 *
 * @brief       This routine is used to trap indexing errors in R2R JT.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void ROM_MAC_Spinlock( void )
{
  volatile uint8 i = 1;

  while(i);
}

/*******************************************************************************
 * @fn          BLE ROM Initialization
 *
 * @brief       This routine initializes the BLE Controller ROM software. First,
 *              the image's CRC is verified. Next, its C runtime is initialized.
 *              Then the ICall function pointers for dispatch, and enter/leave
 *              critical section are initialized. Finally, the location of the
 *              R2F and R2R flash jump tables are initialized.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void TIMAC_ROM_Init( void )
{
#if 0
  volatile uint8 i;

  // verify the Controller ROM image
  i = validChecksum(&__checksum_begin, &__checksum_end);

  // trap a checksum failure - what now?
  while( !i );
#endif // ENABLE_ROM_CHECKSUM_CHECK

  /*
  ** Controller ROM
  */
  Icall_Osal_ROM_Init();
#ifdef FEATURE_FREQ_HOP_MODE
  FH_ROM_Init();
#endif
  HMAC_ROM_Init();
  LMAC_ROM_Init();

  return;
}



