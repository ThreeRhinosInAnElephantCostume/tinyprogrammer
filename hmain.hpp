#pragma once
#include "includes.hpp"
#include "pico/MyPicoLib.hpp"
#include "globals.hpp"
#include "hvp.hpp"
#include "usbcomms.hpp"
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

inline void set_leds(LED::Config lc)
{
    struct 
    {
        uint8 pin, v;
    }data[3] = {{PIN::RED, lc.red}, {PIN::GREEN, lc.green}, {PIN::BLUE, lc.blue}};
    for (uint8 i = 0; i < ArraySize(data); i++)
    {
        pwm_set_gpio_level(data[i].pin, LED::OFFSET + data[i].v);
    }
}
inline void set_led_mode(LED::PROG_STATUS status)
{
    static int laststatus = -1;
    if(laststatus == (int)status)
        return;
    laststatus = (int)status;
    LED::Config lc;
    for(uint i = 0; i < ArraySize(LED::configs); i++) 
    {
        if(LED::configs[i].status == status)
        {
            lc = LED::configs[i];
            break;
        }
    }
    set_leds(lc);
}
inline void init_leds()
{
    for(auto it : std::vector<uint8>{PIN::RED, PIN::GREEN, PIN::BLUE})
    {
        gpio_init(it);
        gpio_set_function(it, GPIO_FUNC_PWM);
        uint slice = pwm_gpio_to_slice_num(it);
        pwm_set_wrap(slice, LED::WRAP);
        pwm_set_clkdiv_int_frac(slice, LED::CLOCK_DIV, 0);
        pwm_set_gpio_level(it, 0);
        pwm_set_enabled(slice, true);
    }
}