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
    init_out({PIN::SII, PIN::SCI, PIN::SDI}, false);
    init_in({ PIN::SDO }, false, true);

    gpio_put(PIN::LED, 1);

    printpower = true;
    multicore_launch_core1(core1_main);

    while(!is_power_safe || BOOST::testpower);
    printpower = false;
    sleep_us(1000);
    puts("\nstarting tinyprogrammer\n");
    printf("power safe at %.3f\n\n", boostvoltage);

    tusb_init();

    while (true)
    {
        if(panicnow)
        {
            while(true);
        }
        tud_task();
        usb_task();
    }

    return 0;
}
