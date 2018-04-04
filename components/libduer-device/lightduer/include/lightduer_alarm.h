// Copyright (2017) Baidu Inc. All rights reserveed.
/**
 * File: lightduer_alarm.h
 * Auth: Gang Chen (chengang12@baidu.com)
 * Desc: Light duer alarm APIs.
 */

#ifndef BAIDU_DUER_LIGHTDUER_INCLUDE_LIGHTDUER_ALARM_H
#define BAIDU_DUER_LIGHTDUER_INCLUDE_LIGHTDUER_ALARM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "baidu_json.h"

// It means the id doesn't needed to report the cloud
#define ALARM_DUMMY_TOPIC_ID -1

typedef enum {
    SET_ALERT_SUCCESS,
    SET_ALERT_FAIL,
    DELETE_ALERT_SUCCESS,
    DELETE_ALERT_FAIL,
    ALERT_START,
    ALERT_STOP,
} alarm_event_type;

typedef struct {
    char *type;    // alarm or timer
    char *time;    // the time stamp of the alarm, it's the seconds from 1970-01-01 to now
    char *token;   // the alarm token, it is the unique identification of an alarm
    char *msg_id;  // it is needed when report alarm event
    char *url;     // the alerting tone of the alarm
} alarm_info_type;

/**
 * The callback to set alarm
 * PARAM:
 * @param[in] alarm_info: the alarm information
 * @param[in] id: the id should be used when report set alarm success/failed event
 * @return: 0 when success, negative value when failed.
 */
typedef int (*duer_set_alarm_func)(const alarm_info_type *alarm_info, int id);

/**
 * The callback to delete alarm
 * PARAM:
 * @param[in] token: the alarm token, it is the unique identification of an alarm
 * @param[in] msg_id: the message_id should be used when report delete alarm success/failed event
 * @param[in] id: the id should be used when report delete success/failed event
 * @return: 0 when success, negative value when failed.
 */
typedef int (*duer_delete_alarm_func)(const char *token, const char *msg_id, int id);

/**
 * The callback to get all alarm information
 * PARAM:
 * @param[out] alert_list: the json object used to storage all alert infomation
 * @return: 0 when success, negative value when failed.
 */
typedef int (*duer_get_alarm_list)(baidu_json *alert_list);

/**
 * Report alarm event
 * PARAM:
 * @param[in] type: event type, refer to alarm_event_type.
 * @param[in] alarm_info: the alarm information
 * @param[in] msg_id: the message_id which is included in set/delete alarm directive
 * @param[in] id: the id should be set when report set/delete alarm success/failed event,
 *                else it should be 0
 * @return: 0 when success, negative value when failed.
 */
int duer_report_alarm_event(const char *token, const char *msg_id, alarm_event_type type, int id);

/**
 * Add a alarm info to the alarm list, it should be used in duer_get_alarm_list callback.
 * PARAM:
 * @param[in] alert_list: the json object used to storage all alert infomation
 * @param[in] alarm_info: the alarm information
 * @param[in] is_active: the alarm is soulding or not
 * @return: 0 when success, negative value when failed.
 */
int duer_insert_alarm_list(baidu_json *alert_list, alarm_info_type *alarm_info, bool is_active);

/**
 * Initalize alarm function
 * PARAM:
 * @param[in] set_alarm_cb: callback to set alarm
 * @param[in] delete_alarm_cb: callback to delete alarm
 * @return: none
 */
void duer_alarm_initialize(duer_set_alarm_func set_alarm_cb,
                           duer_delete_alarm_func delete_alarm_cb,
                           duer_get_alarm_list get_alarm_list_cb);

#ifdef __cplusplus
}
#endif

#endif/*BAIDU_DUER_LIGHTDUER_INCLUDE_LIGHTDUER_ALARM_H*/

