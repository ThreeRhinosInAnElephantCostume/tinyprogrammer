#include "hmain.hpp"

void core1_main()
{
    watchdog_enable(10, true);
    init_power_control();
    while(true)
    {
        watchdog_update();
        tick_power();
    }
}