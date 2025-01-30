/*
 * Copyright (c) 2019-2024, Texas Instruments Incorporated
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
/*
 *  ======== UART2.c ========
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/UART2.h>
#include <ti/drivers/uart2/UART2Support.h>
#include <ti/log/Log.h>

extern const UART2_Config UART2_config[];
extern const uint_least8_t UART2_count;

/* Macro for weak definition of the UART2 Log module */
Log_MODULE_DEFINE_WEAK(LogModule_UART2, {0});

/* Default UART parameters structure */
const UART2_Params UART2_defaultParams = {
    UART2_Mode_BLOCKING,          /* readMode */
    UART2_Mode_BLOCKING,          /* writeMode */
    NULL,                         /* readCallback */
    NULL,                         /* writeCallback */
    NULL,                         /* eventCallback */
    0,                            /* eventMask */
    UART2_ReadReturnMode_PARTIAL, /* readReturnMode */
    115200,                       /* baudRate */
    UART2_DataLen_8,              /* dataLength */
    UART2_StopBits_1,             /* stopBits */
    UART2_Parity_NONE,            /* parityType */
    NULL                          /* userArg */
};

/* Functions used internally */
int_fast16_t UART2_writeTimeoutBlocking(UART2_Handle handle,
                                        const void *buffer,
                                        size_t size,
                                        size_t *bytesWritten,
                                        uint32_t timeout);

int_fast16_t UART2_writeTimeoutNonblocking(UART2_Handle handle,
                                           const void *buffer,
                                           size_t size,
                                           size_t *bytesWritten,
                                           uint32_t timeout);

static int_fast16_t UART2_writePolling(UART2_Handle handle, const void *buffer, size_t size, size_t *bytesWritten);

/*
 *  ======== UART2_getRxCount ========
 */
size_t __attribute__((weak)) UART2_getRxCount(UART2_Handle handle)
{
    UART2_Object *object = handle->object;
    uintptr_t key;
    int count = 0;

    key = HwiP_disable();
    if (object->state.opened)
    {
        /* Stop and start the DMA transaction. This will update the count in the ring buffer */
        UART2Support_dmaStopRx(handle);
        UART2Support_dmaStartRx(handle);
        count = RingBuf_getCount(&object->rxBuffer);
    }
    HwiP_restore(key);

    return count;
}

/*
 *  ======== UART2_Params_init ========
 */
void UART2_Params_init(UART2_Params *params)
{
    *params = UART2_defaultParams;
}

/*
 *  ======== UART2_read ========
 */
int_fast16_t __attribute__((weak)) UART2_read(UART2_Handle handle, void *buffer, size_t size, size_t *bytesRead)
{
    UART2_Object *object = handle->object;
    int status           = UART2_STATUS_SUCCESS;

    Log_printf(LogModule_UART2,
               Log_INFO,
               "UART2_read: Start reading %d byte(s) to buffer address 0x%x",
               size,
               buffer);

    if ((object->state.readMode == UART2_Mode_BLOCKING) && (object->state.readReturnMode == UART2_ReadReturnMode_FULL))
    {
        status = UART2_readFull(handle, buffer, size, bytesRead);
    }
    else
    {
        status = UART2_readTimeout(handle, buffer, size, bytesRead, UART2_WAIT_FOREVER);
    }

    return status;
}

/*
 *  ======== UART2_readFull ========
 */
int_fast16_t __attribute__((weak)) UART2_readFull(UART2_Handle handle, void *buffer, size_t size, size_t *bytesRead)
{
    unsigned char *buf = (unsigned char *)buffer;
    int status         = UART2_STATUS_SUCCESS;
    size_t totalRead   = 0;
    size_t sizeTemp    = size;
    size_t nRead;

    while (size > 0)
    {
        status = UART2_readTimeout(handle, buf + totalRead, size - totalRead, &nRead, UART2_WAIT_FOREVER);

        totalRead += nRead;
        size -= nRead;

        if (status != UART2_STATUS_SUCCESS)
        {
            break;
        }
    }

    if (bytesRead != NULL)
    {
        *bytesRead = totalRead;
    }

    if (sizeTemp == 0)
    {
        /* Reading 0 bytes is not valid */
        status = UART2_STATUS_EINVALID;
    }

    return status;
}

/*
 *  ======== UART2_readTimeout ========
 */
int_fast16_t UART2_readTimeout(UART2_Handle handle, void *buffer, size_t size, size_t *bytesRead, uint32_t timeout)
{
    UART2_Object *object = handle->object;
    int status           = UART2_STATUS_SUCCESS;
    uintptr_t key;          /* Key returned by HwiP_disable, used to restore interrupts */
    int available;          /* Number of available bytes in ring buffer */
    unsigned char *srcAddr; /* Address in ring buffer where data can be read from */
    void *readBufCopy;      /* Temporarily copy readBuf before clearing it, to pass copy to callback */

    if (bytesRead != NULL)
    {
        *bytesRead = 0;
    }

    key = HwiP_disable();

    if (!object->state.opened)
    {
        /* Instance not yet opened */
        HwiP_restore(key);
        return UART2_STATUS_ENOTOPEN;
    }

    if (object->readInUse)
    {
        /* Another read is ongoing */
        HwiP_restore(key);
        return UART2_STATUS_EINUSE;
    }

    if (size == 0)
    {
        /* Reading 0 bytes is not valid */
        HwiP_restore(key);
        return UART2_STATUS_EINVALID;
    }

    /* The driver is now free to start a read transaction. Even though there is no active "user read" ongoing,
     * the ring buffer could be receiving data in the background. Stop potentially ongoing transfers before proceeding.
     */
    UART2Support_dmaStopRx(handle);

    /* Update driver state variables */
    object->readInUse          = true;
    object->readBuf            = (unsigned char *)buffer;
    object->readSize           = size;
    object->readCount          = size; /* Number remaining to be read */
    object->bytesRead          = 0;    /* Number of bytes read */
    object->rxStatus           = 0;    /* Clear receive errors */
    object->state.rxCancelled  = false;
    object->state.readTimedOut = false;

    /* Enable RX. If RX has already been enabled, this function call does nothing */
    UART2_rxEnable(handle);

    /* Start RX. Depending on the number of bytes in the ring buffer, this will either start a DMA transaction
     * straight into the user buffer, or continue reading into the ring buffer (if it contains enough data for this
     * read)
     */
    UART2Support_dmaStartRx(handle);

    /* Read data from the ring buffer */
    do
    {
        available = RingBuf_getPointer(&object->rxBuffer, &srcAddr);

        if (available > object->readCount)
        {
            available = object->readCount;
        }
        memcpy((unsigned char *)buffer + object->bytesRead, srcAddr, available);

        Log_printf(LogModule_UART2, Log_VERBOSE,
                   "UART2_readTimeout: Read %d byte(s) from the ring buffer.",
                   available);
        RingBuf_getConsume(&object->rxBuffer, available);

        object->readCount -= available;
        object->bytesRead += available;

    } while ((available > 0) && (object->readCount > 0));

    /* If we are in nonblocking mode, the read operation is done. Update state variables */
    if (object->state.readMode == UART2_Mode_NONBLOCKING)
    {
        object->readCount = 0;
        object->readBuf   = NULL;
        object->readInUse = false;
    }
    /* If there are no more bytes to read, the read operation is complete */
    if (object->readCount == 0)
    {
        /* Update driver state variables */
        object->readInUse = false;
        /* Set readBuf to NULL, but first make a copy to pass to the callback. We cannot set it to NULL after
         * the callback in case another read was issued from the callback.
         */
        readBufCopy       = object->readBuf;
        object->readBuf   = NULL;

        if (object->state.readMode == UART2_Mode_CALLBACK)
        {
            /* Safely call the callback function in case UART2_read()
             * is called from within the callback, to avoid recursion.
             */
            object->state.readCallbackPending = true;
            HwiP_restore(key);

            /* If we're not inside the read callback function... */
            if (object->state.inReadCallback == false)
            {
                while (object->state.readCallbackPending)
                {
                    key                               = HwiP_disable();
                    object->state.readCallbackPending = false;
                    object->state.inReadCallback      = true;
                    HwiP_restore(key);

                    object->readCallback(handle, readBufCopy, object->bytesRead, object->userArg, UART2_STATUS_SUCCESS);

                    object->state.inReadCallback = false;
                }
            }
            key = HwiP_disable();
        }
    }

    HwiP_restore(key);

    if (object->state.readMode == UART2_Mode_BLOCKING)
    {
        /* If the driver still has bytes to read */
        if (object->readCount > 0)
        {
            /* Wait for more data, with given timeout */
            if (SemaphoreP_pend(&object->readSem, timeout) != SemaphoreP_OK)
            {
                /* The semaphore-pend returned with a timeout */
                object->state.readTimedOut = true;
            }

            key = HwiP_disable();

            /* Stop RX temporarily*/
            UART2Support_dmaStopRx(handle);

            /* Update state variables */
            object->readCount = 0;
            object->readBuf   = NULL;
            object->readInUse = false;

            /* In case RX was cancelled or timed out, start a new RX. This will start a transaction into the ring buffer
             */
            UART2Support_dmaStartRx(handle);

            if (object->state.readTimedOut)
            {
                status = UART2_STATUS_ETIMEOUT;
            }

            if (object->state.rxCancelled)
            {
                status = UART2_STATUS_ECANCELLED;
            }

            HwiP_restore(key);
        }
        else
        {
            /* Flush read-semaphore */
            SemaphoreP_pend(&object->readSem, SemaphoreP_NO_WAIT);
        }
    }

    key = HwiP_disable();

    /* Set the number of bytes read, but not in callback mode. The number of bytes read here will be 0 in that case,
     * and the correct number will be passed to the callback
     */
    if ((object->state.readMode != UART2_Mode_CALLBACK) && (bytesRead != NULL))
    {
        *bytesRead = object->bytesRead;
    }

    HwiP_restore(key);

    return status;
}

/*
 *  ======== UART2_readCancel ========
 */
void __attribute__((weak)) UART2_readCancel(UART2_Handle handle)
{
    UART2_Object *object = handle->object;
    uintptr_t key;

    if ((object->state.readMode != UART2_Mode_BLOCKING) && (object->state.readMode != UART2_Mode_CALLBACK))
    {
        return;
    }

    key = HwiP_disable();

    if (object->readInUse == false)
    {
        HwiP_restore(key);
        return;
    }

    object->readInUse = false;
    object->readCount = 0;

    if (object->state.rxCancelled == false)
    {
        object->state.rxCancelled = true;
        SemaphoreP_post(&object->readSem);

        if (object->state.readMode == UART2_Mode_CALLBACK)
        {
            object->readCallback(handle, object->readBuf, object->bytesRead, object->userArg, UART2_STATUS_ECANCELLED);
        }
    }

    HwiP_restore(key);
}

/*
 *  ======== UART2_rxDisable ========
 */
void __attribute__((weak)) UART2_rxDisable(UART2_Handle handle)
{
    UART2_Object *object         = handle->object;
    UART2_HWAttrs const *hwAttrs = handle->hwAttrs;

    uintptr_t key = HwiP_disable();
    if (object->state.rxEnabled)
    {
        Log_printf(LogModule_UART2,
               Log_INFO,
               "UART2_rxDisable: Disable RX");

        UART2Support_dmaStopRx(handle);
        UART2Support_disableRx(hwAttrs);

        /* Release standby-constraint, but not flash-constraint */
        UART2Support_powerRelConstraint(handle, false);

        object->state.rxEnabled = false;
    }
    HwiP_restore(key);
}

/*
 *  ======== UART2_rxEnable ========
 */
void __attribute__((weak)) UART2_rxEnable(UART2_Handle handle)
{
    UART2_Object *object = handle->object;

    uintptr_t key = HwiP_disable();
    if (!object->state.rxEnabled)
    {
        object->state.rxEnabled = true;

        /* Enable registered event callbacks/interrupts and receive */
        UART2Support_enableInts(handle);
        UART2Support_enableRx(handle->hwAttrs);

        /* Start a DMA RX transaction */
        UART2Support_dmaStartRx(handle);

        /* Set standby constraint to guarantee transaction, but not flash constraint */
        UART2Support_powerSetConstraint(handle, false);
    }
    HwiP_restore(key);
}

/*
 *  ======== UART2_write ========
 */
int_fast16_t __attribute__((weak))
UART2_write(UART2_Handle handle, const void *buffer, size_t size, size_t *bytesWritten)
{
    Log_printf(LogModule_UART2,
               Log_INFO,
               "UART2_write: Start writing %d byte(s) from buffer address 0x%x",
               size,
               buffer);

    return UART2_writeTimeout(handle, buffer, size, bytesWritten, UART2_WAIT_FOREVER);
}

/*
 *  ======== UART2_writeTimeout ========
 */
int_fast16_t __attribute__((weak))
UART2_writeTimeout(UART2_Handle handle, const void *buffer, size_t size, size_t *bytesWritten, uint32_t timeout)
{
    UART2_Object *object = handle->object;

    if (object->state.writeMode == UART2_Mode_NONBLOCKING)
    {
        return UART2_writeTimeoutNonblocking(handle, buffer, size, bytesWritten, timeout);
    }
    else
    {
        return UART2_writeTimeoutBlocking(handle, buffer, size, bytesWritten, timeout);
    }
}

int_fast16_t UART2_writePolling(UART2_Handle handle, const void *buffer, size_t size, size_t *bytesWritten)
{
    UART2_Object *object         = handle->object;
    UART2_HWAttrs const *hwAttrs = handle->hwAttrs;
    uint32_t writeCount;

    if (object->eventMask & UART2_EVENT_TX_BEGIN)
    {
        Log_printf(LogModule_UART2,
               Log_INFO,
               "UART2_writePolling: Entering event callback with event = 0x%x and user argument = 0x%x",
               UART2_EVENT_TX_BEGIN,
               object->userArg);

        object->eventCallback(handle, UART2_EVENT_TX_BEGIN, 0, object->userArg);
    }

    while (size)
    {
        writeCount = UART2Support_sendData(hwAttrs, size, (unsigned char *)buffer + object->bytesWritten);
        object->bytesWritten += writeCount;
        size -= writeCount;
    }
    /* Wait for data to be fully shifted out of TX shift register. This is
     * required so that the device does not transition to low-power mode
     * prematurely.
     */
    while (!UART2Support_txDone(hwAttrs)) {}

    UART2Support_disableTx(hwAttrs);
    if (object->eventMask & UART2_EVENT_TX_FINISHED)
    {
        Log_printf(LogModule_UART2,
               Log_INFO,
               "UART2_writePolling: Entering event callback with event = 0x%x and user argument = 0x%x",
               UART2_EVENT_TX_FINISHED,
               object->userArg);

        object->eventCallback(handle, UART2_EVENT_TX_FINISHED, 0, object->userArg);
    }

    object->writeInUse = false;

    /* Set output argument, if supplied */
    if (bytesWritten != NULL)
    {
        *bytesWritten = object->bytesWritten;
    }

    return UART2_STATUS_SUCCESS;
}

/*
 *  ======== UART2_writeTimeout ========
 */
int_fast16_t UART2_writeTimeoutBlocking(UART2_Handle handle,
                                        const void *buffer,
                                        size_t size,
                                        size_t *bytesWritten,
                                        uint32_t timeout)
{
    UART2_Object *object         = handle->object;
    UART2_HWAttrs const *hwAttrs = handle->hwAttrs;
    uintptr_t key;
    bool globalInterruptsEnabled;

    /* Set output argument, if supplied */
    if (bytesWritten != NULL)
    {
        *bytesWritten = 0;
    }

    /* Return error if size is 0 */
    if (size == 0)
    {
        return UART2_STATUS_EINVALID;
    }

    /* Save state of global interrupts before entering critical section */
    globalInterruptsEnabled = HwiP_interruptsEnabled();

    /* Enter critical section while checking and modifying state variables */
    key = HwiP_disable();

    if (!object->state.opened)
    {
        /* Instance not yet opened */
        HwiP_restore(key);
        return UART2_STATUS_ENOTOPEN;
    }

    if (object->writeInUse)
    {
        /* An active write call is already in progress */
        HwiP_restore(key);
        return UART2_STATUS_EINUSE;
    }

    /* Update state variables */
    object->writeInUse          = true;
    object->writeBuf            = buffer;
    object->writeSize           = size;
    object->writeCount          = size;
    object->bytesWritten        = 0;
    object->txStatus            = UART2_STATUS_SUCCESS;
    object->state.writeTimedOut = false;
    object->state.txCancelled   = false;

    /* Flush TX semaphore */
    SemaphoreP_pend(&object->writeSem, SemaphoreP_NO_WAIT);

    /* Enable TX - interrupts must be disabled */
    UART2Support_enableTx(hwAttrs);

    if ((globalInterruptsEnabled == false) && (object->state.writeMode == UART2_Mode_BLOCKING))
    {
        /* RTOS not started yet, called from main(), use polling mode */
        HwiP_restore(key);
        return UART2_writePolling(handle, buffer, size, bytesWritten);
    }

    /* Start TX transaction */
    UART2Support_dmaStartTx(handle);

    HwiP_restore(key);

    /* If blocking mode, wait until transfer is complete */
    if (object->state.writeMode == UART2_Mode_BLOCKING)
    {
        /* Wait until semaphore is posted by either
         * - EOT interrupt is received
         * - Write is cancelled
         * - SemaphoreP_pend times out
         */
        if (SemaphoreP_pend(&object->writeSem, timeout) != SemaphoreP_OK)
        {
            object->state.writeTimedOut = true;
        }

        /* If write was cancelled or timed out, stop TX transaction */
        UART2Support_dmaStopTx(handle);

        object->writeInUse = false;
    }

    /* Set output argument, if supplied */
    if (bytesWritten != NULL)
    {
        *bytesWritten = object->bytesWritten;
    }

    if (object->state.txCancelled)
    {
        object->state.txCancelled = false;
        return UART2_STATUS_ECANCELLED;
    }

    if (object->state.writeTimedOut)
    {
        return UART2_STATUS_ETIMEOUT;
    }

    return UART2_STATUS_SUCCESS;
}

/*
 *  ======== UART2_writeTimeout ========
 */
int_fast16_t UART2_writeTimeoutNonblocking(UART2_Handle handle,
                                           const void *buffer,
                                           size_t size,
                                           size_t *bytesWritten,
                                           uint32_t timeout)
{
    UART2_Object *object         = handle->object;
    UART2_HWAttrs const *hwAttrs = handle->hwAttrs;
    int_fast16_t status          = UART2_STATUS_SUCCESS;
    uintptr_t key;
    unsigned char *dstAddr;
    int space;

    /* Set output argument, if supplied */
    if (bytesWritten != NULL)
    {
        *bytesWritten = 0;
    }

    /* Return error if size is 0 */
    if (size == 0)
    {
        return UART2_STATUS_EINVALID;
    }

    key = HwiP_disable();

    if (!object->state.opened)
    {
        /* Instance not yet opened */
        HwiP_restore(key);
        return UART2_STATUS_ENOTOPEN;
    }

    if (object->writeInUse)
    {
        /* An active write call is already in progress */
        HwiP_restore(key);
        return UART2_STATUS_EINUSE;
    }

    /* Update state variables */
    object->writeInUse          = true;
    object->writeBuf            = buffer;
    object->writeSize           = size;
    object->writeCount          = size;
    object->bytesWritten        = 0;
    object->txStatus            = UART2_STATUS_SUCCESS;
    object->state.writeTimedOut = false;
    object->state.txCancelled   = false;

    /* Enable TX - interrupts must be disabled */
    UART2Support_enableTx(hwAttrs);

    HwiP_restore(key);

    /* Copy as much data as we can to the ring buffer */
    do
    {
        /* Get the number of bytes we can copy to the ring buffer
         * and the location where we can start the copy into the
         * ring buffer.
         */
        key = HwiP_disable();

        space = RingBuf_putPointer(&object->txBuffer, &dstAddr);

        if (space > object->writeCount)
        {
            space = object->writeCount;
        }
        memcpy(dstAddr, (unsigned char *)buffer + object->bytesWritten, space);

        /* Update the ring buffer state with the number of bytes copied */
        Log_printf(LogModule_UART2, Log_VERBOSE,
                   "UART2_writeTimeoutNonblocking: Write %d byte(s) to the ring buffer.",
                   space);
        RingBuf_putAdvance(&object->txBuffer, space);

        object->writeCount -= space;
        object->bytesWritten += space;

        HwiP_restore(key);
    } while ((space > 0) && (object->writeCount > 0));

    key = HwiP_disable();

    /* If anything was written to the ring buffer, enable and start TX */
    if (object->bytesWritten > 0)
    {
        /* Try to start the DMA, it may already be started. */
        UART2Support_enableTx(hwAttrs);
        UART2Support_dmaStartTx(handle);
    }

    /* If there was no space in the ring buffer, set the status to indicate so */
    if (object->bytesWritten == 0)
    {
        status = UART2_STATUS_EAGAIN;
    }

    /* Set output argument, if supplied */
    if (bytesWritten != NULL)
    {
        *bytesWritten = object->bytesWritten;
    }

    object->writeInUse = false;

    HwiP_restore(key);

    return status;
}

/*
 *  ======== UART2_writeCancel ========
 */
void __attribute__((weak)) UART2_writeCancel(UART2_Handle handle)
{
    UART2_Object *object = handle->object;
    uintptr_t key;
    uint32_t bytesRemaining;

    key = HwiP_disable();

    if (!object->state.txCancelled)
    {
        object->state.txCancelled = true;

        /* Stop DMA transaction */
        bytesRemaining = UART2Support_dmaStopTx(handle);

        /* bytesWritten includes bytes commited to DMA transaction. Remove bytes remaining in transaction from count */
        if (object->state.writeMode == UART2_Mode_NONBLOCKING)
        {
            object->bytesWritten -= bytesRemaining;
            RingBuf_flush(&object->txBuffer);
        }

        if (object->writeInUse)
        {
            /* If in blocking mode, wake up the blocked write task */
            if (object->state.writeMode == UART2_Mode_BLOCKING)
            {
                SemaphoreP_post(&object->writeSem);
            }

            /* Invoke callback if in callback mode */
            if (object->state.writeMode == UART2_Mode_CALLBACK)
            {
                object->writeInUse = false;
                object->writeCount = 0;
                object->writeCallback(handle,
                                      (void *)object->writeBuf,
                                      object->bytesWritten,
                                      object->userArg,
                                      UART2_STATUS_ECANCELLED);
            }
        }
    }

    if (object->state.txEnabled)
    {
        /* If the UART peripheral is actively transmitting, wait for it to clear the TX FIFO */
        while (!UART2Support_txDone(handle->hwAttrs)) {}

        /* Disable TX in the peripheral */
        UART2Support_disableTx(handle->hwAttrs);
        object->state.txEnabled = false;
        /* Disable standby constraint and flash constraint */
        UART2Support_powerRelConstraint(handle, true);
    }

    HwiP_restore(key);
}
