#ifndef __HAL_GPIO_IN_OUT_H__
#define __HAL_GPIO_IN_OUT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

//LED�������
#define GPIO_OUTPUT_IO_0			GPIO_NUM_5
#define GPIO_OUTPUT_PIN_SEL			(((uint64_t)1)<<GPIO_OUTPUT_IO_0) 

//���밴������
#define GPIO_INPUT_IO_0				GPIO_NUM_34//��λWIFI����
#define GPIO_INPUT_IO_1				GPIO_NUM_35//
#define GPIO_INPUT_PIN_SEL			((((uint64_t)1)<<GPIO_INPUT_IO_0) | (((uint64_t)1)<<GPIO_INPUT_IO_1))

#define LED1_OnOff(b)				(gpio_set_level(GPIO_OUTPUT_IO_0, b))
#define BTN1_GetStatus				gpio_get_level(GPIO_INPUT_IO_0)
#define BTN2_GetStatus				gpio_get_level(GPIO_INPUT_IO_1)

//LED����
void led1_slow_twinkle();
//����
void led1_quick_twinkle();
//LED���͹�
void led1_on();
void led1_close();

//�ص�
typedef void PF_GPIO_STATUS (int gpio_num, int is_press);
void h_gpio_init(PF_GPIO_STATUS* pf);


#ifdef __cplusplus
}
#endif

#endif /*#ifndef __HAL_GPIO_IN_OUT_H__*/