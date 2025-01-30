/******************************************************************************

 @file  npi_frame.h

 @brief This file contains the Network Processor Interface (NPI) data frame
        specific functions definitions.

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
#ifndef NPIFRAME_H
#define NPIFRAME_H

#ifdef __cplusplus
extern "C"
{
#endif

// ****************************************************************************
// includes
// ****************************************************************************
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_types.h)
#include "npi_data.h"
#ifdef CC33xx
#include "npi_config.h"
#endif

// ****************************************************************************
// defines
// ****************************************************************************
#ifdef CC33xx
// CC33xx defines used for the new NPIFRAMEHCI structure
#define NPIFRAMEHCI_CMD_PKT_HDR_LEN 4
#define NPIFRAMEHCI_EXTENDED_CMD_PKT_HDR_LEN 5
#define NPIFRAMEHCI_DATA_PKT_HDR_LEN 5
#define NPIFRAMEHCI_RX_BUF_SIZE NPI_TL_BUF_SIZE
#endif // CC33xx

// ****************************************************************************
// typedefs
// ****************************************************************************
#ifdef CC33xx
// States for Command and Data packet parser
typedef enum
{
    NPIFRAMEHCI_STATE_PKT_TYPE                = 0,
    NPIFRAMEHCI_CMD_STATE_OPCODE0             = 1,
    NPIFRAMEHCI_CMD_STATE_OPCODE1             = 2,
    NPIFRAMEHCI_CMD_STATE_LENGTH0             = 3,
    NPIFRAMEHCI_CMD_STATE_LENGTH1             = 4,
    NPIFRAMEHCI_CMD_STATE_DATA                = 5,
    NPIFRAMEHCI_DATA_STATE_HANDLE0            = 6,
    NPIFRAMEHCI_DATA_STATE_HANDLE1            = 7,
    NPIFRAMEHCI_DATA_STATE_LENGTH0            = 8,
    NPIFRAMEHCI_DATA_STATE_LENGTH1            = 9,
    NPIFRAMEHCI_DATA_STATE_DATA               = 10,
    NPIFRAMEHCI_STATE_FLUSH                   = 11,
    NPIFRAMEHCI_STATE_SDIO_HEADER_IDENIFIED   = 12,
} NPIFrameHCI_State_e;

//HCI packet meta data
typedef struct
{
    NPIFrameHCI_State_e state;
    uint8_t PKT_Token;
    uint16_t OPCODE_Token;
    uint16_t HANDLE_Token;
    uint16_t LEN_Token;
    uint16_t HeaderLength;
    uint8 RxBuffer[NPIFRAMEHCI_RX_BUF_SIZE];
} NPIFrameHCI_Params_t;

//! \brief typedef for call back function to return a complete NPI message.
//!        The npiFrame module stores the HCI packet meta data of the
//!        complete message and returns via this callback the received message.
//!        NOTE: There is no framing elements (i.e. Start of Frame, FCS/CRC or similar)
typedef void (*npiIncomingFrameCBack_t)( NPIFrameHCI_Params_t *pFrame );
#else
//! \brief typedef for call back function to return a complete NPI message.
//!        The npiFrame module encapsulates the collecting/parsing of the
//!        complete message and returns via this callback the received message.
//!        NOTE: the message buffer does NOT include the framing elements
//!        (i.e. Start of Frame, FCS/CRC or similar).
typedef void (*npiIncomingFrameCBack_t)( uint16_t frameSize, uint8_t *pFrame,
                                         NPIMSG_Type msgType );
#endif // CC33xx

//*****************************************************************************
// globals
//*****************************************************************************

//*****************************************************************************
// function prototypes
//*****************************************************************************
// ----------------------------------------------------------------------------
//! \brief      Initialize Frame module with NPI callbacks.
//!
//! \param[in]  incomingFrameCB   Call back for complete inbound (from host)
//!                               messages
//!
//! \return     void
// ----------------------------------------------------------------------------
extern void NPIFrame_initialize(npiIncomingFrameCBack_t incomingFrameCB);


// ----------------------------------------------------------------------------
//! \brief      Bundles message into Transport Layer frame and NPIMSG_msg_t
//!             container.  A transport layer specific version of this function
//!             must be implemented.
//!
//! \param[in]  pData     Pointer to message buffer.
//!
//! \return     void
// ----------------------------------------------------------------------------
extern NPIMSG_msg_t * NPIFrame_frameMsg(uint8_t *pIncomingMsg);

#ifndef CC33xx
// ----------------------------------------------------------------------------
//! \brief      Collects serial message buffer.  Called based on events
//!             received from the transport layer.  When an entire message has
//!             been successfully received, it is passed back to NPI task via
//!             the callback function above: npiIncomingFrameCBack_t.
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPIFrame_collectFrameData(void);
#else
// ----------------------------------------------------------------------------
//! \brief      Collects serial message buffer.  Called based on events
//!             received from the transport layer.  When an entire message has
//!             been successfully received, it is passed back to NPI task via
//!             the callback function above: npiIncomingFrameCBack_t.
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPIFrame_uartCollectFrameData(void);

// ----------------------------------------------------------------------------
//! \brief      Collects SDIO message buffer.  Called based on events
//!             received from the transport layer.  When an entire message has
//!             been successfully received, it is passed back to NPI task via
//!             the callback function above: npiIncomingFrameCBack_t.
//!
//! \return     void
// -----------------------------------------------------------------------------
extern void NPIFrame_sdioCollectFrameData(void);
#endif // CC33xx

#ifdef __cplusplus
}
#endif

#endif /* NPIFRAME_H */
