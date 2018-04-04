#include "h_gpio.h"

PF_GPIO_STATUS* g_pf_h_gpio;
//闪烁间隔
int g_twinkleDelay=0;
int g_twinkleCacal=0;
int g_twinkleStatus=0;

typedef void PF_UART_READ (const char *buf, int len);

void task_gpio_thread()
{
	//闪烁
    while (1) {
	  //------------------------------------
	  //按钮
	  g_pf_h_gpio(GPIO_INPUT_IO_0, 0==BTN1_GetStatus );
	  g_pf_h_gpio(GPIO_INPUT_IO_1, 0==BTN2_GetStatus );

	  //------------------------------------
	  //闪烁
	  if(g_twinkleDelay>0)
	  {
		  if(0==g_twinkleCacal)
		  {
				g_twinkleStatus++;
				g_twinkleStatus%=2;
				LED1_OnOff(g_twinkleStatus);
		  }
		  g_twinkleCacal++;
		  g_twinkleCacal%=g_twinkleDelay;

	  }
	  else if(0==g_twinkleDelay) //长关
	  {
		  LED1_OnOff(0);
	  }
	  else if(-1==g_twinkleDelay) //长开
	  {
		  LED1_OnOff(1);
	  }
	  //------------------------------------
	  vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void h_gpio_init(PF_GPIO_STATUS* pf)
{
	g_pf_h_gpio=pf;
	//---
    gpio_config_t io_conf;                      // 创建GPIO实例
	//输出-----------------------------------------------------
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

	//输入-----------------------------------------------------
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en =0;
	io_conf.pull_down_en = 0;
    gpio_config(&io_conf);

	//打开LED灯
	led1_on();
	//线程
	xTaskCreate(task_gpio_thread, "task_gpio", 2048, NULL, 6, NULL);
}

//LED慢闪
void led1_slow_twinkle()
{
	g_twinkleDelay=10;
}

//快闪
void led1_quick_twinkle()
{
	g_twinkleDelay=1;
}

void led1_on()
{
	g_twinkleDelay=-1;
}
void led1_close()
{
	g_twinkleDelay=0;
}