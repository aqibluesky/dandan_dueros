#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_log.h"
#include "driver/adc.h"

#include "duerOSService.h"
#include "esp_deep_sleep.h"
#include "driver/rtc_io.h"

#include "lightduer.h"
#include "lightduer_voice.h"
#include "lightduer_net_ntp.h"
#include "duerapp_ctrl.h"
#include "duerapp_alarm.h"

#define RECOG_SERVICE_TAG "RECOG_SERVICE"
#define RECOG_SERVICE_TASK_PRIORITY   10
#define RECOG_SERVICE_TASK_SIZE         7*1024
#define RECORD_SAMPLE_RATE  (16000)
#define ONE_FRAM_SIZE 3*1024
#define duerOS_TRIGGER_URI  "i2s://16000:1@record.pcm#raw"   //TODO: put somewhere else for sharing

char DUER_OS_PROFILE[] ="{\"configures\":\"{}\",\"bindToken\":\"86dfc0efe212753f9055994238f9bf0c\",\"coapPort\":443,\"token\":\"WXUGsCTQtZsBs8VfWjQbEHKAYVVXCbpv\",\"serverAddr\":\"device.iot.baidu.com\",\"lwm2mPort\":443,\"uuid\":\"0f6c000000000a\",\"rsaCaCrt\":\"-----BEGIN CERTIFICATE-----\nMIIDUDCCAjgCCQCmVPUErMYmCjANBgkqhkiG9w0BAQUFADBqMQswCQYDVQQGEwJD\nTjETMBEGA1UECAwKU29tZS1TdGF0ZTEOMAwGA1UECgwFYmFpZHUxGDAWBgNVBAMM\nDyouaW90LmJhaWR1LmNvbTEcMBoGCSqGSIb3DQEJARYNaW90QGJhaWR1LmNvbTAe\nFw0xNjAzMTEwMzMwNDlaFw0yNjAzMDkwMzMwNDlaMGoxCzAJBgNVBAYTAkNOMRMw\nEQYDVQQIDApTb21lLVN0YXRlMQ4wDAYDVQQKDAViYWlkdTEYMBYGA1UEAwwPKi5p\nb3QuYmFpZHUuY29tMRwwGgYJKoZIhvcNAQkBFg1pb3RAYmFpZHUuY29tMIIBIjAN\nBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtbhIeiN7pznzuMwsLKQj2xB02+51\nOvCJ5d116ZFLjecp9qtllqOfN7bm+AJa5N2aAHJtsetcTHMitY4dtGmOpw4dlGqx\nluoz50kWJWQjVR+z6DLPnGE4uELOS8vbKHUoYPPQTT80eNVnl9S9h/l7DcjEAJYC\nIYJbf6+K9x+Ti9VRChvWcvgZQHMRym9j1g/7CKGMCIwkC+6ihkGD/XG40r7KRCyH\nbD53KnBjBO9FH4IL3rGlZWKWzMw3zC6RTS2ekfEsgAtYDvROKd4rNs+uDU9xaBLO\ndXTl5uxgudH2VnVzWtj09OUbBtXcQFD2IhmOl20BrckYul+HEIMR0oDibwIDAQAB\nMA0GCSqGSIb3DQEBBQUAA4IBAQCzTTH91jNh/uYBEFekSVNg1h1kPSujlwEDDf/W\npjqPJPqrZvW0w0cmYsYibNDy985JB87MJMfJVESG/v0Y/YbvcnRoi5gAenWXQNL4\nh2hf08A5wEQfLO/EaD1GTH3OIierKYZ6GItGrz4uFKHV5fTMiflABCdu37ALGjrA\nrIjwjxQG6WwLr9468hkKrWNG3dMBHKvmqO8x42sZOFRJMkqBbKzaBd1uW4xY5XwM\nS1QX56tVrgO0A3S+4dEg5uiLVN4YVP/Vqh4SMtYkL7ZZiZAxD9GtNnhRyFsWlC2r\nOVSdXs1ttZxEaEBGUl7tgsBte556BIvufZX+BXGyycVJdBu3\n-----END CERTIFICATE-----\n\",\"macId\":\"\",\"version\":5336}";

typedef enum {
    UploadState_Idle,
    UploadState_Login,
    UploadState_Starting,
} UploadState;

typedef struct __UploadEvt {
    UploadEventType type;
    uint32_t* pdata;
    uint32_t len;
} UploadEvt;

static QueueHandle_t queRecData=NULL;
PF_DUEROS_PLAY_CALLBACK *g_pfDuerOS_cb=NULL;

static void duer_event_hook(duer_event_t* event)
{
    if (!event) {
        ESP_LOGE(RECOG_SERVICE_TAG, "NULL event!!!");
    }

    ESP_LOGE(RECOG_SERVICE_TAG, "DuerOS Event: %d", event->_event);

    switch (event->_event) {
    case DUER_EVENT_STARTED:
        duer_init_alarm();

        break;
    case DUER_EVENT_STOPPED:
        duerOSEvtSend(UploadEvt_Quit, NULL, 0, 0);
        duerOSEvtSend(UploadEvt_Init, NULL, 0, 0);
        break;
    }
}

static void duerosTask(void* pvParameters)
{
    static UploadState uploadTaskState={0};
    static UploadEvt upEvent={0};
    while (1) {
        if (xQueueReceive(queRecData, &upEvent, portMAX_DELAY)) {
            if (upEvent.type == UploadEvt_Init) {
                //ESP_LOGI(RECOG_SERVICE_TAG, "Recv Que UploadEvt_Init");
                if (uploadTaskState < UploadState_Login) {
                    ESP_LOGI(RECOG_SERVICE_TAG, "duer_start, len:%d\n%s", strlen(DUER_OS_PROFILE), "DUER_OS_PROFILE");
                    duer_start(DUER_OS_PROFILE, strlen(DUER_OS_PROFILE));
                    uploadTaskState = UploadState_Login;
                } else {
                    ESP_LOGW(RECOG_SERVICE_TAG, "line:%d, uploadTaskState = %d", __LINE__, uploadTaskState);
                }
            } else if (upEvent.type == UploadEvt_Start) {
                ESP_LOGI(RECOG_SERVICE_TAG, "Recv Que UploadEvt_Start");
                if (UploadState_Login == uploadTaskState) {
                    duer_voice_start(16000);
                    uploadTaskState = UploadState_Starting;
                } else {
                    ESP_LOGW(RECOG_SERVICE_TAG, "line:%d, uploadTaskState = %d", __LINE__, uploadTaskState);
                }
            } else if (upEvent.type == UploadEvt_Data) {
                ESP_LOGI(RECOG_SERVICE_TAG, "Recv Que UploadEvt_Data");
                duer_voice_send(upEvent.pdata, upEvent.len);
                uploadTaskState = UploadState_Login;
                
            } else if (upEvent.type == UploadEvt_Stop)  {
                ESP_LOGI(RECOG_SERVICE_TAG, "Recv Que UploadEvt_Stop");
                duer_voice_stop();
                uploadTaskState = UploadState_Login;
                
            } else if (upEvent.type == UploadEvt_Quit)  {
                ESP_LOGI(RECOG_SERVICE_TAG, "Recv Que UploadEvt_Quit");
                uploadTaskState = UploadState_Idle;
                
            }
            ESP_LOGI(RECOG_SERVICE_TAG, "duerosTask-- runing around...");
        }
    }
    vTaskDelete(NULL);
}

void duerOSEvtSend(UploadEventType type, void* data, int len, int dir)
{
    UploadEvt evt = {0};
    evt.type = type;
    evt.pdata = data;
    evt.len = len;
    ESP_LOGI(RECOG_SERVICE_TAG, "Line:%d, duerOSEvtSend type=%d\r\n", __LINE__, type);
    if (dir) {
        xQueueSendToFront(queRecData, &evt, 0) ;
    } else {
        xQueueSend(queRecData, &evt, 0);
    }
}

static void duer_voice_result(struct baidu_json* uri)
{
    ESP_LOGI(RECOG_SERVICE_TAG, "*************uri: %s", baidu_json_Print(uri));
    ESP_LOGI(RECOG_SERVICE_TAG, "size: %d", baidu_json_GetArraySize(uri));
    if (uri) {
        baidu_json* root = uri;
        baidu_json* id = baidu_json_GetObjectItem(root, "id");
        if (id) {
            ESP_LOGI(RECOG_SERVICE_TAG, "id: %d", id->valueint);
        }


        baidu_json* speech = baidu_json_GetObjectItem(root, "speech");
        baidu_json* speechUrl = NULL;
        if (speech) {
            baidu_json* content = baidu_json_GetObjectItem(speech, "content");
            if (content && content->valuestring) {
                ESP_LOGI(RECOG_SERVICE_TAG, "content: %s", content->valuestring);
                //文字
                g_pfDuerOS_cb(NULL,content->valuestring,NULL);
            }
            speechUrl = baidu_json_GetObjectItem(speech, "url");
            if (speechUrl && speechUrl->valuestring) {
                ESP_LOGI(RECOG_SERVICE_TAG, "speech URL: %s", speechUrl->valuestring);
                //语音
                g_pfDuerOS_cb(speechUrl->valuestring,NULL,NULL);
            }
        }

        baidu_json* playload = baidu_json_GetObjectItem(root, "payload");
        baidu_json* playUrl = NULL;
        if (playload) {
            playUrl = baidu_json_GetObjectItem(playload, "play_behavior");
            if (playUrl && playUrl->valuestring) {
                ESP_LOGI(RECOG_SERVICE_TAG, "play_behavior: %s", playUrl->valuestring);
            }
            playUrl = baidu_json_GetObjectItem(playload, "url");
            if (playUrl && playUrl->valuestring) {
                ESP_LOGI(RECOG_SERVICE_TAG, "payload URL: %s", playUrl->valuestring);
				//音乐
                g_pfDuerOS_cb(NULL,NULL, playUrl->valuestring);
            }
            if (playUrl && playUrl->valuestring) {
                ESP_LOGI(RECOG_SERVICE_TAG, "playload: %s", playUrl->valuestring);
            }
        }
        baidu_json* header = baidu_json_GetObjectItem(root, "header");
        if (header && header->valuestring) {
            ESP_LOGI(RECOG_SERVICE_TAG, "header: %s", header->valuestring);
        }
    }
}


static void duerOSActive()
{
    if (xTaskCreate(duerosTask, "duerosTask", 8*1024, NULL, 5, NULL) != pdPASS) {
        ESP_LOGE(RECOG_SERVICE_TAG, "Error create duerosTask");
        return;
    }
    ESP_LOGE(RECOG_SERVICE_TAG, "success create duerosTask");
    duer_initialize();
    // Set the Duer Event Callback
    duer_set_event_callback(duer_event_hook);
    duer_init_voice_ctrl();
    // Set the voice interaction result
    duer_voice_set_result_callback(duer_voice_result);
    ESP_LOGI(RECOG_SERVICE_TAG, "duerOSActive\r\n");
}

static void duerOSDeactive()
{
    vQueueDelete(queRecData);
    queRecData = NULL;
    ESP_LOGI(RECOG_SERVICE_TAG, "duerOSDeactive\r\n");
}

void initDuerOS(PF_DUEROS_PLAY_CALLBACK*pf)
{
    g_pfDuerOS_cb=pf;

    queRecData = xQueueCreate(10, sizeof(UploadEvt));
    configASSERT(queRecData);
    duerOSActive();
    ESP_LOGI(RECOG_SERVICE_TAG, "duerOSService has create\r\n");
    
    duerOSEvtSend(UploadEvt_Init, NULL, 0, 0);
    return ;
}
