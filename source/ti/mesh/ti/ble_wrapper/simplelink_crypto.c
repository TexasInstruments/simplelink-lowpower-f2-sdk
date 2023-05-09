#include <string.h>
#include <zephyr/types.h>
#include <sys/byteorder.h>

#include <tinycrypt/constants.h>
#include <tinycrypt/aes.h>

#include <subsys/bluetooth/host/ecc.h>

#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

#include <ti/drivers/AESCCM.h>
#include <ti/drivers/aesccm/AESCCMCC26XX.h>

#include <ti/drivers/AESECB.h>
#include <ti/drivers/aesecb/AESECBCC26XX.h>

#include <ti/drivers/AESCTRDRBG.h>
#include <ti/drivers/aesctrdrbg/AESCTRDRBGXX.h>

#include <ti/drivers/ECDH.h>
#include <ti/drivers/ecdh/ECDHCC26X2.h>

#include <ti/drivers/TRNG.h>
#include <ti/drivers/trng/TRNGCC26XX.h>

#define ENC_KEY_LEN_MAX                32
#define ENC_KEY_LEN                    16
#define PRIVATE_KEY_LEN                32
#define PUBLIC_KEY_LEN                 64

/* Runtime data for AESECB */
static AESECB_Handle    ecbHandle;

/* Configuration data for AESECB */
static AESECBCC26XX_HWAttrs ecbHwAttrs = {
    .intPriority = ~0
};
static AESECBCC26XX_Object ecbObject = {0};
static AESECB_Config ecbConfig = {
    .object = &ecbObject,
    .hwAttrs = &ecbHwAttrs
};

/* Runtime data for AESCTRDRBG (pseudo-random counter) */
static AESCTRDRBG_Handle drbgHandle;

/* Configuration data for AESCTRDRBG */
// must have two instances of DRBG, 
// BLE LL will use index 0, mesh crypto will use index 1 here
static AESCTRDRBGXX_HWAttrs drbgHwAttrs = {1}; 
static AESCTRDRBGXX_Object drbgObject = {0};
static AESCTRDRBG_Config drbgConfig = {
    .object = &drbgObject,
    .hwAttrs = &drbgHwAttrs
};

/* Runtime data for ECDH (key generation) */
static ECDH_Handle ecdhHandle;

/* Configuration data for ECDH */
static ECDHCC26X2_HWAttrs ecdhHwAttrs = {
    .intPriority = ~0
};

static ECDHCC26X2_Object ecdhObject = {0};
static ECDH_Config ecdhConfig = {
    .object = &ecdhObject,
    .hwAttrs = &ecdhHwAttrs
};

/* Runtime data for TRNG */
static TRNG_Handle trngHandle;

/* Configuration data for TRNG */
static TRNGCC26XX_HWAttrs trngHwAttrs = {
    .intPriority = ~0
};
static TRNGCC26XX_Object trngObject = {0};
static TRNG_Config trngConfig = {
    .object = &trngObject,
    .hwAttrs = &trngHwAttrs
};

static CryptoKey cryptoKey;

static struct bt_pub_key_cb *callbackListHead = NULL;
static struct bt_pub_key_cb *callbackListTail = NULL;

static bt_dh_key_cb_t dh_key_gen_cb;

static CryptoKey meshPrivateKey;
static CryptoKey meshPublicKey;
static CryptoKey meshSharedSecretKey;
static ECDH_OperationComputeSharedSecret ecdhSharedSecretOperation;
static ECDH_OperationGeneratePublicKey   ecdhPublicKeyOperation;

static uint8_t meshPrivateKeyMaterial[PRIVATE_KEY_LEN];
static uint8_t meshPublicKeyMaterial[1 + PUBLIC_KEY_LEN];
static uint8_t meshSharedSecretMaterial[1 + PUBLIC_KEY_LEN];

static uint8_t isInitialized = 0;

static int_fast16_t init_crypto();
static void executeAllCallbacks(uint8_t key[64]);
static void ecdhCallback (ECDH_Handle handle, int_fast16_t returnStatus,
                   ECDH_Operation operation, ECDH_OperationType operationType);


/* Generate a pseudorandom sequence of bytes with length len into buf */
int bt_rand(void *buf, size_t len)
{
    int_fast16_t result;
    int_fast16_t status;
    status = init_crypto();
    if (status != 0)
    {
        return status;
    }

    /* Make a first attempt to generate bytes */
    CryptoKeyPlaintext_initBlankKey(&cryptoKey, buf, len);
    result = AESCTRDRBG_getBytes(drbgHandle, &cryptoKey);

    if (result == AESCTRDRBG_STATUS_RESEED_REQUIRED)
    {
        /* Use the TRNG to generate a seed into a buffer */
        uint8_t reseedBuffer[AESCTRDRBG_SEED_LENGTH_AES_128];
        CryptoKeyPlaintext_initBlankKey(
            &cryptoKey, reseedBuffer, AESCTRDRBG_SEED_LENGTH_AES_128
        );
        result = TRNG_generateEntropy(trngHandle, &cryptoKey);
        if (result == TRNG_STATUS_SUCCESS)
        {
            /* Reseed the counter */
            result = AESCTRDRBG_reseed(drbgHandle, reseedBuffer, NULL, 0);
            if (result == TRNG_STATUS_SUCCESS)
            {
                /* Make a second attempt to generate bytes */
                CryptoKeyPlaintext_initBlankKey(&cryptoKey, buf, len);
                result = AESCTRDRBG_getBytes(drbgHandle, &cryptoKey);
                if (result == TRNG_STATUS_SUCCESS)
                {
                    return 0;
                }
            }
        }
    }

    return result;
}

/* Encrypt a single AES block without byte reversal */
int bt_encrypt_be(const uint8_t key[16],
                  const uint8_t plaintext[16],
                  uint8_t enc_data[16])
{
    AESECB_Operation operation;
    int_fast16_t status;

    status = init_crypto();
    if (status != 0)
    {
        return status;
    }
    /* Use CryptoKeyPlaintext_initKey for received/already exist key */
    CryptoKeyPlaintext_initKey(&cryptoKey, (uint8_t *)key, 16);

    AESECB_Operation_init(&operation);
    operation.inputLength = TC_AES_BLOCK_SIZE;
    operation.output = enc_data;
    operation.input = (uint8_t *)plaintext;
    operation.key = &cryptoKey;

    status = AESECB_oneStepEncrypt(ecbHandle, &operation);

    return status;
}

/* Encrypt a single AES block with byte reversal on mey, plaintext, and resulting data
 * These byte swaps are exactly what they do in the Zephyr software */
int bt_encrypt_le(const uint8_t key[16],
                  const uint8_t plaintext[16],
                  uint8_t enc_data[16])
{
    AESECB_Operation operation;
    int_fast16_t status;

    status = init_crypto();
    if (status != 0)
    {
        return status;
    }

    sys_mem_swap((void *)key, 16);
    sys_mem_swap((void *)plaintext, 16);
    
    /* Use CryptoKeyPlaintext_initKey for received/already exist key */
    CryptoKeyPlaintext_initKey(&cryptoKey, (uint8_t*) key, 16);

    AESECB_Operation_init(&operation);
    operation.inputLength = TC_AES_BLOCK_SIZE;
    operation.output = enc_data;
    operation.input = (uint8_t*) plaintext;
    operation.key = &cryptoKey;

    status = AESECB_oneStepEncrypt(ecbHandle, &operation);

    sys_mem_swap(enc_data, 16);

    return status;
}

/* Emulate the TinyCrypt AES functions - this means we can use TinyCrypt's
 * CMAC routines but with harware acceleration */
int tc_aes128_set_encrypt_key(TCAesKeySched_t s, const uint8_t *k)
{
    int_fast16_t status;
    status = init_crypto();
    if (status != 0)
    {
        return status;
    }

    /* Instead of mimicing the KeySched used by Tinycrypt we just use their
     * structure as a scratchpad for the key itself. This is guaranteed
     * to be large enough and makes the encrypt functions much easier */
    memcpy(s, k, ENC_KEY_LEN);
    return TC_CRYPTO_SUCCESS;
}

/* Emulate the TinyCrypt AES functions - this means we can use TinyCrypt's
 * CMAC routines but with harware acceleration */
int tc_aes_encrypt(uint8_t *out, const uint8_t *in, const TCAesKeySched_t s)
{
    AESECB_Operation operation;
    int_fast16_t status;
    status = init_crypto();
    if (status != 0)
    {
        return status;
    }

    /* Interpret the KeySched as a key, see note in set_encrypt_key */
    /* Use CryptoKeyPlaintext_initKey for received/already exist key */
    CryptoKeyPlaintext_initKey(&cryptoKey, (uint8_t*) s, 16);

    AESECB_Operation_init(&operation);
    operation.inputLength = TC_AES_BLOCK_SIZE;
    operation.output = out;
    operation.input = (uint8_t*) in;
    operation.key = &cryptoKey;

    status = AESECB_oneStepEncrypt(ecbHandle, &operation);
    if (status == AESECB_STATUS_SUCCESS)
    {
        return TC_CRYPTO_SUCCESS;
    }
    else
    {
        return TC_CRYPTO_FAIL;
    }
}

/* Generate a new public key for this Mesh instance
 * All previous callers are notified in addition to the current caller. */
int bt_pub_key_gen(struct bt_pub_key_cb *cb)
{
    int_fast16_t status;
    status = init_crypto();
    if (status != 0)
    {
        return status;
    }

    if (callbackListHead == NULL)
    {
        callbackListHead = cb;
        callbackListTail = cb;
        cb->_next = NULL;
    }
    else
    {
        callbackListTail->_next = cb;
        callbackListTail = cb;
        cb->_next = NULL;
    }

    CryptoKeyPlaintext_initBlankKey(
        &meshPrivateKey, meshPrivateKeyMaterial, PRIVATE_KEY_LEN
    );
    CryptoKeyPlaintext_initBlankKey(
        &meshPublicKey, meshPublicKeyMaterial, PUBLIC_KEY_LEN + 1
    );

    status = TRNG_generateEntropy(trngHandle, &meshPrivateKey);
    if (status != TRNG_STATUS_SUCCESS) {
        return status;
    }

    ECDH_OperationGeneratePublicKey_init(&ecdhPublicKeyOperation);
    ecdhPublicKeyOperation.myPrivateKey = &meshPrivateKey;
    ecdhPublicKeyOperation.myPublicKey = &meshPublicKey;
    ecdhPublicKeyOperation.curve = &ECCParams_NISTP256;

    status = ECDH_generatePublicKey(ecdhHandle, &ecdhPublicKeyOperation);

    ECDH_Operation operation = {.generatePublicKey = &ecdhPublicKeyOperation};

    ecdhCallback(ecdhHandle, status, operation, ECDH_OPERATION_TYPE_GENERATE_PUBLIC_KEY);

    return status;

}

/* Get a pointer to the current public key */
const uint8_t *bt_pub_key_get(void)
{
    /* This means we have never executed pub_key_gen */
    if (callbackListHead == NULL) {
        return NULL;
    } else {
        /* Increment key material by 1 to strip the octet formatting byte */

        static uint8_t le_pub_key[PUBLIC_KEY_LEN];

        /* Change to little endian as the profile expects */
        sys_memcpy_swap(le_pub_key, &meshPublicKeyMaterial[1], PUBLIC_KEY_LEN/2);
        sys_memcpy_swap(&le_pub_key[PUBLIC_KEY_LEN/2], &meshPublicKeyMaterial[1+PUBLIC_KEY_LEN/2], PUBLIC_KEY_LEN/2);

        return le_pub_key;
    }
}

/* Generate a shared secret from a remote public key */
int bt_dh_key_gen(const uint8_t remote_pk[64], bt_dh_key_cb_t cb)
{
    uint8_t remote_pk_octet[PUBLIC_KEY_LEN + 1];
    int_fast16_t status;
    status = init_crypto();
    if (status != 0)
    {
        return status;
    }

    remote_pk_octet[0] = 0x04;

    /* Remote key is sent in little endian, switch back to big endian*/
    sys_memcpy_swap(&(remote_pk_octet[1]), remote_pk, (PUBLIC_KEY_LEN/2));
    sys_memcpy_swap(&(remote_pk_octet[1+(PUBLIC_KEY_LEN/2)]),
                    &(remote_pk[(PUBLIC_KEY_LEN/2)]), (PUBLIC_KEY_LEN/2));

    /* Use CryptoKeyPlaintext_initKey for received/already exist key */
    CryptoKeyPlaintext_initKey(
        &meshPrivateKey, meshPrivateKeyMaterial, PRIVATE_KEY_LEN
    );

    CryptoKeyPlaintext_initKey(
        &cryptoKey, remote_pk_octet, PUBLIC_KEY_LEN + 1
    );

    CryptoKeyPlaintext_initBlankKey(
        &meshSharedSecretKey, meshSharedSecretMaterial, PUBLIC_KEY_LEN + 1
    );

    ECDH_OperationComputeSharedSecret_init(&ecdhSharedSecretOperation);
    ecdhSharedSecretOperation.sharedSecret = &meshSharedSecretKey;
    ecdhSharedSecretOperation.myPrivateKey = &meshPrivateKey;
    ecdhSharedSecretOperation.theirPublicKey = &cryptoKey;
    ecdhSharedSecretOperation.curve = &ECCParams_NISTP256;

    dh_key_gen_cb = cb;

    status = ECDH_computeSharedSecret(ecdhHandle, &ecdhSharedSecretOperation);

    ECDH_Operation operation = {.computeSharedSecret = &ecdhSharedSecretOperation};

    ecdhCallback(ecdhHandle, status, operation, ECDH_OPERATION_TYPE_COMPUTE_SHARED_SECRET);

    return status;
}

/* STATIC LOCAL FUNCTIONS */

static int_fast16_t init_crypto()
{
    if (!isInitialized)
    {
        TRNG_Params trngParams;
        ECDH_Params ecdhParams;
        AESECB_Params ecbParams;
        AESCTRDRBG_Params drbgParams;

        uint8_t seedBuffer[AESCTRDRBG_SEED_LENGTH_AES_128];

        AESECB_init();
        AESECB_Params_init(&ecbParams);
        ecbParams.returnBehavior = AESECB_RETURN_BEHAVIOR_POLLING;
        ecbHandle = AESECB_construct(&ecbConfig, &ecbParams);
        if (ecbHandle == NULL) {
            return -10;
        }

        ECDH_init();
        ECDH_Params_init(&ecdhParams);
        ecdhParams.returnBehavior = ECDH_RETURN_BEHAVIOR_POLLING;
        ecdhParams.callbackFxn = ecdhCallback;
        ecdhHandle = ECDH_construct(&ecdhConfig, &ecdhParams);
        if (ecdhHandle == NULL) {
            return -11;
        }

        TRNG_init();
        TRNG_Params_init(&trngParams);
        trngParams.returnBehavior = TRNG_RETURN_BEHAVIOR_BLOCKING;

        trngHandle = TRNG_construct(&trngConfig, &trngParams);
        if (trngHandle == NULL) {
            return -12;
        }

        /* Use the TRNG to generate a seed */
        CryptoKeyPlaintext_initBlankKey(
            &cryptoKey, seedBuffer, AESCTRDRBG_SEED_LENGTH_AES_128
        );
        if(TRNG_STATUS_SUCCESS != TRNG_generateEntropy(trngHandle, &cryptoKey))
        {
            return -13;
        }

        AESCTRDRBG_init();
        AESCTRDRBG_Params_init(&drbgParams);
        drbgParams.keyLength = AESCTRDRBG_AES_KEY_LENGTH_128;
        drbgParams.reseedInterval = 10000;
        drbgParams.seed = seedBuffer;

        drbgHandle = AESCTRDRBG_construct(&drbgConfig, &drbgParams);
        if (drbgHandle == NULL) {
            return -14;
        }

        isInitialized = 1;
    }
    return 0;
}

static void executeAllCallbacks(uint8_t key[64])
{
    struct bt_pub_key_cb* cb = callbackListHead;
    while (cb != NULL)
    {
        cb->func(key);
        cb = cb->_next;
    }
}

static void ecdhCallback (ECDH_Handle handle, int_fast16_t returnStatus,
                   ECDH_Operation operation, ECDH_OperationType operationType)
{
    if (operationType == ECDH_OPERATION_TYPE_GENERATE_PUBLIC_KEY)
    {
        if (returnStatus == ECDH_STATUS_SUCCESS)
        {
            static uint8_t pubkey[64];
            /* Increment key material by 1 to strip the octet formatting byte */
            sys_memcpy_swap((void *)pubkey ,&meshPublicKeyMaterial[1], 64);
            executeAllCallbacks(pubkey);
        }
        else
        {
            executeAllCallbacks(NULL);
        }
    }
    else /* ECDH_OPERATION_TYPE_COMPUTE_SHARED_SECRET */
    {
        if (dh_key_gen_cb != NULL)
        {
            if (returnStatus == ECDH_STATUS_SUCCESS)
            {
                static uint8_t dhkey[32];
                /* Increment key material by 1 to strip the octet formatting byte */
                sys_memcpy_swap((void *)dhkey ,&meshSharedSecretMaterial[1], 32);
                dh_key_gen_cb(dhkey);
            }
            else
            {
                dh_key_gen_cb(NULL);
            }
            dh_key_gen_cb = NULL;
        }
    }
}
