/*
 * Copyright (c) 2017-2021, Texas Instruments Incorporated
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
 *  ======== sha2hash.c ========
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Driver Header files */
#include <ti/drivers/UART2.h>
#include <ti/drivers/SHA2.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/* Defines */
#define MAX_MSG_LENGTH 256

/* UART pre-formated strings */
static const char promptStartup[] = "\n\n\rSHA2 Driver hash demo.";
static const char promptEnter[]   = "\n\n\rEnter a string to hash using SHA2:\n\n\r";
static const char promptHash[]    = "\n\n\rThe hashed result: ";

/* Message buffers */
static char msg[MAX_MSG_LENGTH];
static char formatedMsg[MAX_MSG_LENGTH];

/*
 *  ======== printHash ========
 */
void printHash(UART2_Handle handle, uint8_t *msg)
{
    uint32_t i;

    /* Format the message as printable hex */
    for (i = 0; i < SHA2_DIGEST_LENGTH_BYTES_256; i++)
    {
        sprintf(formatedMsg + (i * 2), "%02X", *(msg + i));
    }

    /* Print prompt */
    UART2_write(handle, promptHash, strlen(promptHash), NULL);
    /* Print result */
    UART2_write(handle, formatedMsg, strlen(formatedMsg), NULL);
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    int_fast16_t result;

    /* Driver handles */
    UART2_Handle uart2Handle;
    SHA2_Handle sha2Handle;

    /* UART variables */
    UART2_Params uart2Params;
    uint8_t input;
    uint16_t msgLength = 0;

    /* SHA2 variables */
    uint8_t hashedMsg[SHA2_DIGEST_LENGTH_BYTES_256];

    /* Call driver initialization functions */
    SHA2_init();

    /* Open UART for console output */
    UART2_Params_init(&uart2Params);
    uart2Params.readReturnMode = UART2_ReadReturnMode_FULL;

    uart2Handle = UART2_open(CONFIG_UART2_0, &uart2Params);

    if (!uart2Handle)
    {
        /* UART2_open() failed */
        while (1) {}
    }

    /* Open SHA2 */
    sha2Handle = SHA2_open(0, NULL);

    if (!sha2Handle)
    {
        /* SHA2_open() failed */
        while (1) {}
    }

    /* Prompt startup message */
    UART2_write(uart2Handle, promptStartup, strlen(promptStartup), NULL);

    /* Loop forever */
    while (1)
    {
        /* Print prompt */
        UART2_write(uart2Handle, promptEnter, strlen(promptEnter), NULL);

        /* Reset message length */
        msgLength = 0;

        /* Read in from the console until carriage-return is detected */
        while (1)
        {
            UART2_read(uart2Handle, &input, 1, NULL);

            /* If not carriage-return, echo input and continue to read */
            if (input != 0x0D)
            {
                UART2_write(uart2Handle, &input, 1, NULL);
            }
            else
            {
                break;
            }
            /* Store the received message */
            msg[msgLength] = input;
            msgLength++;

            /* Check if message buffer is full */
            if (msgLength == MAX_MSG_LENGTH)
            {
                /* We can't read in any more data, continue on to perform the hash */
                break;
            }
        }

        /* Perform a single step hash operation of the message */
        result = SHA2_hashData(sha2Handle, msg, msgLength, hashedMsg);
        if (result != SHA2_STATUS_SUCCESS)
        {
            // handle error
            while (1) {}
        }

        /* Print out the hashed result */
        printHash(uart2Handle, hashedMsg);
    }
}
