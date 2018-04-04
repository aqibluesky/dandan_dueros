#ifndef __ZH__NET_HTTP_SOCK_H__
#define __ZH__NET_HTTP_SOCK_H__

#ifdef __cplusplus
extern "C"{
#endif



#include <string.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

int get_socket_error_code(int socket);
void zhSockClose();
void zhPlatSleep(int ms);
esp_err_t zhSockConnect(char* SERVER_HOST,int SERVER_PORT);
int zhSockSend(char*buf,int buf_len);
int zhSockRecv(char*buf,int buf_len);


#ifdef __cplusplus
}
#endif
#endif