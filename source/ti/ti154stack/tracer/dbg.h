/******************************************************************************

 @file  dbg.h

 @brief Provides data logging

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

#ifndef _DBG_H
#define _DBG_H

#if defined(USE_ITM_DBG)
/**
 *
 * The tracer module enables ITM via a high speed single wire output.
 */
#include "itm.h"
#elif defined(USE_TIRTOS_DBG)
#include <stdint.h>
#include <xdc/runtime/Log.h>
#endif //USE_ITM_DBG


#if defined(USE_TIRTOS_DBG) || defined(USE_ITM_DBG)
#define MAC_DBG_CH  0
#define APP_DBG_CH  1
#define DBGSYS      MAC_DBG_CH
#endif

#if USE_TIRTOS_DBG
/// Emulates DBG_PRINT with no arguments, but sends an ID instead of the format string
#define DBG_PRINT0(ch, str) \
   (void) ch; \
   Log_info0(str);

/// Emulates DBG_PRINT with 1 numeric argument (16-bit)
#define DBG_PRINT1(ch, str, a0) \
        (void) ch; \
        Log_info1(str, a0);

/// Emulates DBG_PRINT with 2 numeric arguments (16-bit)
#define DBG_PRINT2(ch, str, a0, a1) \
        (void) ch; \
        Log_info2(str, a0, a1);

/// Emulates DBG_PRINT with 3 numeric arguments (16-bit)
#define DBG_PRINT3(ch, str, a0, a1, a2) \
        (void) ch; \
        Log_info3(str, a0, a1, a2);

/// Emulates DBG_PRINT with 4 numeric arguments (16-bit)
#define DBG_PRINT4(ch, str, a0, a1, a2, a3) \
        (void) ch; \
        Log_info4(str, a0, a1, a2, a3);

/// Emulates DBG_PRINT with 1 long numeric argument (32-bit)
#define DBG_PRINTL1(ch, str, a0) \
        (void) ch; \
        Log_info1(str, a0);

/// Emulates DBG_PRINT with 2 long numeric arguments (32-bit)
#define DBG_PRINTL2(ch, str, a0, a1) \
        (void) ch; \
        Log_info2(str, a0, a1);

#elif defined(USE_ITM_DBG)
/// Emulates DBG_PRINT with no arguments, but sends an ID instead of the format string
#define DBG_PRINT0(ch, str) \
  {  \
      ITM_putStringBlocking(str, ch); \
      ITM_putNewLineBlocking(ch); \
  }

/// Emulates DBG_PRINT with 1 numeric argument (16-bit)
#define DBG_PRINT1(ch, str, a0) \
    { \
      ITM_putStringBlocking(str, ch); \
      ITM_putStringBlocking(" ,", ch); \
      ITM_putValueBlocking(a0, ch); \
      ITM_putNewLineBlocking(ch); \
    }

/// Emulates DBG_PRINT with 2 numeric arguments (16-bit)
#define DBG_PRINT2(ch, str, a0, a1) \
    { \
      ITM_putStringBlocking(str, ch); \
      ITM_putStringBlocking(", ", ch); \
      ITM_putValueBlocking(a0, ch); \
      ITM_putStringBlocking(", ", ch); \
      ITM_putValueBlocking(a1, ch); \
      ITM_putNewLineBlocking(ch); \
    }

/// Emulates DBG_PRINT with 3 numeric arguments (16-bit)
#define DBG_PRINT3(ch, str, a0, a1, a2) \
    { \
      ITM_putStringBlocking(str, ch); \
      ITM_putStringBlocking(", ", ch); \
      ITM_putValueBlocking(a0, ch); \
      ITM_putStringBlocking(", ", ch); \
      ITM_putValueBlocking(a1, ch); \
      ITM_putStringBlocking(", ", ch); \
      ITM_putValueBlocking(a2, ch); \
      ITM_putNewLineBlocking(ch); \
    }

/// Emulates DBG_PRINT with 4 numeric arguments (16-bit)
#define DBG_PRINT4(ch, str, a0, a1, a2, a3) \
    { \
      ITM_putStringBlocking(str, ch); \
      ITM_putStringBlocking(", ", ch); \
      ITM_putValueBlocking(a0, ch); \
      ITM_putStringBlocking(", ", ch); \
      ITM_putValueBlocking(a1, ch); \
      ITM_putStringBlocking(", ", ch); \
      ITM_putValueBlocking(a2, ch); \
      ITM_putNewLineBlocking(ch); \
    }

/// Emulates DBG_PRINT with 1 long numeric argument (32-bit)
#define DBG_PRINTL1(ch, str, a0) \
    { \
      ITM_putStringBlocking(str, ch); \
      ITM_putStringBlocking(", ", ch); \
      ITM_putValueBlocking(a0, ch); \
      ITM_putNewLineBlocking(ch); \
    }

/// Emulates DBG_PRINT with 2 long numeric arguments (32-bit)
#define DBG_PRINTL2(ch, str, a0, a1) \
    { \
      ITM_putStringBlocking(str, ch); \
      ITM_putStringBlocking(", ", ch); \
      ITM_putValueBlocking(a0, ch); \
      ITM_putStringBlocking(", ", ch); \
      ITM_putValueBlocking(a1, ch); \
      ITM_putNewLineBlocking(ch); \
    }


#elif defined(DEBUG_SW_TRACE)
/** \addtogroup module_dbg Debug Functionality (dbg)
 * \ingroup module_cc26_rfcore_fw
 *
 *
 *
 * \section section_dbg_responsibilities Responsibilities
 *
 * The tracer module implements three CPU debug channels that share the same serial interface.
 *
 * With debug IDs 0-1 reserved for special purposes, there can be 254 DBG_PRINTn() statements per
 * debug channel. Having more DBG_PRINTn() statements will result in compilation error (undefined
 * symbol).
 *
 *
 * \section section_dbg_usage Usage
 *
 * \subsection section_dbg_enabling Enabling and Disabling
 * To use the debug functionality in a component, the following code must be added in file header:
 * \code
 * #define DBG_ENABLE
 * #include <dbgid.h>
 * \endcode
 *
 * The "dbgid.h" file is autogenerated by the Makefile and put in the src directory of the active
 * project. It has been generated on the basis of scanning all .c files and finding any usage of
 * the DBG_PRINTn macros.
 *
 * The debug functionality can be disabled altogether by defining \c DBG_GLOBAL_DISABLE at compilation.
 *
 *
 * @{
 */
#include "rfctrc_regs.h"
#include <stdint.h>

//-------------------------------------------------------------------------------------------------------
/// \name Internal Functionality
//@{

#if defined(DBG_ENABLE) && !defined(DBG_GLOBAL_DISABLE)
#define DBG_MACRO(x) do{ x }while(0)
#else
#define DBG_MACRO(x)
#endif

// Functions used to implement DBG_PRINTn
extern void dbgPrintf(uint32_t x, uint32_t y, uint32_t z);
extern void dbgPrintf2(uint32_t x, uint32_t y);
extern void dbgPrintf0(uint32_t x);

//@}
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
/// \anchor debugid_defs \name Reserved Debug IDs
//@{

#define DBGID_SYSTICK           1   ///< Debug ID reserved for system tick trace packets
#define DBGID_ENDSIM            2   ///< Debug ID reserved for end of simulation directive. Takes one parameter.


//@}

//-------------------------------------------------------------------------------------------------------

/// \anchor debug_preemp_levels \name Preemption Levels for DBG_PRINT()
//@{

#define DBGCH1      1   ///< Trace channel #1
#define DBGCH2      2   ///< Trace channel #2
#define DBGCH3      3   ///< Trace channel #3

#define DBGCPE      1   ///< Use trace channel #1 in CPE
#define DBGTOPSM    2   ///< Use trace channel #2 in TOPsm
#define DBGSYS      3   ///< Use trace channel #3in System CPU

//@}
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
/// \name Available Operations
//@{

/// Helper macro
#define _XDBGFILESTR(x) #x
/// Helper macro
#define _XXDBG(x,y) DBGID_ ## x ## _ ## y
/// Helper macro
#define _XDBG(x,y) _XXDBG(x,y)

/// Emulates printf() with no arguments, but sends an ID instead of the format string
#define DBG_PRINT0(pre, str) \
   DBG_MACRO( \
      dbgPrintf0(((pre)<<16) | (_XDBG(_DBGFILE,__LINE__)<<8) | 0); \
   )

/// Emulates printf() with 1 numeric argument (16-bit)
#define DBG_PRINT1(pre, str, a0) \
   DBG_MACRO( \
      dbgPrintf2(((pre)<<16) | (_XDBG(_DBGFILE,__LINE__)<<8) | 1, (((uint32_t)(a0))&0xFFFF)); \
   )

/// Emulates printf() with 2 numeric arguments (16-bit)
#define DBG_PRINT2(pre, str, a0, a1) \
   DBG_MACRO( \
      dbgPrintf2(((pre)<<16) | (_XDBG(_DBGFILE,__LINE__)<<8) | 2, ((((uint32_t)(a0))&0xFFFF) | (((uint32_t)(a1))&0xFFFF)<< 16)); \
   )

/// Emulates printf() with 3 numeric arguments (16-bit)
#define DBG_PRINT3(pre, str, a0, a1, a2) \
   DBG_MACRO( \
      dbgPrintf(((pre)<<16) | (_XDBG(_DBGFILE,__LINE__)<<8) | 3, (((uint32_t)(a0))&0xFFFF) | ((((uint32_t)(a1))&0xFFFF)<<16), ((uint32_t)(a2)&0xFFFF)); \
   )

/// Emulates printf() with 4 numeric arguments (16-bit)
#define DBG_PRINT4(pre, str, a0, a1, a2, a3) \
   DBG_MACRO( \
      dbgPrintf(((pre)<<16) | (_XDBG(_DBGFILE,__LINE__)<<8) | 4, (((uint32_t)(a0))&0xFFFF) | ((((uint32_t)(a1))&0xFFFF)<<16), (((uint32_t)(a2))&0xFFFF) | ((((uint32_t)(a3))&0xFFFF)<<16)); \
   )

/// Emulates printf() with 1 long numeric argument (32-bit)
#define DBG_PRINTL1(pre, str, a0) \
   DBG_MACRO( \
      dbgPrintf2(((pre)<<16) | (_XDBG(_DBGFILE,__LINE__)<<8) | 2, (((uint32_t)(a0)))); \
   )

/// Emulates printf() with 2 long numeric arguments (32-bit)
#define DBG_PRINTL2(pre, str, a0, a1) \
   DBG_MACRO( \
      dbgPrintf(((pre)<<16) | (_XDBG(_DBGFILE,__LINE__)<<8) | 4, ((uint32_t)(a0)), ((uint32_t)(a1))); \
   )


/// End of simulation macro
#define DBG_ENDSIM(pre, code) \
   DBG_MACRO( \
      dbgPrintf2(((pre)<<16) | ((DBGID_ENDSIM) << 8) | 1, (((uint32_t)(code))&0xFFFF)); \
   )

/// End of simulation with success
#define DBG_SUCCESS(pre) DBG_ENDSIM(pre, 0)

/// Verifies the given \a condition, and halts further code execution upon a negative result
#define DBG_ASSERT(pre, condition) \
   DBG_MACRO( \
      if (!(condition)) { \
         DBG_PRINT0(pre, #condition ); \
         FATAL_ERROR(); \
      } \
   )


/// Used in the generated file dbgid.h to define preprocessor symbols
#define DBG_DEF(sym, id, pre, nargs, str, fname, lineno) \
   static const uint32_t sym = id;

//@}
//-------------------------------------------------------------------------------------------------------
#else
/// Emulates DBG_PRINT with no arguments, but sends an ID instead of the format string
#define DBG_PRINT0(ch, str)
/// Emulates DBG_PRINT with 1 numeric argument (16-bit)
#define DBG_PRINT1(ch, str, a0)
/// Emulates DBG_PRINT with 2 numeric arguments (16-bit)
#define DBG_PRINT2(ch, str, a0, a1)
/// Emulates DBG_PRINT with 3 numeric arguments (16-bit)
#define DBG_PRINT3(ch, str, a0, a1, a2)
/// Emulates DBG_PRINT with 4 numeric arguments (16-bit)
#define DBG_PRINT4(ch, str, a0, a1, a2, a3)
/// Emulates DBG_PRINT with 1 long numeric argument (32-bit)
#define DBG_PRINTL1(ch, str, a0)
/// Emulates DBG_PRINT with 2 long numeric arguments (32-bit)
#define DBG_PRINTL2(ch, str, a0, a1)

#endif

//@}
//-------------------------------------------------------------------------------------------------------
#endif
//@}
