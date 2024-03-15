/*
 * Copyright (c) 2023 Texas Instruments Incorporated - http://www.ti.com
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
 *  ======== LogSinkUART.c ========
 */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

#include <ti/log/Log.h>
#include <ti/log/LogSinkUART.h>

#include <ti/drivers/dpl/TimestampP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/UART2.h>
#include <ti/drivers/utils/RingBuf.h>

#define LogSinkUART_RESET_FRAME (0xBBBBBBBB)

/* Each log packet is build with fields that are 4 bytes long */
#define LogSinkUART_BYTES_PER_FIELD (4)

/* A log_printf packet will always have a metadata pointer and a timestamp field
 * followed by a variable amount of arguments. If no arguments are added, the
 * packet will have 2 fields.
 */
#define LogSinkUART_PRINTF_MIN_FIELDS (2)

/* A log_printf packet can have up to 8 arguments. Therefore, the maximum fields
 * a packet can have is 10.
 */
#define LogSinkUART_PRINTF_MAX_FIELDS (10)

/* A log_buf packet will always have a metadata pointer, timestamp and size
 * field, followed by a buffer of variable size. Therefore, the minimum fixed
 * fields a packet can have is 3.
 */
#define LogSinkUART_BUF_MIN_FIELDS (3)

/* An overflow packet consists on a single metadata pointer field that occupies
 * 4 bytes.
 */
#define LogSinkUART_OVERFLOW_PACKET_SIZE (4)

/* Mask to change a metadata pointer from 0x9... to 0x8... */
#define LogSinkUART_OVERFLOW_MASK (0xEFFFFFFF)

extern const uint_least8_t LogSinkUART_count;

/*
 *  =========== LogSinkUART_storePacket ==========
 *  Helper function to store a log packet into an intermediate ring buffer. To
 *  make the function thread-safe it should be called from a context where HWI
 *  is disabled.
 */
static void LogSinkUART_storePacket(RingBuf_Handle ringObject, unsigned char *packet, size_t packetLength)
{
    int linearSpace;                    /* Number of unsigned chars available in linear memory of ring buffer */
    unsigned char *dstAddr;             /* Pointer reference to the next chunk of linear memory available */
    size_t bytesWritten = 0;            /* Number of bytes written in the ring buffer */
    size_t writeCount   = packetLength; /* Number of bytes left to write in the ring buffer */

    /* Store packet in a Ring Buffer */
    do
    {
        /* Get the number of contiguous bytes we can copy to the ring
         * buffer and the location where we can start the copy into the
         * ring buffer.
         */
        linearSpace = RingBuf_putPointer(ringObject, &dstAddr);
        if (linearSpace > writeCount)
        {
            linearSpace = writeCount;
        }

        memcpy(dstAddr, packet + bytesWritten, linearSpace);

        /* Update the ring buffer state with the number of bytes copied */
        RingBuf_putAdvance(ringObject, linearSpace);

        writeCount -= linearSpace;
        bytesWritten += linearSpace;
    } while ((linearSpace > 0) && (writeCount > 0));
}

/*
 *  ======== LogSinkUART_flush ========
 */
void LogSinkUART_flush(void)
{
    /* Loop through all the UART2 LogSinks and flush each ring buffer */
    for (size_t i = 0; i < LogSinkUART_count; i++)
    {
        uint32_t key;
        LogSinkUART_Config *config = (LogSinkUART_Config *)&LogSinkUART_config[i];
        LogSinkUART_Object *object = config->object;

        size_t bytesWritten;    /* Number of bytes written to the UART */
        int available;          /* Number of available linear bytes in ring buffer */
        unsigned char *srcAddr; /* Address in ring buffer where data can be read from */
        size_t readCount;       /* Number of bytes left to read */

        key = HwiP_disable();

        /* If there is no data to flush go to the next ring buffer */
        readCount = RingBuf_getCount(&object->ringObj);
        if (readCount == 0)
        {
            HwiP_restore(key);
            continue;
        }

        /* Read data from the ring buffer and put as much as possible on the UART */
        available = RingBuf_getPointer(&object->ringObj, &srcAddr);
        UART2_write(object->uartHandle, srcAddr, available, &bytesWritten);
        RingBuf_getConsume(&object->ringObj, bytesWritten);

        HwiP_restore(key);
    }
}

/*
 *  ======== LogSinkUART_init ========
 */
void LogSinkUART_init(uint_least8_t index)
{
    uint32_t key;
    UART2_Params uartParams;
    uint32_t LogSinkUARTResetSequence[2] = {LogSinkUART_RESET_FRAME, TimestampP_nativeFormat32.value};

    LogSinkUART_Config *config         = (LogSinkUART_Config *)&LogSinkUART_config[index];
    LogSinkUART_Object *object         = config->object;
    LogSinkUART_HWAttrs const *hwAttrs = config->hwAttrs;

    /* Disable interrupts */
    key = HwiP_disable();

    /* Construct ring buffer for intermediate storage */
    RingBuf_construct(&object->ringObj, hwAttrs->bufPtr, hwAttrs->bufSize);

    /* Setup and open UART2 */
    UART2_Params_init(&uartParams);

    uartParams.writeMode  = UART2_Mode_NONBLOCKING;
    uartParams.baudRate   = hwAttrs->baudRate;
    uartParams.parityType = hwAttrs->parity;

    object->uartHandle = UART2_open(hwAttrs->uartIndex, &uartParams);

    if (object->uartHandle == NULL)
    {
        /* UART2_open() failed */
        while (1) {}
    }

    /* Send the reset sequence */
    UART2_write(object->uartHandle, (void *)LogSinkUARTResetSequence, sizeof(LogSinkUARTResetSequence), NULL);

    /* enable interrupts */
    HwiP_restore(key);
}

/*
 *  ======== LogSinkUART_printf ========
 */
void ti_log_LogSinkUART_printf(const Log_Module *handle, uint32_t header, uint32_t index, uint32_t numArgs, ...)
{
    uintptr_t key;
    va_list argptr;
    uint32_t packet[LogSinkUART_PRINTF_MAX_FIELDS];

    LogSinkUART_Handle inst    = (LogSinkUART_Handle)handle->sinkConfig;
    LogSinkUART_Config *config = (LogSinkUART_Config *)&LogSinkUART_config[inst->index];
    LogSinkUART_Object *object = config->object;

    size_t packetSize = (LogSinkUART_PRINTF_MIN_FIELDS + numArgs) * LogSinkUART_BYTES_PER_FIELD;

    /* Get faithful timestamp and ensure that we check
     * if we have space for this packet.
     */
    key = HwiP_disable();

    packet[1] = TimestampP_getNative32();

    /* Check if the ring buffer is full */
    if (RingBuf_isFull(&object->ringObj))
    {
        HwiP_restore(key);
        return;
    }

    /* Assuming that the ring buffer is not full, we check if we
     * have space for the current packet and the overflow packet.
     *
     * If there is space, we proceed normally, and we ensure that
     * if there were not space for the next message, at least there
     * would be space for the overflow message.
     *
     * If there is not enough space, we put an overflow packet into
     * the ring buffer.
     */
    if (RingBuf_space(&object->ringObj) >= packetSize + LogSinkUART_OVERFLOW_PACKET_SIZE)
    {
        /* Construct packet to be sent over UART */
        packet[0] = header;

        va_start(argptr, numArgs);
        for (uint32_t i = 0; i < numArgs; i++)
        {
            packet[LogSinkUART_PRINTF_MIN_FIELDS + i] = va_arg(argptr, uintptr_t);
        }
        va_end(argptr);
    }
    else
    {
        /* Construct overflow packet to be sent over UART.
         * This packet is the header of the original printf
         * but we mask it so it points to an invalid region.
         * This can then be detected by the host tool. Information
         * about which printf it is is conserved.
         */
        packet[0]  = header & LogSinkUART_OVERFLOW_MASK;
        packetSize = LogSinkUART_OVERFLOW_PACKET_SIZE;
    }

    /* Store packet in intermediate storage */
    LogSinkUART_storePacket(&object->ringObj, (unsigned char *)packet, packetSize);

    HwiP_restore(key);
}

/*
 *  ======== LogSinkUART_buf ========
 */
void ti_log_LogSinkUART_buf(const Log_Module *handle, uint32_t header, uint32_t index, uint8_t *data, size_t size)
{
    uintptr_t key;
    uint32_t packet[LogSinkUART_BUF_MIN_FIELDS];

    LogSinkUART_Handle inst    = (LogSinkUART_Handle)handle->sinkConfig;
    LogSinkUART_Config *config = (LogSinkUART_Config *)&LogSinkUART_config[inst->index];
    LogSinkUART_Object *object = config->object;

    size_t packetSize = LogSinkUART_BUF_MIN_FIELDS * LogSinkUART_BYTES_PER_FIELD;

    /* Get faithful timestamp and ensure that we check
     * if we have space for this packet.
     */
    key = HwiP_disable();

    packet[1] = TimestampP_getNative32();

    /* Check if the ring buffer is full */
    if (RingBuf_isFull(&object->ringObj))
    {
        HwiP_restore(key);
        return;
    }

    /* Assuming that the ring buffer is not full, we check if we
     * have space for the current packet and the overflow packet.
     *
     * If there is space, we proceed normally, and we ensure that
     * if there were not space for the next message, at least there
     * would be space for the overflow message.
     *
     * If there is not enough space, we put an overflow packet into
     * the ring buffer.
     */
    if (RingBuf_space(&object->ringObj) >= packetSize + size + LogSinkUART_OVERFLOW_PACKET_SIZE)
    {
        /* Construct and store packet to be sent over UART */
        packet[0] = header;
        packet[2] = size;
        LogSinkUART_storePacket(&object->ringObj, (unsigned char *)packet, packetSize);
        LogSinkUART_storePacket(&object->ringObj, data, size);
    }
    else
    {
        /* Construct overflow packet to be sent over UART.
         * This packet is the header of the original buffer
         * but we mask it so it points to an invalid region.
         * This can then be detected by the host tool. Information
         * about which buffer it is is conserved.
         */
        packet[0] = header & LogSinkUART_OVERFLOW_MASK;
        LogSinkUART_storePacket(&object->ringObj, (unsigned char *)packet, LogSinkUART_OVERFLOW_PACKET_SIZE);
    }

    /* enable interrupts */
    HwiP_restore(key);
}

/*
 *  ======== LogSinkUART_finalize ========
 */
void LogSinkUART_finalize(uint_least8_t index)
{
    LogSinkUART_Config *config = (LogSinkUART_Config *)&LogSinkUART_config[index];
    LogSinkUART_Object *object = config->object;

    /* Close the UART peripheral making sure that there are no ongoing writes*/
    UART2_writeCancel(object->uartHandle);
    UART2_close(object->uartHandle);
}
