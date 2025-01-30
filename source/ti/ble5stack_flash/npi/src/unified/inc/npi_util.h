/******************************************************************************

 @file  npi_util.h

 @brief NPI Utilities

 Group: WCS, LPC, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2015-2025, Texas Instruments Incorporated
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
#ifndef NPI_UTIL_H
#define NPI_UTIL_H

#ifdef __cplusplus
extern "C"
{
#endif

// ****************************************************************************
// includes
// ****************************************************************************

#ifdef USE_ICALL
#include "icall.h"
#else
#include <stdlib.h>
#include "hal_assert.h"
#endif //USE_ICALL

#include "util.h"
#include <ti/drivers/dpl/TaskP.h>
#ifdef USE_RCL
#include <drivers/dpl/HwiP.h>
#define Hwi_disable()  HwiP_disable()
#define Hwi_restore(a) HwiP_restore(a)
#include <drivers/dpl/SwiP.h>
#define Swi_disable()  SwiP_disable()
#define Swi_restore(a) SwiP_restore(a)
#include <drivers/dpl/TaskP.h>
#else
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Swi.h>
#endif


// ****************************************************************************
// defines
// ****************************************************************************

// ****************************************************************************
// typedefs
// ****************************************************************************

//! \brief Keys used to enter and exit critical sections
typedef struct _npiCSKey_t
{
    uint_least16_t hwikey;
    uint_least16_t taskkey;
} _npiCSKey_t;

//*****************************************************************************
// globals
//*****************************************************************************

//*****************************************************************************
// function prototypes
//*****************************************************************************

// -----------------------------------------------------------------------------
//! \brief    Memory allocation for NPI
//!
//! \param    size - number of bytes to be allocated.
//!
//! \return   pointer to the allocated memory.
// -----------------------------------------------------------------------------
extern uint8_t *NPIUtil_malloc(uint16_t size);

// -----------------------------------------------------------------------------
//! \brief    Memory free for NPI
//!
//! \param    pMsg - pointer to the message to be freed.
//!
//! \return   None
// -----------------------------------------------------------------------------
extern void NPIUtil_free(uint8_t *pMsg);

// -----------------------------------------------------------------------------
//! \brief      Critical section entrance. Disables Tasks and HWI
//!
//! \return     _npiCSKey_t 	CS Key used to later exit CS
// -----------------------------------------------------------------------------
extern _npiCSKey_t NPIUtil_EnterCS(void);

// -----------------------------------------------------------------------------
//! \brief      Critical section exit. Enables Tasks and HWI
//!
//! \param		key 	key obtained with corresponding call to EnterCS()
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPIUtil_ExitCS(_npiCSKey_t key);


// -----------------------------------------------------------------------------
//! \brief      Create a POSIX task.
//              In case the stackaddr is not provided, allocate it on the heap.
//!
//! \param    newthread     threadId
//! \param    startroutine  Pointer to the task entry function
//! \param    priority
//! \param    stackaddr
//! \param    stacksize
//!
//! \return   void
// -----------------------------------------------------------------------------
extern int NPIUtil_createPTask(pthread_t *newthread, void *(*startroutine)(void *), int priority, void *stackaddr, size_t stacksize);

// -----------------------------------------------------------------------------
//! \brief      Create a POSIX queue.
//!
//! \param    queueHandle     queue handle
//! \param    mq_name         name
//! \param    mq_size         number of elements for the queue
//! \param    mq_msgsize      size of queue element
//! \param    mq_flags        flags
//!
//! \return   void
// -----------------------------------------------------------------------------
extern int NPIUtil_createPQueue(mqd_t *queueHandle, char *mq_name, uint32_t mq_size, uint32_t mq_msgsize, uint32_t mq_flags);

#ifdef __cplusplus
}
#endif

#endif /* NPI_UTIL_H */
