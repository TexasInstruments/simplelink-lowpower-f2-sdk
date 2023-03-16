/******************************************************************************
*  Filename:       spi.c
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

#include "spi.h"

//*****************************************************************************
//
// Configures the serial peripheral port
//
//*****************************************************************************
void
SPIConfigSetExpClk(uint32_t ui32Base, uint32_t ui32SPIClk,
                   uint32_t ui32Protocol, uint32_t ui32Mode,
                   uint32_t ui32BitRate, uint32_t ui32DataWidth)
{

    uint32_t ui32RegVal;

    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));
    ASSERT((ui32Protocol == SPI_FRF_MOTO_MODE_0) ||
           (ui32Protocol == SPI_FRF_MOTO_MODE_1) ||
           (ui32Protocol == SPI_FRF_MOTO_MODE_2) ||
           (ui32Protocol == SPI_FRF_MOTO_MODE_3) ||
           (ui32Protocol == SPI_FRF_MOTO_MODE_4) ||
           (ui32Protocol == SPI_FRF_MOTO_MODE_5) ||
           (ui32Protocol == SPI_FRF_MOTO_MODE_6) ||
           (ui32Protocol == SPI_FRF_MOTO_MODE_7) ||
           (ui32Protocol == SPI_FRF_TI) ||
           (ui32Protocol == SPI_FRF_NMW));
    ASSERT((ui32Mode == SPI_MODE_CONTROLLER) ||
           (ui32Mode == SPI_MODE_PERIPHERAL) ||
           (ui32Mode == SPI_MODE_PERIPHERAL_OD));
    ASSERT(ui32BitRate > 0);
    ASSERT(((ui32Mode == SPI_MODE_CONTROLLER) && (ui32BitRate <= (ui32SPIClk / 2))) ||
           ((ui32Mode != SPI_MODE_CONTROLLER) && (ui32BitRate <= (ui32SPIClk / 12))));
    ASSERT((ui32SPIClk / ui32BitRate) <= (254 * 256));
    ASSERT((ui32DataWidth >= 4) && (ui32DataWidth <= 16));

    // Set operating mode
    ui32RegVal = (ui32Mode == SPI_MODE_PERIPHERAL_OD) ? SPI_CTL1_SOD_ENABLE : SPI_CTL1_SOD_DISABLE;
    ui32RegVal |= (ui32Mode == SPI_MODE_CONTROLLER) ? SPI_CTL1_MS_ENABLE : SPI_CTL1_MS_DISABLE;

    // Set data mode MSB first
    HWREG(ui32Base + SPI_O_CTL1) = ui32RegVal | SPI_CTL1_MSB_ENABLE;

    // Set Serial clock divider
    HWREG(ui32Base + SPI_O_CLKCTL) = (ui32SPIClk / (ui32BitRate << 1)) - 1;

    // Set clock ratio
    HWREG(ui32Base + SPI_O_CLKDIV2) = SPI_CLKDIV2_RATIO_DIV_BY_1;

    // Set protocol and data length
    ui32RegVal = ui32Protocol | (ui32DataWidth - 1);
    HWREG(ui32Base + SPI_O_CTL0) = ui32RegVal;
}

//*****************************************************************************
//
// Puts a data element into the SPI transmit FIFO
//
//*****************************************************************************
int32_t
SPIDataPutNonBlocking(uint32_t ui32Base, uint32_t ui32Data)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));
    ASSERT((ui32Data & (0xfffffffe << (HWREG(ui32Base + SPI_O_CTL0) &
                                             SPI_CTL0_DSS_M))) == 0);

    // Check for space to write
    if(HWREG(ui32Base + SPI_O_STAT) & SPI_STAT_TNF_NOT_FULL)
    {
        HWREG(ui32Base + SPI_O_TXDATA) = ui32Data;
        return (1);
    }
    else
    {
        return (0);
    }
}

//*****************************************************************************
//
// Puts a data element into the SPI transmit FIFO
//
//*****************************************************************************
void
SPIDataPut(uint32_t ui32Base, uint32_t ui32Data)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));
    ASSERT((ui32Data & (0xfffffffe << (HWREG(ui32Base + SPI_O_CTL0) &
                                             SPI_CTL0_DSS_M))) == 0);

    // Wait until there is space
    while(!(HWREG(ui32Base + SPI_O_STAT) & SPI_STAT_TNF_NOT_FULL))
    {
    }

    // Write the data to the SPI
    HWREG(ui32Base + SPI_O_TXDATA) = ui32Data;
}

//*****************************************************************************
//
// Gets a data element from the SPI receive FIFO
//
//*****************************************************************************
void
SPIDataGet(uint32_t ui32Base, uint32_t *pui32Data)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Wait until there is data to be read
    while((HWREG(ui32Base + SPI_O_STAT) & SPI_STAT_RFE_EMPTY))
    {
    }

    // Read data from SPI
    *pui32Data = HWREG(ui32Base + SPI_O_RXDATA);
}

//*****************************************************************************
//
// Gets a data element from the SPI receive FIFO
//
//*****************************************************************************
int32_t
SPIDataGetNonBlocking(uint32_t ui32Base, uint32_t *pui32Data)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Check for data to read
    if(!(HWREG(ui32Base + SPI_O_STAT) & SPI_STAT_RFE_EMPTY))
    {
        *pui32Data = HWREG(ui32Base + SPI_O_RXDATA);
        return (1);
    }
    else
    {
        return (0);
    }
}

//*****************************************************************************
//
// Get SPI interrupt port from SPI instance base address
//
//*****************************************************************************
static uint32_t
SPIIntNumberGet(uint32_t ui32Base)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Determine the interrupt number based on the SPI port
    if(ui32Base == SPI3_BASE)
    {
        return INT_SPI3_COMB;
    }
    else if(ui32Base == SPI2_BASE)
    {
        return INT_SPI2_COMB;
    }
    else if(ui32Base == SPI1_BASE)
    {
        return INT_SPI1_COMB;
    }
    else
    {
        return INT_SPI0_COMB;
    }
}

//*****************************************************************************
//
// Registers an interrupt handler for the serial peripheral port
//
//*****************************************************************************
void
SPIIntRegister(uint32_t ui32Base, void (*pfnHandler)(void))
{
    uint32_t ui32Int;

    // Determine the interrupt number based on the SPI port
    ui32Int = SPIIntNumberGet(ui32Base);

    // Register the interrupt handler
    IntRegister(ui32Int, pfnHandler);

    // Enable the synchronous serial port interrupt
    IntEnable(ui32Int);
}

//*****************************************************************************
//
// Unregisters an interrupt handler for the serial peripheral port
//
//*****************************************************************************
void
SPIIntUnregister(uint32_t ui32Base)
{
    uint32_t ui32Int;

    // Determine the interrupt number based on the SPI port
    ui32Int = SPIIntNumberGet(ui32Base);

    // Disable the interrupt
    IntDisable(ui32Int);

    // Unregister the interrupt handler
    IntUnregister(ui32Int);
}
