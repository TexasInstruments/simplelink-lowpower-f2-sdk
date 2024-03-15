/*
 * Copyright (c) 2019, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define HAVE_6LOWPAN_ROUTER
#define HAVE_WS_ROUTER
#define HAVE_RPL
//#define HAVE_IPV6_ND
//#define HAVE_6LOWPAN_ND
#define HAVE_MPL
#define HAVE_WS
#define HAVE_PAE_SUPP
#define HAVE_EAPOL_RELAY
//#define TI_WISUN_FAN_OPT
#define TI_WISUN_FAN_OPT_ICMPV6

//Additional Optimizations
#define NO_TCP
#define NO_IPV6_PMTUD
#define NO_IP_FRAGMENT_TX
#define NO_MLE


#define FEA_TRACE_SUPPORT
#define EXTRA_CONSISTENCY_CHECKS

