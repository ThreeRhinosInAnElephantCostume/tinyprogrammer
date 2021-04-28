#include "hmain.hpp"

void usb_task()
{

}

int main()
{
    stdio_init_all();

    if(watchdog_caused_reboot())
    {
        printf("THE PROGRAMMER HAS ENCOUNTERED A POWER, LOGIC, OR HARDWARE FAILURE!!!\n");
        printf("THE PROGRAMMER HAS ENCOUNTERED A POWER, LOGIC, OR HARDWARE FAILURE!!!\n");
        printf("THE PROGRAMMER HAS ENCOUNTERED A POWER, LOGIC, OR HARDWARE FAILURE!!!\n");
        printf("You should probably investigate this.\n");
        while(true);
    }

    init_out({ PIN::POWER, PIN::SCI, PIN::SDI, PIN::SII, PIN_LED }, false);
    init_in({ PIN::SDO });

    gpio_put(PIN_LED, 1);

    multicore_launch_core1(core1_main);

    tusb_init();

    puts("starting tinyprogrammer");

    while (true)
    {
        usb_task();
    }

    return 0;
}
