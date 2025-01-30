/******************************************************************************

 @file  CommonROM_Init.c

 @brief This file contains the entry point for the Common ROM.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2014-2025, Texas Instruments Incorporated

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

#include "hal_types.h"
#include "hw_types.h"
#include "CommonROM_Init.h"
//
#include "OnBoard.h"
#include "osal_bufmgr.h"
#include "hal_assert.h"

#ifdef OSAL_PORT2TIRTOS
#include <ICall.h>
#endif

/*******************************************************************************
 * EXTERNS
 */

// RAM address containing a pointer to the R2F flash jump table base address.
// Note: This linker imported symbol is treated as a variable by the compiler.

// ROM base address where the ROM's C runtime routine is expected.
extern uint32 COMMON_ROM_BASE_ADDR;

// RAM base address of a table a pointers that are used by ROM and which are
// initialized by ROM_Init.
extern uint32 COMMON_RAM_BASE_ADDR;

// Checksum start/end address and value for Common ROM.
extern const uint32 __checksum_begin_CommonROM;
extern const uint32 __checksum_end_CommonROM;

// Function pointers used by the Flash software (provided by RTOS).
extern ICall_Dispatcher ICall_dispatcher;
extern ICall_EnterCS    ICall_enterCriticalSection;
extern ICall_LeaveCS    ICall_leaveCriticalSection;

// calculate checksum routine
extern uint16 slow_crc16( uint16 crc, uint8 *pAddr, uint32 len );
extern uint8  validChecksum( const uint32 *beginAddr, const uint32 *endAddr );

/*******************************************************************************
 * PROTOTYPES
 */

/*******************************************************************************
 * MACROS
 */

// ICall function pointers and R2F/R2R flash JT pointers for Common ROM
#define icallCmnRomDispatchPtr (uint32 *)(&COMMON_RAM_BASE_ADDR+0)
#define icallCmnRomEnterCSPtr  (uint32 *)(&COMMON_RAM_BASE_ADDR+1)
#define icallCmnRomLeaveCSPtr  (uint32 *)(&COMMON_RAM_BASE_ADDR+2)
#define r2fCmnRomPtr           (uint32 *)(&COMMON_RAM_BASE_ADDR+3)
#define r2rCmnRomPtr           (uint32 *)(&COMMON_RAM_BASE_ADDR+4)

// Runtime Init code for ROM
#define RT_Init_CommonROM ((RT_Init_fp)&COMMON_ROM_BASE_ADDR)

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * TYPEDEFS
 */

typedef void (*RT_Init_fp)(void);

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

// ROM-to-Flash Flash Jump Table
// Note: Any change here must accompany a change to R2F_CommonFlashJT.h defines!

#pragma data_alignment=4
const uint32 R2F_Flash_JT_CommonROM[] =
{
  // ROM-to-Flash Functions
  (uint32)osal_mem_alloc,                                 // R2F_JT_OFFSET[0]
  (uint32)osal_mem_free,                                  // R2F_JT_OFFSET[1]
  (uint32)osal_bm_free,                                   // R2F_JT_OFFSET[2]
  (uint32)halAssertHandler,                               // R2F_JT_OFFSET[3]
};

/*******************************************************************************
 * @fn          Common ROM Initialization
 *
 * @brief       This routine initializes the Common ROM software. First,
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
void CommonROM_Init( void )
{
  volatile uint8 i;

  // verify the Controller ROM image
  i = validChecksum(&__checksum_begin_CommonROM, &__checksum_end_CommonROM);

  // trap a checksum failure - what now?
  while( !i );

  // execute the Common ROM C runtime initializion
  // Note: This is the ROM's C Runtime initialization, not the flash's, which
  //       has already taken place.
  RT_Init_CommonROM();

  // initialize ICall function pointers for Common ROM
  // Note: The address of these functions is determined by the Application, and
  //       is passed to the Stack image via startup_entry.
  *icallCmnRomDispatchPtr = (uint32)ICall_dispatcher;
  *icallCmnRomEnterCSPtr  = (uint32)ICall_enterCriticalSection;
  *icallCmnRomLeaveCSPtr  = (uint32)ICall_leaveCriticalSection;

  // initialize RAM pointer to R2F Flash JT for Common ROM code
  *r2fCmnRomPtr = (uint32)R2F_Flash_JT_CommonROM;

  return;
}

/*******************************************************************************
 */

