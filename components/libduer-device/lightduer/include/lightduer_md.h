/**
 * Copyright (2017) Baidu Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// Author: Lelilang Zhang(zhangleliang@baidu.com)
//
// Description:  multi-dialog API

#ifndef BAIDU_DUER_LIGHTDUER_INCLUDE_LIGHTDUER_MD_H
#define BAIDU_DUER_LIGHTDUER_INCLUDE_LIGHTDUER_MD_H

#include "lightduer_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The reason of multi_dialog_close
 */
typedef enum {
    DUER_MD_CLOSE_REASON_REBOOT          = 0x01,     // reboot
    DUER_MD_CLOSE_REASON_MODEL_CHANGE    = 0x02,     // model change
    DUER_MD_CLOSE_REASON_MANUAL_EXIT     = 0x03,     // manual exit
    DUER_MD_CLOSE_REASON_TIME_OUT        = 0x04,      // timeout
    DUER_MD_CLOSE_REASON_OTHER           = 0x05,      // other
    DUER_MD_CLOSE_REASON_EXTENSION_START = 0x1000    // extension start number
} duer_reason_close_md_e;

// /**
//  * trigger to start the timers
//  */
void duer_md_start_timer(void);

// /**
//  * trigger to stop the timers
//  */
void duer_md_stop_timer(void);

/**
 * report event to exit the multi-dialog
 * @param reason, the reason to exit multi-cialog
 * @param need_play, if need the topicid update
 */
void duer_md_report_close_multi_dialog(duer_reason_close_md_e reason, duer_bool need_play);

#ifdef __cplusplus
}
#endif

#endif // BAIDU_DUER_LIGHTDUER_INCLUDE_LIGHTDUER_MD_H

