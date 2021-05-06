#include "hmain.hpp"

void core1_main()
{
    if constexpr(!DEBUG)
        watchdog_enable(10, true);
    init_power_control();
    while(true)
    {
        while(multicore_fifo_rvalid())
        {
            uint code = multicore_fifo_pop_blocking();
            switch(code)
            {
                case MULTICORE::FAILURE_CODE:
                {
                    disable_power();
                    return;
                    break;
                }
            }
        }
        if constexpr(!DEBUG)
            watchdog_update();
        tick_power();
    }
}