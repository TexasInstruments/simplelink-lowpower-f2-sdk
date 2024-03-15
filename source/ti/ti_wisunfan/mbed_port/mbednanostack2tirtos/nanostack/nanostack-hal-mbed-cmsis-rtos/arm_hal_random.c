/*
 * Copyright (c) 2015, 2018, Arm Limited and affiliates.
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
#include "ns_types.h"
#include "arm_hal_random.h"

#include "ti_drivers_config.h"
#include <ti/drivers/TRNG.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

static TRNG_Handle TRNG_handle = NULL;

void arm_random_module_init(void)
{
    TRNG_Params TRNGParams;

    /* Only initialize once */
    if (!TRNG_handle) {
        TRNG_init();
        TRNG_Params_init(&TRNGParams);
        TRNGParams.returnBehavior = TRNG_RETURN_BEHAVIOR_POLLING;
        TRNG_handle = TRNG_open(NANOSTACK_TRNG, &TRNGParams);
    }
}

uint32_t arm_random_seed_get(void)
{
    uint16_t result = 0;
    uint32_t randNum;
    CryptoKey entropyKey;

  CryptoKeyPlaintext_initBlankKey(&entropyKey, (uint8_t*) &randNum, sizeof(randNum));

  result = TRNG_generateEntropy(TRNG_handle, &entropyKey);

  if (result != TRNG_STATUS_SUCCESS) {
      // Handle error
  }

   return randNum;
}
