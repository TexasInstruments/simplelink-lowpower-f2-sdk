/*
 * Copyright  2020-2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.  This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sid_api.h>
#include <sid_pal_crypto_ifc.h>
#include <sid_pal_mfg_store_ifc.h>
#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_uptime_ifc.h>
#include <sid_time_ops.h>
#include <sid_900_cfg.h>
#include <sid_demo_parser.h>
#include <sid_demo_types.h>

#include <FreeRTOS.h>
#include <timers.h>
#include <queue.h>
#include <task.h>

//FIXME: These are internal deps of sidewalk,
// the application shouldn't be using these headers
#include <sid_pal_log_ifc.h>
#include <ti_itm.h>

#include <ti/drivers/Board.h>
#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>
#include <ti/drivers/Temperature.h>
#include <ti_drivers_config.h>
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)

#if SID_SDK_CONFIG_ENABLE_LINK_TYPE_1 == 1
#include <app_ble_config.h>
#endif
#if SID_SDK_CONFIG_ENABLE_LINK_TYPE_2 == 1
#include <app_900_config.h>
#endif

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define MAIN_TASK_STACK_SIZE        (2048 / sizeof(configSTACK_DEPTH_TYPE))
#define RECEIVE_TASK_STACK_SIZE     (2048 / sizeof(configSTACK_DEPTH_TYPE))
#define MSG_QUEUE_LEN 10
#define MSG_LOG_BLOCK_SIZE 80

 //FIXME: this is actually less, but we have to set to 0x2000 for sid_pal_mfg_store_init to succeed
#define MANUFACTURE_FLASH_SIZE          0x02000
#define MANUFACTURE_FLASH_END           (MANUFACTURE_FLASH_START + MANUFACTURE_FLASH_SIZE)

#define DEMO_BUTTONS_MAX                    1
#define DEMO_LEDS_MAX                       2
#define PAYLOAD_MAX_SIZE                    255
#define DEMO_CAPABILITY_PERIOD_MS           5000
#define BUTTON_PRESS_CHECK_PERIOD_SECS      30
#define DEMO_NOTIFY_SENSOR_DATA_PERIOD_MS   15000
#define CONNECT_LINK_TYPE_1_DELAY_MS        60000
#define CONNECT_LINK_TYPE_1_INIT_DELAY_MS   5000
#define PROFILE_CHECK_TIMER_DELAY_MS        60000

#define LED_ACTION_REPONSE_PAYLOAD_SIZE_MAX 32
#define SENSOR_NOTIFY_PAYLOAD_SIZE_MAX      32
#define CAPABILITY_NOTIFY_PAYLOAD_SIZE_MAX  32

#define SID_DEMO_APP_TTL_MAX                60
#define SID_DEMO_APP_RETRIES_MAX            3

enum event_type
{
    EVENT_TYPE_SIDEWALK,
    EVENT_FACTORY_RESET,
    EVENT_BUTTON_PRESS,
    EVENT_CONNECT_LINK_TYPE_1,
    EVENT_SET_DEVICE_PROFILE_LINK_TYPE_2,
    EVENT_NOTIFICATION_TIMER_FIRED,
    EVENT_BUTTON_PRESS_TIMER_FIRED,
    EVENT_SEND_MESSAGE,
};

enum app_sidewalk_state
{
    STATE_SIDEWALK_INIT,
    STATE_SIDEWALK_READY,
    STATE_SIDEWALK_NOT_READY,
    STATE_SIDEWALK_SECURE_CONNECTION,
};

enum demo_app_state
{
    DEMO_APP_STATE_INIT,
    DEMO_APP_STATE_REGISTERED,
    DEMO_APP_STATE_NOTIFY_CAPABILITY,
    DEMO_APP_STATE_NOTIFY_SENSOR_DATA,
};

enum demo_app_led_id
{
    DEMO_APP_LED_ID_0 = 0,
    DEMO_APP_LED_ID_1 = 1,
    DEMO_APP_LED_ID_LAST,
};

enum demo_app_button_id
{
    DEMO_APP_BUTTON_ID_0 = 0,
    DEMO_APP_BUTTON_ID_LAST,
};

struct link_status
{
    enum sid_time_sync_status time_sync_status;
    uint32_t link_mask;
    uint32_t supported_link_mode[SID_LINK_TYPE_MAX_IDX];
};

struct app_demo_rx_msg {
    uint16_t msg_id;
    size_t pld_size;
    uint8_t rx_payload[PAYLOAD_MAX_SIZE];
};

struct app_demo_tx_msg {
    struct sid_msg_desc desc;
    size_t pld_size;
    uint8_t tx_payload[PAYLOAD_MAX_SIZE];
};

struct demo_app_event {
    enum event_type type;
    void *data;
};

typedef struct app_context
{
    TaskHandle_t main_task;
    QueueHandle_t event_queue;
    TimerHandle_t cap_timer_handle;
    TimerHandle_t button_press_timer_handle;
    TimerHandle_t device_profile_timer_handle;
    struct sid_handle *sidewalk_handle;
    enum app_sidewalk_state sidewalk_state;
    enum demo_app_state app_state;
    struct link_status link_status;
    Button_Handle button_1;
    Button_Handle button_2;
    LED_Handle led_handles[DEMO_LEDS_MAX];
    uint8_t led_id_arr[DEMO_LEDS_MAX];
    uint8_t button_id_arr[DEMO_BUTTONS_MAX];
    uint32_t button_press_mask;
    uint32_t button_notify_mask;
    bool button_event_pending_processing;
    uint32_t button_press_time_in_sec_id_arr[DEMO_BUTTONS_MAX];
    uint8_t buffer[PAYLOAD_MAX_SIZE];
} app_context_t;

typedef struct receive_context
{
    TaskHandle_t receive_task;
    QueueHandle_t receive_event_queue;
} receive_context_t;

/* Global mainly because bsp_evt_handler does not have a context pointer */
static QueueHandle_t g_event_queue;
static app_context_t g_app_context = {0};

static QueueHandle_t g_receieve_event_queue;
static receive_context_t g_receive_context = {0};

void __assert_func(const char* file, int line, const char* func, const char* failedexpr)
{
    TI_LOG_ERROR("assert '%s' in %s:%s line %d", failedexpr, file, func, line);
    ITM_flushModule();
    taskDISABLE_INTERRUPTS();
    while(true);
}

static void queue_event(QueueHandle_t queue, enum event_type event, void *data, bool in_isr)
{
    struct demo_app_event evt = {.type = event, .data = data};
    if (in_isr) {
        BaseType_t task_woken = pdFALSE;
        xQueueSendFromISR(queue, &evt, &task_woken);
        portYIELD_FROM_ISR(task_woken);
    }
    else {
        xQueueSend(queue, &evt, 0);
    }
}

static void queue_rx_msg(QueueHandle_t queue, struct app_demo_rx_msg *rx_msg, bool in_isr)
{
    if (in_isr) {
        BaseType_t task_woken = pdFALSE;
        xQueueSendToBackFromISR(queue, rx_msg, &task_woken);
        portYIELD_FROM_ISR(task_woken);
    }
    else {
        xQueueSendToBack(queue, rx_msg, 0);
    }
}

static void on_sidewalk_event(bool in_isr, void *context)
{
    app_context_t *app_context = (app_context_t *)context;
    queue_event(app_context->event_queue, EVENT_TYPE_SIDEWALK, NULL, in_isr);
}

static void log_sid_msg(const struct sid_msg *msg)
{
    for (size_t i = 0; i < msg->size; i += MSG_LOG_BLOCK_SIZE) {
        if (i + MSG_LOG_BLOCK_SIZE > msg->size) {
            TI_LOG_HEXDUMP_INFO(msg->data + i, msg->size - i);
        } else {
            TI_LOG_HEXDUMP_INFO(msg->data + i, MSG_LOG_BLOCK_SIZE);
        }
    }
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context)
{
    TI_LOG_INFO("received message(link_type: %d, type: %d, link_mode: %d, id: %u size %u rssi %d snr %d)",
                 (int)msg_desc->link_type,
                 (int)msg_desc->type,
                 (int)msg_desc->link_mode, msg_desc->id, msg->size,
                 (int)msg_desc->msg_desc_attr.rx_attr.rssi,  (int)msg_desc->msg_desc_attr.rx_attr.snr);
    log_sid_msg(msg);

    if (msg_desc->type == SID_MSG_TYPE_RESPONSE && msg_desc->msg_desc_attr.rx_attr.is_msg_ack) {
        TI_LOG_INFO("Received Ack for msg id %d", msg_desc->id);
    } else {
        struct app_demo_rx_msg rx_msg = {
            .msg_id = msg_desc->id,
            .pld_size = msg->size,
        };
        memcpy(rx_msg.rx_payload, msg->data, msg->size);
        queue_rx_msg(g_receieve_event_queue, &rx_msg, false);
    }
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
    TI_LOG_INFO("sent message(link_type: %d, type: %d, id: %u)", (int)msg_desc->link_type, (int)msg_desc->type, msg_desc->id);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context)
{
    TI_LOG_ERROR("failed to send message(type: %d, id: %u), err:%d",
                  (int)msg_desc->type, msg_desc->id, (int)error);
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
    app_context_t *app_context = (app_context_t *)context;
    TI_LOG_INFO("status changed: %d", (int)status->state);
    switch (status->state) {
        case SID_STATE_READY:
            app_context->sidewalk_state = STATE_SIDEWALK_READY;
            break;
        case SID_STATE_NOT_READY:
            app_context->sidewalk_state = STATE_SIDEWALK_NOT_READY;
            break;
        case SID_STATE_ERROR:
            TI_LOG_INFO("sidewalk error: %d", (int)sid_get_error(app_context->sidewalk_handle));
            assert(false);
            break;
        case SID_STATE_SECURE_CHANNEL_READY:
            app_context->sidewalk_state = STATE_SIDEWALK_SECURE_CONNECTION;
            break;
    }
    TI_LOG_INFO("Registration Status = %d, Time Sync Status = %d and Link Status Mask = %x, Sidewalk State = %d",
                 status->detail.registration_status, status->detail.time_sync_status,
                 status->detail.link_status_mask, status->state);

    app_context->link_status.link_mask = status->detail.link_status_mask;
    app_context->link_status.time_sync_status = status->detail.time_sync_status;

    for (int i = 0; i < SID_LINK_TYPE_MAX_IDX; i++) {
        app_context->link_status.supported_link_mode[i] = status->detail.supported_link_modes[i];
        TI_LOG_INFO("Link %d Mode %x", i, status->detail.supported_link_modes[i]);
    }

    if (app_context->app_state == DEMO_APP_STATE_INIT &&
            status->detail.registration_status == SID_STATUS_REGISTERED) {
        app_context->app_state = DEMO_APP_STATE_REGISTERED;
    }

    if (app_context->sidewalk_state == STATE_SIDEWALK_READY) {
        TickType_t delay = pdMS_TO_TICKS(DEMO_CAPABILITY_PERIOD_MS);
        if (app_context->app_state == DEMO_APP_STATE_REGISTERED) {
            app_context->app_state = DEMO_APP_STATE_NOTIFY_CAPABILITY;
        } else if (app_context->app_state == DEMO_APP_STATE_NOTIFY_SENSOR_DATA) {
            delay = DEMO_NOTIFY_SENSOR_DATA_PERIOD_MS;
        }
        BaseType_t ret = xTimerIsTimerActive(g_app_context.cap_timer_handle);
        if (ret == pdTRUE) {
            ret = xTimerStop(g_app_context.cap_timer_handle, 0);
            ASSERT(ret == pdPASS)
        }
        ret = xTimerChangePeriod(g_app_context.cap_timer_handle, pdMS_TO_TICKS(delay), 0);
        ASSERT(ret == pdPASS)
    }

    if (BUILD_SID_SDK_LINK_TYPE == 1 || BUILD_SID_SDK_LINK_TYPE == 4) {
        if (!(status->detail.link_status_mask & SID_LINK_TYPE_1) && (status->detail.registration_status == SID_STATUS_REGISTERED)
            && (status->detail.time_sync_status == SID_STATUS_TIME_SYNCED)) {
        BaseType_t ret = xTimerIsTimerActive(g_app_context.cap_timer_handle);
        if (ret == pdTRUE) {
            ret = xTimerStop(g_app_context.cap_timer_handle, 0);
            ASSERT(ret == pdPASS)
        }
        ret = xTimerChangePeriod(g_app_context.cap_timer_handle, pdMS_TO_TICKS(CONNECT_LINK_TYPE_1_INIT_DELAY_MS), 0);
        ASSERT(ret == pdPASS)
        }
    }

    if (BUILD_SID_SDK_LINK_TYPE == 2 || BUILD_SID_SDK_LINK_TYPE == 4) {
        if ((status->detail.link_status_mask & SID_LINK_TYPE_2) && (status->detail.registration_status == SID_STATUS_REGISTERED)
            && (status->detail.time_sync_status == SID_STATUS_TIME_SYNCED)) {
            BaseType_t ret = xTimerIsTimerActive(g_app_context.device_profile_timer_handle);
            if (ret == pdTRUE) {
                ret = xTimerStop(g_app_context.device_profile_timer_handle, 0);
                ASSERT(ret == pdPASS);
            }
            ret = xTimerChangePeriod(g_app_context.device_profile_timer_handle, pdMS_TO_TICKS(PROFILE_CHECK_TIMER_DELAY_MS), 0);
            ASSERT(ret == pdPASS);
        }
    }
}

static void on_sidewalk_factory_reset(void *context)
{
    TI_LOG_INFO("factory reset notification received from sid api");
    SysCtrlSystemReset();
}

static void get_active_link_type(enum sid_link_type *link_type)
{
    if (g_app_context.link_status.link_mask & SID_LINK_TYPE_1) {
        *link_type = SID_LINK_TYPE_1;
    } else if (g_app_context.link_status.link_mask & SID_LINK_TYPE_2) {
        *link_type = SID_LINK_TYPE_2;
    }
}

static void send_msg(struct sid_msg_desc *desc, struct sid_msg *msg)
{
    if (g_app_context.sidewalk_state == STATE_SIDEWALK_READY ||
        g_app_context.sidewalk_state == STATE_SIDEWALK_SECURE_CONNECTION) {
        sid_error_t ret = sid_put_msg(g_app_context.sidewalk_handle, msg, desc);
        if (ret != SID_ERROR_NONE) {
            TI_LOG_ERROR("failed queueing data, err:%d", (int) ret);
        } else {
            TI_LOG_INFO("queued data message id:%u", desc->id);
            log_sid_msg(msg);
        }
    } else {
        TI_LOG_ERROR("sidewalk is not ready yet!");
    }
}

static void factory_reset(void)
{
    sid_error_t ret = sid_set_factory_reset(g_app_context.sidewalk_handle);
    if (ret != SID_ERROR_NONE) {
        TI_LOG_ERROR("Notification of factory reset to sid api failed!");
        SysCtrlSystemReset();
    } else {
        TI_LOG_INFO("Wait for Sid api to notify to proceed with factory reset!");
    }
}

static void demo_app_notify_capability(void)
{
    struct sid_parse_state state = {};
    enum sid_link_type link_type = SID_LINK_TYPE_1;
    struct sid_demo_capability_discovery cap = {
        .num_buttons = sizeof(g_app_context.button_id_arr) / sizeof(g_app_context.button_id_arr[0]),
        .button_id_arr = g_app_context.button_id_arr,
        .num_leds = sizeof(g_app_context.led_id_arr) / sizeof(g_app_context.led_id_arr[0]),
        .led_id_arr = g_app_context.led_id_arr,
        .temp_sensor = SID_DEMO_TEMPERATURE_SENSOR_UNITS_CELSIUS,
    };

    uint8_t temp_buffer[CAPABILITY_NOTIFY_PAYLOAD_SIZE_MAX] = {0};
    get_active_link_type(&link_type);
    cap.link_type = link_type;

    sid_parse_state_init(&state, temp_buffer, sizeof(temp_buffer));
    sid_demo_app_capability_discovery_notification_serialize(&state, &cap);
    if (state.ret_code != SID_ERROR_NONE) {
        TI_LOG_ERROR("capability discovery serialize failed -%d", state.ret_code);
        return;
    }

    struct sid_demo_msg_desc msg_desc = {
        .status_hdr_ind = false,
        .opc = SID_DEMO_MSG_TYPE_NOTIFY,
        .cmd_class = SID_DEMO_APP_CLASS,
        .cmd_id = SID_DEMO_APP_CLASS_CMD_CAP_DISCOVERY_ID,
    };

    struct sid_demo_msg demo_msg = {.payload = temp_buffer, .payload_size = state.offset};

    sid_parse_state_init(&state, g_app_context.buffer, sizeof(g_app_context.buffer));
    sid_demo_app_msg_serialize(&state, &msg_desc, &demo_msg);

    if (state.ret_code != SID_ERROR_NONE) {
        TI_LOG_ERROR("demo msg capability discovery serialize failed -%d", state.ret_code);
        return;
    }

    struct sid_msg msg = {
        .data = g_app_context.buffer,
        .size = state.offset,
    };
    struct sid_msg_desc desc = {
        .link_type = link_type,
        .type = SID_MSG_TYPE_NOTIFY,
        .link_mode = SID_LINK_MODE_CLOUD,
    };
    TI_LOG_INFO("Sending demo app message notify capability");
    send_msg(&desc, &msg);
}

static void demo_app_notify_sensor_data(bool button_pressed)
{
    struct sid_parse_state state = {0};
    enum sid_link_type link_type = SID_LINK_TYPE_1;
    struct sid_demo_action_notification action_notify = {0};
    struct sid_timespec curr_time = {0};
    uint8_t temp_buffer[SENSOR_NOTIFY_PAYLOAD_SIZE_MAX] = {0};

    action_notify.button_action_notify.action_resp = (button_pressed) ? SID_DEMO_ACTION_BUTTON_PRESSED :
                                                     SID_DEMO_ACTION_BUTTON_NOT_PRESSED;
    sid_get_time(g_app_context.sidewalk_handle, SID_GET_GPS_TIME, &curr_time);
    if (!button_pressed) {
        action_notify.temp_sensor = SID_DEMO_TEMPERATURE_SENSOR_UNITS_CELSIUS;
        action_notify.temperature = Temperature_getTemperature();
        action_notify.button_action_notify.action_resp = SID_DEMO_ACTION_BUTTON_NOT_PRESSED;
    } else {
        action_notify.temp_sensor = SID_DEMO_TEMPERATURE_SENSOR_NOT_SUPPORTED;
        action_notify.temperature = 0;
        action_notify.button_action_notify.action_resp = SID_DEMO_ACTION_BUTTON_PRESSED;
        uint8_t temp_button_arr[DEMO_BUTTONS_MAX] = {0};
        uint8_t num_buttons_pressed = 0;
        for (size_t i = 0; i < DEMO_BUTTONS_MAX; i++) {
            if (((1 << g_app_context.button_id_arr[i]) & (g_app_context.button_press_mask)) &&
                 (!((1 << g_app_context.button_id_arr[i]) & (g_app_context.button_notify_mask)))) {
                temp_button_arr[num_buttons_pressed] = g_app_context.button_id_arr[i];
                num_buttons_pressed += 1;
                g_app_context.button_notify_mask |= (1 << g_app_context.button_id_arr[i]);
                g_app_context.button_press_time_in_sec_id_arr[i] = curr_time.tv_sec;
                BaseType_t ret = xTimerIsTimerActive(g_app_context.button_press_timer_handle);
                if (ret == pdFALSE) {
                    BaseType_t ret = xTimerChangePeriod(g_app_context.button_press_timer_handle,
                        pdMS_TO_TICKS(BUTTON_PRESS_CHECK_PERIOD_SECS * 1000), 0);
                    ASSERT(ret == pdPASS)
                }
            }
        }
        action_notify.button_action_notify.button_id_arr = temp_button_arr;
        action_notify.button_action_notify.num_buttons = num_buttons_pressed;
    }

    action_notify.gps_time_in_seconds = curr_time.tv_sec;
    get_active_link_type(&link_type);
    action_notify.link_type = link_type;

    sid_parse_state_init(&state, temp_buffer, sizeof(temp_buffer));
    sid_demo_app_action_notification_serialize(&state, &action_notify);
    if (state.ret_code != SID_ERROR_NONE) {
        TI_LOG_ERROR("capability discovery serialize failed -%d", state.ret_code);
        return;
    }

    struct sid_demo_msg_desc msg_desc = {
        .status_hdr_ind = false,
        .opc = SID_DEMO_MSG_TYPE_NOTIFY,
        .cmd_class = SID_DEMO_APP_CLASS,
        .cmd_id = SID_DEMO_APP_CLASS_CMD_ACTION,
    };

    struct sid_demo_msg demo_msg = {.payload = temp_buffer, .payload_size = state.offset};

    sid_parse_state_init(&state, g_app_context.buffer, sizeof(g_app_context.buffer));
    sid_demo_app_msg_serialize(&state, &msg_desc, &demo_msg);

    if (state.ret_code != SID_ERROR_NONE) {
        TI_LOG_ERROR("demo msg notify sensor data serialize failed -%d", state.ret_code);
        return;
    }

    struct sid_msg msg = {
        .data = g_app_context.buffer,
        .size = state.offset,
    };
    struct sid_msg_desc desc = {
        .link_type = link_type,
        .type = SID_MSG_TYPE_NOTIFY,
        .link_mode = SID_LINK_MODE_CLOUD,
    };

    if (button_pressed) {
        desc.msg_desc_attr.tx_attr.ttl_in_seconds = SID_DEMO_APP_TTL_MAX;
        desc.msg_desc_attr.tx_attr.num_retries = SID_DEMO_APP_RETRIES_MAX;
        desc.msg_desc_attr.tx_attr.request_ack = true;
        TI_LOG_INFO("Sending demo app message notify button press");
    } else {
        TI_LOG_INFO("Sending demo app message notify sensor data");
    }
    send_msg(&desc, &msg);
}

static void demo_app_create_led_response(struct sid_demo_action_resp *resp)
{
    struct sid_parse_state state = {0};
    enum sid_link_type link_type = SID_LINK_TYPE_1;
    uint8_t temp_buffer[LED_ACTION_REPONSE_PAYLOAD_SIZE_MAX] = {0};

    sid_parse_state_init(&state, temp_buffer, sizeof(temp_buffer));
    sid_demo_app_action_resp_serialize(&state, resp);

    struct sid_demo_msg_desc msg_desc = {
        .status_hdr_ind = true,
        .opc = SID_DEMO_MSG_TYPE_RESP,
        .cmd_class = SID_DEMO_APP_CLASS,
        .cmd_id = SID_DEMO_APP_CLASS_CMD_ACTION,
        .status_code = SID_ERROR_NONE,
    };

    struct sid_demo_msg demo_msg = {.payload = temp_buffer, .payload_size = state.offset};

    sid_parse_state_init(&state, g_app_context.buffer, sizeof(g_app_context.buffer));
    sid_demo_app_msg_serialize(&state, &msg_desc, &demo_msg);

    if (state.ret_code != SID_ERROR_NONE) {
        TI_LOG_ERROR("demo msg notify sensor data serialize failed -%d", state.ret_code);
        return;
    }

    get_active_link_type(&link_type);
    struct sid_msg msg = {
        .data = g_app_context.buffer,
        .size = state.offset,
    };
    struct sid_msg_desc desc = {
        .link_type = link_type,
        .type = SID_MSG_TYPE_NOTIFY,
        .link_mode = SID_LINK_MODE_CLOUD,
        .msg_desc_attr = {
            .tx_attr = {
                .ttl_in_seconds = SID_DEMO_APP_TTL_MAX,
                .num_retries = SID_DEMO_APP_RETRIES_MAX,
                .request_ack = true,
            }
        }
    };

    struct app_demo_tx_msg *data = (struct app_demo_tx_msg *)malloc(sizeof(struct app_demo_tx_msg));
    if (data == NULL) {
        TI_LOG_ERROR("Failed to allocate tx data memory !");
        return;
    } else {
        data->pld_size = state.offset;
        memcpy(data->tx_payload, g_app_context.buffer, data->pld_size);
        data->desc = desc;
    }

    TI_LOG_INFO("Led response created");
    queue_event(g_app_context.event_queue, EVENT_SEND_MESSAGE, data, false);
}

static int32_t init_and_start_link(app_context_t *context, struct sid_config *config, uint32_t link_mask)
{
    sid_error_t ret = SID_ERROR_NONE;
    struct sid_handle *sid_handle = NULL;
    config->link_mask = link_mask;

    if (context->sidewalk_handle != NULL) {
        ret = sid_deinit(context->sidewalk_handle);
        if (ret != SID_ERROR_NONE) {
            TI_LOG_ERROR("failed to deinitialize sidewalk link_mask:%x, err:%d", link_mask, (int)ret);
            ITM_flushModule();
            goto error;
        }
    }

    // Initialize sidewalk
    ret = sid_init(config, &sid_handle);
    if (ret != SID_ERROR_NONE) {
        TI_LOG_ERROR("failed to initialize sidewalk link_mask:%x, err:%d", link_mask, (int)ret);
        ITM_flushModule();
        goto error;
    }

    // Register sidewalk handler to the application context
    context->sidewalk_handle = sid_handle;

    // Start the sidewalk stack
    ret = sid_start(sid_handle, link_mask);
    if (ret != SID_ERROR_NONE) {
        TI_LOG_ERROR("failed to start sidewalk, link_mask:%x, err:%d", link_mask, (int)ret);
        ITM_flushModule();
        goto error;
    }
    return 0;

error:
    context->sidewalk_handle = NULL;
    config->link_mask = 0;
    return -1;
}

static void cap_timer_cb(TimerHandle_t xTimer)
{
    if (BUILD_SID_SDK_LINK_TYPE == 4) {
        if (g_app_context.sidewalk_state == STATE_SIDEWALK_INIT ||
            g_app_context.sidewalk_state == STATE_SIDEWALK_NOT_READY) {
            return;
        }
    }

    TickType_t delay = pdMS_TO_TICKS(DEMO_CAPABILITY_PERIOD_MS);
    enum event_type event = EVENT_NOTIFICATION_TIMER_FIRED;

    if (BUILD_SID_SDK_LINK_TYPE == 1) {
        if (g_app_context.sidewalk_state != STATE_SIDEWALK_READY) {
            delay = pdMS_TO_TICKS(CONNECT_LINK_TYPE_1_DELAY_MS);
            event = EVENT_CONNECT_LINK_TYPE_1;
        }
    }
    if (g_app_context.app_state == DEMO_APP_STATE_NOTIFY_SENSOR_DATA && g_app_context.sidewalk_state == STATE_SIDEWALK_READY) {
        delay = pdMS_TO_TICKS(DEMO_NOTIFY_SENSOR_DATA_PERIOD_MS);
    }
    BaseType_t ret = xTimerChangePeriod(g_app_context.cap_timer_handle, delay, 0);
    ASSERT(ret == pdPASS)

    queue_event(g_app_context.event_queue, event, NULL, true);
}

static void button_press_timer_cb(TimerHandle_t xTimer)
{
    queue_event(g_event_queue, EVENT_BUTTON_PRESS_TIMER_FIRED, NULL, true);
}

static void device_profile_timer_cb(TimerHandle_t xTimer)
{
    queue_event(g_event_queue, EVENT_SET_DEVICE_PROFILE_LINK_TYPE_2, NULL, true);
}

static void turn_on_leds(enum demo_app_led_id id)
{
    if (id >= DEMO_APP_LED_ID_LAST) {
        TI_LOG_ERROR("Invalid LED id: %d", id);
        return;
    }
    LED_setOn(g_app_context.led_handles[id], 100);
}

static void turn_on_all_leds()
{
    turn_on_leds(DEMO_APP_LED_ID_0);
    turn_on_leds(DEMO_APP_LED_ID_1);
}

static void turn_off_leds(enum demo_app_led_id id)
{
    if (id >= DEMO_APP_LED_ID_LAST) {
        TI_LOG_ERROR("Invalid LED id: %d", id);
        return;
    }
    LED_setOff(g_app_context.led_handles[id]);
}

static void turn_off_all_leds()
{
    turn_off_leds(DEMO_APP_LED_ID_0);
    turn_off_leds(DEMO_APP_LED_ID_1);
}

static bool is_led_on(enum demo_app_led_id id)
{
    bool ret = true;
    if (id >= DEMO_APP_LED_ID_LAST) {
        TI_LOG_ERROR("Invalid LED id: %d", id);
        return false;
    }
    LED_State state = LED_getState(g_app_context.led_handles[id]);
    ret = (state == LED_STATE_ON) ? true : false;

    return ret;
}

static void process_action_response(struct sid_parse_state *state)
{
    uint8_t temp_button_id_arr[DEMO_BUTTONS_MAX] = {0};
    struct sid_demo_action_resp action_resp = {0};

    action_resp.button_action_resp.button_id_arr = temp_button_id_arr;
    sid_demo_app_action_resp_deserialize(state, &action_resp);
    if (state->ret_code != SID_ERROR_NONE) {
        TI_LOG_ERROR("de-serialize action resp failed %d", state->ret_code);
    } else if (action_resp.resp_type == SID_DEMO_ACTION_TYPE_BUTTON) {
        if (action_resp.button_action_resp.num_buttons == 0xFF) {
            g_app_context.button_press_mask = 0;
            g_app_context.button_notify_mask = 0;
            for (size_t i = 0; i < DEMO_BUTTONS_MAX; i++) {
                g_app_context.button_press_time_in_sec_id_arr[i] = 0;
            }
        } else if (action_resp.button_action_resp.num_buttons <= DEMO_BUTTONS_MAX) {
            for (size_t i = 0; i < action_resp.button_action_resp.num_buttons; i++) {
                g_app_context.button_press_mask &= ~(1 << action_resp.button_action_resp.button_id_arr[i]);
                g_app_context.button_notify_mask &= ~(1 << action_resp.button_action_resp.button_id_arr[i]);
                g_app_context.button_press_time_in_sec_id_arr[i] = 0;
            }
        } else {
            TI_LOG_ERROR("Invalid number of button Max allowed %d received %d",
                    DEMO_BUTTONS_MAX, action_resp.button_action_resp.num_buttons);
        }
    } else {
       TI_LOG_ERROR("Invalid response received %d", action_resp.resp_type);
    }
}

static void process_turn_all_leds(enum sid_demo_led_action led_action_req, struct sid_demo_action_resp *action_resp)
{
    if (led_action_req == SID_DEMO_ACTION_LED_ON) {
        turn_on_all_leds();
    } else {
        turn_off_all_leds();
    }

    action_resp->led_action_resp.action_resp = led_action_req;

    for (size_t i = 0; i < DEMO_LEDS_MAX; i++) {
        bool result = (led_action_req == SID_DEMO_ACTION_LED_ON) ? is_led_on(g_app_context.led_id_arr[i]) :
                       !is_led_on(g_app_context.led_id_arr[i]);
        if (result) {
            action_resp->led_action_resp.num_leds += 1;
            action_resp->led_action_resp.led_id_arr[i] = g_app_context.led_id_arr[i];
        }
    }

    if (action_resp->led_action_resp.num_leds) {
        demo_app_create_led_response(action_resp);
    } else {
        TI_LOG_ERROR("LED response invalid num leds %d", action_resp->led_action_resp.num_leds);
    }
}

static void process_turn_leds(struct sid_demo_led_action_req *led_req, struct sid_demo_action_resp *action_resp)
{
    for (size_t i = 0; i < led_req->num_leds; i++) {
        if (led_req->action_req == SID_DEMO_ACTION_LED_ON) {
            turn_on_leds(led_req->led_id_arr[i]);
        } else {
            turn_off_leds(led_req->led_id_arr[i]);
        }
    }
    action_resp->led_action_resp.action_resp = led_req->action_req;
    for (size_t i = 0; i < led_req->num_leds; i++) {
        bool result = (led_req->action_req == SID_DEMO_ACTION_LED_ON) ? is_led_on(led_req->led_id_arr[i]) :
                       !is_led_on(led_req->led_id_arr[i]);
        if (result) {
            action_resp->led_action_resp.num_leds += 1;
            action_resp->led_action_resp.led_id_arr[i] = led_req->led_id_arr[i];
        }
    }

    if (action_resp->led_action_resp.num_leds) {
        demo_app_create_led_response(action_resp);
    } else {
        TI_LOG_ERROR("LED response invalid num leds %d", action_resp->led_action_resp.num_leds);
    }
}

static void process_action_request(struct sid_parse_state *state)
{
    uint8_t temp_led_id_arr[DEMO_LEDS_MAX] = {0};
    struct sid_demo_led_action_req led_req = {.led_id_arr = temp_led_id_arr};

    sid_demo_app_action_req_deserialize(state, &led_req);
    if (state->ret_code != SID_ERROR_NONE) {
        TI_LOG_ERROR("de-serialize led action req failed %d", state->ret_code);
    } else if (led_req.action_req == SID_DEMO_ACTION_LED_ON || led_req.action_req == SID_DEMO_ACTION_LED_OFF) {
        uint8_t temp_led_id_arr_resp[DEMO_LEDS_MAX] = {0};
        struct sid_demo_action_resp action_resp = {.resp_type = SID_DEMO_ACTION_TYPE_LED,};
        action_resp.led_action_resp.led_id_arr = temp_led_id_arr_resp;
        struct sid_timespec curr_time = {0};
        sid_get_time(g_app_context.sidewalk_handle, SID_GET_GPS_TIME, &curr_time);
        action_resp.gps_time_in_seconds = curr_time.tv_sec;
        if ((curr_time.tv_sec - led_req.gps_time_in_seconds) > 0) {
            action_resp.down_link_latency_secs = curr_time.tv_sec - led_req.gps_time_in_seconds;
        }
        if (led_req.num_leds == 0xFF) {
            process_turn_all_leds(led_req.action_req, &action_resp);
        } else if (led_req.num_leds <= DEMO_LEDS_MAX) {
            process_turn_leds(&led_req, &action_resp);
        } else {
            TI_LOG_ERROR("Invalid led action req max allowed %d received  %d", DEMO_LEDS_MAX, led_req.num_leds);
        }
    } else {
        TI_LOG_ERROR("Invalid led action request %d", led_req.action_req);
    }
}

static void receive_task(void *context)
{
    receive_context_t *rcv_context = (receive_context_t *)context;
    while (1) {
        ITM_flushModule();
        struct app_demo_rx_msg rx_msg;

        if (xQueueReceive(rcv_context->receive_event_queue, &rx_msg, portMAX_DELAY) == pdTRUE) {
            struct sid_demo_msg_desc msg_desc = {0};
            static uint8_t temp_msg_payload[PAYLOAD_MAX_SIZE];
            memset(temp_msg_payload, 0, sizeof(temp_msg_payload));
            struct sid_demo_msg msg = {.payload = temp_msg_payload};
            static struct sid_parse_state state = {0};

            sid_parse_state_init(&state, rx_msg.rx_payload, rx_msg.pld_size);
            sid_demo_app_msg_deserialize(&state, &msg_desc, &msg);
            TI_LOG_INFO("opc %d, class %d cmd %d status indicator %d status_code %d paylaod size %d",
                    msg_desc.opc, msg_desc.cmd_class, msg_desc.cmd_id, msg_desc.status_hdr_ind,
                    msg_desc.status_code,  msg.payload_size);

            if (state.ret_code != SID_ERROR_NONE) {
                TI_LOG_ERROR("de-serialize demo app msg failed %d", state.ret_code);
            } else if (msg_desc.status_hdr_ind && msg_desc.opc == SID_DEMO_MSG_TYPE_RESP &&
                    msg_desc.cmd_class == SID_DEMO_APP_CLASS && msg_desc.cmd_id == SID_DEMO_APP_CLASS_CMD_CAP_DISCOVERY_ID
                    && msg_desc.status_code == SID_ERROR_NONE && msg.payload_size == 0) {
                TI_LOG_INFO("Capability response received");
                g_app_context.app_state = DEMO_APP_STATE_NOTIFY_SENSOR_DATA;
            } else if (msg_desc.status_hdr_ind && msg_desc.opc == SID_DEMO_MSG_TYPE_RESP &&
                    msg_desc.cmd_class == SID_DEMO_APP_CLASS && msg_desc.cmd_id == SID_DEMO_APP_CLASS_CMD_ACTION
                    && msg_desc.status_code == SID_ERROR_NONE) {
                TI_LOG_INFO("Action response received");
                sid_parse_state_init(&state, msg.payload, msg.payload_size);
                process_action_response(&state);
            } else if (msg_desc.opc == SID_DEMO_MSG_TYPE_WRITE && msg_desc.cmd_class == SID_DEMO_APP_CLASS && msg_desc.cmd_id == SID_DEMO_APP_CLASS_CMD_ACTION) {
                TI_LOG_INFO("Action request received");
                sid_parse_state_init(&state, msg.payload, msg.payload_size);
                process_action_request(&state);
            }
        }
    }
}

static void demo_app_check_button_press_notify(void)
{
    struct sid_timespec curr_time = {0};
    uint16_t next_timer_schedule_secs = 0;
    sid_get_time(g_app_context.sidewalk_handle, SID_GET_GPS_TIME, &curr_time);
    for (size_t i = 0; i < DEMO_BUTTONS_MAX; i++) {
        if ((g_app_context.button_notify_mask & (1 << g_app_context.button_id_arr[i])) &&
                g_app_context.button_press_time_in_sec_id_arr[i] != 0) {
            next_timer_schedule_secs = curr_time.tv_sec - g_app_context.button_press_time_in_sec_id_arr[i];
            TI_LOG_INFO("Button press timeout pre check: button_notify_mask %x next_timer_schedule_secs %d",
                    g_app_context.button_notify_mask, next_timer_schedule_secs);
            if (next_timer_schedule_secs >= BUTTON_PRESS_CHECK_PERIOD_SECS) {
                g_app_context.button_notify_mask &= ~(1 << g_app_context.button_id_arr[i]);
                g_app_context.button_press_mask &= ~(1 << g_app_context.button_id_arr[i]);
                next_timer_schedule_secs = 0;
                g_app_context.button_press_time_in_sec_id_arr[i] = 0;
            } else if (next_timer_schedule_secs > (curr_time.tv_sec - g_app_context.button_press_time_in_sec_id_arr[i])) {
                next_timer_schedule_secs = curr_time.tv_sec - g_app_context.button_press_time_in_sec_id_arr[i];
            }
        } else if (g_app_context.button_press_time_in_sec_id_arr[i]) {
            g_app_context.button_press_time_in_sec_id_arr[i] = 0;
        }
    }
    TI_LOG_INFO("Button press timeout post check: button_notify_mask %x next_timer_schedule_secs %d",
            g_app_context.button_notify_mask, next_timer_schedule_secs);
    ITM_flushModule();
    if (next_timer_schedule_secs) {
        BaseType_t ret = xTimerChangePeriod(g_app_context.button_press_timer_handle, pdMS_TO_TICKS(next_timer_schedule_secs * 1000), 0);
        ASSERT(ret == pdPASS)
    }
}

static void initialize_pals(void)
{
    const sid_pal_mfg_store_region_t ti_mfg = {
        .addr_start = MANUFACTURE_FLASH_START,
        .addr_end   = MANUFACTURE_FLASH_END,
    };

    sid_pal_mfg_store_init(ti_mfg);

    sid_error_t ret_code = sid_pal_storage_kv_init();
    if (ret_code != SID_ERROR_NONE) {
        SID_PAL_LOG_INFO("Sidewalk KV store init failed err: %d", ret_code);
        ITM_flushModule();
    }
    assert(ret_code == SID_ERROR_NONE);

    ret_code = sid_pal_crypto_init();
    if (ret_code != SID_ERROR_NONE) {
        SID_PAL_LOG_INFO("Sidewalk Init Crypto PAL err: %d", ret_code);
        ITM_flushModule();
    }
    assert(ret_code == SID_ERROR_NONE);
}

static void main_task(void *context)
{
    app_context_t *app_context = (app_context_t *)context;

    initialize_pals();

    struct sid_event_callbacks event_callbacks = {
        .context = app_context,
        .on_event = on_sidewalk_event, /* Called from ISR context */
        .on_msg_received = on_sidewalk_msg_received, /* Called from sid_process() */
        .on_msg_sent = on_sidewalk_msg_sent,  /* Called from sid_process() */
        .on_send_error = on_sidewalk_send_error, /* Called from sid_process() */
        .on_status_changed = on_sidewalk_status_changed, /* Called from sid_process() */
        .on_factory_reset = on_sidewalk_factory_reset, /* Called from sid_process */
    };

    struct sid_config config = {
        .link_mask = 0,
        .time_sync_periodicity_seconds = 7200,
        .callbacks = &event_callbacks,
        .link_config = app_get_ble_config(),
#if SID_SDK_CONFIG_ENABLE_LINK_TYPE_2 == 1
        .sub_ghz_link_config = app_get_sub_ghz_config(),
#endif
    };

#if SID_SDK_CONFIG_ENABLE_LINK_TYPE_2 == 1
    set_radio_cc13xx_device_config(&radio_cc13xx_default_cfg);
#endif

    struct sid_handle *sid_handle = NULL;
    uint32_t link_mask = 0;
    _Static_assert(BUILD_SID_SDK_LINK_TYPE == 1 || BUILD_SID_SDK_LINK_TYPE == 2 || BUILD_SID_SDK_LINK_TYPE == 4, "BUILD_SID_SDK_LINK_TYPE not supported");
    if (BUILD_SID_SDK_LINK_TYPE == 1) {
        link_mask = SID_LINK_TYPE_1;
    } else if (BUILD_SID_SDK_LINK_TYPE == 2) {
        link_mask = SID_LINK_TYPE_2;
    } else if (BUILD_SID_SDK_LINK_TYPE == 4) {
        link_mask = SID_LINK_TYPE_1;
    }
    if (init_and_start_link(app_context, &config, link_mask) != 0) {
        goto error;
    }
    ITM_flushModule();
    sid_handle = app_context->sidewalk_handle;
    app_context->cap_timer_handle = xTimerCreate("demo_cap_timer",
                                                  pdMS_TO_TICKS(DEMO_CAPABILITY_PERIOD_MS),
                                                  pdFALSE, (void *)0, cap_timer_cb);

    if (app_context->cap_timer_handle == NULL) {
        TI_LOG_ERROR("create capability timer failed!");
        goto error;
    }

    app_context->button_press_timer_handle = xTimerCreate("button_press_timer",
                                              pdMS_TO_TICKS(BUTTON_PRESS_CHECK_PERIOD_SECS * 1000),
                                              pdFALSE, (void *)0, button_press_timer_cb);

    if (app_context->button_press_timer_handle == NULL) {
        TI_LOG_ERROR("create button press timer failed!");
        goto error;
    }

    if (BUILD_SID_SDK_LINK_TYPE == 2 || BUILD_SID_SDK_LINK_TYPE == 4) {
        app_context->device_profile_timer_handle = xTimerCreate("device_profile_timer",
                                                      pdMS_TO_TICKS(PROFILE_CHECK_TIMER_DELAY_MS),
                                                      pdFALSE, (void *)0, device_profile_timer_cb);

        if (app_context->device_profile_timer_handle == NULL) {
            TI_LOG_ERROR("create device profile timer failed!");
            goto error;
        }
    }
    app_context->sidewalk_state = STATE_SIDEWALK_NOT_READY;

    while (1) {
        struct demo_app_event event;
        ITM_flushModule();

        if (xQueueReceive(app_context->event_queue, &event, portMAX_DELAY) == pdTRUE) {
            switch (event.type) {
                case EVENT_TYPE_SIDEWALK:
                    sid_process(sid_handle);
                    break;
                case EVENT_FACTORY_RESET:
                    factory_reset();
                    break;
                case EVENT_BUTTON_PRESS:
                    demo_app_notify_sensor_data(true);
                    g_app_context.button_event_pending_processing = false;
                    break;
                case EVENT_NOTIFICATION_TIMER_FIRED:
                    if (g_app_context.app_state == DEMO_APP_STATE_NOTIFY_CAPABILITY) {
                        if (BUILD_SID_SDK_LINK_TYPE == 4) {
                            if (((g_app_context.link_status.link_mask & SID_LINK_TYPE_2) == 0) &&
                                (g_app_context.link_status.time_sync_status == SID_STATUS_TIME_SYNCED)) {
                                    TI_LOG_INFO("Switching link to FSK 900MHz");
                                    link_mask = SID_LINK_TYPE_2;
                                    if (init_and_start_link(app_context, &config, link_mask) != 0) {
                                        goto error;
                                    }
                            }
                        }
                        demo_app_notify_capability();
                    } else if (g_app_context.app_state == DEMO_APP_STATE_NOTIFY_SENSOR_DATA) {
                        demo_app_notify_sensor_data(false);
                    }
                    break;
                case EVENT_BUTTON_PRESS_TIMER_FIRED:
                    TI_LOG_INFO("Button press timeout check timer fired");
                    demo_app_check_button_press_notify();
                    break;
                case EVENT_CONNECT_LINK_TYPE_1:
                    if (BUILD_SID_SDK_LINK_TYPE == 1 || BUILD_SID_SDK_LINK_TYPE == 4) {
                        if (link_mask == SID_LINK_TYPE_1) {
                            TI_LOG_INFO("Connecting link type 1");
                            sid_error_t ret = sid_ble_bcn_connection_request(sid_handle, true);
                            if (ret != SID_ERROR_NONE) {
                                TI_LOG_ERROR("Failed to set connect request on link type 1 %d", ret);
                            }
                        }
                    }
                    break;
                case EVENT_SET_DEVICE_PROFILE_LINK_TYPE_2:
                    if (BUILD_SID_SDK_LINK_TYPE == 2 || BUILD_SID_SDK_LINK_TYPE == 4) {
                        BaseType_t err = xTimerIsTimerActive(g_app_context.device_profile_timer_handle);
                        if (err == pdTRUE) {
                            err = xTimerStop(g_app_context.device_profile_timer_handle, 0);
                            ASSERT(err == pdPASS);
                        }
                        struct sid_device_profile curr_dev_cfg = {
                            .unicast_params = {
                                .device_profile_id = SID_LINK2_PROFILE_2,
                            },
                        };
                        sid_error_t ret = sid_option(sid_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE, &curr_dev_cfg, sizeof(curr_dev_cfg));
                        if (ret == SID_ERROR_NONE) {
                            struct sid_device_profile target_dev_cfg = {
                                .unicast_params = {
                                    .device_profile_id = SID_LINK2_PROFILE_2,
                                    .rx_window_count = SID_RX_WINDOW_CNT_INFINITE,
                                    .unicast_window_interval = {
                                        .sync_rx_interval_ms = SID_LINK2_RX_WINDOW_SEPARATION_1,
                                    },
                                    .wakeup_type = SID_TX_AND_RX_WAKEUP,
                                },
                            };
                            if (!memcmp(&curr_dev_cfg, &target_dev_cfg, sizeof(curr_dev_cfg))) {
                                TI_LOG_INFO("Device profile for Link type 2 already set");
                            } else {
                                ret = sid_option(sid_handle, SID_OPTION_900MHZ_SET_DEVICE_PROFILE, &target_dev_cfg, sizeof(target_dev_cfg));
                                if (ret != SID_ERROR_NONE) {
                                    TI_LOG_ERROR("Device profile configuration for Link type 2 failed ret = %d", ret);
                                    err = xTimerChangePeriod(app_context->device_profile_timer_handle, pdMS_TO_TICKS(PROFILE_CHECK_TIMER_DELAY_MS), 0);
                                    ASSERT(err == pdPASS);
                                } else {
                                    TI_LOG_INFO("Device profile Link type 2 set success");
                                }
                            }
                        } else {
                            TI_LOG_ERROR("Failed to get device profile configuration for Link type 2 ret = %d", ret);
                            err = xTimerChangePeriod(app_context->device_profile_timer_handle, pdMS_TO_TICKS(PROFILE_CHECK_TIMER_DELAY_MS), 0);
                            ASSERT(err == pdPASS);
                        }
                    }
                case EVENT_SEND_MESSAGE:
                    if (event.data) {
                        struct app_demo_tx_msg *evt_data = (struct app_demo_tx_msg*)event.data;
                        struct sid_msg msg = {.size = evt_data->pld_size, .data = evt_data->tx_payload};

                        send_msg(&evt_data->desc, &msg);
                    } else {
                        TI_LOG_ERROR("Invalid data pointer in event");
                    }
                    break;
                default:
                    TI_LOG_ERROR("Invalid event queued %d", event);
                    break;
            }
            if (event.data != NULL) {
                free(event.data);
            }
        }
    }

error:
    if (sid_handle != NULL) {
        sid_stop(sid_handle, config.link_mask);
        sid_deinit(sid_handle);
        app_context->sidewalk_handle = NULL;
    }
    vTaskDelete(NULL);
}

void button1_handler(Button_Handle handle, Button_EventMask events)
{
    if (g_app_context.app_state != DEMO_APP_STATE_NOTIFY_SENSOR_DATA) {
        return;
    }
    if (events & Button_EV_CLICKED) {
        bool notify_event = false;
        if (!(g_app_context.button_press_mask & (1 << g_app_context.button_id_arr[DEMO_APP_BUTTON_ID_0]))) {
            g_app_context.button_press_mask |= (1 << g_app_context.button_id_arr[DEMO_APP_BUTTON_ID_0]);
            notify_event = true;
        }
        if (!g_app_context.button_event_pending_processing && notify_event) {
            queue_event(g_event_queue, EVENT_BUTTON_PRESS, NULL, true);
            g_app_context.button_event_pending_processing = true;
        }
    }
}

void button2_handler(Button_Handle handle, Button_EventMask events)
{
    if (events & Button_EV_LONGPRESSED) {
         queue_event(g_event_queue, EVENT_FACTORY_RESET, NULL, true);
    } else if (events & Button_EV_CLICKED) {
         queue_event(g_event_queue, EVENT_CONNECT_LINK_TYPE_1, NULL, true);
    }
}

int main(void)
{
    Board_init();
    Button_init();
    Temperature_init();

    const ITM_config itm_config = {
        .system_clock = 48000000,
        .baud_rate = ITM_921600,
        .defer_logging = true,
    };
    ITM_initModule(&itm_config);

    TI_LOG_INFO("Sidewalk example started");
    ITM_flushModule();

    Button_Params button_params;
    Button_Params_init(&button_params);
    button_params.buttonCallback = button1_handler;
    g_app_context.button_1 = Button_open(CONFIG_BUTTON_0, &button_params);
    assert(g_app_context.button_1 != NULL);

    button_params.buttonCallback = button2_handler;
    g_app_context.button_2 = Button_open(CONFIG_BUTTON_1, &button_params);
    assert(g_app_context.button_2 != NULL);

    LED_Params led_params;
    LED_Params_init(&led_params);
    g_app_context.led_handles[DEMO_APP_LED_ID_0] = LED_open(CONFIG_LED_0, &led_params);
    assert(g_app_context.led_handles[DEMO_APP_LED_ID_0] != NULL);

    g_app_context.led_handles[DEMO_APP_LED_ID_1] = LED_open(CONFIG_LED_1, &led_params);
    assert(g_app_context.led_handles[DEMO_APP_LED_ID_1] != NULL);

    g_event_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(struct demo_app_event));
    if (g_event_queue == NULL) {
        TI_LOG_ERROR("failed creating queue");
        assert(false);
    }

    g_app_context.sidewalk_state = STATE_SIDEWALK_INIT;
    g_app_context.app_state = DEMO_APP_STATE_INIT;
    g_app_context.link_status.time_sync_status = SID_STATUS_NO_TIME;

    for (size_t i = 0; i < DEMO_LEDS_MAX; i++) {
        g_app_context.led_id_arr[i] = i;
    }

    for (size_t i = 0; i < DEMO_BUTTONS_MAX; i++) {
        g_app_context.button_id_arr[i] = i;
    }

    g_app_context.event_queue = g_event_queue;

    if (pdPASS != xTaskCreate(main_task, "MAIN", MAIN_TASK_STACK_SIZE, &g_app_context, 1, &g_app_context.main_task)) {
        TI_LOG_ERROR("failed creating main task");
        assert(false);
    }

    g_receieve_event_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(struct app_demo_rx_msg));
    if (g_event_queue == NULL) {
        assert(false);
    }

    g_receive_context.receive_event_queue = g_receieve_event_queue;

    if (pdPASS != xTaskCreate(receive_task, "RECEIVE", RECEIVE_TASK_STACK_SIZE, &g_receive_context, 2, &g_receive_context.receive_task)) {
        assert(false);
    }

    vTaskStartScheduler();

    for (;;) {
        assert(false);
    }
}
