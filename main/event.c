//event engine
#include "event.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "h_gpio.h"

#include "wifi.h"

#include "cJSON.h"
#include "dirent.h"

#include "netstatus.h"
#include "h_gpio.h"

#define TAG "myevent:"

//事件
//EventGroupHandle_t	event_group0 = xEventGroupCreate();
//xEventGroupSetBits(event_group0 , BIT0);
//xEventGroupClearBits(event_group0 , BIT0); //BIT0,BIT1,BIT0,BIT2
//xEventGroupWaitBits(event_group0 , BIT0,pdTRUE,pdTRUE,portMAX_DELAY);


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
	case SYSTEM_EVENT_WIFI_READY:            /**< ESP32 WiFi ready */
			break;
    case SYSTEM_EVENT_SCAN_DONE:               /**< ESP32 finish scanning AP */
			break;
    case SYSTEM_EVENT_STA_START:                /**< ESP32 station start */
		{
			setCurNetStatus(ezhNetStat_NoNet);
			esp_wifi_connect();
			led1_on(); //状态LED亮
		}
			break;
    case SYSTEM_EVENT_STA_STOP:                 /**< ESP32 station stop */
			break;
    case SYSTEM_EVENT_STA_CONNECTED:            /**< ESP32 station connected to AP */
			break;
    case SYSTEM_EVENT_STA_DISCONNECTED:         /**< ESP32 station disconnected from AP */
		{
				ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED ");
				setCurNetStatus(ezhNetStat_NoNet);
				led1_on(); //状态LED亮
				
				//重新连接路由器
				vTaskDelay(100 / portTICK_PERIOD_MS);
				esp_wifi_disconnect();
				esp_wifi_stop();
				esp_wifi_connect();
		}
			break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:      /**< the auth mode of AP connected by ESP32 station changed */
			break;
    case SYSTEM_EVENT_STA_GOT_IP:               /**< ESP32 station got IP from connected AP */
		{
					setCurNetStatus(ezhNetStat_NET_OK);
					led1_close();//状态LED关
    			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP:%s\n",	ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
					
		}
			break;
    /*case SYSTEM_EVENT_STA_LOST_IP:              //< ESP32 station lost IP and the IP is reset to 0
		{
					ESP_LOGI(TAG, "SYSTEM_EVENT_STA_LOST_IP ");
					setCurNetStatus(ezhNetStat_NoNet);
					led1_on(); //状态LED亮
					
					//重新连接路由器
					vTaskDelay(100 / portTICK_PERIOD_MS);
					esp_wifi_disconnect();
					esp_wifi_stop();
					esp_wifi_connect();
		}
			break;*/
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:       /**< ESP32 station wps succeeds in enrollee mode */
			break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:        /**< ESP32 station wps fails in enrollee mode */
			break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:      /**< ESP32 station wps timeout in enrollee mode */
			break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:           /**< ESP32 station wps pin code in enrollee mode */
			break;
    case SYSTEM_EVENT_AP_START:                /**< ESP32 soft-AP start */
		{
		}
			break;
    case SYSTEM_EVENT_AP_STOP:                 /**< ESP32 soft-AP stop */
			break;
    case SYSTEM_EVENT_AP_STACONNECTED:          /**< a station connected to ESP32 soft-AP */
		{
				ESP_LOGI(TAG, "station:"MACSTR" join,AID=%d\n",
				MAC2STR(event->event_info.sta_connected.mac),
				event->event_info.sta_connected.aid);
		}
			break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:      /**< a station disconnected from ESP32 soft-AP */
		{
				ESP_LOGI(TAG, "station:"MACSTR"leave,AID=%d\n",
				MAC2STR(event->event_info.sta_disconnected.mac),
				event->event_info.sta_disconnected.aid);
		}
			break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:        /**< Receive probe request packet in soft-AP interface */
			break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:           /**< ESP32 station or ap interface v6IP addr is preferred */
			break;
    case SYSTEM_EVENT_ETH_START:                /**< ESP32 ethernet start */
			break;
    case SYSTEM_EVENT_ETH_STOP:                 /**< ESP32 ethernet stop */
			break;
    case SYSTEM_EVENT_ETH_CONNECTED:           /**< ESP32 ethernet phy link up */
			break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:        /**< ESP32 ethernet phy link down */
		{
			setCurNetStatus(ezhNetStat_NoNet);
		}
			break;
    case SYSTEM_EVENT_ETH_GOT_IP:               /**< ESP32 ethernet got IP from connected AP */
		{
			setCurNetStatus(ezhNetStat_NET_OK);
		}
			break;
    case SYSTEM_EVENT_MAX:
			break;
    default:
        break;
    }
    return ESP_OK;
}


void event_engine_init()
{
	esp_event_loop_init(event_handler,NULL);
}
