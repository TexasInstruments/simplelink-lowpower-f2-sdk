/******************************************************************************

 @file  hal_types.h

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2006-2025, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

#ifndef _HAL_TYPES_H
#define _HAL_TYPES_H

#include <stdint.h>

/* ------------------------------------------------------------------------------------------------
 *                                               Types
 * ------------------------------------------------------------------------------------------------
 */
typedef signed   char   int8;
typedef unsigned char   uint8;

typedef signed   short  int16;
typedef unsigned short  uint16;

typedef signed   long   int32;
typedef unsigned long   uint32;

typedef uint32          halDataAlign_t;

#ifndef __cplusplus
#define bool            _Bool
#endif

/* ------------------------------------------------------------------------------------------------
 *                                        Compiler Macros
 * ------------------------------------------------------------------------------------------------
 */
/* ----------- IAR Compiler ----------- */
#ifdef __IAR_SYSTEMS_ICC__
#define ASM_NOP    asm("NOP")
#ifdef USE_DMM
#define NO_INIT    __no_init
#endif

/* ----------- KEIL Compiler ----------- */
#elif defined __KEIL__
#define ASM_NOP   __nop()

/* ----------- CCS Compiler ----------- */
#elif defined __TI_COMPILER_VERSION || defined __TI_COMPILER_VERSION__
#define ASM_NOP    asm(" NOP")
#ifdef USE_DMM
#define NO_INIT    __attribute__((noinit))
#endif

/* ----------- GNU & TI-CLANG Compiler ----------- */
#elif defined(__GNUC__) || defined(__clang__)
#define ASM_NOP __asm__ __volatile__ ("nop")

/* ---------- MSVC compiler ---------- */
#elif _MSC_VER
#define ASM_NOP __asm NOP

/* ----------- Unrecognized Compiler ----------- */
#else
#error "ERROR: Unknown compiler."
#endif


/* ------------------------------------------------------------------------------------------------
 *                                        Standard Defines
 * ------------------------------------------------------------------------------------------------
 */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef UTRUE
#define UTRUE TRUE
#endif

#ifndef UFALSE
#define UFALSE FALSE
#endif

#ifndef USUCCESS
#define USUCCESS 0U
#endif

#ifndef UFAILURE
#define UFAILURE 1U
#endif

#ifndef NULL
#define NULL 0L
#endif


/* ------------------------------------------------------------------------------------------------
 *                                       Memory Attributes
 * ------------------------------------------------------------------------------------------------
 */

#if defined (__IAR_SYSTEMS_ICC__)
#define XDATA
#define CODE
#define DATA_ALIGN(x)               _Pragma data_alignment=(x)
#define ALIGNED
#define PACKED                      __packed
#define PACKED_STRUCT               PACKED struct
#define PACKED_TYPEDEF_STRUCT       PACKED typedef struct
#define PACKED_TYPEDEF_CONST_STRUCT PACKED typedef const struct
#define PACKED_TYPEDEF_UNION        PACKED typedef union
#ifdef USE_DMM
#define PACKED_ALIGNED                  PACKED
#define PACKED_ALIGNED_TYPEDEF_STRUCT   PACKED_TYPEDEF_STRUCT
#endif

#elif defined __TI_COMPILER_VERSION || defined __TI_COMPILER_VERSION__ || defined __clang__
#define XDATA
#define CODE
#define DATA
#define NEARFUNC
#if defined (__clang__)
#define ALIGNED                     __attribute__((aligned(4)))
#else
#define ALIGNED
#endif
#define PACKED                      __attribute__((__packed__))
#define PACKED_STRUCT               struct PACKED
#define PACKED_TYPEDEF_STRUCT       typedef struct PACKED
#define PACKED_TYPEDEF_CONST_STRUCT typedef const struct PACKED
#define PACKED_TYPEDEF_UNION        typedef union PACKED
#ifdef USE_DMM
#define PACKED_ALIGNED                      __attribute__((packed,aligned(4)))
#define PACKED_ALIGNED_TYPEDEF_STRUCT       typedef struct PACKED_ALIGNED
#endif

#elif defined (__GNUC__)
#define XDATA
#define CODE
#define DATA
#define PACKED                      __attribute__((__packed__))
#define PACKED_TYPEDEF_STRUCT       typedef struct PACKED
#define PACKED_STRUCT                       struct PACKED
#define PACKED_TYPEDEF_CONST_STRUCT         typedef const struct PACKED
#define PACKED_TYPEDEF_UNION                typedef union PACKED
#ifdef USE_DMM
#define PACKED_ALIGNED                      __attribute__((packed,aligned(4)))
#define PACKED_ALIGNED_TYPEDEF_STRUCT       typedef struct PACKED_ALIGNED
#endif
#endif

/**************************************************************************************************
 */
#endif
