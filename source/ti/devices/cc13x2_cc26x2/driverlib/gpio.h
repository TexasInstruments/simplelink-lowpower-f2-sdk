/******************************************************************************
*  Filename:       gpio.h
*
*  Description:    Defines and prototypes for the GPIO.
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
//! \addtogroup gpio_api
//! @{
//
//*****************************************************************************

#ifndef __GPIO_H__
#define __GPIO_H__

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

#include <stdint.h>
#include "../inc/hw_types.h"
#include "../inc/hw_memmap.h"
#include "../inc/hw_gpio.h"
#include "debug.h"

//*****************************************************************************
//
// Check for legal range of variable dioNumber
//
//*****************************************************************************
#ifdef DRIVERLIB_DEBUG
#include "../inc/hw_fcfg1.h"
#include "chipinfo.h"

static bool
dioNumberLegal( uint32_t dioNumber )
{
    uint32_t ioCount =
        (( HWREG( FCFG1_BASE + FCFG1_O_IOCONF ) &
            FCFG1_IOCONF_GPIO_CNT_M ) >>
            FCFG1_IOCONF_GPIO_CNT_S ) ;

    // CC13x2 + CC26x2
    if ( ChipInfo_ChipFamilyIs_CC13x2_CC26x2() )
    {
        return ( (dioNumber >= (31 - ioCount)) && (dioNumber < 31) );
    }
    // Special handling of CC13x0 7x7, where IO_CNT = 30 and legal range is 1..30
    // for all other chips legal range is 0..(dioNumber-1)
    else if (( ioCount == 30 ) && ChipInfo_ChipFamilyIs_CC13x0() )
    {
        return (( dioNumber > 0 ) && ( dioNumber <= ioCount ));
    }
    else
    {
        return ( dioNumber < ioCount );
    }
}
#endif

//*****************************************************************************
//
// Define constants that shall be passed as the outputEnableValue parameter to
// GPIO_setOutputEnableDio() and will be returned from the function
// GPIO_getOutputEnableDio().
//
//*****************************************************************************
#define GPIO_OUTPUT_DISABLE     0x00000000  // DIO output is disabled
#define GPIO_OUTPUT_ENABLE      0x00000001  // DIO output is enabled

//*****************************************************************************
//
// API Functions and prototypes
//
//*****************************************************************************

//*****************************************************************************
//
//! \brief Reads a specific DIO.
//!
//! \param dioNumber specifies the DIO to read (0-31).
//!
//! \return Returns 0 or 1 reflecting the input value of the specified DIO.
//!
//! \sa \ref GPIO_writeDio()
//
//*****************************************************************************
__STATIC_INLINE uint32_t
GPIO_readDio( uint32_t dioNumber )
{
    // Check the arguments.
    ASSERT( dioNumberLegal( dioNumber ));

    // Return the input value from the specified DIO.
    return (( HWREG( GPIO_BASE + GPIO_O_DIN31_0 ) >> dioNumber ) & 1 );
}

//*****************************************************************************
//
//! \brief Writes a value to a specific DIO.
//!
//! \param dioNumber specifies the DIO to update (0-31).
//! \param value specifies the value to write
//! - 0 : Logic zero (low)
//! - 1 : Logic one (high)
//!
//! \return None
//!
//! \sa \ref GPIO_readDio()
//
//*****************************************************************************
__STATIC_INLINE void
GPIO_writeDio( uint32_t dioNumber, uint32_t value )
{
    // Check the arguments.
    ASSERT( dioNumberLegal( dioNumber ));
    ASSERT(( value == 0 ) || ( value == 1 ));

    // Write 0 or 1 to the byte indexed DOUT map
    HWREGB( GPIO_BASE + dioNumber ) = value;
}

//*****************************************************************************
//
//! \brief Sets a specific DIO to 1 (high).
//!
//! \param dioNumber specifies the DIO to set (0-31).
//!
//! \return None
//!
//! \sa \ref GPIO_clearDio()
//
//*****************************************************************************
__STATIC_INLINE void
GPIO_setDio( uint32_t dioNumber )
{
    GPIO_writeDio(dioNumber, 1);
}

//*****************************************************************************
//
//! \brief Clears a specific DIO to 0 (low).
//!
//! \param dioNumber specifies the DIO to clear (0-31).
//!
//! \return None
//!
//! \sa \ref GPIO_setDio()
//
//*****************************************************************************
__STATIC_INLINE void
GPIO_clearDio( uint32_t dioNumber )
{
    GPIO_writeDio(dioNumber, 0);
}

//*****************************************************************************
//
//! \brief Toggles a specific DIO.
//!
//! \param dioNumber specifies the DIO to toggle (0-31).
//!
//! \return None
//
//*****************************************************************************
__STATIC_INLINE void
GPIO_toggleDio( uint32_t dioNumber )
{
    // Check the arguments.
    ASSERT( dioNumberLegal( dioNumber ));

    // Toggle the specified DIO.
    HWREG( GPIO_BASE + GPIO_O_DOUTTGL31_0 ) = ( 1 << dioNumber );
}

//*****************************************************************************
//
//! \brief Gets the output enable status of a specific DIO.
//!
//! This function returns the output enable status for the specified DIO.
//! The DIO can be configured as either input or output under software control.
//!
//! \param dioNumber specifies the DIO to get the output enable setting from (0-31).
//!
//! \return Returns one of the enumerated data types (0 or 1):
//! - \ref GPIO_OUTPUT_DISABLE : DIO output is disabled.
//! - \ref GPIO_OUTPUT_ENABLE  : DIO output is enabled.
//!
//! \sa \ref GPIO_setOutputEnableDio()
//
//*****************************************************************************
__STATIC_INLINE uint32_t
GPIO_getOutputEnableDio( uint32_t dioNumber )
{
    // Check the arguments.
    ASSERT( dioNumberLegal( dioNumber ));

    // Return the output enable status for the specified DIO.
    return (( HWREG( GPIO_BASE + GPIO_O_DOE31_0 ) >> dioNumber ) & 1 );
}

//*****************************************************************************
//
//! \brief Sets output enable of a specific DIO.
//!
//! This function sets the GPIO output enable bit for the specified DIO.
//! The DIO can be configured as either input or output under software control.
//!
//! \param dioNumber specifies the DIO to configure (0-31).
//! \param outputEnableValue specifies the output enable setting of the specified DIO:
//! - \ref GPIO_OUTPUT_DISABLE : DIO output is disabled.
//! - \ref GPIO_OUTPUT_ENABLE  : DIO output is enabled.
//!
//! \return None
//!
//! \sa \ref GPIO_getOutputEnableDio()
//
//*****************************************************************************
__STATIC_INLINE void
GPIO_setOutputEnableDio( uint32_t dioNumber, uint32_t outputEnableValue )
{
    // Check the arguments.
    ASSERT( dioNumberLegal( dioNumber ));
    ASSERT(( outputEnableValue == GPIO_OUTPUT_DISABLE ) ||
           ( outputEnableValue == GPIO_OUTPUT_ENABLE  )    );

    // Update the output enable bit for the specified DIO.
    HWREGBITW( GPIO_BASE + GPIO_O_DOE31_0, dioNumber ) = outputEnableValue;
}

//*****************************************************************************
//
//! \brief Gets the event status of a specific DIO.
//!
//! \param dioNumber specifies the DIO to get the event status from (0-31).
//!
//! \return Returns the current event status on the specified DIO.
//! - 0 : Non-triggered event.
//! - 1 : Triggered event.
//!
//! \sa \ref GPIO_clearEventDio()
//
//*****************************************************************************
__STATIC_INLINE uint32_t
GPIO_getEventDio( uint32_t dioNumber )
{
    // Check the arguments.
    ASSERT( dioNumberLegal( dioNumber ));

    // Return the event status for the specified DIO.
    return (( HWREG( GPIO_BASE + GPIO_O_EVFLAGS31_0 ) >> dioNumber ) & 1 );
}

//*****************************************************************************
//
//! \brief Clears the IO event status of a specific DIO.
//!
//! \param dioNumber specifies the DIO on which to clear the event status (0-31).
//!
//! \return None
//!
//! \sa \ref GPIO_getEventDio()
//
//*****************************************************************************
__STATIC_INLINE void
GPIO_clearEventDio( uint32_t dioNumber )
{
    // Check the arguments.
    ASSERT( dioNumberLegal( dioNumber ));

    // Clear the event status for the specified DIO.
    HWREG( GPIO_BASE + GPIO_O_EVFLAGS31_0 ) = ( 1 << dioNumber );
}

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __GPIO_H__

//*****************************************************************************
//
//! Close the Doxygen group.
//! @}
//! @}
//
//*****************************************************************************
