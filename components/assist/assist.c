#include "assist.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

//定时器
void delay_restart_task() 
{
	printf("delay delay_restart_task\n");
	//初始化延时
	vTaskDelay(3000/ portTICK_PERIOD_MS);
	//重启
	esp_restart();
}

void delay_restart()
{
	printf("delay restart\n");
	xTaskCreate(delay_restart_task, "delay_restart_task", 1024, NULL, 6, NULL);
}

//////////////////////////////////////////////////////////////////
/* 查找设备名称是否在buf里面
返回 0=成功
     1=失败
*/
int isDevExist(const char *buf,const char* name)
{
	char nabuf[128]={0};
	const char*p=NULL,*p1=NULL;
	int nn=0;
	p=strstr(buf,name);
	if(p)
	{
		p1=p;
		while(*p1!=',' && *p1!=0x00)
		{
			nabuf[nn]=*p1;
			nn++;
			p1++;
			if(nn>126)
			{break;}
		}
	}
	return strcmp(nabuf,name)?1:0;
}

//路径获取扩展名
void str_get_ext(const char*src,char*dst)
{
      int i=0;
      int l=0;
	  int n=0;

      l=strlen(src);
      for(i=l-2;i>0;i--)
      {
		  if( src[i]==0x00)
		  {
			  break;
		  }
          if(src[i]=='.')
          {
				 i++;
				break;
		  }
      }
	  for(n=0;;n++)
	  {
		dst[n]=src[i+n];
		if(dst[n]==0x00)
		{
			break;
		}
	  }
}
