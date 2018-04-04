
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/uart.h"


#define DGB_PRINTF(...) (printf(__VA_ARGS__),printf("\r\n"))

//
void delay_restart();

//////////////////////////////////////////////////////////////////
/* 
结果 0=没有找到
     1=找到名称
*/
int isDevExist(const char *buf,const char* name);

//获取扩展名
void str_get_ext(const char*src,char*dst);
