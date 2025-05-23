/*
 *    Copyright (c) 2016, The OpenThread Authors.
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 *    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file contains definitions for a UART based NCP interface to the OpenThread stack.
 */

#include "ncp_uart.hpp"

#include <stdio.h>

#include <openthread/ncp.h>
#include <openthread/platform/logging.h>
#include <openthread/platform/misc.h>
#include <openthread/platform/uart.h>

#include "openthread-core-config.h"
#include "common/code_utils.hpp"
#include "common/debug.hpp"
#include "common/instance.hpp"
#include "common/new.hpp"
//#include "net/ip6.hpp"
#include "utils/static_assert.hpp"

#if OPENTHREAD_CONFIG_DIAG_ENABLE
OT_STATIC_ASSERT(OPENTHREAD_CONFIG_DIAG_OUTPUT_BUFFER_SIZE <= CONFIG_NCP_UART_RX_BUFFER_SIZE -
                                                                  ot::Ncp::NcpBase::kSpinelCmdHeaderSize -
                                                                  ot::Ncp::NcpBase::kSpinelPropIdSize,
                 "diag output should be smaller than NCP UART rx buffer");

OT_STATIC_ASSERT(OPENTHREAD_CONFIG_DIAG_CMD_LINE_BUFFER_SIZE <= CONFIG_NCP_UART_RX_BUFFER_SIZE,
                 "diag command line should be smaller than NCP UART rx buffer");
#endif
#ifdef WISUN_FAN_DEBUG
volatile uint32_t uart_frame_error =0;
volatile uint32_t uart_frame_ok=0;
#endif
namespace ot {
namespace Ncp {

extern "C" void platformNcpSendRspSignal();

static OT_DEFINE_ALIGNED_VAR(sNcpRaw, sizeof(NcpUart), uint64_t);

extern "C" void otNcpInit(otInstance *aInstance)
{
    NcpUart * ncpUart  = NULL;
    Instance *instance = static_cast<Instance *>(aInstance);

    ncpUart = new (&sNcpRaw) NcpUart(instance);

    if (ncpUart == NULL || ncpUart != NcpBase::GetNcpInstance())
    {
        OT_ASSERT(false);
    }
}

extern "C" void platformNcpSendProcess()
{
    NcpUart *ncpUart = static_cast<NcpUart *>(NcpBase::GetNcpInstance());
    ncpUart->EncodeAndSendToUart();
}


NcpUart::NcpUart(Instance *aInstance)
    : NcpBase(aInstance)
    , mFrameEncoder(mUartBuffer)
    , mFrameDecoder(mRxBuffer, &NcpUart::HandleFrame, this)
    , mUartBuffer()
    , mState(kStartingFrame)
    , mByte(0)
    , mRxBuffer()
    , mUartSendImmediate(false)
    //mv , mUartSendTask(*aInstance, EncodeAndSendToUart, this)
#if OPENTHREAD_ENABLE_NCP_SPINEL_ENCRYPTER
    , mTxFrameBufferEncrypterReader(mTxFrameBuffer)
#endif // OPENTHREAD_ENABLE_NCP_SPINEL_ENCRYPTER
{
    mTxFrameBuffer.SetFrameAddedCallback(HandleFrameAddedToNcpBuffer, this);

    otPlatUartEnable();
}

void NcpUart::HandleFrameAddedToNcpBuffer(void *                   aContext,
                                          Spinel::Buffer::FrameTag aTag,
                                          Spinel::Buffer::Priority aPriority,
                                          Spinel::Buffer *         aBuffer)
{
    OT_UNUSED_VARIABLE(aBuffer);
    OT_UNUSED_VARIABLE(aTag);
    OT_UNUSED_VARIABLE(aPriority);

    static_cast<NcpUart *>(aContext)->HandleFrameAddedToNcpBuffer();
}


void NcpUart::HandleFrameAddedToNcpBuffer(void)
{
    if (mUartBuffer.IsEmpty())
    {
        //mv mUartSendTask.Post();
        //post event to Ncp tasklet
        platformNcpSendRspSignal();
    }
}

#if 0//mv
void NcpUart::EncodeAndSendToUart(Tasklet &aTasklet)
{
    OT_UNUSED_VARIABLE(aTasklet);
    static_cast<NcpUart *>(GetNcpInstance())->EncodeAndSendToUart();
}
#endif


// This method encodes a frame from the tx frame buffer (mTxFrameBuffer) into the uart buffer and sends it over uart.
// If the uart buffer gets full, it sends the current encoded portion. This method remembers current state, so on
// sub-sequent calls, it restarts encoding the bytes from where it left of in the frame .
void NcpUart::EncodeAndSendToUart(void)
{
    uint16_t len;
    bool     prevHostPowerState;
#if OPENTHREAD_ENABLE_NCP_SPINEL_ENCRYPTER
    Spinel::BufferEncrypterReader &txFrameBuffer = mTxFrameBufferEncrypterReader;
#else
    Spinel::Buffer &txFrameBuffer = mTxFrameBuffer;
#endif // OPENTHREAD_ENABLE_NCP_SPINEL_ENCRYPTER

    while (!txFrameBuffer.IsEmpty() || (mState == kFinalizingFrame))
    {
        switch (mState)
        {
        case kStartingFrame:

            if (super_t::ShouldWakeHost())
            {
                otPlatWakeHost();
            }

            VerifyOrExit(!super_t::ShouldDeferHostSend(), OT_NOOP);
            SuccessOrExit(mFrameEncoder.BeginFrame());

            txFrameBuffer.OutFrameBegin();

            mState = kEncodingFrame;

            while (!txFrameBuffer.OutFrameHasEnded())
            {
                mByte = txFrameBuffer.OutFrameReadByte();

            case kEncodingFrame:

                SuccessOrExit(mFrameEncoder.Encode(mByte));
            }

            // track the change of mHostPowerStateInProgress by the
            // call to OutFrameRemove.
            prevHostPowerState = mHostPowerStateInProgress;

            txFrameBuffer.OutFrameRemove();

            if (prevHostPowerState && !mHostPowerStateInProgress)
            {
                // If mHostPowerStateInProgress transitioned from true -> false
                // in the call to OutFrameRemove, then the frame should be sent
                // out the UART without attempting to push any new frames into
                // the mUartBuffer. This is necessary to avoid prematurely calling
                // otPlatWakeHost.
                mUartSendImmediate = true;
            }

            mState = kFinalizingFrame;

            // fall through

        case kFinalizingFrame:

            SuccessOrExit(mFrameEncoder.EndFrame());

            mState = kStartingFrame;

            if (mUartSendImmediate)
            {
                // clear state and break;
                mUartSendImmediate = false;
                break;
            }
        }
    }

exit:
    len = mUartBuffer.GetLength();

    if (len > 0)
    {
        if (otPlatUartSend(mUartBuffer.GetFrame(), len) != OT_ERROR_NONE)
        {
            OT_ASSERT(false);
        }
    }
}

extern "C" void otPlatUartSendDone(void)
{
    NcpUart *ncpUart = static_cast<NcpUart *>(NcpBase::GetNcpInstance());

    if (ncpUart != NULL)
    {
        ncpUart->HandleUartSendDone();
    }
}

void NcpUart::HandleUartSendDone(void)
{
    mUartBuffer.Clear();

    //mv mUartSendTask.Post();
    //post event to Ncp tasklet
    platformNcpSendRspSignal();
}

extern "C" void otPlatUartReceived(const uint8_t *aBuf, uint16_t aBufLength)
{
    NcpUart *ncpUart = static_cast<NcpUart *>(NcpBase::GetNcpInstance());

    if (ncpUart != NULL)
    {
        ncpUart->HandleUartReceiveDone(aBuf, aBufLength);
    }
}

void NcpUart::HandleUartReceiveDone(const uint8_t *aBuf, uint16_t aBufLength)
{
    mFrameDecoder.Decode(aBuf, aBufLength);
}

void NcpUart::HandleFrame(void *aContext, otError aError)
{
    static_cast<NcpUart *>(aContext)->HandleFrame(aError);
}

void NcpUart::HandleFrame(otError aError)
{
    uint8_t *buf       = mRxBuffer.GetFrame();
    uint16_t bufLength = mRxBuffer.GetLength();

    if (aError == OT_ERROR_NONE)
    {
#ifdef WISUN_FAN_DEBUG
        uart_frame_ok++;
#endif

#if OPENTHREAD_ENABLE_NCP_SPINEL_ENCRYPTER
        size_t dataLen = bufLength;
        if (SpinelEncrypter::DecryptInbound(buf, kRxBufferSize, &dataLen))
        {
            super_t::HandleReceive(buf, dataLen);
        }
#else
        super_t::HandleReceive(buf, bufLength);
#endif // OPENTHREAD_ENABLE_NCP_SPINEL_ENCRYPTER
    }
    else
    {
#ifdef WISUN_FAN_DEBUG
        uart_frame_error++;
#endif
        HandleError(aError, buf, bufLength);
    }

    mRxBuffer.Clear();
}

void NcpUart::HandleError(otError aError, uint8_t *aBuf, uint16_t aBufLength)
{
    char     hexbuf[128];
    uint16_t i = 0;

    super_t::IncrementFrameErrorCounter();

    /*
    // We can get away with sprintf because we know
    // `hexbuf` is large enough.
    snprintf(hexbuf, sizeof(hexbuf), "Framing error %d: [", aError);

    // Write out the first part of our log message.
    otNcpStreamWrite(0, reinterpret_cast<uint8_t *>(hexbuf), static_cast<int>(strlen(hexbuf)));

    // The first '3' comes from the trailing "]\n\000" at the end o the string.
    // The second '3' comes from the length of two hex digits and a space.
    for (i = 0; (i < aBufLength) && (i < (sizeof(hexbuf) - 3) / 3); i++)
    {
        // We can get away with sprintf because we know
        // `hexbuf` is large enough, based on our calculations
        // above.
        snprintf(&hexbuf[i * 3], sizeof(hexbuf) - i * 3, " %02X", static_cast<uint8_t>(aBuf[i]));
    }

    // Append a final closing bracket and newline character
    // so our log line looks nice.
    snprintf(&hexbuf[i * 3], sizeof(hexbuf) - i * 3, "]\n");

    // Write out the second part of our log message.
    // We skip the first byte since it has a space in it.
    otNcpStreamWrite(0, reinterpret_cast<uint8_t *>(hexbuf + 1), static_cast<int>(strlen(hexbuf) - 1));
    */
}

#if OPENTHREAD_ENABLE_NCP_SPINEL_ENCRYPTER

NcpUart::Spinel::BufferEncrypterReader::SpinelBufferEncrypterReader(Spinel::Buffer &aTxFrameBuffer)
    : mTxFrameBuffer(aTxFrameBuffer)
    , mDataBufferReadIndex(0)
    , mOutputDataLength(0)
{
}

bool NcpUart::Spinel::BufferEncrypterReader::IsEmpty(void) const
{
    return mTxFrameBuffer.IsEmpty() && !mOutputDataLength;
}

otError NcpUart::Spinel::BufferEncrypterReader::OutFrameBegin(void)
{
    otError status = OT_ERROR_FAILED;

    Reset();

    if ((status = mTxFrameBuffer.OutFrameBegin()) == OT_ERROR_NONE)
    {
        mOutputDataLength = mTxFrameBuffer.OutFrameGetLength();

        if (mOutputDataLength > 0)
        {
            OT_ASSERT(mOutputDataLength <= sizeof(mDataBuffer));
            mTxFrameBuffer.OutFrameRead(mOutputDataLength, mDataBuffer);

            if (!SpinelEncrypter::EncryptOutbound(mDataBuffer, sizeof(mDataBuffer), &mOutputDataLength))
            {
                mOutputDataLength = 0;
                status            = OT_ERROR_FAILED;
            }
        }
        else
        {
            status = OT_ERROR_FAILED;
        }
    }

    return status;
}

bool NcpUart::Spinel::BufferEncrypterReader::OutFrameHasEnded(void)
{
    return (mDataBufferReadIndex >= mOutputDataLength);
}

uint8_t NcpUart::Spinel::BufferEncrypterReader::OutFrameReadByte(void)
{
    return mDataBuffer[mDataBufferReadIndex++];
}

otError NcpUart::Spinel::BufferEncrypterReader::OutFrameRemove(void)
{
    return mTxFrameBuffer.OutFrameRemove();
}

void NcpUart::Spinel::BufferEncrypterReader::Reset(void)
{
    mOutputDataLength    = 0;
    mDataBufferReadIndex = 0;
}

#endif // OPENTHREAD_ENABLE_NCP_SPINEL_ENCRYPTER

} // namespace Ncp
} // namespace ot
