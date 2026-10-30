#include "LED.h"

// 初始化LED
void LED_Init(void)
{
    // 初始化使能LED引脚所属的GPIO时钟
    rcu_periph_clock_enable(RCU_GPIOA);
    // 初始化使能LED引脚所属的AF时钟(复用功能时钟)
    rcu_periph_clock_enable(RCU_AF);
    // 配置LED引脚为推挽输出模式
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1);
    gpio_bit_set(GPIOA, GPIO_PIN_0); // 默认关闭LED1
    gpio_bit_reset(GPIOA, GPIO_PIN_1); // 默认关闭LED2
}

// 打开LED
void LED_On(uint8_t LED_Num)
{
    switch(LED_Num)
    {
        case LED1:
            gpio_bit_reset(GPIOA, GPIO_PIN_0);
            break;
        case LED2:
            gpio_bit_set(GPIOA, GPIO_PIN_1);
            break;
        default:
            break;
    }
}

// 关闭LED
void LED_Off(uint8_t LED_Num)
{
    switch(LED_Num)
    {
        case LED1:
            gpio_bit_set(GPIOA, GPIO_PIN_0);
            break;
        case LED2:
            gpio_bit_reset(GPIOA, GPIO_PIN_1);
            break;
        default:
            break;
    }
}
