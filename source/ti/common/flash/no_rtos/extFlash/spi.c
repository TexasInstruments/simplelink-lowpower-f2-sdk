/******************************************************************************
*  Filename:       spi.c
*
*  Description:    Driver for Serial Peripheral Interface
*
// ##### LICENSE HEADER #####
*
******************************************************************************/

#include "spi.h"

/* DSAMPLE default value limits based bit rate */
#define DSAMPLE_MED_BITRATE  4000000
#define DSAMPLE_HIGH_BITRATE 8000000

//*****************************************************************************
//
// Configures the serial peripheral port
//
//*****************************************************************************
static void configSPI(uint32_t baseAddr,
                      uint32_t freq,
                      uint32_t format,
                      uint32_t mode,
                      uint32_t bitRate,
                      uint32_t dataSize,
                      uint32_t dsample)
{
    uint16_t scr;
    uint32_t ratio;

    /* Get existing settings */
    uint32_t reg  = HWREG(baseAddr + SPI_O_CTL0);
    /* Create mask for settings to modify */
    uint32_t mask = (SPI_CTL0_DSS_M | SPI_CTL0_FRF_M | SPI_CTL0_SPO_M | SPI_CTL0_SPH_M);

    /* Convert and mask data size to HW register format */
    dataSize                     = (SPI_CTL0_DSS_M & (dataSize - 1));
    /* Apply updated register */
    HWREG(baseAddr + SPI_O_CTL0) = (reg & ~mask) | format | dataSize;
            //| SPI_CTL0_CSCLR_ENABLE; // Mine - do it only when mode is slave

    /* Set master/slave mode, MSB first */
    HWREG(baseAddr + SPI_O_CTL1) = mode | SPI_CTL1_MSB_MSB;

    /* Get existing settings */
    reg = HWREG(baseAddr + SPI_O_CLKCFG1);

    /* Create a mask for settings to modify */
    mask = (SPI_CLKCFG1_DSAMPLE_M | SPI_CLKCFG1_SCR_M);

    /* Calculate scr variable */
    ratio = freq / (2 * bitRate);
    if (ratio > 0 && ratio <= SPI_CLKCFG1_SCR_MAXIMUM)
    {
        scr = (uint16_t)(ratio - 1);
    }
    else
    {
        scr = 0;
    }

    /* Set clock divider */
    HWREG(baseAddr + SPI_O_CLKCFG1) = (reg & ~mask) | dsample | scr;
}

void
SPIConfigSetExpClk(uint32_t ui32Base, uint32_t ui32SPIClk,
                   uint32_t ui32Protocol, uint32_t ui32Mode,
                   uint32_t ui32BitRate, uint32_t ui32DataWidth)
{
    uint32_t ui32dsample = 0;

    ASSERT(SPIBaseValid(ui32Base));

    if (ui32BitRate >= DSAMPLE_MED_BITRATE)
    {
        ui32dsample = 1 << SPI_CLKCFG1_DSAMPLE_S;
    }
    else if (ui32BitRate >= DSAMPLE_HIGH_BITRATE)
    {
        ui32dsample = 2 << SPI_CLKCFG1_DSAMPLE_S;
    }

    configSPI(ui32Base, ui32SPIClk,
              ui32Protocol,
              ui32Mode, ui32BitRate,
              ui32DataWidth, ui32dsample);
}

//*****************************************************************************
//
// Puts a data element into the SPI transmit FIFO
//
//*****************************************************************************
int32_t
SPIDataPutNonBlocking(uint32_t ui32Base, uint32_t ui32Data)
{
    ASSERT(SPIBaseValid(ui32Base));

    /* Check for space to write. */
    if (HWREG(ui32Base + SPI_O_STAT) & SPI_STAT_TNF_NOT_FULL)
    {
        /* Write the data to the SPI TX FIFO. */
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
    ASSERT(SPIBaseValid(ui32Base));

    /* Check for space to write. */
    while(!(HWREG(ui32Base + SPI_O_STAT) & SPI_STAT_TNF_NOT_FULL)) {}

    /* Write the data to the SPI TX FIFO. */
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
    ASSERT(SPIBaseValid(ui32Base));

    /* Wait until there is data to be read. */
    while (HWREG(ui32Base + SPI_O_STAT) & SPI_STAT_RFE_EMPTY) {}

    /* Read data from SPI RX FIFO. */
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
// Registers an interrupt handler for the serial peripheral port
//
//*****************************************************************************
void
SPIIntRegister(uint32_t ui32Base, void (*pfnHandler)(void))
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Register the interrupt handler
    IntRegister(INT_SPI0_COMB, pfnHandler);

    // Enable the synchronous serial port interrupt
    IntEnable(INT_SPI0_COMB);
}

//*****************************************************************************
//
// Unregisters an interrupt handler for the serial peripheral port
//
//*****************************************************************************
void
SPIIntUnregister(uint32_t ui32Base)
{
    // Check the arguments
    ASSERT(SPIBaseValid(ui32Base));

    // Disable the interrupt
    IntDisable(INT_SPI0_COMB);

    // Unregister the interrupt handler
    IntUnregister(INT_SPI0_COMB);
}
