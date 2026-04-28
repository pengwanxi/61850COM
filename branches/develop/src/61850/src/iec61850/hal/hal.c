#include "hal.h"
#include "led.h"

int hal_init()
{
    if (led_init() < 0) {
        return -1;
    }
    return 0;
}

int hal_exit()
{
    led_exit();
    return 0;
}
