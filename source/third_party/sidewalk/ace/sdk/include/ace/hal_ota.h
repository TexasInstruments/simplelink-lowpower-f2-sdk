/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
/**
 * @file hal_ota.h
 * @brief A list of interfaces that define the platform-specific functionality
 * required by ACE OTA client in order to successfully synchronize (with the
 * configured OTA service), download, and install software updates.
 * @addtogroup ACE_HAL_OTA
 * @{
 */
#ifndef ACE_OTA_HAL_INTERFACE_H
#define ACE_OTA_HAL_INTERFACE_H

#include <ace/ace_status.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A helper function provided by the ACE OTA MW that allows the platform
 * to add download ranges (i.e. HTTP range requests).
 *
 * An example of where this may be useful is if the update contains two
 * system images (say one for A, and one for B) packaged together, and the
 * platform intends to only use one. The callback can then be invoked to
 * specify the appropriate offset + range to start from in order to restrict the
 * download to only the appropriate image.
 *
 * Another example might be a "resume download" feature, provided by the
 * platform. When aceOtaHal_on_download_prepare is invoked, the platform can
 * check to see if it has a partially completed version of this package, and can
 * add the appropriate download range to continue downloading where it left off.
 *
 * Typically, this will be called from aceOtaHal_on_download_data, after parsing
 * the relevant metadata from the package.
 *
 * @param[in] update Pointer to the current update; varies with OTA service. If
 * an Amazon OTA service is used, this will be a pointer to an
 * ota_available_update_t.
 * @param[in] start A zero-based offset for the range.
 * @param[in] size The number of bytes to download for this range; pass 0 to
 * signal the end of the stream.
 * @return Returns zero on success, and a negative value otherwise. Failures
 * shall result in the current OTA sequence being aborted.
 */
typedef ace_status_t (*add_download_range)(const void* update, size_t start,
                                           size_t size);

/**
 * @brief Invoked by the ACE OTA client when it has successfully completed a
 * sync with the configured service.
 *
 * This can be used by the platform to determine the validity of the list of
 * available updates.
 *
 * @param[in] updates The list of available updates provided by the configured
 * OTA service; this will be a meta-list that contains a size. If an Amazon OTA
 * service is used, it is a pointer to an ota_updates_t.
 * @return Returns zero on success, and a negative value otherwise. Failures
 * shall result in the current OTA sequence being aborted.
 */
ace_status_t aceOtaHal_on_sync_complete(const void* updates);

/**
 * @brief Invoked by the ACE OTA client prior to after syncing, but prior
 * to beginning the download.
 *
 * This can be used by the platform to prepare for the download. For example,
 * it might be used to prepare the file system.
 *
 * @param[in] update Pointer to the current update; varies with OTA service. If
 * an Amazon OTA service is used, this will be a pointer to an
 * ota_available_update_t.
 * @param cb A callback provided by the ACE OTA client used for adding download
 * ranges to the session.
 * @return Returns zero on success, and a negative value otherwise. Failures
 * shall result in the current OTA sequence being aborted.
 *
 * @note The contents of the update data structure (as pointed to by the update
 * pointer) will remain constant throughout the entire download sequence. For
 * example, the same update structure will be provided during on_download_data.
 *
 * @note This will be invoked prior to any download-related network actions
 * (establishing a TLS connection, for example).
 */
ace_status_t aceOtaHal_on_download_prepare(const void* update,
                                           add_download_range cb);

/**
 * @brief Invoked by the ACE OTA client for each HTTP header received during
 * the download.
 *
 * This can be used by the platform to extract any relevant metadata.
 *
 * @param[in] update Pointer to the current update; varies with OTA service. If
 * an Amazon OTA service is used, this will be a pointer to an
 * ota_available_update_t.
 * @param[in] header Pointer to the current HTTP header being processed
 * @param[in] length The length of the string pointed to by header.
 * @return Returns zero on success, and a negative value otherwise. Failures
 * shall result in the current OTA sequence being aborted.
 */
ace_status_t aceOtaHal_on_download_http_header(const void* update,
                                               const char* header,
                                               size_t length);
/**
 * @brief Invoked by the ACE OTA client for each chunk of data received
 * during the download.
 *
 * The client will always initially start downloading from an offset of zero,
 * and continue until the end of package. If there is a need to download a
 * specific range, after parsing the package meta-data for example (typically
 * located at the beginning of a package), new download ranges can be appended
 * by invoking the add_download_range callback provided during
 * aceOtaHal_on_download_prepare.
 *
 * When ACE_STATUS_OK is returned, the client will continue the current
 * download, and thus continue to call aceOtaHal_on_download_data, until the end
 * of current download range.
 *
 * The platform can cancel the current download range downloading by returning
 * ACE_STATUS_CANCELED. The client will then stop the current download range,
 * and start downloading the the next available range. When there are no further
 * download ranges available, the client will stop downloading and invoke
 * aceOtaHal_on_download_complete.
 *
 * @param[in] update Pointer to the current update; varies with OTA service. If
 * an Amazon OTA service is used, this will be a pointer to an
 * ota_available_update_t.
 * @param[in] data Pointer to the current chunk of data received from the
 * stream.
 * @param[in] length The length of the data pointed to by the data pointer.
 * @param[in] offset The current offset (zero-based) of the data pointed to by
 * the data pointer, relative to the beginning of the download stream.
 * @return Returns zero on success, and a negative value otherwise.
 * ACE_STATUS_CANCELED can be returned to cancel the current download range;
 * other negative values will be treated as failures, and shall result in the
 * current OTA sequence being aborted.
 */
ace_status_t aceOtaHal_on_download_data(const void* update, const uint8_t* data,
                                        size_t length, size_t offset);

/**
 * @brief Invoked by the ACE OTA client when the current download has finished,
 * be it because the download has completed or the current sequence was aborted.
 *
 * The platform can use this to synchronize all of the previous writes if the
 * download has succeeded, or release all relevant resources if it failed.
 *
 * @param[in] update Pointer to the current update; varies with OTA service. If
 * an Amazon OTA service is used, this will be a pointer to an
 * ota_available_update_t.
 * @param[in] success The status of the current download; true if it succeeded,
 * false otherwise.
 *
 * @return Returns zero on success, and a negative value otherwise. Failures
 * shall result in the current OTA sequence being aborted. Note that if the
 * client invokes this function with success equal to false, the current
 * sequence will be aborted regardless.
 */
ace_status_t aceOtaHal_on_download_complete(const void* update, bool success);

/**
 * @brief Invoked by the ACE OTA client after the current download has
 * successfully completed.
 *
 * The platform should use this to validate the integrity of the downloaded
 * image. This can be something simple, such as using the MD5 provided by KOTA,
 * or something more thorough.
 *
 * @param[in] update Pointer to the current update; varies with OTA service. If
 * an Amazon OTA service is used, this will be a pointer to an
 * ota_available_update_t.
 * @return Returns zero on success, and a negative value otherwise. Failures
 * shall result in the current OTA sequence being aborted.
 */
ace_status_t aceOtaHal_validate(const void* update);

/**
 * @brief Invoked by the ACE OTA client after the downloaded image was
 * successfully validated by the platform.
 *
 * The platform can use this to perform any final steps required to "activate"
 * the new downloaded image, such as switching the active boot slot, or to
 * trigger a recovery update sequence.
 *
 * @param[in] update Pointer to the current update; varies with OTA service. If
 *     an Amazon OTA service is used, this will be a pointer to an
 * ota_available_update_t.
 *
 * @return ACE_STATUS_OK or ACE_STATUS_IN_PROGRESS on success, and a different
 *     negative value otherwise. Failures shall result in the current OTA
 *     sequence being aborted.
 * @retval ACE_STATUS_OK The update was installed immediately.
 * @retval ACE_STATUS_IN_PROGRESS The update was installed, but requires a
 *     reboot to take effect.
 */
ace_status_t aceOtaHal_install(const void* update);

#ifdef __cplusplus
}
#endif

#endif  // ACE_OTA_HAL_INTERFACE_H
/** @} */
