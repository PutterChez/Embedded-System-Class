#include "mbed.h"

DigitalOut led(LED1);
DigitalIn button(PA_0);

int main()
{
    while (true)
    {
        if(led.read() == 1) {
            if(button.read() == 1)
             led.write(0);
        } 

        else if (led.read() == 0) {
            if(button.read() == 1){
                thread_sleep_for(3000);
                
                if(button.read() == 1) {
                    led.write(1);

                    while (true) {
                        if (button.read() == 0)
                            break;
                    }
                }
            }
        }
        
    }
}