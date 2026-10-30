#ifndef __LED_H__
#define __LED_H__

#include "gd32f30x.h"
#include <stdio.h>

#define LED1 1
#define LED2 2

void LED_Init(void);// 初始化LED引脚
void LED_On(uint8_t LED_Num);// 打开LED
void LED_Off(uint8_t LED_Num);// 关闭LED

#endif
