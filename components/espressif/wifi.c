

#include "esp_system.h"
#include "esp_err.h"
#include "esp_event_loop.h"
#include "esp_event.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_eth.h"

#include "rom/ets_sys.h"
#include "rom/gpio.h"

#include "soc/dport_reg.h"
#include "soc/io_mux_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_sig_map.h"

#include "nvs_flash.h"
#include "driver/gpio.h"

#include "eth_phy/phy_lan8720.h"
#include "eth.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "esp_log.h"
#include "esp_event_loop.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "wifi.h"
#include "tcpip_adapter.h"

extern esp_err_t tcpip_adapter_ap_start(uint8_t *mac, tcpip_adapter_ip_info_t *ip_info);




#define EXAMPLE_MAX_STA_CONN 1 


void app_wifi_sta(const char *ssid , const char *passwd)
{
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_FLASH));

	wifi_config_t wifi_config={0};
	strcpy((char*)wifi_config.sta.ssid,ssid);
	strcpy((char*)wifi_config.sta.password,passwd);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG_WIFI, "wifi_init_sta finished.");
    ESP_LOGI(TAG_WIFI, "connect to SSID:%s , password:%s \n", ssid,passwd);}

void app_wifi_softap(const char *ssid, const char *passwd)
{	
	//修改IP地址
	tcpip_adapter_ip_info_t		ip_info={0};
	IP4_ADDR(&ip_info.ip, 192, 168 , 1, 10);
	IP4_ADDR(&ip_info.gw, 192, 168 , 1, 10);
	IP4_ADDR(&ip_info.netmask, 255, 255 , 255, 0);
	//获取APP模式下的MAC地址
	uint8_t mac_addr[6]={0};
    esp_err_t err = esp_read_mac(mac_addr, ESP_MAC_WIFI_SOFTAP);
	if (err != ESP_OK){
		int i=0;
		for(;i<6;i++) mac_addr[i]=i;
	};
	tcpip_adapter_ap_start(mac_addr,&ip_info);
	//初始化
	wifi_config_t wifi_config={0};
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	char apSSID[64]={0};
	sprintf(apSSID,"%s%02x%02x%02x",ssid,mac_addr[3],mac_addr[4],mac_addr[5]);

	strcpy((char*)wifi_config.ap.ssid,apSSID);
	wifi_config.ap.ssid_len=strlen(apSSID)+1;
	strcpy((char*)wifi_config.ap.password,passwd);
	wifi_config.ap.max_connection=EXAMPLE_MAX_STA_CONN;
	wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

	if(0==strlen(passwd))
	{ wifi_config.ap.authmode = WIFI_AUTH_OPEN; }    

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));   	
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG_WIFI, "wifi_init_softap finished.SSID:%s password:%s \n",apSSID, passwd);
}

int wifiGetIP(char* dstIP,char* dstMask,char* dstGW)
{
	dstIP[0]=0;
	dstMask[0]=0;
	dstGW[0]=0;

	tcpip_adapter_ip_info_t ip={0};
	memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
	if (tcpip_adapter_get_ip_info(ESP_IF_WIFI_STA, &ip) == 0) {

			sprintf(dstIP,IPSTR, IP2STR(&ip.ip));
			sprintf(dstMask,IPSTR, IP2STR(&ip.netmask));
			sprintf(dstGW,IPSTR, IP2STR(&ip.gw));
			return 0;
	}
	return 1;
}