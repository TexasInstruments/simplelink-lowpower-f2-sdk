/*
 * Copyright (c) 2023-2024, Texas Instruments Incorporated
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
 *  ======== CAN.c ========
 */
#include <stdbool.h>
#include <stdint.h>

#include <ti/drivers/CAN.h>
#include <ti/drivers/dpl/HwiP.h>

#include <third_party/mcan/MCAN.h>

/* Externally defined device-specific functions */
extern int_fast16_t CAN_initDevice(uint_least8_t index, CAN_Params *params);
extern int_fast16_t CAN_deinitDevice(void);

/* Default CAN parameters structure */
static const CAN_Params CAN_defaultParams = {
    .msgRamConfig = NULL,
    .bitTiming    = NULL,
    .tsPrescaler  = 24U,
    .eventCbk     = NULL,
    .eventMask    = 0U,
    .userArg      = NULL,
};

/*
 *  ======== CAN_Params_init ========
 */
void CAN_Params_init(CAN_Params *params)
{
    *params = CAN_defaultParams;
}

/*
 *  ======== CAN_init ========
 */
__attribute__((weak)) void CAN_init(void)
{
    /* Do nothing */
}

/*
 *  ======== CAN_open ========
 */
CAN_Handle CAN_open(uint_least8_t index, CAN_Params *params)
{
    const CAN_Config *config   = &CAN_config[index];
    CAN_Handle handle          = NULL;
    const CAN_HWAttrs *hwAttrs = config->hwAttrs;
    CAN_Object *object         = config->object;
    uintptr_t interruptKey     = HwiP_disable();

    if (object->isOpen)
    {
        HwiP_restore(interruptKey);

        return NULL;
    }

    object->isOpen = true;

    HwiP_restore(interruptKey);

    object->eventCbk = params->eventCbk;
    if (params->eventCbk == NULL)
    {
        /* Set the event mask to 0 if the callback is NULL to simplify checks */
        object->eventMask = 0U;
    }
    else
    {
        object->eventMask = params->eventMask;
        object->userArg   = params->userArg;
    }

    StructRingBuf_construct(&object->rxStructRingBuf,
                            hwAttrs->rxRingBufPtr,
                            hwAttrs->rxRingBufSize,
                            sizeof(MCAN_RxBufElement));
    StructRingBuf_construct(&object->txStructRingBuf,
                            hwAttrs->txRingBufPtr,
                            hwAttrs->txRingBufSize,
                            sizeof(MCAN_TxBufElement));

    if (CAN_initDevice(index, params) == CAN_STATUS_SUCCESS)
    {
        handle = (CAN_Handle)config;
    }
    else
    {
        object->isOpen = false;
    }

    return handle;
}

/*
 *  ======== CAN_close ========
 */
__attribute__((weak)) void CAN_close(CAN_Handle handle)
{
    CAN_Object *object = handle->object;

    object->isOpen = false;
}

/*
 *  ======== CAN_read ========
 */
int_fast16_t CAN_read(CAN_Handle handle, MCAN_RxBufElement *buf)
{
    CAN_Object *object  = handle->object;
    int_fast16_t status = CAN_STATUS_NO_RX_MSG_AVAIL;

    if (StructRingBuf_get(&object->rxStructRingBuf, buf) >= 0)
    {
        status = CAN_STATUS_SUCCESS;
    }

    return status;
}

/*
 *  ======== CAN_write ========
 */
int_fast16_t CAN_write(CAN_Handle handle, const MCAN_TxBufElement *elem)
{
    CAN_Object *object             = handle->object;
    int_fast16_t status            = CAN_STATUS_ERROR;
    MCAN_TxFifoQStatus fifoQStatus = {0};
    uintptr_t hwiKey;

    if (object->txFifoQNum != 0U)
    {
        /* Disable interrupts as the ISR may call CAN_write */
        hwiKey = HwiP_disable();

        MCAN_getTxFifoQStatus(&fifoQStatus);

        if (fifoQStatus.fifoFull == 0U)
        {
            MCAN_writeTxMsg(fifoQStatus.putIdx, elem);

            MCAN_setTxBufAddReq(fifoQStatus.putIdx);
            status = CAN_STATUS_SUCCESS;
        }
        else
        {
            if (StructRingBuf_put(&object->txStructRingBuf, elem) < 0)
            {
                status = CAN_STATUS_TX_BUF_FULL;
            }
            else
            {
                status = CAN_STATUS_SUCCESS;
            }
        }

        HwiP_restore(hwiKey);
    }

    return status;
}

/*
 *  ======== CAN_writeBuffer ========
 */
int_fast16_t CAN_writeBuffer(CAN_Handle handle, uint32_t bufIdx, const MCAN_TxBufElement *elem)
{
    CAN_Object *object  = handle->object;
    int_fast16_t status = CAN_STATUS_ERROR;
    uint32_t pendingTx;

    if (bufIdx < object->txBufNum)
    {
        pendingTx = MCAN_getTxBufReqPend();

        if ((((uint32_t)1U << bufIdx) & pendingTx) == 0U)
        {
            MCAN_writeTxMsg(bufIdx, elem);

            MCAN_setTxBufAddReq(bufIdx);

            status = CAN_STATUS_SUCCESS;
        }
    }

    return status;
}

/*
 *  ======== CAN_readTxEvent ========
 */
int_fast16_t CAN_readTxEvent(CAN_Handle handle, CAN_TxEventElement *elem)
{
    CAN_Object *object  = handle->object;
    int_fast16_t status = CAN_STATUS_ERROR;

    if (object->txEventFifoNum != 0U)
    {
        if (MCAN_readTxEventFifo(elem) == MCAN_STATUS_SUCCESS)
        {
            status = CAN_STATUS_SUCCESS;
        }
        else
        {
            status = CAN_STATUS_NO_TX_EVENT_AVAIL;
        }
    }

    return status;
}
