#include "hmain.hpp"

int main()
{
    stdio_init_all();
    board_init();

    if(watchdog_caused_reboot() && !DEBUG)
    {
        printf("THE PROGRAMMER HAS ENCOUNTERED A POWER, LOGIC, OR HARDWARE FAILURE!!!\n");
        printf("THE PROGRAMMER HAS ENCOUNTERED A POWER, LOGIC, OR HARDWARE FAILURE!!!\n");
        printf("THE PROGRAMMER HAS ENCOUNTERED A POWER, LOGIC, OR HARDWARE FAILURE!!!\n");
        printf("You should probably investigate this.\n");

        bool b = false;
        while(true)
        {
            sleep_ms(100);
            gpio_put(PIN::LED, b = !b);
        }
    }

    init_out({ PIN::POWER, PIN::SCI, PIN::SDI, PIN::SII, PIN::LED }, false);
    init_in({ PIN::SDO });

    gpio_put(PIN::LED, 1);

    multicore_launch_core1(core1_main);

    tusb_init();

    puts("starting tinyprogrammer");

    while (true)
    {
        tud_task();
        usb_task();
    }

    return 0;
}
