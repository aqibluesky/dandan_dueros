#ifndef _WIFI_H
#define _WIFI_H

#define TAG_WIFI "wifi:"

void app_wifi_softap(const char *ssid, const char *passwd);
void app_wifi_sta(const char *ssid , const char *passwd);
int wifiGetIP(char* dstIP,char* dstMask,char* dstGW);

#endif