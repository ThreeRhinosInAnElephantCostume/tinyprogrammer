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

    init_out({ PIN::POWER, PIN::HIGHVOLT, PIN::LED }, false);
    init_in({ PIN::SDO });

    gpio_put(PIN::LED, 1);

    printpower = true;
    multicore_launch_core1(core1_main);
    while(!is_power_safe);
    printpower = false;
    printf("\npower safe at %.3f\n", boostvoltage);

    puts("starting tinyprogrammer\n");

    tusb_init();

    while (true)
    {
        tud_task();
        usb_task();
    }

    return 0;
}
