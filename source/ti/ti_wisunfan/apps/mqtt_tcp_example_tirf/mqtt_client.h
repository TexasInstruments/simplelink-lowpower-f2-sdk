/******************************************************************************
 @file  mqtt_client.h
 @brief MQTT 3.1.1 client — ported from lwIP mqtt.c for Nanostack/tcp_client

 Original: lwIP mqtt.c by Erik Andersson <erian747@gmail.com>
 Ported:   Replaced altcp/pbuf/sys_timeout with tcp_client/flat buffers/eventOS
 *****************************************************************************/
#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Configuration ---- */
#define MQTT_OUTPUT_RINGBUF_SIZE         (256)     /* output ring buffer bytes     */
#define MQTT_VAR_HEADER_BUFFER_LEN       (256)     /* rx variable header buffer    */
#define MQTT_REQ_MAX_IN_FLIGHT           (4)       /* max pending QoS 1/2 requests */
#define MQTT_REQ_TIMEOUT                 (30)      /* request timeout seconds      */
#define MQTT_CONNECT_TIMEO               (100)     /* connect timeout seconds      */
#define MQTT_CYCLIC_TIMER_INTERVAL       (5)       /* timer tick seconds           */

/* ---- Error codes (replacing lwIP err_t) ---- */
#define MQTT_ERR_OK        (0)
#define MQTT_ERR_MEM      (-1)
#define MQTT_ERR_CONN     (-2)
#define MQTT_ERR_ARG      (-3)
#define MQTT_ERR_TIMEOUT  (-4)
#define MQTT_ERR_ABORT    (-5)
#define MQTT_ERR_ISCONN   (-6)
#define MQTT_ERR_VAL      (-7)

/* ---- Connection status (from lwIP mqtt.h) ---- */
typedef enum {
    MQTT_CONNECT_ACCEPTED                 = 0,
    MQTT_CONNECT_REFUSED_PROTOCOL_VERSION = 1,
    MQTT_CONNECT_REFUSED_IDENTIFIER       = 2,
    MQTT_CONNECT_REFUSED_SERVER           = 3,
    MQTT_CONNECT_REFUSED_USERNAME_PASS    = 4,
    MQTT_CONNECT_REFUSED_NOT_AUTHORIZED   = 5,
    MQTT_CONNECT_DISCONNECTED             = 256,
    MQTT_CONNECT_TIMEOUT                  = 257
} mqtt_connection_status_t;

/* ---- Connection states ---- */
enum {
    TCP_DISCONNECTED = 0,
    TCP_CONNECTING,
    MQTT_CONNECTING,
    MQTT_CONNECTED
};

/* ---- Forward declaration ---- */
typedef struct mqtt_client_s mqtt_client_t;

/* ---- Callback types ---- */

/** Connection status change callback */
typedef void (*mqtt_connection_cb_t)(mqtt_client_t *client, void *arg,
                                     mqtt_connection_status_t status);

/** Incoming publish: topic notification (called first) */
typedef void (*mqtt_incoming_publish_cb_t)(void *arg, const char *topic,
                                           uint32_t tot_len);

/** Incoming publish: data fragment (called after topic, may be called multiple times) */
#define MQTT_DATA_FLAG_LAST  1
typedef void (*mqtt_incoming_data_cb_t)(void *arg, const uint8_t *data,
                                        uint16_t len, uint8_t flags);

/** Request completion callback (for publish QoS 1/2, subscribe, unsubscribe) */
typedef void (*mqtt_request_cb_t)(void *arg, int err);

/* ---- Connect parameters ---- */
typedef struct {
    const char *client_id;
    const char *client_user;       /* NULL if unused */
    const char *client_pass;       /* NULL if unused */
    uint16_t    keep_alive;        /* seconds, 0 = disabled */
    const char *will_topic;        /* NULL if unused */
    const char *will_msg;          /* NULL if unused */
    uint8_t     will_msg_len;      /* 0 = use strlen(will_msg) */
    uint8_t     will_qos;
    uint8_t     will_retain;
} mqtt_connect_client_info_t;

/* ---- Output ring buffer ---- */
typedef struct {
    uint16_t put;
    uint16_t get;
    uint8_t  buf[MQTT_OUTPUT_RINGBUF_SIZE];
} mqtt_ringbuf_t;

/* ---- Request queue entry ---- */
typedef struct mqtt_request_s {
    struct mqtt_request_s *next;
    mqtt_request_cb_t      cb;
    void                  *arg;
    uint16_t               pkt_id;
    int16_t                timeout_diff;
} mqtt_request_t;

/* ---- MQTT client instance ---- */
struct mqtt_client_s {
    /* Connection state */
    uint8_t  conn_state;
    uint16_t keep_alive;          /* seconds */
    uint16_t pkt_id_seq;

    /* Cyclic timer */
    uint16_t cyclic_tick;
    uint16_t server_watchdog;
    void    *timer_handle;        /* eventOS timeout_t* */

    /* Output */
    mqtt_ringbuf_t output;

    /* Input */
    uint8_t  rx_buffer[MQTT_VAR_HEADER_BUFFER_LEN];
    uint32_t msg_idx;
    uint16_t inpub_pkt_id;

    /* Callbacks */
    mqtt_connection_cb_t       connect_cb;
    void                      *connect_arg;
    mqtt_incoming_publish_cb_t pub_cb;
    mqtt_incoming_data_cb_t    data_cb;
    void                      *inpub_arg;

    /* Request queue */
    mqtt_request_t  req_list[MQTT_REQ_MAX_IN_FLIGHT];
    mqtt_request_t *pend_req_queue;
};

/* ---- Public API ---- */

/**
 * Initialize the MQTT client (static instance, call once at startup).
 */
void mqtt_init(void);

/**
 * Connect to MQTT broker via tcp_client.
 * @param ip_addr   Broker IPv6 address string
 * @param port      Broker port (typically 1883)
 * @param cb        Connection status callback
 * @param arg       User argument for callback
 * @param client_info  Connection parameters
 * @return MQTT_ERR_OK on success
 */
int mqtt_client_connect(const char *ip_addr, uint16_t port,
                        mqtt_connection_cb_t cb, void *arg,
                        const mqtt_connect_client_info_t *client_info);

/**
 * Disconnect from MQTT broker.
 */
void mqtt_client_disconnect(void);

/**
 * Check if connected.
 * @return 1 if MQTT_CONNECTED, 0 otherwise
 */
uint8_t mqtt_client_is_connected(void);

/**
 * Publish a message.
 * @param topic   Topic string
 * @param payload Payload data (NULL allowed)
 * @param payload_length  Payload length
 * @param qos     0, 1, or 2
 * @param retain  MQTT retain flag
 * @param cb      Completion callback
 * @param arg     User argument
 * @return MQTT_ERR_OK on success
 */
int mqtt_publish(const char *topic, const void *payload,
                 uint16_t payload_length, uint8_t qos, uint8_t retain,
                 mqtt_request_cb_t cb, void *arg);

/**
 * Subscribe to a topic.
 */
int mqtt_subscribe(const char *topic, uint8_t qos,
                   mqtt_request_cb_t cb, void *arg);

/**
 * Unsubscribe from a topic.
 */
int mqtt_unsubscribe(const char *topic,
                     mqtt_request_cb_t cb, void *arg);

/**
 * Set callbacks for incoming PUBLISH messages from broker.
 */
void mqtt_set_inpub_callback(mqtt_incoming_publish_cb_t pub_cb,
                             mqtt_incoming_data_cb_t data_cb, void *arg);

/**
 * Get pointer to the static client instance (for status queries).
 */
mqtt_client_t *mqtt_get_client(void);

/**
 * Simplified connect — hides mqtt_connect_client_info_t from callers.
 * Calls mqtt_init() internally before connecting.
 * @return 0 on success, negative on error
 */
int mqtt_connect_simple(const char *broker_addr, uint16_t port,
                        const char *client_id, uint16_t keepalive);

/**
 * Get connection state as uint8_t (0=disconnected, 1=tcp_connecting,
 * 2=mqtt_connecting, 3=mqtt_connected).
 */
uint8_t mqtt_get_conn_state(void);

/**
 * Get connection state as a human-readable string.
 */
const char *mqtt_get_status_str(void);

#ifdef __cplusplus
}
#endif
#endif /* MQTT_CLIENT_H */