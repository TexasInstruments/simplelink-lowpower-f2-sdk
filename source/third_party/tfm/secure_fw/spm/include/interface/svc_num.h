/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __SVC_NUM_H__
#define __SVC_NUM_H__

/*
 * The SVC number field descriptions.
 *
 * Bit assignments:
 * +-------+----------+--------------+-----+-------+
 * |  Bit  |    7     |     6        |  5  |  4:0  |
 * --------+----------+--------------+-----+-------+
 * |       | Platform | Handler Mode | PSA | Index |
 * +-------+----------+--------------+-----+-------+
 *
 * Platform, bit [7]
 *   Whether it's a platform SVC that should be handled by platform codes.
 *   1 - The SVC should be left to platform to handle
 *   0 - The SVC should be handled by TF-M
 *
 * Handler Mode, bit [6]:
 *   Whether the SVC should be called from Handler mode.
 *   1 - The SVC can be only called from Handler mode
 *   0 - The SVC can be only called from Thread mode
 *
 * PSA, bit [5]:
 *   Whether it's PSA API or TF-M SPM SVC requests.
 *   Ignored when Bit [7] is 1.
 *   1 - PSA API requests
 *   0 - TF-M SPM SVC requests
 *
 * Index, bits [4:0]:
 *   The SVC number index
 */

#define TFM_SVC_NUM_PLATFORM_POS        7U
#define TFM_SVC_NUM_PLATFORM_MSK        (1U << TFM_SVC_NUM_PLATFORM_POS)

#define TFM_SVC_NUM_HANDLER_MODE_POS    6U
#define TFM_SVC_NUM_HANDLER_MODE_MSK    (1U << TFM_SVC_NUM_HANDLER_MODE_POS)

#define TFM_SVC_NUM_PSA_API_POS         5U
#define TFM_SVC_NUM_PSA_API_MSK         (1U << TFM_SVC_NUM_PSA_API_POS)

#define TFM_SVC_NUM_INDEX_MSK           (0x1FU)

/* SVC number constructors */

/* SPM SVC from Thread mode */
#define TFM_SVC_NUM_SPM_THREAD(index)       ((index) & TFM_SVC_NUM_INDEX_MSK)
/* SPM SVC from Handler mode */
#define TFM_SVC_NUM_SPM_HANDLER(index)      (TFM_SVC_NUM_HANDLER_MODE_MSK | \
                                             ((index) & TFM_SVC_NUM_INDEX_MSK))
/* PSA API SVC from Thread mode (from Handler mode is not supported) */
#define TFM_SVC_NUM_PSA_API_THREAD(index)   (TFM_SVC_NUM_PSA_API_MSK | \
                                             ((index) & TFM_SVC_NUM_INDEX_MSK))
/* Platform SVC from Thread mode */
#define TFM_SVC_NUM_PLATFORM_THREAD(index)  (TFM_SVC_NUM_PLATFORM_MSK | \
                                             ((index) & TFM_SVC_NUM_INDEX_MSK))
/* Platform SVC from Handler mode */
#define TFM_SVC_NUM_PLATFORM_HANDLER(index) (TFM_SVC_NUM_PLATFORM_MSK | \
                                             TFM_SVC_NUM_HANDLER_MODE_MSK |\
                                             ((index) & TFM_SVC_NUM_INDEX_MSK))

/* TF-M SPM and for Thread mode */
#define TFM_SVC_SPM_INIT                TFM_SVC_NUM_SPM_THREAD(0)
#define TFM_SVC_FLIH_FUNC_RETURN        TFM_SVC_NUM_SPM_THREAD(1)
#define TFM_SVC_OUTPUT_UNPRIV_STRING    TFM_SVC_NUM_SPM_THREAD(2)
#define TFM_SVC_GET_BOOT_DATA           TFM_SVC_NUM_SPM_THREAD(3)
#define TFM_SVC_THREAD_MODE_SPM_RETURN  TFM_SVC_NUM_SPM_THREAD(4)

/* TF-M SPM and for Handler mode */
#define TFM_SVC_PREPARE_DEPRIV_FLIH     TFM_SVC_NUM_SPM_HANDLER(0)

/* PSA API (from Thread mode, Handler mode is not supported */
/* PSA Client APIs */
#define TFM_SVC_PSA_FRAMEWORK_VERSION   TFM_SVC_NUM_PSA_API_THREAD(0)
#define TFM_SVC_PSA_VERSION             TFM_SVC_NUM_PSA_API_THREAD(1)
#define TFM_SVC_PSA_CALL                TFM_SVC_NUM_PSA_API_THREAD(2)
#define TFM_SVC_PSA_CONNECT             TFM_SVC_NUM_PSA_API_THREAD(3)
#define TFM_SVC_PSA_CLOSE               TFM_SVC_NUM_PSA_API_THREAD(4)
/* PSA Secure Partition APIs */
#define TFM_SVC_PSA_WAIT                TFM_SVC_NUM_PSA_API_THREAD(5)
#define TFM_SVC_PSA_GET                 TFM_SVC_NUM_PSA_API_THREAD(6)
#define TFM_SVC_PSA_SET_RHANDLE         TFM_SVC_NUM_PSA_API_THREAD(7)
#define TFM_SVC_PSA_READ                TFM_SVC_NUM_PSA_API_THREAD(8)
#define TFM_SVC_PSA_SKIP                TFM_SVC_NUM_PSA_API_THREAD(9)
#define TFM_SVC_PSA_WRITE               TFM_SVC_NUM_PSA_API_THREAD(10)
#define TFM_SVC_PSA_REPLY               TFM_SVC_NUM_PSA_API_THREAD(11)
#define TFM_SVC_PSA_NOTIFY              TFM_SVC_NUM_PSA_API_THREAD(12)
#define TFM_SVC_PSA_CLEAR               TFM_SVC_NUM_PSA_API_THREAD(13)
#define TFM_SVC_PSA_EOI                 TFM_SVC_NUM_PSA_API_THREAD(14)
#define TFM_SVC_PSA_PANIC               TFM_SVC_NUM_PSA_API_THREAD(15)
#define TFM_SVC_PSA_LIFECYCLE           TFM_SVC_NUM_PSA_API_THREAD(16)
#define TFM_SVC_PSA_IRQ_ENABLE          TFM_SVC_NUM_PSA_API_THREAD(17)
#define TFM_SVC_PSA_IRQ_DISABLE         TFM_SVC_NUM_PSA_API_THREAD(18)
#define TFM_SVC_PSA_RESET_SIGNAL        TFM_SVC_NUM_PSA_API_THREAD(19)
#define TFM_SVC_AGENT_PSA_CALL          TFM_SVC_NUM_PSA_API_THREAD(20)
#define TFM_SVC_AGENT_PSA_CONNECT       TFM_SVC_NUM_PSA_API_THREAD(21)

#define TFM_SVC_IS_PLATFORM(svc_num)        (!!((svc_num) & TFM_SVC_NUM_PLATFORM_MSK))
#define TFM_SVC_IS_HANDLER_MODE(svc_num)    (!!((svc_num) & TFM_SVC_NUM_HANDLER_MODE_MSK))
#define TFM_SVC_IS_PSA_API(svc_num)         (((svc_num) & (TFM_SVC_NUM_PLATFORM_MSK | \
                                                           TFM_SVC_NUM_PSA_API_MSK)) \
                                                                         == TFM_SVC_NUM_PSA_API_MSK)
#define TFM_SVC_IS_SPM(svc_num)             (((svc_num) & (TFM_SVC_NUM_PLATFORM_MSK | \
                                                           TFM_SVC_NUM_PSA_API_MSK)) == 0)

#endif /* __SVC_NUM_H__ */
