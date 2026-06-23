/******************************************************************************
 @file  mqtt_client.c
 @brief MQTT 3.1.1 client — ported from lwIP mqtt.c for Nanostack/tcp_client

 Original copyright:
   Copyright (c) 2016 Erik Andersson <erian747@gmail.com>
   Part of the lwIP TCP/IP stack, BSD licensed.

 *****************************************************************************/

#include "mbed_config_app.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "ns_trace.h"
#include "eventOS_event_timer.h"
#include "ti_wisunfan_features.h"
#include "tcp_client.h"
#include "mqtt_client.h"


#define TRACE_GROUP  ("mqtt")

#define MQTT_MIN(a, b)  ((a) < (b) ? (a) : (b))
#define MQTT_ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))

/* ---- MQTT message types ---- */
enum mqtt_message_type {
    MQTT_MSG_TYPE_CONNECT     = 1,
    MQTT_MSG_TYPE_CONNACK     = 2,
    MQTT_MSG_TYPE_PUBLISH     = 3,
    MQTT_MSG_TYPE_PUBACK      = 4,
    MQTT_MSG_TYPE_PUBREC      = 5,
    MQTT_MSG_TYPE_PUBREL      = 6,
    MQTT_MSG_TYPE_PUBCOMP     = 7,
    MQTT_MSG_TYPE_SUBSCRIBE   = 8,
    MQTT_MSG_TYPE_SUBACK      = 9,
    MQTT_MSG_TYPE_UNSUBSCRIBE = 10,
    MQTT_MSG_TYPE_UNSUBACK    = 11,
    MQTT_MSG_TYPE_PINGREQ     = 12,
    MQTT_MSG_TYPE_PINGRESP    = 13,
    MQTT_MSG_TYPE_DISCONNECT  = 14
};

#define MQTT_CTL_PACKET_TYPE(b)  (((b) & 0xf0) >> 4)
#define MQTT_CTL_PACKET_QOS(b)   (((b) & 0x6) >> 1)

/* Connect flags */
#define MQTT_CONNECT_FLAG_USERNAME      (1 << 7)
#define MQTT_CONNECT_FLAG_PASSWORD      (1 << 6)
#define MQTT_CONNECT_FLAG_WILL_RETAIN   (1 << 5)
#define MQTT_CONNECT_FLAG_WILL          (1 << 2)
#define MQTT_CONNECT_FLAG_CLEAN_SESSION (1 << 1)

/* ---- Static client instance ---- */
static mqtt_client_t mqtt_client;

/* Socket slot index returned by tcp_client_connect.
 * -1 = no socket allocated. Reset on disconnect.
 * TCP layer is unaware this slot is used by MQTT. */
static int32_t g_mqtt_tcp_ind = -1;

/* ---- Forward declarations ---- */
static void mqtt_cyclic_timer(void *arg);
static void mqtt_output_send(mqtt_ringbuf_t *rb);
static void mqtt_close(mqtt_client_t *client, mqtt_connection_status_t reason);
static void mqtt_rx_data_cb(const uint8_t *data, uint16_t len);
static void mqtt_socket_event_cb(int event);

/* ==================================================================
 * PACKET ID GENERATOR
 * ================================================================ */

static uint16_t msg_generate_packet_id(mqtt_client_t *client)
{
    client->pkt_id_seq++;
    if (client->pkt_id_seq == 0)
        client->pkt_id_seq++;
    return client->pkt_id_seq;
}

/* ==================================================================
 * OUTPUT RING BUFFER (preserved from lwIP)
 * ================================================================ */

static void mqtt_ringbuf_put(mqtt_ringbuf_t *rb, uint8_t item)
{
    rb->buf[rb->put] = item;
    rb->put++;
    if (rb->put >= MQTT_OUTPUT_RINGBUF_SIZE)
        rb->put = 0;
}

static uint8_t *mqtt_ringbuf_get_ptr(mqtt_ringbuf_t *rb)
{
    return &rb->buf[rb->get];
}

static void mqtt_ringbuf_advance_get_idx(mqtt_ringbuf_t *rb, uint16_t len)
{
    rb->get += len;
    if (rb->get >= MQTT_OUTPUT_RINGBUF_SIZE)
        rb->get = rb->get - MQTT_OUTPUT_RINGBUF_SIZE;
}

static uint16_t mqtt_ringbuf_len(mqtt_ringbuf_t *rb)
{
    uint32_t len = rb->put - rb->get;
    if (len > 0xFFFF)
        len += MQTT_OUTPUT_RINGBUF_SIZE;
    return (uint16_t)len;
}

#define mqtt_ringbuf_free(rb)              (MQTT_OUTPUT_RINGBUF_SIZE - mqtt_ringbuf_len(rb))
#define mqtt_ringbuf_linear_read_length(rb) MQTT_MIN(mqtt_ringbuf_len(rb), (MQTT_OUTPUT_RINGBUF_SIZE - (rb)->get))

/**
 * Flush ring buffer contents via tcp_send_data().
 * Replaces lwIP's altcp_write/altcp_output.
 */
static void mqtt_output_send(mqtt_ringbuf_t *rb)
{
    uint16_t lin_len;

    /* Send first linear chunk */
    lin_len = mqtt_ringbuf_linear_read_length(rb);
    if (lin_len > 0) {
        tcp_send_data(g_mqtt_tcp_ind, mqtt_ringbuf_get_ptr(rb), lin_len);
        mqtt_ringbuf_advance_get_idx(rb, lin_len);
    }
    /* Send second chunk if ring buffer wrapped */
    lin_len = mqtt_ringbuf_linear_read_length(rb);
    if (lin_len > 0) {
        tcp_send_data(g_mqtt_tcp_ind, mqtt_ringbuf_get_ptr(rb), lin_len);
        mqtt_ringbuf_advance_get_idx(rb, lin_len);
    }
}

/* ==================================================================
 * REQUEST QUEUE (preserved from lwIP)
 * ================================================================ */

static mqtt_request_t *
mqtt_create_request(mqtt_request_t *r_objs, size_t r_objs_len,
                    uint16_t pkt_id, mqtt_request_cb_t cb, void *arg)
{
    mqtt_request_t *r = NULL;
    uint8_t n;
    for (n = 0; n < r_objs_len; n++) {
        if (r_objs[n].next == &r_objs[n]) {
            r = &r_objs[n];
            r->next = NULL;
            r->cb = cb;
            r->arg = arg;
            r->pkt_id = pkt_id;
            break;
        }
    }
    return r;
}

static void mqtt_append_request(mqtt_request_t **tail, mqtt_request_t *r)
{
    mqtt_request_t *head = NULL;
    int16_t time_before = 0;
    mqtt_request_t *iter;

    for (iter = *tail; iter != NULL; iter = iter->next) {
        time_before += iter->timeout_diff;
        head = iter;
    }
    r->timeout_diff = MQTT_REQ_TIMEOUT - time_before;
    if (head == NULL)
        *tail = r;
    else
        head->next = r;
}

static void mqtt_delete_request(mqtt_request_t *r)
{
    if (r != NULL)
        r->next = r;  /* self-pointer = unused */
}

static mqtt_request_t *
mqtt_take_request(mqtt_request_t **tail, uint16_t pkt_id)
{
    mqtt_request_t *iter = NULL, *prev = NULL;
    for (iter = *tail; iter != NULL; iter = iter->next) {
        if (iter->pkt_id == pkt_id)
            break;
        prev = iter;
    }
    if (iter != NULL) {
        if (prev == NULL)
            *tail = iter->next;
        else
            prev->next = iter->next;
        if (iter->next != NULL)
            iter->next->timeout_diff += iter->timeout_diff;
        iter->next = NULL;
    }
    return iter;
}

static void mqtt_request_time_elapsed(mqtt_request_t **tail, uint8_t t)
{
    mqtt_request_t *r = *tail;
    while (t > 0 && r != NULL) {
        if (t >= r->timeout_diff) {
            t -= (uint8_t)r->timeout_diff;
            *tail = r->next;
            if (r->cb != NULL)
                r->cb(r->arg, MQTT_ERR_TIMEOUT);
            mqtt_delete_request(r);
            r = *(mqtt_request_t *const volatile *)tail;
        } else {
            r->timeout_diff -= t;
            t = 0;
        }
    }
}

static void mqtt_clear_requests(mqtt_request_t **tail)
{
    mqtt_request_t *iter, *next;
    for (iter = *tail; iter != NULL; iter = next) {
        next = iter->next;
        mqtt_delete_request(iter);
    }
    *tail = NULL;
}

static void mqtt_init_requests(mqtt_request_t *r_objs, size_t r_objs_len)
{
    uint8_t n;
    for (n = 0; n < r_objs_len; n++)
        r_objs[n].next = &r_objs[n];  /* self-pointer = unused */
}

/* ==================================================================
 * OUTPUT MESSAGE BUILD HELPERS (preserved from lwIP)
 * ================================================================ */

static void mqtt_output_append_u8(mqtt_ringbuf_t *rb, uint8_t value)
{
    mqtt_ringbuf_put(rb, value);
}

static void mqtt_output_append_u16(mqtt_ringbuf_t *rb, uint16_t value)
{
    mqtt_ringbuf_put(rb, value >> 8);
    mqtt_ringbuf_put(rb, value & 0xff);
}

static void mqtt_output_append_buf(mqtt_ringbuf_t *rb, const void *data,
                                   uint16_t length)
{
    uint16_t n;
    for (n = 0; n < length; n++)
        mqtt_ringbuf_put(rb, ((const uint8_t *)data)[n]);
}

static void mqtt_output_append_string(mqtt_ringbuf_t *rb, const char *str,
                                      uint16_t length)
{
    uint16_t n;
    mqtt_ringbuf_put(rb, length >> 8);
    mqtt_ringbuf_put(rb, length & 0xff);
    for (n = 0; n < length; n++)
        mqtt_ringbuf_put(rb, str[n]);
}

static void mqtt_output_append_fixed_header(mqtt_ringbuf_t *rb, uint8_t msg_type,
                                            uint8_t fdup, uint8_t fqos,
                                            uint8_t fretain, uint16_t r_length)
{
    mqtt_output_append_u8(rb, (((msg_type & 0x0f) << 4) | ((fdup & 1) << 3) |
                               ((fqos & 3) << 1) | (fretain & 1)));
    do {
        mqtt_output_append_u8(rb, (r_length & 0x7f) | (r_length >= 128 ? 0x80 : 0));
        r_length >>= 7;
    } while (r_length > 0);
}

static uint8_t mqtt_output_check_space(mqtt_ringbuf_t *rb, uint16_t r_length)
{
    uint16_t total_len = 1 + r_length;
    do {
        total_len++;
        r_length >>= 7;
    } while (r_length > 0);
    return (total_len <= mqtt_ringbuf_free(rb));
}

/* ==================================================================
 * CLOSE / TIMER (adapted from lwIP)
 * ================================================================ */

static void mqtt_close(mqtt_client_t *client, mqtt_connection_status_t reason)
{
    /* Tear down TCP — tcp_disconnect clears hooks internally */
    if (g_mqtt_tcp_ind >= 0) {
        if (client->conn_state != TCP_DISCONNECTED)
            tcp_disconnect(g_mqtt_tcp_ind);
        g_mqtt_tcp_ind = -1;
    }

    mqtt_clear_requests(&client->pend_req_queue);

    /* Stop cyclic timer */
    if (client->timer_handle) {
        eventOS_timeout_cancel((timeout_t *)client->timer_handle);
        client->timer_handle = NULL;
    }

    if (client->conn_state != TCP_DISCONNECTED) {
        client->conn_state = TCP_DISCONNECTED;
        if (client->connect_cb != NULL)
            client->connect_cb(client, client->connect_arg, reason);
    }
}

/**
 * Cyclic timer — called every MQTT_CYCLIC_TIMER_INTERVAL seconds.
 * Handles keepalive PINGREQ, server watchdog, and request timeouts.
 */
static void mqtt_cyclic_timer(void *arg)
{
    uint8_t restart_timer = 1;
    mqtt_client_t *client = (mqtt_client_t *)arg;
    client->timer_handle = NULL;

    if (client->conn_state == MQTT_CONNECTING) {
        client->cyclic_tick++;
        if ((client->cyclic_tick * MQTT_CYCLIC_TIMER_INTERVAL) >= MQTT_CONNECT_TIMEO) {
            tr_warn("MQTT: CONNECT timeout");
            mqtt_close(client, MQTT_CONNECT_TIMEOUT);
            restart_timer = 0;
        }
    } else if (client->conn_state == MQTT_CONNECTED) {
        /* Handle request timeouts */
        mqtt_request_time_elapsed(&client->pend_req_queue, MQTT_CYCLIC_TIMER_INTERVAL);

        if (client->keep_alive > 0) {
            client->server_watchdog++;
            /* Server unresponsive if idle for 1.5 * keep_alive */
            if ((client->server_watchdog * MQTT_CYCLIC_TIMER_INTERVAL) >
                (client->keep_alive + client->keep_alive / 2)) {
                tr_warn("MQTT: Server keep-alive timeout");
                mqtt_close(client, MQTT_CONNECT_TIMEOUT);
                restart_timer = 0;
            } else {
                /* Send PINGREQ at keep_alive interval */
                client->cyclic_tick++;
                if ((client->cyclic_tick * MQTT_CYCLIC_TIMER_INTERVAL) >= client->keep_alive) {
                    if (mqtt_output_check_space(&client->output, 0) != 0) {
                        mqtt_output_append_fixed_header(&client->output,
                            MQTT_MSG_TYPE_PINGREQ, 0, 0, 0, 0);
                        mqtt_output_send(&client->output);
                        client->cyclic_tick = 0;
                    }
                }
            }
        }
    } else {
        restart_timer = 0;
    }

    if (restart_timer) {
        client->timer_handle = (void *)eventOS_timeout_ms(
            mqtt_cyclic_timer, MQTT_CYCLIC_TIMER_INTERVAL * 1000, client);
    }
}

/* ==================================================================
 * INCOMING MESSAGE HANDLER (adapted from lwIP — pbuf replaced with flat buffer)
 * ================================================================ */

static int
pub_ack_rec_rel_response(mqtt_client_t *client, uint8_t msg, uint16_t pkt_id, uint8_t qos)
{
    if (mqtt_output_check_space(&client->output, 2)) {
        mqtt_output_append_fixed_header(&client->output, msg, 0, qos, 0, 2);
        mqtt_output_append_u16(&client->output, pkt_id);
        mqtt_output_send(&client->output);
        return MQTT_ERR_OK;
    }
    return MQTT_ERR_MEM;
}

static mqtt_connection_status_t
mqtt_message_received(mqtt_client_t *client, uint8_t fixed_hdr_len,
                      uint16_t length, uint32_t remaining_length,
                      uint8_t *var_hdr_payload)
{
    mqtt_connection_status_t res = MQTT_CONNECT_ACCEPTED;
    uint8_t pkt_type = MQTT_CTL_PACKET_TYPE(client->rx_buffer[0]);
    uint16_t pkt_id = 0;

    if (pkt_type == MQTT_MSG_TYPE_CONNACK) {
        if (client->conn_state == MQTT_CONNECTING) {
            if (length < 2) goto out_disconnect;
            res = (mqtt_connection_status_t)var_hdr_payload[1];
            tr_info("MQTT: CONNACK rc=%d", (int)res);
            if (res == MQTT_CONNECT_ACCEPTED) {
                client->cyclic_tick = 0;
                client->conn_state = MQTT_CONNECTED;
                if (client->connect_cb != NULL)
                    client->connect_cb(client, client->connect_arg, res);
            }
        }
    } else if (pkt_type == MQTT_MSG_TYPE_PINGRESP) {
        tr_debug("MQTT: PINGRESP");

    } else if (pkt_type == MQTT_MSG_TYPE_PUBLISH) {
        uint16_t payload_offset = 0;
        uint16_t payload_length = length;
        uint8_t qos = MQTT_CTL_PACKET_QOS(client->rx_buffer[0]);

        if (client->msg_idx == (uint32_t)(fixed_hdr_len + length)) {
            /* First frame: extract topic */
            uint16_t topic_len;
            uint16_t after_topic;
            uint8_t bkp;
            uint16_t qos_len = (qos ? 2U : 0U);

            if (length < 2 + qos_len) goto out_disconnect;
            topic_len = ((uint16_t)var_hdr_payload[0] << 8) | var_hdr_payload[1];
            if (topic_len > length - (2 + qos_len)) goto out_disconnect;

            after_topic = 2 + topic_len;
            if (qos > 0) {
                if (length < after_topic + 2U) goto out_disconnect;
                client->inpub_pkt_id = ((uint16_t)var_hdr_payload[after_topic] << 8) |
                                        var_hdr_payload[after_topic + 1];
                after_topic += 2;
            } else {
                client->inpub_pkt_id = 0;
            }

            bkp = var_hdr_payload[2 + topic_len];
            var_hdr_payload[2 + topic_len] = 0;  /* null-terminate topic */

            payload_length = length - after_topic;
            payload_offset = after_topic;

            tr_info("MQTT RX: topic=%s len=%lu qos=%d",
                    (const char *)&var_hdr_payload[2],
                    (unsigned long)(remaining_length + payload_length), qos);

            if (client->pub_cb != NULL)
                client->pub_cb(client->inpub_arg,
                               (const char *)&var_hdr_payload[2],
                               remaining_length + payload_length);

            var_hdr_payload[2 + topic_len] = bkp;  /* restore */
        }

        if (payload_length > 0 || remaining_length == 0) {
            if (client->data_cb != NULL)
                client->data_cb(client->inpub_arg,
                                var_hdr_payload + payload_offset,
                                payload_length,
                                remaining_length == 0 ? MQTT_DATA_FLAG_LAST : 0);

            if (remaining_length == 0 && qos > 0) {
                uint8_t resp = (qos == 1) ? MQTT_MSG_TYPE_PUBACK : MQTT_MSG_TYPE_PUBREC;
                pub_ack_rec_rel_response(client, resp, client->inpub_pkt_id, 0);
            }
        }
    } else {
        /* PUBACK, SUBACK, UNSUBACK, PUBREC, PUBREL, PUBCOMP */
        if (length < 2) goto out_disconnect;
        pkt_id = ((uint16_t)var_hdr_payload[0] << 8) | var_hdr_payload[1];
        if (pkt_id == 0) goto out_disconnect;

        if (pkt_type == MQTT_MSG_TYPE_PUBREC) {
            pub_ack_rec_rel_response(client, MQTT_MSG_TYPE_PUBREL, pkt_id, 1);
        } else if (pkt_type == MQTT_MSG_TYPE_PUBREL) {
            pub_ack_rec_rel_response(client, MQTT_MSG_TYPE_PUBCOMP, pkt_id, 0);
        } else if (pkt_type == MQTT_MSG_TYPE_SUBACK ||
                   pkt_type == MQTT_MSG_TYPE_UNSUBACK ||
                   pkt_type == MQTT_MSG_TYPE_PUBCOMP ||
                   pkt_type == MQTT_MSG_TYPE_PUBACK) {
            mqtt_request_t *r = mqtt_take_request(&client->pend_req_queue, pkt_id);
            if (r != NULL) {
                if (pkt_type == MQTT_MSG_TYPE_SUBACK) {
                    if (length < 3) goto out_disconnect;
                    if (r->cb != NULL)
                        r->cb(r->arg, var_hdr_payload[2] < 3 ? MQTT_ERR_OK : MQTT_ERR_ABORT);
                } else if (r->cb != NULL) {
                    r->cb(r->arg, MQTT_ERR_OK);
                }
                mqtt_delete_request(r);
            }
        }
    }
    return res;

out_disconnect:
    return MQTT_CONNECT_DISCONNECTED;
}

/**
 * Parse incoming TCP data into MQTT packets.
 * Adapted from lwIP mqtt_parse_incoming — pbuf replaced with flat buffer.
 */
static mqtt_connection_status_t
mqtt_parse_incoming(mqtt_client_t *client, const uint8_t *data, uint16_t data_len)
{
    uint16_t in_offset = 0;
    uint32_t msg_rem_len = 0;
    uint8_t  fixed_hdr_len = 0;
    uint8_t  b = 0;

    while (data_len > in_offset) {
        if ((fixed_hdr_len < 2) || ((b & 0x80) != 0)) {
            if (fixed_hdr_len < client->msg_idx) {
                /* Re-parse header from buffer (cross-segment header) */
                b = client->rx_buffer[fixed_hdr_len];
            } else {
                b = data[in_offset++];
                client->rx_buffer[client->msg_idx++] = b;
            }
            fixed_hdr_len++;

            if (fixed_hdr_len >= 2) {
                msg_rem_len |= (uint32_t)(b & 0x7f) << ((fixed_hdr_len - 2) * 7);
                if ((b & 0x80) == 0) {
                    if (msg_rem_len == 0) {
                        mqtt_message_received(client, fixed_hdr_len, 0, 0, NULL);
                        client->msg_idx = 0;
                        fixed_hdr_len = 0;
                    } else {
                        msg_rem_len = (msg_rem_len + fixed_hdr_len) - client->msg_idx;
                    }
                }
            }
        } else {
            /* Variable header + payload */
            uint16_t cpy_len, buffer_space;
            uint8_t *var_hdr_payload;
            mqtt_connection_status_t res;

            cpy_len = (uint16_t)MQTT_MIN((uint16_t)(data_len - in_offset), msg_rem_len);
            buffer_space = MQTT_VAR_HEADER_BUFFER_LEN - fixed_hdr_len;
            if (cpy_len > buffer_space)
                cpy_len = buffer_space;

            /* Copy into rx_buffer (flat buffer — no pbuf_get_contiguous needed) */
            memcpy(client->rx_buffer + fixed_hdr_len, &data[in_offset], cpy_len);
            var_hdr_payload = client->rx_buffer + fixed_hdr_len;

            client->msg_idx += cpy_len;
            in_offset += cpy_len;
            msg_rem_len -= cpy_len;

            res = mqtt_message_received(client, fixed_hdr_len, cpy_len,
                                        msg_rem_len, var_hdr_payload);
            if (res != MQTT_CONNECT_ACCEPTED)
                return res;

            if (msg_rem_len == 0) {
                client->msg_idx = 0;
                fixed_hdr_len = 0;
            }
        }
    }
    return MQTT_CONNECT_ACCEPTED;
}

/* ==================================================================
 * TCP_CLIENT HOOKS (replace lwIP altcp callbacks)
 * ================================================================ */

/** Called by tcp_app when data arrives on the client socket. */
static void mqtt_rx_data_cb(const uint8_t *data, uint16_t len)
{
    mqtt_client_t *client = &mqtt_client;

    mqtt_connection_status_t res = mqtt_parse_incoming(client, data, len);
    if (res != MQTT_CONNECT_ACCEPTED)
        mqtt_close(client, res);

    /* Reset server watchdog on any received data */
    if (client->keep_alive != 0)
        client->server_watchdog = 0;
}

/** Called by tcp_client on TCP_EVENT_CONNECTED or TCP_EVENT_DISCONNECTED. */
static void mqtt_socket_event_cb(int event)
{
    mqtt_client_t *client = &mqtt_client;

    if (event == TCP_EVENT_CONNECTED) {
        /* TCP connected — initialize receiver and send CONNECT (already in ring buffer) */
        tr_info("MQTT: TCP connected, flushing CONNECT packet");
        client->msg_idx = 0;
        client->conn_state = MQTT_CONNECTING;

        /* Start cyclic timer */
        client->cyclic_tick = 0;
        client->timer_handle = (void *)eventOS_timeout_ms(
            mqtt_cyclic_timer, MQTT_CYCLIC_TIMER_INTERVAL * 1000, client);

        /* Flush the CONNECT message that was buffered in the ring buffer */
        mqtt_output_send(&client->output);

        /* Handle QoS 0 callbacks for anything already queued */
        {
            mqtt_request_t *r;
            while ((r = mqtt_take_request(&client->pend_req_queue, 0)) != NULL) {
                if (r->cb != NULL) r->cb(r->arg, MQTT_ERR_OK);
                mqtt_delete_request(r);
            }
        }
    } else {
        /* TCP disconnected */
        tr_info("MQTT: TCP connection lost");

        /* Socket slot freed by tcp layer; reset our index */
        g_mqtt_tcp_ind = -1;

        mqtt_clear_requests(&client->pend_req_queue);
        if (client->timer_handle) {
            eventOS_timeout_cancel((timeout_t *)client->timer_handle);
            client->timer_handle = NULL;
        }
        if (client->conn_state != TCP_DISCONNECTED) {
            client->conn_state = TCP_DISCONNECTED;
            if (client->connect_cb != NULL)
                client->connect_cb(client, client->connect_arg,
                                   MQTT_CONNECT_DISCONNECTED);
        }
    }
}

/* ==================================================================
 * PUBLIC API (adapted from lwIP)
 * ================================================================ */

void mqtt_init(void)
{
    memset(&mqtt_client, 0, sizeof(mqtt_client));
    mqtt_init_requests(mqtt_client.req_list, MQTT_ARRAYSIZE(mqtt_client.req_list));
    tr_info("MQTT: Client initialized (lwIP port)");
}

mqtt_client_t *mqtt_get_client(void)
{
    return &mqtt_client;
}

void mqtt_set_inpub_callback(mqtt_incoming_publish_cb_t pub_cb,
                             mqtt_incoming_data_cb_t data_cb, void *arg)
{
    mqtt_client.data_cb   = data_cb;
    mqtt_client.pub_cb    = pub_cb;
    mqtt_client.inpub_arg = arg;
}

int mqtt_client_connect(const char *ip_addr, uint16_t port,
                        mqtt_connection_cb_t cb, void *arg,
                        const mqtt_connect_client_info_t *client_info)
{
    mqtt_client_t *client = &mqtt_client;
    size_t len;
    uint16_t client_id_length;
    uint16_t remaining_length = 2 + 4 + 1 + 1 + 2;  /* "MQTT" + level + flags + keepalive */
    uint8_t flags = 0;
    uint8_t will_topic_len = 0, will_msg_len = 0;
    uint16_t client_user_len = 0, client_pass_len = 0;
    mqtt_incoming_data_cb_t    save_data_cb;
    mqtt_incoming_publish_cb_t save_pub_cb;
    void *save_inpub_arg;

    if (client->conn_state != TCP_DISCONNECTED) {
        tr_warn("MQTT: Already connected");
        return MQTT_ERR_ISCONN;
    }

    /* Preserve callbacks across reset */
    save_data_cb   = client->data_cb;
    save_pub_cb    = client->pub_cb;
    save_inpub_arg = client->inpub_arg;
    memset(client, 0, sizeof(mqtt_client_t));
    client->data_cb   = save_data_cb;
    client->pub_cb    = save_pub_cb;
    client->inpub_arg = save_inpub_arg;

    client->connect_arg = arg;
    client->connect_cb  = cb;
    client->keep_alive  = client_info->keep_alive;
    mqtt_init_requests(client->req_list, MQTT_ARRAYSIZE(client->req_list));

    /* ---- Build CONNECT message into ring buffer ---- */

    /* Will message */
    if (client_info->will_topic != NULL && client_info->will_msg != NULL) {
        flags |= MQTT_CONNECT_FLAG_WILL;
        flags |= (client_info->will_qos & 3) << 3;
        if (client_info->will_retain)
            flags |= MQTT_CONNECT_FLAG_WILL_RETAIN;

        len = strlen(client_info->will_topic);
        if (len == 0 || len > 0xFF) return MQTT_ERR_VAL;
        will_topic_len = (uint8_t)len;

        if (client_info->will_msg_len == 0) {
            len = strlen(client_info->will_msg);
            if (len > 0xFF) return MQTT_ERR_VAL;
            will_msg_len = (uint8_t)len;
        } else {
            will_msg_len = client_info->will_msg_len;
        }
        len = remaining_length + 2 + will_topic_len + 2 + will_msg_len;
        if (len > 0xFFFF) return MQTT_ERR_VAL;
        remaining_length = (uint16_t)len;
    }

    /* Username */
    if (client_info->client_user != NULL) {
        flags |= MQTT_CONNECT_FLAG_USERNAME;
        len = strlen(client_info->client_user);
        if (len == 0 || len > 0xFFFF) return MQTT_ERR_VAL;
        client_user_len = (uint16_t)len;
        len = remaining_length + 2 + client_user_len;
        if (len > 0xFFFF) return MQTT_ERR_VAL;
        remaining_length = (uint16_t)len;
    }

    /* Password */
    if (client_info->client_pass != NULL) {
        flags |= MQTT_CONNECT_FLAG_PASSWORD;
        len = strlen(client_info->client_pass);
        if (len == 0 || len > 0xFFFF) return MQTT_ERR_VAL;
        client_pass_len = (uint16_t)len;
        len = remaining_length + 2 + client_pass_len;
        if (len > 0xFFFF) return MQTT_ERR_VAL;
        remaining_length = (uint16_t)len;
    }

    flags |= MQTT_CONNECT_FLAG_CLEAN_SESSION;

    len = strlen(client_info->client_id);
    if (len > 0xFFFF) return MQTT_ERR_VAL;
    client_id_length = (uint16_t)len;
    len = remaining_length + 2 + client_id_length;
    if (len > 0xFFFF) return MQTT_ERR_VAL;
    remaining_length = (uint16_t)len;

    if (mqtt_output_check_space(&client->output, remaining_length) == 0)
        return MQTT_ERR_MEM;

    /* Append fixed header */
    mqtt_output_append_fixed_header(&client->output, MQTT_MSG_TYPE_CONNECT,
                                    0, 0, 0, remaining_length);
    mqtt_output_append_string(&client->output, "MQTT", 4);
    mqtt_output_append_u8(&client->output, 4);  /* Protocol level 3.1.1 */
    mqtt_output_append_u8(&client->output, flags);
    mqtt_output_append_u16(&client->output, client_info->keep_alive);
    mqtt_output_append_string(&client->output, client_info->client_id, client_id_length);

    if ((flags & MQTT_CONNECT_FLAG_WILL) != 0) {
        mqtt_output_append_string(&client->output, client_info->will_topic, will_topic_len);
        mqtt_output_append_string(&client->output, client_info->will_msg, will_msg_len);
    }
    if ((flags & MQTT_CONNECT_FLAG_USERNAME) != 0)
        mqtt_output_append_string(&client->output, client_info->client_user, client_user_len);
    if ((flags & MQTT_CONNECT_FLAG_PASSWORD) != 0)
        mqtt_output_append_string(&client->output, client_info->client_pass, client_pass_len);

    /* ---- Initiate TCP connection, then register hooks on the returned slot ---- */
    tr_info("MQTT: Connecting to [%s]:%d as '%s'", ip_addr, port, client_info->client_id);
    client->conn_state = TCP_CONNECTING;

    /* Hooks are passed at connect time — TCP stores and fires them without
     * knowing they belong to MQTT */
    g_mqtt_tcp_ind = tcp_client_connect(ip_addr, port,
                                      mqtt_rx_data_cb, mqtt_socket_event_cb);
    if (g_mqtt_tcp_ind < 0) {
        tr_error("MQTT: tcp_client_connect failed");
        client->conn_state = TCP_DISCONNECTED;
        return MQTT_ERR_CONN;
    }

    return MQTT_ERR_OK;
}

void mqtt_client_disconnect(void)
{
    mqtt_client_t *client = &mqtt_client;
    if (client->conn_state != TCP_DISCONNECTED) {
        /* Send DISCONNECT packet if connected */
        if (client->conn_state == MQTT_CONNECTED) {
            if (mqtt_output_check_space(&client->output, 0)) {
                mqtt_output_append_fixed_header(&client->output,
                    MQTT_MSG_TYPE_DISCONNECT, 0, 0, 0, 0);
                mqtt_output_send(&client->output);
            }
        }
        mqtt_close(client, (mqtt_connection_status_t)0);
    }
}

uint8_t mqtt_client_is_connected(void)
{
    return mqtt_client.conn_state == MQTT_CONNECTED;
}

int mqtt_publish(const char *topic, const void *payload,
                 uint16_t payload_length, uint8_t qos, uint8_t retain,
                 mqtt_request_cb_t cb, void *arg)
{
    mqtt_client_t *client = &mqtt_client;
    mqtt_request_t *r;
    uint16_t pkt_id;
    uint16_t topic_len;
    uint16_t remaining_length;
    size_t total_len;

    if (client->conn_state == TCP_DISCONNECTED)
        return MQTT_ERR_CONN;

    topic_len = (uint16_t)strlen(topic);
    total_len = 2 + topic_len + payload_length;
    if (qos > 0) {
        total_len += 2;
        pkt_id = msg_generate_packet_id(client);
    } else {
        pkt_id = 0;
    }
    if (total_len > 0xFFFF) return MQTT_ERR_ARG;
    remaining_length = (uint16_t)total_len;

    r = mqtt_create_request(client->req_list, MQTT_ARRAYSIZE(client->req_list),
                            pkt_id, cb, arg);
    if (r == NULL) return MQTT_ERR_MEM;

    if (mqtt_output_check_space(&client->output, remaining_length) == 0) {
        mqtt_delete_request(r);
        return MQTT_ERR_MEM;
    }

    mqtt_output_append_fixed_header(&client->output, MQTT_MSG_TYPE_PUBLISH,
                                    0, qos, retain, remaining_length);
    mqtt_output_append_string(&client->output, topic, topic_len);
    if (qos > 0)
        mqtt_output_append_u16(&client->output, pkt_id);
    if (payload != NULL && payload_length > 0)
        mqtt_output_append_buf(&client->output, payload, payload_length);

    mqtt_append_request(&client->pend_req_queue, r);
    mqtt_output_send(&client->output);

    /* Immediately complete QoS 0 requests (no server ACK expected) */
    if (qos == 0) {
        mqtt_request_t *r0;
        while ((r0 = mqtt_take_request(&client->pend_req_queue, 0)) != NULL) {
            if (r0->cb != NULL) r0->cb(r0->arg, MQTT_ERR_OK);
            mqtt_delete_request(r0);
        }
    }

    return MQTT_ERR_OK;
}

int mqtt_subscribe(const char *topic, uint8_t qos,
                   mqtt_request_cb_t cb, void *arg)
{
    mqtt_client_t *client = &mqtt_client;
    uint16_t topic_len, remaining_length, pkt_id;
    size_t total_len;
    mqtt_request_t *r;

    if (client->conn_state == TCP_DISCONNECTED) return MQTT_ERR_CONN;

    topic_len = (uint16_t)strlen(topic);
    total_len = topic_len + 2 + 2 + 1;  /* topic + pkt_id + qos */
    if (total_len > 0xFFFF) return MQTT_ERR_ARG;
    remaining_length = (uint16_t)total_len;

    pkt_id = msg_generate_packet_id(client);
    r = mqtt_create_request(client->req_list, MQTT_ARRAYSIZE(client->req_list),
                            pkt_id, cb, arg);
    if (r == NULL) return MQTT_ERR_MEM;

    if (mqtt_output_check_space(&client->output, remaining_length) == 0) {
        mqtt_delete_request(r);
        return MQTT_ERR_MEM;
    }

    mqtt_output_append_fixed_header(&client->output, MQTT_MSG_TYPE_SUBSCRIBE,
                                    0, 1, 0, remaining_length);
    mqtt_output_append_u16(&client->output, pkt_id);
    mqtt_output_append_string(&client->output, topic, topic_len);
    mqtt_output_append_u8(&client->output, MQTT_MIN(qos, 2));

    mqtt_append_request(&client->pend_req_queue, r);
    mqtt_output_send(&client->output);
    return MQTT_ERR_OK;
}

int mqtt_unsubscribe(const char *topic, mqtt_request_cb_t cb, void *arg)
{
    mqtt_client_t *client = &mqtt_client;
    uint16_t topic_len, remaining_length, pkt_id;
    size_t total_len;
    mqtt_request_t *r;

    if (client->conn_state == TCP_DISCONNECTED) return MQTT_ERR_CONN;

    topic_len = (uint16_t)strlen(topic);
    total_len = topic_len + 2 + 2;
    if (total_len > 0xFFFF) return MQTT_ERR_ARG;
    remaining_length = (uint16_t)total_len;

    pkt_id = msg_generate_packet_id(client);
    r = mqtt_create_request(client->req_list, MQTT_ARRAYSIZE(client->req_list),
                            pkt_id, cb, arg);
    if (r == NULL) return MQTT_ERR_MEM;

    if (mqtt_output_check_space(&client->output, remaining_length) == 0) {
        mqtt_delete_request(r);
        return MQTT_ERR_MEM;
    }

    mqtt_output_append_fixed_header(&client->output, MQTT_MSG_TYPE_UNSUBSCRIBE,
                                    0, 1, 0, remaining_length);
    mqtt_output_append_u16(&client->output, pkt_id);
    mqtt_output_append_string(&client->output, topic, topic_len);

    mqtt_append_request(&client->pend_req_queue, r);
    mqtt_output_send(&client->output);
    return MQTT_ERR_OK;
}

int mqtt_connect_simple(const char *broker_addr, uint16_t port,
                        const char *client_id, uint16_t keepalive)
{
    static mqtt_connect_client_info_t ci;
    memset(&ci, 0, sizeof(ci));
    ci.client_id  = client_id;
    ci.keep_alive = keepalive;

    mqtt_init();
    return mqtt_client_connect(broker_addr, port, NULL, NULL, &ci);
}

uint8_t mqtt_get_conn_state(void)
{
    return mqtt_client.conn_state;
}

const char *mqtt_get_status_str(void)
{
    switch (mqtt_client.conn_state) {
    case MQTT_CONNECTED:  return "connected";
    case MQTT_CONNECTING: return "connecting";
    case TCP_CONNECTING:  return "tcp_connecting";
    default:              return "disconnected";
    }
}

