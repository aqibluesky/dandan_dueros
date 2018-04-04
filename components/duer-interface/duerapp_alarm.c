// Copyright (2017) Baidu Inc. All rights reserved.
/**
 * File: duerapp_alarm.c
 * Auth: Gang Chen(chengang12@baidu.com)
 * Desc: Duer alarm functions.
 */

#include "duerapp_alarm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lightduer_alarm.h"
#include "duerapp_config.h"
#include "lightduer_net_ntp.h"
#include "lightduer_types.h"
#include "lightduer_timers.h"
#include "lightduer_mutex.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

typedef enum duerapp_alarm_msg_type {
    DUER_SET_ALARM,
    DUER_ALARM_STARTED,
} duerapp_alarm_msg_e;

typedef struct {
    alarm_info_type alarm_info;
    duer_timer_handler handle;
    bool is_active;
    int id;
} duerapp_alarm_data;

typedef struct _duer_alarm_list_node {
    struct _duer_alarm_list_node *next;
    duerapp_alarm_data *data;
} duer_alarm_list_node_t;

typedef struct {
    duerapp_alarm_data *data;
    duerapp_alarm_msg_e type;
} duerapp_alarm_msg_t;

static duer_alarm_list_node_t s_alarm_list;
static const int ALARM_QUEUE_SIZE = 10;
static const int ALARM_TASK_STACK_SIZE = 1024 * 4;
static xQueueHandle s_message_q;
static xTaskHandle s_alarm_task;
static duer_mutex_t s_alarm_mutex;
static const int QUEUE_WAIT_FOREVER = 0x7fffffff;

static duer_status_t duer_alarm_list_push(duerapp_alarm_data *data)
{
    duer_status_t rt = DUER_OK;
    duer_alarm_list_node_t *new_node = NULL;
    duer_alarm_list_node_t *tail = &s_alarm_list;

    new_node = (duer_alarm_list_node_t *)malloc(sizeof(duer_alarm_list_node_t));
    if (!new_node) {
        DUER_LOGE("Memory too low");
        rt = DUER_ERR_MEMORY_OVERLOW;
        goto error_out;
    }
    new_node->next = NULL;
    new_node->data = data;

    while (tail->next) {
        tail = tail->next;
    }

    tail->next = new_node;

error_out:
    return rt;
}

static duer_status_t duer_alarm_list_remove(duerapp_alarm_data *data)
{
    duer_alarm_list_node_t *pre = &s_alarm_list;
    duer_alarm_list_node_t *cur = NULL;
    duer_status_t rt = DUER_ERR_FAILED;

    while (pre->next) {
        cur = pre->next;
        if (cur->data == data) {
            pre->next = cur->next;
            free(cur);
            rt = DUER_OK;
            break;
        }
        pre = pre->next;
    }

    return rt;
}

static char *duerapp_alarm_strdup(const char *str)
{
    int len = 0;
    char *dest = NULL;

    if (!str) {
        return NULL;
    }

    len = strlen(str);
    dest = (char *)malloc(len + 1);
    if (!dest) {
        return NULL;
    }

    snprintf(dest, len + 1, "%s", str);
    return dest;
}

static void duer_free_alarm_data(duerapp_alarm_data *alarm)
{
    if (alarm) {
        if (alarm->alarm_info.type) {
            free(alarm->alarm_info.type);
            alarm->alarm_info.type = NULL;
        }

        if (alarm->alarm_info.time) {
            free(alarm->alarm_info.time);
            alarm->alarm_info.time = NULL;
        }

        if (alarm->alarm_info.token) {
            free(alarm->alarm_info.token);
            alarm->alarm_info.token = NULL;
        }

        if (alarm->alarm_info.msg_id) {
            free(alarm->alarm_info.msg_id);
            alarm->alarm_info.msg_id = NULL;
        }

        if (alarm->alarm_info.url) {
            free(alarm->alarm_info.url);
            alarm->alarm_info.url = NULL;
        }

        if (alarm->handle) {
            duer_timer_release(alarm->handle);
            alarm->handle = NULL;
        }

        free(alarm);
    }
}

static duerapp_alarm_data *duer_create_alarm_data(const alarm_info_type *alarm_info,
                                                  duer_timer_handler handle,
                                                  int id)
{
    duerapp_alarm_data *alarm = NULL;

    alarm = (duerapp_alarm_data *)malloc(sizeof(duerapp_alarm_data));
    if (!alarm) {
        goto error_out;
    }

    memset(alarm, 0, sizeof(duerapp_alarm_data));

    alarm->alarm_info.type = duerapp_alarm_strdup(alarm_info->type);
    if (!alarm->alarm_info.type) {
        goto error_out;
    }

    alarm->alarm_info.time = duerapp_alarm_strdup(alarm_info->time);
    if (!alarm->alarm_info.time) {
        goto error_out;
    }

    alarm->alarm_info.token = duerapp_alarm_strdup(alarm_info->token);
    if (!alarm->alarm_info.token) {
        goto error_out;
    }

    alarm->alarm_info.msg_id = duerapp_alarm_strdup(alarm_info->msg_id);
    if (!alarm->alarm_info.msg_id) {
        goto error_out;
    }

    alarm->alarm_info.url = duerapp_alarm_strdup(alarm_info->url);
    if (!alarm->alarm_info.url) {
        goto error_out;
    }

    alarm->handle = handle;
    alarm->id = id;

    return alarm;

error_out:
    DUER_LOGE("Memory too low");
    duer_free_alarm_data(alarm);

    return NULL;
}

static void duer_alarm_callback(void *param)
{
    duerapp_alarm_msg_t msg;

    msg.data = (duerapp_alarm_data *)param;
    msg.type = DUER_ALARM_STARTED;
    xQueueSend(s_message_q, &msg, 0);
}

static duer_status_t duer_alarm_set(const alarm_info_type *alarm_info, int id)
{
    duer_status_t rs = DUER_OK;
    duerapp_alarm_data* alarm = NULL;
    duerapp_alarm_msg_t msg;

    DUER_LOGI("set alarm: scheduled_time: %s, token: %s\n", alarm_info->time, alarm_info->token);

    alarm = duer_create_alarm_data(alarm_info, NULL, id);
    if (!alarm) {
        duer_report_alarm_event(alarm_info->token, alarm_info->msg_id, SET_ALERT_FAIL, id);
        rs = DUER_ERR_FAILED;
    } else {
        // create alarm in duer_alarm_task and return immediately
        msg.data = alarm;
        msg.type = DUER_SET_ALARM;
        xQueueSend(s_message_q, &msg, 0);
    }

    return rs;
}

static duerapp_alarm_data *duer_find_target_alarm(const char *token)
{
    duer_alarm_list_node_t *node = s_alarm_list.next;

    while (node) {
        if (node->data) {
            if (strcmp(token, node->data->alarm_info.token) == 0) {
                return node->data;
            }
        }

        node = node->next;
    }

    return NULL;
}

static int duer_report_alarm_list(baidu_json *alert_list)
{
    int rs = DUER_OK;
    duer_alarm_list_node_t *node = s_alarm_list.next;

    while (node) {
        duer_insert_alarm_list(alert_list, &node->data->alarm_info, node->data->is_active);
        node = node->next;
    }

    return rs;
}

/**
 * We use ntp to get current time, it might spend too long time and block the task,
 * hence we use a new task to create alarm.
 */
static void duer_alarm_task(void *param)
{
    DuerTime cur_time;
    size_t delay = 0;
    int rs = DUER_OK;
    duerapp_alarm_data* alarm = NULL;
    uint32_t time = 0;
    duerapp_alarm_msg_t msg;

    while (1) {
        if (pdTRUE != xQueueReceive(s_message_q, &msg, QUEUE_WAIT_FOREVER)) {
            continue;
        }
        alarm = msg.data;

        switch (msg.type) {
        case DUER_SET_ALARM:
            rs = sscanf(alarm->alarm_info.time, "%u", &time);
            if (rs != 1) {
                DUER_LOGE("Failed to get schedulr time\n");
                rs = DUER_ERR_FAILED;
                break;
            }

            rs = duer_ntp_client(NULL, 5000, &cur_time);
            if (rs < 0) {
                DUER_LOGE("Failed to get NTP time\n");
                rs = DUER_ERR_FAILED;
                break;
            }

            if (time <= cur_time.sec) {
                DUER_LOGE("The alarm is expired\n");
                rs = DUER_ERR_FAILED;
                break;
            }

            delay = (time - cur_time.sec) * 1000 - cur_time.usec / 1000;
            alarm->handle = duer_timer_acquire(duer_alarm_callback,
                                               alarm,
                                               DUER_TIMER_ONCE);
            if (!alarm->handle) {
                DUER_LOGE("Failed to create timer\n");
                rs = DUER_ERR_FAILED;
                break;
            }

            rs = duer_timer_start(alarm->handle, delay);
            if (rs != DUER_OK) {
                DUER_LOGE("Failed to start timer\n");
                break;
            }

            /*
             * The alarms is storaged in the ram, hence the alarms will be lost after close the device.
             * You could stoage them into flash or sd card, and restore them after restart the device
             * according to your request.
             */
            duer_mutex_lock(s_alarm_mutex);
            duer_alarm_list_push(alarm);
            duer_report_alarm_event(alarm->alarm_info.token,
                                    alarm->alarm_info.msg_id,
                                    SET_ALERT_SUCCESS,
                                    alarm->id);
            duer_mutex_unlock(s_alarm_mutex);
            rs = DUER_OK;
            break;
        case DUER_ALARM_STARTED:
            duer_mutex_lock(s_alarm_mutex);
            alarm->is_active = true;
            duer_report_alarm_event(alarm->alarm_info.token,
                                    alarm->alarm_info.msg_id,
                                    ALERT_START,
                                    ALARM_DUMMY_TOPIC_ID);
            DUER_LOGI("Alarm started! token: %s", alarm->alarm_info.token);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            DUER_LOGI("Alarm stoped! token: %s", alarm->alarm_info.token);
            duer_report_alarm_event(alarm->alarm_info.token,
                                    alarm->alarm_info.msg_id,
                                    ALERT_STOP,
                                    ALARM_DUMMY_TOPIC_ID);
            alarm->is_active = false;
            duer_mutex_unlock(s_alarm_mutex);
            break;
        default:
            break;
        }

        if ((msg.type == DUER_SET_ALARM) && (rs != DUER_OK)) {
            duer_report_alarm_event(alarm->alarm_info.token,
                                    alarm->alarm_info.msg_id,
                                    SET_ALERT_FAIL,
                                    alarm->id);
            duer_free_alarm_data(alarm);
        }
    }
}

static int duer_alarm_delete(const char *token, const char *msg_id, int id)
{
    duerapp_alarm_data *target_alarm = NULL;

    DUER_LOGI("delete alarm: token %s", token);

    duer_mutex_lock(s_alarm_mutex);

    target_alarm = duer_find_target_alarm(token);
    if (!target_alarm) {
        duer_report_alarm_event(token, msg_id, DELETE_ALERT_FAIL, id);
        duer_mutex_unlock(s_alarm_mutex);
        DUER_LOGE("Cannot find the target alarm\n");
        return DUER_ERR_FAILED;
    }

    duer_alarm_list_remove(target_alarm);
    duer_report_alarm_event(token, msg_id, DELETE_ALERT_SUCCESS, id);

    duer_mutex_unlock(s_alarm_mutex);

    duer_free_alarm_data(target_alarm);

    return DUER_OK;
}

void duer_init_alarm()
{
    static bool is_first_time = true;

    if (is_first_time) {
        is_first_time = false;
        xTaskCreate(&duer_alarm_task, "duer_alarm_task", ALARM_TASK_STACK_SIZE, NULL, 5, &s_alarm_task);
        s_message_q = xQueueCreate(ALARM_QUEUE_SIZE, sizeof(duerapp_alarm_msg_t));
        s_alarm_mutex = duer_mutex_create();
        if (!s_alarm_task || !s_message_q || !s_alarm_mutex) {
            DUER_LOGE("Failed to init alarm");
            return;
        }
    }
    duer_alarm_initialize(duer_alarm_set, duer_alarm_delete, duer_report_alarm_list);
}
