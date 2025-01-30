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
 *  ======== TCAN455X.c ========
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <ti/drivers/can/TCAN455X.h>
#include <ti/drivers/CAN.h>
#include <ti/drivers/can/common/CANMCAN.h>

#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/utils/StructRingBuf.h>
#include <ti/drivers/utils/Math.h>

#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/dpl/TaskP.h>

#include <third_party/mcan/MCAN.h>
#include <third_party/mcan/inc/TCAN455X_reg.h>

#if (defined(__IAR_SYSTEMS_ICC__) || defined(__TI_COMPILER_VERSION__))
    #include <arm_acle.h>
    #define BSWAP32 __rev
#elif defined(__GNUC__)
    #define BSWAP32 __builtin_bswap32
#else
    #error Unsupported compiler
#endif

#define TCAN455X_RESET_DELAY_US 800U /* Must be at least 700us */

#define TCAN455X_SPI_READ_OPCODE  0x41U
#define TCAN455X_SPI_WRITE_OPCODE 0x61U

/*
 * Counter for number of times Rx Ring buffer was full when there was a Rx
 * message available in Rx FIFO0/1 resulting in a lost message. This can be used
 * to fine tune the size of the Rx ring buffer when the CAN bus is under maximum
 * load.
 */
static uint32_t rxRingBufFullCnt = 0U;

static SemaphoreP_Struct tcanIRQSemaphore;
static SemaphoreP_Struct spiAccessSemaphore;
static TaskP_Params taskParams;
static TaskP_Struct tcanTask;

static MCAN_TxBufElement txElem;
static MCAN_RxBufElement rxElem;

extern const TCAN455X_Config TCAN455X_config;

/* Default device-specific message RAM configuration:
 *  - Each standard filter element occupies 4-bytes.
 *  - Each extended filter element occupies 8-bytes.
 *  - Each Rx/Tx buffer occupies 72-bytes when CAN FD is enabled or 16-bytes
 *    for classic CAN.
 *  - Each Tx Event occupies 8-bytes.
 */
const CAN_MsgRamConfig TCAN455X_defaultMsgRamConfig = {
    .stdFilterNum       = 0U,
    .extFilterNum       = 0U,
    .stdMsgIDFilterList = NULL,
    .extMsgIDFilterList = NULL,

    .rxFifoNum[0U]  = 18U,
    .rxFifoNum[1U]  = 0U,
    .rxBufNum       = 0U,
    .txBufNum       = 0U,
    .txFifoQNum     = 10U,
    .txFifoQMode    = 1U, /* Tx Queue mode */
    .txEventFifoNum = 4U,
};

static SPI_Handle TCAN455X_spiHandle;

/* Forward declarations */
static inline void TCAN455X_assertSPICSN(void);
static inline void TCAN455X_deassertSPICSN(void);
static void TCAN455X_startSPITransfer(uint8_t opcode, uint16_t addr, uint8_t numWords);
static void TCAN455X_doSPITransfer(SPI_Transaction *transaction);
static inline void TCAN455X_endSPITransfer(void);
static void TCAN455X_writeSPI(uint32_t data);
static uint32_t TCAN455X_readSPI(void);
static void TCAN455X_writeReg(uint16_t offset, uint32_t value);
static uint32_t TCAN455X_readReg(uint16_t offset);
static inline void TCAN455X_disableInterrupt(void);
static inline void TCAN455X_enableInterrupt(void);
static bool TCAN455X_isRxStructRingBufFull(CAN_Handle handle);
static void TCAN455X_handleRxFifo(CAN_Handle handle, uint32_t fifoNum);
static void TCAN455X_handleRxBuf(CAN_Handle handle);
static void TCAN455X_irqHandler(uint_least8_t index);
static inline int_fast16_t TCAN455X_initSPI(void);
static void TCAN455X_clearMsgRam(void);
static int_fast16_t TCAN455X_setBitRate(const CAN_Config *config, uint32_t canInputClkFreq);
static void TCAN455X_setInterruptEnable(uint32_t ie);
static void TCAN455X_clearInterrupt(uint32_t mask);
static void TCAN455X_modifyModeReg(uint32_t mask, uint32_t val);
static void TCAN455X_reset(void);
static int_fast16_t TCAN455X_init(const CAN_Config *config,
                                  const CAN_MsgRamConfig *msgRamConfig,
                                  const CAN_BitRateTimingRaw *bitTiming,
                                  uint32_t tsPrescaler);
static void TCAN455X_enableLoopback(bool externalModeEnable);
static inline int_fast16_t TCAN455X_setExtTsPrescaler(uint32_t prescalar);
static void TCAN455X_taskFxn(void *arg);

/* Definitions for extern functions defined in MCAN.h */

/*
 *  ======== MCAN_getMRAMOffset ========
 */
uint32_t MCAN_getMRAMOffset(void)
{
    return TCAN455X_MRAM;
}

/*
 *  ======== MCAN_writeReg ========
 */
void MCAN_writeReg(uint32_t offset, uint32_t value)
{
    uint32_t addr = offset;

    if (addr < TCAN455X_MRAM)
    {
        addr += TCAN455X_MCAN;
    }

    TCAN455X_writeReg((uint16_t)addr, value);
}

/*
 *  ======== MCAN_writeMsgRam ========
 */
void MCAN_writeMsgRam(uint32_t offset, const uint8_t *src, size_t numBytes)
{
    size_t bytesRemaining;
    size_t bytesWritten = 0U;
    size_t numWords     = numBytes >> 2U;
    uint32_t loopCnt;
    uint32_t word;

    if (numBytes != 0U)
    {
        /*
         * Check whether numBytes[1:0] is non-zero which means numBytes is a
         * non-word multiple.
         */
        if ((numBytes << 30U) != 0U)
        {
            /* Additional word write is required for remaining data bytes */
            TCAN455X_startSPITransfer(TCAN455X_SPI_WRITE_OPCODE, (uint16_t)offset, (uint8_t)(numWords + 1U));
        }
        else
        {
            TCAN455X_startSPITransfer(TCAN455X_SPI_WRITE_OPCODE, (uint16_t)offset, (uint8_t)numWords);
        }

        /* Write all the full words of source data */
        for (loopCnt = 0U; loopCnt < numWords; loopCnt++)
        {
            (void)memcpy(&word, &src[bytesWritten], sizeof(word));

            TCAN455X_writeSPI(word);
            bytesWritten += 4U;
        }

        bytesRemaining = numBytes - bytesWritten;

        /* Create a full word out of any remaining source data bytes */
        if (bytesRemaining > 0U)
        {
            word = 0U;
            (void)memcpy(&word, &src[bytesWritten], bytesRemaining);

            TCAN455X_writeSPI(word);
        }

        TCAN455X_endSPITransfer();
    }
}

/*
 *  ======== MCAN_readReg ========
 */
uint32_t MCAN_readReg(uint32_t offset)
{
    uint32_t addr = offset;

    if (addr < TCAN455X_MRAM)
    {
        addr += TCAN455X_MCAN;
    }

    return TCAN455X_readReg((uint16_t)addr);
}

/*
 *  ======== MCAN_readMsgRam ========
 */
void MCAN_readMsgRam(uint8_t *dst, uint32_t offset, size_t numBytes)
{
    size_t numWords = numBytes >> 2U;
    SPI_Transaction xfer;
    uint32_t loopCnt;
    /* Save memory by reusing the Rx element data buffer which is guaranteed to
     * be word-aligned.
     */
    uint32_t *rxBuf = (uint32_t *)&rxElem.data[0];

    /* Verify numBytes does not exceed the size of Rx element data buffer */
    if ((numBytes != 0U) && (numBytes <= sizeof(rxElem.data)))
    {
        /*
         * Check whether numBytes[1:0] is non-zero which means numBytes is a
         * non-word multiple.
         */
        if ((numBytes << 30U) != 0U)
        {
            /* Additional word read is required for remaining data bytes */
            numWords++;
        }

        TCAN455X_startSPITransfer(TCAN455X_SPI_READ_OPCODE, (uint16_t)offset, (uint8_t)numWords);

        xfer.txBuf = NULL;
        xfer.rxBuf = rxBuf;
        xfer.count = numWords << 2U; /* Number of 8-bit frames */

        TCAN455X_doSPITransfer(&xfer);

        for (loopCnt = 0U; loopCnt < numWords; loopCnt++)
        {
            rxBuf[loopCnt] = BSWAP32(rxBuf[loopCnt]);
        }

        (void)memcpy(&dst[0], &rxBuf[0], numBytes);

        TCAN455X_endSPITransfer();
    }
}

/*
 *  ======== TCAN455X_assertSPICSN ========
 */
static inline void TCAN455X_assertSPICSN(void)
{
    GPIO_write(TCAN455X_config.spiCSPin, 0U);
}

/*
 *  ======== TCAN455X_deassertSPICSN ========
 */
static inline void TCAN455X_deassertSPICSN(void)
{
    GPIO_write(TCAN455X_config.spiCSPin, 1U);
}

/*
 *  ======== TCAN455X_doSPITransfer ========
 */
static void TCAN455X_doSPITransfer(SPI_Transaction *transaction)
{
    const CAN_Config *config = &CAN_config[0];
    CAN_Handle handle        = (CAN_Handle)config;
    CAN_Object *object       = config->object;

    if (!SPI_transfer(TCAN455X_spiHandle, transaction) || (transaction->status != SPI_TRANSFER_COMPLETED))
    {
        if ((CAN_EVENT_SPI_XFER_ERROR & object->eventMask) != 0U)
        {
            /* Call the event callback function provided by the application */
            object->eventCbk(handle, CAN_EVENT_SPI_XFER_ERROR, transaction->status, object->userArg);
        }
    }
}

/*
 *  ======== TCAN455X_startSPITransfer ========
 */
static void TCAN455X_startSPITransfer(uint8_t opcode, uint16_t addr, uint8_t numWords)
{
    SPI_Transaction xfer;
    uint8_t hdr[4];

    /* No need to check return value when waiting forever */
    (void)SemaphoreP_pend(&spiAccessSemaphore, SemaphoreP_WAIT_FOREVER);

    TCAN455X_assertSPICSN();

    hdr[0] = opcode;

    /* Set 16-bit address */
    hdr[1] = (uint8_t)(addr >> 8U);
    hdr[2] = (uint8_t)(addr);

    /* Set number of words */
    hdr[3] = numWords;

    xfer.txBuf = &hdr[0];
    xfer.rxBuf = NULL;
    xfer.count = 4U; /* 1 word converted to 8-bit frames */

    TCAN455X_doSPITransfer(&xfer);
}

/*
 *  ======== TCAN455X_endSPITransfer ========
 */
static inline void TCAN455X_endSPITransfer(void)
{
    TCAN455X_deassertSPICSN();

    SemaphoreP_post(&spiAccessSemaphore);
}

/*
 *  ======== TCAN455X_writeSPI ========
 */
static void TCAN455X_writeSPI(uint32_t data)
{
    SPI_Transaction xfer;
    uint32_t txData;

    /* Byte swap */
    txData = BSWAP32(data);

    xfer.txBuf = &txData;
    xfer.rxBuf = NULL;
    xfer.count = sizeof(txData); /* Number of 8-bit frames */

    TCAN455X_doSPITransfer(&xfer);
}

/*
 *  ======== TCAN455X_readSPI ========
 */
static uint32_t TCAN455X_readSPI(void)
{
    SPI_Transaction xfer;
    uint32_t data;

    xfer.txBuf = NULL;
    xfer.rxBuf = &data;
    xfer.count = sizeof(data); /* Number of 8-bit frames */

    TCAN455X_doSPITransfer(&xfer);

    /* Byte swap */
    data = BSWAP32(data);

    return data;
}

/*
 *  ======== TCAN455X_writeReg ========
 */
static void TCAN455X_writeReg(uint16_t offset, uint32_t val)
{
    TCAN455X_startSPITransfer(TCAN455X_SPI_WRITE_OPCODE, offset, 1U);
    TCAN455X_writeSPI(val);
    TCAN455X_endSPITransfer();
}

/*
 *  ======== TCAN455X_readReg ========
 */
static uint32_t TCAN455X_readReg(uint16_t offset)
{
    uint32_t data;

    TCAN455X_startSPITransfer(TCAN455X_SPI_READ_OPCODE, offset, 1U);
    data = TCAN455X_readSPI();
    TCAN455X_endSPITransfer();

    return data;
}

/*
 *  ======== TCAN455X_disableInterrupt ========
 */
static inline void TCAN455X_disableInterrupt(void)
{
    GPIO_disableInt(TCAN455X_config.interruptPin);
}

/*
 *  ======== TCAN455X_enableInterrupt ========
 */
static inline void TCAN455X_enableInterrupt(void)
{
    GPIO_enableInt(TCAN455X_config.interruptPin);
}

/*
 *  ======== TCAN455X_isRxStructRingBufFull ========
 */
static bool TCAN455X_isRxStructRingBufFull(CAN_Handle handle)
{
    CAN_Object *object = (CAN_Object *)handle->object;
    bool isFull        = StructRingBuf_isFull(&object->rxStructRingBuf);

    if (isFull)
    {
        rxRingBufFullCnt++;

        if ((CAN_EVENT_RX_RING_BUFFER_FULL & object->eventMask) != 0U)
        {
            /* Call the event callback function provided by the application */
            object->eventCbk(handle, CAN_EVENT_RX_RING_BUFFER_FULL, rxRingBufFullCnt, object->userArg);
        }
    }

    return isFull;
}

/*
 *  ======== TCAN455X_handleRxFifo ========
 */
static void TCAN455X_handleRxFifo(CAN_Handle handle, uint32_t fifoNum)
{
    CAN_Object *object           = (CAN_Object *)handle->object;
    MCAN_RxFifoStatus fifoStatus = {0};

    MCAN_getRxFifoStatus(fifoNum, &fifoStatus);

    if ((fifoStatus.fillLvl > 0U) && !TCAN455X_isRxStructRingBufFull(handle))
    {
        MCAN_readRxMsg(MCAN_MEM_TYPE_FIFO, fifoNum, &rxElem);
        /* Return value can be ignored since ring buffer is not full */
        (void)StructRingBuf_put(&object->rxStructRingBuf, &rxElem);

        fifoStatus.fillLvl--;

        while ((fifoStatus.fillLvl > 0U) && !TCAN455X_isRxStructRingBufFull(handle))
        {
            MCAN_readRxMsg(MCAN_MEM_TYPE_FIFO, fifoNum, &rxElem);
            /* Return value can be ignored since ring buffer is not full */
            (void)StructRingBuf_put(&object->rxStructRingBuf, &rxElem);

            fifoStatus.fillLvl--;
            fifoStatus.getIdx++;

            /* Check for rollover */
            if (fifoStatus.getIdx >= object->rxFifoNum[fifoNum])
            {
                fifoStatus.getIdx = 0U;
            }
        }
    }

    /* Return value can be ignored since the inputs are known to be valid */
    (void)MCAN_setRxFifoAck(fifoNum, fifoStatus.getIdx);
}

/*
 *  ======== TCAN455X_handleRxBuf ========
 */
static void TCAN455X_handleRxBuf(CAN_Handle handle)
{
    CAN_Object *object                      = (CAN_Object *)handle->object;
    MCAN_RxNewDataStatus clearNewDataStatus = {0U};
    MCAN_RxNewDataStatus newDataStatus;
    uint32_t bufNum;

    MCAN_getNewDataStatus(&newDataStatus);

    /* Check for Rx messages in buffers 0-31 */
    if (newDataStatus.statusLow != 0U)
    {
        for (bufNum = 0U; bufNum < Math_MIN(object->rxBufNum, 32U); bufNum++)
        {
            if ((newDataStatus.statusLow & ((uint32_t)1U << bufNum)) != 0U)
            {
                if (!TCAN455X_isRxStructRingBufFull(handle))
                {
                    MCAN_readRxMsg(MCAN_MEM_TYPE_BUF, bufNum, &rxElem);

                    (void)StructRingBuf_put(&object->rxStructRingBuf, &rxElem);
                }

                clearNewDataStatus.statusLow |= ((uint32_t)1U << bufNum);
            }
        }
    }

    if (object->rxBufNum > 32U)
    {
        /* Check for Rx messages in buffers 32-63 */
        if (newDataStatus.statusHigh != 0U)
        {
            for (bufNum = 0U; bufNum < Math_MIN((object->rxBufNum - 32U), 32U); bufNum++)
            {
                if ((newDataStatus.statusHigh & ((uint32_t)1U << bufNum)) != 0U)
                {
                    if (!TCAN455X_isRxStructRingBufFull(handle))
                    {
                        MCAN_readRxMsg(MCAN_MEM_TYPE_BUF, (bufNum + 32U), &rxElem);

                        (void)StructRingBuf_put(&object->rxStructRingBuf, &rxElem);
                    }

                    clearNewDataStatus.statusHigh |= ((uint32_t)1U << bufNum);
                }
            }
        }
    }

    MCAN_clearNewDataStatus(&clearNewDataStatus);
}

/*
 *  ======== TCAN455X_irqHandler ========
 */
static void TCAN455X_irqHandler(uint_least8_t index)
{
    (void)index; /* unused arg */

    /* Since TCAN455X is controlled via SPI, IRQ handling must be deferred
     * to a task to avoid delaying lower priority interrupts while waiting
     * for SPI transactions.
     */

    /* Unblock task to handle the interrupt */
    SemaphoreP_post(&tcanIRQSemaphore);
}

/*
 *  ======== TCAN455X_initSPI ========
 */
static inline int_fast16_t TCAN455X_initSPI(void)
{
    int_fast16_t status = CAN_STATUS_SUCCESS;
    SPI_Params spiParams;

    SPI_init();

    if (TCAN455X_spiHandle == NULL)
    {
        /* Configure SPI as controller for TCAN455X */
        SPI_Params_init(&spiParams);
        spiParams.bitRate      = TCAN455X_config.spiBitRate;
        spiParams.frameFormat  = SPI_POL0_PHA0;
        spiParams.mode         = SPI_CONTROLLER;
        spiParams.transferMode = SPI_MODE_BLOCKING;

        TCAN455X_spiHandle = SPI_open(TCAN455X_config.spiIndex, &spiParams);
    }

    if (TCAN455X_spiHandle == NULL)
    {
        status = CAN_STATUS_ERROR;
    }
    else
    {
        /* Config GPIO for SW-controlled SPI CS */
        GPIO_setConfig(TCAN455X_config.spiCSPin, GPIO_CFG_OUTPUT | GPIO_CFG_OUT_HIGH);

        /* Create binary semaphore for SPI resource access */
        if (SemaphoreP_constructBinary(&spiAccessSemaphore, 1) == NULL)
        {
            status = CAN_STATUS_ERROR;
        }
    }

    return status;
}

/*
 *  ======== TCAN455X_clearMsgRam ========
 * To avoid ECC errors right after initialization, the MRAM should be zeroed out
 * during the initialization, power up, power on reset and wake events, a
 * process thus ensuring ECC is properly calculated.
 */
static void TCAN455X_clearMsgRam(void)
{
    uint16_t endAddr;
    uint32_t addr;
    uint8_t buf[32] = {0};

    addr    = MCAN_getMRAMOffset();
    endAddr = (uint16_t)(addr + TCAN455X_MRAM_SIZE);

    while (addr < endAddr)
    {
        MCAN_writeMsgRam(addr, &buf[0], sizeof(buf));
        addr += sizeof(buf);
    }
}

/*
 *  ======== TCAN455X_setBitRate ========
 */
static int_fast16_t TCAN455X_setBitRate(const CAN_Config *config, uint32_t canInputClkFreq)
{
    const CAN_HWAttrs *hwAttrs     = config->hwAttrs;
    int_fast16_t status            = CAN_STATUS_SUCCESS;
    MCAN_BitTimingParams bitTiming = {0};

    /* Only 40MHz input clock is supported for maximum bit rate */
    if (canInputClkFreq == 40U)
    {
        /* NOTE: Add 1 to each programmed bit time to get functional value and +1 for for sync segment.
         *    Bit Time = TSEG1 + TSEG2 + 1
         *    Bit Rate = (MCAN clock / Prescaler) / (Bit Time)
         *    Sample Point % = ((TSEG1 + 1) / (Bit Time)) * 100
         * - All TSEG1 and TSEG2 values above refer to the functional values
         *
         * Sampling Point % was chosen to be < 80 according to CiA 601 CAN FD Node
         * and System Design, Part 3 System Design Recommendation v1.0.0.
         */
        switch (hwAttrs->nominalBitRate)
        {
            case 125000U:
                /* 125kbps nominal with 40MHz clk and 75% sample point: ((40E6 / 2) / (119 + 40 + 1) = 125E3) */
                bitTiming.nomRatePrescaler  = 1U;
                bitTiming.nomTimeSeg1       = 118U;
                bitTiming.nomTimeSeg2       = 39U;
                bitTiming.nomSynchJumpWidth = 39U; /* typically set equal to seg 2 */
                break;

            case 250000U:
                /* 250kbps nominal with 40MHz clk and 75% sample point: ((40E6 / 1) / (119 + 40 + 1) = 250E3) */
                bitTiming.nomRatePrescaler  = 0U;
                bitTiming.nomTimeSeg1       = 118U;
                bitTiming.nomTimeSeg2       = 39U;
                bitTiming.nomSynchJumpWidth = 39U; /* typically set equal to seg 2 */
                break;

            case 500000U:
                /* 500kbps nominal with 40MHz clk and 75% sample point ((40E6 / 1) / (59 + 20 + 1) = 500E3) */
                bitTiming.nomRatePrescaler  = 0U;
                bitTiming.nomTimeSeg1       = 58U;
                bitTiming.nomTimeSeg2       = 19U;
                bitTiming.nomSynchJumpWidth = 19U; /* typically set equal to seg 2 */
                break;

            case 1000000U:
                /* 1Mbps nominal with 40MHz clk and 75% sample point ((40E6 / 1) / (29 + 10 + 1) = 1E6) */
                bitTiming.nomRatePrescaler  = 0U;
                bitTiming.nomTimeSeg1       = 28U;
                bitTiming.nomTimeSeg2       = 9U;
                bitTiming.nomSynchJumpWidth = 9U; /* typically set equal to seg 2 */
                break;

            default:
                status = CAN_STATUS_ERROR;
                break;
        }

        if (hwAttrs->enableBRS)
        {
            switch (hwAttrs->dataBitRate)
            {
                case 125000U:
                    /* 125kbps with 40MHz clk and 75% sample point: ((40E6 / 10) / (23 + 8 + 1) = 125E3) */
                    bitTiming.dataRatePrescaler  = 9U;
                    bitTiming.dataTimeSeg1       = 22U;
                    bitTiming.dataTimeSeg2       = 7U;
                    bitTiming.dataSynchJumpWidth = 7U; /* typically set equal to seg 2 */
                    break;

                case 250000U:
                    /* 250kbps with 40MHz clk and 75% sample point: ((40E6 / 8) / (14 + 5 + 1) = 250E3) */
                    bitTiming.dataRatePrescaler  = 7U;
                    bitTiming.dataTimeSeg1       = 13U;
                    bitTiming.dataTimeSeg2       = 4U;
                    bitTiming.dataSynchJumpWidth = 4U; /* typically set equal to seg 2 */
                    break;

                case 500000U:
                    /* 500kbps with 40MHz clk and 75% sample point ((40E6 / 2) / (29 + 10 + 1) = 500E3) */
                    bitTiming.dataRatePrescaler  = 1U;
                    bitTiming.dataTimeSeg1       = 28U;
                    bitTiming.dataTimeSeg2       = 9U;
                    bitTiming.dataSynchJumpWidth = 9U; /* typically set equal to seg 2 */
                    break;

                case 1000000U:
                    /* 1Mbps with 40MHz clk and 75% sample point ((40E6 / 1) / (29 + 10 + 1) = 1E6) */
                    bitTiming.dataRatePrescaler  = 0U;
                    bitTiming.dataTimeSeg1       = 28U;
                    bitTiming.dataTimeSeg2       = 9U;
                    bitTiming.dataSynchJumpWidth = 9U; /* typically set equal to seg 2 */
                    break;

                case 2000000U:
                    /* 2Mbps with 40MHz clk and 75% sample point ((40E6 / 1) / (14 + 5 + 1) = 2E6) */
                    bitTiming.dataRatePrescaler  = 0U;
                    bitTiming.dataTimeSeg1       = 13U;
                    bitTiming.dataTimeSeg2       = 4U;
                    bitTiming.dataSynchJumpWidth = 4U; /* typically set equal to seg 2 */
                    break;

                case 4000000U:
                    /* 4Mbps with 40MHz clk and 70% sample point ((40E6 / 1) / (6 + 3 + 1) = 4E6) */
                    bitTiming.dataRatePrescaler  = 0U;
                    bitTiming.dataTimeSeg1       = 5U;
                    bitTiming.dataTimeSeg2       = 2U;
                    bitTiming.dataSynchJumpWidth = 2U; /* typically set equal to seg 2 */
                    break;

                case 5000000U:
                    /* 5Mbps with 40MHz clk and 62.5% sample point ((40E6 / 1) / (4 + 3 + 1) = 5E6) */
                    bitTiming.dataRatePrescaler  = 0U;
                    bitTiming.dataTimeSeg1       = 3U;
                    bitTiming.dataTimeSeg2       = 2U;
                    bitTiming.dataSynchJumpWidth = 2U; /* typically set equal to seg 2 */
                    break;

                default:
                    status = CAN_STATUS_ERROR;
                    break;
            }
        }
    }
    else
    {
        status = CAN_STATUS_ERROR;
    }

    if (status == CAN_STATUS_SUCCESS)
    {
        if (MCAN_setBitTime(&bitTiming) != MCAN_STATUS_SUCCESS)
        {
            status = CAN_STATUS_ERROR;
        }
    }

    return status;
}

/*
 *  ======== TCAN455X_getStatus ========
 */
uint32_t TCAN455X_getStatus(void)
{
    return TCAN455X_readReg(TCAN455X_STATUS);
}

/*
 *  ======== TCAN455X_clearStatus ========
 */
void TCAN455X_clearStatus(void)
{
    TCAN455X_writeReg(TCAN455X_STATUS, (uint32_t)0xFFFFFFFFU);
}

/*
 *  ======== TCAN455X_setInterruptEnable ========
 *  Sets TCAN455X-specific interrupt enables.
 */
static void TCAN455X_setInterruptEnable(uint32_t ie)
{
    TCAN455X_writeReg(TCAN455X_IE, ie);
}

/*
 *  ======== TCAN455X_clearInterrupt ========
 *  Clears TCAN455X-specific interrupt flags.
 */
static void TCAN455X_clearInterrupt(uint32_t mask)
{
    TCAN455X_writeReg(TCAN455X_IR, mask);
}

/*
 *  ======== TCAN455X_modifyModeReg ========
 */
static void TCAN455X_modifyModeReg(uint32_t mask, uint32_t val)
{
    uint32_t regVal = TCAN455X_readReg(TCAN455X_MODE);

    regVal &= ~mask;
    /* Bit 5 is a reserved bit that must be written as a 1 */
    regVal |= ((val & mask) | (uint32_t)TCAN455X_MODE_FORCED_SET_BITS);

    TCAN455X_writeReg(TCAN455X_MODE, regVal);
}

/*
 *  ======== TCAN455X_setExtTsPrescaler ========
 */
static inline int_fast16_t TCAN455X_setExtTsPrescaler(uint32_t prescaler)
{
    /* Copy the parameter - MISRA 17.8 */
    uint32_t myPrescaler = prescaler;

    if (myPrescaler == 1U)
    {
        myPrescaler = 0U;
    }

    /* Return error if prescaler value is not a multiple of 8 */
    if ((myPrescaler & 0x7U) != 0U)
    {
        return CAN_STATUS_ERROR;
    }

    /* Divide prescaler value by 8 when writing the register */
    TCAN455X_writeReg(TCAN455X_TS_PRESCALER, myPrescaler >> 3U);

    return CAN_STATUS_SUCCESS;
}

/*
 *  ======== TCAN455X_setMode ========
 */
void TCAN455X_setMode(uint32_t mode)
{
    TCAN455X_modifyModeReg(TCAN455X_MODE_OPMODE_MASK, mode);
}

/*
 *  ======== TCAN455X_getMode ========
 */
uint32_t TCAN455X_getMode(void)
{
    uint32_t mode = TCAN455X_readReg(TCAN455X_MODE) & (uint32_t)TCAN455X_MODE_OPMODE_MASK;

    return mode;
}

/*
 *  ======== TCAN455X_disableSleepWakeErrorTimeout ========
 */
void TCAN455X_disableSleepWakeErrorTimeout(void)
{
    /* Disable Sleep Wake Error */
    TCAN455X_modifyModeReg(TCAN455X_MODE_SWE_DIS, TCAN455X_MODE_SWE_DIS);
}

/*
 *  ======== TCAN455X_reset ========
 *  Note: This function will trigger TCAN455X interrupt during the reset which
 *  should be ignored.
 */
static void TCAN455X_reset(void)
{
    /* Perform TCAN455X device reset.
     *
     * Default configuration after reset:
     *  - 40 MHz input clock
     *  - GPIO1 is MCAN_INT 1
     *  - GPIO2 has no action
     *  - INH pin (inhibit for system voltage regulators) enabled
     *  - nWKRQ pin mirrors INH pin
     *  - Standby mode
     *  - WDT enabled
     *  - Sleep wake error enabled (if no SPI writes for 4 minutes after wake,
     *    go back to sleep)
     *  - Test mode disabled
     *  - All device-specific interrupts enabled
     */
    TCAN455X_modifyModeReg(TCAN455X_MODE_RESET, TCAN455X_MODE_RESET);

    /* After device reset, a wait of at least 700us should be used before R/W
     * to TCAN455X.
     */
    ClockP_usleep(TCAN455X_RESET_DELAY_US);
}

/*
 *  ======== TCAN455X_init ========
 */
static int_fast16_t TCAN455X_init(const CAN_Config *config,
                                  const CAN_MsgRamConfig *msgRamConfig,
                                  const CAN_BitRateTimingRaw *bitTiming,
                                  uint32_t tsPrescaler)
{
    const CAN_HWAttrs *hwAttrs     = config->hwAttrs;
    CAN_Object *object             = config->object;
    int_fast16_t status            = CAN_STATUS_SUCCESS;
    MCAN_ConfigParams configParams = {0U};
    MCAN_InitParams initParams     = {0U};
    MCAN_RxNewDataStatus newDataStatus;
    const CAN_MsgRamConfig *tempMsgRamConfig;

    TCAN455X_reset();

    /* Ensure SPI communication is operational */
    if (MCAN_readReg(MCAN_ENDN) != MCAN_ENDN_ETV_VALUE)
    {
        return CAN_STATUS_ERROR;
    }

    /* Disable TCAN4550-specific WDT */
    TCAN455X_modifyModeReg(TCAN455X_MODE_WDT_MASK, TCAN455X_MODE_WDT_DIS);

    /* Disable all device-specific interrupt enables */
    TCAN455X_setInterruptEnable((uint32_t)0x0U);

    /* Clear all status bits */
    TCAN455X_clearStatus();

    /* Clear all interrupts */
    TCAN455X_clearInterrupt((uint32_t)0xFFFFFFFFU);

    /* MCAN SW init mode should be set already but explicitly set it again */
    TCAN455X_setMode(TCAN455X_MODE_OPMODE_STANDBY);

    /* Set FD mode and bit rate switching */
    initParams.fdMode    = hwAttrs->enableCANFD ? 1U : 0U;
    initParams.brsEnable = hwAttrs->enableBRS ? 1U : 0U;

    if (MCAN_init(&initParams) != MCAN_STATUS_SUCCESS)
    {
        status = CAN_STATUS_ERROR;
    }

    if (status == CAN_STATUS_SUCCESS)
    {
        /* TCAN455X uses a prescaler off of the Host Clock frequency
         * (crystal/clkin) that allows for a divided down clock value to be used
         * as the timestamp.
         */
        status = TCAN455X_setExtTsPrescaler(tsPrescaler);
    }

    if (status == CAN_STATUS_SUCCESS)
    {
        /* CAN FD requires using "external" timestamp value. "External" means
         * external to MCAN IP.
         */
        configParams.tsSelect = MCAN_TSCC_COUNTER_EXTERNAL;

        /* Reject remote frames since they are rarely used and do not even exist
         * in the CAN FD format.
         */
        configParams.filterConfig.rrfs = 1U;
        configParams.filterConfig.rrfe = 1U;

        if (hwAttrs->rejectNonMatchingMsgs)
        {
            /* Reject incoming messages that do not match a filter, the default
             * is to accept them into Rx FIFO0.
             */
            configParams.filterConfig.anfs = MCAN_GFC_NM_REJECT;
            configParams.filterConfig.anfe = MCAN_GFC_NM_REJECT;
        }

        if (MCAN_config(&configParams) != MCAN_STATUS_SUCCESS)
        {
            status = CAN_STATUS_ERROR;
        }
    }

    if (status == CAN_STATUS_SUCCESS)
    {
        object->intMask = CANMCAN_getInterruptMask(object->eventMask);

        /* Always enable transmit complete IRQ if there is a Tx FIFO/Queue
         * and the Tx ring buffer size is non-zero.
         */
        if ((object->txFifoQNum != 0U) && (hwAttrs->txRingBufSize != 0U))
        {
            object->intMask |= (uint32_t)MCAN_INT_SRC_TRANS_COMPLETE;
        }

        MCAN_setIntLineSel(object->intMask, MCAN_INT_LINE_NUM_0);
        MCAN_enableInt(object->intMask);
        MCAN_enableIntLine(MCAN_INT_LINE_NUM_0);

        if ((object->intMask & MCAN_INT_SRC_TRANS_COMPLETE) != 0U)
        {
            /* Enable transmission interrupt for all buffers */
            MCAN_enableTxBufTransInt(0xFFFFFFFFU);
        }

        if (bitTiming != NULL)
        {
            status = CANMCAN_setBitTimingRaw(bitTiming);
        }
        else
        {
            status = TCAN455X_setBitRate(config, TCAN455X_config.clkFreqMHz);
        }
    }

    if (status == CAN_STATUS_SUCCESS)
    {
        /* Clear message RAM to avoid ECC errors */
        TCAN455X_clearMsgRam();

        if (msgRamConfig == NULL)
        {
            /* If msg RAM config is NULL use default */
            tempMsgRamConfig = &TCAN455X_defaultMsgRamConfig;
        }
        else
        {
            tempMsgRamConfig = msgRamConfig;
        }

        /* Copy config attributes needed for run-time */
        object->txBufNum       = tempMsgRamConfig->txBufNum;
        object->txFifoQNum     = tempMsgRamConfig->txFifoQNum;
        object->txEventFifoNum = tempMsgRamConfig->txEventFifoNum;
        object->rxBufNum       = tempMsgRamConfig->rxBufNum;
        object->rxFifoNum[0]   = tempMsgRamConfig->rxFifoNum[0];
        object->rxFifoNum[1]   = tempMsgRamConfig->rxFifoNum[1];

        /* Setup message RAM sections and filters */
        status = CANMCAN_configMsgRam(tempMsgRamConfig, TCAN455X_MRAM_SIZE, hwAttrs->enableCANFD);
    }

    if (status == CAN_STATUS_SUCCESS)
    {
        MCAN_clearIntStatus(object->intMask);
        newDataStatus.statusLow  = 0xFFFFFFFFU;
        newDataStatus.statusHigh = 0xFFFFFFFFU;
        MCAN_clearNewDataStatus(&newDataStatus);
    }

    return status;
}

/*
 *  ======== TCAN455X_taskFxn ========
 */
static void TCAN455X_taskFxn(void *arg)
{
    CAN_Handle handle  = (CAN_Handle)arg;
    CAN_Object *object = (CAN_Object *)handle->object;
    int32_t rxCnt;
    MCAN_ProtocolStatus protStatus;
    MCAN_TxFifoQStatus fifoQStatus;
    MCAN_TxEventFifoStatus txEventFifoStatus;
    uint32_t event;
    uint32_t intStatus;
    uint32_t txOccurred = 0U;

    while (1)
    {
        /* Wait for TCAN455X_irqHandler to post semaphore. No need to check return value when waiting forever. */
        (void)SemaphoreP_pend(&tcanIRQSemaphore, SemaphoreP_WAIT_FOREVER);

        intStatus = MCAN_getIntStatus() & object->intMask;

        while (intStatus != 0U)
        {
            MCAN_clearIntStatus(intStatus);

            if ((intStatus & MCAN_INT_SRC_BUS_OFF_STATUS) != 0U)
            {
                MCAN_getProtocolStatus(&protStatus);

                /* Node is Bus Off when transmit error count >= 256 */
                if (protStatus.busOffStatus == 1U)
                {
                    /* Error recovery - normal operation will resume after 129 occurrences of Bus Idle */
                    MCAN_setOpMode(MCAN_OPERATION_MODE_NORMAL);
                    event = CAN_EVENT_BUS_OFF;
                }
                else
                {
                    event = CAN_EVENT_BUS_ON;
                }

                /* Bus Off interrupt is always enabled regardless of event mask
                 * so we must check the event mask before executing the callback.
                 */
                if ((event & object->eventMask) != 0U)
                {
                    /* Call the event callback function provided by the application */
                    object->eventCbk(handle, event, 0U, object->userArg);
                }
            }

            if ((intStatus & MCAN_INT_SRC_ERR_PASSIVE) != 0U)
            {
                MCAN_getProtocolStatus(&protStatus);

                if (protStatus.errPassive == 1U)
                {
                    /* Node is Error Passive when either transmit or receiver error count >= 128 */
                    event = CAN_EVENT_ERR_PASSIVE;
                }
                else
                {
                    /* Node is Error Active when both transmit and receiver error count < 128 */
                    event = CAN_EVENT_ERR_ACTIVE;
                }

                /* Call the event callback function provided by the application */
                object->eventCbk(handle, event, 0U, object->userArg);
            }

            if ((intStatus & MCAN_INT_SRC_RX_FIFO0_NEW_MSG) != 0U)
            {
                TCAN455X_handleRxFifo(handle, MCAN_RX_FIFO_NUM_0);
            }

            if ((intStatus & MCAN_INT_SRC_RX_FIFO1_NEW_MSG) != 0U)
            {
                TCAN455X_handleRxFifo(handle, MCAN_RX_FIFO_NUM_1);
            }

            if ((intStatus & MCAN_INT_SRC_DEDICATED_RX_BUFF_MSG) != 0U)
            {
                TCAN455X_handleRxBuf(handle);
            }

            if ((intStatus & MCAN_INT_SRC_TRANS_COMPLETE) != 0U)
            {
                /* Read TX buffer transmission status if the Tx finished event mask
                 * is set so it can be provided to the event callback.
                 */
                if ((object->eventMask & CAN_EVENT_TX_FINISHED) != 0U)
                {
                    txOccurred = MCAN_getTxBufTransmissionStatus();
                }

                if ((object->txFifoQNum != 0U) && (StructRingBuf_getCount(&object->txStructRingBuf) > 0))
                {
                    MCAN_getTxFifoQStatus(&fifoQStatus);

                    if (fifoQStatus.fifoFull == 0U)
                    {
                        /* Return value can be ignored since count was checked */
                        (void)StructRingBuf_get(&object->txStructRingBuf, &txElem);

                        /* Return value can be ignored as the FIFO is not full */
                        (void)CAN_write(handle, &txElem);
                    }
                }

                /* Source Tx complete interrupt is always enabled if the Tx ring
                 * buffer size is non-zero so we must check the event mask before
                 * executing the callback.
                 */
                if ((object->eventMask & CAN_EVENT_TX_FINISHED) != 0U)
                {
                    /* Call the event callback function provided by the application */
                    object->eventCbk(handle, CAN_EVENT_TX_FINISHED, txOccurred, object->userArg);
                }
            }

            if ((intStatus & MCAN_INT_SRC_TX_EVT_FIFO_ELEM_LOST) != 0U)
            {
                if ((object->eventMask & CAN_EVENT_TX_EVENT_LOST) != 0U)
                {
                    /* Call the event callback function provided by the application */
                    object->eventCbk(handle, CAN_EVENT_TX_EVENT_LOST, 0U, object->userArg);
                }
            }

            if ((intStatus & MCAN_INT_SRC_TX_EVT_FIFO_NEW_ENTRY) != 0U)
            {
                MCAN_getTxEventFifoStatus(&txEventFifoStatus);

                if ((object->eventMask & CAN_EVENT_TX_EVENT_AVAIL) != 0U)
                {
                    /* Call the event callback function provided by the application */
                    object->eventCbk(handle, CAN_EVENT_TX_EVENT_AVAIL, txEventFifoStatus.fillLvl, object->userArg);
                }
            }

            if ((intStatus & MCAN_INT_SRC_RX_MASK) != 0U)
            {
                event = CAN_EVENT_RX_DATA_AVAIL;

                rxCnt = StructRingBuf_getCount(&object->rxStructRingBuf);

                if (rxCnt > 0)
                {
                    /* Call the event callback function provided by the application */
                    object->eventCbk(handle, CAN_EVENT_RX_DATA_AVAIL, (uint32_t)rxCnt, object->userArg);
                }
            }

            if ((intStatus & MCAN_INT_SRC_RX_FIFO0_MSG_LOST) != 0U)
            {
                /* Call the event callback function provided by the application */
                object->eventCbk(handle, CAN_EVENT_RX_FIFO_MSG_LOST, 0U, object->userArg);
            }

            if ((intStatus & MCAN_INT_SRC_RX_FIFO1_MSG_LOST) != 0U)
            {
                /* Call the event callback function provided by the application */
                object->eventCbk(handle, CAN_EVENT_RX_FIFO_MSG_LOST, 1U, object->userArg);
            }

            /* Since we are using edge-triggered IRQ, re-check the status to ensure interrupts are not missed */
            intStatus = MCAN_getIntStatus() & object->intMask;
        }
    }
}

/*
 *  ======== CAN_initDevice ========
 */
int_fast16_t CAN_initDevice(uint_least8_t index, CAN_Params *params)
{
    const CAN_Config *config = &CAN_config[index];
    int_fast16_t status      = CAN_STATUS_SUCCESS;

    /* Call driver init functions */
    status = TCAN455X_initSPI();

    if (status == CAN_STATUS_SUCCESS)
    {
        /* Config GPIO for TCAN455X nINT: internal pull-up, interrupt on falling edge.
         * Many devices do not support interrupt on level low (GPIO_CFG_IN_INT_LOW).
         */
        GPIO_setConfig(TCAN455X_config.interruptPin, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);
        GPIO_setCallback(TCAN455X_config.interruptPin, TCAN455X_irqHandler);

        /* TCAN455X_init() issues a device reset which will trigger an interrupt
         * so the GPIO interrupt must be disable to avoid R/W to TCAN455X before
         * the reset is completed (~700us).
         */
        TCAN455X_disableInterrupt();

        status = TCAN455X_init(config, params->msgRamConfig, params->bitTiming, params->tsPrescaler);
    }

    if (status == CAN_STATUS_SUCCESS)
    {
        /* Create binary semaphore for IRQ handling */
        if (SemaphoreP_constructBinary(&tcanIRQSemaphore, 0) == NULL)
        {
            status = CAN_STATUS_ERROR;
        }
    }

    if (status == CAN_STATUS_SUCCESS)
    {
        /* Initialize task params */
        TaskP_Params_init(&taskParams);
        taskParams.name      = "TCAN455X";
        taskParams.priority  = TCAN455X_config.taskPri;
        taskParams.stack     = TCAN455X_config.taskStack;
        taskParams.stackSize = TCAN455X_config.taskStackSize;
        taskParams.arg       = (void *)config;

        /* Construct a task for handling TCAN455X interrupts */
        if (TaskP_construct(&tcanTask, TCAN455X_taskFxn, &taskParams) == NULL)
        {
            status = CAN_STATUS_ERROR;
        }
    }

    if (status == CAN_STATUS_SUCCESS)
    {
        TCAN455X_enableInterrupt();

        /* Set mode of operation to Normal - this also clears MCAN.CCCR.INIT
         * so no need to call MCAN_setOpMode(MCAN_OPERATION_MODE_NORMAL)
         */
        TCAN455X_setMode(TCAN455X_MODE_OPMODE_NORMAL);
    }

    return status;
}

/*
 *  ======== CAN_close ========
 */
void CAN_close(CAN_Handle handle)
{
    CAN_Object *object = handle->object;

    TCAN455X_disableInterrupt();

    TCAN455X_reset();

    TaskP_destruct(&tcanTask);

    SemaphoreP_destruct(&tcanIRQSemaphore);
    SemaphoreP_destruct(&spiAccessSemaphore);

    object->isOpen = false;
}

/*
 *  ======== TCAN455X_enableLoopback ========
 */
static void TCAN455X_enableLoopback(bool externalModeEnable)
{
    /* Set mode of operation to Standby - this also sets MCAN.CCCR.INIT
     * so no need to call MCAN_setOpMode(MCAN_OPERATION_MODE_SW_INIT)
     */
    TCAN455X_setMode(TCAN455X_MODE_OPMODE_STANDBY);

    if (externalModeEnable)
    {
        MCAN_enableLoopbackMode(MCAN_LPBK_MODE_EXTERNAL);
    }
    else
    {
        MCAN_enableLoopbackMode(MCAN_LPBK_MODE_INTERNAL);
    }

    /* Set mode of operation to Normal - this also clears MCAN.CCCR.INIT
     * so no need to call MCAN_setOpMode(MCAN_OPERATION_MODE_NORMAL)
     */
    TCAN455X_setMode(TCAN455X_MODE_OPMODE_NORMAL);
}

/*
 *  ======== CAN_enableLoopbackExt ========
 */
int_fast16_t CAN_enableLoopbackExt(CAN_Handle handle)
{
    (void)handle; /* unused arg */

    TCAN455X_enableLoopback(true);

    return CAN_STATUS_SUCCESS;
}

/*
 *  ======== CAN_enableLoopbackInt ========
 */
int_fast16_t CAN_enableLoopbackInt(CAN_Handle handle)
{
    (void)handle; /* unused arg */

    TCAN455X_enableLoopback(false);

    return CAN_STATUS_SUCCESS;
}

/*
 *  ======== CAN_disableLoopback ========
 */
int_fast16_t CAN_disableLoopback(CAN_Handle handle)
{
    (void)handle; /* unused arg */

    /* Set mode of operation to Normal - this also sets MCAN.CCCR.INIT
     * so no need to call MCAN_setOpMode(MCAN_OPERATION_MODE_SW_INIT)
     */
    TCAN455X_setMode(TCAN455X_MODE_OPMODE_STANDBY);

    MCAN_disableLoopbackMode();

    /* Set mode of operation to Normal - this also clears MCAN.CCCR.INIT
     * so no need to call MCAN_setOpMode(MCAN_OPERATION_MODE_NORMAL)
     */
    TCAN455X_setMode(TCAN455X_MODE_OPMODE_NORMAL);

    return CAN_STATUS_SUCCESS;
}

/*
 *  ======== CAN_getBitTiming ========
 */
void CAN_getBitTiming(CAN_Handle handle, CAN_BitTimingParams *bitTiming, uint32_t *clkFreq)
{
    (void)handle; /* unused arg */

    MCAN_getBitTime(bitTiming);

    *clkFreq = TCAN455X_config.clkFreqMHz * 1000U;
}
