/******************************************************************************

 @file  mt_ext.c

 @brief Extended Monitor/Test command/response processing

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
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

/******************************************************************************
 Includes
 *****************************************************************************/

#include "api_mac.h"
#include "npi_task.h"

#include "mt.h"
#include "mt_ext.h"
#include "mt_mac.h"
#include "mt_sys.h"
#include "mt_util.h"

/******************************************************************************
 Typedefs and Structures
 *****************************************************************************/
/*! Extended MT fragmentation process states */
typedef enum
{
    /*! Starting a new fragmentation sequence */
    FRAG_NEW,
    /*! Fragmentation sequence is active */
    FRAG_BUSY,
    /*! Incoming fragment is repeat of previous */
    FRAG_SAME,
    /*! Stopping an active fragmentation sequence */
    FRAG_STOP,
    /*! Fragmentation sequence is inactive */
    FRAG_DONE
} mtxState_t;

/*! Extended MT fragmentation control block */
typedef struct
{
/*! Fragmentation state */
    mtxState_t state;
    /*! Number of fragment blocks */
    uint8_t blocks;
    /*! Fragment data block length */
    uint8_t blkLen;
    /*! Data packet remaining length */
    uint16_t remLen;
    /*! RPC message parameter block */
    Mt_mpb_t mpb;
} mtxFCB_t;

/******************************************************************************
 Local variables
 *****************************************************************************/
/*! Incoming fragmentation control block */
static mtxFCB_t inFCB;

/*! Outgoing fragmentation control block */
static mtxFCB_t outFCB;

/******************************************************************************
 Local function prototypes
 *****************************************************************************/
static void abortFragMsg(uint8_t status);
static uint8_t processAck(Mt_mpb_t *pMpb);
static uint8_t processFragment(Mt_mpb_t *pMpb);
static uint8_t sendFragment(Mt_mpb_t *pMpb, bool next);
static void sendStatus(Mt_mpb_t *pMpb, uint8_t type, uint8_t status);

/******************************************************************************
 Public Functions
 *****************************************************************************/
/*!
 Initialize the MT system for use with NPI system

 Public function defined in mt_ext.h
 */
void MtExt_init(void)
{
    /* Indicate fragmentations are inactive */
    inFCB.state = FRAG_DONE;
    outFCB.state = FRAG_DONE;
}

/*!
 Process incoming extended MT command/acknowledge messages. Status return
 of MTRPC_EXT_FRAGDONE allows MT to call subsystem processing function.

 Public function defined in mt_ext.h
 */
uint8_t MtExt_processIncoming(Mt_mpb_t *pMpb)
{
    uint8_t len = pMpb->length;
    uint8_t status = MTRPC_ERR_LENGTH;

    if(len > 0)
    {
        uint8_t *pBuf = pMpb->pData;

        /* Save extended RPC frame info */
        pMpb->verid = pBuf[MTRPC_POS_VERS];
        pMpb->block = pBuf[MTRPC_POS_FBLK];

        switch(pMpb->verid & MTRPC_EXTVERS_MASK)
        {
            case MTRPC_EXT_STACKID:
                /* Skip over Version/StackId */
                pMpb->length = len - 1;
                pMpb->pData = &pBuf[MTRPC_POS_VERS + 1];
                status = MTRPC_EXT_FRAGDONE;
                break;

            case MTRPC_EXT_FRAGACK:
                if(len == MTRPC_FRAG_ACK_SZ)
                {
                    /* Process fragmentation ACK */
                    pMpb->length = 1;
                    pMpb->pData = &pBuf[MTRPC_POS_ACKS];
                    status = processAck(pMpb);
                }
                break;

            case MTRPC_EXT_FRAGBLK:
                if(len >= MTRPC_FRAG_HDR_SZ)
                {
                    /* Process fragment data block */
                    status = processFragment(pMpb);
                }
                break;

            case MTRPC_EXT_FRAGSTS:
                if(len == MTRPC_FRAG_ACK_SZ)
                {
                    status = MTRPC_EXT_SUCCESS;
                }
                break;

            default:
                /* Unsupported extended version type */
                status = MTRPC_ERR_EXTTYPE;
                break;
        }
    }

    return(status);
}

/*!
 Send message using extended MT fragmentation. If outgoing fragmentation is
 active, that sequence will be terminated and ACK with error status sent.

 Public function defined in mt_ext.h
 */
uint8_t MtExt_sendMessage(Mt_mpb_t *pMpb)
{
    uint8_t blkLen;
    uint8_t blocks;
    uint16_t remLen;
    uint8_t *(*pFragList);
    uint8_t err = MTRPC_SUCCESS;

    if(outFCB.state != FRAG_DONE)
    {
        /* Previous fragmentation sequence did not complete */
        abortFragMsg(MTRPC_EXT_FRAGABORT);
    }

    /* Length of full data block */
    remLen = pMpb->length;

    /* How many and what size fragments to make */
    blocks = (remLen / MTRPC_FRAG_MAX) + 1;
    blkLen = (remLen + blocks - 1) / blocks;

    /* Build a list of ptrs for outgoing fragment data blocks. Storing the
     * outgoing message in fragment-sized blocks instead of one large block
     * reduces the chance of memory allocation failure here and permits the
     * return of one fragment data block each time its ACK is received. */
    pFragList = MAP_ICall_malloc(blocks * sizeof(uint8_t*));
    if(pFragList != NULL)
    {
        uint8_t i;
        mtxState_t state = FRAG_NEW;

        /* Ptr to caller's 'long' outgoing data block */
        uint8_t *pData = pMpb->pData;

        /* Build list of outgoing fragments */
        for(i = 0; i < blocks; i++)
        {
            uint8_t cpyLen;
            uint8_t *pFrag = NULL;

            /* Length of data block to allocate this time */
            cpyLen = (blkLen < remLen) ? blkLen : remLen;

            if(state == FRAG_NEW)
            {
                /* Allocate memory for fragmentation header and data block */
                pFrag = (uint8_t*)MAP_ICall_malloc(MTRPC_FRAG_HDR_SZ + cpyLen);
            }
            pFragList[i] = pFrag;

            if(pFrag != NULL)
            {
                /* Set up fragmentation header */
                pFrag[MTRPC_POS_VERS] = MTRPC_EXT_FRAGBLK;
                pFrag[MTRPC_POS_FBLK] = i;
                (void)Util_bufferUint16(&pFrag[MTRPC_POS_FLEN], pMpb->length);

                /* Append the fragment data block */
                memcpy(&pFrag[MTRPC_POS_DATX], pData, cpyLen);

                /* Prepare for next fragment */
                pData += cpyLen;
                remLen -= cpyLen;
            }
            else
            {
                /* First allocation failure stops others */
                state = FRAG_STOP;
            }
        }

        /* Save elements used for send or abort */
        outFCB.state = state;
        outFCB.blocks = blocks;
    }

    /* Initialize outgoing MPB */
    outFCB.mpb = *pMpb;
    outFCB.mpb.block = 0;
    outFCB.mpb.pData = pFragList;
    outFCB.mpb.cmd0 |= MTRPC_CMD_EXTN;
    outFCB.mpb.verid = MTRPC_EXT_FRAGBLK;

    if(outFCB.state == FRAG_NEW)
    {
        /* Get ready to start sending fragments */
        outFCB.blkLen = blkLen;
        outFCB.remLen = pMpb->length;
        /* Send the first fragment */
        err = sendFragment(&outFCB.mpb, false);
    }
    else
    {
        /* Not enough memory to store fragment(s) */
        abortFragMsg(MTRPC_EXT_NOMEMORY);
    }

    /* Report dropped message */
    return(err);
}

/******************************************************************************
 Local Functions
 *****************************************************************************/
/*!
 * @brief   Abort outgoing fragmentation and issue status message
 *
 * @param   status - fragmentation status code
 */
static void abortFragMsg(uint8_t status)
{
    uint8_t *(*pFragList) = outFCB.mpb.pData;

    if(pFragList != NULL)
    {
        uint8_t i;

        /* Delete list of outgoing fragments */
        for(i = 0; i < outFCB.blocks; i++)
        {
            if(pFragList[i] != NULL)
            {
                MAP_ICall_free(pFragList[i]);
            }
        }

        /* Delete list of fragment ptrs */
        MAP_ICall_free(pFragList);
        outFCB.mpb.pData = NULL;
    }

    /* Send ACK message for this fragment */
    sendStatus(&outFCB.mpb, MTRPC_EXT_FRAGSTS, status);

    /* Allow another fragmentation sequence to start */
    outFCB.state = FRAG_DONE;
}

/*!
 * @brief   Process an incoming fragmentation acknowledge frame
 *
 * @param   pMpb - pointer to RPC command/response message parameter block
 *
 * @return  Status of processing the ACK message
 */
static uint8_t processAck(Mt_mpb_t *pMpb)
{
    uint8_t status = MTRPC_EXT_SUCCESS;

    if(outFCB.state == FRAG_BUSY)
    {
        if((pMpb->cmd0 ^ outFCB.mpb.cmd0) || (pMpb->cmd1 ^ outFCB.mpb.cmd1) ||
          ((pMpb->verid ^ outFCB.mpb.verid) & MTRPC_STACKID_MASK))
        {
            /* Command info doesn't match last fragment */
            abortFragMsg(MTRPC_EXT_FRAGABORT);
        }
        else if(pMpb->block != outFCB.mpb.block)
        {
            /* Not expecting this block number */
            status = MTRPC_EXT_BADBLOCK;
        }
        else
        {
            switch(*((uint8_t*)pMpb->pData))
            {
                case MTRPC_EXT_SUCCESS:
                    /* Send next fragment */
                    status = sendFragment(pMpb, true);
                    break;

                case MTRPC_EXT_RESEND:
                    /* Resend last fragment */
                    status = sendFragment(pMpb, false);
                    break;

                case MTRPC_EXT_FRAGDONE:
                    if(pMpb->block == (outFCB.blocks - 1))
                    {
                        /* Final fragment was accepted */
                        status = sendFragment(pMpb, true);
                    }
                    else
                    {
                        /* Not supposed to be done yet */
                        abortFragMsg(MTRPC_EXT_FRAGABORT);
                    }
                    break;

                case MTRPC_EXT_BADSTACK:
                case MTRPC_EXT_BADBLOCK:
                case MTRPC_EXT_BADLENGTH:
                case MTRPC_EXT_NOMEMORY:
                case MTRPC_EXT_FRAGABORT:
                    /* Fatal ACK from host on last fragment */
                    abortFragMsg(MTRPC_EXT_FRAGABORT);
                    break;

                default:
                    status = MTRPC_EXT_BADACK;
                    break;
            }
        }
    }

    if(status != MTRPC_EXT_SUCCESS)
    {
        /* Report the ACK problem now */
        sendStatus(pMpb, MTRPC_EXT_FRAGSTS, status);
        /* This is not an RPC problem */
        status = MTRPC_SUCCESS;
    }

    return(status);
}

/*!
 * @brief   Process an incoming fragmentation data frame
 *
 * @param   pMpb - pointer to RPC command/response message parameter block
 *
 * @return  Status of processing the Fragment message
 */
static uint8_t processFragment(Mt_mpb_t *pMpb)
{
    uint8_t *pBuf = pMpb->pData;
    uint8_t status = MTRPC_EXT_SUCCESS;
    uint8_t blkLen = pMpb->length - MTRPC_FRAG_HDR_SZ;
    uint16_t pktLen = Util_parseUint16(&pBuf[MTRPC_POS_FLEN]);

    /* Validate incoming fragment */
    if(inFCB.state == FRAG_BUSY)
    {
        if((inFCB.mpb.cmd0 != pMpb->cmd0) ||
           (inFCB.mpb.cmd1 != pMpb->cmd1) ||
           (inFCB.mpb.block > pMpb->block))
        {
            /* MT command codes shouldn't be changing */
            status = MTRPC_EXT_BADBLOCK;
        }
        else if(inFCB.mpb.verid != pMpb->verid)
        {
            /* Version/Stack ID shouldn't be changing */
            status = MTRPC_EXT_BADSTACK;
        }
        else if(inFCB.mpb.length != pktLen)
        {
            /* Full packet length shouldn't be changing */
            status = MTRPC_EXT_BADLENGTH;
        }
        else if(inFCB.mpb.block == pMpb->block)
        {
            /* Assume same block as last one */
            inFCB.state = FRAG_SAME;

            if(inFCB.blkLen != blkLen)
            {
                /* Fragment length shouldn't be changing */
                status = MTRPC_EXT_BADLENGTH;
            }
        }
        else if((inFCB.blkLen != blkLen) &&
                (inFCB.remLen != blkLen))
        {
            /* Fragment length is wrong */
            status = MTRPC_EXT_BADLENGTH;
        }

        if(status != MTRPC_EXT_SUCCESS)
        {
            /* Aborting RX, give back incoming packet buffer */
            MAP_ICall_free(inFCB.mpb.pData);
            inFCB.mpb.block = pMpb->block;
            inFCB.state = FRAG_DONE;
        }
    }

    /* Start a new fragmentation sequence? */
    if(inFCB.state == FRAG_DONE)
    {
        if(pMpb->block == 0)
        {
            /* Save command info (for validation and ACKs) */
            inFCB.mpb = *pMpb;

            /* Get a buffer to hold 'large' packet */
            inFCB.mpb.pData = MAP_ICall_malloc(pktLen);
            if(inFCB.mpb.pData != NULL)
            {
                /* Good to go - start fragment reception process */
                inFCB.blocks = 0;
                inFCB.blkLen = blkLen;
                inFCB.remLen = pktLen;
                inFCB.mpb.length = pktLen;
                inFCB.state = FRAG_NEW;
            }
            else
            {
                /* Nowhere to store the packet */
                status = MTRPC_ERR_NOMEMORY;
            }
        }
        else if(status == MTRPC_EXT_SUCCESS)
        {
            /* Must start with first block */
            status = MTRPC_EXT_BADBLOCK;
        }
    }

    /* Continuing an active fragmentation sequence? */
    if(inFCB.state != FRAG_DONE)
    {
        if(inFCB.state != FRAG_SAME)
        {
            uint8_t *pData = inFCB.mpb.pData;
            uint16_t ofs = pMpb->block * inFCB.blkLen;

            /* Copy the incoming data fragment */
            memcpy(&pData[ofs], &pBuf[MTRPC_POS_DATX], blkLen);

            /* Update 'finished' parameters */
            inFCB.mpb.block = pMpb->block;
            inFCB.remLen -= blkLen;

            if(inFCB.remLen == 0)
            {
                /* All fragments have been reassembled */
                pMpb->pData = pData;
                pMpb->length = pktLen;
                inFCB.state = FRAG_DONE;
                status = MTRPC_EXT_FRAGDONE;
            }
        }

        if(inFCB.state != FRAG_DONE)
        {
            /*  NEW, SAME, BUSY states are now just BUSY */
            inFCB.state = FRAG_BUSY;
        }
    }

    /* Send ACK message for this fragment */
    sendStatus(&inFCB.mpb, MTRPC_EXT_FRAGACK, status);

    return(status);
}

/*!
 * @brief   Send next outgoing MT RPC fragment message to the NPI task.
 *          Buffering of the outgoing data packet was done by allocating
 *          multiple memory blocks of the fragment size instead of one
 *          large block. This may increase robustness in two ways: less
 *          chance of memory allocation failure on a single large block,
 *          and return of some heap memory after each fragment gets ACKed.
 *
 * @param   pMpb - pointer to RPC command/response message parameter block
 * @param   next - false=resend block, true=advance to next block
 *
 * @return  err  - message dropped if not MTRPC_SUCCESS
 */
static uint8_t sendFragment(Mt_mpb_t *pMpb, bool next)
{
    uint8_t err = MTRPC_SUCCESS;

    /* Check whether a fragment should go out */
    if(outFCB.state != FRAG_DONE)
    {
        uint8_t block = pMpb->block;
        uint8_t *(*pFragList) = outFCB.mpb.pData;

        /* If the previous outgoing fragment has been ACKed... */
        if((outFCB.state == FRAG_BUSY) && (next == true))
        {
            uint8_t *pFrag = pFragList[block];

            if(pFrag != NULL)
            {
                /* Release the fragment memory */
                MAP_ICall_free(pFrag);
                /* Don't do it again */
                pFragList[block] = NULL;
            }

            /* Index of next block to send */
            block++;

            if(block < outFCB.blocks)
            {
                /* Update the block number */
                outFCB.mpb.block = block;
                /* Reduce the remaining length */
                outFCB.remLen -= outFCB.blkLen;
            }
            else
            {
                /* Nothing left to send */
                outFCB.remLen = 0;
                /* Done with this packet */
                outFCB.state = FRAG_STOP;
            }
        }

        if(outFCB.state != FRAG_STOP)
        {
            uint8_t dLen;
            uint8_t fLen;
            uint8_t *pFrag = pFragList[block];

            /* Length of data block - last one may be shorter than others */
            dLen = (block < (outFCB.blocks-1)) ? outFCB.blkLen : outFCB.remLen;

            /* Length of outgoing buffer - header and data */
            fLen = MTRPC_FRAG_HDR_SZ + dLen;

            /* Indicate that an ACK is expected */
            outFCB.state = FRAG_BUSY;

            /* Send out this fragment block */
            err = MT_sendResponse(outFCB.mpb.cmd0, outFCB.mpb.cmd1, fLen, pFrag);

            if(err != MTRPC_EXT_SUCCESS)
            {
                /* Failure to send this fragment kills the process */
                outFCB.state = FRAG_STOP;
            }
        }
    }

    if(outFCB.state == FRAG_STOP)
    {
        uint8_t status;

        /* Memory allocation error if fragment data remaining */
        status = (outFCB.remLen) ? MTRPC_EXT_NOMEMORY : MTRPC_EXT_FRAGDONE;

        /* Clean up and get ready for next one */
        abortFragMsg(status);
    }

    /* Report dropped message */
    return(err);
}

/*!
 * @brief   Format and send fragmentation status (ACK/STS) message
 *
 * @param   pMpb   - pointer to message parameter block
 * @param   type   - status type
 * @param   status - status code
 */
static void sendStatus(Mt_mpb_t *pMpb, uint8_t type, uint8_t status)
{
    uint8_t ack[MTRPC_FRAG_ACK_SZ];

    /* Build fragmentation ACK message */
    ack[MTRPC_POS_VERS] = type | (pMpb->verid & MTRPC_STACKID_MASK);
    ack[MTRPC_POS_FBLK] = pMpb->block;
    ack[MTRPC_POS_ACKS] = status;

    /* Send the message */
    (void)MT_sendResponse(pMpb->cmd0 | MTRPC_CMD_AREQ,
                          pMpb->cmd1, sizeof(ack), ack);
}
