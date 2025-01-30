/******************************************************************************

 @file  npi_util.c

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

// -----------------------------------------------------------------------------
// includes
// -----------------------------------------------------------------------------

#include "inc/npi_util.h"
#include "inc/npi_data.h"

// -----------------------------------------------------------------------------
// defines
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// typedefs
// -----------------------------------------------------------------------------

/// -----------------------------------------------------------------------------
// globals
/// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// PUBLIC FUNCTIONS
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//! \brief    Memory allocation for NPI
//!
//! \param    size - number of bytes to be allocated.
//!
//! \return   pointer to the allocated memory.
// -----------------------------------------------------------------------------
uint8_t *NPIUtil_malloc(uint16_t size)
{
    uint8_t *pMsg;
#ifdef USE_ICALL
    pMsg = ICall_malloc(size);
#else
    volatile uint32_t keyHwi;
    keyHwi = Hwi_disable();
    pMsg = malloc(size);
    Hwi_restore(keyHwi);
#endif
    return pMsg;
}

// -----------------------------------------------------------------------------
//! \brief    Memory free for NPI
//!
//! \param    pMsg - pointer to the message to be freed.
//!
//! \return   None
// -----------------------------------------------------------------------------
void NPIUtil_free(uint8_t *pMsg)
{
#ifdef USE_ICALL
    ICall_free(pMsg);
#else
    volatile uint32_t keyHwi;
    keyHwi = Hwi_disable();
    free(pMsg);
    Hwi_restore(keyHwi);
#endif
}

// -----------------------------------------------------------------------------
//! \brief      Critical section entrance. Disables Tasks and HWI
//!
//! \return     _npiCSKey_t   CS Key used to later exit CS
// -----------------------------------------------------------------------------
_npiCSKey_t NPIUtil_EnterCS(void)
{
    volatile _npiCSKey_t key;
    key.hwikey =  (uint16_t) Hwi_disable();
    key.taskkey = (uint16_t) TaskP_disableScheduler();
    return key;
}

// -----------------------------------------------------------------------------
//! \brief      Critical section exit. Enables Tasks and HWI
//!
//! \param    key   key obtained with corresponding call to EnterCS()
//!
//! \return   void
// -----------------------------------------------------------------------------
void NPIUtil_ExitCS(_npiCSKey_t key)
{
    TaskP_restoreScheduler(key.taskkey);
    Hwi_restore(key.hwikey);
}

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
int NPIUtil_createPTask(pthread_t *newthread, void *(*startroutine)(void *), int priority, void *stackaddr, size_t stacksize)
{

  int retVal = 0;
  pthread_attr_t param_attribute;
  struct sched_param param;

  // Task Stack was not pre-allocated by user.
  // Allocate it.
  if ((char *)stackaddr == NULL)
  {
    // Allocated space for task stack.
    stackaddr = NPIUtil_malloc(stacksize);
    if ((char *)stackaddr == NULL)
    {
      // Failed to allocate
        return -1 /*ERROR*/;
    }
  }
  retVal =  pthread_attr_init(&param_attribute);
  param.sched_priority = priority;

  retVal |= pthread_attr_setschedparam(&param_attribute, &param);
  retVal |= pthread_attr_setstack(&param_attribute, stackaddr, stacksize);
  retVal |= pthread_attr_setdetachstate(&param_attribute, PTHREAD_CREATE_DETACHED);

  retVal |= pthread_create(newthread,
                        &param_attribute,
                        startroutine,
                        NULL);
  return retVal;
}

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
int NPIUtil_createPQueue(mqd_t *queueHandle, char *mq_name, uint32_t mq_size, uint32_t mq_msgsize, uint32_t mq_flags)
{
  struct mq_attr attr;

  attr.mq_flags =  O_CREAT | O_RDWR | mq_flags;
  attr.mq_curmsgs = 0;
  attr.mq_maxmsg = mq_size;
  attr.mq_msgsize = mq_msgsize;

  /* Create the message queue */
  *queueHandle = mq_open(mq_name, O_CREAT | O_RDWR | mq_flags, 0, &attr);

  return 0;
}
