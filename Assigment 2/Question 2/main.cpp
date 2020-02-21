#include "mbed.h"
#include <string>
#include "stm32f413h_discovery.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"

TS_StateTypeDef  TS_State = {0};

Timer timer_fast;

int main()
{
    uint16_t x1, y1;

    BSP_LCD_Init();

    /* Touchscreen initialization */
    if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) == TS_ERROR) {
        printf("BSP_TS_Init error\n");
    }

    /* Clear the LCD */
    BSP_LCD_Clear(LCD_COLOR_WHITE);

    /* Set Touchscreen Demo1 description */
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());

    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_GREEN);

    int touches = 0;
        
    int game = 0; 
    
    while (1) {
        BSP_TS_GetState(&TS_State);

        string touches_str = to_string(touches);
        string timeout = to_string(timer_fast.read());

        const uint8_t *touches_display = reinterpret_cast<const uint8_t*>(touches_str.c_str());
        const uint8_t *timeout_display = reinterpret_cast<const uint8_t*>(timeout.c_str());

        
        if(game == 0){
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
            BSP_LCD_SetFont(&Font24);
            BSP_LCD_DisplayStringAtLine(1, (uint8_t *) "Tap to Start!");

            if(TS_State.touchDetected){
                BSP_LCD_Clear(LCD_COLOR_GREEN);
                timer_fast.reset();
                timer_fast.start();
                game = 1;
            }
        }

        if(game == 1){
            
            BSP_LCD_Clear(LCD_COLOR_GREEN);

            BSP_LCD_SetFont(&Font16);
            BSP_LCD_DisplayStringAtLine(1, (uint8_t *) "Number of Touches:");
            
            BSP_LCD_SetFont(&Font24);
            BSP_LCD_DisplayStringAtLine(2, (uint8_t *) touches_display);

            
            BSP_LCD_SetFont(&Font16);
            BSP_LCD_DisplayStringAtLine(6, (uint8_t *) "Time left:");

            BSP_LCD_SetFont(&Font24);
            BSP_LCD_DisplayStringAtLine(5, (uint8_t *) timeout_display);

            if(TS_State.touchDetected) {
                touches++;
            }

            if(timer_fast.read() >= 5){
                timer_fast.stop();
                thread_sleep_for(500);

                BSP_LCD_Clear(LCD_COLOR_RED);
                game = 2;
            }

            thread_sleep_for(100);
        }

        if(game == 2){

            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_SetBackColor(LCD_COLOR_RED);
            BSP_LCD_SetFont(&Font16);
            BSP_LCD_DisplayStringAtLine(6, (uint8_t *) "Total Score:");

            BSP_LCD_SetFont(&Font24);
            BSP_LCD_DisplayStringAtLine(5, (uint8_t *) touches_display);

            BSP_LCD_DisplayStringAtLine(6, (uint8_t *) "Tap to Retry");

            if(TS_State.touchDetected){
                touches = 0;
                game = 0;
                BSP_LCD_Clear(LCD_COLOR_GREEN);
            }
        }

    }
}
