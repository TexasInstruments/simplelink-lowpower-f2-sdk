/******************************************************************************

 @file  mac_autopend.c

 @brief This file contains the data structures and APIs for CC13xx
        RF Core Firmware Specification for IEEE 802.15.4.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2009-2023, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

#ifndef LIBRARY
#include "ti_wisunfan_features.h"
#endif

#if defined(FREQ_2_4G) || defined(COMBO_MAC)
/* low-level */
#include "timac_api.h"
#include "mac_radio_defs.h"

#include "saddr.h"

#include "mac_autopend.h"

#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#include "dbgid_sys_mst.h"
#endif /* DEBUG_SW_TRACE */

#include "rom_jt_154.h"
/* ------------------------------------------------------------------------------------------------
 *                                           Defines
 * ------------------------------------------------------------------------------------------------
 */
          
/* ------------------------------------------------------------------------------------------------
 *                                      Global Variables
 * ------------------------------------------------------------------------------------------------
 */
extern extAddrList_t    macSrcExtAddrList;
extern shortAddrList_t  macSrcShortAddrList;

/* ------------------------------------------------------------------------------------------------
 *                                         Local Variables
 * ------------------------------------------------------------------------------------------------
 */

/* 
 The following local Varables are only set in MAC_SrcMatchEnable()  
 They are read only to the rest of the module.
 */
bool  macSrcMatchIsAckAllPending = FALSE;
uint8 macSrcResIndex;

/* ------------------------------------------------------------------------------------------------
 *                                         Local Functions
 * ------------------------------------------------------------------------------------------------
 */
static uint8 macSrcMatchFindEmptyEntry( uint8 macSrcMatchAddrMode );
static uint8 macSrcMatchCheckSrcAddr ( sAddr_t *addr, uint16 panID  );
static void macSrcMatchSetPendEnBit( uint8 index, uint8 macSrcMatchAddrMode );
static void macSrcMatchSetEnableBit( uint8 index, bool option, uint8 macSrcMatchAddrMode );
static bool macSrcMatchCheckEnableBit( uint8 index, uint8 macSrcMatchAddrMode );
static uint32 macSrcMatchGetEnableBit( uint8 macSrcMatchAddrMode );
static uint32 macSrcMatchGetPendEnBit( uint8 macSrcMatchAddrMode );
static void macSrcMatchModifyOption(void);

/*********************************************************************
 * @fn          MAC_SrcMatchEnable
 *
 * @brief      Enabled AUTOPEND and source address matching. If number of source
 *             address table entries asked for is more than the hardware
 *             supports. It will allocate maximum number of entries and return 
 *             MAC_INVALID_PARAMETER. This function shall be not be called from 
 *             ISR. It is not thread safe.
 *
 * @param      None
 *
 * @return     MAC_SUCCESS or MAC_INVALID_PARAMETER
 */
void MAC_SrcMatchEnable (void)
{
  /* Turn on Frame Filter (TIMAC enables frame filter by default) */
  MAC_RADIO_TURN_ON_RX_FRAME_FILTERING();
  
  /* Turn on Auto ACK (TIMAC turn on Auto ACK by default) */
  MAC_RADIO_TURN_ON_AUTO_ACK();
  
  /* Turn on Autopend: set SRCMATCH.AUTOPEND and SRCMATCH.SRC_MATCH_EN */
  MAC_RADIO_TURN_ON_SRC_MATCH();
 
  /* Set SRCMATCH.AUTOPEND */
  MAC_RADIO_TURN_ON_AUTOPEND();
  
  /* Turn on autopend for Data Request command only */
  MAC_RADIO_TURN_ON_PEND_DATA_REQ();
  
  /* Configure all the globals */
  macSrcMatchIsEnabled = TRUE;

  /* Apply change if needed */
  macSrcMatchModifyOption();
}

/*********************************************************************
 * @fn          MAC_SrcMatchAddEntry
 *
 * @brief       Add a short or extended address to source address table. This 
 *              function shall not be called from ISR. It is not thread safe.
 *
 * @param       addr - a pointer to sAddr_t which contains addrMode 
 *                     and a union of a short 16-bit MAC address or an extended 
 *                     64-bit MAC address to be added to the source address table. 
 * @param       panID - the device PAN ID. It is only used when the addr is 
 *                      using short address 

 * @return      MAC_SUCCESS or MAC_NO_RESOURCES (source address table full) 
 *              or MAC_DUPLICATED_ENTRY (the entry added is duplicated),
 *              or MAC_INVALID_PARAMETER if the input parameters are invalid.
 */
uint8 MAC_SrcMatchAddEntry ( sAddr_t *addr, uint16 panID )
{
  uint8 index;
  uint8 entry[MAC_SRCMATCH_SHORT_ENTRY_SIZE];
  
  /* Check if the input parameters are valid */
  if ( addr == NULL || (addr->addrMode != SADDR_MODE_SHORT && addr->addrMode != SADDR_MODE_EXT))
  {
    return MAC_INVALID_PARAMETER;  
  }
  
  /* Check if the entry already exists. Do not add duplicated entry */
  if ( macSrcMatchCheckSrcAddr( addr, panID ) != MAC_SRCMATCH_INVALID_INDEX )
  {
    return MAC_DUPLICATED_ENTRY; 
  }
  
  /* If not duplicated, write to the radio RAM and enable the control bit */
  
  /* Find the first empty entry */
  index = macSrcMatchFindEmptyEntry(addr->addrMode);
  
  if ( (index == MAC_SRCMATCH_SHORT_MAX_NUM_ENTRIES && addr->addrMode == SADDR_MODE_SHORT) || 
       (index == MAC_SRCMATCH_EXT_MAX_NUM_ENTRIES && addr->addrMode == SADDR_MODE_EXT) )
  {
    return MAC_NO_RESOURCES;   /* Table is full */
  }
  
  if ( addr->addrMode == SADDR_MODE_SHORT )
  {
    /* Write the PanID and short address */
    entry[0] = LO_UINT16( addr->addr.shortAddr );
    entry[1] = HI_UINT16( addr->addr.shortAddr );
    entry[2] = LO_UINT16( panID );  /* Little Endian for the radio RAM */
    entry[3] = HI_UINT16( panID );
    MAC_RADIO_SRC_MATCH_TABLE_SHORT_WRITE( index, entry, MAC_SRCMATCH_SHORT_ENTRY_SIZE );
    DBG_PRINT3(DBGSYS, "Short add: pan 0x%X, addr 0x%X index 0x%X", panID, addr->addr.shortAddr, index);
  }
  else
  {
    /* Write the extended address */
    MAC_RADIO_SRC_MATCH_TABLE_EXT_WRITE( index, addr->addr.extAddr, MAC_SRCMATCH_EXT_ENTRY_SIZE ); 
    DBG_PRINT4(DBGSYS, "Ext add[0123]: 0x%X, 0x%X 0x%X, 0x%X", addr->addr.extAddr[0], addr->addr.extAddr[1], addr->addr.extAddr[2], addr->addr.extAddr[3]);
    DBG_PRINT4(DBGSYS, "Ext addr add[4567]: 0x%X, 0x%X 0x%X, 0x%X", addr->addr.extAddr[4], addr->addr.extAddr[5], addr->addr.extAddr[6], addr->addr.extAddr[7]);
    DBG_PRINT1(DBGSYS, "Ext index: index 0x%X", index);
  }
  
  /* Set the Autopend enable bits */
  macSrcMatchSetPendEnBit( index, addr->addrMode );
  
  /* Set the Src Match enable bits */
  macSrcMatchSetEnableBit( index, TRUE, addr->addrMode);
  
  return MAC_SUCCESS;
}

/*********************************************************************
 * @fn         MAC_SrcMatchDeleteEntry
 *
 * @brief      Delete a short or extended address from source address table. 
 *             This function shall be not be called from ISR. It is not thread safe.
 *
 * @param      addr - a pointer to sAddr_t which contains addrMode 
 *                    and a union of a short 16-bit MAC address or an extended 
 *                    64-bit MAC address to be deleted from the source address table. 
 * @param      panID - the device PAN ID. It is only used when the addr is 
 *                     using short address  
 *
 * @return     MAC_SUCCESS or MAC_INVALID_PARAMETER (address to be deleted 
 *                  cannot be found in the source address table).
 */
uint8 MAC_SrcMatchDeleteEntry ( sAddr_t *addr, uint16 panID  )
{
  uint8 index;
  
  if ( addr == NULL || (addr->addrMode != SADDR_MODE_SHORT && addr->addrMode != SADDR_MODE_EXT))
  {
    return MAC_INVALID_PARAMETER;  
  }
  
  /* Look up the source address table and find the entry. */
  index = macSrcMatchCheckSrcAddr( addr, panID );

  if( index == MAC_SRCMATCH_INVALID_INDEX )
  {
    return MAC_INVALID_PARAMETER; 
  }
  
  /* Clear Src Match enable bits */
  macSrcMatchSetEnableBit( index, FALSE, addr->addrMode);

  return MAC_SUCCESS;
}
                  
/*********************************************************************
 * @fn          MAC_SrcMatchAckAllPending
 *
 * @brief       Enabled/disable acknowledging all packets with pending bit set
 *              The application normally enables it when adding new entries to 
 *              the source address table fails due to the table is full, or 
 *              disables it when more entries are deleted and the table has
 *              empty slots.
 *
 * @param       option - TRUE (acknowledging all packets with pending field set)
 *                       FALSE (address filtering and FSM control sets the pending field) 
 *
 * @return      none
 */
void MAC_SrcMatchAckAllPending ( uint8 option  ) 
{
  if( option == TRUE )
  {
    macSrcMatchIsAckAllPending = TRUE;
    
    /* Set the PENDING_OR register */
    MAC_RADIO_TURN_ON_PENDING_OR();
  }
  else
  {
    macSrcMatchIsAckAllPending = FALSE;
    
    /* Clear the PENDING_OR register */
    MAC_RADIO_TURN_OFF_PENDING_OR();
  }
}

/*********************************************************************
 * @fn          MAC_SrcMatchCheckAllPending
 *
 * @brief       Check if acknowledging all packets with pending bit set
 *              is enabled. 
 *
 * @param       none 
 *
 * @return      MAC_AUTOACK_PENDING_ALL_ON or MAC_AUTOACK_PENDING_ALL_OFF
 */
uint8 MAC_SrcMatchCheckAllPending ( void )
{
  if( macSrcMatchIsAckAllPending == TRUE )
  {
    return MAC_AUTOACK_PENDING_ALL_ON; 
  }
  
  return MAC_AUTOACK_PENDING_ALL_OFF;
}

/*********************************************************************
 * @fn          MAC_SrcMatchCheckResult
 *
 * @brief       Check the result of source matching
 *
 * @param       index - index of the entry in the source address table
 *
 * @return      TRUE or FALSE
 */
MAC_INTERNAL_API bool MAC_SrcMatchCheckResult( void )
{
  if ( macSrcMatchIsAckAllPending )
  {
    return (TRUE);
  }
  
  return ( (macSrcResIndex != MAC_SRCMATCH_INVALID_INDEX) );
}

/*********************************************************************
 * @fn          macSrcMatchFindEmptyEntry
 *
 * @brief       return index of the first empty entry found
 *
 * @param       macSrcMatchAddrMode - Source Matching Address Mode
 *
 * @return      uint8 - return index of the first empty entry found
 */
static uint8 macSrcMatchFindEmptyEntry( uint8 macSrcMatchAddrMode )
{
  uint8  index;
  uint32 enable;
  uint8  macSrcMatchMaxNumEntries;
     
  enable = MAC_RADIO_SRC_MATCH_GET_EN(macSrcMatchAddrMode);

  macSrcMatchMaxNumEntries = (macSrcMatchAddrMode == SADDR_MODE_SHORT)? 
                              MAC_SRCMATCH_SHORT_MAX_NUM_ENTRIES : 
                              MAC_SRCMATCH_EXT_MAX_NUM_ENTRIES;
  
  for( index = 0; index < macSrcMatchMaxNumEntries; index++ )
  {  
    if( ( enable & ( (uint32)0x01 << index ) ) == 0 )
    {
      return index;
    }
  }
  
  return index;
}

/*********************************************************************
 * @fn         macSrcMatchCheckSrcAddr
 *
 * @brief      Check if a short or extended address is in the source address table.
 *             This function shall not be called from ISR. It is not thread safe.
 *
 * @param      addr - a pointer to sAddr_t which contains addrMode 
 *                    and a union of a short 16-bit MAC address or an extended 
 *                    64-bit MAC address to be checked in the source address table. 
 * @param      panID - the device PAN ID. It is only used when the addr is 
 *                     using short address 

 * @return     uint8 - index of the entry in the table. Return 
 *                     MAC_SRCMATCH_INVALID_INDEX (0xFF) if address not found.
 */
static uint8 macSrcMatchCheckSrcAddr ( sAddr_t *addr, uint16 panID  )
{
  uint8 index;     
  uint8 *pAddr;
  uint8 entrySize;
  uint8 entry[MAC_SRCMATCH_SHORT_ENTRY_SIZE];  
  uint8 ramEntry[MAC_SRCMATCH_EXT_ENTRY_SIZE];
  uint8 macSrcMatchMaxNumEntries;
      
  if( addr->addrMode ==  SADDR_MODE_SHORT )
  {
    entry[0] = LO_UINT16( addr->addr.shortAddr );
    entry[1] = HI_UINT16( addr->addr.shortAddr );
    entry[2] = LO_UINT16( panID );  /* Little Endian for the radio RAM */
    entry[3] = HI_UINT16( panID );

    pAddr = entry;
    entrySize = MAC_SRCMATCH_SHORT_ENTRY_SIZE;
    macSrcMatchMaxNumEntries = MAC_SRCMATCH_SHORT_MAX_NUM_ENTRIES;
  }
  else
  {
    pAddr = addr->addr.extAddr;
    entrySize = MAC_SRCMATCH_EXT_ENTRY_SIZE;
    macSrcMatchMaxNumEntries = MAC_SRCMATCH_EXT_MAX_NUM_ENTRIES;
  }
  
  for( index = 0; index < macSrcMatchMaxNumEntries; index++ )
  {
    /* Check if the entry is enabled */
    if( macSrcMatchCheckEnableBit( index, addr->addrMode ) == FALSE )
    {
      continue; 
    }
      
    /* Compare the short address and pan ID */
    if( addr->addrMode ==  SADDR_MODE_SHORT )
    {
      MAC_RADIO_SRC_MATCH_TABLE_SHORT_READ( index, ramEntry, entrySize );
    }
    else
    {
      MAC_RADIO_SRC_MATCH_TABLE_EXT_READ( index, ramEntry, entrySize );
    }
     
    if( MAP_osal_memcmp( pAddr, ramEntry, entrySize ) == TRUE )
    {
      /* Match found */
      return index;
    }
  }
  
  return MAC_SRCMATCH_INVALID_INDEX;
}

/*********************************************************************
 * @fn          macSrcMatchSetPendEnBit
 *
 * @brief       Set the enable bit in the source address table
 *
 * @param       index - index of the entry in the source address table
 * @param       macSrcMatchAddrMode - Source Matching Address Mode
 *
 * @return      none
 */
static void macSrcMatchSetPendEnBit( uint8 index, uint8 macSrcMatchAddrMode )
{
  uint32 enable;
  
  enable = MAC_RADIO_SRC_MATCH_GET_PENDEN(macSrcMatchAddrMode) | ((uint32)0x01 << index);
  if( macSrcMatchAddrMode == SADDR_MODE_SHORT )
  {
    MAC_RADIO_SRC_MATCH_SET_SHORTPENDEN( enable );
  }
  else
  {
    MAC_RADIO_SRC_MATCH_SET_EXTPENDEN( enable );
  }
}

/*********************************************************************
 * @fn          macSrcMatchSetEnableBit
 *
 * @brief       Set or clear the enable bit in the SRCMATCH EN register
 *
 * @param       index - index of the entry in the source address table
 * @param       option - true (set the enable bit), or false (clear the enable bit)
 * @param       macSrcMatchAddrMode - Source Matching Address Mode

 *
 * @return      none
 */
static void macSrcMatchSetEnableBit( uint8 index, bool option, uint8 macSrcMatchAddrMode )
{
  uint32 enable;  
  
  enable = MAC_RADIO_SRC_MATCH_GET_EN(macSrcMatchAddrMode);
  if( option == TRUE )
  {
    enable |= ((uint32)0x01 << index);
    if( macSrcMatchAddrMode == SADDR_MODE_SHORT )
    {
      MAC_RADIO_SRC_MATCH_SET_SHORTEN( enable );
    }
    else
    {
      MAC_RADIO_SRC_MATCH_SET_EXTEN( enable );
    }
  }
  else
  {
    enable &= ~((uint32)0x01 << index);
    if( macSrcMatchAddrMode == SADDR_MODE_SHORT )
    {
      MAC_RADIO_SRC_MATCH_SET_SHORTEN( enable );
    }
    else
    {
      MAC_RADIO_SRC_MATCH_SET_EXTEN( enable );
    }
  }
}

/*********************************************************************
 * @fn          macSrcMatchCheckEnableBit
 *
 * @brief       Check the enable bit in the source address table
 *
 * @param       index - index of the entry in the source address table
 * @param       macSrcMatchAddrMode - Source Matching Address Mode
 *
 * @return      TRUE or FALSE
 */
static bool macSrcMatchCheckEnableBit( uint8 index, uint8 macSrcMatchAddrMode )
{
  uint32 enable;
  
  enable = MAC_RADIO_SRC_MATCH_GET_EN(macSrcMatchAddrMode);
  if( enable & ( (uint32)0x01 << index ) )
  {
    return TRUE;
  }
  
  return FALSE; 
}
 
/*********************************************************************
 * @fn          macSrcMatchGetEnableBit
 *
 * @brief       Return the SRCMATCH enable bitmap
 *
 * @param       macSrcMatchAddrMode - Source Matching Address Mode
 *
 * @return      uint32 - 32 bits bitmap
 */
static uint32 macSrcMatchGetEnableBit( uint8 macSrcMatchAddrMode )
{ 
  uint32 buf;
  
  if( macSrcMatchAddrMode == SADDR_MODE_SHORT )
  {
    MAC_RADIO_GET_SRC_SHORTEN( buf );
  }
  else
  {
    MAC_RADIO_GET_SRC_EXTEN( buf );
  }
  
  return buf;
}

/*********************************************************************
 * @fn          macSrcMatchGetPendEnBit
 *
 * @brief       Return the SRCMATCH Pend enable bitmap
 *
 * @param       macSrcMatchAddrMode - Source Matching Address Mode
 *
 * @return      uint32 - 32 bits bitmap
 */
static uint32 macSrcMatchGetPendEnBit( uint8 macSrcMatchAddrMode )
{
  uint32 buf;

  if( macSrcMatchAddrMode == SADDR_MODE_SHORT )
  {
    MAC_RADIO_GET_SRC_SHORTPENDEN( buf );
  }
  else
  {
    MAC_RADIO_GET_SRC_EXTENPEND( buf );
  }
  
  return buf;
}

static void macSrcMatchModifyOption(void)
{
    rfc_CMD_IEEE_MOD_FILT_t rfCmd;

    /* setup a radio command */
    rfCmd.commandNo = CMD_IEEE_MOD_FILT;
    MAP_osal_memcpy(&rfCmd.newFrameFiltOpt, &RF_cmdIEEERx.frameFiltOpt, sizeof(rfCmd.newFrameFiltOpt));
    MAP_osal_memcpy(&rfCmd.newFrameTypes, &RF_cmdIEEERx.frameTypes, sizeof(RF_cmdIEEERx.frameTypes));

    if (RF_cmdIEEERx.status == 2)
    {
        /* issue immediate command */
        RF_runImmediateCmd(RF_handle, (uint32_t*) &rfCmd);
    }
}
#endif
