#include "mbed.h"
#include "stm32f413h_discovery_lcd.h"
 
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

Thread thread, thread2, thread3;

bool color = true;
 
 void led4_thread() 
{
    while (true) 
    {
        color = !color;
        
        if(color)
            BSP_LCD_Clear(LCD_COLOR_GREEN);
        else
            BSP_LCD_Clear(LCD_COLOR_RED);
        
        wait(3);
    }
}

 void led3_thread() 
{
    while (true) 
    {
        led3 = !led3;
        wait(2);
    }
}

void led2_thread() 
{
    while (true) 
    {
        led2 = !led2;
        wait(1);
    }
}
 
int main()
{
    BSP_LCD_Init();
    BSP_LCD_Clear(LCD_COLOR_WHITE);

    thread.start(led2_thread);
    thread2.start(led3_thread);
    thread3.start(led4_thread);

    while (true) 
    {
        led1 = !led1;
        wait(0.5);
    }
}