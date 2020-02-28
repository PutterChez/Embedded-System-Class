#include "mbed.h"
#include "stm32f413h_discovery.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"
#include <string>

Semaphore one_slot(2);
Thread t2;
Thread t3;
TS_StateTypeDef  TS_State = {0};

void test_thread(void const *args) 
{
    int random_val = 0;
    int display_pos = 0;

    uint16_t color;

    if((const char*)args == "TEMPERATURE") {
        color = LCD_COLOR_RED;
        display_pos = -50;
    } 

    else if ((const char*)args == "HUMIDITY") {
        color = LCD_COLOR_BLUE;
        display_pos = 0;
    }

    else if ((const char*)args == "LIGHT") {
        color = LCD_COLOR_ORANGE;
        display_pos = 50;
    }

    while (true) 
    {
        random_val = rand() % 100;
        one_slot.wait();

        string text;
        text = to_string(random_val);
        
        BSP_LCD_SetTextColor(color);
        BSP_LCD_Clear(LCD_COLOR_WHITE);

        BSP_LCD_DisplayStringAt(0, 50, (uint8_t *)"Displaying:", CENTER_MODE);
        BSP_LCD_DisplayStringAt(-45, 150, (uint8_t *)"__", CENTER_MODE);
        BSP_LCD_DisplayStringAt(5, 150, (uint8_t *)"__", CENTER_MODE);
        BSP_LCD_DisplayStringAt(55, 150, (uint8_t *)"__", CENTER_MODE);
        

        string num_str;
        
        if(color == LCD_COLOR_RED){
            BSP_LCD_DisplayStringAt(0, 100, (uint8_t *)"Temperature", CENTER_MODE);
            num_str = to_string(random_val);
        }
        else if(color == LCD_COLOR_BLUE){
            BSP_LCD_DisplayStringAt(0, 100, (uint8_t *)"Humidity", CENTER_MODE);
            num_str = to_string(random_val);
        }
        else{
            BSP_LCD_DisplayStringAt(0, 100, (uint8_t *)"Light", CENTER_MODE);
            num_str = to_string(random_val);
        }
        
        const uint8_t *value = reinterpret_cast<const uint8_t*>(num_str.c_str());
        BSP_LCD_DisplayStringAt(display_pos, 140, (uint8_t *)value, CENTER_MODE);

        wait(1);
        one_slot.release();
    }
}

int main (void) 
{
    
    BSP_LCD_Init();
    if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) == TS_ERROR) {
        printf("BSP_TS_Init error\n");
    }

    BSP_LCD_Clear(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font24);

    t2.start(callback(test_thread, (void *)"HUMIDITY"));
    t3.start(callback(test_thread, (void *)"LIGHT"));

    test_thread((void *) "TEMPERATURE");
}