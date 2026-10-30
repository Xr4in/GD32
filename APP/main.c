#include "gd32f30x.h"
#include <stdio.h>
#include "LED.h"
#include "DELAY.h"

int main()
{
    LED_Init();

while(1)
    {
        LED_On(LED1);
        LED_Off(LED2);
        delay_ms(1000);
        LED_On(LED2);
        LED_Off(LED1);
        delay_ms(1000);
    }
}
