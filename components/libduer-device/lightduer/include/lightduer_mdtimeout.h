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
// Description: timeout strategy in multi-dialog

#ifndef BAIDU_DUER_LIGHTDUER_INCLUDE_LIGHTDUER_MDTIMEOUT_H
#define BAIDU_DUER_LIGHTDUER_INCLUDE_LIGHTDUER_MDTIMEOUT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * create the children_puzzle_silent_timer
 * @param interval the interval to invoke the callback, in milli-second
 * @param priv_data the private data use in the callback, will be free when destroy the timer
 *        priv_data is string as below
 *      {"data":{"query":"ai.dueros.puzzle.timeout","timeout":30},"type":"children_puzzle_ref"}
 * @return >0 OK, <0 fail
 */
int duer_md_create_children_puzzle_silent_timer(int interval, void *priv_data);

/**
 * create the wordschain_silent_timer
 * @param interval the interval to invoke the callback, in milli-second
 * @param priv_data the private data use in the callback, will be free when destroy the timer
 *        priv_data is string as below
 *      {"data":{"query":"wordschain.timeout","timeout":30},"type":"wordschain_ref"}
 * @return >0 OK, <0 fail
 */
int duer_md_create_wordschain_silent_timer(int interval, void *priv_data);

/**
 * create the common_silent_timer
 * @param interval the interval to invoke the callback, in milli-second
 * @param priv_data the private data use in the callback, will be free when destroy the timer
 *        priv_data is string as below
 *      {"id": xxxx, "data":{"query":"xxxxxxxxx","timeout":xx},"type":"timeout_ref"}
 * @return >0 OK, <0 fail
 */
int duer_md_create_common_silent_timer(int interval, void *priv_data);

/**
 * trigger to start the timers
 */
void duer_md_start_silent_timer(void);

/**
 * trigger to stop the timers
 */
void duer_md_stop_silent_timer(void);

#ifdef __cplusplus
}
#endif

#endif // BAIDU_DUER_LIGHTDUER_INCLUDE_LIGHTDUER_MDTIMEOUT_H

