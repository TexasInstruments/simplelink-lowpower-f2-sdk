/******************************************************************************

 @file  AECCTRCC26X4_driverlib.c

 @brief This module implements an AESCTR driver using only driverlib APIs
        for the CC26X2 and CC26X4 device families with the following
        restrictions to minimize code size:
        - Single threaded applications only
        - Polling return behavior only
        - Signature verification only
        - P256 curve support only

 ******************************************************************************
 * Copyright (c) 2018-2024, Texas Instruments Incorporated
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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_crypto.h)
#include DeviceFamily_constructPath(driverlib/aes.h)

#include "AESCTRCC26X4_driverlib.h"
#include "ECDSACC26X4_driverlib.h"


/*
 *  ======== AES Global Variables ========
 */
AESCTR_OneStepOperation operation_g;
AESCTRCC26XX_Object object_g;
CryptoKey_Plaintext aesKey_g;
bool periphRequired_g = true;


/*
 *  ======== AESCTR_waitForResult ========
 */
static inline int_fast16_t AESCTR_waitForResult(AESCTRCC26XX_Object *object) {

    int_fast16_t status = AES_STATUS_ERROR;

    /* Wait until the operation is complete and check for DMA errors */
    if (AESWaitForIRQFlags(AES_RESULT_RDY | AES_DMA_BUS_ERR) & AES_DMA_BUS_ERR) {

        object->returnStatus = AES_STATUS_ERROR;
    }
    else /* Must be RESULT_AVAIL because DMA_IN_DONE was disabled */
    {
        if (AESGetCtrl() & CRYPTO_AESCTL_SAVE_CONTEXT_M)
        {
            /* Save counter value if the context was saved */
            AESReadNonAuthenticationModeIV(object->counter);
        }
    }

    /* Mark that we are done with the operation */
    object->hwBusy = false;

    /* Save the object's returnStatus before clearing operationInProgress or
     * posting the access semaphore in case it is overwritten.
     */
    status = object->returnStatus;

    if (!(object->operationType & AESCTR_OP_FLAG_SEGMENTED))
    {
        /* One-step or finalization operation is complete */
        object->operationInProgress = false;
    }

    /* Since plaintext keys use two reserved (by convention) slots in the keystore,
     * the slots must be invalidated to prevent its re-use without reloading
     * the key material again.
     */
    AESInvalidateKey(AES_KEY_AREA_6);
    AESInvalidateKey(AES_KEY_AREA_7);

    /*  This powers down all sub-modules of the crypto module until needed.
     *  It does not power down the crypto module at PRCM level and provides small
     *  power savings.
     */
    AESSelectAlgorithm(0x00);

    return status;

}

/*
 *  ======== AESCTR_initCounter ========
 */
static void AESCTR_initCounter(AESCTRCC26XX_Object *object, const uint8_t *initialCounter)
{
    if (initialCounter != NULL)
    {
        memcpy(object->counter, initialCounter, sizeof(object->counter));
    }
    else
    {
        memset(object->counter, 0, sizeof(object->counter));
    }
}

/*
 *  ======== AESCTR_processData ========
 */
static int_fast16_t AESCTR_processData(AESCTRCC26XX_Object object)
{
    int_fast16_t status                 = AES_STATUS_SUCCESS;
    size_t keyLength;
    uint8_t *keyingMaterial = NULL;
    uint32_t ctrlVal;

    /* Only plaintext is supported currently */
    keyLength      = object.key.keyLength;
    keyingMaterial = object.key.keyMaterial;


    /* Load the key from RAM or flash into the key store at a hardcoded and
     * reserved location.
     */
    if (AESWriteToKeyStore(keyingMaterial, keyLength, AES_KEY_AREA_6) != AES_STATUS_SUCCESS)
    {
        status = AES_STATUS_ERROR;
    }

    if (status == AES_STATUS_SUCCESS)
    {
        /* AESWriteKeyStore() enables the CRYPTO IRQ. Disable it if polling mode */
        IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);

        /* AESWriteToKeyStore() enables both DMA_IN_DONE and RESULT_AVAIL
         * interrupts but since this driver only utilizes RESULT_AVAIL,
         * disable the DMA_IN_DONE interrupt to simplify handling.
         */
        AESIntDisable(AES_DMA_IN_DONE);

        /* Power the AES sub-module of the crypto module */
        AESSelectAlgorithm(AES_ALGSEL_AES);

        /* Load the key from the key store into the internal register banks of the
         * AES sub-module known as AES_KEY1.
         */
        if (AESReadFromKeyStore(AES_KEY_AREA_6) != AES_STATUS_SUCCESS)
        {
            /* Since plaintext keys use two reserved (by convention) slots in the
             * keystore, the slots must be invalidated to prevent its re-use without
             * reloading the key material again.
             */
            AESInvalidateKey(AES_KEY_AREA_6);
            AESInvalidateKey(AES_KEY_AREA_7);

            /* This powers down all sub-modules of the crypto module until needed.
             * It does not power down the crypto module at PRCM level and provides
             * small power savings.
             */
            AESSelectAlgorithm(0x0);

            status = AES_STATUS_ERROR;
        }
    }

    if (status == AES_STATUS_SUCCESS)
    {
        /* Load counter value */
        AESSetInitializationVector(object.counter);

        ctrlVal = CRYPTO_AESCTL_CTR | CRYPTO_AESCTL_CTR_WIDTH_128_BIT;

        if ((object.operationType & AESCTR_OP_MODE_MASK) == AESCTR_MODE_ENCRYPT)
        {
            ctrlVal |= CRYPTO_AESCTL_DIR;
        }

        /* Context save is only necessary in the middle of segmented
         * operation.
         */
        if (object.operationType & AESCTR_OP_FLAG_SEGMENTED)
        {
            ctrlVal |= CRYPTO_AESCTL_SAVE_CONTEXT;
        }

        AESSetCtrl(ctrlVal);
        AESSetDataLength(object.inputLength);
        AESSetAuthLength(0);

        object.hwBusy = true;

        /* Start the input/output DMA */
        AESStartDMAOperation(object.input, object.inputLength, object.output, object.inputLength);

        status = AESCTR_waitForResult(&object);
    }

    if (status != AES_STATUS_SUCCESS)
    {
        /* Save the failure status in case the application ignores the return value
         * so the driver can reject any attempts to continue the failed operation.
         */
        object.returnStatus = status;
    }
    return status;
}
/*
 *  ======== AESCTR_startOneStepOperation ========
 */
static int_fast16_t AESCTR_startOneStepOperation(AESCTR_OneStepOperation *operation,
                                                 AESCTR_OperationType operationType)
{

    /*AESCTRCC26XX_Object object; -> set this a static def
     *following functions do take the object as input parameter
     * AESCTR_startOneStepOperation( )
     * AES_open()
     * AES_close()
     */

    int_fast16_t status;
    /* Verify input length is non-zero */
    if (operation->inputLength == 0)
    {
        return AES_STATUS_ERROR;
    }

    object_g.operation = (AESCTR_OperationUnion *)operation;
    object_g.operationType = operationType;
    /* We will only change the returnStatus if there is an error or cancellation */
    object_g.returnStatus  = AES_STATUS_SUCCESS;

    /* Make internal copy of operational params */
    object_g.key         = *(operation->key);
    object_g.input       = operation->input;
    object_g.inputLength = operation->inputLength;
    object_g.output      = operation->output;

    AESCTR_initCounter(&object_g, operation->initialCounter);

    status = AESCTR_processData(object_g);

    if (status != AES_STATUS_SUCCESS)
    {
        object_g.operationInProgress  = false;
    }
    return status;
}

/*
 *  ======== AES_cancelOperation ========
 */
static int_fast16_t AES_cancelOperation(AESCTRCC26XX_Object* object)
{
    IntMasterDisable();

    /* Check if the HW operation already completed or was never started */
    if (!object->hwBusy)
    {
        object->returnStatus        = AES_STATUS_CANCELED;
        object->operationInProgress = false;

        IntMasterEnable();

        return AES_STATUS_SUCCESS;
    }

    IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);

    IntMasterEnable();

    /* Reset the DMA to stop transfers */
    AESDMAReset();

    /* Issue SW reset to recover the AES engine */
    AESReset();

    /* Consume any outstanding interrupts we may have accrued since disabling
     * interrupts.
     */
    IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);

    object->returnStatus        = AES_STATUS_CANCELED;
    object->hwBusy              = false;
    object->operationInProgress = false;


    AESInvalidateKey(AES_KEY_AREA_6);
    AESInvalidateKey(AES_KEY_AREA_7);

    /*  This powers down all sub-modules of the crypto module until needed.
     *  It does not power down the crypto module at PRCM level and provides small
     *  power savings.
     */
    AESSelectAlgorithm(0x00);

    /* Always return success */
    return AES_STATUS_SUCCESS;
}

/*
 *  ======== AESCTR_oneStepEncrypt ========
 */
int_fast16_t AESCTR_oneStepEncrypt (AESCTR_OneStepOperation *operationStruct) {
    return AESCTR_startOneStepOperation(operationStruct, AESCTR_OPERATION_TYPE_ENCRYPT);
}

/*
 *  ======== AESCTR_oneStepDecrypt ========
 */
int_fast16_t AESCTR_oneStepDecrypt (AESCTR_OneStepOperation *operationStruct ) {
    return AESCTR_startOneStepOperation(operationStruct, AESCTR_OPERATION_TYPE_DECRYPT);
}

/*
 *  ======== AES_open ========
 */
int_fast16_t AES_open()
{
    if(!object_g.isOpen)
    {
        object_g.isOpen = true;
        object_g.operationInProgress = false;
    }

    if (PRCMPowerDomainsAllOn(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON)
    {
        PRCMPowerDomainOn(PRCM_DOMAIN_PERIPH);
        while (PRCMPowerDomainsAllOn(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON);
    }

    PRCMPeripheralRunEnable(PRCM_PERIPH_CRYPTO);
    PRCMPeripheralSleepEnable(PRCM_PERIPH_CRYPTO);
    PRCMPeripheralDeepSleepEnable(PRCM_PERIPH_CRYPTO);
    PRCMLoadSet();
    while (!PRCMLoadGet());

    IntMasterEnable();
    return AES_STATUS_SUCCESS;
}

int_fast16_t AES_close(void)
{
    /* If there is still an operation ongoing, abort it now. */
    if (object_g.operationInProgress) {
        AES_cancelOperation(&object_g);
    }

    IntMasterDisable();
    object_g.isOpen = false;

    if (!periphRequired_g) {
        PRCMPowerDomainOff(PRCM_DOMAIN_PERIPH);
        while (PRCMPowerDomainsAllOn(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_OFF);
    }
    periphRequired_g = true;

    PRCMPeripheralRunDisable(PRCM_PERIPH_CRYPTO);
    PRCMPeripheralSleepDisable(PRCM_PERIPH_CRYPTO);
    PRCMPeripheralDeepSleepDisable(PRCM_PERIPH_CRYPTO);
    PRCMLoadSet();
    while (!PRCMLoadGet());

    IntMasterEnable();

    return AES_STATUS_SUCCESS;
}
