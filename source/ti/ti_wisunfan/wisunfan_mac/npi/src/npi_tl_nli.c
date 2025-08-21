/******************************************************************************

 @file  npi_tl_nli.c

 @brief NPI Transport Layer interface for spinel mux

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

// ****************************************************************************
// includes
// ****************************************************************************
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "mt.h"

#include "hal_types.h"
#include <ti_drivers_config.h>
#include "inc/npi_config.h"


#include "npi_tl_nli.h"
#include <nli_mux.h>

/* OpenThread Internal/Example Header files */
#include "otsupport/otrtosapi.h"
#include "otsupport/otinstance.h"

#define TI154_NLI_ID (1)



// ****************************************************************************
// defines
// ****************************************************************************

// ****************************************************************************
// typedefs
// ****************************************************************************

//*****************************************************************************
// globals
//*****************************************************************************

//! \brief NPI TL call back function for the end of a UART transaction
static npiCB_t npiTransmitCB = NULL;

//! \brief Pointer to NPI TL TX Buffer
static uint8_t* TransportRxBuf;

//! \brief Pointer to NPI TL RX Buffer
static uint8_t* TransportTxBuf;

//! \brief Length of bytes to send from NPI TL Tx Buffer
static uint16 TransportTxLen = 0;


//*****************************************************************************
// function prototypes
//*****************************************************************************


/**
 * Callback for indicating a new frame has been received. The buffer is not
 * stored in static memory.
 */
 static void NPITLNLI_readCallBack(const uint8_t *aBuf, uint16_t aBufLength, void *aContext);

/**
 * Callback for indicating a frame has been sent over the transport. This can
 * be used to re-claim resources used by the buffer.
 */
 static void NPITLNLI_writeCallBack(const uint8_t *aBuf, uint16_t aBufLength, void *aContext);


// -----------------------------------------------------------------------------
//! \brief      This routine initializes the transport layer and opens the port
//!             of the device.
//!
//! \param[in]  tRxBuf - pointer to NPI TL Tx Buffer
//! \param[in]  tTxBuf - pointer to NPI TL Rx Buffer
//! \param[in]  npiCBack - NPI TL call back function to be invoked at the end of
//!             a UART transaction
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITLNLI_initializeTransport(char *tRxBuf, char *tTxBuf, npiCB_t npiCBack)
{
    TransportRxBuf = (uint8_t*)tRxBuf;
    TransportTxBuf = (uint8_t*)tTxBuf;
    npiTransmitCB = npiCBack;

    /*Register with NLI */
    OtRtosApi_lock();
    // IsSpinel set to false to ignore NLI byte by MAC
    SetNliCallback(TI154_NLI_ID, NPITLNLI_readCallBack, NPITLNLI_writeCallBack, NULL, false);
    OtRtosApi_unlock();

    return;
}

// -----------------------------------------------------------------------------
//! \brief      This callback is invoked on Write completion
//!
//! \param[in]  handle - handle to the UART port
//! \param[in]  ptr    - pointer to data to be transmitted
//! \param[in]  size   - size of the data
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITLNLI_writeCallBack(const uint8_t *aBuf, uint16_t aBufLength, void *aContext)
{
    MAP_ICall_CSState key;
    key = MAP_ICall_enterCriticalSection();

    if ( npiTransmitCB )
    {
        npiTransmitCB(0,TransportTxLen);
    }

    MAP_ICall_leaveCriticalSection(key);
}

// -----------------------------------------------------------------------------
//! \brief      This callback is invoked on Read completion of readSize/receive
//!             timeout
//!
//! \param[in]  handle - handle to the UART port
//! \param[in]  ptr    - pointer to buffer to read data into
//! \param[in]  size   - size of the data
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITLNLI_readCallBack(const uint8_t *aBuf, uint16_t aBufLength, void *aContext)
{
    MAP_ICall_CSState key;
    key = MAP_ICall_enterCriticalSection();

    memcpy(TransportRxBuf, aBuf, aBufLength);

    if ( npiTransmitCB )
    {
        npiTransmitCB(aBufLength, 0);
    }

    MAP_ICall_leaveCriticalSection(key);
}


// -----------------------------------------------------------------------------
//! \brief      This routine writes copies buffer addr to the transport layer.
//!
//! \param[in]  len - Number of bytes to write.
//!
//! \return     uint8 - number of bytes written to transport
// -----------------------------------------------------------------------------
uint16 NPITLNLI_writeTransport(uint16 len)
{
    MAP_ICall_CSState key;
    key = MAP_ICall_enterCriticalSection();

    TransportTxLen = len;

    SendNli(TI154_NLI_ID, (const uint8_t*)TransportTxBuf, TransportTxLen);

    MAP_ICall_leaveCriticalSection(key);

    return TransportTxLen;
}
