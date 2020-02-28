#include "mbed.h"
#include "stm32f413h_discovery.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"
#include <string>

Mutex stdio_mutex;
Thread t2;
Thread t3;

TS_StateTypeDef  TS_State = {0};
    
void notify(uint16_t color, int num) 
{
    stdio_mutex.lock();
    BSP_LCD_SetTextColor(color);

    BSP_LCD_Clear(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(0, 50, (uint8_t *)"Displaying:", CENTER_MODE);

    string num_str;

    if(color == LCD_COLOR_RED){
        BSP_LCD_DisplayStringAt(0, 100, (uint8_t *)"Temperature=", CENTER_MODE);
        num_str = to_string(num) + " C";
    }
    else if(color == LCD_COLOR_BLUE){
        BSP_LCD_DisplayStringAt(0, 100, (uint8_t *)"Humidity=", CENTER_MODE);
        num_str = to_string(num) + " g/m3";
    }
    else{
        BSP_LCD_DisplayStringAt(0, 100, (uint8_t *)"Light=", CENTER_MODE);
        num_str = to_string(num) + " lux";
    }

    
    const uint8_t *value = reinterpret_cast<const uint8_t*>(num_str.c_str());

    BSP_LCD_DisplayStringAt(0, 140, (uint8_t *)value, CENTER_MODE);
    stdio_mutex.unlock();
    wait(1);
}

void test_thread(void const *args)
 {
    
    int val = 0;
    uint16_t color;
    if((const char*)args == "TEMPERATURE") {
        color = LCD_COLOR_RED;
    } else if ((const char*)args == "HUMIDITY") {
        color = LCD_COLOR_BLUE;
    } else if ((const char*)args == "LIGHT") {
        color = LCD_COLOR_ORANGE;
    }


    while (true) 
    {
        val = rand() % 100;
        BSP_TS_GetState(&TS_State);
        if(TS_State.touchDetected) {
            notify(color, val);   
            wait(1);
        }
        
    }
}

int main() 
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