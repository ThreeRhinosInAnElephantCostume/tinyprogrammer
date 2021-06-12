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