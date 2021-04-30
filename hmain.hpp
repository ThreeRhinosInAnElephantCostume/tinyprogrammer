#pragma once
#include "includes.hpp"
#include "pico/MyPicoLib.hpp"
#include "globals.hpp"
void disable_power();
inline void gracefulfailure()
{
    disable_power();
    sleep_us(10);
    multicore_fifo_push_blocking(MULTICORE::FAILURE_CODE);
    printf("RUNTIME ERROR!\n");
    if constexpr(DEBUG)
    {
        while(true);
    }
    else 
        assert("graceful failure");
}

void tick_power();
void init_power_control();
void core1_main();
void usb_task();