/******************************************************************************

 @file  sha2_driverlib.c

 @brief This module ports the TI-Drivers implementation of SHA2 using
        driverlib APIs. SHA2 & HMAC.

 ******************************************************************************

 Copyright (c) 2012-2024, Texas Instruments Incorporated
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

/*******************************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/sha2.h)
#include DeviceFamily_constructPath(driverlib/prcm.h)

#if defined(DeviceFamily_CC13X4) || defined(DeviceFamily_CC26X4) || defined(DeviceFamily_CC26X3)
#include DeviceFamily_constructPath(inc/hw_cpu_fpu.h)
#endif

#include "ti/common/cc26xx/sha2/sha2_driverlib.h"

/*******************************************************************************
 * CONSTANTS
 */
/* Digest length in bytes of the SHA256 hash */
#define SHA2_DIGEST_LENGTH_BYTES_256  32

/* Block size in bytes of the SHA256 hash */
#define SHA2_BLOCK_SIZE_BYTES_256     64

/* Outer and inner padding bytes used in HMAC */
#define HMAC_OPAD_BYTE 0x5C
#define HMAC_IPAD_BYTE 0x36

/*******************************************************************************
 * TYPEDEFS
 */
/* SHA2 operation types */
typedef enum {
    /* Single step unsupported */
    /* SHA2_OperationType_SingleStep, */
    SHA2_OperationType_MultiStep,
    SHA2_OperationType_Finalize,
} SHA2_OperationType;

/* SHA2 object for tracking SHA driver state */
typedef struct {
    bool                            isOpen;
    volatile bool                   operationInProgress;
    bool                            operationCanceled;
    int_fast16_t                    returnStatus;
    uint16_t                        bytesInBuffer;
    uint32_t                        bytesProcessed;
    uint8_t                         buffer[SHA2_BLOCK_SIZE_BYTES_256];
    uint32_t                        digest[SHA2_DIGEST_LENGTH_BYTES_256 / 4];
    uint32_t                        hmacDigest[SHA2_DIGEST_LENGTH_BYTES_256 / 4];
} SHA2CC26X2_Object;

extern bool periphRequired_g;
/*******************************************************************************
 * LOCAL VARIABLES
 */
static SHA2CC26X2_Object SHA2_object;
static const uint8_t *SHA2_data;
static uint32_t SHA2_dataBytesRemaining;
static SHA2_OperationType SHA2_operationType;

static uint32_t fpccrRestore;

/*******************************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @fn     floorUint32
 *
 * @brief  Calculate the largest multiple of divider less than or equal to value
 *         (the floor relative to the divider)
 *
 * @param  value    The value to calculate the floor of
 * @param  divider  The divider that the value is divided by
 *
 * @return The floor of value.
 */
static uint32_t floorUint32(uint32_t value, uint32_t divider) {
    return (value / divider) * divider;
}

/**
 * @fn     SHA2_hwiFxn
 *
 * @brief  Helper function for handling the SHA2 interrupt
 */
static void SHA2_hwiFxn() {
    uint32_t irqStatus;

    irqStatus = SHA2IntStatusRaw();
    SHA2IntClear(SHA2_RESULT_RDY | SHA2_DMA_IN_DONE | SHA2_DMA_BUS_ERR);

    /*
     * Prevent the following section from being interrupted by SHA2_cancelOperation().
     */
    IntMasterDisable();

    if (SHA2_object.operationCanceled) {
        /*
         * If the operation has been canceled we can end here.
         * Cleanup is done by SHA2_cancelOperation()
         */
        IntMasterEnable();
        return;

    } else if (irqStatus & SHA2_DMA_BUS_ERR) {
        /*
         * In the unlikely event of an error we can stop here.
         */
        SHA2_object.returnStatus = SHA2_STATUS_ERROR;

    } else if (SHA2_dataBytesRemaining == 0) {
        /*
         * Last transaction has finished. Nothing to do.
         */

    } else if (SHA2_dataBytesRemaining >= SHA2_BLOCK_SIZE_BYTES_256) {
        /*
         * Start another transaction
         */
        uint32_t transactionLength = floorUint32(SHA2_dataBytesRemaining, SHA2_BLOCK_SIZE_BYTES_256);

        SHA2ComputeIntermediateHash(SHA2_data,
                               SHA2_object.digest,
                               SHA2_MODE_SELECT_SHA256,
                               transactionLength);

        SHA2_dataBytesRemaining -= transactionLength;
        SHA2_data += transactionLength;
        SHA2_object.bytesProcessed += transactionLength;

        IntMasterEnable();
        return;

    } else if (SHA2_dataBytesRemaining > 0) {
        /*
         * Copy remaining data into buffer
         */
        memcpy(SHA2_object.buffer, SHA2_data, SHA2_dataBytesRemaining);
        SHA2_object.bytesInBuffer += SHA2_dataBytesRemaining;
        SHA2_dataBytesRemaining = 0;
    }

    /*
     * Since we got here, every transaction has been finished
     */
    SHA2_object.operationInProgress = false;

    /*
     * Reset byte counter if a hash has been finalized
     */
    if (SHA2_operationType != SHA2_OperationType_MultiStep) {
        SHA2_object.bytesProcessed = 0;
        SHA2_object.bytesInBuffer = 0;
    }

    IntMasterEnable();
}

/**
 * @fn     SHA2_waitForResult
 *
 * @brief  Continuously poll for SHA2 interrupts, and handle them until the
 *         SHA operation has completed.
 *
 * @return SHA2_STATUS_SUCCESS on success, SHA2_STATUS_ERROR otherwise.
 */
static int_fast16_t SHA2_waitForResult(){
    do {
        SHA2WaitForIRQFlags(SHA2_RESULT_RDY | SHA2_DMA_BUS_ERR);
        SHA2_hwiFxn();
    } while (SHA2_object.operationInProgress);

    return SHA2_object.returnStatus;
}

/**
 * @fn     SHA2_xorBufferWithByte
 *
 * @brief  xor buffer with byte.
 *
 * @return N/A.
 */
static void SHA2_xorBufferWithByte(uint8_t *buffer, size_t bufferLength, uint8_t byte)
{
    size_t i;

    for (i = 0; i < bufferLength; i++)
    {
        buffer[i] = buffer[i] ^ byte;
    }
}

/*******************************************************************************
 * EXTERNAL FUNCTIONS
 */

/* Documented in sha2_driverlib.h */
int_fast16_t SHA2_open()
{
    /* Initialize the SHA2 object */
    IntMasterDisable();

    if (SHA2_object.isOpen) {
        IntMasterEnable();
        return SHA2_STATUS_ERROR;
    }

    SHA2_object.isOpen = true;
    SHA2_object.operationInProgress = false;
    SHA2_object.operationCanceled = false;

    IntMasterEnable();

    SHA2_object.bytesInBuffer   = 0;
    SHA2_object.bytesProcessed  = 0;

    /* Enable the peripheral domains required for SHA2 */
    IntMasterDisable();

    /* If peripheral power was on before, don't turn the power off in
     * SHA2_close() */
    periphRequired_g = true;
    if (PRCMPowerDomainsAllOn(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON)
    {
        periphRequired_g = false;

        PRCMPowerDomainOn(PRCM_DOMAIN_PERIPH);
        while (PRCMPowerDomainsAllOn(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON);
    }

    PRCMPeripheralRunEnable(PRCM_PERIPH_CRYPTO);
    PRCMPeripheralSleepEnable(PRCM_PERIPH_CRYPTO);
    PRCMPeripheralDeepSleepEnable(PRCM_PERIPH_CRYPTO);
    PRCMLoadSet();
    while (!PRCMLoadGet());

    /* Disable FPU lazy stacking while the SHA2 instance is open. This prevents
     * the side effect from the SHA2ComputeFinalHash call of setting the CONTROL
     * special register. Lazy stacking is restored in SHA2_close().
     */
#if defined(DeviceFamily_CC13X4) || defined(DeviceFamily_CC26X4) || defined(DeviceFamily_CC26X3)
    fpccrRestore = HWREG(CPU_FPU_BASE + CPU_FPU_O_FPCCR);
    HWREG(CPU_FPU_BASE + CPU_FPU_O_FPCCR) = 0;
#else
    fpccrRestore = HWREG(CPU_SCS_BASE + CPU_SCS_O_FPCCR);
    HWREG(CPU_SCS_BASE + CPU_SCS_O_FPCCR) = 0;
#endif

    IntMasterEnable();
    return SHA2_STATUS_SUCCESS;
}

/* Documented in sha2_driverlib.h */
void SHA2_close()
{
    /* If there is still an operation ongoing, abort it now. */
    IntMasterDisable();
    if (SHA2_object.operationInProgress) {
        SHA2_cancelOperation();
    }

    SHA2_object.isOpen = false;

    /* Restore the previous state of the FPCCR register. */
#if defined(DeviceFamily_CC13X4) || defined(DeviceFamily_CC26X4) || defined(DeviceFamily_CC26X3)
    HWREG(CPU_FPU_BASE + CPU_FPU_O_FPCCR) = fpccrRestore;
#else
    HWREG(CPU_SCS_BASE + CPU_SCS_O_FPCCR) = fpccrRestore;
#endif

    IntMasterEnable();

    /* Disable the peripheral domains enabled by SHA2_open() */
    IntMasterDisable();

    /* Only power off the peripheral domain if it was not on prior to calling
     * SHA2_open() */
    if (!periphRequired_g) {
        PRCMPowerDomainOff(PRCM_DOMAIN_PERIPH);
        while (PRCMPowerDomainsAllOn(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_OFF);
    }

    PRCMPeripheralRunDisable(PRCM_PERIPH_CRYPTO);
    PRCMPeripheralSleepDisable(PRCM_PERIPH_CRYPTO);
    PRCMPeripheralDeepSleepDisable(PRCM_PERIPH_CRYPTO);
    PRCMLoadSet();
    while (!PRCMLoadGet());

    IntMasterEnable();
}

/* Documented in sha2_driverlib.h */
int_fast16_t SHA2_addData(const void* data, size_t length)
{
    /* If we are in SHA2_RETURN_BEHAVIOR_POLLING, we do not want an interrupt to trigger.
     * We need to disable it before kicking off the operation.
     */
    IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);

    SHA2_object.returnStatus = SHA2_STATUS_SUCCESS;
    SHA2_object.operationCanceled = false;
    SHA2_operationType = SHA2_OperationType_MultiStep;

    if ((SHA2_object.bytesInBuffer + length) >= SHA2_BLOCK_SIZE_BYTES_256) {
        /* We have accumulated enough data to start a transaction. Now the question
         * remains whether we have to merge bytes from the data stream into the
         * buffer first. If so, we do that now, then start a transaction.
         * If the buffer is empty, we can start a transaction on the data stream.
         * Once the transaction is finished, we will decide how to follow up,
         * i.e. copy remaining data into the buffer.
         */
        uint32_t transactionLength;
        const uint8_t* transactionStartAddress;

        if (SHA2_object.bytesInBuffer > 0) {
            uint8_t *bufferTail = &SHA2_object.buffer[SHA2_object.bytesInBuffer];
            uint32_t bytesToCopyToBuffer = SHA2_BLOCK_SIZE_BYTES_256 - SHA2_object.bytesInBuffer;
            memcpy(bufferTail, data, bytesToCopyToBuffer);

            /* We reset the value already. That saves a comparison
             * in the ISR handler
             */
            SHA2_object.bytesInBuffer = 0;

            transactionStartAddress = SHA2_object.buffer;
            transactionLength       = SHA2_BLOCK_SIZE_BYTES_256;

            SHA2_data = (const uint8_t*)data + bytesToCopyToBuffer;
            SHA2_dataBytesRemaining = length - bytesToCopyToBuffer;
        } else {
            transactionStartAddress = data;
            transactionLength = floorUint32(length, SHA2_BLOCK_SIZE_BYTES_256);

            SHA2_data = (const uint8_t*)data + transactionLength;
            SHA2_dataBytesRemaining = length - transactionLength;
        }

        /*
         * Starting the accelerator and setting the operationInProgress
         * flag must be atomic.
         */
        IntMasterDisable();

        /*
         * Finally we need to decide whether this is the first hash
         * operation or a follow-up from a previous one.
         */
        if (SHA2_object.bytesProcessed > 0) {
            SHA2ComputeIntermediateHash(transactionStartAddress,
                                   SHA2_object.digest,
                                   SHA2_MODE_SELECT_SHA256,
                                   transactionLength);
        } else {
            SHA2ComputeInitialHash(transactionStartAddress,
                                   SHA2_object.digest,
                                   SHA2_MODE_SELECT_SHA256,
                                   transactionLength);
        }

        SHA2_object.bytesProcessed += transactionLength;
        SHA2_object.operationInProgress = true;
        IntMasterEnable();

    } else {
        /* There is no action required by the hardware. But we kick the
         * interrupt in order to follow the same code path as the other
         * operations.
         */
        uint8_t *bufferTail = &SHA2_object.buffer[SHA2_object.bytesInBuffer];
        memcpy(bufferTail, data, length);
        SHA2_object.bytesInBuffer += length;
        SHA2_dataBytesRemaining = 0;

        /*
         * Asserting the IRQ and setting the operationInProgress
         * flag must be atomic.
         */
        IntMasterDisable();
        SHA2_object.operationInProgress = true;
        SHA2IntEnable(SHA2_RESULT_RDY);
        HWREG(CRYPTO_BASE + CRYPTO_O_IRQSET) = SHA2_RESULT_RDY;
        IntMasterEnable();
    }

    return SHA2_waitForResult();
}

/* Documented in sha2_driverlib.h */
int_fast16_t SHA2_finalize(void *digest)
{
    /* If we are in SHA2_RETURN_BEHAVIOR_POLLING, we do not want an interrupt to trigger.
     * We need to disable it before kicking off the operation.
     */
    IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);

    SHA2_object.returnStatus = SHA2_STATUS_SUCCESS;
    SHA2_object.operationCanceled = false;
    SHA2_operationType = SHA2_OperationType_Finalize;

    /*
     * Starting the accelerator and setting the operationInProgress
     * flag must be atomic.
     */
    IntMasterDisable();
    SHA2_object.operationInProgress = true;

    if (SHA2_object.bytesProcessed == 0) {
        /*
         * Since no hash operation has been performed yet and no intermediate
         * digest is available, we have to perform a full hash operation
         */
        SHA2ComputeHash(SHA2_object.buffer,
                        digest,
                        SHA2_object.bytesInBuffer,
                        SHA2_MODE_SELECT_SHA256);
    }
    else if (SHA2_object.bytesInBuffer > 0) {
        uint32_t totalLength = SHA2_object.bytesProcessed + SHA2_object.bytesInBuffer;
        uint32_t chunkLength = SHA2_object.bytesInBuffer;

        SHA2ComputeFinalHash(SHA2_object.buffer,
                             digest,
                             SHA2_object.digest,
                             totalLength,
                             chunkLength,
                             SHA2_MODE_SELECT_SHA256);

    } else {
        /*
         * The hardware is incapable of finalizing an empty partial message,
         * but we can trick it by pretending this to be an intermediate block.
         *
         * Calculate the length in bits and put it at the end of the dummy
         * finalization block in big endian order
         */
        uint64_t lengthInBits = SHA2_object.bytesProcessed * 8;
        uint8_t *lengthBytes  = (uint8_t*)&lengthInBits;

        /*
         * Use the existing buffer as scratch pad
         */
        memset(SHA2_object.buffer, 0, SHA2_BLOCK_SIZE_BYTES_256);

        /*
         * Final block starts with '10000000'.
         */
        SHA2_object.buffer[0] = 0x80;

        /*
         * The length is written into the end of the finalization block
         * in big endian order. We always write only the last 8 bytes.
         */
        uint32_t i = 0;
        for (i = 0; i < 4; i++) {
            SHA2_object.buffer[SHA2_BLOCK_SIZE_BYTES_256 - 8 + i] = lengthBytes[7 - i];
            SHA2_object.buffer[SHA2_BLOCK_SIZE_BYTES_256 - 4 + i] = lengthBytes[3 - i];
        }

        /*
         * SHA2ComputeIntermediateHash uses the same digest location for
         * both input and output. Instead of copying the final digest result
         * we use the final location as input and output.
         */
        memcpy(digest, SHA2_object.digest, SHA2_DIGEST_LENGTH_BYTES_256);

        SHA2ComputeIntermediateHash(SHA2_object.buffer,
                               digest,
                               SHA2_MODE_SELECT_SHA256,
                               SHA2_BLOCK_SIZE_BYTES_256);
    }

    IntMasterEnable();
    return SHA2_waitForResult();
}


/* Documented in sha2_driverlib.h */
int_fast16_t SHA2_setupHmac(const uint8_t *key, size_t keyLength) {

    uint8_t xorBuffer[SHA2_BLOCK_SIZE_BYTES_256];

    /* If we are in SHA2_RETURN_BEHAVIOR_POLLING, we do not want an interrupt to trigger.
     * We need to disable it before kicking off the operation.
     */
    IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);

    /* Reset segmented processing state to ensure we start a fresh
     * transaction
     */
    SHA2_object.bytesInBuffer  = 0;
    SHA2_object.bytesProcessed = 0;

    /* Prepare the buffer of the derived key. We set the entire buffer to 0x00
     * so we do not need to pad it to the block size after copying the keying
     * material provided or the hash thereof there.
     */
    memset(xorBuffer, 0x00, SHA2_BLOCK_SIZE_BYTES_256);

    /* The key material must be <= SHA2_BLOCK_SIZE_BYTES_256
     * MCUboot uses key material of 32bytes
     * We filled the entire buffer with 0x00, we do not need to pad to the block
     * size.
     */
    memcpy(xorBuffer, key, keyLength);

    /* Compute k0 ^ ipad */
    SHA2_xorBufferWithByte(xorBuffer, SHA2_BLOCK_SIZE_BYTES_256, HMAC_IPAD_BYTE);

    /* Start a hash of k0 ^ ipad.
     * The intermediate result will be stored in the object for later
     * use when the application calls SHA2_addData on its actual message.
     */
    SHA2_addData(xorBuffer, SHA2_BLOCK_SIZE_BYTES_256);

    /* Undo k0 ^ ipad to reconstruct k0. Use the memory of k0 instead
     * of allocating a new copy on the stack to save RAM.
     */
    SHA2_xorBufferWithByte(xorBuffer, SHA2_BLOCK_SIZE_BYTES_256, HMAC_IPAD_BYTE);

    /* Compute k0 ^ opad. */
    SHA2_xorBufferWithByte(xorBuffer, SHA2_BLOCK_SIZE_BYTES_256, HMAC_OPAD_BYTE);

    /* Start a hash of k0 ^ opad.
     * We are using driverlib here since using the interal SHA2 driver APIs
     * would corrupt our previously stored intermediate results.
     * This lets us save a second intermediate result.
     */
    SHA2ComputeInitialHash(xorBuffer,
                           SHA2_object.hmacDigest,
                           SHA2_MODE_SELECT_SHA256,
                           SHA2_BLOCK_SIZE_BYTES_256);

    SHA2WaitForIRQFlags(SHA2_RESULT_RDY | SHA2_DMA_BUS_ERR);

    SHA2GetDigest(SHA2_object.hmacDigest, SHA2_DIGEST_LENGTH_BYTES_256);

    IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);
    IntEnable(INT_CRYPTO_RESULT_AVAIL_IRQ);

    return SHA2_STATUS_SUCCESS;

}

/* Documented in sha2_driverlib.h */
int_fast16_t SHA2_finalizeHmac(uint8_t* data) {

    uint8_t tmpDigest[SHA2_DIGEST_LENGTH_BYTES_256];
    /* If we are in SHA2_RETURN_BEHAVIOR_POLLING, we do not want an interrupt to trigger.
     * We need to disable it before kicking off the operation.
     */
    IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);

    /* Finalize H((k0 ^ ipad) || data) */
    SHA2_finalize(tmpDigest);

    memcpy(SHA2_object.digest, SHA2_object.hmacDigest, SHA2_DIGEST_LENGTH_BYTES_256);

    SHA2_object.bytesProcessed = SHA2_BLOCK_SIZE_BYTES_256;

    SHA2_operationType = SHA2_OperationType_MultiStep;

    /* Add the temporary digest computed earlier to the current digest */
    SHA2_addData(tmpDigest, SHA2_DIGEST_LENGTH_BYTES_256);

    //IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);
    //IntEnable(INT_CRYPTO_RESULT_AVAIL_IRQ);

    return SHA2_finalize(data);

}

/* Documented in sha2_driverlib.h */
int_fast16_t SHA2_cancelOperation() {
    IntMasterDisable();

    if (!SHA2_object.operationInProgress) {
        IntMasterEnable();
        return SHA2_STATUS_ERROR;
    }

    /* Reset the accelerator. Immediately stops ongoing operations. */
    HWREG(CRYPTO_BASE + CRYPTO_O_SWRESET) = CRYPTO_SWRESET_SW_RESET;

    /* Consume any outstanding interrupts we may have accrued
     * since disabling interrupts.
     */
    IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);

    SHA2_object.bytesInBuffer = 0;
    SHA2_object.bytesProcessed = 0;
    SHA2_object.operationCanceled = true;
    SHA2_object.returnStatus = SHA2_STATUS_CANCELED;

    IntMasterEnable();
    return SHA2_STATUS_SUCCESS;
}
