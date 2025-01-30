/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __RSS_KEY_DERIVATION_H__
#define __RSS_KEY_DERIVATION_H__

#include <stdint.h>
#include <stddef.h>
#include "rss_kmu_slot_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief                     Derive a VHUK seed.
 *
 * \param[out] vhuk_seed         The buffer to derive the seed into.
 * \param[in]  vhuk_seed_buf_len The size of the seed buffer.
 * \param[out] vhuk_seed_size    The size of the seed.
 *
 * \return                    0 on success, non-zero on error.
 */
int rss_derive_vhuk_seed(uint32_t *vhuk_seed, size_t vhuk_seed_buf_len,
                         size_t *vhuk_seed_size);

/**
 * \brief                     Derive the CPAK seed, and lock in a KMU slot.
 *
 * \param[in]  slot           The KMU slot to derive and lock the seed into.
 *
 * \return                    0 on success, non-zero on error.
 */
int rss_derive_cpak_seed(enum rss_kmu_slot_id_t slot);

/**
 * \brief                     Derive the DAK seed, and lock in a KMU slot.
 *
 * \param[in]  slot           The KMU slot to derive and lock the seed into.
 *
 * \return                    0 on success, non-zero on error.
 */
int rss_derive_dak_seed(enum rss_kmu_slot_id_t slot);

/**
 * \brief                     Derive the RoT CDI, and lock in a KMU slot.
 *
 * \param[in]  slot           The KMU slot to derive and lock the seed into.
 *
 * \return                    0 on success, non-zero on error.
 */
int rss_derive_rot_cdi(enum rss_kmu_slot_id_t slot);

/**
 * \brief                     Derive the VHUK, and lock in a KMU slot.
 *
 * \param[in]  vhuk_seeds     A buffer containing the seed values.
 * \param[in]  vhuk_seeds_len The size of the vhuk_seeds buffer. This must be
 *                            RSS_AMOUNT * 32 in size.
 * \param[in]  slot           The KMU slot to derive and lock the seed into.
 *
 * \return                    0 on success, non-zero on error.
 */
int rss_derive_vhuk(const uint8_t *vhuk_seeds, size_t vhuk_seeds_len,
                    enum rss_kmu_slot_id_t slot);

/**
 * \brief                     Derive the session key, and lock into two KMU
 *                            slots.
 *
 * \note                      Due to a limitation in KMU key export, keys used
 *                            for AEAD (such as this one) require two slots. The
 *                            slots used will be `slot` and `slot + 1`. It is
 *                            invalid for `slot` to be `KMU_USER_SLOT_MAX`
 *
 * \param[in]  ivs            A buffer containing the iv values.
 * \param[in]  ivs_len        The size of the ivs buffer. This must be
 *                            RSS_AMOUNT * 32 in size.
 * \param[in]  slot           The KMU slot to derive and lock the seed into.
 *
 * \return                    0 on success, non-zero on error.
 */
int rss_derive_session_key(const uint8_t *ivs, size_t ivs_len,
                           enum rss_kmu_slot_id_t slot);

/**
 * \brief                     Derive the CM provisioning key, and lock into two
 *                            KMU slots.
 *
 * \note                      Due to a limitation in KMU key export, keys used
 *                            for AEAD (such as this one) require two slots. The
 *                            slots used will be `slot` and `slot + 1`. It is
 *                            invalid for `slot` to be `KMU_USER_SLOT_MAX`
 *
 * \param[in]  slot           The KMU slot to derive and lock the seed into.
 *
 * \return                    0 on success, non-zero on error.
 */
int rss_derive_cm_provisioning_key(enum rss_kmu_slot_id_t slot);

/**
 * \brief                     Derive the DM provisioning key, and lock into two
 *                            KMU slots.
 *
 * \note                      Due to a limitation in KMU key export, keys used
 *                            for AEAD (such as this one) require two slots. The
 *                            slots used will be `slot` and `slot + 1`. It is
 *                            invalid for `slot` to be `KMU_USER_SLOT_MAX`
 *
 * \param[in]  slot           The KMU slot to derive and lock the seed into.
 *
 * \return                    0 on success, non-zero on error.
 */
int rss_derive_dm_provisioning_key(enum rss_kmu_slot_id_t slot);

#ifdef __cplusplus
}
#endif

#endif /* __RSS_KEY_DERIVATION_H__ */
