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
 * @file hal_wifi.h
 *
 * @brief HAL WiFi API to be implemented per platform to support
 * WiFi functionality.
 *
 * @addtogroup ACE_HAL_WIFI
 * @{
 */

#ifndef __ACE_WIFI_HAL_H__
#define __ACE_WIFI_HAL_H__

#include <stdint.h>
#include <ace/ace_config.h>
#include <ace/ace_status.h>

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @cond DEPRECATED
 * @deprecated Please use the new symbols.
 * @{
 */

typedef ace_status_t aceWifiHal_error_t;

/**< API call success */
#define aceWifiHal_ERROR_SUCCESS   ACE_STATUS_OK
/**< API call failure */
#define aceWifiHal_ERROR_FAILURE   ACE_STATUS_GENERAL_ERROR
/**< API call has invalid parameters */
#define aceWifiHal_ERROR_INVALID_PARAM   ACE_STATUS_BAD_PARAM
/**< API call not supported */
#define aceWifiHal_ERROR_NOT_SUPPORTED   ACE_STATUS_NOT_SUPPORTED
/**< API call failed to write to lower layer */
// This macro is not defined correctly, it is only used to align with
// HAL aceWifiHal_ERROR_WRITE, and it's really a type of bad param.
// Using ERROR_WRITE here is confusing.
// Use BAD_PARAM here instead of adding new status code to ace_status
#define aceWifiHal_ERROR_WRITE   ACE_STATUS_BAD_PARAM
/**< API call failed to read from lower layer */
// For all the cases, this error case was used in hal layer
// while doing getxxx api call, which can be treated with
// ACE_STATUS_GENERAL_ERROR. Using error read here is confusing
#define aceWifiHal_ERROR_READ   ACE_STATUS_GENERAL_ERROR
/**< Internal error indicating out of memory situation */
#define aceWifiHal_ERROR_NOMEM   ACE_STATUS_OUT_OF_MEMORY
/**< Internal error indicating violation of the agreed
     data exchange protocol */
#define aceWifiHal_ERROR_PROTOCOL    ACE_STATUS_PROTOCOL_ERROR
/**< Internal error indicating thread is not responding */
// Only one case using this error code, which is when using aipc sync
// failed, would change that part and completely deprecate this error
// code.
#define aceWifiHal_ERROR_HANG    ACE_STATUS_TIMEOUT
/**< Indicating sender will send more data */
#define aceWifiHal_ERROR_MOREDATA   ACE_STATUS_MORE_DATA
/**< Internal code indicating that further
     processing should be done */
// This status code is only used in wifi_eap.c indicating
// that fragment is needed or not. It's confusing and should not
// be used, modified the code so that IRNOGRE code is not needed.
#define aceWifiHal_ERROR_IGNORE    ACE_STATUS_OK
/**< API call failed for unknown reasons */
// This is the same as gernal eror
#define aceWifiHal_ERROR_UNKNOWN   ACE_STATUS_GENERAL_ERROR

/**
 * @}
 * @endcond
 */    // cond DEPRECATED






/**
 * @brief The maximum length of an SSID
 */
#define aceWifiHal_MAX_SSID_LEN        32

/**
 * @brief The length of a BSSID
 */
#define aceWifiHal_BSSID_LEN           6

/**
 * @brief The minimum length of a PSK
 */
#define aceWifiHal_KEY_MIN_LEN         8

/**
 * @brief The maximum length of a PSK
 */
#define aceWifiHal_KEY_MAX_LEN         64

/**
 * @brief The maximum length of a WEP key
 * Only 64- and 128-bit keys are supported.
 */
#define aceWifiHal_WEP_MAX_LEN         26

/**
 * @brief The minimum required buffer length for the country code
 */
#define aceWifiHal_COUNTRY_CODE_LEN    4

/**
 * @brief Wifi lower level supported feature mask.
 */
#define aceWifiHal_WPS_SUPPORTED           0x0001
#define aceWifiHal_ENTERPRISE_SUPPORTED    0x0002
#define aceWifiHal_P2P_SUPPORTED           0x0004
#define aceWifiHal_TDLS_SUPPORTED          0x0008


/**
 * @brief WiFi HAL disconnect reason
 */
typedef enum {
    aceWifiHal_DISCONNECT_ASSOC_FAILURE = 0,    /**< Cannot connect to AP */
    aceWifiHal_DISCONNECT_AUTH_FAILURE,         /**< Password, Key, etc. */
    aceWifiHal_DISCONNECT_IP_FAILURE,           /**< Cannot obtain IP address */
    aceWifiHal_DISCONNECT_LOCALLY_GENERATED,    /**< Locally generated device side disconnect */
    aceWifiHal_DISCONNECT_UNDEFINED             /**< Undefined reason */
} aceWifiHal_disconnectReason_t;

/**
 * @brief Fixed length MAC address string
 * MAC address string is formatted as XX:XX:XX:XX:XX:XX with null termination.
 */
typedef char aceWifiHal_macAddress_t[18];

/**
 * @brief WiFi WEP key (64- and 128-bit keys only)
 */
typedef struct {
    char wepKey[aceWifiHal_WEP_MAX_LEN];    /**< WEP key */
    uint8_t keyLength;                      /**< Length of the key */
} aceWifiHal_wepKey_t;

/**
 * @brief WiFi auth modes
 */
typedef enum {
    aceWifiHal_AUTH_MODE_OPEN = 0,  /**< Open network */
    aceWifiHal_AUTH_MODE_WEP,       /**< WEP (open, not shared) */
    aceWifiHal_AUTH_MODE_WPA_PSK,   /**< WPA (TKIP only) */
    aceWifiHal_AUTH_MODE_WPA2_PSK,  /**< WPA2 (TKIP or CCMP) */
    aceWifiHal_AUTH_MODE_EAP,       /**< WPA2 enterprise */
    aceWifiHal_AUTH_MODE_MAX        /**< Invalid auth mode */
} aceWifiHal_authMode_t;

/**
 * @brief WiFi configuration status codes
 */
typedef enum {
    aceWifiHal_CONFIG_STATUS_CURRENT = 0,   /**< WiFi network is current */
    aceWifiHal_CONFIG_STATUS_DISABLED,      /**< WiFi network is disabled */
    aceWifiHal_CONFIG_STATUS_ENABLED,       /**< WiFi network is eanbled */
    aceWifiHal_CONFIG_STATUS_UPDATED        /**< WiFi network is updated */
} aceWifiHal_configStatus_t;

#if defined(ACE_WIFI_IEEE8021X)
/**
 * @brief WiFi configuration IEEE8021X authentication types
 */
typedef enum {
    aceWifiHal_EAP_NONE = 0,    /**< No EAP Protocol */
    aceWifiHal_EAP_PEAP,        /**< Protected EAP Protocol */
    aceWifiHal_EAP_TLS,         /**< EAP-Transport Layer Protocol */
    aceWifiHal_EAP_TTLS,        /**< EAP-Tunneled Transport Layer Protocol */
} aceWifiHal_enterpriseEap_t;

/**
 * @brief EAP inner authentication methods
 */
typedef enum {
    aceWifiHal_PHASE2_NONE = 0, /**< No Protocol */
    aceWifiHal_PHASE2_MSCHAPV2, /**< Microsoft Challenge Authentication Protocol */
    aceWifiHal_PHASE2_PAP,      /**< Password Authentication Protocol */
} aceWifiHal_enterprisePhase2_t;

typedef struct {
    int32_t  len;    /**< length of encoded certificate */
    uint8_t* p_val; /**< certificate encoded as PEM/DER format */
} aceWifiHal_x509Cert_t;

typedef struct {
    int32_t  len;                   /**< Number of certificates */
    aceWifiHal_x509Cert_t* p_val;   /**< List of certficates. */
}aceWifiHal_x509CertList_t;

typedef struct {
    int32_t  len;        /**< size of user credential */
    uint8_t  *p_val;     /**< value of user credential */
} aceWifiHal_userCred_t;

typedef uint32_t aceWifiHal_crtPolicy_t;

typedef struct {
    aceWifiHal_enterpriseEap_t type;        /**< The Extensible Authentication Protocol method used  */
    aceWifiHal_enterprisePhase2_t phase2;   /**< The inner authentication method used */
    aceWifiHal_crtPolicy_t policy;          /**< Contains certificate validation rules */
    aceWifiHal_x509CertList_t ca_certs;     /**< List of CA certificates in PEM/DER format */
    aceWifiHal_x509Cert_t cert;             /**< Client certificate in PEM/DER format */
    aceWifiHal_x509Cert_t pkey;             /**< Client private RSA key in PEM/DER format. NOT encrypted */
    aceWifiHal_userCred_t anonymous;        /**< Unencrypted identity if eap supports tunnelled type (e.g., PEAP) */
    aceWifiHal_userCred_t identity;         /**< User identity */
} aceWifiHal_enterpriseConfig_t;
#endif

/**
 * @brief IP address type
 */
typedef enum {
    aceWifiHal_IP_TYPE_IPV4 = 2,   /**< IPV4, set to the value of AF_INET to be backward compatible */
    aceWifiHal_IP_TYPE_IPV6 = 10,  /**< IPV6, same value as AF_INET6 */
    aceWifiHal_IP_TYPE_UNSPEC = 0  /**< unknown, same vaue as AF_UNSPEC */
} aceWifiHal_ipType_t;

/**
 * @brief IP address data
 */
typedef struct {
    aceWifiHal_ipType_t type;    /**< e.g. IPV4, IPV6 */
    uint32_t ipAddress[4];       /**< IP address in binary form i.e. use inet_ntop/inet_pton for conversion */
} aceWifiHal_ipAddress_t;

/**
 * @brief IP address configuration
 */
typedef struct {
    uint8_t isStatic;                   /**< 1 if static IP config, else 0 */
    aceWifiHal_ipAddress_t ipAddress;   /**< IP address */
    aceWifiHal_ipAddress_t netMask;     /**< Network mask */
    aceWifiHal_ipAddress_t gateway;     /**< Gateway IP address */
    aceWifiHal_ipAddress_t dns1;        /**< First DNS server IP address */
    aceWifiHal_ipAddress_t dns2;        /**< Second DNS server IP address */
    aceWifiHal_ipAddress_t dns3;        /**< Third DNS server IP address */
    uint32_t leaseTime;                 /**< dhcp lease time in seconds. Optional, required when doing DHCP renew */
    uint32_t renewTime;                 /**< dhcp renew time in seconds. Optional, required when doing DHCP renew */
    uint32_t rebindTime;                /**< dhcp rebind time in seconds. Optional, required when doing DHCP renew */
} aceWifiHal_ipConfiguration_t;

/**
 * @brief Wifi configuration (STA mode)
 * Describes a configured network
 */
typedef struct {
    char ssid[aceWifiHal_MAX_SSID_LEN]; /**< The network's SSID. NOT a null terminated string, can be raw bytes e.g. for unicode */
    uint8_t ssidLength;                 /**< Length of the SSID. MUST be set instead of a null terminator */
    char psk[aceWifiHal_KEY_MAX_LEN];   /**< Pre-shared key for use with WPA-PSK, NOT null terminated */
    uint8_t pskLength;                  /**< Length of the psk. MUST be set instead of a null terminator */
    aceWifiHal_wepKey_t wepKeys[4];     /**< WEP keys, up to four keys, for WEP networks */
    uint8_t wepKeyIndex;                /**< Default WEP key index, ranging from 0 to 3 */
    aceWifiHal_authMode_t authMode;     /**< The authentication protocol supported by this config */
    uint8_t hiddenSsid;                 /**< Whether this network is hidden - 0 = false, 1 = true */
    aceWifiHal_configStatus_t status;   /**< Current status of this network configuration entry */

    uint8_t isTethered;                 /**< Whether this network is tethered (i.e. a phone hotspot) */
#if defined(ACE_WIFI_IEEE8021X)
    aceWifiHal_enterpriseConfig_t eap;  /**< 802.1X configuration */
#endif
#if defined(ACE_WIFI_STATIC_IP)
    aceWifiHal_ipConfiguration_t ipConfig; /**< Static IP configuration */
#endif
#if defined(ACE_WIFI_PROFILE_PRIORITY)
    uint8_t priority;                   /**< Determines the preference given to a network
                                              during auto connection attempt. */
#endif
} aceWifiHal_config_t;

/**
 * @brief Configured networks list
 * List of configured networks. Used by aceWifiHal_getConfiguredNetworks()
 */
typedef struct {
    uint16_t length;    /**< Number of configured networks */
    aceWifiHal_config_t list[ACE_WIFI_MAX_CONFIGURED_NETWORKS]; /**< List to hold configured networks */
} aceWifiHal_configList_t;

/**
 * @brief Scan results record format
 */
typedef struct {
    char ssid[aceWifiHal_MAX_SSID_LEN]; /**< The network's SSID. NOT a null terminated string, can be raw bytes e.g. for unicode */
    uint8_t ssidLength;                 /**< Length of the SSID. Must be set in place of a null terminator */
    aceWifiHal_authMode_t authMode;     /**< The authentication protocol supported by this network */
    int8_t rssi;                        /**< The detected signal level in dBm */
} aceWifiHal_scanResult_t;

/**
 * @brief Scan results list. Used by aceWifiHal_getScanResults()
 */
typedef struct {
    uint16_t length;                                            /**< Number of scan results */
    aceWifiHal_scanResult_t list[ACE_WIFI_MAX_SCAN_RESULTS];    /**< List to hold scan results */
} aceWifiHal_scanResultList_t;

/** Detailed scan Result
 *  Detailed results of a scan request, this is an extension of aceWifiHal_scanResult_t,
 *  includes bssid and freq info.
 */
typedef struct {
    char ssid[aceWifiHal_MAX_SSID_LEN];         /**< The network's SSID. NOT a null terminated string, can be raw bytes e.g. for unicode */
    uint8_t bssid[aceWifiHal_BSSID_LEN];        /**< BSSID of the network */
    uint8_t ssidLength;                         /**< Length of the SSID. Must be set in place of a null terminator */
    aceWifiHal_authMode_t authMode;  /**< The authentication protocol supported by this network */
    int8_t rssi;                     /**< The detected signal level in dBm, also known as the RSSI */
    uint16_t frequency;              /**< Frequency */
} aceWifiHal_detailedScanResult_t;

/** Detailed scan results list
 *  List of detailed scan results. Used by aceWifiHal_getDetailedScanResults()
 */
typedef struct {
    uint16_t length;
    aceWifiHal_detailedScanResult_t list[ACE_WIFI_MAX_SCAN_RESULTS];
} aceWifiHal_detailedScanResultList_t;

/**
 * @brief ssid structure
 */
typedef struct {
    char ssid[aceWifiHal_MAX_SSID_LEN];         /**< SSID */
    uint8_t ssidLength;                         /**< SSID length */
} aceWifiHal_ssid_t;

/**
 * @brief Scan request configuration
 * If broadcast only, set broadcast to be 1, targetedScanItemNum to be 0.
 * Note:
 *    The first three data fields will be all zeroed from middleware layer, as they are
 * being deprecated.
 */
typedef struct {
    char ssid[aceWifiHal_MAX_SSID_LEN]; /**< @deprecated SSID for targeted scan */
    uint8_t ssidLength;                 /**< @deprecated SSID length, 0 if broadcast scan */
    uint8_t channel;                    /**< @deprecated Scan in single channel only */

    uint8_t broadcast;                 /**< 1 - broadcast scan is included */
    uint8_t targetedScanItemNum;      /**< Number of targeted scan ssid */
    aceWifiHal_ssid_t scanList[ACE_WIFI_MAX_TARGETED_SCAN_SSID_NUM];  /**< targeted scan ssid list */
    uint8_t channelNum;               /**< Number of channels to be scanned. 0 means full scan */
    uint8_t channelList[ACE_WIFI_MAX_PARTIAL_SCAN_CHANNEL_NUM];   /**< channels to be scanned,
                                                                       when not doing default full channel scan */
} aceWifiHal_scanConfig_t;


/**
 * @brief WiFi info. Describes the WiFi network information when connected,
 *        or the disconnect reason when WiFi is disconnected.
 */
typedef struct {
    uint8_t bssid[aceWifiHal_BSSID_LEN];    /**< BSSID of the connected network */
    char ssid[aceWifiHal_MAX_SSID_LEN];     /**< The network's SSID. NOT a null terminated string, can be raw bytes e.g. for unicode */
    uint8_t ssidLength;                     /**< Length of the SSID. MUST be set instead of a null terminator */
    aceWifiHal_authMode_t authMode;         /**< The authentication protocol supported by this network */
    aceWifiHal_ipAddress_t ipAddress;       /**< IP address */
    uint16_t frequency;                     /**< Frequency of the connected network */
    aceWifiHal_disconnectReason_t reason;   /**< Reason code for a disconnect */
} aceWifiHal_wifiInfo_t;

/**
 * @brief WiFi network state codes
 */
typedef enum {
    aceWifiHal_NET_STATE_CONNECTING = 0, /**< Associating/authenticating */
    aceWifiHal_NET_STATE_L2_CONNECTED,   /**< L2 Connected */
    aceWifiHal_NET_STATE_OBTAINEDIP,     /**< Obtained IP */
    aceWifiHal_NET_STATE_DISCONNECTED    /**< Disconnected */
} aceWifiHal_networkState_t;

/**
 * @brief WiFi network state info
 */
typedef struct {
    aceWifiHal_networkState_t networkState; /**< WiFi and IP network state */
    aceWifiHal_wifiInfo_t wifiInfo;         /**< WiFi connection details */
} aceWifiHal_networkStateInfo_t;


/**
 * @brief Soft AP state codes
 * State events types that indicate the state of the soft AP. When registered
 * for soft AP state change events, the state will only be published at the
 * moment of the state change.
 */
typedef enum {
    aceWifiHal_SOFTAP_UP = 0,       /**< soft AP created */
    aceWifiHal_SOFTAP_DOWN,         /**< soft AP removed */
    aceWifiHal_SOFTAP_CONNECTED,    /**< a device has connected to the soft AP */
    aceWifiHal_SOFTAP_DISCONNECTED  /**< a device has disconnected to the soft AP */
} aceWifiHal_softAPState_t;

/**
 * @brief SoftAP configuration.
 */
typedef struct {
    char ssid[32];                  /**< SSID is a binary array, not null-terminated string */
    uint8_t ssidLength;             /**< Length of SSID */
    uint8_t ssidHidden;             /**< SSID is not broadcasted */
    char psk[64];                   /**< WPA PSK */
    uint8_t pskLength;              /**< PSK length (must be between 8 and 64) */
    uint16_t frequency;             /**< Frequency of the WiFi channel.
                                         A default freq should be defined if 0 is given */

    uint8_t bssid[6];               /**< BSSID */
    aceWifiHal_authMode_t authMode; /**< Auth mode */

    uint16_t beaconInterval;        /**< Beacon interval */
    uint8_t dtim;                   /**< DTIM interval */
} aceWifiHal_apConfig_t;

/**
 * @brief Event code describing the type of a WiFi profile update
 */
typedef enum {
    aceWifiHal_PROFILE_ADDED = 0,   /**< A WiFi profile was added */
    aceWifiHal_PROFILE_DELETED,     /**< A WiFi profile was deleted */
    aceWifiHal_PROFILE_CHANGED      /**< A WiFi profile was updated */
} aceWifiHal_profileUpdateEvent_t;

/**
 * @brief Parameters for the {@link aceWifiHal_profileUpdateEvent}
 */
typedef struct {
    char ssid[aceWifiHal_MAX_SSID_LEN]; /**< The network's SSID. NOT a null terminated string, can be raw bytes e.g. for unicode */
    uint8_t ssidLength;                 /**< Length of the SSID. Must be set in place of a null terminator */
    aceWifiHal_authMode_t authMode;     /**< The authentication protocol supported by this network */
    aceWifiHal_profileUpdateEvent_t type; /**< The type of update on the profile */
} aceWifiHal_profileUpdateInfo_t;

/**
 * @brief SoftAP connected station info. Describes stations connected to the SoftAP
 */
typedef struct {
    uint8_t mac[6];                 /**< MAC address of the connected STA */
} aceWifiHal_apStaInfo_t;

/**
 * @brief SoftAP Linked list of connected stations
 */
typedef struct aceWifiHal_apStaList {
    struct aceWifiHal_apStaList* next;  /**< Link to the next item in the list */
    aceWifiHal_apStaInfo_t staInfo;     /**< Detailed info of a connected STA */
} aceWifiHal_apStaList_t;

/**
 * @brief Event types for events to be reported to the caller (WiFi middleware)
 */
typedef enum {
    aceWifiHal_wifiReadyEvent,      /**< WiFi HAL is ready */
    aceWifiHal_scanDoneEvent,       /**< WiFi scan is done */
    aceWifiHal_networkStateEvent,   /**< WiFi network state evnt */
    aceWifiHal_softAPStateEvent,    /**< SoftAP event */
    aceWifiHal_profileUpdateEvent   /**< Profile update event */
} aceWifiHal_eventType_t;

/**
 * @brief Wifi core statistic info.
 */
typedef struct {
    uint32_t txSuccessCount;   /**< No of TX success */
    uint32_t txRetryCount;     /**< No of TX tx packet retries */
    uint32_t txFailCount;      /**< No of TX failure */
    uint32_t rxSuccessCount;   /**< No of RX success */
    uint32_t rxCRCErrorCount;  /**< No of RX CRC */
    uint32_t MICErrorCount;    /**< No of Mic error count */
    uint32_t nss;              /**< No of spatial streams (MIMO)  */
    int8_t   noise;            /**< noise */
    int8_t   noise2;           /**< noise stream2 */
    uint16_t phyRate;          /**< Max phy rate */
    uint16_t txRate;           /**< tx rate */
    uint16_t rxRate;           /**< rx rate */
    int8_t   rssi;             /**< RSSI */
    int8_t   rssi2;            /**< RSSI stream2 */
    int8_t   brssi;            /**< Beacon RSSI  */
    uint8_t  channel;          /**< Channel */
    uint8_t  bandwidth;        /**< Bandwidth */
    uint8_t  idleTimePer;      /**< Percent of idle time */
} aceWifiHal_statistic_t;

/**
 * @brief Wifi band.
 */
typedef enum {
    aceWifiHal_BAND_2G = 0,    /**< 2.4G */
    aceWifiHal_BAND_5G,        /**< 5G */
    aceWifiHal_BAND_DUAL,      /**< dual band */
    aceWifiHal_BAND_MAX        /**< unsupported band */
} aceWifiHal_band_t;

/**
 * @brief Wifi phymode.
 */
typedef enum {
    aceWifiHal_PHY_11B = 0,    /**< 11B */
    aceWifiHal_PHY_11G,        /**< 11G */
    aceWifiHal_PHY_11N,        /**< 11N */
    aceWifiHal_PHY_11AC,       /**< 11AC */
    aceWifiHal_PHY_11AX,       /**< 11AX */
    aceWifiHal_PHY_MAX,        /**< Unsupported phy */
} aceWifiHal_phyMode_t;

/**
 * @brief Wifi bandwidth.
 */
typedef enum {
    aceWifiHal_BW_20 = 0,   /**< Bandwidth 20 */
    aceWifiHal_BW_40,       /**< Bandwidth 40 */
    aceWifiHal_BW_80,       /**< Bandwidth 80 */
    aceWifiHal_BW_160,      /**< Bandwidth 160 */
    aceWifiHal_BW_MAX       /**< Unsupported bandwidth */
} aceWifiHal_bandwidth_t;

/**
 * @brief Wi-Fi device power management modes.
 *
 * Device power management modes supported.
 */
typedef enum
{
    aceWifiHal_PM_NORMAL = 0,  /**< Normal mode. */
    aceWifiHal_PM_LOW,         /**< Low Power mode. */
    aceWifiHal_PM_ALWAYS_ON,   /**< Always On mode. */
    aceWifiHal_PM_MAX          /**< Unsupported PM mode. */
} aceWifiHal_PMMode_t;

/**
 * @brief Wifi capability info.
 */
typedef struct {
    aceWifiHal_band_t      band;        /**< band, for example 2.4g/5g/dual */
    aceWifiHal_phyMode_t   phyMode;     /**< Phymode, for example 11b,11g */
    aceWifiHal_bandwidth_t bandwidth;   /**< Bandwidth, for example 20M */
    uint32_t               maxAggr;     /**< Max aggregation */
    uint16_t               supportedFeatures;  /**< Supported feature, for example wps/p2p/enterprise,
                                                   in bit mask aceWifiHal_WPS_SUPPORTED|xxx */
} aceWifiHal_capability_t;

/**
 * @brief Parameters for connect.
 */
typedef struct {
    char ssid[aceWifiHal_MAX_SSID_LEN];     /**< ssid of the target WiFi network */
    uint8_t ssidLength;                     /**< ssid length */
    uint8_t bssid[aceWifiHal_BSSID_LEN];    /**< The BSSID of the target WiFi network, OPTIONAL */
    aceWifiHal_authMode_t authMode;         /**< The authmode of the AP to connect to.
                                                 When set to aceWifiHal_AUTH_MODE_MAX, then any
                                                 authmode in the profile can apply. */
} aceWifiHal_connectConfig_t;

/**
 * @brief Definition of event handler provided by the caller (WiFi middleware)
 *
 * @param[in] type enum of aceWifiHal_eventType_t
 * @param[in] data Data associated with the event
 * @param[in] dataLen Length of data
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
typedef ace_status_t (*aceWifiHal_eventHandler_t)(aceWifiHal_eventType_t type,
                                                        uint8_t* data,
                                                        uint32_t dataLen);

/**
 * @brief WiFi HAL event handler registration
 *
 * @param[in] handler Event handler provided by WiFi middleware
 * @return None
 */
void aceWifiHal_registerEventHandler(aceWifiHal_eventHandler_t handler);

/**
 * @brief Initialize WiFi HAL
 * Used for internal registration of event callbacks.
 * The HAL must notify the wifi service of events by calling these functions
 * at the appropriate time:
 *   1. aceWifiHal_onScanDoneEvent()
 *   2. aceWifiHal_onNetworkStateEvent()
 *   3. aceWifiHal_onSoftAPStateEvent()
 *
 * @param[in] config Parameters of the WiFi HAL
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_init(aceWifiHal_config_t* config);

/**
 * @brief De-initialize WiFi HAL
 * Release any resources associated with the HAL.
 *
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_deinit(void);

/**
 * @brief Start WiFi HAL
 *
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_start(void);

/**
 * @brief Start a WiFi scan
 *
 * @param[in] scanConfig Parameters for the scan request, NULL means broadcast scan.
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_startScan(aceWifiHal_scanConfig_t* scanConfig);

/**
 * @brief Add a WiFi network
 * A WiFi network is uniquely identified by its SSID and Auth Mode. If the added
 * network has the same SSID and Auth Mode as an existing network, the existing
 * network will be updated. If the added network has the same SSID but a different
 * Auth Mode than an existing network, a new network will be created.
 *
 * @param[in] config Parameters for the WiFi network.
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_addNetwork(const aceWifiHal_config_t* config);

/**
 * @brief Remove a WiFi network
 * Will disconnect WiFi if the removed network is currently active.
 *
 * @param[in] ssid The SSID of an existing WiFi network
 * @param[in] authMode: The authmode of the Profile.
 *        When set to aceWifiHal_AUTH_MODE_MAX, then any authmode can apply.
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_removeNetwork(const char* ssid, aceWifiHal_authMode_t authMode);

/**
 * @brief Save WiFi profiles to persistent storage
 *
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_saveConfig(void);

/**
 * @brief Connect to a WiFi network
 * The target network must be added previously using @ref aceWifi_addNetwork()
 *
 * @param[in] connectConfig Config of the target WiFi network
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_connect(const aceWifiHal_connectConfig_t* connectConfig);

/**
 * @brief Disconnect the current WiFi network
 *
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_disconnect(void);

/**
 * @brief Start IP provisioning using DHCP or static IP
 *
 * @param[in] ipConfig: Static IP configuration.
 *                      Passing NULL will start DHCP.
 *
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_startIpProvisioning(aceWifiHal_ipConfiguration_t* ipConfig);

/**
 * @brief Stop IP provisioning
 *
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_stopIpProvisioning(void);

/**
 * @brief Create a SoftAP
 * SoftAP operation will start automatically.
 *
 * @param[in] config Parameters for the SoftAP
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_createAP(aceWifiHal_apConfig_t* config);

/**
 * @brief Delete a SoftAP
 * SoftAP operation will be stopped.
 *
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_removeAP(void);

/**
 * @brief Get a list of connected STAs to the SoftAP
 * The maximum size of the STA list is controlled by the compile-time parameter
 * WIFI_MAX_NUMBER_OF_STA
 *
 * @param[out] staList List to hold the parameters of connected STAs
 * @param[out] staListSize Size of the STA list
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getStaList(aceWifiHal_apStaList_t* staList,
                                         uint32_t* staListSize);

/**
 * @brief Disconnect a STA connected to the SoftAP
 *
 * @param[in] mac MAC address of the target STA
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_disconnectSta(uint8_t* mac);

/**
 * @brief Enable a WiFi network
 * Will attempt to connect to the WiFi network
 *
 * @param[in] ssid SSID of the target WiFi network
 * @param[in] type Auth mode of the target WiFi network
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_enableNetwork(const char* ssid, aceWifiHal_authMode_t type);

/**
 * @brief Disable a WiFi network
 * If the disabled network is currently active, it will be disconnected.
 *
 * @param[in] ssid SSID of the target WiFi network
 * @param[in] type Auth mode of the target WiFi network
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_disableNetwork(const char* ssid, aceWifiHal_authMode_t type);

/**
 * @brief Mark all existing WiFi networks as enabled
 * Will attempt to connect to a WiFi network
 *
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_enableAllNetworks(void);

/**
 * @brief Mark all existing WiFi networks as disabled
 * If a WiFi network is currently active, it will be disconnected.
 *
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_disableAllNetworks(void);

/**
 * @brief Get WiFi scan results. The list contains the filtered scan results which
 * does not have duplicate ssid (with same security).
 *
 * The maximum number of scan results that can be obtained is controlled by a
 * compile-time parameter ACE_WIFI_MAX_SCAN_RESULTS.
 *
 * @param[out] scanResultList List to hold the scan results
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getScanResults(aceWifiHal_scanResultList_t* scanResultList);

/**
 * @brief Get WiFi scan results, which is not filtered from the original scan
 * results from wifi driver.
 *
 * The maximum number of scan results that can be obtained is controlled by a
 * compile-time parameter ACE_WIFI_MAX_SCAN_RESULTS.
 *
 * @param[out] scanResultList List to hold the scan results
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getDetailedScanResults(aceWifiHal_detailedScanResultList_t* scanResultList);

/**
 * @brief Get all configured WiFi networks
 * The maximum number of networks that can be returned is controlled by a
 * compile-time parameter ACE_WIFI_MAX_CONFIGURED_NETWORKS.
 *
 * @param[out] configList List to hold the configured networks
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getConfiguredNetworks(aceWifiHal_configList_t* configList);

/**
 * @brief Get the current WiFi network state (Layers 2 and 3)
 *
 * @return Return enum of aceWifiHal_networkState_t
 */
aceWifiHal_networkState_t aceWifiHal_getNetworkState(void);

/**
 * @brief Get the current WiFi connection info (Layer 2)
 *
 * @param[out] wifiInfo Current WiFi network info
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getConnectionInfo(aceWifiHal_wifiInfo_t* wifiInfo);

/**
 * @brief Get the IP configuration
 *
 * @param[out] ipInfo IP configuration
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getIpInfo(aceWifiHal_ipConfiguration_t* ipInfo);

/**
 * @brief Get the STA MAC address
 *
 * @param[out] macAddress Buffer to hold the STA MAC address
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getMacAddress(aceWifiHal_macAddress_t* macAddress);

/**
 * @brief Set the STA MAC address
 *
 * @param[in] macAddress The STA MAC address
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_setMacAddress(aceWifiHal_macAddress_t* macAddress);

/**
 * @brief Get the AP MAC address (BSSID)
 *
 * @param[out] macAddress The AP MAC address
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getAPMacAddress(aceWifiHal_macAddress_t* macAddress);

/**
 * @brief Set the country code for the WiFi device
 *
 * @param[in] country The country code
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_setCountryCode(const char* country);

/**
 * @brief Get the country code on the WiFi device
 *
 * @param[in,out] country Pointer to a buffer to hold the country code
 * @param[in]  length  Length of the buffer. It should be at least
 *                     @ref aceWifiHal_COUNTRY_CODE_LEN
 *                     @note https://en.wikipedia.org/wiki/ISO_3166-1
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getCountryCode(char* country, uint32_t length);

/**
 * @brief Sets the power mode of the WiFi device
 * For example, when the power manager broadcasts a power state change, this
 * function can be invoked to set the WiFi device to the corresponding power
 * state
 *
 * @param[in] powerMode The power mode to set
 *
 * @return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_setPowerMode(aceWifiHal_PMMode_t powerMode);

/**
 * @brief Gets the current power mode of the WiFi device
 *
 * @param[out] powerMode Pointer to aceWifiHal_PMMode_t that will be set to the
 * current mode.
 *
 * @return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getPowerMode(aceWifiHal_PMMode_t* powerMode);

/**
 * @brief Reconnect to the last connected WiFi network
 *
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_reconnect(void);

/**
 * @brief Get the RSSI of the connected WiFi network
 *
 * @param[out] rssi RSSI of the connected WiFi network
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getRssi(int8_t* rssi);

/**
 * @deprecated
 * @brief Enable WiFi statistics collection
 *
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_enableWifiStats(void);

/**
 * @deprecated
 * @brief Disable WiFi statistics collection
 *
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_disableWifiStats(void);

/**
 * @deprecated - See @ref aceWifiHal_getCoreWifiStats instead.
 * @brief Get the WiFi statistics string, can be customized string.
 *
 * @param[out] stats Buffer to hold the WiFi statistics
 * @param[in] len Length of the buffer
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getWifiStats(char* stats, int len);

/**
 * @brief Get the core WiFi statistics.
 *
 * @param[out] stats structure to hold the core WiFi statistics
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getCoreWifiStats(aceWifiHal_statistic_t* statsInfo);

/**
 * @brief Get the WiFi capability.
 *
 * @param[out] caps structure to hold the WiFi capabilities.
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getWifiCapability(aceWifiHal_capability_t* capsInfo);

/**
 * @deprecated
 * @brief Get the KDM metric
 *
 * @param[out] data Buffer to hold the KDM data
 * @param[in] size Size of the buffer
 * @return Return ACE_STATUS_OK for success, error codes for failure
 */
ace_status_t aceWifiHal_getKdm(char* data, int32_t size);

#ifdef  __cplusplus
}
#endif

#endif // __ACE_WIFI_HAL_H__
/** @} */
