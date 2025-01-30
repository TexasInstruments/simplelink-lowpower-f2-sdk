/******************************************************************************

 @file  mac_security.h

 @brief Internal interface file for MAC security module.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2010-2025, Texas Instruments Incorporated

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

#ifndef MAC_SECURITY_H
#define MAC_SECURITY_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#ifdef FEATURE_MAC_SECURITY

#include "mac_api.h"
#include "mac_high_level.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* Max frame counter */
#define MAC_MAX_FRAME_COUNTER        0xFFFFFFFF

/* Nonce length */
#define MAC_NONCE_LEN                13

/* MIC length */
#define MAC_MIC_LEN                  16

/* This MAC status is only locally used in MAC security */
#define MAC_CONDITIONALLY_PASSED     (MAC_IMPROPER_SECURITY_LEVEL-1)

/* Device lookup size short */
#define MAC_DEVICE_LOOKUP_SHORT_LEN  4

/* Device lookup size lonh */
#define MAC_DEVICE_LOOKUP_LONG_LEN   8


/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */
/* Length M of authentication tag indexed by security level */
extern CODE const uint8 macAuthTagLen[];

/* Length of key source indexed by key identifier mode */
extern CODE const uint8 macKeySourceLen[];

/* ------------------------------------------------------------------------------------------------
 *                                           Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */
/**************************************************************************************************
 * @fn          macOutgoingFrameSecurity
 *
 * @brief       The inputs to this procedure are the frame to be secured and the security
 *              parameters from the originating primitive or automatic request PIB attributes.
 *              The outputs from this procedure are the status of the procedure and, if this status
 *              is MAC_SUCCESS, pointer to the key to be used to secure the outgoing frame.
 *
 * input parameters
 *
 * @param       pBuf - Pointer to buffer containing tx struct.
 * @param       pDstAddr - Destination address.
 * @param       dstPanId - Destination PAN ID.
 *
 * output parameters
 *
 * @param       ppKeyDesc - Pointer to pointer to key descripter to be used to secure
 *                          the outgoing frame.
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macOutgoingFrameSecurity( macTx_t  *pBuf,
                                                 sAddr_t  *pDstAddr,
                                                 uint16   dstPanId,
                                                 keyDescriptor_t **ppKeyDesc );


/**************************************************************************************************
 * @fn          macIncomingFrameSecurity
 *
 * @brief       The input to this procedure is the frame to be unsecured. The outputs from this
 *              procedure are the unsecured frame, the security level, the key identifier mode, the
 *              key source, the key index, and the status of the procedure. All outputs of this
 *              procedure are assumed to be invalid unless and until explicitly set in this
 *              procedure. It is assumed that the PIB attributes associating KeyDescriptors in
 *              macKeyTable with a single, unique device or a number of devices will have been
 *              established by the next higher layer.
 *
 * input parameters
 *
 * @param       pMsg - pointer to the frame to be unsecured.
 *
 * output parameters
 *
 * @param       pMsg - pointer to the unsecured frame
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macIncomingFrameSecurity( macRx_t *pMsg );


/**************************************************************************************************
 * @fn          macCcmStarTransform
 *
 * @brief       This function is used to do CCM* transformation. The inputs to this procedure are
 *              the key, nonce, a data, m data. The output from this procedure is c data.
 *
 * input parameters
 *
 * @param       pKey - pointer to key
 * @param       frameCounter - frame counter value
 * @param       securityLevel - security level
 * @param       pAData - pointer to a data
 * @param       aDataLen - a data length
 * @param       pMData - pointer to m data
 * @param       mDataLen - m data length
 *
 * output parameters
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macCcmStarTransform( uint8    *pKey,
                                            uint32   frameCounter,
                                            uint8    securityLevel,
                                            uint8    *pAData,
                                            uint16   aDataLen,
                                            uint8    *pMData,
                                            uint16   mDataLen );

								
/**************************************************************************************************
 * @fn          macCcmStarInverseTransform
 *
 * @brief       This function is used to do CCM* inverse transformation. The inputs to this
 *              procedure are ExtAddress of the device descriptor, the frame counter, the security
 *              level, the key, and the secured frame. The output from this procedure is c data.
 *
 * input parameters
 *
 * @param       pKey - pointer to key
 * @param       frameCounter - frame counter
 * @param       securityLevel - security level
 * @param       pAData - pointer to a data
 * @param       aDataLen - a data length
 * @param       pCData - pointer to c data
 * @param       cDataLen - c data length
 * @param       pExtAddr - pointer to exteneded address
 *
 * output parameters
 *
 * @return      MAC_SUCCESS if successful, otherwise failure status.
 **************************************************************************************************
 */
MAC_INTERNAL_API uint8 macCcmStarInverseTransform( uint8    *pKey,
                                                   uint32   frameCounter,
                                                   uint8    securityLevel,
                                                   uint8    *pAData,
                                                   uint16   aDataLen,
                                                   uint8    *pCData,
                                                   uint16   cDataLen,
                                                   uint8    *pExtAddr );
/**************************************************************************************************
 * @fn          macGetEUIIndex
 *
 * @brief       This function is used to get the device table index (EUI index) based on the given
 *              EUI address. if the corresponding entry is not found, it will return the
 *              MAC_EUI_INVALID_INDEX
 *
 * input parameters
 *
 * @param       pEUI - pointer to EUI address
 *
 * output parameters
 *
 * @return      0xFF : invalid device table entry (EUI is not found)
 *              other : index of device table
 **************************************************************************************************
 */
MAC_INTERNAL_API uint16_t macGetEUIIndex(sAddrExt_t *pEUI);

/**************************************************************************************************
 * @fn          macGetEUI
 *
 * @brief       This function is used to get (retreieve) EUI address for device table.
 *
 * input parameters
 *
 * @param       euiIndex - index of device table
 *
 * output parameters
 *
 * @return      pointer of EUI address in the devic etable
 **************************************************************************************************
 */
MAC_INTERNAL_API sAddrExt_t* macGetEUI(uint16_t euiIndex);

#endif // FEATURE_MAC_SECURITY

#endif /* MAC_SECURITY_H */

