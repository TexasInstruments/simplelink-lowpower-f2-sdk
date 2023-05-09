/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _H_RNET_NETWORK_INIT_H_
#define _H_RNET_NETWORK_INIT_H_

#include "rnet_config.h"
#include "rnet_nw_mac_ifc.h"

/*
 * brief Function to initialize network layer
 *      internally this function configure the MAC layer setup and
 *      call the MAC initialization function.
 * @ret success if network layer is successfully initialized and errr otherwise
 */
rnet_error_t rnet_network_layer_init(rnet_config_settings_t *config_init_settings, uint8_t max_num_mod);

/*
 * brief Getter function to pass mac config struct to submodules
 * @ret pointer to mac_config
 */
rnet_mac_config_t* rnet_get_mac_config(void);

/*
 * brief Getter function to nw config
 * @ret pointer to nw_config
 */
rnet_network_config_t* rnet_get_halo_nw_config (void);

#endif /* _H_RNET_NETWORK_INIT_H_ */
