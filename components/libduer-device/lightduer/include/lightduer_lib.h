// Copyright 2017 Baidu Inc. All Rights Reserved.
// Author: Su Hao (suhao@baidu.com)
//
// Description: The internal common header.

#ifndef BAIDU_IOT_TINYDU_IOT_OS_SRC_IOT_BAIDU_CA_SOURCE_BAIDU_CA_INTERNAL_H
#define BAIDU_IOT_TINYDU_IOT_OS_SRC_IOT_BAIDU_CA_SOURCE_BAIDU_CA_INTERNAL_H

#include <string.h>

#define DUER_MEMCPY(...)     memcpy(__VA_ARGS__)
#define DUER_MEMCMP(...)     memcmp(__VA_ARGS__)
#define DUER_MEMSET(...)     memset(__VA_ARGS__)
#define DUER_STRLEN(...)     strlen(__VA_ARGS__)
#define DUER_SNPRINTF(...)   snprintf(__VA_ARGS__)

// Suppress Compiler warning Function&Variable declared never referenced
#define ALLOW_UNUSED_LOCAL(VAR_FUNC) (void)(VAR_FUNC)

#endif // BAIDU_IOT_TINYDU_IOT_OS_SRC_IOT_BAIDU_CA_SOURCE_BAIDU_CA_INTERNAL_H
