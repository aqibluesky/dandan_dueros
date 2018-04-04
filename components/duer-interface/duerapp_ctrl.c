#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "esp_types.h"

#include "duerapp_config.h"
#include "lightduer_net_ntp.h"
#include "lightduer_types.h"
#include "lightduer_memory.h"
#include "lightduer_connagent.h"
#include "lightduer_coap_defs.h"
#include "lightduer_lib.h"

#define STR_LEN 10
void power_on_off(duer_context ctx, duer_msg_t *msg, duer_addr_t *addr)
{
    //TODO what's the purpose of the parameter ctx here??
    char strVol[10] = {0};
    int msg_code = DUER_MSG_RSP_CHANGED;
    duer_handler handler = (duer_handler)ctx;

    if (handler && msg) {
        if (msg->msg_code == DUER_MSG_REQ_GET) {
            int volume = 0;
            DUER_LOGI("power_on_off = %d,DUER_RES_OP_GET", volume);
        } else if (msg->msg_code == DUER_MSG_REQ_PUT) {
            DUER_LOGI("power_on_off DUER_RES_OP_PUT");
            if (msg->payload && msg->payload_len > 0 && msg->payload_len < 10) {
                memset(strVol, 0, sizeof(strVol));
                memcpy(strVol, msg->payload, msg->payload_len);
                DUER_LOGI("power_on_off set : %s, len :%d", strVol, msg->payload_len);
                if (0 == strncmp("true", strVol, strlen("true"))) {
                    DUER_LOGI("It will enter deepsleeping");
                }
            } else {
                msg_code = DUER_MSG_RSP_BAD_REQUEST;
            }
        }
        duer_response(msg, msg_code, strVol, strlen(strVol));
    }
}


void volume_up_down(duer_context ctx, duer_msg_t *msg, duer_addr_t *addr)
{
    duer_handler handler = (duer_handler)ctx;
    char strVol[10] = {0};
    int msg_code = DUER_MSG_RSP_CHANGED;
    if (handler && msg) {
        if (msg->msg_code == DUER_MSG_REQ_GET) {
            int volume = 0;
            //获取音量
            memset(strVol, 0, sizeof(strVol));
            int tmp = volume / 6;
            snprintf(strVol, STR_LEN, "%d", tmp);
            DUER_LOGI("Volume = %d,DUER_RES_OP_GET", tmp);

        } else if (msg->msg_code == DUER_MSG_REQ_PUT) {
            DUER_LOGI("Volume DUER_RES_OP_PUT");
            if (msg->payload && msg->payload_len > 0 && msg->payload_len < 10) {
                memset(strVol, 0, sizeof(strVol));
                memcpy(strVol, msg->payload, msg->payload_len);
                DUER_LOGI("Volume set : %s,len:%d", strVol, msg->payload_len);
                strVol[msg->payload_len] = 0;
                int vol = atol(strVol);
                if (vol > 16 || vol < 0) {
                    msg_code = DUER_MSG_RSP_FORBIDDEN;
                } else {
                    //设置音量
                }
            } else {
                msg_code = DUER_MSG_RSP_BAD_REQUEST;
            }
        }
        duer_response(msg, msg_code, strVol, strlen(strVol));
    }
}

void duer_init_voice_ctrl()
{
    duer_res_t res[] = {
        {DUER_RES_MODE_DYNAMIC, DUER_RES_OP_PUT | DUER_RES_OP_GET, "switch", .res.f_res = power_on_off},
        {DUER_RES_MODE_DYNAMIC, DUER_RES_OP_PUT | DUER_RES_OP_GET, "volume", .res.f_res = volume_up_down},
    };
    duer_add_resources(res, sizeof(res) / sizeof(res[0]));
}
