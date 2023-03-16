/******************************************************************************
*  Filename:       spi.h
*
*  Description:    Driver for Serial Peripheral Interface
*
*  Copyright (c) 2015 - 2022, Texas Instruments Incorporated
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*
*  1) Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2) Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*
*  3) Neither the name of the ORGANIZATION nor the names of its contributors may
*     be used to endorse or promote products derived from this software without
*     specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
*  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
*  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
*  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
*  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
*  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
*  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

//*****************************************************************************
//
//! \addtogroup peripheral_group
//! @{
//! \addtogroup spi_api
//! @{
//
//*****************************************************************************

#ifndef __SPI_H__
#define __SPI_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include "../inc/hw_ints.h"
#include "../inc/hw_memmap.h"
#include "../inc/hw_types.h"
#include "../inc/hw_spi.h"
#include "debug.h"
#include "interrupt.h"
#ifdef DRIVERLIB_DEBUG
#include "chipinfo.h"
#endif

//*****************************************************************************
//
// Values that can be passed to SPIIntEnable(), SPIIntDisable(), and SPIIntClear()
// as the ui32IntFlags parameter, and returned by SPIIntStatus().
//
//*****************************************************************************
#define SPI_DMA_DONE_TX         SPI_IMASK_DMA_DONE_TX //!< DMA Done 1 event for TX event mask
#define SPI_DMA_DONE_RX         SPI_IMASK_DMA_DONE_RX //!< DMA Done 1 event for RX event mask
#define SPI_IDLE                SPI_IMASK_IDLE        //!< SPI Idle event mask
#define SPI_TXEMPTY             SPI_IMASK_TXEMPTY     //!< Transmit FIFO Empty event mask
#define SPI_TX                  SPI_IMASK_TX          //!< Transmit FIFO event mask
#define SPI_RX                  SPI_IMASK_RX          //!< Receive FIFO event. This interrupt is set if the selected Receive FIFO level has been reached
#define SPI_RTOUT               SPI_IMASK_RTOUT       //!< Enable SPI Receive Time-Out event mask
#define SPI_PER                 SPI_IMASK_PER         //!< Parity error event mask
#define SPI_RXFIFO_OVF          SPI_IMASK_RXFIFO_OVF  //!< RXFIFO overflow event mask


//*****************************************************************************
//
// Values that are returned from SPIStatus
//
//*****************************************************************************
#define SPI_BUSY                SPI_STAT_BUSY_ACTIVE  //!< Busy
#define SPI_RX_NOT_FULL         SPI_STAT_RNF_NOT_FULL //!< Receive FIFO not full
#define SPI_RX_EMPTY            SPI_STAT_RFE_EMPTY    //!< Receive FIFO empty
#define SPI_TX_NOT_FULL         SPI_STAT_TNF_NOT_FULL //!< Transmit FIFO not full
#define SPI_TX_EMPTY            SPI_STAT_TFE_EMPTY    //!< Transmit FIFO empty
#define SPI_STATUS_MASK         0x0000001F            //!< Mask for bits above

//*****************************************************************************
//
// Values that can be passed to SPIConfigSetExpClk
//
//*****************************************************************************
//! Motorola format 3-wire, polarity 0, phase 0
#define SPI_FRF_MOTO_MODE_0     ( SPI_CTL0_FRF_MOTOROLA_3WIRE |  \
                                  SPI_CTL0_SPO_LOW | SPI_CTL0_SPH_FIRST )
//! Motorola format 3-wire, polarity 0, phase 1
#define SPI_FRF_MOTO_MODE_1     ( SPI_CTL0_FRF_MOTOROLA_3WIRE |  \
                                  SPI_CTL0_SPO_LOW | SPI_CTL0_SPH_SECOND )
//! Motorola format 3-wire, polarity 1, phase 0
#define SPI_FRF_MOTO_MODE_2     ( SPI_CTL0_FRF_MOTOROLA_3WIRE |  \
                                  SPI_CTL0_SPO_HIGH | SPI_CTL0_SPH_FIRST )
//! Motorola format 3-wire, polarity 1, phase 1
#define SPI_FRF_MOTO_MODE_3     ( SPI_CTL0_FRF_MOTOROLA_3WIRE |  \
                                  SPI_CTL0_SPO_HIGH | SPI_CTL0_SPH_SECOND )
//! Motorola format 4-wire, polarity 0, phase 0
#define SPI_FRF_MOTO_MODE_4     ( SPI_CTL0_FRF_MOTOROLA_4WIRE |  \
                                  SPI_CTL0_SPO_LOW | SPI_CTL0_SPH_FIRST )
//! Motorola format 4-wire, polarity 0, phase 1
#define SPI_FRF_MOTO_MODE_5     ( SPI_CTL0_FRF_MOTOROLA_4WIRE |  \
                                  SPI_CTL0_SPO_LOW | SPI_CTL0_SPH_SECOND )
//! Motorola format 4-wire, polarity 1, phase 0
#define SPI_FRF_MOTO_MODE_6     ( SPI_CTL0_FRF_MOTOROLA_4WIRE |  \
                                  SPI_CTL0_SPO_HIGH | SPI_CTL0_SPH_FIRST )
//! Motorola format 4-wire, polarity 1, phase 1
#define SPI_FRF_MOTO_MODE_7     ( SPI_CTL0_FRF_MOTOROLA_4WIRE |  \
                                  SPI_CTL0_SPO_HIGH | SPI_CTL0_SPH_SECOND )

#define SPI_FRF_TI              SPI_CTL0_FRF_TI_SYNC   //!< TI Sync frame format
#define SPI_FRF_NMW             SPI_CTL0_FRF_MIRCOWIRE //!< MicroWire frame format

#define SPI_MODE_CONTROLLER     SPI_CTL1_MS_ENABLE   //!< SPI controller
#define SPI_MODE_PERIPHERAL     SPI_CTL1_MS_DISABLE  //!< SPI peripheral
#define SPI_MODE_PERIPHERAL_OD  SPI_CTL1_SOD_ENABLE  //!< SPI peripheral with POCI
                                                     //!< output disabled

//*****************************************************************************
//
// Values that can be passed to SPIDMAEnable() and SPIDMADisable()
//
//*****************************************************************************
#define SPI_DMA_TX              SPI_DMACR_TXDMAE  //!< Enable DMA for transmit
#define SPI_DMA_RX              SPI_DMACR_RXDMAE  //!< Enable DMA for receive

//*****************************************************************************
//
// API Functions and prototypes
//
//*****************************************************************************

#ifdef DRIVERLIB_DEBUG
//*****************************************************************************
//
//! \internal
//!
//! \brief Checks an SPI base address.
//!
//! This function determines if an SPI module base address is valid.
//!
//! \param ui32Base specifies the SPI module base address.
//!
//! \return Returns \c true if the base address is valid and \c false
//! otherwise.
//
//*****************************************************************************
static bool
SPIBaseValid(uint32_t ui32Base)
{
    if (ChipInfo_GetChipType() == CHIP_TYPE_CC2653P10)
    {
        return((ui32Base == SPI0_BASE) || (ui32Base == SPI1_BASE));
    }
    else{
        return((ui32Base == SPI0_BASE) || \
               (ui32Base == SPI1_BASE) || \
               (ui32Base == SPI2_BASE) || \
               (ui32Base == SPI3_BASE));
    }
}
#endif

//*****************************************************************************
//
//! \brief Configures the serial peripheral port.
//!
//! This function configures the serial peripheral port.  It sets
//! the SPI protocol, mode of operation, bit rate, and data width.
//!
//! The \c ui32Protocol parameter defines the data frame format. The Motorola
//! frame formats imply the following polarity and phase configurations:
//!
//! <pre>
//! Polarity Phase   Motorola Protocol                  Mode
//!   0       0           3-wire                SPI_FRF_MOTO_MODE_0
//!   0       1           3-wire                SPI_FRF_MOTO_MODE_1
//!   1       0           3-wire                SPI_FRF_MOTO_MODE_2
//!   1       1           3-wire                SPI_FRF_MOTO_MODE_3
//!   0       0           4-wire                SPI_FRF_MOTO_MODE_4
//!   0       1           4-wire                SPI_FRF_MOTO_MODE_5
//!   1       0           4-wire                SPI_FRF_MOTO_MODE_6
//!   1       1           4-wire                SPI_FRF_MOTO_MODE_7
//! </pre>
//!
//! The \c ui32Mode parameter defines the operating mode of the SPI module.
//! The SPI module can operate as a controller or peripheral; if a peripheral, the SPI can be
//! configured to disable output on its serial output line.
//!
//! The \c ui32BitRate parameter defines the bit rate for the SPI.
//!
//! The \c ui32DataWidth parameter defines the width of the data transfers, and
//! can be a value between 4 and 32, for controller mode, and 7 to 32, in peripheral mode.
//!
//! \note The peripheral clock is not necessarily the same as the processor clock.
//! The frequency of the peripheral clock is set by the system control.
//!
//! \param ui32Base specifies the SPI module base address.
//! \param ui32SPIClk is the rate of the clock supplied to the SPI module.
//! \param ui32Protocol specifies the data transfer protocol.
//! The parameter can be one of the following values:
//! - \ref SPI_FRF_MOTO_MODE_0
//! - \ref SPI_FRF_MOTO_MODE_1
//! - \ref SPI_FRF_MOTO_MODE_2
//! - \ref SPI_FRF_MOTO_MODE_3
//! - \ref SPI_FRF_MOTO_MODE_4
//! - \ref SPI_FRF_MOTO_MODE_5
//! - \ref SPI_FRF_MOTO_MODE_6
//! - \ref SPI_FRF_MOTO_MODE_7
//! - \ref SPI_FRF_TI
//! - \ref SPI_FRF_NMW.
//! \param ui32Mode specifies the mode of operation.
//! The parameter can be one of the following values:
//! - \ref SPI_MODE_CONTROLLER
//! - \ref SPI_MODE_PERIPHERAL
//! - \ref SPI_MODE_PERIPHERAL_OD
//! \param ui32BitRate specifies the clock rate.
//! \param ui32DataWidth specifies number of bits transferred per frame.
//! Must be a value between 4 and 32, for controller mode, and 7 to 32, in peripheral mode.
//!
//! \return None
//
//*****************************************************************************
extern void SPIConfigSetExpClk(uint32_t ui32Base, uint32_t ui32SPIClk,
                               uint32_t ui32Protocol, uint32_t ui32Mode,
                               uint32_t ui32BitRate, uint32_t ui32DataWidth);

//*****************************************************************************
//
//! \brief Enables the serial peripheral port.
//!
//! This function enables operation of the serial peripheral port.  The
//! serial peripheral port must be configured before it is enabled.
//!
//! \param ui32Base specifies the SPI module base address.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SPIEnable(uint32_t ui32Base)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Read-modify-write the enable bit
    HWREG(ui32Base + SPI_O_CTL1) |= SPI_CTL1_ENABLE;
}

//*****************************************************************************
//
//! \brief Disables the serial peripheral port.
//!
//! This function disables operation of the serial peripheral port.
//!
//! \param ui32Base specifies the SPI module base address.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SPIDisable(uint32_t ui32Base)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Read-modify-write the enable bit
    HWREG(ui32Base + SPI_O_CTL1) &= ~(SPI_CTL1_ENABLE);
}

//*****************************************************************************
//
//! \brief Puts a data element into the SPI transmit FIFO.
//!
//! This function places the supplied data into the transmit FIFO of the
//! specified SPI module.
//!
//! \note The upper 32 - N bits of the \c ui32Data are discarded by the
//! hardware, where N is the data width as configured by \ref SPIConfigSetExpClk().
//! For example, if the interface is configured for 8-bit data width, the upper
//! 24 bits of \c ui32Data are discarded.
//!
//! \param ui32Base specifies the SPI module base address.
//! \param ui32Data is the data to be transmitted over the SPI interface.
//!
//! \return None
//
//*****************************************************************************
extern void SPIDataPut(uint32_t ui32Base, uint32_t ui32Data);

//*****************************************************************************
//
//! \brief Puts a data element into the SPI transmit FIFO.
//!
//! This function places the supplied data into the transmit FIFO of the
//! specified SPI module. If there is no space in the FIFO, then this function
//! returns a zero.
//!
//! \note The upper 32 - N bits of the \c ui32Data are discarded by the hardware,
//! where N is the data width as configured by \ref SPIConfigSetExpClk(). For
//! example, if the interface is configured for 8-bit data width, the upper 24
//! bits of \c ui32Data are discarded.
//!
//! \param ui32Base specifies the SPI module base address.
//! \param ui32Data is the data to be transmitted over the SPI interface.
//!
//! \return Returns the number of elements written to the SPI transmit FIFO.
//
//*****************************************************************************
extern int32_t SPIDataPutNonBlocking(uint32_t ui32Base, uint32_t ui32Data);

//*****************************************************************************
//
//! \brief Gets a data element from the SPI receive FIFO.
//!
//! This function gets received data from the receive FIFO of the specified
//! SPI module and places that data into the location specified by the
//! \c pui32Data parameter.
//!
//! \note Only the lower N bits of the value written to \c pui32Data contain
//! valid data, where N is the data width as configured by
//! \ref SPIConfigSetExpClk(). For example, if the interface is configured for
//! 8-bit data width, only the lower 8 bits of the value written to
//! \c pui32Data contain valid data.
//!
//! \param ui32Base specifies the SPI module base address.
//! \param pui32Data is a pointer to a storage location for data that was
//! received over the SPI interface.
//!
//! \return None
//
//*****************************************************************************
extern void SPIDataGet(uint32_t ui32Base, uint32_t *pui32Data);

//*****************************************************************************
//
//! \brief Gets a data element from the SPI receive FIFO.
//!
//! This function gets received data from the receive FIFO of the specified SPI
//! module and places that data into the location specified by the \c ui32Data
//! parameter. If there is no data in the FIFO, then this function returns a
//! zero.
//!
//! \note Only the lower N bits of the value written to \c pui32Data contain
//! valid data, where N is the data width as configured by
//! \ref SPIConfigSetExpClk(). For example, if the interface is configured for
//! 8-bit data width, only the lower 8 bits of the value written to \c pui32Data
//! contain valid data.
//!
//! \param ui32Base specifies the SPI module base address.
//! \param pui32Data is a pointer to a storage location for data that was
//! received over the SPI interface.
//!
//! \return Returns the number of elements read from the SPI receive FIFO.
//
//*****************************************************************************
extern int32_t SPIDataGetNonBlocking(uint32_t ui32Base, uint32_t *pui32Data);

//*****************************************************************************
//
//! \brief Determines whether the SPI transmitter is busy or not.
//!
//! Allows the caller to determine whether all transmitted bytes have cleared
//! the transmitter hardware. If \c false is returned, then the transmit FIFO
//! is empty and all bits of the last transmitted word have left the hardware
//! shift register.
//!
//! \param ui32Base is the base address of the SPI port.
//!
//! \return Returns status of the SPI transmit buffer.
//! - \c true  : SPI is transmitting.
//! - \c false : SPI transmissions are complete.
//
//*****************************************************************************
__STATIC_INLINE bool
SPIBusy(uint32_t ui32Base)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Determine if the SPI is busy
    return ((HWREG(ui32Base + SPI_O_STAT) & SPI_STAT_BUSY) ? true : false);
}

//*****************************************************************************
//
//! \brief Get the status of the SPI data buffers.
//!
//! This function is used to poll the status of the internal FIFOs in the SPI
//! module. The status of both TX and RX FIFO is returned.
//!
//! \param ui32Base specifies the SPI module base address.
//!
//! \return Returns the current status of the internal SPI data buffers.
//! The status is a bitwise OR'ed combination of:
//! - \ref SPI_BUSY        : Busy
//! - \ref SPI_RX_NOT_FULL : Receive FIFO not full
//! - \ref SPI_RX_EMPTY    : Receive FIFO empty
//! - \ref SPI_TX_NOT_FULL : Transmit FIFO not full
//! - \ref SPI_TX_EMPTY    : Transmit FIFO empty
//
//*****************************************************************************
__STATIC_INLINE uint32_t
SPIStatus(uint32_t ui32Base)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Return the status
    return (HWREG(ui32Base + SPI_O_STAT) & SPI_STATUS_MASK);
}

//*****************************************************************************
//
//! \brief Registers an interrupt handler for the
//! Serial Peripheral Interface in the dynamic interrupt table.
//!
//! \note Only use this function if you want to use the dynamic vector table (in SRAM)!
//!
//! This function registers a function as the interrupt handler for a specific
//! interrupt and enables the corresponding interrupt in the interrupt controller.
//!
//! Specific SPI interrupts must be enabled via \ref SPIIntEnable(). If necessary,
//! it is the interrupt handler's responsibility to clear the interrupt source
//! via \ref SPIIntClear().
//!
//! \param ui32Base specifies the SPI module base address.
//! \param pfnHandler is a pointer to the function to be called when the
//! serial peripheral port interrupt occurs.
//!
//! \return None
//!
//! \sa \ref IntRegister() for important information about registering interrupt
//! handlers.
//
//*****************************************************************************
extern void SPIIntRegister(uint32_t ui32Base, void (*pfnHandler)(void));

//*****************************************************************************
//
//! \brief Unregisters an interrupt handler for the
//! Serial Peripheral Interface in the dynamic interrupt table.
//!
//! This function will clear the handler to be called when a SPI
//! interrupt occurs. This will also mask off the interrupt in the interrupt
//! controller so that the interrupt handler no longer is called.
//!
//! \param ui32Base specifies the SPI module base address.
//!
//! \return None
//!
//! \sa \ref IntRegister() for important information about registering interrupt
//! handlers.
//
//*****************************************************************************
extern void SPIIntUnregister(uint32_t ui32Base);

//*****************************************************************************
//
//! \brief Enables individual SPI interrupt sources.
//!
//! Enables the indicated SPI interrupt sources. Only the sources that are
//! enabled can be reflected to the processor interrupt; disabled sources have
//! no effect on the processor.
//!
//! \param ui32Base specifies the SPI module base address.
//! \param ui32IntFlags is a bit mask of the interrupt sources to be enabled.
//! - \ref  SPI_DMA_DONE_TX
//! - \ref  SPI_DMA_DONE_RX
//! - \ref  SPI_IDLE
//! - \ref  SPI_TXEMPTY
//! - \ref  SPI_TX
//! - \ref  SPI_RX
//! - \ref  SPI_RTOUT
//! - \ref  SPI_PER
//! - \ref  SPI_RXFIFO_OVF
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SPIIntEnable(uint32_t ui32Base, uint32_t ui32IntFlags)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Enable the specified interrupts
    HWREG(ui32Base + SPI_O_IMASK) |= ui32IntFlags;
}

//*****************************************************************************
//
//! \brief Disables individual SPI interrupt sources.
//!
//! Disables the indicated SPI interrupt sources.
//!
//! \param ui32Base specifies the SPI module base address.
//! \param ui32IntFlags is a bit mask of the interrupt sources to be disabled.
//! - \ref  SPI_DMA_DONE_TX
//! - \ref  SPI_DMA_DONE_RX
//! - \ref  SPI_IDLE
//! - \ref  SPI_TXEMPTY
//! - \ref  SPI_TX
//! - \ref  SPI_RX
//! - \ref  SPI_RTOUT
//! - \ref  SPI_PER
//! - \ref  SPI_RXFIFO_OVF
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SPIIntDisable(uint32_t ui32Base, uint32_t ui32IntFlags)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Disable the specified interrupts
    HWREG(ui32Base + SPI_O_IMASK) &= ~(ui32IntFlags);
}

//*****************************************************************************
//
//! \brief Clears SPI interrupt sources.
//!
//! The specified SPI interrupt sources are cleared so that they no longer
//! assert. This function must be called in the interrupt handler to keep the
//! interrupts from being recognized again immediately upon exit.
//!
//! \note Due to write buffers and synchronizers in the system it may take several
//! clock cycles from a register write clearing an event in a module and until the
//! event is actually cleared in the NVIC of the system CPU. It is recommended to
//! clear the event source early in the interrupt service routine (ISR) to allow
//! the event clear to propagate to the NVIC before returning from the ISR.
//! At the same time, an early event clear allows new events of the same type to be
//! pended instead of ignored if the event is cleared later in the ISR.
//! It is the responsibility of the programmer to make sure that enough time has passed
//! before returning from the ISR to avoid false re-triggering of the cleared event.
//! A simple, although not necessarily optimal, way of clearing an event before
//! returning from the ISR is:
//! -# Write to clear event (interrupt source). (buffered write)
//! -# Dummy read from the event source module. (making sure the write has propagated)
//! -# Wait two system CPU clock cycles (user code or two NOPs),
//! (allowing cleared event to propagate through any synchronizers)
//!
//! \param ui32Base specifies the SPI module base address.
//! \param ui32IntFlags is a bit mask of the interrupt sources to be cleared.
//! The parameter can consist of either or both of:
//! - \ref  SPI_DMA_DONE_TX
//! - \ref  SPI_DMA_DONE_RX
//! - \ref  SPI_IDLE
//! - \ref  SPI_TXEMPTY
//! - \ref  SPI_TX
//! - \ref  SPI_RX
//! - \ref  SPI_RTOUT
//! - \ref  SPI_PER
//! - \ref  SPI_RXFIFO_OVF
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SPIIntClear(uint32_t ui32Base, uint32_t ui32IntFlags)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Clear the requested interrupt sources
    HWREG(ui32Base + SPI_O_ICLR) = ui32IntFlags;
}

//*****************************************************************************
//
//! \brief Gets the current interrupt status.
//!
//! This function returns the interrupt status for the SPI module.  Either the
//! raw interrupt status or the status of interrupts that are allowed to
//! reflect to the processor can be returned.
//!
//! \param ui32Base specifies the SPI module base address.
//! \param bMasked selects either raw or masked interrupt.
//! \c false : Raw interrupt status is required.
//! \c true  : Masked interrupt status is required.
//!
//! \return Returns the current interrupt status as an OR'ed combination of:
//! - \ref  SPI_DMA_DONE_TX
//! - \ref  SPI_DMA_DONE_RX
//! - \ref  SPI_IDLE
//! - \ref  SPI_TXEMPTY
//! - \ref  SPI_TX
//! - \ref  SPI_RX
//! - \ref  SPI_RTOUT
//! - \ref  SPI_PER
//! - \ref  SPI_RXFIFO_OVF
//
//*****************************************************************************
__STATIC_INLINE uint32_t
SPIIntStatus(uint32_t ui32Base, bool bMasked)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Return either the interrupt status or the raw interrupt status as
    // requested
    if(bMasked)
    {
        return (HWREG(ui32Base + SPI_O_MIS));
    }
    else
    {
        return (HWREG(ui32Base + SPI_O_RIS));
    }
}

//*****************************************************************************
//
//! \brief Enable SPI DMA operation.
//!
//! The specified SPI DMA features are enabled. The SPI can be
//! configured to use DMA for transmit and/or receive data transfers.
//!
//! \note The uDMA controller must also be set up before DMA can be used
//! with the SPI.
//!
//! \param ui32Base is the base address of the SPI port.
//! \param ui32DMAFlags is a bit mask of the DMA features to enable.
//! The parameter is the bitwise OR of any of the following values:
//! - \ref SPI_DMA_RX : Enable DMA for receive.
//! - \ref SPI_DMA_TX : Enable DMA for transmit.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SPIDMAEnable(uint32_t ui32Base, uint32_t ui32DMAFlags)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Set the requested bits in the SPI DMA control register
    HWREG(ui32Base + SPI_O_DMACR) |= ui32DMAFlags;
}

//*****************************************************************************
//
//! \brief Disable SPI DMA operation.
//!
//! This function is used to disable SPI DMA features that were enabled
//! by \ref SPIDMAEnable(). The specified SPI DMA features are disabled.
//!
//! \param ui32Base is the base address of the SPI port.
//! \param ui32DMAFlags is a bit mask of the DMA features to disable.
//! The parameter is the bitwise OR of any of the following values:
//! - \ref SPI_DMA_RX : Disable DMA for receive.
//! - \ref SPI_DMA_TX : Disable DMA for transmit.
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
SPIDMADisable(uint32_t ui32Base, uint32_t ui32DMAFlags)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Clear the requested bits in the SPI DMA control register
    HWREG(ui32Base + SPI_O_DMACR) &= ~(ui32DMAFlags);
}

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __SPI_H__

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//! @}
//
//*****************************************************************************
