#include "netstatus.h"

#define TAG "netstatus:"

EzhNetStatus		g_cur_netstatus;

void setCurNetStatus(EzhNetStatus ezh)
{
	g_cur_netstatus=ezh;
}

EzhNetStatus getCurNetStatus()
{
	//ESP_LOGI(TAG, "");
	return g_cur_netstatus;
}