/* tcp_perf Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include <lwip/opt.h>
#include <lwip/api.h>
#include <lwip/dns.h>
#include <lwip/ip.h>
#include <stddef.h> 
#include <lwip/netbuf.h>
#include <lwip/sys.h>
#include <lwip/ip_addr.h>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include <lwip/netdb.h>
#include "zhsock.h"

#define TAG		"zhsock"

//send data
void http_files_send(char* databuff, int len);
void hxtcp_close_socket();

/*socket*/
static struct sockaddr_in hf_serv_addr;
static int hf_conn_sock = 0;

int zhsock_error_code(int socket);
int zhsock_show_error_reason(const char *str, int socket);

//-------------------------------------------------------------
//send data
int zhSockSend(char*buf,int buf_len)
{
		int len;
        len = send(hf_conn_sock, buf , buf_len, 0);
        if (len > 0) {
        } else {
            int err = zhsock_error_code(hf_conn_sock);
            if (err != ENOMEM) {
                zhsock_show_error_reason("send_data", hf_conn_sock);
            }
        }
		return len;
 }

//use this esp32 as a tcp client. return ESP_OK:success ESP_FAIL:error
esp_err_t zhSockConnect(char* SERVER_HOST,int SERVER_PORT)
{
		char SERVER_IP[20]={0};
		ESP_LOGI(TAG, "zhSockConnect.   addr=%s:%d\n", SERVER_HOST, SERVER_PORT);
		if(0==SERVER_HOST[0])
				return ESP_FAIL;
		if(0==SERVER_PORT)
				return ESP_FAIL;
		zhSockClose();
		//---------------------------------------------------------
		//ESP_LOGI(TAG, "zhSockConnect.   begin get host ip\n");
		//获取服务器IP
		struct hostent * host = gethostbyname (SERVER_HOST);
		char* dn_or_ip = (char *)inet_ntoa(*(struct in_addr *)(host->h_addr));
		if(dn_or_ip)
		{
			//printf("zhSockConnect---------- >>> %s... ip=%s\n",SERVER_HOST,dn_or_ip);
			strcpy(SERVER_IP,dn_or_ip);

			//---------------------------------------------------------
			//客户端连接
			hf_conn_sock = socket(AF_INET, SOCK_STREAM, 0);
			if (hf_conn_sock < 0) {
				zhsock_show_error_reason("create client", hf_conn_sock);
				return ESP_FAIL;
			}

			//复位端口
			unsigned int len;
			int opt;
			opt = 1;
			len = sizeof(opt);
			if(setsockopt(hf_conn_sock,SOL_SOCKET,SO_REUSEADDR,
			(const void*)&opt,len)==-1)
			{
				ESP_LOGE("hxkong_tcp", "Failed to SO_REUSEADDR socket. Error %d", errno);
				close(hf_conn_sock);
				hf_conn_sock=0;
				return ESP_FAIL;
			}

			hf_serv_addr.sin_family = AF_INET;
			hf_serv_addr.sin_port = htons(SERVER_PORT);
			hf_serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
			ESP_LOGI(TAG, "connecting to server...");
			if (connect(hf_conn_sock, (struct sockaddr *)&hf_serv_addr, sizeof(hf_serv_addr)) < 0) {
				zhsock_show_error_reason("client connect", hf_conn_sock);
				return ESP_FAIL;
			}
			ESP_LOGI(TAG, "connect to server success!");
			return ESP_OK;
		}

		printf("----------------- >>> %s  get ip error\n",SERVER_HOST);
		return ESP_FAIL;
}

int zhsock_error_code(int socket)
{
    int result;
    u32_t optlen = sizeof(int);
    int err = getsockopt(socket, SOL_SOCKET, SO_ERROR, &result, &optlen);
    if (err == -1) {
        ESP_LOGE(TAG, "getsockopt failed:%s", strerror(err));
        return -1;
    }
    return result;
}

void zhSockClose()
{
	if(hf_conn_sock>0)
	{
		close(hf_conn_sock);
		hf_conn_sock=0;
	}
}

void zhPlatSleep(int ms)
{
	vTaskDelay(ms / portTICK_RATE_MS);
}

int zhSockRecv(char*buf,int buf_len)
{
	return recv(hf_conn_sock, buf , buf_len, 0);
}

int zhsock_show_error_reason(const char *str, int socket)
{
    int err = zhsock_error_code(socket);

    if (err != 0) {
        ESP_LOGW(TAG, "%s socket error %d %s", str, err, strerror(err));
    }

    return err;
}