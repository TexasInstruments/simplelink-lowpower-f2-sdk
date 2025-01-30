/******************************************************************************

 @file  npi_tl.c

 @brief NPI Transport Layer API

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
#include <ti/drivers/GPIO.h>
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include "hal_types.h"

#include "inc/npi_tl.h"
#include "inc/npi_data.h"
#include "inc/npi_util.h"

// ****************************************************************************
// defines
// ****************************************************************************

#if defined(NPI_USE_SPI)
#include "inc/npi_tl_spi.h"
#elif defined(NPI_USE_UART)
#include "inc/npi_tl_uart.h"
#else
#error "Must define an underlying serial bus for NPI"
#endif //NPI_USE_SPI

#if (NPI_FLOW_CTRL == 1)

#define LocRDY_ENABLE()      GPIO_write(LOC_RDY_GPIO, 0)
#define LocRDY_DISABLE()     GPIO_write(LOC_RDY_GPIO, 1)
#else
#define LocRDY_ENABLE()
#define LocRDY_DISABLE()
#endif // NPI_FLOW_CTRL = 1

// ****************************************************************************
// typedefs
// ****************************************************************************

//*****************************************************************************
// globals
//*****************************************************************************

//! \brief Flag for low power mode
static volatile bool npiPMSetConstraint = FALSE;

//! \brief Flag for ongoing NPI TX
static volatile bool npiTxActive = FALSE;
static volatile bool npiRxActive = FALSE;

//! \brief The packet that was being sent when MRDY HWI negedge was received
static volatile uint32_t mrdyPktStamp = 0;

//! \brief Packets transmitted counter
static uint32_t txPktCount = 0;

//! \brief NPI Transport Layer receive buffer
uint8_t *npiRxBuf;

//! \brief Index to last byte written into NPI Transport Layer receive buffer
static uint16_t npiRxBufTail = 0;

//! \brief Index to first byte to read from NPI Transport Layer receive buffer
static uint16_t npiRxBufHead = 0;

//! \brief NPI Transport Layer transmit buffer
uint8_t *npiTxBuf;

//! \brief Number of bytes in NPI Transport Layer transmit buffer
static uint16_t npiTxBufLen = 0;

//! \brief Size of allocated Tx and Rx buffers
uint16_t npiBufSize = 0;

npiTLCallBacks taskCBs;

#if (NPI_FLOW_CTRL == 1)
//! \brief GPIO PinConfig for Mrdy and Srdy signals without GPIO index

static GPIO_PinConfig REM_RDY_GPIO_CONFIG = GPIO_CFG_INPUT_INTERNAL | GPIO_CFG_IN_INT_BOTH_EDGES | GPIO_CFG_PULL_UP_INTERNAL;
static GPIO_PinConfig LOC_RDY_GPIO_CONFIG = GPIO_CFG_OUTPUT_INTERNAL | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_HIGH;
static uint_least8_t REM_RDY_GPIO;
static uint_least8_t LOC_RDY_GPIO;

//! \brief No way to detect whether positive or negative edge with GPIO Driver
//!        Use a flag to keep track of state
static uint8_t remRdy_state;
#endif // NPI_FLOW_CTRL = 1

//*****************************************************************************
// function prototypes
//*****************************************************************************

//! \brief Call back function provided to underlying serial interface to be
//         invoked upon the completion of a transmission
static void NPITL_transmissionCallBack(uint16_t Rxlen, uint16_t Txlen);

#if (NPI_FLOW_CTRL == 1)
//! \brief HWI interrupt function for remRdy
static void NPITL_remRdyGPIOHwiFxn(uint_least8_t index);

//! \brief This routine is used to set constraints on power manager
static void NPITL_setPM(void);

//! \brief This routine is used to release constraints on power manager
static void NPITL_relPM(void);
#endif // NPI_FLOW_CTRL = 1

// -----------------------------------------------------------------------------
//! \brief      This routine initializes the transport layer and opens the port
//!             of the device. Note that based on project defines, either the
//!             UART, or SPI driver can be used.
//!
//! \param[in]  params - Transport Layer parameters
//!
//! \return     uint8_t   Status NPI_SUCCESS, or NPI_TASK_FAILURE
// -----------------------------------------------------------------------------
uint8_t NPITL_openTL(NPITL_Params *params)
{
    _npiCSKey_t key;
    key = NPIUtil_EnterCS();

    // Set NPI Task Call backs
    memcpy(&taskCBs, &params->npiCallBacks, sizeof(params->npiCallBacks));

    // Allocate memory for Transport Layer Tx/Rx buffers
    npiBufSize = params->npiTLBufSize;
    if ((npiRxBuf = NPIUtil_malloc(params->npiTLBufSize)) != NULL )
    {
       memset(npiRxBuf, 0, npiBufSize);
    }
    else
    {
        NPIUtil_ExitCS(key);
        return NPI_TASK_FAILURE;
    }
    if ((npiTxBuf = NPIUtil_malloc(params->npiTLBufSize)) != NULL )
    {
       memset(npiTxBuf, 0, npiBufSize);
    }
    else
    {
        NPIUtil_ExitCS(key);
        return NPI_TASK_FAILURE;
    }
    // This will be updated to be able to select SPI/UART TL at runtime
    // Now only compile time with the NPI_USE_[UART,SPI] flag
#if defined(NPI_USE_UART)
    transportOpen(params->portBoardID,
                  &params->portParams.uartParams,
                  NPITL_transmissionCallBack);
#elif defined(NPI_USE_SPI)
    transportOpen(params->portBoardID,
                  &params->portParams.spiParams,
                  NPITL_transmissionCallBack);
#endif //NPI_USE_UART

#if (NPI_FLOW_CTRL == 1)
    // Assign GPIO index to remRdy and locRrdy
#ifdef NPI_CENTRAL
    REM_RDY_GPIO = params->srdyGpioIndex;
    LOC_RDY_GPIO = params->mrdyGpioIndex;
#else
    REM_RDY_GPIO = params->mrdyGpioIndex;
    LOC_RDY_GPIO = params->srdyGpioIndex;
#endif //NPI_CENTRAL

    // Add GPIO index to GPIO_PinConfig
    GPIO_setConfig(REM_RDY_GPIO, REM_RDY_GPIO_CONFIG);
    GPIO_setConfig(LOC_RDY_GPIO, LOC_RDY_GPIO_CONFIG);

    // set callback
    GPIO_setCallback(REM_RDY_GPIO, NPITL_remRdyGPIOHwiFxn);
    // Enable interrupt
    GPIO_enableInt(REM_RDY_GPIO);

    remRdy_state = GPIO_read(REM_RDY_GPIO);

    // If MRDY is already low then we must initiate a read because there was
    // a prior MRDY negedge that was missed
    if (!remRdy_state)
    {
        NPITL_setPM();
        if (taskCBs.remRdyCB)
        {
            transportRemRdyEvent();
            LocRDY_ENABLE();
        }
    }
#endif // NPI_FLOW_CTRL = 1

#if (NPI_FLOW_CTRL == 0)
    // This call will start repeated Uart Reads when Power Savings is disabled
    transportRead();
#endif // NPI_FLOW_CTRL = 0

    NPIUtil_ExitCS(key);
    return NPI_SUCCESS;
}

// -----------------------------------------------------------------------------
//! \brief      This routine closes the transport layer
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITL_closeTL(void)
{
    _npiCSKey_t key;
    key = NPIUtil_EnterCS();

    // Clear NPI Task Call backs
    memset(&taskCBs, 0, sizeof(taskCBs));

    // Free Transport Layer RX/TX buffers
    npiBufSize = 0;
    NPIUtil_free(npiRxBuf);
    NPIUtil_free(npiTxBuf);

    // Close Transport Layer
    transportClose();

#if (NPI_FLOW_CTRL == 1)
    // GPIO_resetConfig for REM_RDY_GPIO and LOC_RDY_GPIO
    GPIO_resetConfig(REM_RDY_GPIO);
    GPIO_resetConfig(LOC_RDY_GPIO);

    // Release Power Management
    NPITL_relPM();
#endif // NPI_FLOW_CTRL = 1

    NPIUtil_ExitCS(key);
}

// -----------------------------------------------------------------------------
//! \brief      This routine returns the state of transmission on NPI
//!
//! \return     bool - state of NPI transmission - 1 - active, 0 - not active
// -----------------------------------------------------------------------------
bool NPITL_checkNpiBusy(void)
{
#if (NPI_FLOW_CTRL == 1)
#ifdef NPI_CENTRAL
    return !GPIO_read(LOC_RDY_GPIO) || npiRxActive;
#else
    return !GPIO_read(LOC_RDY_GPIO);
#endif //NPI_CENTRAL
#else
    return npiTxActive;
#endif // NPI_FLOW_CTRL = 1
}

#if (NPI_FLOW_CTRL == 1)
// -----------------------------------------------------------------------------
//! \brief      This routine is used to set constraints on power manager
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITL_setPM(void)
{
    if (npiPMSetConstraint)
    {
        return;
    }

    // set constraints for Standby and idle mode
    Power_setConstraint(PowerCC26XX_SB_DISALLOW);
    Power_setConstraint(PowerCC26XX_IDLE_PD_DISALLOW);
    npiPMSetConstraint = TRUE;
}
#endif // NPI_FLOW_CTRL = 1

#if (NPI_FLOW_CTRL == 1)
// -----------------------------------------------------------------------------
//! \brief      This routine is used to release constraints on power manager
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITL_relPM(void)
{
    if (!npiPMSetConstraint)
    {
        return;
    }

    // release constraints for Standby and idle mode
    Power_releaseConstraint(PowerCC26XX_SB_DISALLOW);
    Power_releaseConstraint(PowerCC26XX_IDLE_PD_DISALLOW);
    npiPMSetConstraint = FALSE;
}
#endif // NPI_FLOW_CTRL = 1

#if (NPI_FLOW_CTRL == 1)
// -----------------------------------------------------------------------------
//! \brief      This routine is used to handle an MRDY transition from a task
//!             context. Certain operations such as UART_read() cannot be
//!             performed from a HWI context
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITL_handleRemRdyEvent(void)
{
    _npiCSKey_t key;
    key = NPIUtil_EnterCS();

    // Check to make sure this event is not occurring during the next packet
    // transmission
    if (GPIO_read(REM_RDY_GPIO) == 0 ||
        (npiTxActive && mrdyPktStamp == txPktCount))
    {
        transportRemRdyEvent();
        npiRxActive = TRUE;
        LocRDY_ENABLE();
    }

    NPIUtil_ExitCS(key);
}
#endif // NPI_FLOW_CTRL = 1

#if (NPI_FLOW_CTRL == 1)
// -----------------------------------------------------------------------------
//! \brief      This is a HWI function handler for the MRDY GPIO. Some MRDY
//!             functionality can execute from this HWI context. Others
//!             must be executed from task context hence the taskCBs.remRdyCB()
//!
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITL_remRdyGPIOHwiFxn(uint_least8_t index)
{
    // The gpio driver does not currently support returning whether the int
    // was neg or pos edge so we must use a variable to keep track of state.
    remRdy_state ^= 1;

    if (remRdy_state == 0)
    {
        mrdyPktStamp = txPktCount;
        NPITL_setPM();
    }
    else
    {
        transportStopTransfer();
        npiRxActive = FALSE;
    }

    // Signal to registered task that Rem Ready signal has changed state
    if (taskCBs.remRdyCB)
    {
        taskCBs.remRdyCB(remRdy_state);
    }

    // Check the physical state of the gpio to see if it matches the variable
    // state. If not trigger another task call back
    if (remRdy_state != GPIO_read(REM_RDY_GPIO))
    {
        remRdy_state = GPIO_read(REM_RDY_GPIO);

        if (taskCBs.remRdyCB)
        {
            taskCBs.remRdyCB(remRdy_state);
        }
    }
}
#endif // NPI_FLOW_CTRL = 1

// -----------------------------------------------------------------------------
//! \brief      This callback is invoked on the completion of one transmission
//!             to/from the host MCU. Any bytes receives will be [0,Rxlen) in
//!             npiRxBuf.
//!             If bytes were receives or transmitted, this function notifies
//!             the NPI task via registered call backs
//!
//! \param[in]  Rxlen   - length of the data received
//! \param[in]  Txlen   - length of the data transferred
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITL_transmissionCallBack(uint16_t Rxlen, uint16_t Txlen)
{
    npiRxBufHead = 0;
    npiRxBufTail = Rxlen;

    // Only set TX Active flag false if a tx was taking place
    if (Txlen > 0)
    {
      npiTxActive = FALSE;
    }

    // If Task is registered, invoke transaction complete callback
    if (taskCBs.transCompleteCB)
    {
        taskCBs.transCompleteCB(Rxlen, Txlen);
    }

#if (NPI_FLOW_CTRL == 1)
    NPITL_relPM();
    LocRDY_DISABLE();
#endif // NPI_FLOW_CTRL = 1
}

// -----------------------------------------------------------------------------
//! \brief      This routine reads data from the transport layer based on len,
//!             and places it into the buffer.
//!
//! \param[in]  buf - Pointer to buffer to place read data.
//! \param[in]  len - Number of bytes to read.
//!
//! \return     uint16_t - the number of bytes read from transport
// -----------------------------------------------------------------------------
uint16_t NPITL_readTL(uint8_t *buf, uint16_t len)
{
    // Only copy the lowest number between len and bytes remaining in buffer
    len = (len > NPITL_getRxBufLen()) ? NPITL_getRxBufLen() : len;

    memcpy(buf, &npiRxBuf[npiRxBufHead], len);
    npiRxBufHead += len;

    return len;
}

// -----------------------------------------------------------------------------
//! \brief      This routine writes data from the buffer to the transport layer.
//!
//! \param[in]  buf - Pointer to buffer to write data from.
//! \param[in]  len - Number of bytes to write.
//!
//! \return     uint16_t - NPI error code value
// -----------------------------------------------------------------------------
uint8_t NPITL_writeTL(uint8_t *buf, uint16_t len)
{
#if (NPI_FLOW_CTRL == 1)
    _npiCSKey_t key;
    key = NPIUtil_EnterCS();
#endif // NPI_FLOW_CTRL = 1

    // Check to make sure NPI is not currently in a transaction
    if (NPITL_checkNpiBusy())
    {
#if (NPI_FLOW_CTRL == 1)
        NPIUtil_ExitCS(key);
#endif // NPI_FLOW_CTRL = 1

        return NPI_BUSY;
    }

    // Check to make sure that write size is not greater than what is
    // allowed
    if (len > npiBufSize)
    {
#if (NPI_FLOW_CTRL == 1)
        NPIUtil_ExitCS(key);
#endif // NPI_FLOW_CTRL = 1

        return NPI_TX_MSG_OVERSIZE;
    }

    // Copy into the second byte of npiTxBuf. This will save Serial Port
    // Specific TL code from having to shift one byte later on for SOF.
    memcpy(&npiTxBuf[1], buf, len);
    npiTxBufLen = len;
    npiTxActive = TRUE;
    txPktCount++;

    transportWrite(npiTxBufLen);

#if (NPI_FLOW_CTRL == 1)
    LocRDY_ENABLE();
    NPIUtil_ExitCS(key);
#endif // NPI_FLOW_CTRL = 1

    return NPI_SUCCESS;
}

// -----------------------------------------------------------------------------
//! \brief      This routine writes data from the buffer to the transport layer
//!             and bypasses flow control and len check. Used for emergency
//!             assert transmission if device is failing.
//!
//! \param[in]  buf - Pointer to buffer to write data from.
//! \param[in]  len - Number of bytes to write. Must be lest the txBuff size
//!
//! \return     None
// -----------------------------------------------------------------------------
void NPITL_writeBypassSafeTL(uint8_t *buf, uint16_t len)
{
#if (NPI_FLOW_CTRL == 1)
    _npiCSKey_t key;
    key = NPIUtil_EnterCS();
#endif // NPI_FLOW_CTRL = 1

    // Copy into the second byte of npiTxBuf. This will save Serial Port
    // Specific TL code from having to shift one byte later on for SOF.
    memcpy(&npiTxBuf[1], buf, len);
    npiTxBufLen = len;
    npiTxActive = TRUE;
    txPktCount++;

    transportWrite(npiTxBufLen);

#if (NPI_FLOW_CTRL == 1)
    LocRDY_ENABLE();
    NPIUtil_ExitCS(key);
#endif // NPI_FLOW_CTRL = 1
}

// -----------------------------------------------------------------------------
//! \brief      This routine returns the max size receive buffer.
//!
//! \return     uint16_t - max size of the receive buffer
// -----------------------------------------------------------------------------
uint16_t NPITL_getMaxRxBufSize(void)
{
    return(npiBufSize);
}

// -----------------------------------------------------------------------------
//! \brief      This routine returns the max size transmit buffer.
//!
//! \return     uint16_t - max size of the transmit buffer
// -----------------------------------------------------------------------------
uint16_t NPITL_getMaxTxBufSize(void)
{
    return(npiBufSize);
}

// -----------------------------------------------------------------------------
//! \brief      Returns number of bytes that are unread in RxBuf
//!
//! \return     uint16_t - number of unread bytes
// -----------------------------------------------------------------------------
uint16_t NPITL_getRxBufLen(void)
{
    return ((npiRxBufTail - npiRxBufHead) + npiBufSize) % npiBufSize;
}
