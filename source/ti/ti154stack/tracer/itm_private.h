/******************************************************************************

 @file  itm_private.h

 @brief Provides data logging

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2011-2025, Texas Instruments Incorporated

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

#ifndef ITM_PRIVATE_H_
#define ITM_PRIVATE_H_

// Define for major and minor version
#define ITM_LIB_MAJOR_VER   0
#define ITM_LIB_MINOR_VER   5

#define SCS_DEMCR               (*(volatile uint32_t *)0xE000EDFC)
#define SCS_DEMCR_TRCEN         (0x01000000)

#define CS_LAR_UNLOCK          (0xC5ACCE55)

#define ITM_STIM_PORT(x)        (0xE0000000 + 4*x)

#define ITM_TER                 (*(volatile uint32_t *)0xE0000E00)
#define ITM_TER_ENABLE_ALL      (0xFFFFFFFF)

#define ITM_TCR                 (*(volatile uint32_t *)0xE0000E80)
#define ITM_TCR_ENABLE_ITM      (0x00000001)
#define ITM_TCR_ENABLE_TS       (0x00000002)
#define ITM_TCR_ENABLE_SYNC     (0x00000004)
#define ITM_TCR_ENABLE_DWT_TX   (0x00000008)
#define ITM_TCR_ENABLE_SWO      (0x00000010)
#define ITM_TCR_BUSY            (0x00800000)
#define ITM_TCR_TS_PRESCALE_SHIFT  (8)
#define ITM_TCR_TS_PRESCALE_MASK   (0x00000300)

#define ITM_TPR                 (*(volatile uint32_t *)0xE0000E40)
#define ITM_TPR_ENABLE_USER_ALL (0x0000000F)

#define ITM_LAR                 (*(volatile uint32_t *)0xE0000FB0)

#define TPIU_ACPR               (*(volatile uint32_t *)0xE0040010)
#define TPIU_ACPR_MASK          (0x00001FFF)

#define TPIU_SPPR               (*(volatile uint32_t *)0xE00400F0)
#define TPIU_SPPR_SWO_UART      (0x00000002)

#define TPIU_FFCR               (*(volatile uint32_t *)0xE0040304)
#define TPIU_FFCR_DISABLE       (0x00000100)

#define TPIU_LAR                (*(volatile uint32_t *)0xE0040FB0)

#define TPIU_CSPSR              (*(volatile uint32_t *)0xE0040004)
#define TPIU_CSPSR_PIN_1        (0x00000001)

#define DWT_CTRL                 (*(volatile uint32_t *)0xE0001000)
#define DWT_CTRL_MASK_NUM_COMP   (0xF0000000)
#define DWT_CTRL_SHIFT_NUM_COMP  (28)
#define DWT_CTRL_ENABLE_PC_SAMP  (0x00001000)
#define DWT_CTRL_ENABLE_EXC_TRC  (0x00010000)
#define DWT_CTRL_ENABLE_CYC_EVT  (0x00400000)
#define DWT_CTRL_ENABLE_CYC_CNT  (0x00000001)
#define DWT_CTRL_CYC_CNT_1024    (0x0000001E)
#define DWT_CTRL_MASK_SYNCTAP    (0x00000C00)
#define DWT_CTRL_SHIFT_SYNCTAP   (10)


#define DWT_LAR                  (*(volatile uint32_t *)0xE0001FB0)

#define DWT_COMP(x)                 (*(volatile uint32_t *)(0xE0001020 + 16 *(x)))
#define DWT_MASK(x)                 (*(volatile uint32_t *)(0xE0001024 + 16 *(x)))
#define DWT_FUNC(x)                 (*(volatile uint32_t *)(0xE0001028 + 16 *(x)))
#define DWT_FUNC_DATA_SIZE_32       (0x00000800)
#define DWT_FUNC_ENABLE_DATA_MATCH  (0x00000100)
#define DWT_FUNC_ENABLE_ADDR_OFFSET (0x00000020)
#define DWT_FUNC_ENABLE_COMP_RW     (0x00000002)


#endif /* ITM_PRIVATE_H_ */
