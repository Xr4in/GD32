#include "DELAY.h"

// 延时函数，单位为毫秒
void delay_ms(uint16_t ms)
{
    while(ms--)
    {
        uint16_t i = 20000; // 根据系统时钟频率调整循环次数
        while(i--);
    }
}
