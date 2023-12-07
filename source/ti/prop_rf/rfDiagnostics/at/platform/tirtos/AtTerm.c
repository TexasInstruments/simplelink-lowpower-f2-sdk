/*
 * Copyright (c) 2015-2022, Texas Instruments Incorporated
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

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#include <xdc/runtime/System.h>
#include <ti/drivers/UART2.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/spi/SPICC26X2DMA.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/drivers/GPIO.h>
#include <semaphore.h>
#include <ti/sysbios/knl/Clock.h>

#include "ti_drivers_config.h"

#include "at/platform/inc/AtTerm.h"
#include "at/AtProcess.h"

#ifdef AT_SPI
#define SPI_BITRATE      (1000000U)
#define SPI_FRAME_FORMAT (SPI_POL0_PHA1)
static char spiRxBuffer[SPI_MSG_LENGTH];
static char spiTxBuffer[SPI_MSG_LENGTH];
static sem_t spiSem;
static SPI_Handle spiHandle;
static SPI_Transaction spiTransaction;
static void spiTransferCompleteFxn(SPI_Handle handle, SPI_Transaction *transaction);
#else
#define UART_BAUD (115200)
static UART2_Handle uartHandle;
#endif

int32_t AtTerm_init(void)
{
#ifndef AT_SPI
    UART2_Params uartParams;

    // Create a UART with data processing off
    UART2_Params_init(&uartParams);
    uartParams.readMode         = UART2_Mode_BLOCKING;
    uartParams.writeMode        = UART2_Mode_NONBLOCKING;
    uartParams.readReturnMode   = UART2_ReadReturnMode_FULL;
    uartParams.baudRate         = UART_BAUD;

    uartHandle = UART2_open(CONFIG_UART_0, &uartParams);

    if(uartHandle == NULL)
    {
        System_abort("Error opening the UART");
    }
#else
    // SPI peripheral set up
    SPI_init();

    SPI_Params spiParams;
    SPI_Params_init(&spiParams);
    spiParams.frameFormat = SPI_FRAME_FORMAT;
    spiParams.transferCallbackFxn = spiTransferCompleteFxn;
    spiParams.transferMode = SPI_MODE_CALLBACK;
    spiParams.mode = SPI_PERIPHERAL;
    spiParams.bitRate = SPI_BITRATE;
    spiParams.dataSize = 8;

    spiHandle = SPI_open(CONFIG_SPI_0, &spiParams);
    if(spiHandle == NULL)
    {
        System_abort("Error opening the SPI");
    }

    memset((void *) spiTxBuffer,0, SPI_MSG_LENGTH);
    SPI_control(spiHandle, SPICC26X2DMA_CMD_RETURN_PARTIAL_ENABLE, NULL);

    sem_init(&spiSem, 0, 0);
#endif
    return 0;
}

int32_t AtTerm_getChar(char* ch)
{
    int32_t bytes = 0;
#ifndef AT_SPI
    char c = '\r';
    (void)UART2_read(uartHandle, &c, 1, (size_t *)&bytes);
    *ch = c;
#else
    uint16_t        i;
    uint16_t        j;
    bool           validCmdFmt =false;

    /* Initialize peripheral SPI transaction structure */
    memset(spiRxBuffer, 0, SPI_MSG_LENGTH);
    while(1)
    {
        if(validCmdFmt == false)
        {
            spiTransaction.count = SPI_MSG_LENGTH;
            spiTransaction.txBuf = spiTxBuffer;
            spiTransaction.rxBuf = spiRxBuffer;

            // Only start a SPI transfer when CS is de-asserted
            uint_fast8_t csValue = 0U;
            while(csValue == 0U)
            {
                csValue = GPIO_read(CONFIG_GPIO_SPI_0_CSN);
            }

            SPI_transfer(spiHandle, &spiTransaction);
            sem_wait(&spiSem);
            memset((void *) spiTxBuffer,0, SPI_MSG_LENGTH);

            //make sure cmd starts with AT and end with a '\r'
            for(i = 0; i< SPI_MSG_LENGTH;i++)
            {
                if((spiRxBuffer[i] == 'A' || spiRxBuffer[i] == 'a')  && (spiRxBuffer[i+1] == 'T' || spiRxBuffer[i+1] == 't'))
                {
                    for(j = i; j < SPI_MSG_LENGTH; j++)
                    {
                        // '\r 'check
                        if(spiRxBuffer[j] == '\r')
                        {
                            validCmdFmt = true;
                            break;
                        }
                    }
                }
                if(validCmdFmt == true)
                {
                    bytes = (j + 1) - i;
                    memcpy((uint8_t *)ch, (uint8_t *)&spiRxBuffer[i], bytes);
                    break;
                }
            }
        }
        else
        {
            break;
        }
    }
#endif
    return bytes;
}

void AtTerm_putChar(char ch)
{
#ifndef AT_SPI
    UART2_write(uartHandle, &ch, 1, NULL);
#else
    strncat(spiTxBuffer, &ch, 1);
#endif
}

void AtTerm_sendStringUi8Value(char *string, uint8_t value, uint8_t format)
{
    char strVal[128] = {0};

    if (format == 10)
    {
        sprintf(strVal, "%s%3d", (char*) string, value);
    }
    else
    {
        sprintf(strVal, "%s%2x", (char*) string, value);
    }

#ifndef AT_SPI
    UART2_write(uartHandle, strVal, strlen(strVal), NULL);
#else
    strncat(spiTxBuffer, strVal, (SPI_MSG_LENGTH - strlen(spiTxBuffer) - 1));
#endif
}

void AtTerm_sendStringI8Value(char *string, int8_t value, uint8_t format)
{
    char strVal[128] = {0};

    if (format == 10)
    {
        sprintf(strVal, "%s%3d", (char*) string, value);
    }
    else
    {
        sprintf(strVal, "%s%2x", (char*) string, value);
    }

#ifndef AT_SPI
    UART2_write(uartHandle, strVal, strlen(strVal), NULL);
#else
    strncat(spiTxBuffer, strVal, (SPI_MSG_LENGTH - strlen(spiTxBuffer) - 1));
#endif
}

void AtTerm_sendStringUi16Value(char *string, uint16_t value, uint8_t format)
{
    char strVal[128] = {0};

    if (format == 10)
    {
        sprintf(strVal, "%s%5d", (char*) string, value);
    }
    else
    {
        sprintf(strVal, "%s%4x", (char*) string, value);
    }

#ifndef AT_SPI
    UART2_write(uartHandle, strVal, strlen(strVal), NULL);
#else
    strncat(spiTxBuffer, strVal, (SPI_MSG_LENGTH - strlen(spiTxBuffer) - 1));
#endif
}

void AtTerm_sendStringI16Value(char *string, int16_t value, uint8_t format)
{
    char strVal[128] = {0};

    if (format == 10)
    {
        sprintf(strVal, "%s%5d", (char*) string, value);
    }
    else
    {
        sprintf(strVal, "%s%4x", (char*) string, value);
    }

#ifndef AT_SPI
    UART2_write(uartHandle, strVal, strlen(strVal), NULL);
#else
    strncat(spiTxBuffer, strVal, (SPI_MSG_LENGTH - strlen(spiTxBuffer) - 1));
#endif
}

void AtTerm_sendStringUi32Value(char *string, uint32_t value, uint8_t format)
{
    char strVal[128] = {0};

    if (format == 10)
    {
        sprintf(strVal, "%s%10u", (char*) string, (unsigned int)value);
    }
    else
    {
        sprintf(strVal, "%s%8x", (char*) string, (unsigned int)value);
    }

#ifndef AT_SPI
    UART2_write(uartHandle, strVal, strlen(strVal), NULL);
#else
    strncat(spiTxBuffer, strVal, (SPI_MSG_LENGTH - strlen(spiTxBuffer) - 1));
#endif
}

void AtTerm_sendStringI32Value(char *string, int32_t value, uint8_t format)
{
    char strVal[128] = {0};

    if (format == 10)
    {
        sprintf(strVal, "%s%10d", (char*) string, (int)value);
    }
    else
    {
        sprintf(strVal, "%s%8x", (char*) string, (unsigned int)value);
    }

#ifndef AT_SPI
    UART2_write(uartHandle, strVal, strlen(strVal), NULL);
#else
    strncat(spiTxBuffer, strVal, (SPI_MSG_LENGTH - strlen(spiTxBuffer) - 1));
#endif
}

void AtTerm_sendString(char *string)
{
#ifndef AT_SPI
    uint32_t len = strlen(string);
    UART2_write(uartHandle, string, len, NULL);
#else
    strncat(spiTxBuffer, string, (SPI_MSG_LENGTH - strlen(spiTxBuffer) - 1));
#endif
}

void AtTerm_clearTerm(void)
{
#ifndef AT_SPI
    //char clear[] = {0x0C};
    char c = '\f';
    UART2_write(uartHandle, &c, 1, NULL);
#else
    memset((void *) spiTxBuffer,0, SPI_MSG_LENGTH);
#endif
}

void AtTerm_getIdAndParam(char *paramStr, uint8_t *radioId, uintptr_t fxnParam, uintptr_t fxnParam2, size_t fxnParamLen)
{
    uint32_t _fxnParam;
    uint32_t _fxnParam2 = 0;
    uint8_t _radioId = 1;

    char *token;
    char delimiter[] = " ";
    token = strtok(paramStr, delimiter);

    if (NULL != token)
    {
        // a single numeric argument? save it to fxnParam, assume the default radio.
        _fxnParam = atoi(token);

        token = strtok(NULL, delimiter);
        if (NULL != token)
        {
            // 2 numeric arguments separated by a space? the first token
            // must have been the radioId and the next, fxnParam.
            _radioId = _fxnParam;
            _fxnParam = atoi(token);
            token = strtok(NULL, delimiter);
            if (NULL != token)
            {
                _fxnParam2 = atoi(token);
            }
            else
            {
                _fxnParam2 = RADIO_NO_PHY;
            }

        }

        if(NULL != (void *)fxnParam){
            switch(fxnParamLen){
            case (sizeof(uint8_t)):
                *(uint8_t *)fxnParam = (uint8_t)_fxnParam;
                *(uint8_t *)fxnParam2 = (uint8_t)_fxnParam2;
                break;
            case (sizeof(uint16_t)):
                *(uint16_t *)fxnParam = (uint16_t)_fxnParam;
                *(uint16_t *)fxnParam2 = (uint16_t)_fxnParam2;
                break;
            default:
                *(uint32_t *)fxnParam = _fxnParam;
                *(uint32_t *)fxnParam2 = _fxnParam2;
            break;
            }
        }

    }

    if(NULL != radioId)
        *radioId = _radioId;
}

#ifdef AT_SPI
void spiTransferCompleteFxn(SPI_Handle handle, SPI_Transaction *transaction)
{
    if ((transaction->status == SPI_TRANSFER_CSN_DEASSERT) || (transaction->status == SPI_STATUS_SUCCESS))
    {
        sem_post(&spiSem);
    }
}
#endif
