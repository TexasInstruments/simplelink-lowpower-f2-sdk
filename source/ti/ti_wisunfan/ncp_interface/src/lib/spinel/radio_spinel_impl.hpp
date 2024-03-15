///*
// *  Copyright (c) 2020, The OpenThread Authors.
// *  All rights reserved.
// *
// *  Redistribution and use in source and binary forms, with or without
// *  modification, are permitted provided that the following conditions are met:
// *  1. Redistributions of source code must retain the above copyright
// *     notice, this list of conditions and the following disclaimer.
// *  2. Redistributions in binary form must reproduce the above copyright
// *     notice, this list of conditions and the following disclaimer in the
// *     documentation and/or other materials provided with the distribution.
// *  3. Neither the name of the copyright holder nor the
// *     names of its contributors may be used to endorse or promote products
// *     derived from this software without specific prior written permission.
// *
// *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// *  POSSIBILITY OF SUCH DAMAGE.
// */
//
///**
// * @file
// *   This file implements the spinel based radio transceiver.
// */
//
//#include <assert.h>
//#include <errno.h>
//#include <stdarg.h>
//#include <stdlib.h>
//
//#include <openthread/dataset.h>
//#include <openthread/platform/diag.h>
//#include <openthread/platform/settings.h>
//#include <openthread/platform/time.h>
//
//#include "common/code_utils.hpp"
//#include "common/encoding.hpp"
//#include "common/logging.hpp"
//#include "common/new.hpp"
//#include "common/settings.hpp"
//#include "lib/platform/exit_code.h"
//#include "lib/spinel/spinel_decoder.hpp"
//#include "meshcop/dataset.hpp"
//#include "meshcop/meshcop_tlvs.hpp"
//
//#ifndef MS_PER_S
//#define MS_PER_S 1000
//#endif
//#ifndef US_PER_MS
//#define US_PER_MS 1000
//#endif
//#ifndef US_PER_S
//#define US_PER_S (MS_PER_S * US_PER_MS)
//#endif
//#ifndef NS_PER_US
//#define NS_PER_US 1000
//#endif
//
//#ifndef TX_WAIT_US
//#define TX_WAIT_US (5 * US_PER_S)
//#endif
//
//using ot::Spinel::Decoder;
//
//namespace ot {
//namespace Spinel {
//
//static otError SpinelStatusToOtError(spinel_status_t aError)
//{
//    otError ret;
//
//    switch (aError)
//    {
//    case SPINEL_STATUS_OK:
//        ret = OT_ERROR_NONE;
//        break;
//
//    case SPINEL_STATUS_FAILURE:
//        ret = OT_ERROR_FAILED;
//        break;
//
//    case SPINEL_STATUS_DROPPED:
//        ret = OT_ERROR_DROP;
//        break;
//
//    case SPINEL_STATUS_NOMEM:
//        ret = OT_ERROR_NO_BUFS;
//        break;
//
//    case SPINEL_STATUS_BUSY:
//        ret = OT_ERROR_BUSY;
//        break;
//
//    case SPINEL_STATUS_PARSE_ERROR:
//        ret = OT_ERROR_PARSE;
//        break;
//
//    case SPINEL_STATUS_INVALID_ARGUMENT:
//        ret = OT_ERROR_INVALID_ARGS;
//        break;
//
//    case SPINEL_STATUS_UNIMPLEMENTED:
//        ret = OT_ERROR_NOT_IMPLEMENTED;
//        break;
//
//    case SPINEL_STATUS_INVALID_STATE:
//        ret = OT_ERROR_INVALID_STATE;
//        break;
//
//    case SPINEL_STATUS_NO_ACK:
//        ret = OT_ERROR_NO_ACK;
//        break;
//
//    case SPINEL_STATUS_CCA_FAILURE:
//        ret = OT_ERROR_CHANNEL_ACCESS_FAILURE;
//        break;
//
//    case SPINEL_STATUS_ALREADY:
//        ret = OT_ERROR_ALREADY;
//        break;
//
//    case SPINEL_STATUS_PROP_NOT_FOUND:
//    case SPINEL_STATUS_ITEM_NOT_FOUND:
//        ret = OT_ERROR_NOT_FOUND;
//        break;
//
//    default:
//        if (aError >= SPINEL_STATUS_STACK_NATIVE__BEGIN && aError <= SPINEL_STATUS_STACK_NATIVE__END)
//        {
//            ret = static_cast<otError>(aError - SPINEL_STATUS_STACK_NATIVE__BEGIN);
//        }
//        else
//        {
//            ret = OT_ERROR_FAILED;
//        }
//        break;
//    }
//
//    return ret;
//}
//
//static void LogIfFail(const char *aText, otError aError)
//{
//    OT_UNUSED_VARIABLE(aText);
//    OT_UNUSED_VARIABLE(aError);
//
//    if (aError != OT_ERROR_NONE)
//    {
//        otLogWarnPlat("%s: %s", aText, otThreadErrorToString(aError));
//    }
//}
//
//} // namespace Spinel
//} // namespace ot
