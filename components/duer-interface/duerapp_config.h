// Copyright (2017) Baidu Inc. All rights reserveed.
/**
 * File: duerapp_config.h
 * Auth: Su Hao (suhao@baidu.com)
 * Desc: Duer Configuration.
 */

#ifndef BAIDU_DUER_DUERAPP_DUERAPP_CONFIG_H
#define BAIDU_DUER_DUERAPP_DUERAPP_CONFIG_H


#include "esp_err.h"
#include "esp_log.h"

#define DUER_TAG            "duerapp"
#define DUER_LOGV(...)      ESP_LOGV(DUER_TAG, __VA_ARGS__)
#define DUER_LOGD(...)      ESP_LOGD(DUER_TAG, __VA_ARGS__)
#define DUER_LOGI(...)      ESP_LOGI(DUER_TAG, __VA_ARGS__)
#define DUER_LOGW(...)      ESP_LOGW(DUER_TAG, __VA_ARGS__)
#define DUER_LOGE(...)      ESP_LOGE(DUER_TAG, __VA_ARGS__)


void initialize_wifi(void);

void initialize_sdcard(void);

const char *duer_load_profile(const char *path);

/*
 * Recorder APIS
 */

enum duer_record_events_enum {
    REC_START,
    REC_DATA,
    REC_STOP
};

typedef void (*duer_record_event_func)(int status, const void *data, size_t size);

void duer_record_set_event_callback(duer_record_event_func func);

void duer_record_all();

#endif/*BAIDU_DUER_DUERAPP_DUERAPP_CONFIG_H*/
