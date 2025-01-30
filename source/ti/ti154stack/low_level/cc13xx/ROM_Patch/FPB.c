/******************************************************************************

 @file  FPB.c

 @brief Flash Patch and Breakpoint.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2009-2025, Texas Instruments Incorporated

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

#include "comdef.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "gpio.h"

/*******************************************************************************
 * EXTERNS
 */

// address of flash jump table based defined in linker configuration table
// Note: This symbol is treated as a variable by the compiler.
extern uint32 FLASH_JT_ADDR;

// Patch Function Externs
// Add external patch functions here and in FPB_AddrTable below.
extern uint8 macCheckPendAddr_PATCH(uint8 pendAddrSpec, uint8 *pAddrList);

/*******************************************************************************
 * PROTOTYPES
 */

#if defined( PATCHED_CODE_WORD_ALIGNED )
void FPB_Relay0( void );
void FPB_Relay1( void );
void FPB_Relay2( void );
void FPB_Relay3( void );
void FPB_Relay4( void );
void FPB_Relay5( void );
void FPB_Relay6( void );
void FPB_Relay7( void );
#endif // PATCHED_CODE_WORD_ALIGNED

/*******************************************************************************
 * MACROS
 */

#define FPB_REMAP_OPCODE_UPPER(index)  *((uint16 *)((uint32)&FPB_OpcodeTable + ((index)*4)))
#define FPB_REMAP_OPCODE_LOWER(index)  *((uint16 *)((uint32)&FPB_OpcodeTable + ((index)*4) + 2))

#define FP_COMPx_REG(index)            HWREG(FPB_BASE + 0x08 + ((index)*4))

/*******************************************************************************
 * CONSTANTS
 */

#define FPB_BASE                       0xE0002000

// Flash Patch and Breakpoint (FPB) Registers
#define FP_CTRL_REG                    HWREG(FPB_BASE + 0x00)
#define FP_REMAP_REG                   HWREG(FPB_BASE + 0x04)
#define FP_COMP0_REG                   HWREG(FPB_BASE + 0x08)
#define FP_COMP1_REG                   HWREG(FPB_BASE + 0x0C)
#define FP_COMP2_REG                   HWREG(FPB_BASE + 0x10)
#define FP_COMP3_REG                   HWREG(FPB_BASE + 0x14)
#define FP_COMP4_REG                   HWREG(FPB_BASE + 0x18)
#define FP_COMP5_REG                   HWREG(FPB_BASE + 0x1C)
#define FP_COMP6_REG                   HWREG(FPB_BASE + 0x20)
#define FP_COMP7_REG                   HWREG(FPB_BASE + 0x24)

#define FPB_ENABLE                     0x00000001
#define FPB_KEY                        0x00000002
#define FPB_CTRL_INIT                  0x00000260

#define FPB_NUM_REMAP_REGS             6
#define FPB_NUM_LITERAL_REGS           2
#define FPB_NUM_COMPARE_ADDRS          (FPB_NUM_REMAP_REGS + FPB_NUM_LITERAL_REGS)

#define UNCOND_BRANCH_OPCODE           0xF400B800
#define IMM21_MASK                     0x001FFFFF
#define IMM10_MASK                     0x03FF0000
#define IMM11_MASK                     0x000007FF

#define BRANCH_TO_SELF_OPCODE          0xF7FFBFFE
#define NOP_OPCODE                     0xBF00
#define SVC_OPCODE(n)                  (0xDF00 | ((n) & 0xFF))

#define EXCSTK_R0                      0
#define EXCSTK_R1                      1
#define EXCSTK_R2                      2
#define EXCSTK_R3                      3
#define EXCSTK_R12                     4
#define EXCSTK_LR                      5
#define EXCSTK_RETURN                  6
#define EXCSTK_XPSR                    7

// Hardcoded address of functions to be patched in ROM as of the April 25, 2013 code freeze.
// Note:  To obtain this address, look in the timac_rom.map file and
//        search for the function's name.
// Note:  Should the ROM be altered and this patch is still in use, the new address
//        for this function must be looked up.
// address of function macCheckPendAddr()
#define MAC_CHECK_PEND_ADDR_ADDRRESS   0x10013c2b // This is the current addr in code freeze.


/*******************************************************************************
 * TYPEDEFS
 */

// user defined sections for RAM, ROM, and Flash tables, functions, and pointers
#pragma section = "FPB_RAM_OPCODE_TBL"
#pragma section = "FPB_ROM_RELAY"
#pragma section = "FPB_ROM_RELAY_TBL"
#pragma section = "FPB_FLS_ADDR_TBL"
#pragma section = "FPB_FLS_ADDR_TBL_PTR"

// Note: For now, all FPB patches will be on function boundary addresses!
// TODO: NEED TO ALLOW EITHER TWO FUNCTIONS FOR CODE ADDRESS COMPARES, OR A
//       DATA ADDRESS AND DATA FOR LITERAL COMPARATOR MATCHES.
typedef void (*funcPtr_t)(void);
typedef struct
{
  funcPtr_t pFunc;      // function that is being patched
  funcPtr_t pFuncPatch; // patch function
} FPB_AddrTable_t;


/*******************************************************************************
 * LOCAL VARIABLES
 */


/*******************************************************************************
 * GLOBAL VARIABLES
 */

//
// FPB Flash Address Table
//
// This is the Flash Patch and Breakpoint (FPB) address table located in Flash.
// There is an entry for each compare, and each entry contains two function
// addresses: the first is the address of the function to patch, and the second
// is the address of the patch function. If the first function is NULL, the
// entry is skipped.
//
// Note: The last two entries in the table are for literal address compares. In
//       this case, the first entry is the address of the data read, and the
//       second entry is the data to replace the patched read. Currently, this
//       code doesn't support this. This address table would need to be updated
//       to support just data addresses, or symbols that resolve to data
//       addresses.
//
// Note: The FPB is also used for setting hardware breakpoints. It is possible
//       to use the FPB for breakpoints and patching as long is care is taken
//       not to collide with the debugger. It is therefore recommended that the
//       FPB Address Table be filled from the bottom up as the IAR debugger
//       appears to use the comparators from the top down (i.e. beginning at
//       comparator zero).
//
#pragma data_alignment=4
const FPB_AddrTable_t FPB_AddrTable[] @ "FPB_FLS_ADDR_TBL" =
  {
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {(funcPtr_t)MAC_CHECK_PEND_ADDR_ADDRRESS, (funcPtr_t)&macCheckPendAddr_PATCH},
    {NULL, NULL},  // literal
    {NULL, NULL}   // literal
  };

#pragma data_alignment=4
const uint32 FPB_AddrTablePtr @ "FPB_FLS_ADDR_TBL_PTR" = (uint32)FPB_AddrTable;


//
// FPB ROM Relay Table
//
// This is the Flash Patch and Breakpoint (FPB) relay table located in ROM.
// There is one relay routine for each compare. The relay routines are used to
// transfer control from the ROM region to another memory region (usually Flash)
// to execute the patch routine. The relay routines get control when the opcode
// table uses PC relative branch opcodes, which are only supported when all
// patch functions are guaranteed to be word aligned.
//
#if defined( PATCHED_CODE_WORD_ALIGNED )
#pragma data_alignment=4
const funcPtr_t FPB_RelayTable[] @ "FPB_ROM_RELAY_TBL" =
  {
    FPB_Relay0,
    FPB_Relay1,
    FPB_Relay2,
    FPB_Relay3,
    FPB_Relay4,
    FPB_Relay5
  };
#endif // PATCHED_CODE_WORD_ALIGNED


//
// FPB RAM Opcode Table
//
// This is the Flash Patch and Breakpoint (FPB) opcode table located in RAM.
// Each 32 bit location contains either a single 32-bit Thumb-2 opcode, or a
// pair of Thumb/Thumb-2 opcodes for each of the available address comparators.
//
// Note: It is extremely important that this table be aligned on a memory
//       address boundary that corresponds to the number of comparators. As
//       the FPB has six address and two literal comparators, this table must
//       be located on a 32 byte boundary.
//
// Note: It is extremely important to understand that the opcode will execute
//       relative to the comparator address, not this table's address.
//
// Note: If you wish to view the Opcode table as disassembly, you'll need to
//       comment out this declaration, and replace with a function of the
//       same name with inline assembly of equivalent size.
//
#pragma data_alignment=4
uint32 FPB_OpcodeTable[ FPB_NUM_COMPARE_ADDRS ] @ "FPB_RAM_OPCODE_TBL";


/*******************************************************************************
 * @fn          FPB Initialization
 *
 * @brief       This routine sets up the Flash Patch and Breakpoint (FPB)
 *              hardware registers and the FPB Opcode Table located in RAM.
 *              The remap register holds the RAM location of patch opcodes.
 *              The comparator registers contain the address of the code that
 *              is to be patched. The control register enables FPB.
 *
 *              One of two possible solutions are provided. The first uses
 *              a PC relative branch while the second uses a Supervision Call.
 *              The PC relative branch opcodes stored in RAM must be built
 *              based on the address of the patch routine and the location of
 *              the FPB Compare Relay function. The addresses of the routine
 *              to be patched, and the patch routine, are located in the FPB
 *              Address Table. Please note that this approach will only work if
 *              the address of any patched routine is always on a word boundary.
 *              This is due to an ARM hardware limitation of the Comparator
 *              register. The second approach must build the opcodes in RAM
 *              based on whether the patch address is on a halfword or word
 *              boundary (and thus, supports both). A Supervison call is then
 *              used to determine the correct comparator, and to transfer
 *              control to the correct patch routine.
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
void FPB_Init( void )
{
  uint8  i;
#if defined( PATCHED_CODE_WORD_ALIGNED )
  int32  delta;
  uint32 opcode;
#endif // PATCHED_CODE_WORD_ALIGNED

  // set the re-map address in the FPB block
  FP_REMAP_REG = (uint32)&FPB_OpcodeTable;

  // disable flash patch
  FP_CTRL_REG = (FPB_CTRL_INIT | FPB_KEY) & ~FPB_ENABLE;

#if defined( PATCHED_CODE_WORD_ALIGNED )
  // build opcodes for patch routines
  for (i=0; i<FPB_NUM_REMAP_REGS; i++)
  {
    if ( FPB_AddrTable[i].pFunc != NULL )
    {
      // find the delta for the branch
      delta = (int32)( (uint32)FPB_RelayTable[i] -
                       (uint32)FPB_AddrTable[i].pFunc );

      // adjust for post PC operation
      // Note: The branch is relative to the PC address after the opcode.
      delta = ((delta - 4) / 2) & IMM21_MASK;

      // create unconditional branch opcode with correct immediate values
      // Note: For additional details on opcode encoding, please see the
      //       ARMv7-M Architecture Reference Manual.
      // Note: The values of J1 and J2 should be set to !(S ^ I1) and !(S ^ I2),
      //       but since our delta is always negative, and never beyond our
      //       relatively small ROM, the values of J1/J2 are always the same
      //       as I1/I2, i.e. 1.
      opcode = UNCOND_BRANCH_OPCODE        |
               ((delta << 5) & IMM10_MASK) |
               (delta & IMM11_MASK);

      // store opcode in FPB RAM jump table
      // Note: The ARM Cortex M3 is a Thumb-2 architecture, which means it
      //       always fetches 16-bit instructions. If the instruction is a
      //       32-bit Thumb instruction, then it will fetch an additional
      //       16-bit halfword to complete the opcode. As such, the most
      //       significant halfword of the 32-bit Thumb instruction must be
      //       placed in the first halfword. If stored as a 32-bit word, then
      //       the halfwords would have to first be swapped to ensure the
      //       most significant halfword is fetched first in little endian
      //       order.
      FPB_REMAP_OPCODE_UPPER(i) = (uint16)(opcode >> 16);
      FPB_REMAP_OPCODE_LOWER(i) = (uint16)(opcode & 0xFFFF);

      // update FPB Comparator register with the address of the patched function
      FP_COMPx_REG(i) = (uint32)FPB_AddrTable[i].pFunc | FPB_ENABLE;
    }
    else // nothing asssigned for this comparator
    {
      // so put a spinlock instruction to trap the system as this is an error
      FPB_REMAP_OPCODE_UPPER(i) = (uint16)(BRANCH_TO_SELF_OPCODE >> 16);
      FPB_REMAP_OPCODE_LOWER(i) = (uint16)(BRANCH_TO_SELF_OPCODE & 0xFFFF);
    }
  }

#else // !PATCHED_CODE_WORD_ALIGNED

  // build opcodes for patch routines
  for (i=0; i<FPB_NUM_REMAP_REGS; i++)
  {
    if ( FPB_AddrTable[i].pFunc != NULL )
    {
      // check if the address of the patch routine is on a halfword
      if ( (uint32)FPB_AddrTable[i].pFunc & 0x2 )
      {
        // yes, so read the last opcode of the previous routine and use that
        // as the first spoofed opcode
        FPB_REMAP_OPCODE_UPPER(i) = *((uint16 *)((uint32)FPB_AddrTable[i].pFunc & ~0x3));

        // and set the SVC opcode as the second spoofed opcode
        FPB_REMAP_OPCODE_LOWER(i) = SVC_OPCODE(i);
      }
      else // patched routine is on a word boundary
      {
        // so set the SVC opcde first
        FPB_REMAP_OPCODE_UPPER(i) = SVC_OPCODE(i);

        // followed by NOP
        FPB_REMAP_OPCODE_LOWER(i) = NOP_OPCODE;
      }

      // update FPB Comparator register with the address of the patched function
      FP_COMPx_REG(i) = (uint32)FPB_AddrTable[i].pFunc | FPB_ENABLE;
    }
    else // nothing assigned for this comparator
    {
      // so put a spinlock instruction to trap the system as this is an error
      FPB_REMAP_OPCODE_UPPER(i) = (uint16)(BRANCH_TO_SELF_OPCODE >> 16);
      FPB_REMAP_OPCODE_LOWER(i) = (uint16)(BRANCH_TO_SELF_OPCODE & 0xFFFF);
    }
  }
#endif // PATCHED_CODE_WORD_ALIGNED

  // enable flash patch hardware
  FP_CTRL_REG = FPB_CTRL_INIT | FPB_KEY | FPB_ENABLE;

  return;
}


#if !defined( PATCHED_CODE_WORD_ALIGNED )
/*******************************************************************************
 * @fn          FPB_ProcessSVC
 *
 * @brief       This routine is the SVC handler for processing Flash Patch
 *              and Breakpoint (FPB) remaps. It is called from the SVC_ISR.
 *              It first determines if an SVC in code executed, or if the SVC
 *              instruction was spoofed by FPB. In the former case, the SVC
 *              number is obtained from the opcode, and is returned. In the
 *              latter case, the correct patch routine is determined based on
 *              the SVC Exception stack Return Address. If located, the
 *              Return Address on the stack is updated with the patch routine,
 *              otherwise the Return Address is set to the address of the
 *              routine that called the routine to be patched and an SVC
 *              number of zero is returned.
 *
 *              Note:
 *              SP[0] = R0
 *              SP[1] = R1
 *              SP[2] = R2:
 *              SP[3] = R3:
 *              SP[4] = R12:
 *              SP[5] = LR: Return address to the function that called the
 *                          function that got patched.
 *              SP[6] = SVC return address.
 *
 * input parameters
 *
 * @param       stackPointer - The SVC stack pointer.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      SVC Number 0-255, where 0-7 are reserved for FPB.
 */
uint8 FPB_ProcessSVC( uint32 *stackPointer )
{
  uint8  i;
  uint32 patchedFunc;
  uint16 patchedOpcode;

  // get the patched function address
  // Note: The SVC is a Thumb instruction, so the SVCall Exception stack
  //       Return Address contains the next address on a even address boundary.
  //       We only need subtract one to get the address of the patched routine.
  patchedFunc = (stackPointer[EXCSTK_RETURN] - 1);

  // get the first Thumb opcode at the patched function address
  // Note: Since the Thumb address always sets bit 0 to one, subtract one to
  //       get the correct memory address.
  patchedOpcode = *(uint16 *)(patchedFunc-1);

  // check if the SVC was directly called or was spoofed by FPB
  if ( (patchedOpcode & 0xFF00) == SVC_OPCODE(0) )
  {
    // we have a real SVC in code, so not an FPB operation; get the SVC number
    return( (uint8)(patchedOpcode & 0x00FF) );
  }
  else // SVC was spoofed by FPB
  {
    // search patch table for function address match
    for (i=0; i<FPB_NUM_REMAP_REGS; i++)
    {
      // check for a patched function address match
      if ( patchedFunc == (uint32)FPB_AddrTable[i].pFunc )
      {
        // found, so set Return Address to the address of the patched routine
        // Note: The Return Address always has bit 0 set to zero.
        stackPointer[EXCSTK_RETURN] = (uint32)(FPB_AddrTable[i].pFuncPatch)-1;

        // return implied SVC number based on the FPB comparator used
        return( i );
      }
    }

    // no match found, so set return address to the address that called the
    // patch routine, rendering the SVCall Handler moot
    stackPointer[EXCSTK_RETURN] = stackPointer[EXCSTK_LR];

    // indicate invalid SVC number
    // Note: As FPB handling doesn't use the SVC number directly, it isn't a
    //       problem to return value even though no corresponding patch routine
    //       was found.
    return( 0 );
  }
}
#endif // !PATCHED_CODE_WORD_ALIGNED


#if defined( PATCHED_CODE_WORD_ALIGNED )
/*******************************************************************************
 * @fn          FPB Relay 0
 *
 * @brief       This routine is the Flash Patch and Breakpoint (FPB) relay. It
 *              is located in ROM, and is called by the opcode located in the
 *              FPB Opcode Table when a patched address in ROM matches the
 *              address stored in the FPB Comparator register in hardware. Its
 *              purpose is to save the Patch Address Table offset and transfer
 *              control to a common handler.
 *
 *              Note: While R0-R3 and R12 are specified by ARM as scratch
 *                    registers, R0-R3 can also be used for passing parameters,
 *                    and thus may be in use. Thus, at this point, only R12 is
 *                    available.
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
void FPB_Relay0( void ) @ "FPB_ROM_RELAY"
{
  // save the patch address table offset
  // Note: The Patch Address Table contains the address of the routine to be
  //       patched and the address of the patch routine; thus, eight bytes per
  //       comparator. As the address of the patch routine is what's required,
  //       the table offset is: (N * 8) + 4.
  asm("MOV R12, #(0*8)+4");

  // and go to common handler
  asm("B FPB_RelayCommon");
}


/*******************************************************************************
 * @fn          FPB Relay 1
 *
 * @brief       This routine is the Flash Patch and Breakpoint (FPB) relay. It
 *              is located in ROM, and is called by the opcode located in the
 *              FPB Opcode Table when a patched address in ROM matches the
 *              address stored in the FPB Comparator register in hardware. Its
 *              purpose is to save the Patch Address Table offset and transfer
 *              control to a common handler.
 *
 *              Note: While R0-R3 and R12 are specified by ARM as scratch
 *                    registers, R0-R3 can also be used for passing parameters,
 *                    and thus may be in use. Thus, at this point, only R12 is
 *                    available.
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
void FPB_Relay1( void ) @ "FPB_ROM_RELAY"
{
  // save the patch address table offset
  // Note: The Patch Address Table contains the address of the routine to be
  //       patched and the address of the patch routine; thus, eight bytes per
  //       comparator. As the address of the patch routine is what's required,
  //       the table offset is: (N * 8) + 4.
  asm("MOV R12, #(1*8)+4");

  // and go to common handler
  asm("B FPB_RelayCommon");
}


/*******************************************************************************
 * @fn          FPB Relay 2
 *
 * @brief       This routine is the Flash Patch and Breakpoint (FPB) relay. It
 *              is located in ROM, and is called by the opcode located in the
 *              FPB Opcode Table when a patched address in ROM matches the
 *              address stored in the FPB Comparator register in hardware. Its
 *              purpose is to save the Patch Address Table offset and transfer
 *              control to a common handler.
 *
 *              Note: While R0-R3 and R12 are specified by ARM as scratch
 *                    registers, R0-R3 can also be used for passing parameters,
 *                    and thus may be in use. Thus, at this point, only R12 is
 *                    available.
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
void FPB_Relay2( void ) @ "FPB_ROM_RELAY"
{
  // save the patch address table offset
  // Note: The Patch Address Table contains the address of the routine to be
  //       patched and the address of the patch routine; thus, eight bytes per
  //       comparator. As the address of the patch routine is what's required,
  //       the table offset is: (N * 8) + 4.
  asm("MOV R12, #(2*8)+4");

  // and go to common handler
  asm("B FPB_RelayCommon");
}


/*******************************************************************************
 * @fn          FPB Relay 3
 *
 * @brief       This routine is the Flash Patch and Breakpoint (FPB) relay. It
 *              is located in ROM, and is called by the opcode located in the
 *              FPB Opcode Table when a patched address in ROM matches the
 *              address stored in the FPB Comparator register in hardware. Its
 *              purpose is to save the Patch Address Table offset and transfer
 *              control to a common handler.
 *
 *              Note: While R0-R3 and R12 are specified by ARM as scratch
 *                    registers, R0-R3 can also be used for passing parameters,
 *                    and thus may be in use. Thus, at this point, only R12 is
 *                    available.
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
void FPB_Relay3( void ) @ "FPB_ROM_RELAY"
{
  // save the patch address table offset
  // Note: The Patch Address Table contains the address of the routine to be
  //       patched and the address of the patch routine; thus, eight bytes per
  //       comparator. As the address of the patch routine is what's required,
  //       the table offset is: (N * 8) + 4.
  asm("MOV R12, #(3*8)+4");

  // and go to common handler
  asm("B FPB_RelayCommon");
}


/*******************************************************************************
 * @fn          FPB Relay 4
 *
 * @brief       This routine is the Flash Patch and Breakpoint (FPB) relay. It
 *              is located in ROM, and is called by the opcode located in the
 *              FPB Opcode Table when a patched address in ROM matches the
 *              address stored in the FPB Comparator register in hardware. Its
 *              purpose is to save the Patch Address Table offset and transfer
 *              control to a common handler.
 *
 *              Note: While R0-R3 and R12 are specified by ARM as scratch
 *                    registers, R0-R3 can also be used for passing parameters,
 *                    and thus may be in use. Thus, at this point, only R12 is
 *                    available.
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
void FPB_Relay4( void ) @ "FPB_ROM_RELAY"
{
  // save the patch address table offset
  // Note: The Patch Address Table contains the address of the routine to be
  //       patched and the address of the patch routine; thus, eight bytes per
  //       comparator. As the address of the patch routine is what's required,
  //       the table offset is: (N * 8) + 4.
  asm("MOV R12, #(4*8)+4");

  // and go to common handler
  asm("B FPB_RelayCommon");
}


/*******************************************************************************
 * @fn          FPB Relay 5
 *
 * @brief       This routine is the Flash Patch and Breakpoint (FPB) relay. It
 *              is located in ROM, and is called by the opcode located in the
 *              FPB Opcode Table when a patched address in ROM matches the
 *              address stored in the FPB Comparator register in hardware. Its
 *              purpose is to save the Patch Address Table offset and transfer
 *              control to a common handler.
 *
 *              Note: While R0-R3 and R12 are specified by ARM as scratch
 *                    registers, R0-R3 can also be used for passing parameters,
 *                    and thus may be in use. Thus, at this point, only R12 is
 *                    available.
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
void FPB_Relay5( void ) @ "FPB_ROM_RELAY"
{
  // save the patch address table offset
  // Note: The Patch Address Table contains the address of the routine to be
  //       patched and the address of the patch routine; thus, eight bytes per
  //       comparator. As the address of the patch routine is what's required,
  //       the table offset is: (N * 8) + 4.
  asm("MOV R12, #(5*8)+4");

  // and go to common handler
  asm("B FPB_RelayCommon");
}


/*******************************************************************************
 * @fn          FPB Relay Common
 *
 * @brief       This routine is the common Flash Patch and Breakpoint (FPB)
 *              relay. It is located in ROM, and is called by one of the FPB
 *              Relay functions (FPB Relay 0 to FPB Relay 7). Its purpose is
 *              to provide a common routine to complete the transfer of control
 *              from the ROM memory region to the patched function in the
 *              Flash or RAM memory region. Other than the transfer, this
 *              routine will appear transparent in the patch process, and
 *              the patched routine's return through LR will pass control
 *              directly to the function that called the patch function.
 *
 *              Note: This solution was originally developed using the
 *                    Monolithic Build method where the addresses of the
 *                    Relay Table and the relay functions are resolved at the
 *                    same time as the Flash Address Table. However, with the
 *                    Multiple Build method, the ROM build would not have the
 *                    Flash Address Table address resolved. This can be solved
 *                    by adding the address of the Flash Address Table to the
 *                    Flash Jump Table. Alternatively, the location of the
 *                    Flash Address Table can be directly conveyed by a linker
 *                    symbol. In either case, this file would have to be
 *                    included in the ROM build and conditional compiling would
 *                    be needed to exclude flash related software in the ROM
 *                    build and ROM related software in the flash build. For
 *                    now, this was not done, so this approach is not supported
 *                    by the Multiple Build method.
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
void FPB_RelayCommon( void ) @ "FPB_ROM_RELAY"
{
  // preserve needed registers
  asm("PUSH {R0, R1}");

  // move patch address table offset to spare register
  asm("MOV R1, R12");

  // load the address of the Flash Address Table
  asm("LDR.W R0, FPB_AddrTablePtr");

  // load the address of the patch function based on the offset
  asm("LDR R12, [R0, R1]");

  // restore preserved registers
  asm("POP {R0, R1}");

  // branch unconditionally to the patch routine
  // Note: The patch routine's normal return through LR will return control
  //       directly to the routine that called the patched routine.
  asm("BX R12");
}
#endif // PATCHED_CODE_WORD_ALIGNED


/*******************************************************************************
 */

