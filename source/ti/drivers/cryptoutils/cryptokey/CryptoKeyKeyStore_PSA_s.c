/*
 * Copyright (c) 2022 Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <stdbool.h>

#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_helpers.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_s.h>

#include <third_party/tfm/interface/include/tfm_api.h>
#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/service.h>
#include <third_party/tfm/secure_fw/spm/include/tfm_memory_utils.h>

#include <third_party/tfm/platform/ext/target/ti/cc26x4/cmse.h> /* TI CMSE helper functions */
#include "ti_drivers_config.h"                                  /* Sysconfig generated header */

/*
 *  ======== KeyStore_s_copyKeyAttributesFromClient ========
 */
psa_status_t KeyStore_s_copyKeyAttributesFromClient(struct psa_client_key_attributes_s *clientKeyAttr,
                                                    int32_t clientId,
                                                    psa_key_attributes_t *keyAttributes)
{
    if (clientKeyAttr == NULL || keyAttributes == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    *keyAttributes = psa_key_attributes_init();

    /* Copy core key attributes from the client core key attributes */
    keyAttributes->core.type         = clientKeyAttr->type;
    keyAttributes->core.lifetime     = clientKeyAttr->lifetime;
    keyAttributes->core.policy.usage = clientKeyAttr->usage;
    keyAttributes->core.policy.alg   = clientKeyAttr->alg;
    keyAttributes->core.bits         = clientKeyAttr->bits;

    /* Use the client key id as the key_id and its partition id as the owner */
    keyAttributes->core.id.key_id = clientKeyAttr->id;
    keyAttributes->core.id.owner  = clientId;

    return PSA_SUCCESS;
}

/**
 * \brief Converts key attributes to client key attributes.
 *        Follows tfm_crypto_key_attributes_to_client()
 *
 * \param[in]  keyAttributes   Key attributes, no address verification necessary as this is always in secure side
 * \param[out] clientKeyAttr  Client key attributes, address location must be verified to be in non-secure memory by
 *                            calling functions
 *
 * \return Return values as described in \ref psa_status_t
 */
/*
 *  ======== KeyStore_s_copyKeyAttributesToClient ========
 */
static psa_status_t KeyStore_s_copyKeyAttributesToClient(const psa_key_attributes_t *keyAttributes,
                                                         struct psa_client_key_attributes_s *clientKeyAttr)
{
    if (clientKeyAttr == NULL || keyAttributes == NULL)
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    struct psa_client_key_attributes_s v = {0, 0, 0, 0, 0, 0};
    *clientKeyAttr                       = v;

    /* Copy core key attributes from the client core key attributes */
    clientKeyAttr->type     = keyAttributes->core.type;
    clientKeyAttr->lifetime = keyAttributes->core.lifetime;
    clientKeyAttr->usage    = keyAttributes->core.policy.usage;
    clientKeyAttr->alg      = keyAttributes->core.policy.alg;
    clientKeyAttr->bits     = keyAttributes->core.bits;

    /* Return the key_id as the client key id, do not return the owner */
    clientKeyAttr->id = keyAttributes->core.id.key_id;

    return PSA_SUCCESS;
}

/**
 * \brief Copies key ID from secure side to client key ID
 *
 * \param[in]  keyID         Key ID, no address verification necessary as this is always in secure side
 * \param[out] clientKeyID  Client key ID, address location must be verified to be in non-secure memory by
 *                          calling functions
 *
 */
/*
 *  ======== KeyStore_s_copyKeyIDtoClient ========
 */
static void KeyStore_s_copyKeyIDtoClient(KeyStore_PSA_KeyFileId *keyID, uint32_t *clientKeyID)
{
    /* Copy the keyID output from the KeyStore driver to client keyID */
    *clientKeyID = keyID->key_id;
}

/*
 *  ======== KeyStore_s_copyKeyIDFromClient ========
 */
void KeyStore_s_copyKeyIDFromClient(KeyStore_PSA_KeyFileId *keyID, int32_t clientId, uint32_t *clientKeyID)
{
    /* Copy keyID from client to KeyStore driver and set the owner to the caller's ID */
    keyID->key_id = *clientKeyID;
    keyID->owner  = clientId;
}

/*
 *  ======== KeyStore_s_getKeyAttributes ========
 */
psa_status_t KeyStore_s_getKeyAttributes(psa_msg_t *msg)
{
    KeyStore_s_GetKeyAttributesMsg getKeyAttributesMsg;
    int_fast16_t ret                         = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
    KeyStore_PSA_KeyAttributes keyAttributes = KEYSTORE_PSA_KEY_ATTRIBUTES_INIT;
    KeyStore_PSA_KeyFileId keyID;

    if ((msg->in_size[0] != sizeof(getKeyAttributesMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &getKeyAttributesMsg, sizeof(getKeyAttributesMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        if (cmse_has_unpriv_nonsecure_rw_access(getKeyAttributesMsg.attributes,
                                                sizeof(struct psa_client_key_attributes_s)) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }
    }

    KeyStore_s_copyKeyIDFromClient(&keyID, msg->client_id, &getKeyAttributesMsg.key);

    ret = KeyStore_PSA_getKeyAttributes(keyID, &keyAttributes);

    if (ret == KEYSTORE_PSA_STATUS_SUCCESS)
    {
        ret = KeyStore_s_copyKeyAttributesToClient(&keyAttributes,
                                                   (struct psa_client_key_attributes_s *)
                                                       getKeyAttributesMsg.attributes);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== KeyStore_s_importKey ========
 */
psa_status_t KeyStore_s_importKey(psa_msg_t *msg)
{
    KeyStore_s_ImportKeyMsg importKeyMsg;
    int_fast16_t ret                         = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
    KeyStore_PSA_KeyAttributes keyAttributes = KEYSTORE_PSA_KEY_ATTRIBUTES_INIT;
    KeyStore_PSA_KeyFileId keyID;

    if ((msg->in_size[0] != sizeof(importKeyMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &importKeyMsg, sizeof(importKeyMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        /* Validate input address range */
        if ((cmse_has_unpriv_nonsecure_rw_access(importKeyMsg.key, sizeof(KeyStore_PSA_KeyFileId)) == NULL) ||
            (cmse_has_unpriv_nonsecure_read_access(importKeyMsg.attributes, sizeof(KeyStore_PSA_KeyAttributes)) ==
             NULL) ||
            (cmse_has_unpriv_nonsecure_read_access(importKeyMsg.data, importKeyMsg.dataLength) == NULL))
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }
    }

    /* Copy keyID from application for persistent keys */
    if (importKeyMsg.attributes->lifetime)
    {
        KeyStore_s_copyKeyIDFromClient(&keyID, msg->client_id, importKeyMsg.key);
    }

    ret = KeyStore_s_copyKeyAttributesFromClient((struct psa_client_key_attributes_s *)importKeyMsg.attributes,
                                                 msg->client_id,
                                                 &keyAttributes);

    if (ret == PSA_SUCCESS)
    {
        ret = KeyStore_PSA_importKey(&keyAttributes, importKeyMsg.data, importKeyMsg.dataLength, &keyID);

        KeyStore_s_copyKeyIDtoClient(&keyID, importKeyMsg.key);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== KeyStore_s_importCertificate ========
 */
psa_status_t KeyStore_s_importCertificate(psa_msg_t *msg)
{
    KeyStore_s_ImportCertificateMsg importCertificateMsg;
    int_fast16_t ret                         = KEYSTORE_PSA_STATUS_RESOURCE_UNAVAILABLE;
    KeyStore_PSA_KeyAttributes keyAttributes = KEYSTORE_PSA_KEY_ATTRIBUTES_INIT;
    KeyStore_PSA_KeyFileId keyID;

    if ((msg->in_size[0] != sizeof(importCertificateMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &importCertificateMsg, sizeof(importCertificateMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        /* Validate input address range */
        if ((cmse_has_unpriv_nonsecure_rw_access(importCertificateMsg.key, sizeof(KeyStore_PSA_KeyFileId)) == NULL) ||
            (cmse_has_unpriv_nonsecure_read_access(importCertificateMsg.attributes,
                                                   sizeof(KeyStore_PSA_KeyAttributes)) == NULL) ||
            (cmse_has_unpriv_nonsecure_read_access(importCertificateMsg.data, importCertificateMsg.dataLength) == NULL))
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }
    }

    /* Copy keyID from application for certificates */
    KeyStore_s_copyKeyIDFromClient(&keyID, msg->client_id, importCertificateMsg.key);

    ret = KeyStore_s_copyKeyAttributesFromClient((struct psa_client_key_attributes_s *)importCertificateMsg.attributes,
                                                 msg->client_id,
                                                 &keyAttributes);

    if (ret == PSA_SUCCESS)
    {
        ret = KeyStore_PSA_importCertificate(&keyAttributes,
                                             &keyID,
                                             importCertificateMsg.data,
                                             importCertificateMsg.dataLength);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== KeyStore_s_destroyCertificateKey ========
 */
psa_status_t KeyStore_s_destroyCertificateKey(psa_msg_t *msg, int32_t msgType)
{
    KeyStore_s_DestroyPurgeKeyCertificateMsg destroyMsg;
    int_fast16_t ret = PSA_ERROR_PROGRAMMER_ERROR;
    KeyStore_PSA_KeyFileId keyID;

    if ((msg->in_size[0] != sizeof(destroyMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &destroyMsg, sizeof(destroyMsg));

    KeyStore_s_copyKeyIDFromClient(&keyID, msg->client_id, &destroyMsg.key);

    if (msgType == KEYSTORE_PSA_S_MSG_TYPE_DESTROY_CERTIFICATE)
    {
        ret = KeyStore_PSA_destroyCertificate(keyID);
    }
    else if (msgType == KEYSTORE_PSA_S_MSG_TYPE_DESTROY_KEY)
    {
        ret = KeyStore_PSA_destroyKey(keyID);
    }
    else if (msgType == KEYSTORE_PSA_S_MSG_TYPE_PURGE_KEY)
    {
        ret = KeyStore_PSA_purgeKey(keyID);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== KeyStore_s_exportCertificateKey ========
 */
psa_status_t KeyStore_s_exportCertificateKey(psa_msg_t *msg, int32_t msgType)
{
    KeyStore_s_ExportMsg exportMsg;
    int_fast16_t ret = PSA_ERROR_PROGRAMMER_ERROR;
    KeyStore_PSA_KeyFileId keyID;

    if ((msg->in_size[0] != sizeof(exportMsg)) || (msg->out_size[0] != sizeof(ret)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &exportMsg, sizeof(exportMsg));

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        /* Validate input address range */
        if (cmse_has_unpriv_nonsecure_rw_access(exportMsg.data, exportMsg.dataSize) == NULL)
        {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }
    }

    KeyStore_s_copyKeyIDFromClient(&keyID, msg->client_id, &exportMsg.key);

    if (msgType == KEYSTORE_PSA_S_MSG_TYPE_EXPORT_CERTIFICATE)
    {
        ret = KeyStore_PSA_exportCertificate(keyID, exportMsg.data, exportMsg.dataSize, exportMsg.dataLength);
    }
    else if (msgType == KEYSTORE_PSA_S_MSG_TYPE_EXPORT_KEY)
    {
        ret = KeyStore_PSA_exportKey(keyID, exportMsg.data, exportMsg.dataSize, exportMsg.dataLength);
    }
    else if (msgType == KEYSTORE_PSA_S_MSG_TYPE_EXPORT_PUBLIC_KEY)
    {
        ret = KeyStore_PSA_exportPublicKey(keyID, exportMsg.data, exportMsg.dataSize, exportMsg.dataLength);
    }

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== KeyStore_s_resetKeyAttributes ========
 */
psa_status_t KeyStore_s_resetKeyAttributes(psa_msg_t *msg)
{
    KeyStore_s_ResetKeyAttributesMsg resetKeyAttributeMsg;
    KeyStore_PSA_KeyAttributes keyAttributes = KEYSTORE_PSA_KEY_ATTRIBUTES_INIT;

    if ((msg->in_size[0] != sizeof(resetKeyAttributeMsg)))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &resetKeyAttributeMsg, sizeof(resetKeyAttributeMsg));

    KeyStore_s_copyKeyAttributesFromClient((struct psa_client_key_attributes_s *)resetKeyAttributeMsg.attributes,
                                           msg->client_id,
                                           &keyAttributes);

    /*
     * Following TF-M's implementation of psa_reset_key_attributes(), KeyStore_PSA_resetKeyAttributes() cannot be
     * directly called from non-secure application. KeyStore S/NS interface makes a PSA call to reset the attributes
     * using the function in secure partition
     */
    KeyStore_PSA_resetKeyAttributes(&keyAttributes);

    KeyStore_s_copyKeyAttributesToClient(&keyAttributes, resetKeyAttributeMsg.attributes);

    return PSA_SUCCESS;
}

/*
 *  ======== KeyStore_s_getKey ========
 */
psa_status_t KeyStore_s_getKey(psa_msg_t *msg)
{
    KeyStore_s_GetKeyMsg getKeyMsg;
    int_fast16_t ret = PSA_ERROR_PROGRAMMER_ERROR;
    KeyStore_PSA_KeyFileId keyID;

    if ((msg->in_size[0] != sizeof(getKeyMsg)) || (msg->out_size[0] != sizeof(ret)) ||
        TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &getKeyMsg, sizeof(getKeyMsg));

    KeyStore_s_copyKeyIDFromClient(&keyID, msg->client_id, &getKeyMsg.key);

    ret = KeyStore_PSA_getKey(keyID,
                              getKeyMsg.data,
                              getKeyMsg.dataSize,
                              getKeyMsg.dataLength,
                              getKeyMsg.alg,
                              getKeyMsg.usage);

    psa_write(msg->handle, 0, &ret, sizeof(ret));

    return PSA_SUCCESS;
}

/*
 *  ======== KeyStore_s_handlePsaMsg ========
 */
psa_status_t KeyStore_s_handlePsaMsg(psa_msg_t *msg)
{
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    switch (msg->type)
    {
        case KEYSTORE_PSA_S_MSG_TYPE_GET_KEY:
            status = KeyStore_s_getKey(msg);
            break;
        /* Fall through for exporting */
        case KEYSTORE_PSA_S_MSG_TYPE_EXPORT_CERTIFICATE:
        case KEYSTORE_PSA_S_MSG_TYPE_EXPORT_PUBLIC_KEY:
        case KEYSTORE_PSA_S_MSG_TYPE_EXPORT_KEY:
            status = KeyStore_s_exportCertificateKey(msg, msg->type);
            break;
        /* Fall through for destroying */
        case KEYSTORE_PSA_S_MSG_TYPE_DESTROY_CERTIFICATE:
        case KEYSTORE_PSA_S_MSG_TYPE_DESTROY_KEY:
        case KEYSTORE_PSA_S_MSG_TYPE_PURGE_KEY:
            status = KeyStore_s_destroyCertificateKey(msg, msg->type);
            break;
        case KEYSTORE_PSA_S_MSG_TYPE_IMPORT_CERTIFICATE:
            status = KeyStore_s_importCertificate(msg);
            break;
        case KEYSTORE_PSA_S_MSG_TYPE_IMPORT_KEY:
            status = KeyStore_s_importKey(msg);
            break;
        case KEYSTORE_PSA_S_MSG_TYPE_GET_KEY_ATTRIBUTES:
            status = KeyStore_s_getKeyAttributes(msg);
            break;
        case KEYSTORE_PSA_S_MSG_TYPE_RESET_KEY_ATTRIBUTES:
            status = KeyStore_s_resetKeyAttributes(msg);
            break;
        default:
            /* Unkown PSA message type */
            status = PSA_ERROR_PROGRAMMER_ERROR;
            break;
    }

    return status;
}

/*
 *  ======== KeyStore_s_init ========
 */
void KeyStore_s_init(void)
{
    KeyStore_PSA_init();
}
