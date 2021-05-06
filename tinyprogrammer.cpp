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


    // init_out(PIN::SCI, false);
    // gpio_put(PIN::SCI, 1);
    // sleep_ms(10);
    // gpio_put(PIN::SCI, 0);

    // init_out(PIN::SDI, false);
    // gpio_put(PIN::SDI, 1);
    // sleep_ms(10);
    // gpio_put(PIN::SDI, 0);

    // init_out(PIN::SII, false);
    // gpio_put(PIN::SII, 1);
    // sleep_ms(10);
    // gpio_put(PIN::SII, 0);

    // sleep_ms(1000);
    // bool b = false;
    // while(true)
    // {
    //     gpio_put(PIN::SCI, !b);
    //     gpio_put(PIN::SDI, !b);
    //     gpio_put(PIN::SII, b = !b);
    //     sleep_ms(4000);
    // }

    while(!is_power_safe || BOOST::testpower);
    printpower = false;
    sleep_us(1000);
    puts("\nstarting tinyprogrammer\n");
    printf("power safe at %.3f\n\n", boostvoltage);


    // uint delay = 0;
    // bool b = false;
    // int i = 0;
    // std::vector<int> vsend = {};
    // std::vector<int> vrec = {};
    // int n = 64;
    // vsend.reserve(n);
    // vrec.reserve(n);
    // for(int i = 0; i < n; i++)
    // {
    //     b = !b;
    //     sleep_us(1);
    //     gpio_put(PIN::SCI, b);
    //     bool r = gpio_get(PIN::SDO);
    //     vsend.push_back(!!b);
    //     vrec.push_back(!!r);
    // }
    // printf("sent: ");
    // for(int i = 0; i < n; i++)
    // {
    //     printf("%i", vsend[i]);
    // }
    // printf("\n");

    // printf("recv: ");
    // for(int i = 0; i < n; i++)
    // {
    //     printf("%i", vrec[i]);
    // }
    // printf("\n");

    // printf("eql:  ");
    // for(int i = 0; i < n; i++)
    // {
    //     printf("%i", !!(vsend[i] == vrec[i]));
    // }
    // printf("\n");
    // while(true);

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
