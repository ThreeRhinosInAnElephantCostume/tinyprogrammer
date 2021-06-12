    // This file is part of tinyavrprogrammer.

    // tinyavrprogrammer is free software: you can redistribute it and/or modify
    // it under the terms of the GNU General Public License as published by
    // the Free Software Foundation, version 3.

    // tinyavrprogrammer is distributed in the hope that it will be useful,
    // but WITHOUT ANY WARRANTY; without even the implied warranty of
    // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    // GNU General Public License for more details.

    // You should have received a copy of the GNU General Public License
    // along with tinyavrprogrammer.  If not, see <https://www.gnu.org/licenses/>.

#include "hmain.hpp"
int main()
{
    stdio_init_all();
    board_init();

    init_leds();

    if(watchdog_caused_reboot() && !DEBUG)
    {
        printf("THE PROGRAMMER HAS ENCOUNTERED A POWER, LOGIC, OR HARDWARE FAILURE!!!\n");
        printf("THE PROGRAMMER HAS ENCOUNTERED A POWER, LOGIC, OR HARDWARE FAILURE!!!\n");
        printf("THE PROGRAMMER HAS ENCOUNTERED A POWER, LOGIC, OR HARDWARE FAILURE!!!\n");
        printf("You should probably investigate this.\n");

        set_led_mode(LED::PROG_STATUS::GENERICERROR);
        bool b = false;
        while(true)
        {
            sleep_ms(100);
            gpio_put(PIN::LED, b = !b);
        }
    }

    set_led_mode(LED::PROG_STATUS::STARTING);

    init_out({ PIN::POWER, PIN::HIGHVOLT, PIN::LED }, false);
    init_out({PIN::SII, PIN::SCI, PIN::SDI}, false);
    init_in({ PIN::SDO }, false, true);

    gpio_put(PIN::LED, 1);

    printpower = true;
    multicore_launch_core1(core1_main);

    while(!ispowersafe || BOOST::testpower);
    printpower = false;
    sleep_us(1000);
    puts("\nstarting tinyprogrammer\n");
    printf("power safe at %.3f\n\n", boostvoltage);

    tusb_init();


    set_led_mode(LED::PROG_STATUS::READY);
    uint64 lastup = 0;
    while (true)
    {
        if(panicnow)
        {
            set_led_mode(LED::PROG_STATUS::POWERFAILURE);
            while(true);
        }
        tud_task();
        bool execcmd = usb_task();
        if(execcmd)
        {
            lastup = gettime();
        }
        else if((gettime()-lastup) > LED::RETURN_TO_READY_US)
        {
            if(chippowered)
            {
                if((gettime()-lastup) > CHIPS::CHIP_MAX_IDLE_US)
                {
                    power_off();
                    printf("Shuting down the chip after excessive inactivity\n");
                    if constexpr(DEBUG)
                        gracefulfailure();
                }
                set_led_mode(LED::PROG_STATUS::STILLPOWERED);
            }
            else 
                set_led_mode(LED::PROG_STATUS::READY);
        }
    }

    return 0;
}
