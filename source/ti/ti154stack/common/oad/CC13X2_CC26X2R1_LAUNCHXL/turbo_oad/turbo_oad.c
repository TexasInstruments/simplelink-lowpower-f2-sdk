/******************************************************************************

 @file turbo_oad.c

 @brief This file contains the implementation of the Turbo OAD delta image
        decoding API

 Group: LPRF SW RND
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2019-2025, Texas Instruments Incorporated
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

/******************************************************************************
 Includes
 *****************************************************************************/

#include <string.h>
#include <ti/drivers/utils/RingBuf.h>
#include "turbo_oad.h"

#if defined(FEATURE_TOAD)

/******************************************************************************
 Typedefs
 *****************************************************************************/
/* Represents a control block instruction for a delta block:
 * - diffReadLen:  The amount of data remaining to be read in 
 *                 the difference block
 * - extraReadLen: The amount of data remaining to be read in
 *                 the extra block
 * - seekOffset:   The offset to advanced the current image delta pointer
 *                 for the next delta instruction's difference block decoding
 */
typedef struct
{
    uint16_t diffReadLen;
    uint16_t extraReadLen;
    int16_t seekOffset;
} TOAD_ctrlBlock_t;

/******************************************************************************
 Constants
 *****************************************************************************/
#define TOAD_DEFAULT_OAD_BLOCK_SIZE         128
#define TOAD_DEFAULT_OAD_BLOCK_CACHE_SIZE   TOAD_BLOCK_CACHE_SIZE
#define TOAD_DEFAULT_WRITE_BUFFER_SIZE      TOAD_WRITE_BUFFER_SIZE

#define TOAD_MAGIC_WORD            "DELTAIMG"
#define TOAD_MAGIC_WORD_START_CHAR 'D'
#define TOAD_MAGIC_WORD_LEN         8

#define TOAD_CTRL_BLOCK_LEN         6

/******************************************************************************
 Global Variables
 *****************************************************************************/
// Module initialization variables
static const TOAD_params_t gToadDefaultParams = {
    .pAppImg            = 0x00000000,
    .oadBlockSize       = TOAD_DEFAULT_OAD_BLOCK_SIZE,
    .oadBlockCacheSize  = TOAD_DEFAULT_OAD_BLOCK_CACHE_SIZE,
    .writeBufferSize    = TOAD_DEFAULT_WRITE_BUFFER_SIZE,
    .newImgLen          = 0x00000000,
    .pfnWriteDataCb     = 0x00000000,
};

static TOAD_params_t gToadParams;
static RingBuf_Object gOadBlockCacheRingBuf;
static uint8_t* gpOadBlockCache;
static uint8_t* gpWriteBuffer;

/* Function static variables declared globally (required for closing and
 * re-opening the module)
 */

// Pointer to the application image address to base the next difference block decoding
static uint8_t* gTOADdecode_pAppImgDelta = 0;

// The total amount of bytes decoded from the delta image
static uint32_t gTOADdecode_totalDecodedLen = 0;

// The length of the run for a given run-length encoded symbol
static uint8_t gRunLengthDecode_rleCount = 0;

// Indicates if the symbol is run-length encoded
static bool gRunLengthDecode_isRleEncoded = false;

// Indicates if the TOAD module has seen the start of the delta payload
static bool gSeenMagic = false;
static TOAD_state_t gToadState = TOAD_State_uninitialized;

// Debug variables
//static uint32_t deltaBytesRead;

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/

static uint32_t readOADHeader(uint8_t* pWriteBuffer);
static uint32_t runLengthDecode(uint8_t* pWriteBuffer, uint32_t readSize);

/******************************************************************************
 * Public TOAD API
 *****************************************************************************/

/*********************************************************************
 * @fn          TOAD_init
 *
 * @brief       Initialize the TOAD module. This function must be called
 *                  before any other TOAD functions.
 *
 * @param       params - Pointer to a TOAD_params_t struct
 *
 * @return      TOAD_Status_success if the module was initialized successfully.
 *              Alternatively, this can return TOAD_Status_noMemory error
 *              if memory is available on the heap to initialize the module or
 *              TOAD_Status_initializationError if the parameter structure
 *              was improperly initialized.
 */
TOAD_status_t TOAD_init(TOAD_params_t* params)
{
    if (gToadState == TOAD_State_uninitialized)
    {
        if (params == NULL)
        {
            TOAD_paramsInit(&gToadParams);
        }
        else
        {
            memcpy(&gToadParams, params, sizeof(TOAD_params_t));
        }

        if (gToadParams.pfnWriteDataCb == NULL ||
            gToadParams.oadBlockCacheSize < gToadParams.oadBlockSize)
        {
            return TOAD_Status_initializationError;
        }

        gpOadBlockCache = malloc(gToadParams.oadBlockCacheSize);
        gpWriteBuffer = malloc(gToadParams.writeBufferSize);

        if (!gpOadBlockCache || !gpWriteBuffer)
        {
            return TOAD_Status_noMemory;
        }

        gTOADdecode_pAppImgDelta = gToadParams.pAppImg;
        RingBuf_construct(&gOadBlockCacheRingBuf, gpOadBlockCache,
                          gToadParams.oadBlockCacheSize);

        gToadState = TOAD_State_idle;
        return TOAD_Status_success;
    }
    else
    {
        return TOAD_Status_initializationError;
    }
}

/*********************************************************************
 * @fn          TOAD_paramsInit
 *
 * @brief       Initialize a TOAD_params_t struct to its default state
 *
 * @param       params - Pointer to a TOAD_params_t struct
 */
void TOAD_paramsInit(TOAD_params_t* params)
{
    if (params != NULL)
    {
        *params = gToadDefaultParams;
    }
}

/*********************************************************************
 * @fn          TOAD_close
 *
 * @brief       Close the TOAD module and release all resources and memory.
 */
void TOAD_close()
{
    if (gToadState != TOAD_State_uninitialized)
    {
        free(gpOadBlockCache);
        free(gpWriteBuffer);

        // Reset function static variables
        gSeenMagic = false;
        gTOADdecode_totalDecodedLen = 0;

        gRunLengthDecode_rleCount = 0;
        gRunLengthDecode_isRleEncoded = false;

        gToadState = TOAD_State_uninitialized;
    }
}

/*********************************************************************
 * @fn          TOAD_addBlockToCache
 *
 * @brief       Stores an OAD block to the internal block cache for decoding
 *
 * @param       pData - Block data to store in the cache
 *
 * @return      TOAD_Status_success if block was successfully
 *              added, or TOAD_Status_noMemory if no memory is
 *              available in the cache
 */
TOAD_status_t TOAD_addBlockToCache(uint8_t* pData)
{
    if (TOAD_isBlockCacheFull())
    {
        return TOAD_Status_noMemory;
    }

    for (uint32_t i = 0; i < gToadParams.oadBlockSize; i++)
    {
        RingBuf_put(&gOadBlockCacheRingBuf, pData[i]);
    }

    return TOAD_Status_success;
}

/*********************************************************************
 * @fn          TOAD_isBlockCacheFull
 *
 * @brief       Checks if the internal OAD block cache is full and
 *              cannot add another OAD block
 *
 * @return      True if there is no room to fit an additional full
 *              block in the cache. False otherwise.
 */
bool TOAD_isBlockCacheFull()
{
    return RingBuf_getCount(&gOadBlockCacheRingBuf) +
            gToadParams.oadBlockSize > gToadParams.oadBlockCacheSize;
}

/*********************************************************************
 * @fn          TOAD_isBlockCacheEmpty
 *
 * @brief       Checks if the internal OAD block cache is empty
 *
 * @return      True if the internal OAD block cache is empty.
 *              False otherwise.
 */
bool TOAD_isBlockCacheEmpty()
{
    return RingBuf_getCount(&gOadBlockCacheRingBuf) == 0;
}

/*********************************************************************
 * @fn          TOAD_getState
 *
 * @brief       Returns the current state of the TOAD module
 *
 * @return      TOAD_state_t representing the current state of the
 *              TOAD module.
 */
TOAD_state_t TOAD_getState()
{
    return gToadState;
}

/*********************************************************************
 * @fn          TOAD_decode
 *
 * @brief       Attempts to start or continue decoding the next delta block
 *              in the OAD block cache and write decoded data to flash
 *
 * @return      TOAD_Status_success if there were no issues in trying
 *              to decode the next block. Otherwise TOAD_Status_decodingError.
 */
TOAD_status_t TOAD_decode()
{
    static TOAD_ctrlBlock_t ctrlBlock = {0};

    uint32_t writeLenDiff = 0;
    uint32_t writeLenExtra = 0;
    uint32_t writeLenTotal = 0;
    uint32_t freeWriteBufferSpace = gToadParams.writeBufferSize;

    // Wait until delta payload is found before starting to decode
    if (!gSeenMagic)
    {
        writeLenTotal = readOADHeader(gpWriteBuffer);
        gTOADdecode_pAppImgDelta += writeLenTotal;
    }
    else
    {
        // Read next control block if previously it was fully decoded
        if (gToadState == TOAD_State_idle || (gToadState == TOAD_State_decodingWaiting &&
                ctrlBlock.diffReadLen == 0 && ctrlBlock.extraReadLen == 0))
        {
            if (RingBuf_getCount(&gOadBlockCacheRingBuf) < TOAD_CTRL_BLOCK_LEN)
            {
                // Not enough data to start decoding
                gToadState = TOAD_State_decodingWaiting;
                return TOAD_Status_success;
            }

            // Read next control block instruction
            for (uint32_t i = 0; i < TOAD_CTRL_BLOCK_LEN; i++)
            {
                RingBuf_get(&gOadBlockCacheRingBuf, (((uint8_t*)&ctrlBlock) + i));
            }

            //deltaBytesRead += TOAD_CTRL_BLOCK_LEN;
        }

        // Start, or continue decoding difference block
        if (ctrlBlock.diffReadLen != 0)
        {
            // Ensure data can fit in the write buffer
            writeLenDiff = ctrlBlock.diffReadLen > gToadParams.writeBufferSize ?
                    gToadParams.writeBufferSize : ctrlBlock.diffReadLen;

            // Add image data to add the delta
            for (uint32_t i = 0; i < writeLenDiff; i++)
            {
                gpWriteBuffer[i] = gToadParams.pAppImg[(uint32_t)gTOADdecode_pAppImgDelta + i];
            }

            writeLenDiff = runLengthDecode(gpWriteBuffer, writeLenDiff);

            // Update control variables
            gTOADdecode_pAppImgDelta += writeLenDiff;
            ctrlBlock.diffReadLen -= writeLenDiff;
            freeWriteBufferSpace -= writeLenDiff;
        }

        // Start, or continue decoding extra block
        if (ctrlBlock.diffReadLen == 0 && ctrlBlock.extraReadLen != 0 &&
                freeWriteBufferSpace > 0)
        {
            // Ensure data can fit in the write buffer
            writeLenExtra = ctrlBlock.extraReadLen > freeWriteBufferSpace ?
                    freeWriteBufferSpace : ctrlBlock.extraReadLen;

            if (writeLenExtra > RingBuf_getCount(&gOadBlockCacheRingBuf))
            {
                gToadState = TOAD_State_decodingWaiting;
            }
            else
            {
                gToadState = TOAD_State_decoding;
            }

            // Ensure there is enough data has been transferred
            writeLenExtra = RingBuf_getCount(&gOadBlockCacheRingBuf) >= writeLenExtra ?
                    writeLenExtra : RingBuf_getCount(&gOadBlockCacheRingBuf);

            for (uint32_t i = 0; i < writeLenExtra; i++)
            {
                RingBuf_get(&gOadBlockCacheRingBuf, &gpWriteBuffer[writeLenDiff + i]);
            }

            //deltaBytesRead += writeLenExtra;

            // Update control variables
            ctrlBlock.extraReadLen -= writeLenExtra;
        }

        writeLenTotal = writeLenDiff + writeLenExtra;

        /* Advance seekOffset pointer if control block instruction if
         * it has been fully decoded
         */
        if (ctrlBlock.diffReadLen == 0 && ctrlBlock.extraReadLen == 0 &&
                ctrlBlock.seekOffset != 0)
        {
            gTOADdecode_pAppImgDelta += ctrlBlock.seekOffset;
            ctrlBlock.seekOffset = 0;
            gToadState = TOAD_State_idle;
        }
    }

    // Write to OAD storage
    if (writeLenTotal > 0)
    {
        gTOADdecode_totalDecodedLen += writeLenTotal;

        if (gTOADdecode_totalDecodedLen > gToadParams.newImgLen)
        {
            gToadState = TOAD_State_idle;
            return TOAD_Status_decodingError;
        }

        gToadParams.pfnWriteDataCb(gpWriteBuffer, writeLenTotal);
    }

    // Set the ending state if not previously set when decoding
    if (gToadState != TOAD_State_decodingWaiting)
    {
        if (ctrlBlock.diffReadLen > 0 || ctrlBlock.extraReadLen > 0)
        {
            gToadState = TOAD_State_decoding;
        }
        else
        {
            gToadState = (gTOADdecode_totalDecodedLen == gToadParams.newImgLen) ?
                    TOAD_State_decodingDone : TOAD_State_idle;
        }

    }

    return TOAD_Status_success;
}

/******************************************************************************
 * Private TOAD functions
 *****************************************************************************/

/******************************************************************************
 * @fn     readOADHeader
 *
 * @brief  Reads the OAD head from the OAD block cache and sets the global
 *         flag when the delta payload magic word has been seen
 *
 * @param  pWriteBuffer - pointer to the buffer to write the OAD header
 *
 * @return The amount of data written into the write buffer
 */
static uint32_t readOADHeader(uint8_t* pWriteBuffer)
{
    uint32_t writeBufferIndex = 0;
    char readVal;

    while (RingBuf_getCount(&gOadBlockCacheRingBuf) > 0)
    {
        RingBuf_peek(&gOadBlockCacheRingBuf, (uint8_t*)&readVal);

        // Perform further processing if found the starting character
        if (readVal == TOAD_MAGIC_WORD_START_CHAR)
        {
            if (RingBuf_getCount(&gOadBlockCacheRingBuf) >= TOAD_MAGIC_WORD_LEN)
            {
                bool isMatch = true;

                for (int i = 0; i < TOAD_MAGIC_WORD_LEN; i++)
                {
                    RingBuf_peek(&gOadBlockCacheRingBuf, (uint8_t*)&readVal);

                    if (TOAD_MAGIC_WORD[i] != readVal)
                    {
                        // Failed comparison. Search for the next start char
                        isMatch = false;
                        break;
                    }

                    RingBuf_get(&gOadBlockCacheRingBuf, &pWriteBuffer[writeBufferIndex]);
                    writeBufferIndex++;

                    //deltaBytesRead++;
                }

                if (!isMatch)
                {
                    continue;
                }

                // Magic word should not be written to flash
                writeBufferIndex -= TOAD_MAGIC_WORD_LEN;
                gSeenMagic = true;
                break;
            }
            else
            {
                // Wait until more data arrives
                break;
            }
        }
        else
        {
            // Write OAD header data to buffer
            RingBuf_get(&gOadBlockCacheRingBuf, &pWriteBuffer[writeBufferIndex]);
            writeBufferIndex++;

            //deltaBytesRead++;
        }

    }

    return writeBufferIndex;
}

/******************************************************************************
 * @fn     runLengthDecode
 *
 * @brief  Decodes a compressed RLE block into uncompressed binary data
 *
 * @param  pWriteBuffer - address of the write buffer
 * @param  readSize     - the amount of data to read in the RLE block
 *
 * @return The amount of data written into the write buffer
 */
static uint32_t runLengthDecode(uint8_t* pWriteBuffer, uint32_t readSize)
{
    uint8_t readValue = 0;

    // Abort early if block cache is empty
    if (TOAD_isBlockCacheEmpty())
    {
        gToadState = TOAD_State_decodingWaiting;
        return 0;
    }

    gToadState = TOAD_State_decoding;

    for (uint32_t i = 0; i < readSize; i++)
    {
        if (!gRunLengthDecode_isRleEncoded)
        {
            if (!RingBuf_peek(&gOadBlockCacheRingBuf, &readValue))
            {
                return i;
            }

            if (readValue == 0)
            {
                if (RingBuf_getCount(&gOadBlockCacheRingBuf) > 1)
                {
                    gRunLengthDecode_isRleEncoded = true;
                    RingBuf_get(&gOadBlockCacheRingBuf, &readValue);
                    RingBuf_get(&gOadBlockCacheRingBuf, &gRunLengthDecode_rleCount);

                    //deltaBytesRead += 2;
                }
                else
                {
                    // Wait until more data arrives to determine the run-length
                    gToadState = TOAD_State_decodingWaiting;
                    return i;
                }

            }
        }

        if (gRunLengthDecode_isRleEncoded)
        {
            // no need to add 0 to the image data
            gRunLengthDecode_rleCount--;

            if (gRunLengthDecode_rleCount == 0)
            {
                gRunLengthDecode_isRleEncoded = false;
            }
        }
        else
        {
            // Write non run-length encoded symbol to the write buffer
            if (RingBuf_get(&gOadBlockCacheRingBuf, &readValue) == -1)
            {
                return i;
            }

            pWriteBuffer[i] += readValue;
            //deltaBytesRead++;
        }
    }

    return readSize;
}

#endif
