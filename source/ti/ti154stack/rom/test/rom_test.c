/******************************************************************************

 @file rom_test.c

 @brief TIMAC 2.0 FH API

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated

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

/******************************************************************************
 Includes
 *****************************************************************************/


#include "hal_types.h"
#include "mac_main.h"
#include "mac_data.h"
#include "fh_api.h"
#include "fh_pib.h"
#include "fh_nt.h"
#include "fh_data.h"
#include "fh_util.h"
#include "fh_mgr.h"
#include "fh_sm.h"
#include "fh_ie.h"
#include "string.h"
#include "mac_high_level.h"
#include "fh_dh1cf.h"

/* LMAC function prototypes */
#include "mac.h"
#include "mac_rx.h"
#include "mac_radio.h"
#include "mac_tx.h"
#include "mac_mcu.h"
#include "mac_mgmt.h"
#include "mac_spec.h"
#include "osal_memory.h"

// SW Tracer
#include "dbg.h"
#ifdef DEBUG_SW_TRACE
#define DBG_ENABLE
#ifndef _DBGFILE
#define _DBGFILE fh_api_c
#endif
#include "dbgid_sys_mst.h"
#endif // DEBUG_SW_TRACE

//#pragma LOCATION(RAM_BASE_ADDR,0x20000200)
//uint32_t RAM_BASE_ADDR[4];


/*!
 This module is the FH module interface for the HMAC and LMAC. All function
 APIS in this file can be used by HMAC and LMAC.
 */
uint32_t x = 30;
//uint32_t y = *(uint32 *) (0x4000);

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/******************************************************************************
 Local variables
 *****************************************************************************/

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/

/******************************************************************************
 Glocal variables
 *****************************************************************************/
#include "rom_jt_154.h"

#if 0
#define MAP_test_func2      ( (void)(*)(int) (uint32 *)(0x50000))
void test_fh_func2(int x1)
{
    static int x=0;
    x++;
}

#define MAP_test_func2      ( (void (*)(int) ) test_fh_func2 )
void test_fh_func(void)
{
    static int x=0;
    x++;
    test_fh_func2(x);
    MAP_test_func2(10);
    // test_fh_func(FH_HND *pFhHnd, ...)
    //              HMAC_HND *pHmacHnd
    //              MAC_HND *pHnd
    // 1. function call first para ==> handler
    // 2. FH, HMAC,LMAC replacement

    //pFhHnd->test_func2();
    //pHmacHnd->test_func2();

    //RAM_BASE_ADDR[0] = 0x2001;
}
#endif


