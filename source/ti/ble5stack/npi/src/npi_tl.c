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
#include <xdc/std.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Swi.h>

#include <ti/drivers/GPIO.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include <ti_drivers_config.h>
#include "icall.h"
#include "hal_types.h"
#include "inc/npi_tl.h"
#include "inc/npi_config.h"
#include <semaphore.h>

// ****************************************************************************
// defines
// ****************************************************************************

#if defined(NPI_USE_SPI)
#include "inc/npi_tl_spi.h"
#elif defined(NPI_USE_UART)
#include "inc/npi_tl_uart.h"
#else
#error Must define an underlying serial bus for NPI
#endif


// ****************************************************************************
// typedefs
// ****************************************************************************

//*****************************************************************************
// globals
//*****************************************************************************

#if (NPI_FLOW_CTRL == 1)
//! \brief Flag for low power mode
static volatile bool npiPMSetConstraint = FALSE;
#endif

#if (NPI_FLOW_CTRL == 1)
static volatile bool npiTxActive = FALSE;
#else
//! \brief Flag for ongoing NPI TX
static sem_t npiTxActiveLock;
#endif

#if (NPI_FLOW_CTRL == 1)
//! \brief The packet that was being sent when HWI of MRDY going low was received
static volatile uint32 mrdyPktStamp = 0;
#endif

//! \brief Packets transmitted counter
static uint32 txPktCount = 0;

//! \brief NPI Transport Layer receive buffer
static Char npiRxBuf[NPI_TL_BUF_SIZE];

//! \brief Index to last byte written into NPI Transport Layer receive buffer
static uint16_t npiRxBufTail = 0;

//! \brief Index to first byte to be read from NPI Transport Layer receive buffer
static uint16_t npiRxBufHead = 0;

//! \brief NPI Transport Layer transmit buffer
static Char npiTxBuf[NPI_TL_BUF_SIZE];

//! \brief Call back function in NPI Task for transmit complete
static npiRtosCB_t taskTxCB = NULL;

//! \brief Call back function in NPI Task for receive complete
static npiRtosCB_t taskRxCB = NULL;

//! \brief The remainder of any message that is fragmented
static uint8 *msgFrag = NULL;

//! \brief The length of the remaining message fragment
static uint16 msgFragLen = 0;

#if (NPI_FLOW_CTRL == 1)
//! \brief Call back function in NPI Task for MRDY hardware interrupt
static npiMrdyRtosCB_t taskMrdyCB = NULL;

//! \brief No way to detect whether positive or negative edge with PIN Driver
//!             Use a flag to keep track of state
static uint8 mrdy_state;
#endif // NPI_FLOW_CTRL = 1

//*****************************************************************************
// function prototypes
//*****************************************************************************

//! \brief Call back function provided to underlying serial interface to be
//              invoked upon the completion of a transmission
static void NPITL_transmissionCallBack(uint16 Rxlen, uint16 Txlen);

#if (NPI_FLOW_CTRL == 1)
//! \brief HWI interrupt function for MRDY
static void NPITL_MRDYPinHwiFxn(uint_least8_t index);
#endif // NPI_FLOW_CTRL = 1

// -----------------------------------------------------------------------------
//! \brief      This routine initializes the transport layer and opens the port
//!             of the device. Note that based on project defines, either the
//!             UART, or SPI driver can be used.
//!
//! \param[in]  npiCBTx - Call back function for TX complete event
//! \param[in]  npiCBRx - Call back function for RX event
//! \param[in]  npiCBMrdy - Call back function for MRDY event
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITL_initTL(npiRtosCB_t npiCBTx, npiRtosCB_t npiCBRx, npiRtosCB_t npiCBMrdy)
{
    ICall_CSState key;
    key = ICall_enterCriticalSection();

    taskTxCB = npiCBTx;
    taskRxCB = npiCBRx;
#if (NPI_FLOW_CTRL == 1)
    taskMrdyCB = npiCBMrdy;
#endif // NPI_FLOW_CTRL = 1

    transportInit(npiRxBuf,npiTxBuf, NPITL_transmissionCallBack);

#if (NPI_FLOW_CTRL == 1)
    SRDY_DISABLE();

    // Initialize SRDY/MRDY
    GPIO_setConfig(MRDY_PIN, GPIO_CFG_INPUT_INTERNAL | GPIO_CFG_IN_INT_BOTH_EDGES | GPIO_CFG_PULL_UP_INTERNAL);
    GPIO_setConfig(SRDY_PIN, GPIO_CFG_OUTPUT_INTERNAL | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_HIGH);
    // set callback
    GPIO_setCallback(MRDY_PIN, NPITL_MRDYPinHwiFxn);
    // Enable interrupt
    GPIO_enableInt(MRDY_PIN);
    mrdy_state = GPIO_read(MRDY_PIN);
#else
    /* Create a binary semaphore */
    sem_init(&npiTxActiveLock, 0, 1);
#endif // NPI_FLOW_CTRL = 1

    ICall_leaveCriticalSection(key);

    return;
}

#if (NPI_FLOW_CTRL == 1)
// -----------------------------------------------------------------------------
//! \brief      This routine reads the SRDY_PIN which represents the peripheral
//!             readiness for NPI transaction in Flow Control scenario
//!
//! \return     bool - state of FLOW_CTRL:
//                   - TRUE: ready
//                   - FALSE: not ready
// -----------------------------------------------------------------------------
bool NPITask_NpiTlTestIsFree(void)
{
    /* (SRDY_PIN == 1): Peripheral ready, (SRDY_PIN == 0):  Peripheral not ready */
    return GPIO_read(SRDY_PIN);
}
// -----------------------------------------------------------------------------
//! \brief      This routine obtains the semaphore which represents the NPI
//!             Serial Bus (UART/SPI) Availability.
//!
//! \return     bool - state of NPI:
//                   - TRUE: succeed to obtain the semaphore, NPI wasn't busy
//                   - FALSE: failed to obtain the semaphore, NPI already busy
// -----------------------------------------------------------------------------
bool NPITL_ObtainTxLock(void)
{
  npiTxActive = TRUE;
  return TRUE;
}

// -----------------------------------------------------------------------------
//! \brief      This routine releases the semaphore which represents the NPI
//!             Serial Bus (UART/SPI) Availability.
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITL_ReleaseTxLock(void)
{
  npiTxActive = FALSE;
}

#else
// -----------------------------------------------------------------------------
//! \brief      This routine obtains the semaphore which represents the NPI
//!             Serial Bus (UART/SPI) Availability.
//!
//! \return     bool - state of NPI:
//                   - TRUE: succeed to obtain the semaphore, NPI wasn't busy
//                   - FALSE: failed to obtain the semaphore, NPI already busy
// -----------------------------------------------------------------------------
bool NPITL_ObtainTxLock(void)
{
  if (sem_trywait(&npiTxActiveLock) == 0 /* semaphore is available */)
  {
      return (TRUE); /* NPI IS NOT BUSY */
  }

  return FALSE; /* NPI IS BUSY */
}

// -----------------------------------------------------------------------------
//! \brief      This routine releases the semaphore which represents the NPI
//!             Serial Bus (UART/SPI) Availability.
//!
//! \return     void
// -----------------------------------------------------------------------------
void NPITL_ReleaseTxLock(void)
{
  sem_post(&npiTxActiveLock);
}
// -----------------------------------------------------------------------------
//! \brief      This routine verifies that NPI Serial Bus (UART/SPI) is available.
//!             It obtains the semaphore, and in case it succeeds - releases it.
//!             This sequence means that the NPI Bus is available.
//!             Failure in obtaining the semaphore means that the NPI Bus is in use.
//!
//! \return     bool - state of NPI availability - 1 - free, 0 - used
// -----------------------------------------------------------------------------
bool NPITask_NpiTlTestIsFree(void)
{
  if (NPITL_ObtainTxLock() == TRUE)
  {
    NPITL_ReleaseTxLock();
    return TRUE;
  }
  return FALSE;
}
#endif //NPI_FLOW_CTRL == 1

#if (NPI_FLOW_CTRL == 1)
// -----------------------------------------------------------------------------
//! \brief      This routine is used to set constraints on power manager
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITL_setPM(void)
{
    if( npiPMSetConstraint )
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
static void NPITL_relPM(void)
{
    if ( ! npiPMSetConstraint )
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
void NPITL_handleMrdyEvent(void)
{
    ICall_CSState key;
    key = ICall_enterCriticalSection();

    // Check to make sure this event is not occurring during the next packet
    // transmission
    if ( GPIO_read(MRDY_PIN) == 0 ||
        ( npiTxActive && (mrdyPktStamp == txPktCount) ) )
    {
#if defined(NPI_USE_SPI)
        transportMrdyEvent();
        SRDY_ENABLE();
#elif defined(NPI_USE_UART)
        SRDY_ENABLE();
        transportMrdyEvent();
#endif
    }

    ICall_leaveCriticalSection(key);
}
#endif // NPI_FLOW_CTRL = 1

#if (NPI_FLOW_CTRL == 1)
// -----------------------------------------------------------------------------
//! \brief      This is a HWI function handler for the MRDY pin. Some MRDY
//!             functionality can execute from this HWI context. Others
//!             must be executed from task context hence the taskMrdyCB()
//!
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITL_MRDYPinHwiFxn(uint_least8_t index)
{
    // The gpio driver does not currently support returning whether the int
    // was neg or pos edge so we must use a variable to keep track of state.
    // If the physical state of the gpio was used then a very quick toggle of
    // of MRDY could be missed.
    mrdy_state ^= 1;

    if (mrdy_state == 0)
    {
        mrdyPktStamp = txPktCount;
        NPITL_setPM();
        if ( taskMrdyCB )
        {
            taskMrdyCB();
        }
    }
    else
    {
        transportStopTransfer();
    }

    // Check the physical state of the pin to see if it matches the variable
    // state. If not then edge has been missed
    if (mrdy_state != GPIO_read(MRDY_PIN))
    {
        mrdy_state = GPIO_read(MRDY_PIN);

        if (mrdy_state == 0)
        {
            mrdyPktStamp = txPktCount;
            NPITL_setPM();

            if (taskMrdyCB)
            {
                taskMrdyCB();
            }
        }
        else
        {
            transportStopTransfer();
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
//! \param[in]  Rxlen   - lenth of the data received
//! \param[in]  Txlen   - length of the data transferred
//!
//! \return     void
// -----------------------------------------------------------------------------
static void NPITL_transmissionCallBack(uint16 Rxlen, uint16 Txlen)
{
    npiRxBufHead = 0;
    npiRxBufTail = Rxlen;

    if(Rxlen)
    {
        if ( taskRxCB )
        {
            /*NPITask_transportRXCallBack*/
            taskRxCB(Rxlen);
        }
    }
    if(Txlen)
    {
        NPITL_ReleaseTxLock();
        // Only perform call back if NPI Task has been registered
        // and if there is not another fragment to send of this message
        if ( taskTxCB && !msgFragLen )
        {
            /*NPITask_transportTxDoneCallBack*/
            taskTxCB(Txlen);
        }
    }

#if (NPI_FLOW_CTRL == 1)
    // Reset mrdy state in case of missed HWI
    mrdy_state = GPIO_read(MRDY_PIN);

    NPITL_relPM();
    SRDY_DISABLE();
#endif // NPI_FLOW_CTRL = 1

    // If there is another fragment to send, begin write without notifying
    // higher level tasks
    if ( msgFragLen )
    {
        NPITL_writeTL(msgFrag,msgFragLen);
    }
}

// -----------------------------------------------------------------------------
//! \brief      This routine reads data from the transport layer based on len,
//!             and places it into the buffer.
//!
//! \param[in]  buf - Pointer to buffer to place read data.
//! \param[in]  len - Number of bytes to read.
//!
//! \return     uint16 - the number of bytes read from transport
// -----------------------------------------------------------------------------
uint16 NPITL_readTL(uint8 *buf, uint16 len)
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
//! \return     uint16 - the number of bytes written to transport
// -----------------------------------------------------------------------------
uint16 NPITL_writeTL(uint8 *buf, uint16 len)
{
#if (NPI_FLOW_CTRL == 1)
    if (!NPITask_NpiTlTestIsFree())
    {
      return 0;
    }
#endif
    if (NPITL_ObtainTxLock() == FALSE) /* NPI is already busy*/
    {
      return 0;
    }
    // If len of message is greater than fragment size
    // then message must be sent over the span of multiple
    // fragments
    if ( len > NPI_MAX_FRAG_SIZE )
    {
      msgFrag = buf + NPI_MAX_FRAG_SIZE;
      msgFragLen = len - NPI_MAX_FRAG_SIZE;
      len = NPI_MAX_FRAG_SIZE;
    }
    else
    {
      msgFrag = NULL;
      msgFragLen = 0;
    }

    memcpy(npiTxBuf, buf, len);
    txPktCount++;

    len = transportWrite(len);

#if (NPI_FLOW_CTRL == 1)
    SRDY_ENABLE();
#endif // NPI_FLOW_CTRL = 1

    return len;
}

// -----------------------------------------------------------------------------
//! \brief      This routine returns the max size receive buffer.
//!
//! \return     uint16 - max size of the receive buffer
// -----------------------------------------------------------------------------
uint16 NPITL_getMaxRxBufSize(void)
{
    return(NPI_TL_BUF_SIZE);
}

// -----------------------------------------------------------------------------
//! \brief      This routine returns the max size transmit buffer.
//!
//! \return     uint16 - max size of the transmit buffer
// -----------------------------------------------------------------------------
uint16 NPITL_getMaxTxBufSize(void)
{
    return(NPI_TL_BUF_SIZE);
}

// -----------------------------------------------------------------------------
//! \brief      Returns number of bytes that are unread in RxBuf
//!
//! \return     uint16 - number of unread bytes
// -----------------------------------------------------------------------------
uint16 NPITL_getRxBufLen(void)
{
    return ((npiRxBufTail - npiRxBufHead) + NPI_TL_BUF_SIZE) % NPI_TL_BUF_SIZE;
}
